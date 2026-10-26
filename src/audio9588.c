/* Bounded cooperative PCM producer for the verified 9588 raw audio queue. */
#ifdef JOEY_AUDIO_HOST_TEST
#include "audio_host_platform.h"
#else
#include "platform9588.h"
#include "duel_paths.h"
#endif
#include "duel_audio.h"
#include "duel_ima.h"

#define OUTPUT_RATE9588 22050u
#define OUTPUT_FRAMES9588 2048u
#define ENCODED_BLOCKS9588 16u
#define EFFECT_SLOTS9588 2u
#define PUMP_LIMIT9588 3u
typedef struct {uint32_t offset,bytes,frames,rate,loop,gain,family,id;} AudioEntry9588;
typedef struct {
    unsigned asset,frame,fraction,decoded_block,encoded_first,encoded_count;
    int16_t pcm[JOEY_IMA_BLOCK_FRAMES];
    uint8_t encoded[ENCODED_BLOCKS9588*JOEY_IMA_BLOCK_BYTES];
} Stream9588;
static AudioEntry9588 audio_entries[JOEY_AUDIO_ASSET_COUNT];
static Stream9588 music9588,effects9588[EFFECT_SLOTS9588];
static unsigned effect_stamp9588[EFFECT_SLOTS9588],effect_clock9588;
static h1_file *sound9588;
static unsigned sound_open9588,pumping9588,music_wanted9588;
static unsigned pending_at9588,pending_bytes9588,music_fade9588;
static int music_last9588,music_from9588;
static int16_t output9588[OUTPUT_FRAMES9588] __attribute__((aligned(32)));
/* C200 applies attenuation in place before copying into its DMA queue. */
static int16_t submit9588[OUTPUT_FRAMES9588] __attribute__((aligned(32)));
volatile uint32_t joey_audio_diagnostic[32];

