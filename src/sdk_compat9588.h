#ifndef JOEY_SDK_COMPAT9588_H
#define JOEY_SDK_COMPAT9588_H
/* Application-local C200 compatibility extension, Apache-2.0.
 * The pinned public SDK f2fcbe0 does not yet expose FS+0x074. Keep this
 * verified global flush operation here, rather than depending on unpublished
 * SDK workspace edits. It intentionally discards the mixed firmware result.
 * See docs/SDK_COMPATIBILITY.md and the save/reload verification records.
 */
#ifndef BDA_SDK_INTERNAL_FS_FLUSH_ALL
static inline void bda_fs_flush_all(void) {
    typedef int (*fn_t)(void);
    fn_t fn = (fn_t)bda_sdk_internal_api(bda_sdk_internal_fs(), 0x074u);
    (void)fn();
}
#endif
#endif
