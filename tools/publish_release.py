"""Publish already-built assets after a tag workflow; never overwrites a release."""
from pathlib import Path
import argparse, json, subprocess
from check_release import check, ROOT
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--repo',required=True);ap.add_argument('--tag',required=True);ap.add_argument('--directory',type=Path,required=True)
    args=ap.parse_args();report=check(args.directory,args.tag)
    evidence=ROOT/'docs/verification/public-build.json'
    verified=json.loads(evidence.read_text(encoding='utf8'))
    assert verified['passed'] and verified['bda_sha256']==report['bda_sha256'],'No successful firmware evidence for this exact BDA'
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    assert report['source_commit']==head,'Release source commit differs from build'
    notes=ROOT/'docs/releases'/f'{args.tag}.md'
    if not notes.is_file():raise FileNotFoundError('Write reviewed release notes first: '+str(notes))
    existing=subprocess.run(['gh','release','view',args.tag,'--repo',args.repo],capture_output=True,text=True)
    if existing.returncode==0:raise RuntimeError('Release already exists; refusing to overwrite it')
    # Distinguish a missing release from authentication/network errors.
    if 'not found' not in (existing.stdout+existing.stderr).lower():
        raise RuntimeError('Cannot establish release state: '+existing.stderr)
    command=['gh','release','create',args.tag,'--repo',args.repo,'--verify-tag','--title',f'BBK9588-yugioh {args.tag}','--notes-file',str(notes)]
    if '-' in args.tag:command.append('--prerelease')
    command += [str(args.directory/name) for name in ('Yugioh9588.bda','BBK9588-yugioh.zip','BUILD.json','SHA256SUMS.txt')]
    subprocess.run(command,check=True)
    subprocess.run(['gh','release','view',args.tag,'--repo',args.repo,'--json','tagName,targetCommitish,isPrerelease,url,assets'],check=True)
if __name__=='__main__':main()
