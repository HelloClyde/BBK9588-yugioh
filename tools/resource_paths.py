"""One install directory shared by GBK device paths and UTF-8 ZIP names."""
RESOURCE_DIRECTORY = '应用/数据/游戏/YUGIOH'


def device_root(drive):
    return drive + ':\\' + RESOURCE_DIRECTORY.replace('/', '\\') + '\\'


def c_string(value):
    return '"' + ''.join('\\\\' if b == 92 else chr(b) if 32 <= b < 127
                         else f'\\x{b:02x}' for b in value.encode('gbk')) + '"'


def header():
    return '\n'.join([
        '#ifndef JOEY_DUEL_PATHS_H', '#define JOEY_DUEL_PATHS_H',
        '/* Firmware filesystem strings are GBK, not UTF-8. */',
        '#define JOEY_RESOURCE_ROOT_A ' + c_string(device_root('A')),
        '#define JOEY_RESOURCE_ROOT_B ' + c_string(device_root('B')),
        'static inline h1_file *joey_resource_open(const char *a,const char *b,unsigned *drive){',
        '    h1_file *file=h1_fopen(a,"rb");',
        '    if(file){if(drive)*drive=0;return file;}',
        '    file=h1_fopen(b,"rb");if(file&&drive)*drive=1;return file;',
        '}',
        '#define JOEY_OPEN_RESOURCE(name,drive) joey_resource_open(JOEY_RESOURCE_ROOT_A name,JOEY_RESOURCE_ROOT_B name,drive)',
        '#endif', '',
    ])
