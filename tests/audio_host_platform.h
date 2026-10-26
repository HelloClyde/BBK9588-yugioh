#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef FILE h1_file;
#define H1_SEEK_SET SEEK_SET
#define H1_SEEK_END SEEK_END
static FILE *host_audio_resource(const char *name){const char *root=getenv("YUGIOH_ASSETS_DIR");if(!root)root=".local-assets";char path[1024];snprintf(path,sizeof(path),"%s/%s",root,name);return fopen(path,"rb");}
#define JOEY_OPEN_RESOURCE(name,drive) host_audio_resource(name)
#define h1_fread fread
#define h1_fclose fclose
static int h1_fseek(FILE *f,int offset,int origin){return fseek(f,offset,origin)?-1:(int)ftell(f);}
void bda_audio_open_pcm(unsigned,unsigned,unsigned);
int bda_audio_ready(void);
int bda_audio_write(const void *,unsigned);
void bda_audio_stop(void);
