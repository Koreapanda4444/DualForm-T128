"""Reproduction manifest shared by the research and measurement tools."""
import hashlib
import platform
import subprocess
import sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def digest_files(paths):
    h=hashlib.sha256()
    for p in sorted(paths):
        h.update(str(p.relative_to(ROOT)).encode()+b'\0'+p.read_bytes()+b'\0')
    return h.hexdigest()

def manifest(compiler,openssl,flags,compiler_version=None):
    paths=list((ROOT/'src').glob('*.[ch]'))+list((ROOT/'include').glob('*.h'))
    cpu=platform.processor()
    if Path('/proc/cpuinfo').exists():
        cpu=next((x.split(':',1)[1].strip() for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),cpu)
    return dict(core_sha256=digest_files(paths),source_sha256={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)},
                spec_sha256=hashlib.sha256((ROOT/'docs/spec_ko.md').read_bytes()).hexdigest(),
                tools_sha256=digest_files(list((ROOT/'research').glob('*.py'))+list((ROOT/'research').glob('*.c'))+list((ROOT/'tests').glob('*.py'))),
                python=sys.version,os=platform.platform(),cpu=cpu,compiler=compiler_version or (subprocess.check_output([compiler,'--version'],text=True,stderr=subprocess.STDOUT).splitlines()[0] if Path(compiler).name.lower() not in ('cl.exe','cl') else 'MSVC (CMake selected compiler)'),
                compiler_path=compiler,openssl=openssl,flags=flags)

def write_json(path,value):
    import json
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    temp=path.with_suffix(path.suffix+'.tmp');temp.write_text(json.dumps(value,indent=2,allow_nan=False)+'\n');temp.replace(path)
