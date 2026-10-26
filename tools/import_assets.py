"""Validate and import local reference resources; never downloads resources."""
from pathlib import Path
import argparse, hashlib, json, zipfile
ROOT=Path(__file__).resolve().parents[1]
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    group=ap.add_mutually_exclusive_group(required=True)
    group.add_argument('--directory',type=Path)
    group.add_argument('--archive',type=Path)
    args=ap.parse_args()
    files=json.loads((ROOT/'upstream/assets/reference-manifest.json').read_text(encoding='utf8'))['files']
    payload={}
    if args.directory:
        for name in files:payload[name]=(args.directory/name).read_bytes()
    else:
        with zipfile.ZipFile(args.archive) as archive:
            for name in files:
                matches=[entry for entry in archive.infolist() if Path(entry.filename.replace('\\','/')).name==name and not entry.is_dir()]
                if len(matches)!=1:raise ValueError(f'Expected one {name}, found {len(matches)}')
                if matches[0].file_size!=files[name]['bytes']:raise ValueError(f'Wrong size: {name}')
                payload[name]=archive.read(matches[0])
    for name,data in payload.items():
        expected=files[name]
        if len(data)!=expected['bytes'] or hashlib.sha256(data).hexdigest()!=expected['sha256']:
            raise ValueError('Incompatible resource: '+name)
    target=ROOT/'.local-assets';target.mkdir(exist_ok=True)
    for name,data in payload.items():(target/name).write_bytes(data)
    print('Validated local resources; excluded from Git and public build output.')
if __name__=='__main__':main()
