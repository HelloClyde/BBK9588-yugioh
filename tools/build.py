"""Build the portrait BDA using only pinned, clean source dependencies.

Public output never contains commercial cards, fonts, music or personal saves.
"""
from pathlib import Path
import argparse, hashlib, json, os, re, shutil, struct, subprocess, sys, zipfile
from .prepare import prepare, ROOT, UP
from .make_icon import make_icon

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def git(folder, *args):
    return subprocess.check_output(['git', '-C', str(folder), *args], text=True, encoding='utf8').strip()

def check_dependency(folder, name, pins):
    commit = git(folder, 'rev-parse', 'HEAD')
    if commit != pins[name]['commit']:
        raise ValueError(f'{name} is {commit}; expected {pins[name]["commit"]}')
    if git(folder, 'status', '--porcelain', '--untracked-files=no'):
        raise ValueError(f'{name} has tracked local changes; use the fixed clean submodule')
    return commit

def toolchain(folder, sdk):
    suffix = '.exe' if os.name == 'nt' else ''
    candidates = [Path(folder).resolve()] if folder else [sdk/'.toolchain/bin', sdk/'.toolchain/g++-mipsel-none-elf-15.2.0/bin']
    for candidate in candidates:
        gcc = candidate/f'mipsel-none-elf-gcc{suffix}'
        objcopy = candidate/f'mipsel-none-elf-objcopy{suffix}'
        if gcc.is_file() and objcopy.is_file():
            version = subprocess.check_output([str(gcc), '--version'], text=True).splitlines()[0]
            if '15.2.0' not in version:
                raise ValueError('Reproducible build requires MIPS GCC 15.2.0: '+version)
            return gcc, objcopy, version
    raise FileNotFoundError('Install sdk/scripts/setup_toolchain.ps1 or pass --toolchain <bin directory>')

