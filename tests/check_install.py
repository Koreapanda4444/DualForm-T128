import argparse
import subprocess
import tempfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--build',required=True);p.add_argument('--config',default='Release');p.add_argument('--cmake',default='cmake');p.add_argument('--ctest',default='ctest');p.add_argument('--openssl-root',required=True)
a=p.parse_args()
source=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='install-check-',dir=a.build) as temp:
    prefix=Path(temp)/'prefix';consumer=Path(temp)/'consumer'
    def run(*cmd):subprocess.run(cmd,check=True)
    run(a.cmake,'--install',a.build,'--prefix',str(prefix),'--config',a.config)
    assert sorted(x.name for x in (prefix/'include').iterdir())==['dualform.h']
    assert not list(prefix.rglob('*internal*')) and not list(prefix.rglob('*research*'))
    run(a.cmake,'-S',str(source/'examples/consumer'),'-B',str(consumer),'-DCMAKE_PREFIX_PATH='+str(prefix),'-DCMAKE_BUILD_TYPE='+a.config,'-DOPENSSL_ROOT_DIR='+a.openssl_root)
    run(a.cmake,'--build',str(consumer),'--config',a.config)
    run(a.ctest,'--test-dir',str(consumer),'-C',a.config,'--output-on-failure')
print('Installed public header/package/library consumed by an independent CMake project; KAT passed')
