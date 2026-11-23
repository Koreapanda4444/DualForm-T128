"""Cross-implementation comparison, not a differential attack."""
import json
import subprocess
import sys
import reference as ref
cases=[(ref.sample('kat-key',j,0,0,32),ref.sample('kat-tweak',j,0,0,16),ref.sample('kat-plain',j,0,0,16)) for j in range(1024)]
request=''.join(f'{k.hex()} {t.hex()} {p.hex()}\n' for k,t,p in cases)
proc=subprocess.run([sys.argv[1]],input=request,text=True,capture_output=True,check=True)
rows=[json.loads(x) for x in proc.stdout.splitlines()]
assert len(rows)==1024
for j,((k,t,p),row) in enumerate(zip(cases,rows)):
    sk=ref.schedule(k,t)
    assert row['encoded']==ref.encode(k,t).hex(),('encoding',j)
    assert row['sk']==[f'{x:016x}' for x in sk],('subkey',j)
    trace=ref.encrypt(sk,p,trace=True)
    assert row['trace']==trace,('trace',j)
    assert row['inverse']==[p.hex()]*25,('C inverse',j)
    assert row['public_decrypted']==p.hex(),('public decryption',j)
    for q,c in enumerate(trace):
        assert ref.decrypt(sk,bytes.fromhex(c),q)==p,('Python inverse',j,q)
print('1024 combinations, 24576 subkeys, 25600 prefixes and both inverses: 0 mismatches')
