/* Execute the production mixer with a bounded copying FIFO and real ADPCM. */
#define JOEY_AUDIO_HOST_TEST
#include "../src/audio9588.c"
#include <assert.h>
#include <stdlib.h>
static unsigned quota,max_write,fail_write,opened,stopped;
static uint8_t captured[3*4096*4];
static unsigned captured_count;
void bda_audio_open_pcm(unsigned r,unsigned b,unsigned c){assert(r==22050&&b==16&&c==1);++opened;}
int bda_audio_ready(void){return quota!=0;}
int bda_audio_write(const void *p,unsigned bytes){
    unsigned offered=bytes;
    assert(quota);--quota;if(fail_write){fail_write=0;memset((void *)p,0,offered);return -1;}
    if(max_write&&bytes>max_write)bytes=max_write;
    assert(captured_count+bytes<=sizeof(captured));memcpy(captured+captured_count,p,bytes);captured_count+=bytes;
    memset((void *)p,0,offered); /* Model the firmware's destructive processing. */
    return bytes;
}
void bda_audio_stop(void){++stopped;}
static void begin(void){quota=max_write=fail_write=captured_count=0;assert(joey_audio_open());joey_audio_music(JOEY_SOUND_M_MENU);}
int main(void){
    begin();quota=3;joey_audio_tick();assert(!quota&&captured_count==3*4096);
    unsigned samples=captured_count/2;
    assert(music9588.frame==(uint64_t)samples*32000/22050);
    assert(music9588.fraction==(uint64_t)samples*32000%22050);
    assert(joey_audio_diagnostic[4]<10); /* Batched source reads, not per block. */
    unsigned frame=music9588.frame;joey_audio_tick();assert(music9588.frame==frame);
    uint8_t reference[3*4096];memcpy(reference,captured,sizeof(reference));
    joey_audio_close();assert(stopped==1);
    begin();max_write=512;
    for(unsigned i=0;i<24;++i){quota=1;joey_audio_tick();}
    assert(captured_count==sizeof(reference)&&memcmp(captured,reference,sizeof(reference))==0);
    assert(joey_audio_diagnostic[3]>0&&joey_audio_diagnostic[2]==0);
    joey_audio_close();
    begin();quota=1;fail_write=1;joey_audio_tick();
    assert(pending_bytes9588==4096&&captured_count==0&&joey_audio_diagnostic[2]==1);
    frame=music9588.frame;quota=1;joey_audio_tick();
    assert(music9588.frame==frame&&captured_count==4096&&!memcmp(captured,reference,4096));
    /* Preserve both overlapping effect channels. */
    joey_audio_effect(JOEY_SOUND_CURSOR);joey_audio_effect(JOEY_SOUND_DECIDE);
    assert(effects9588[0].asset!=JOEY_AUDIO_NONE&&effects9588[1].asset!=JOEY_AUDIO_NONE);
    assert(effects9588[0].asset!=effects9588[1].asset);
    /* A finite music asset must stay ended when audio_scene requests it again. */
    unsigned id=music_wanted9588,original_loop=audio_entries[id].loop;
    audio_entries[id].loop=0;music9588.frame=audio_entries[id].frames;
    assert(stream_next9588(&music9588)==0&&music9588.asset==JOEY_AUDIO_NONE);
    joey_audio_music(id);assert(music9588.asset==JOEY_AUDIO_NONE);
    audio_entries[id].loop=original_loop;
    joey_audio_close();
    /* Reference PCM comes from the same validated production decoder/resampler. */
    begin();Stream9588 stream;
    for(unsigned id=0;id<JOEY_AUDIO_MUSIC_COUNT;++id){
        stream_reset9588(&stream,id);unsigned count=(uint64_t)audio_entries[id].frames*22050/32000;
        char path[100];snprintf(path,sizeof(path),id?"build/tests/audio-reference-%u.pcm":"build/tests/audio-menu-reference.pcm",id);
        FILE *out=fopen(path,"wb");assert(out);
        for(unsigned i=0;i<count;++i){int16_t value=(int16_t)stream_next9588(&stream);assert(fwrite(&value,2,1,out)==1);}
        fclose(out);
    }
    joey_audio_close();assert(!sound_open9588&&!pending_bytes9588);
    printf("audio: queue prefill, phase continuity, batched reads, partial writes, error retry, two effects, finite-track completion, stop: PASS\n");
    return 0;
}
