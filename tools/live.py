"""Small input/screenshot driver for the private firmware emulator."""
import argparse,json,time,os
from pathlib import Path
import requests
ROOT=Path(__file__).resolve().parents[1]
BASE=os.environ.get('YUGIOH_EMULATOR_URL','http://127.0.0.1:8016')
PROGRAM_PATH=os.environ.get('YUGIOH_PROGRAM_PATH','/应用/程序/游戏王.bda')
def req(endpoint,post=False,**params):
    r=(requests.post if post else requests.get)(BASE+endpoint,params=params,timeout=30);r.raise_for_status();return r
def key(code,count=1):
    for _ in range(count):
        for down in (1,0):req('/api/key',True,code=code,down=down);time.sleep(.22)
def touch(x,y):
    for down in (1,0):req('/api/touch',True,x=x,y=y,down=down);time.sleep(.25)
def shot(name):
    time.sleep(.7);p=ROOT/'qa'/(name+'.png');p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(req('/screen.png').content);print(p)
def diagnostics():
    import struct
    from elftools.elf.elffile import ELFFile
    with (ROOT/'build/Yugioh9588.elf').open('rb') as f:
        e=ELFFile(f);symbols={s.name:s['st_value'] for s in e.get_section_by_name('.symtab').iter_symbols()}
    out={}
    for name,count in (('joey_native_diagnostic',24),('joey_profile_diagnostic',12),('joey_opening_diagnostic',12),('joey_audio_diagnostic',32)):
        out[name]=list(struct.unpack('<'+'I'*count,req('/debug/mem.bin',addr=hex(symbols[name]),size=count*4).content))
    return out
if __name__=='__main__':
    a=argparse.ArgumentParser();a.add_argument('op');a.add_argument('values',nargs='*');v=a.parse_args()
    if v.op=='touch':touch(*map(int,v.values[:2]))
    elif v.op=='key':key(*map(int,v.values))
    elif v.op=='launch':touch(198,84);time.sleep(2);touch(120,100);time.sleep(3)
    elif v.op=='status':print(json.dumps(req('/api/status').json(),indent=2))
    elif v.op=='diag':print(json.dumps(diagnostics(),indent=2))
    if v.op not in ('status','diag'):shot(v.values[-1] if v.op=='shot' else 'current')
