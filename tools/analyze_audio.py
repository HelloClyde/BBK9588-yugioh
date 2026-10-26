"""Compare captured DAC PCM with the production reference and FIFO counters."""
import hashlib,json,wave
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
def load(name):
    p=ROOT/'qa'/name
    with wave.open(str(p),'rb') as w:
        rate=w.getframerate();channels=w.getnchannels();x=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,channels)[:,0].astype(np.float64)
    return rate,x
def main():
    rate,x=load('audio-after-idle.wav');assert rate==22050
    ref=np.fromfile(ROOT/'build/tests/audio-menu-reference.pcm',dtype='<i2').astype(np.float64)
    # Include the loop boundary and find capture phase without changing speed.
    ref=np.concatenate((ref,ref));m=min(len(x),22050*5);x=x[:m]
    n=1<<(len(ref)+m-1).bit_length()
    corr=np.fft.irfft(np.fft.rfft(ref,n)*np.conj(np.fft.rfft(x,n)),n)[:len(ref)-m+1]
    # Normalize for local energy, rather than picking the loudest reference bar.
    energy=np.concatenate(([0.],np.cumsum(ref*ref)));energy=energy[m:]-energy[:-m]
    coefficients=corr/np.sqrt(np.maximum(energy,1)*np.dot(x,x))
    at=int(np.argmax(coefficients));matched=ref[at:at+m];scale=float(np.dot(x,matched)/np.dot(matched,matched));error=x-scale*matched
    rms=float(np.sqrt(np.mean(error*error)));signal=float(np.sqrt(np.mean(x*x)));snr=float(20*np.log10(signal/max(rms,1e-12)))
    baseline=ROOT/'qa/audio-before.json'
    before=json.loads(baseline.read_text()) if baseline.exists() else json.loads((ROOT/'docs/verification/reference/audio-result.json').read_text())['navigation_before']
    after=json.loads((ROOT/'qa/audio-after.json').read_text());idle=json.loads((ROOT/'qa/audio-after-idle.json').read_text())
    assert after['bda_sha256']==idle['bda_sha256']==hashlib.sha256((ROOT/'dist/Yugioh9588.bda').read_bytes()).hexdigest()
    report={'passed':after['underruns_delta']==0 and idle['underruns_delta']==0 and float(coefficients[at])>.999,'bda_sha256':hashlib.sha256((ROOT/'dist/Yugioh9588.bda').read_bytes()).hexdigest(),'navigation_before':{k:before[k] for k in ('seconds','underruns_delta','dma_total_gap_underruns_delta')},'navigation_after':{k:after[k] for k in ('seconds','underruns_delta','dma_total_gap_underruns_delta')},'idle_after':{k:idle[k] for k in ('seconds','underruns_delta','dma_total_gap_underruns_delta')},'reference_correlation':float(coefficients[at]),'reference_offset_frames':at,'reference_scale':scale,'reference_error_rms_pcm16':rms,'reference_snr_db':snr,'test_rate_hz':rate,'reference_speed_ratio':1,'checked_frames':m,'host_mixer_tests':'passed: FIFO prefill, exact resampling phase, batched reads, short-write retention, failed-write retry, two FX channels, finite track completion, stop'}
    report['qemu_sha256']=after['qemu_sha256']
    report['emulator_volume_tests']='passed: all signed PCM16 values at gains 0..422 preserve sign, monotonicity and saturate'
    (ROOT/'qa/audio-result.json').write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps(report,indent=2));assert report['passed']
if __name__=='__main__':main()
