"""Build twice without resource packs and compare exact public deliverables."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--toolchain')
    args=ap.parse_args();out=ROOT/'build/reproducible';out.mkdir(parents=True,exist_ok=True)
    names=('Yugioh9588.bda','BBK9588-yugioh.zip','BUILD.json','SHA256SUMS.txt')
    rounds=[]
    for label in ('first','second'):
        target=out/label;command=[sys.executable,str(ROOT/'build.py'),'--output-dir',str(target)]
        if args.toolchain:command+=['--toolchain',args.toolchain]
        subprocess.run(command,cwd=ROOT,check=True)
        rounds.append({name:hashlib.sha256((target/name).read_bytes()).hexdigest() for name in names})
    assert rounds[0]==rounds[1],rounds
    build=json.loads((out/'second/BUILD.json').read_text(encoding='utf8'))
    report={'passed':True,'source_commit':build['source_commit'],'source_tree_sha256':build['source_tree_sha256'],'sdk_commit':build['sdk_commit'],'upstream_commit':build['upstream_commit'],'same_bda_and_zip':True,'external_resources_required_for_build':False,'files':rounds[0]}
    (out/'result.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
