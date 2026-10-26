"""Check exact public assets and SHA-256; standard library only."""
from pathlib import Path
import argparse, hashlib, json, re, zipfile
ROOT=Path(__file__).resolve().parents[1]
def check(folder, tag=None):
    report=json.loads((folder/'BUILD.json').read_text(encoding='utf8'))
    if tag and (not re.fullmatch(r'v\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?',tag) or report['version']!=tag[1:]):
        raise ValueError('Release tag and built version differ')
    pins=json.loads((ROOT/'dependencies.json').read_text(encoding='utf8'))
    assert report['public_assets_only'] and report['sdk_commit']==pins['sdk']['commit']
    assert report['upstream_commit']==pins['upstream']['commit']
    assert report['validation']['path']=='Yugioh9588.bda'
    expected={'Yugioh9588.bda','BBK9588-yugioh.zip','BUILD.json'}
    lines=(folder/'SHA256SUMS.txt').read_text().splitlines()
    seen=set()
    for line in lines:
        digest,name=line.split('  ',1);assert name in expected and name not in seen
        assert hashlib.sha256((folder/name).read_bytes()).hexdigest()==digest,name
        seen.add(name)
    assert seen==expected
    program=(folder/'Yugioh9588.bda').read_bytes()
    assert hashlib.sha256(program).hexdigest()==report['bda_sha256']
    resource_directory=report.get('resource_directory','YUGIOH')
    allowed={'应用/程序/游戏王.bda','BUILD.json','INSTALL.md','LICENSE','NOTICE','SOURCE_NOTICE.md',resource_directory+'/需要外部资源.txt',resource_directory+'/resources-manifest.json'}
    with zipfile.ZipFile(folder/'BBK9588-yugioh.zip') as archive:
        assert set(archive.namelist())==allowed
        assert archive.read('应用/程序/游戏王.bda')==program
        assert archive.read('BUILD.json')==(folder/'BUILD.json').read_bytes()
    return report
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--directory',type=Path,default=ROOT/'dist');ap.add_argument('--tag')
    args=ap.parse_args();report=check(args.directory,args.tag)
    print(json.dumps({'passed':True,'version':report['version'],'bda_sha256':report['bda_sha256'],'public_assets_only':True},indent=2))
if __name__=='__main__':main()
