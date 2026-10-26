"""Capture actual emulator PCM over its public audio WebSocket."""
import argparse,base64,hashlib,json,os,socket,struct,time,wave
from pathlib import Path
from urllib.parse import urlparse
import requests

def capture(base,seconds,path,events=None):
    url=urlparse(base);sock=socket.create_connection((url.hostname,url.port or 80),timeout=8)
    key=base64.b64encode(os.urandom(16)).decode()
    sock.sendall((f'GET /audio HTTP/1.1\r\nHost: {url.netloc}\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n').encode())
    data=b''
    while b'\r\n\r\n' not in data:data+=sock.recv(4096)
    header,pending=data.split(b'\r\n\r\n',1);assert b'101 Switching' in header,header
    def read(n):
        nonlocal pending
        while len(pending)<n:
            new=sock.recv(max(4096,n-len(pending)))
            if not new:raise EOFError('audio WebSocket closed')
            pending+=new
        out,pending=pending[:n],pending[n:];return out
    start=time.monotonic();chunks=[];timings=[];rate=channels=None;done=set()
    status0=requests.get(base+'/api/status',timeout=10).json()
    while time.monotonic()-start<seconds:
        now=time.monotonic()-start
        for i,(when,x,y) in enumerate(events or []):
            if now>=when and i not in done:
                for down in (1,0):requests.post(base+'/api/touch',params=dict(x=x,y=y,down=down),timeout=8);time.sleep(.12)
                done.add(i)
        flags,length=read(2);size=length&127
        if size==126:size=struct.unpack('>H',read(2))[0]
        elif size==127:size=struct.unpack('>Q',read(8))[0]
        mask=read(4) if length&128 else None;payload=read(size)
        if mask:payload=bytes(x^mask[i%4] for i,x in enumerate(payload))
        if flags&15==8:break
        if flags&15!=2:continue
        magic,seq,r,c,bits,n=struct.unpack_from('<8sIIHHI',payload)
        assert magic==b'BBKAUD1\0' and bits==16 and n==len(payload)-24
        if rate is None:rate,channels=r,c
        assert (r,c)==(rate,channels),(r,c,rate,channels)
        timings.append([round(time.monotonic()-start,6),seq,n]);chunks.append(payload[24:])
    sock.close();pcm=b''.join(chunks)
    with wave.open(str(path),'wb') as wav:wav.setnchannels(channels);wav.setsampwidth(2);wav.setframerate(rate);wav.writeframes(pcm)
    status1=requests.get(base+'/api/status',timeout=10).json()
    def audio(j):return j['qemu']['performance']['audio']
    report={'rate':rate,'channels':channels,'seconds':len(pcm)/2/channels/rate,'packets':len(chunks),'sha256':hashlib.sha256(pcm).hexdigest(),'timings':timings,'before':audio(status0),'after':audio(status1)}
    program=requests.get(base+'/api/files/export',params={'path':os.environ.get('YUGIOH_PROGRAM_PATH','/应用/程序/游戏王.bda')},timeout=30)
    program.raise_for_status();assert len(program.content)>10000
    report['bda_sha256']=hashlib.sha256(program.content).hexdigest()
    executable=Path(status1['qemu']['command'][0]);report['qemu_executable']=str(executable)
    if executable.exists():report['qemu_sha256']=hashlib.sha256(executable.read_bytes()).hexdigest()
    for k in ('underruns','dma_total_gap_underruns','dma_completion_count','tx_dma_samples','output_frames'):report[k+'_delta']=audio(status1)[k]-audio(status0)[k]
    path.with_suffix('.json').write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps({k:v for k,v in report.items() if k not in ('timings','before','after')},indent=2))

if __name__=='__main__':
    a=argparse.ArgumentParser();a.add_argument('--base',default='http://127.0.0.1:8017');a.add_argument('--seconds',type=float,default=12);a.add_argument('--out',type=Path,required=True);a.add_argument('--navigate',action='store_true');v=a.parse_args();v.out.parent.mkdir(parents=True,exist_ok=True)
    events=[(2,120,180),(5,120,281),(8,210,20),(10,30,305)] if v.navigate else []
    capture(v.base,v.seconds,v.out,events)
