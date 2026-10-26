/* Exercise the actual generated firmware path helper without a device. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct { unsigned drive; } h1_file;
static h1_file handles[2]={{0},{1}};
static unsigned available,calls,checks;
static char requested[2][256];
static h1_file *h1_fopen(const char *path,const char *mode){
    assert(!strcmp(mode,"rb")&&calls<2);
    snprintf(requested[calls++],sizeof(requested[0]),"%s",path);
    unsigned drive=path[0]=='B';
    return available&(1u<<drive)?&handles[drive]:0;
}
#include "duel_paths.h"
#define CHECK(x) do{assert(x);++checks;}while(0)
int main(void){
    const char *a="A:\\\xd3\xa6\xd3\xc3\\\xca\xfd\xbe\xdd\\\xd3\xce\xcf\xb7\\YUGIOH\\";
    const char *b="B:\\\xd3\xa6\xd3\xc3\\\xca\xfd\xbe\xdd\\\xd3\xce\xcf\xb7\\YUGIOH\\";
    CHECK(!strcmp(JOEY_RESOURCE_ROOT_A,a));CHECK(!strcmp(JOEY_RESOURCE_ROOT_B,b));
    const char *names[]={"FONT.BIN","H1UI.PAK","H1ART.PAK","H1SCN.PAK","H1SND.PAK"};
    for(unsigned mask=0;mask<4;++mask)for(unsigned i=0;i<5;++i){
        char pa[256],pb[256];snprintf(pa,sizeof(pa),"%s%s",JOEY_RESOURCE_ROOT_A,names[i]);
        snprintf(pb,sizeof(pb),"%s%s",JOEY_RESOURCE_ROOT_B,names[i]);
        available=mask;calls=0;unsigned drive=99;
        h1_file *f=joey_resource_open(pa,pb,&drive);
        char expected[256];snprintf(expected,sizeof(expected),"%s%s",a,names[i]);
        CHECK(!strcmp(requested[0],expected));
        CHECK(calls==(mask&1?1:2));
        if(!(mask&1)){snprintf(expected,sizeof(expected),"%s%s",b,names[i]);CHECK(!strcmp(requested[1],expected));}
        CHECK(f==(mask&1?&handles[0]:mask&2?&handles[1]:0));
        CHECK(drive==(mask&1?0:mask&2?1:99));
    }
    available=2;calls=0;CHECK(JOEY_OPEN_RESOURCE("H1UI.PAK",0)==&handles[1]);
    printf("{\"passed\":true,\"checks\":%u,\"gbk_paths\":true,\"a_preferred\":true,\"b_fallback\":true,\"resources\":5}\n",checks);
    return 0;
}