static void stream_reset9588(Stream9588 *s,unsigned id){
    s->asset=id;s->frame=s->fraction=s->encoded_count=0;
    s->decoded_block=s->encoded_first=~0u;
}
static int stream_sample9588(Stream9588 *s,unsigned frame){
    const AudioEntry9588 *e=&audio_entries[s->asset];
    unsigned block=frame/JOEY_IMA_BLOCK_FRAMES,at=frame%JOEY_IMA_BLOCK_FRAMES;
    if(block!=s->decoded_block){
        if(!s->encoded_count||block<s->encoded_first||block-s->encoded_first>=s->encoded_count){
            unsigned available=e->bytes/JOEY_IMA_BLOCK_BYTES-block;
            unsigned blocks=available<ENCODED_BLOCKS9588?available:ENCODED_BLOCKS9588;
            unsigned bytes=blocks*JOEY_IMA_BLOCK_BYTES;
            if(!bytes||h1_fseek(sound9588,e->offset+block*JOEY_IMA_BLOCK_BYTES,H1_SEEK_SET)<0||h1_fread(s->encoded,1,bytes,sound9588)!=bytes){s->asset=JOEY_AUDIO_NONE;++joey_audio_diagnostic[5];return 0;}
            s->encoded_first=block;s->encoded_count=blocks;++joey_audio_diagnostic[4];
        }
        if(joey_ima_decode_block(s->encoded+(block-s->encoded_first)*JOEY_IMA_BLOCK_BYTES,JOEY_IMA_BLOCK_BYTES,s->pcm,JOEY_IMA_BLOCK_FRAMES)!=JOEY_IMA_BLOCK_FRAMES){s->asset=JOEY_AUDIO_NONE;++joey_audio_diagnostic[5];return 0;}
        s->decoded_block=block;
    }
    return s->pcm[at];
}
static int stream_next9588(Stream9588 *s){
    if(s->asset>=JOEY_AUDIO_ASSET_COUNT)return 0;
    const AudioEntry9588 *e=&audio_entries[s->asset];
    if(s->frame>=e->frames){if(!e->loop){s->asset=JOEY_AUDIO_NONE;return 0;}s->frame%=e->frames;}
    int first=stream_sample9588(s,s->frame);if(s->asset==JOEY_AUDIO_NONE)return 0;
    int next=first;
    if(s->fraction){
        if(s->frame+1<e->frames)next=stream_sample9588(s,s->frame+1);
        else if(e->loop)next=stream_sample9588(s,0);
    }
    int sample=first+(next-first)*(int)s->fraction/(int)OUTPUT_RATE9588;
    sample=sample*(int)e->gain/256/10;
    /* 32000 / 22050 = 1 + 9950 / 22050. Keep phase across every IMA block. */
    ++s->frame;s->fraction+=32000-OUTPUT_RATE9588;
    if(s->fraction>=OUTPUT_RATE9588){s->fraction-=OUTPUT_RATE9588;++s->frame;}
    return sample;
}
static void diagnostic9588(void){
    joey_audio_diagnostic[6]=music_wanted9588;joey_audio_diagnostic[7]=music9588.frame;
    joey_audio_diagnostic[8]=pending_bytes9588;joey_audio_diagnostic[9]=OUTPUT_FRAMES9588;
    joey_audio_diagnostic[10]=sound_open9588;
}
int joey_audio_open(void){
    if(sound_open9588||sound9588)joey_audio_close();
    uint32_t h[8];sound9588=JOEY_OPEN_RESOURCE("H1SND.PAK",0);
    if(!sound9588)return 0;
    if(h1_fread(h,4,8,sound9588)!=8||h[0]!=0x59454f4a||h[1]!=0x31444e53||h[2]!=1||h[3]!=JOEY_AUDIO_ASSET_COUNT||h[4]!=32000||h[5]!=JOEY_IMA_BLOCK_BYTES||h1_fread(audio_entries,32,JOEY_AUDIO_ASSET_COUNT,sound9588)!=JOEY_AUDIO_ASSET_COUNT)goto invalid;
    unsigned size=h1_fseek(sound9588,0,H1_SEEK_END);
    for(unsigned i=0;i<JOEY_AUDIO_ASSET_COUNT;++i){AudioEntry9588 *e=&audio_entries[i];if(e->rate!=32000||e->offset<h[7]||e->offset>size||e->bytes>size-e->offset||!e->frames||e->gain>128||e->loop>1||e->bytes%JOEY_IMA_BLOCK_BYTES||e->frames>e->bytes/JOEY_IMA_BLOCK_BYTES*JOEY_IMA_BLOCK_FRAMES)goto invalid;}
    stream_reset9588(&music9588,JOEY_AUDIO_NONE);
    for(unsigned i=0;i<EFFECT_SLOTS9588;++i){stream_reset9588(&effects9588[i],JOEY_AUDIO_NONE);effect_stamp9588[i]=0;}
    music_wanted9588=JOEY_AUDIO_NONE;music_last9588=music_from9588=0;music_fade9588=0;
    effect_clock9588=pending_at9588=pending_bytes9588=pumping9588=0;
    for(unsigned i=0;i<32;++i)joey_audio_diagnostic[i]=0;
    bda_audio_open_pcm(OUTPUT_RATE9588,16,1);sound_open9588=1;diagnostic9588();return 1;
invalid:
    h1_fclose(sound9588);sound9588=0;return 0;
}
void joey_audio_music(unsigned id){
    if(!sound_open9588||id>=JOEY_AUDIO_MUSIC_COUNT||id==music_wanted9588)return;
    music_wanted9588=id;music_from9588=music_last9588;music_fade9588=256;
    stream_reset9588(&music9588,id);
}
void joey_audio_effect(unsigned id){
    if(!sound_open9588||id<JOEY_AUDIO_MUSIC_COUNT||id>=JOEY_AUDIO_ASSET_COUNT)return;
    unsigned slot=0;
    for(unsigned i=0;i<EFFECT_SLOTS9588;++i){if(effects9588[i].asset==id){slot=i;break;}if(effects9588[i].asset==JOEY_AUDIO_NONE||effect_stamp9588[i]<effect_stamp9588[slot])slot=i;}
    stream_reset9588(&effects9588[slot],id);effect_stamp9588[slot]=++effect_clock9588;
}
void joey_audio_tick(void){
    if(!sound_open9588||pumping9588)return;
    pumping9588=1;
    /* Fill each currently free firmware slot, rather than one 23 ms block.
     * The firmware copies submitted PCM into its own queue. Keep a short write
     * unchanged for retry; never advance the mixer a second time for it. */
    for(unsigned work=0;work<PUMP_LIMIT9588&&bda_audio_ready();++work){
        if(!pending_bytes9588){
            for(unsigned i=0;i<OUTPUT_FRAMES9588;++i){
                int sample=stream_next9588(&music9588);
                if(music_fade9588){sample=(music_from9588*(int)music_fade9588+sample*(int)(256-music_fade9588))/256;--music_fade9588;}
                music_last9588=sample;
                for(unsigned j=0;j<EFFECT_SLOTS9588;++j)sample+=stream_next9588(&effects9588[j]);
                output9588[i]=(int16_t)(sample>32767?32767:sample<-32768?-32768:sample);
            }
            pending_at9588=0;pending_bytes9588=sizeof(output9588);++joey_audio_diagnostic[11];
        }
        uint8_t *source=(uint8_t *)output9588+pending_at9588;
        uint8_t *submission=(uint8_t *)submit9588;
        for(unsigned i=0;i<pending_bytes9588;++i)submission[i]=source[i];
        int n=bda_audio_write(submit9588,pending_bytes9588);
        if(n<=0||(unsigned)n>pending_bytes9588||(n&1)){++joey_audio_diagnostic[2];break;}
        if((unsigned)n<pending_bytes9588)++joey_audio_diagnostic[3];
        pending_at9588+=(unsigned)n;pending_bytes9588-=(unsigned)n;
        joey_audio_diagnostic[1]+=(unsigned)n/2;++joey_audio_diagnostic[0];
    }
    diagnostic9588();pumping9588=0;
}
void joey_audio_close(void){
    if(sound_open9588)bda_audio_stop();sound_open9588=0;
    if(sound9588)h1_fclose(sound9588);sound9588=0;
    pending_at9588=pending_bytes9588=0;diagnostic9588();
}
