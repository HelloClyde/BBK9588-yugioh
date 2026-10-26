#ifndef JOEY_PLATFORM9588_H
#define JOEY_PLATFORM9588_H
#include <stdint.h>
#include <stddef.h>
#include "bda_sdk.h"
#include "sdk_compat9588.h"
/* Only the source-level file/time helpers are shared with the H1 frontend.
 * Every device operation below uses the verified 9588 SDK. */
typedef void h1_file;
#define H1_SEEK_SET BDA_SEEK_SET
#define H1_SEEK_END BDA_SEEK_END
#define H1_KEY_UP 1
#define H1_KEY_DOWN 2
#define H1_KEY_LEFT 3
#define H1_KEY_RIGHT 4
#define H1_KEY_SHIFT 5
#define h1_memcpy bda_memcpy
#define h1_memset bda_memset
#define h1_alloc bda_alloc
#define h1_free bda_free
static inline h1_file *h1_fopen(const char *path,const char *mode){int f=bda_fs_fopen_raw(path,mode);return bda_fs_file_is_valid(f)?(h1_file *)(uintptr_t)f:0;}
static inline unsigned h1_fread(void *p,unsigned size,unsigned count,h1_file *f){int n=bda_fs_fread_raw(p,size,count,(int)(uintptr_t)f);return n<0?0:(unsigned)n;}
static inline unsigned h1_fwrite(const void *p,unsigned size,unsigned count,h1_file *f){int n=bda_fs_fwrite_raw(p,size,count,(int)(uintptr_t)f);bda_fs_flush_all();return n<0?0:(unsigned)n;}
static inline int h1_fseek(h1_file *f,int offset,int origin){return bda_fs_seek_raw((int)(uintptr_t)f,offset,origin);}
static inline int h1_ftell(h1_file *f){return bda_fs_tell_raw((int)(uintptr_t)f);}
static inline int h1_fclose(h1_file *f){return bda_fs_close_raw((int)(uintptr_t)f);}
static inline uint32_t h1_timer_read_ms(void){return bda_gui_tick_count_25ms()*25u;}
static inline uint32_t h1_raw_tick_80hz(void){return bda_gui_tick_count_25ms()*2u;}
#endif
