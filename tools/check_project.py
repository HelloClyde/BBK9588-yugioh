"""Audit tracked public files, fixed gitlinks and README image references."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
    def git(*args):return subprocess.check_output(['git','-C',str(ROOT),*args],text=True,encoding='utf8').strip()
    pins=json.loads((ROOT/'dependencies.json').read_text(encoding='utf8'))
    for name in ('sdk','upstream'):
        row=git('ls-files','--stage','--',name).split()
        assert row[:2]==['160000',pins[name]['commit']],(name,row)
        assert git('-C',str(ROOT/name),'rev-parse','HEAD')==pins[name]['commit']
    paths=git('ls-files','-z').split('\0')
    forbidden={'.bda','.bin','.pak','.sav','.exe','.dll','.ttf','.otf','.wav','.pcm','.zip','.elf','.o'}
    for name in paths:
        if not name or name in ('sdk','upstream'):continue
        path=ROOT/name
        assert path.suffix.lower() not in forbidden,name
        assert not any(part in ('.local-assets','.toolchain','build','dist','qa') for part in path.relative_to(ROOT).parts),name
        if path.suffix in ('.py','.ps1','.h','.c','.inc','.yml','.md','.json'):
            text=path.read_text(encoding='utf8')
            assert not re.search(r'[A-Za-z]:[/\\]Users[/\\]|E:[/\\]CodexTemp',text),name
    readme=(ROOT/'README.md').read_text(encoding='utf8')
    for heading in ('快速开始','截图','依赖','感谢'):assert '## '+heading in readme
    images=re.findall(r'!\[[^\]]*\]\(([^)]+)\)',readme)
    assert len(images)>=2
    for image in images:assert not image.startswith(('http:','https:','file:')) and (ROOT/image).is_file(),image
    for md in [ROOT/'README.md',*sorted((ROOT/'docs').glob('*.md'))]:
        for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)',md.read_text(encoding='utf8')):
            if '://' in target or target.startswith('#'):continue
            assert (md.parent/target.split('#')[0]).exists(),(md.name,target)
    print(json.dumps({'passed':True,'tracked_files':len(paths),'relative_readme_images':len(images),'private_resources_tracked':False},indent=2))
if __name__=='__main__':main()
