import argparse
import json
import subprocess
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'research'))
from common import manifest,write_json
MODES=['prepared_encrypt','key_setup','tweak_setup','tweak_and_block','key_tweak_and_block','prepared_decrypt']

import math
import statistics
from common import ROOT,digest_files

def validate_sample(row,mode):
    assert row['mode']==mode and isinstance(row['iterations'],int) and row['iterations']>0
    for k in ('seconds','ns_per_op','ops_per_second'):
        assert isinstance(row[k],(int,float)) and math.isfinite(row[k]) and row[k]>0
    assert isinstance(row['checksum'],int)
    n=row['iterations'];sec=row['seconds']
    assert math.isclose(row['ns_per_op'],sec*1e9/n,rel_tol=1e-4)
    assert math.isclose(row['ops_per_second'],n/sec,rel_tol=1e-4)
    if mode in ('key_setup','tweak_setup'):
        assert 'mib_per_second' not in row
    else:
        assert math.isclose(row['mib_per_second'],n*16/1048576/sec,rel_tol=1e-4)

def collect(exe,mode):
    n=1024;calibration=[]
    while True:
        row=json.loads(subprocess.check_output([exe,mode,str(n)],text=True))
        validate_sample(row,mode);calibration.append(row)
        if row['seconds']>=.22:break
        n=min(50000000,max(n+1,math.ceil(n*.25/row['seconds'])))
        if n==50000000 and row['seconds']<.001:raise ValueError('cannot calibrate workload')
    samples=[]
    while len(samples)<5:
        row=json.loads(subprocess.check_output([exe,mode,str(n)],text=True))
        validate_sample(row,mode)
        if row['seconds']<.2:
            calibration.append(row);n=math.ceil(n*.25/row['seconds']);continue
        samples.append(row)
    values=[x['ns_per_op'] for x in samples];median=statistics.median(values)
    result=dict(samples=samples,calibration=calibration,median_ns=median,min_ns=min(values),max_ns=max(values),ops_per_second=1e9/median)
    if mode not in ('key_setup','tweak_setup'):result['mib_per_second']=16*1e9/median/1048576
    return result

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--exe',required=True);p.add_argument('--compiler',required=True)
    p.add_argument('--openssl',required=True);p.add_argument('--flags',default='');p.add_argument('--output',required=True)
    a=p.parse_args()
    doc=dict(manifest=manifest(a.compiler,a.openssl,a.flags),status='incomplete',modes={},
        measurement_tools_sha256=digest_files(list((ROOT/'bench').glob('*.[ch]'))+list((ROOT/'bench').glob('*.py'))+[ROOT/'research/timer.h']),
        contract='C internal loops; input generation outside clock; API allocation and cleansing included; 128 untimed warmup operations per invocation; at least 5 measured samples >= 200ms',
        hardware_acceleration='OpenSSL automatic runtime dispatch, selected machine-code path unverified',cpu_affinity='not pinned')
    for mode in MODES:
        doc['modes'][mode]=collect(a.exe,mode);write_json(a.output,doc)
    doc['status']='completed';write_json(a.output,doc)
    print('mode: median ns/op (5 samples, each >=200ms)')
    for mode,row in doc['modes'].items():print(f"{mode}: {row['median_ns']:.3f}")
if __name__=='__main__':main()
