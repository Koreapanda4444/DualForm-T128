import json
import subprocess
import sys
from pathlib import Path
import reference as ref
v=json.loads(Path(__file__).with_name('vectors.json').read_text())
lines=''.join(f"{x['key']} {x['tweak']} {x['plaintext']}\n" for x in v)
r=subprocess.run([sys.argv[1]],input=lines,text=True,capture_output=True,check=True)
results=[json.loads(line) for line in r.stdout.splitlines()]
assert len(results)==len(v)
for x,y in zip(v,results):
    assert y['trace'][24]==x['ciphertext']
    for field,target in [('prefix1',y['trace'][1]),('prefix2',y['trace'][2]),('encoded',y['encoded']),('sk0',y['sk'][0]),('sk1',y['sk'][1])]:
        if field in x: assert x[field]==target,(x['name'],field)
    assert y['trace']==ref.encrypt(ref.schedule(bytes.fromhex(x['key']),bytes.fromhex(x['tweak'])),bytes.fromhex(x['plaintext']),trace=True)
    assert y['inverse']==[x['plaintext']]*25
    assert y['public_decrypted']==x['plaintext']
print('3 fixed fixtures and 75 prefixes passed')
