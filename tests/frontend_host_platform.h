/* Host-only IO for exercising the production portrait frontend. */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef FILE h1_file;
#define H1_SEEK_SET SEEK_SET
#define H1_SEEK_END SEEK_END
#define H1_KEY_UP 1
#define H1_KEY_DOWN 2
#define H1_KEY_LEFT 3
#define H1_KEY_RIGHT 4
#define H1_KEY_SHIFT 5
#define h1_memcpy memcpy
#define h1_memset memset
#define h1_alloc malloc
#define h1_free free
static h1_file *h1_fopen(const char *path,const char *mode){
    /* Tests may read original assets, but cannot create or overwrite saves. */
    if(strcmp(mode,"rb"))return 0;
    const char *name=strrchr(path,'\\');name=name?name+1:path;
    const char *root=getenv("YUGIOH_ASSETS_DIR");if(!root)root=".local-assets";
    char local[1024];snprintf(local,sizeof(local),"%s/%s",root,name);
    return fopen(local,mode);
}
#define h1_fread fread
static unsigned h1_fwrite(const void *p,unsigned size,unsigned count,h1_file *f){(void)p;(void)size;(void)count;(void)f;assert(!"Host UI test attempted a save");return 0;}
static int h1_fseek(h1_file *f,int offset,int origin){return fseek(f,offset,origin)?-1:(int)ftell(f);}
#define h1_fclose fclose
static uint32_t h1_timer_read_ms(void){return 10000;}
static uint32_t h1_raw_tick_80hz(void){return 800;}