def write_zip(target, files):
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(files.items()):
            item = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            item.compress_type = zipfile.ZIP_DEFLATED
            item.create_system = 3
            item.external_attr = 0o100644 << 16
            archive.writestr(item, data)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--sdk', type=Path, default=Path(os.environ.get('BDA_SDK_ROOT', ROOT/'sdk')))
    ap.add_argument('--toolchain', default=os.environ.get('MIPS_GNU_BIN'))
    ap.add_argument('--output-dir', type=Path, default=ROOT/'dist')
    ap.add_argument('--tag', help='On release builds require v<VERSION>')
    args = ap.parse_args()
    pins = json.loads((ROOT/'dependencies.json').read_text(encoding='utf8'))
    sdk = args.sdk.resolve()
    sdk_commit = check_dependency(sdk, 'sdk', pins)
    upstream_commit = check_dependency(UP, 'upstream', pins)
    version = (ROOT/'VERSION').read_text().strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?', version):
        raise ValueError('Invalid VERSION')
    if args.tag and args.tag != 'v'+version:
        raise ValueError(f'Tag {args.tag!r} does not match v{version}')
    # Import exclusively from the selected clean SDK, never a globally installed packer.
    sys.path.insert(0, str(sdk))
    from bda_packer.build import build_icons, ENTRY_OFFSET, ICON_START, ICON_SIZES, ENTRY_VA
    from bda_packer.header import BdaHeaderFields, write_header
    from bda_packer.validate import validate_bda
    from elftools.elf.elffile import ELFFile
    gcc, objcopy, compiler_version = toolchain(args.toolchain, sdk)
    out = ROOT/'build'; prepare(out)
    paths = (UP/'src/duel_paths.h').read_text(encoding='utf8')
    paths = re.sub(r'#define JOEY_RESOURCE_ROOT_A .*', lambda m: '#define JOEY_RESOURCE_ROOT_A "A:\\\\YUGIOH\\\\"', paths)
    paths = re.sub(r'#define JOEY_RESOURCE_ROOT_B .*', lambda m: '#define JOEY_RESOURCE_ROOT_B "B:\\\\YUGIOH\\\\"', paths)
    (out/'duel_paths.h').write_text(paths, encoding='utf8')
    flags = ['-EL','-march=mips32','-mabi=32','-msoft-float','-mno-abicalls','-G0','-fno-pic','-O2','-std=c11','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-strict-aliasing','-ffunction-sections','-fdata-sections']
    includes = [v for p in (out, ROOT/'src', UP/'src', UP/'data', sdk/'sdk/include') for v in ('-I', str(p))]
    sources = [UP/'src'/n for n in ('duel.c','duel_profile.c','duel_opening.c','duel_ima.c','mini.c')]+[out/'duel9588.c', ROOT/'src/audio9588.c']
    objects = []
    def run(cmd):subprocess.run([str(x) for x in cmd], check=True, cwd=ROOT)
    for source in sources:
        obj = out/(source.stem+'.o');run([gcc,*flags,*includes,'-c',source,'-o',obj]);objects.append(obj)
    ld = out/'9588.ld'
    ld.write_text('ENTRY(bda_main)\nSECTIONS { . = 0x81c00020; .text : { *(.text.bda_main) *(.text*) } .rodata : { *(.rodata*) } .data : { *(.data*) *(.sdata*) . = ALIGN(16); __bss_start = .; *(.bss*) *(.sbss*) *(COMMON) __bss_end = .; } }\n')
    elf = out/'Yugioh9588.elf'
    run([gcc,*flags,'-nostdlib','-Wl,--build-id=none,--gc-sections,--undefined=joey_duel_layout,--undefined=joey_profile_layout',f'-Wl,-Map,{out/"Yugioh9588.map"}','-T',ld,*objects,'-lgcc','-o',elf])
    with elf.open('rb') as stream:
        parsed = ELFFile(stream)
        symbols = {s.name:s['st_value'] for s in parsed.get_section_by_name('.symtab').iter_symbols()}
        assert parsed.header['e_entry'] == ENTRY_VA
        assert symbols['__bss_end'] < 0x81e00000, 'Application memory window exceeded'
        text_sha = hashlib.sha256(parsed.get_section_by_name('.text').data()).hexdigest()
        for section in parsed.iter_sections():
            if section['sh_flags'] & 4:
                for (word,) in struct.iter_unpack('<I', section.data()):
                    assert word >> 26 not in (0x11,0x13,0x31,0x35,0x39,0x3d), 'Hardware floating point linked'
    raw = out/'Yugioh9588.bin';run([objcopy,'-O','binary',elf,raw])
    icon = out/'open-icon.png';make_icon(icon)
    data = bytearray(ENTRY_OFFSET)
    data[ICON_START:ENTRY_OFFSET] = build_icons(icon, (0,0,0))
    data.extend(raw.read_bytes());data.extend(bytes(-len(data)%4))
    fields = BdaHeaderFields(category=4,file_size_minus_4=len(data)-4,entry_offset=ENTRY_OFFSET,icon_start=ICON_START,icon0_size=ICON_SIZES[0],icon1_size=ICON_SIZES[1],icon2_size=ICON_SIZES[2],icon3_size=ICON_SIZES[3])
    write_header(data, fields, '游戏王')
    dist = args.output_dir.resolve();dist.mkdir(parents=True,exist_ok=True)
    program = dist/'Yugioh9588.bda';program.write_bytes(data)
    validation = validate_bda(program);assert validation['ok'], validation
    validation['path']=program.name
    reference = json.loads((UP/'assets/reference-manifest.json').read_text(encoding='utf8'))['files']
    inputs=[*sorted((ROOT/'src').glob('*')),ROOT/'tools/prepare.py',ROOT/'tools/build.py',ROOT/'tools/make_icon.py',ROOT/'dependencies.json',ROOT/'VERSION']
    fingerprint=hashlib.sha256(''.join(p.relative_to(ROOT).as_posix()+':'+sha(p)+'\n' for p in inputs if p.is_file()).encode()).hexdigest()
    report = dict(passed=True,version=version,resolution=[240,320],entry=hex(ENTRY_VA),bss_end=hex(symbols['__bss_end']),bda_bytes=len(data),bda_sha256=sha(program),load_image_sha256=sha(raw),text_sha256=text_sha,source_commit=git(ROOT,'rev-parse','HEAD') if (ROOT/'.git').exists() else None,source_tree_sha256=fingerprint,sdk_commit=sdk_commit,upstream_commit=upstream_commit,compiler=compiler_version,public_assets_only=True,icon='original geometric crossed cards; Apache-2.0',physical_9588_tested=False,ci_emulator_tested=False,required_external_resources=reference,validation=validation)
    manifest = (json.dumps(report, ensure_ascii=False, indent=2)+'\n').encode('utf8')
    (dist/'BUILD.json').write_bytes(manifest)
    (out/'build.json').write_bytes(manifest)
    files = {'应用/程序/游戏王.bda':bytes(data),'BUILD.json':manifest}
    for name in ('INSTALL.md','LICENSE','NOTICE','SOURCE_NOTICE.md'):
        files[name]=(ROOT/name).read_bytes()
    files['YUGIOH/需要外部资源.txt']=(ROOT/'assets/RESOURCE_README.txt').read_bytes()
    files['YUGIOH/resources-manifest.json']=(json.dumps(reference,indent=2)+'\n').encode('utf8')
    write_zip(dist/'BBK9588-yugioh.zip', files)
    names = ('Yugioh9588.bda','BBK9588-yugioh.zip','BUILD.json')
    (dist/'SHA256SUMS.txt').write_text(''.join(sha(dist/name)+'  '+name+'\n' for name in names),encoding='utf8')
    print(json.dumps({k:report[k] for k in ('version','bda_sha256','load_image_sha256','text_sha256','sdk_commit','bda_bytes')},indent=2))

if __name__ == '__main__':main()
