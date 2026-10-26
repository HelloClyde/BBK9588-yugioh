"""Package validated local resources with an already-published BDA; no rebuild."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import zipfile

from check_release import ROOT, check
from resource_paths import RESOURCE_DIRECTORY, device_root


def sha(data):
    return hashlib.sha256(data).hexdigest()


def json_bytes(value):
    return (json.dumps(value, ensure_ascii=False, indent=2) + '\n').encode('utf8')


def write_zip(path, files):
    with zipfile.ZipFile(path, 'w') as archive:
        for name, data in sorted(files.items()):
            item = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            item.create_system = 3
            item.external_attr = 0o100644 << 16
            archive.writestr(item, data, compress_type=zipfile.ZIP_DEFLATED,
                             compresslevel=9)


def package(program_dir, assets, output):
    if subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no'],
                               cwd=ROOT, text=True).strip():
        raise ValueError('Commit reviewed packaging sources before creating release packages')
    build = check(program_dir)
    version = build['version']
    if build.get('resource_directory') != RESOURCE_DIRECTORY:
        raise ValueError('The BDA uses a different resource directory')
    evidence = json.loads((ROOT / 'docs/verification/public-build.json').read_text(encoding='utf8'))
    if not evidence['passed'] or evidence['bda_sha256'] != build['bda_sha256']:
        raise ValueError('The published BDA lacks matching firmware verification')
    source = json.loads((ROOT / 'upstream/assets/reference-manifest.json').read_text(encoding='utf8'))['files']
    payload = {}
    # Read exactly the five declared files. Never include saves, source images or
    # arbitrary neighbouring files from the user's resource directory.
    for name, expected in source.items():
        data = (assets / name).read_bytes()
        if len(data) != expected['bytes'] or sha(data) != expected['sha256']:
            raise ValueError('Incompatible resource: ' + name)
        payload[RESOURCE_DIRECTORY + '/' + name] = data
    names = {
        'full': f'BBK9588-yugioh-v{version}-full.zip',
        'resources': f'BBK9588-yugioh-v{version}-resources.zip',
    }
    packaging_commit = subprocess.check_output(
        ['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    manifest = dict(
        version=version, program_source_commit=build['source_commit'],
        packaging_source_commit=packaging_commit,
        bda_sha256=build['bda_sha256'], sdk_commit=build['sdk_commit'],
        upstream_commit=build['upstream_commit'],
        resource_source_release='https://github.com/HelloClyde/BBKH1-yugioh/releases/tag/v0.1.1',
        resources=source, archives=names,
        resource_directory=RESOURCE_DIRECTORY,
        resource_roots=[device_root(drive) for drive in ('A', 'B')],
        third_party_resources_under_apache_2_0=False,
        physical_9588_tested=False,
    )
    payload[RESOURCE_DIRECTORY+'/resources-manifest.json'] = json_bytes(source)
    payload[RESOURCE_DIRECTORY+'/资源说明.txt'] = (ROOT / 'assets/RESOURCE_RELEASE_README.txt').read_bytes()
    payload['RESOURCE_BUILD.json'] = json_bytes(manifest)
    for name in ('LICENSE', 'NOTICE', 'SOURCE_NOTICE.md'):
        payload[name] = (ROOT / name).read_bytes()
    full = dict(payload)
    full['应用/程序/游戏王.bda'] = (program_dir / 'Yugioh9588.bda').read_bytes()
    full['BUILD.json'] = (program_dir / 'BUILD.json').read_bytes()
    full['INSTALL.md'] = (ROOT / 'docs/INSTALL-FULL.md').read_bytes()
    resources = dict(payload)
    resources['INSTALL.md'] = (ROOT / 'docs/INSTALL-RESOURCES.md').read_bytes()
    outputs = {names['full']: full, names['resources']: resources}
    output.mkdir(parents=True, exist_ok=True)
    for name in (*outputs, 'RESOURCE_BUILD.json', 'RESOURCES-SHA256SUMS.txt'):
        if (output / name).exists():
            raise FileExistsError('Refusing to replace an existing package: ' + str(output / name))
    for name, files in outputs.items():
        write_zip(output / name, files)
        with zipfile.ZipFile(output / name) as archive:
            if len(archive.infolist()) != len(files) or set(archive.namelist()) != set(files):
                raise ValueError('Unexpected ZIP contents: ' + name)
            for member, data in files.items():
                if archive.read(member) != data:
                    raise ValueError('ZIP data mismatch: ' + member)
    (output / 'RESOURCE_BUILD.json').write_bytes(json_bytes(manifest))
    sums = ''.join(sha((output / name).read_bytes()) + '  ' + name + '\n'
                   for name in (*outputs, 'RESOURCE_BUILD.json'))
    (output / 'RESOURCES-SHA256SUMS.txt').write_bytes(sums.encode('utf8'))
    return dict(passed=True, **manifest, assets=[
        dict(name=name, bytes=(output / name).stat().st_size,
             sha256=sha((output / name).read_bytes()))
        for name in (*outputs, 'RESOURCE_BUILD.json', 'RESOURCES-SHA256SUMS.txt')
    ])


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--program-dir', type=Path, required=True,
                    help='Downloaded and verified four CI release assets')
    ap.add_argument('--assets', type=Path, required=True,
                    help='Local directory containing the five reference BIN/PAK files')
    ap.add_argument('--output-dir', type=Path, required=True)
    args = ap.parse_args()
    report = package(args.program_dir.resolve(), args.assets.resolve(), args.output_dir.resolve())
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
