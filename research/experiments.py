import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tests'))
from reference import sample

def input_reproduction(mode,budget):
    n=16 if mode=='smoke' else 1024
    observed=[]
    for j in range(n):
        if budget(): break
        a=sample('kat-plain',j,0,0,16)
        assert a==sample('kat-plain',j,0,0,16)
        assert a!=sample('kat-plain',j,1,0,16)
        observed.append(a.hex())
    return dict(requested=n,executed=len(observed),fixtures=observed[:2],status='completed' if len(observed)==n else 'incomplete')

EXPERIMENTS=[('input-reproduction',input_reproduction)]

from reference import S,rol,even,odd,M64,M32
INVERSE_S=tuple(S.index(i) for i in range(16))
def inverse_linear(y):
    for j in range(6): y=y^rol(y,11*(1<<j))^rol(y,37*(1<<j))
    return y

def inverse_even(y):
    x=inverse_linear(y)
    return sum(INVERSE_S[(x>>(4*i))&15]<<(4*i) for i in range(16))

def inverse_odd(y):
    a,b=y&M32,y>>32
    b=(b-rol(a,5,32))&M32
    a^=rol(b,17,32)
    b^=rol(a,11,32)
    a=(a-rol(b,7,32))&M32
    return a|(b<<32)

def inversion(mode,budget):
    basis=[(1<<i)^rol(1<<i,11)^rol(1<<i,37) for i in range(64)]
    pivots={}
    for col in basis:
        v=col
        while v:
            b=v.bit_length()-1
            if b in pivots:v^=pivots[b]
            else:pivots[b]=v;break
    assert len(pivots)==64
    for i,x in enumerate(basis):assert inverse_linear(x)==1<<i
    boundary=[0,1,M64,M32,1<<63,1<<31,0xffffffff00000000]
    words=boundary+([int.from_bytes(sample('inverse-word',i,0,0,8),'little') for i in range(10000)] if mode=='full' else [])
    count=0
    for x in words:
        if budget():break
        assert inverse_even(even(x))==x and even(inverse_even(x))==x
        assert inverse_odd(odd(x))==x and odd(inverse_odd(x))==x
        count+=1
    return dict(status='completed' if count==len(words) else 'incomplete',rank=64,basis=64,requested=len(words),executed=count,rounds=[0,1],directions='both',failures=0)
EXPERIMENTS.append(('round-inversion',inversion))

import json
import subprocess
import struct
from reference import schedule,encrypt,shake
VECTOR_RUNNER=None

def c_cases(cases):
    payload=''.join(f'{k.hex()} {t.hex()} {p.hex()}\n' for k,t,p in cases)
    result=subprocess.run([VECTOR_RUNNER],input=payload,text=True,capture_output=True,check=True)
    rows=[json.loads(x) for x in result.stdout.splitlines()]
    assert len(rows)==len(cases)
    return rows

def recover_two_rounds(plain,cipher):
    """Attack receives only the external plaintext and two-round ciphertext."""
    l,r=struct.unpack('<QQ',plain);cl,cr=struct.unpack('<QQ',cipher)
    return r^inverse_even(cl^l),cl^inverse_odd(cr^r)

def subkey_recovery(mode,budget):
    n=16 if mode=='smoke' else 1024
    executed=0
    for offset in range(0,n,16):
        if budget():break
        cases=[]
        for j in range(offset,min(offset+16,n)):
            k=sample('kat-key',j,0,0,32);t=sample('kat-tweak',j,0,0,16)
            cases.extend([(k,t,sample('kat-plain',j,0,0,16)),(k,t,sample('recovery-verify',j,0,0,16))])
        rows=c_cases(cases)
        for j in range(0,len(cases),2):
            p=cases[j][2];ct=bytes.fromhex(rows[j]['trace'][2])
            recovered=recover_two_rounds(p,ct)
            assert recovered==tuple(int(x,16) for x in rows[j]['sk'][:2])
            assert encrypt(recovered,cases[j+1][2],2).hex()==rows[j+1]['trace'][2]
            executed+=1
    return dict(status='completed' if executed==n else 'incomplete',requested=n,executed=executed,rounds=2,
                recovered='SK0 and SK1 only; not the master key or 24-round schedule',verified_second_plaintexts=executed,failures=0,
                fixture_rule='kat-key/kat-tweak/kat-plain and recovery-verify; j=0..N-1, r=i=0')
EXPERIMENTS.append(('two-round-subkey-recovery',subkey_recovery))
