import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'bench'))
from measure import validate_sample
row=dict(mode='prepared_encrypt',iterations=1000000,seconds=.25,ns_per_op=250,ops_per_second=4000000,checksum=1,mib_per_second=16*4000000/1048576)
validate_sample(row,'prepared_encrypt')
for change in [dict(seconds=0),dict(ns_per_op=-1),dict(ops_per_second=float('nan')),dict(mib_per_second=1),dict(iterations=0)]:
    try:validate_sample(row|change,'prepared_encrypt')
    except AssertionError:pass
    else:raise AssertionError(change)
s=dict(row,mode='key_setup');s.pop('mib_per_second');validate_sample(s,'key_setup')
try:validate_sample(s|{'mib_per_second':1},'key_setup')
except AssertionError:pass
else:raise AssertionError('setup-only throughput accepted')
print('benchmark units, positivity and workload boundary schemas passed')
