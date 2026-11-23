"""Exercise option dependencies in separate build trees; no dependency downloads."""
import argparse
import subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--cmake',default='cmake')
a=p.parse_args()
configs={
 'library-only':['-DBUILD_TESTING=OFF','-DCMAKE_DISABLE_FIND_PACKAGE_Python3=TRUE'],
 'research-only':['-DBUILD_TESTING=OFF','-DCRYPTO_BUILD_RESEARCH=ON'],
 'bench-only':['-DBUILD_TESTING=OFF','-DCRYPTO_BUILD_BENCHMARKS=ON']}
for name,flags in configs.items():
    build=a.root/name
    subprocess.run([a.cmake,'-S',str(a.source),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release',*flags],check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    subprocess.run([a.cmake,'--build',str(build),'--config','Release','--parallel','4'],check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    if name=='library-only':
        cache=(build/'CMakeCache.txt').read_text();assert 'Python3_EXECUTABLE:' not in cache
    if name=='research-only':subprocess.run([a.cmake,'--build',str(build),'--config','Release','--target','research-smoke'],check=True)
r=subprocess.run([a.cmake,'-S',str(a.source),'-B',str(a.root/'missing-python'),'-DBUILD_TESTING=ON','-DCMAKE_DISABLE_FIND_PACKAGE_Python3=TRUE'],capture_output=True,text=True)
assert r.returncode!=0 and 'Python3' in r.stdout+r.stderr
print('Library-only without Python, research-only, bench-only and required-Python failure validated')
