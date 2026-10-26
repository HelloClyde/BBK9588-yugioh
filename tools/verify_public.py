"""Check the exact public-icon BDA in a dedicated firmware emulator.

Only normal input is sent; guest memory is read solely for assertions.
The private fixture's program path can differ from the normal installation.
"""
import argparse, hashlib, json, time
from pathlib import Path
import live
import verify_firmware as v
ROOT=Path(__file__).resolve().parents[1]
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--base',required=True)
    ap.add_argument('--program',default=live.PROGRAM_PATH)
    ap.add_argument('--selection',action='store_true')
    args=ap.parse_args();live.BASE=args.base;live.PROGRAM_PATH=args.program
    build=json.loads((ROOT/'build/build.json').read_text(encoding='utf8'))
    reference=json.loads((ROOT/'docs/verification/reference/load-image.json').read_text(encoding='utf8'))
    report={'passed':False,'bda_sha256':build['bda_sha256'],'sdk_commit':build['sdk_commit'],'upstream_commit':build['upstream_commit'],'load_image_sha256':build['load_image_sha256'],'text_sha256':build['text_sha256'],'reference_bda_sha256':reference['reference_bda_sha256'],'same_load_image_as_reference':build['load_image_sha256']==reference['load_image_sha256'],'guest_memory_written':False,'physical_9588_tested':False,'checks':{}}
    assert report['same_load_image_as_reference']
    assert v.diag()['view']==2
    report['program_at_start']=v.verify_program()
    live.shot('public-menu')
    initial=v.u32('native_menu_focus');assert initial==0
    live.key(5);assert v.u32('native_menu_focus')==1
    live.key(4);assert v.u32('native_menu_focus')==0
    live.req('/api/key',True,code=5,down=1);time.sleep(.8);live.req('/api/key',True,code=5,down=0);time.sleep(.3)
    assert v.u32('native_menu_focus')==1 and v.diag()['view']==2
    live.key(4)
    report['checks']['short_and_long_menu_input']={'passed':True,'long_press_steps':1}
    live.touch(120,180);before=v.diag()['profile'];base=before[10];assert before[4]==3 and base>=40
    live.touch(90,120);assert v.diag()['profile'][7]==1
    live.touch(90,84);assert v.diag()['profile'][7]==0
    remove=base>40;live.touch(145 if remove else 90,305)
    expected=base-1 if remove else base+1;assert v.diag()['profile'][10]==expected
    live.touch(205,305);p=v.diag()['profile'];assert p[0]==before[0]+1 and p[3]==1
    saved=v.save_export(p[1]);assert saved['main_count']==expected
    live.touch(90 if remove else 145,305);live.touch(205,305)
    p=v.diag()['profile'];saved=v.save_export(p[1]);assert saved['main_count']==base
    live.touch(30,305);live.touch(120,180);assert v.diag()['profile'][10]==base
    live.shot('public-deck');live.touch(30,305)
    report['checks']['touch_escape_and_dual_slot_save']={'passed':True,'restored_main_count':base,'sequence':saved['sequence']}
    live.touch(120,288);assert v.u32('about_open')==1
    live.key(9);assert v.u32('about_open')==0 and v.diag()['view']==2
    live.touch(120,150)
    for _ in range(24):
        if v.diag()['view']!=10:break
        phase=live.diagnostics()['joey_opening_diagnostic'][0]
        if phase==0:live.touch(120,110)
        elif phase==2:live.touch(65,281)
        else:live.touch(120,281)
    v.wait_idle();live.key(6);assert v.diag()['focus']==0
    live.key(7);start=v.diag()['focus'];assert start==1
    live.req('/api/key',True,code=7,down=1);time.sleep(.85);live.req('/api/key',True,code=7,down=0);time.sleep(.3)
    d=v.diag();assert d['focus']>=min(start+2,d['hand']-1) and d['focus']<d['hand'],d
    report['checks']['held_direction_repeat']={'passed':True,'start_focus':start,'end_focus':d['focus']}
    live.key(9);assert v.u32('menu9588')==1
    live.touch(120,245);assert v.diag()['view']==2
    if args.selection:
        import verify_selection
        report['checks']['selection_details']=verify_selection.run()
    live.key(9);time.sleep(2);live.shot('public-desktop-after-exit')
    live.touch(120,100);time.sleep(3)
    assert v.diag()['view']==2 and v.diag()['profile'][0]==saved['sequence']
    live.touch(120,180);assert v.diag()['profile'][10]==base
    p=v.diag()['profile'];assert v.save_export(p[1])==saved
    live.touch(30,305)
    report['checks']['normal_exit_relaunch_saved_profile']={'passed':True,'save':saved}
    report['program_after_relaunch']=v.verify_program()
    report['passed']=True
    (ROOT/'build/public-firmware.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps(report,ensure_ascii=False,indent=2))
if __name__=='__main__':main()
