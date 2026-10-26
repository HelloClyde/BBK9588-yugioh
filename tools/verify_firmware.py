"""Exercise the final BDA through physical input in a private firmware NAND.

Guest memory is read only for assertions. No state writes, guest function calls,
save injection or patched rules are used. Start this script at the title menu.
"""
import hashlib,io,json,struct,time,zlib
from pathlib import Path
from elftools.elf.elffile import ELFFile
import live
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'qa'
OUT.mkdir(exist_ok=True)
elf=ELFFile(io.BytesIO((ROOT/'build/Yugioh9588.elf').read_bytes()))
SYMS={s.name:s for s in elf.get_section_by_name('.symtab').iter_symbols() if s.name}
def static(name):
    s=SYMS[name];sec=elf.get_section(s['st_shndx']);at=s['st_value']-sec['sh_addr'];return sec.data()[at:at+s['st_size']]
LAYOUT=list(struct.unpack('<'+'I'*(len(static('joey_duel_layout'))//4),static('joey_duel_layout')))
DEFINITIONS=[struct.unpack_from('<IHH6BH',static('duel_card_definitions'),i*16) for i in range(1115)]
def memory(name,size=None):
    s=SYMS[name];return live.req('/debug/mem.bin',addr=hex(s['st_value']),size=size or s['st_size']).content
def diag():
    d=list(struct.unpack('<24I',memory('joey_native_diagnostic')))
    p=list(struct.unpack('<12I',memory('joey_profile_diagnostic')))
    return dict(view=d[1],commands=d[4],result=d[5],life=d[6:8],turn=d[8],active=d[9],phase=d[10],hand=d[11],selected=d[12],focus=d[13],pending=d[14],revision=d[15],response=d[22],profile=p)
def u32(name):return struct.unpack('<I',memory(name,4))[0]
def wait_saved(sequence,timeout=15):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        p=diag()['profile']
        if p[0]==sequence and p[3]==1:return p
        if p[3]==2:raise AssertionError(('Profile write failed',p))
        time.sleep(.15)
    raise AssertionError(('Profile save did not finish',sequence,diag()))
def state():
    raw=memory('joey_duel_state');p=LAYOUT[2];n=raw[p+LAYOUT[12]]
    hand=list(struct.unpack_from('<'+'H'*n,raw,p+LAYOUT[9]))
    monsters=[struct.unpack_from('<H',raw,p+LAYOUT[14]+i*LAYOUT[16])[0] for i in range(5)]
    return hand,monsters
REPORT={'passed':False,'bda_sha256':hashlib.sha256((ROOT/'dist/Yugioh9588.bda').read_bytes()).hexdigest(),'guest_memory_written':False,'original_rules_patched':False,'checks':{},'timeline':[]}
def verify_program():
    installed=hashlib.sha256(live.req('/api/files/export',path=live.PROGRAM_PATH).content).hexdigest()
    assert installed==REPORT['bda_sha256'],('Installed BDA changed',installed,REPORT['bda_sha256'])
    text=elf.get_section_by_name('.text');loaded=live.req('/debug/mem.bin',addr=hex(text['sh_addr']),size=text['sh_size']).content
    expected=hashlib.sha256(text.data()).hexdigest();actual=hashlib.sha256(loaded).hexdigest()
    assert actual==expected,('Loaded code differs from build',actual,expected)
    return {'installed_bda_sha256':installed,'loaded_text_sha256':actual,'text_bytes':len(loaded)}
def record(name,**data):
    REPORT['checks'][name]=data or {'passed':True};REPORT['timeline'].append({'name':name,'state':diag()});print(name,flush=True)
    (OUT/'firmware-result.json').write_text(json.dumps(REPORT,ensure_ascii=False,indent=2),encoding='utf8')
def wait_idle(timeout=45):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        d=diag()
        if d['pending']==8:live.key(9);continue
        if d['active']==0 and d['pending']==0 and d['response']==255:return d
        if d['view']==5:return d
        time.sleep(.35)
    raise AssertionError(('AI did not settle',diag()))
def save_export(slot):
    data=live.req('/api/files/export',path=f'/YUGIOH/JNAT{slot}.SAV').content
    assert len(data)==1377,len(data)
    buf=bytearray(data);expected=struct.unpack_from('<I',buf,20)[0];buf[20:24]=bytes(4);assert zlib.crc32(buf)==expected
    (OUT/f'firmware-JNAT{slot}.SAV').write_bytes(data)
    return {'sequence':struct.unpack_from('<I',data,12)[0],'main_count':struct.unpack_from('<H',data,24)[0],'wins':struct.unpack_from('<I',data,1333)[0],'losses':struct.unpack_from('<I',data,1337)[0],'sha256':hashlib.sha256(data).hexdigest()}
def main():
    assert diag()['profile'][4]==0
    REPORT['program_at_start']=verify_program()
    live.shot('menu')
    live.touch(120,180);assert diag()['profile'][4]==3
    before=diag()['profile'];base=before[10];assert base>=40
    # Add/remove a legal copy and save; return to the title and reload it.
    remove=base>40
    live.touch(145 if remove else 90,305);expected=base-1 if remove else base+1
    assert diag()['profile'][10]==expected
    live.touch(205,305);p=wait_saved(before[0]+1)
    exported=save_export(p[1]);assert exported['main_count']==expected
    live.touch(30,305);live.touch(120,180);assert diag()['profile'][10]==expected
    live.shot('deck');record('deck_save_and_reload',save=exported)
    # Restore the deck count, and verify the other journal slot is valid.
    sequence=p[0]+1;live.touch(90 if remove else 145,305);live.touch(205,305);p=wait_saved(sequence);assert p[10]==base
    restored=save_export(p[1]);assert restored['main_count']==base;record('two_slot_profile',save=restored)
    REPORT['program_after_profile_save']=verify_program()
    # Find a real owned card with a long effect and scroll its actual text.
    live.touch(206,50);assert diag()['profile'][5]==3
    count=diag()['profile'][6];cards=list(struct.unpack('<'+'H'*count,memory('editor_cards',count*2)))
    resource=(ROOT/'.local-assets/H1UI.PAK').read_bytes();h=struct.unpack_from('<18I',resource)
    index=next((i for i,c in enumerate(cards) if struct.unpack_from('<H',resource,h[5]+c*20+18)[0]>100),0)
    for _ in range(index//5):live.touch(198,281)
    live.touch(110,83+(index%5)*36);live.touch(120,281);assert u32('details_open')==1
    live.key(5,3);scroll=u32('detail_scroll');assert scroll>0
    live.shot('details');record('long_description_scroll',card=cards[index],scroll=scroll)
    live.key(9);live.touch(30,305);assert diag()['profile'][4]==0
    live.touch(120,288);assert u32('about_open')==1;live.shot('about');live.key(9)
    live.touch(120,233);assert diag()['profile'][4]==6;live.shot('records');live.key(9)
    record('about_and_records')
    live.touch(120,150)
    for _ in range(12):
        opening=list(struct.unpack('<12I',memory('joey_opening_diagnostic')))
        if opening[0]==0:live.touch(120,110)
        elif opening[0]==1:live.touch(120,281)
        elif opening[0]==2:live.shot('opening');live.touch(65,281)
        elif opening[0]==3:live.shot('opening');live.touch(120,281)
        else:break
    assert diag()['profile'][4]==1,diag()
    wait_idle();live.shot('board');record('opening_and_duel')
    # Entire pause menu and secondary mode are reachable using only six keys.
    live.key(9);assert u32('menu9588')==1;live.shot('pause-menu')
    live.key(5,2);live.key(10);assert u32('menu9588')==0 and u32('secondary9588')==1
    live.touch(90,305);assert u32('secondary9588')==0
    record('physical_key_menu')
    hand,_=state();candidates=[i for i,c in enumerate(hand) if DEFINITIONS[c][4]==0 and DEFINITIONS[c][3]<=4 and DEFINITIONS[c][5]==0]
    assert candidates,hand
    i=candidates[0];live.touch(31+i*35,251);live.touch(31+i*35,251)
    if diag()['pending']==14:live.touch(120,163)
    wait_idle();_,monsters=state();assert any(monsters),(hand,diag())
    live.shot('summoned');record('summon',card=hand[i],monsters=monsters)
    hand,_=state();i=next((i for i,c in enumerate(hand) if DEFINITIONS[c][4] in (1,2)),None)
    if i is not None:
        live.touch(31+i*35,251);live.touch(90,305);live.touch(31+i*35,251);wait_idle();live.shot('set-card');record('set_backrow',card=hand[i])
    # Progress naturally; letting the AI win verifies combat, results and record
    # persistence without constructing a privileged winning game state.
    full_end=time.monotonic()+240;turns=set();pagination=False
    while time.monotonic()<full_end:
        d=diag();turns.add(d['turn'])
        if d['view']==5:break
        if d['pending']==8:live.shot('response');live.key(9);continue
        if d['pending']==7:live.shot('effect-choice');live.key(10);continue
        if d['active']==0:
            if d['hand']>6 and not pagination:
                live.touch(234,251);assert u32('hand_scroll')>0;live.shot('hand-page');live.touch(5,251);pagination=True
            live.touch(220,216);time.sleep(.35);after=diag()
            if after['result']==11 or (after['phase']==5 and after['active']==0 and after['hand']>6):
                for _ in range(max(0,after['hand']-6)):live.touch(31,251);live.touch(31,251)
        time.sleep(.5)
    end=diag();assert end['view']==5,end
    live.shot('result');record('complete_duel',turns=sorted(turns),life=end['life'],hand_pagination=pagination)
    assert end['profile'][3]==1,end
    final_save=save_export(end['profile'][1]);assert final_save['losses']>restored['losses']
    live.touch(100,290);live.touch(120,233);live.shot('records-after-duel');live.key(9)
    record('result_record_persisted',save=final_save)
    REPORT['program_after_duel']=verify_program()
    status=live.req('/api/status').json();assert status['running'] and not status.get('stop_reason')
    assert status['audio_transport']['received_packets']>0
    REPORT['audio_transport']=status['audio_transport'];REPORT['passed']=True
    (OUT/'firmware-result.json').write_text(json.dumps(REPORT,ensure_ascii=False,indent=2),encoding='utf8');print('PASS',flush=True)
if __name__=='__main__':main()
