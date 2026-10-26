"""Source-only host regressions; optional local assets enable rendering/audio."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from prepare import prepare

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cc',default=os.environ.get('HOST_CC','gcc'))
    ap.add_argument('--assets',type=Path)
    args=ap.parse_args()
    compiler=shutil.which(args.cc)
    if not compiler:raise FileNotFoundError('Set --cc or HOST_CC to a host GCC executable')
    cc=Path(compiler).resolve();env=os.environ.copy();env['PATH']=str(cc.parent)+os.pathsep+env.get('PATH','')
    out=ROOT/'build/tests';out.mkdir(parents=True,exist_ok=True)
    if args.assets:
        env['YUGIOH_ASSETS_DIR']=str(args.assets.resolve())
        env['YUGIOH_TEST_OUT']=str(out)
    def run(cmd,**kw):return subprocess.run([str(x) for x in cmd],check=True,cwd=ROOT,env=env,**kw)
    run([sys.executable,ROOT/'upstream/tools/test_host.py','--cc',cc,'--out',out/'rules'])
    prepare(ROOT/'build')
    paths_executable=out/('resource-paths.exe' if os.name=='nt' else 'resource-paths')
    run([cc,'-std=c11','-I',ROOT/'build',ROOT/'tests/test_resource_paths.c','-o',paths_executable])
    paths_report=json.loads(run([paths_executable],capture_output=True,text=True).stdout)
    (out/'resource-paths.json').write_text(json.dumps(paths_report,indent=2)+'\n',encoding='utf8')
    source=(ROOT/'build/duel9588.c').read_text(encoding='utf8')
    source=source.replace('#include "platform9588.h"','#include "frontend_host_platform.h"').replace('#include "entry9588.inc"','#include "test_selection.inc"')
    host=ROOT/'build/selection_host.c';host.write_text(source,encoding='utf8')
    executable=out/('selection.exe' if os.name=='nt' else 'selection')
    cmd=[cc,'-std=c11','-O1','-ffunction-sections','-fdata-sections',host]
    if args.assets:cmd+=['-DJOEY_UI_WITH_ASSETS']
    for folder in (ROOT/'tests',ROOT/'build',ROOT/'src',ROOT/'upstream/src',ROOT/'upstream/data'):cmd+=['-I',folder]
    cmd += [ROOT/'upstream/src'/name for name in ('duel.c','duel_profile.c','duel_opening.c')]
    cmd += ['-Wl,--gc-sections','-o',executable]
    run(cmd)
    result=run([executable],capture_output=True,text=True)
    report=json.loads(result.stdout)
    report['ui_source_sha256']=hashlib.sha256((ROOT/'src/ui9588.inc').read_bytes()).hexdigest()
    report['uses_local_resources']=bool(args.assets)
    report['resource_paths']=paths_report
    if args.assets:
        audio=out/('audio.exe' if os.name=='nt' else 'audio')
        run([cc,'-std=c11','-O2','-I',ROOT/'tests','-I',ROOT/'upstream/src','-I',ROOT/'upstream/data',ROOT/'tests/test_audio.c',ROOT/'upstream/src/duel_ima.c','-o',audio])
        run([audio])
        from PIL import Image
        for path in out.glob('*.ppm'):Image.open(path).save(path.with_suffix('.png'))
        report['production_audio_tests_passed']=True
    (out/'selection.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps(report,indent=2))
if __name__=='__main__':main()
