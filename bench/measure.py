import argparse
import json
import subprocess
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'research'))
from common import manifest,write_json
MODES=['prepared_encrypt','key_setup','tweak_setup','tweak_and_block','key_tweak_and_block','prepared_decrypt']

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--exe',required=True);p.add_argument('--compiler',required=True)
    p.add_argument('--openssl',required=True);p.add_argument('--flags',default='');p.add_argument('--output',required=True)
    a=p.parse_args()
    rows=[json.loads(subprocess.check_output([a.exe,m,'1024'],text=True)) for m in MODES]
    write_json(a.output,dict(manifest=manifest(a.compiler,a.openssl,a.flags),phase='workload-validation-only',samples=rows))
    print(f'6 workload modes recorded: {a.output}')
if __name__=='__main__':main()
