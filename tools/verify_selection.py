"""Reach a real search choice via normal play, then inspect before choosing.

This script does not write guest memory or alter game rules. It summons Sangan
or Witch from a normally shuffled hand, and lets the opponent destroy it.
"""
import json,struct,time
from pathlib import Path
import live
import verify_firmware as v
ROOT=Path(__file__).resolve().parents[1]
def scalar(name):return v.u32(name)
def opening():return live.diagnostics()['joey_opening_diagnostic']
def title_from_duel():
    live.key(9);live.touch(120,245);assert v.diag()['view']==2
def run(leave_open=False):
    assert v.diag()['view']==2
    program=v.verify_program()
    attempts=0
    for attempts in range(1,25):
        live.touch(120,150)
        for _ in range(24):
            if v.diag()['view']!=10:break
            p=opening()[0]
            if p==0:live.touch(120,110)
            elif p==2:live.touch(65,281)
            else:live.touch(120,281)
        v.wait_idle()
        hand,_=v.state();indices=[i for i,c in enumerate(hand) if c in (248,261)]
        if indices:break
        title_from_duel()
    assert indices,'No search monster drawn'
    index=indices[0];source=hand[index]
    live.touch(31+index*35,251);live.touch(31+index*35,251)
    if v.diag()['pending']==14:live.touch(120,163)
    _,monsters=v.state();assert source in monsters,(source,v.diag())
    limit=time.monotonic()+150
    while time.monotonic()<limit:
        d=v.diag()
        if d['pending']==7:break
        if d['pending']==8:live.key(9);continue
        assert d['view']!=5,'Duel ended before search'
        if d['active']==0 and d['pending']==0:
            live.touch(220,216)
            if v.diag()['hand']>6:
                for _ in range(v.diag()['hand']-6):live.touch(31,251);live.touch(31,251)
        time.sleep(.35)
    assert v.diag()['pending']==7,'Search choice not reached'
    count=scalar('picker_count');options=[struct.unpack_from('<HBB',v.memory('effect_options',count*4),i*4) for i in range(count)]
    choice=next(i for i in range(1,count) if options[i][0] and options[i][2]==0)
    for _ in range(choice//5):live.touch(200,259)
    live.touch(90,86+(choice%5)*36)
    expected=options[choice][0];before=v.memory('joey_duel_state');commands=v.diag()['commands']
    prefix='selection-preview' if leave_open else 'selection-firmware'
    live.shot(prefix+'-list')
    live.touch(120,281)
    assert v.diag()['view']==4 and v.diag()['selected']==expected
    live.shot(prefix+'-details')
    live.key(5);live.touch(200,20)
    assert v.diag()['view']==3 and v.diag()['pending']==7 and scalar('grave_choice')==choice
    assert v.memory('joey_duel_state')==before and v.diag()['commands']==commands
    live.shot(prefix+'-return')
    live.key(7);assert v.diag()['view']==4 and v.diag()['selected']==expected
    live.key(9);assert scalar('grave_choice')==choice and v.diag()['pending']==7
    paging=False
    if count>5 and options[5][0] and options[5][2]==0:
        live.touch(200,259);live.touch(120,281)
        assert v.diag()['view']==4 and v.diag()['selected']==options[5][0]
        live.shot(prefix+'-page-details');live.key(9)
        live.touch(40,259);live.touch(90,86+(choice%5)*36);paging=True
    skipped=next((i for i,o in enumerate(options) if o[2] in (2,3)),None)
    if skipped is not None:
        while scalar('grave_choice')!=skipped:live.key(5)
        live.touch(120,281);assert v.diag()['view']==3 and v.diag()['pending']==7
        while scalar('grave_choice')!=choice:live.key(4)
    if leave_open:
        live.shot('selection-preview-fixed')
        print('Preview ready at the live search choice',flush=True)
        return {'preview_ready':True,'selected_card':expected}
    hand,_=v.state();copies=hand.count(expected)
    live.touch(190,281)
    hand,_=v.state();assert hand.count(expected)==copies+1,(expected,hand)
    assert v.diag()['commands']==commands+1
    v.wait_idle();title_from_duel()
    previous=ROOT/'qa/selection-result.json'
    if not previous.exists():previous=ROOT/'build/tests/selection.json'
    report=json.loads(previous.read_text(encoding='utf8'))
    report['bda_sha256']=v.REPORT['bda_sha256']
    report['firmware']={'passed':True,'normal_inputs_only':True,'guest_memory_written':False,'attempts':attempts,'source_card':source,'inspected_card':expected,'options':count,'page_details_checked':paging,'selection_retained':True,'game_unchanged_while_inspecting':True,'confirm_added_chosen_card_to_hand':True,'program':program}
    (ROOT/'qa/selection-result.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
    print('PASS real firmware selection details',flush=True)
    return report['firmware']
if __name__=='__main__':run()
