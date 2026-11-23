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

def cancellation(mode,budget):
    n=16 if mode=='smoke' else 1024
    executed=positive=observed=0
    for offset in range(0,n,16):
        if budget():break
        cases=[];controls=[]
        for j in range(offset,min(offset+16,n)):
            k=sample('control-key',j,0,0,32);t=sample('control-tweak',j,0,0,16);p=sample('control-plain',j,0,0,16)
            a,b=struct.unpack('<QQ',sample('control-delta',j,0,0,16))
            dl=struct.pack('<QQ',b,a);dt=struct.pack('<QQ',a,b)
            t2=bytes(x^y for x,y in zip(t,dt));p2=bytes(x^y for x,y in zip(p,dl))
            base=struct.unpack('<24Q',shake('DualForm-T128/base',k,192))
            low,high=struct.unpack('<QQ',t);low2,high2=struct.unpack('<QQ',t2)
            c1=encrypt((base[0]^low,base[1]^high),p,2);c2=encrypt((base[0]^low2,base[1]^high2),p2,2)
            assert bytes(x^y for x,y in zip(c1,c2))==dl
            controls.append(dl);cases.extend([(k,t,p),(k,t2,p2)])
        rows=c_cases(cases)
        for i,delta in enumerate(controls):
            c1=bytes.fromhex(rows[2*i]['trace'][2]);c2=bytes.fromhex(rows[2*i+1]['trace'][2])
            observed+=bytes(x^y for x,y in zip(c1,c2))==delta
            positive+=1;executed+=1
    return dict(status='completed' if executed==n else 'incomplete',requested=n,executed=executed,rounds=2,
                positive_control_successes=positive,current_schedule_successes=observed,
                fixture_rule='control-key/tweak/plain/delta; j=0..N-1, r=i=0',
                limitation='current success count is observed, not an acceptance threshold or proof against related-tweak attacks')
EXPERIMENTS.append(('public-tweak-cancellation-control',cancellation))

import math
WORKER=None

def bounds(k,n,tests):
    if not n:return dict(rate=None,log10_p_upper=None,log10_p_adjusted=None)
    d=abs(k/n-.5)
    log=min(0,math.log10(2)-2*n*d*d/math.log(10))
    return dict(rate=k/n,log10_p_upper=log,log10_p_adjusted=min(0,log+math.log10(tests)))

def worker(mode,j,r,n,v,budget):
    proc=subprocess.run([WORKER,mode,str(j),str(r),str(n),str(v),str(budget.remaining()),'v1'],capture_output=True,text=True,check=True)
    row=json.loads(proc.stdout)
    assert row['requested']==n and 0<=row['executed']<=n
    assert all(0<=k<=row['executed'] for k in row['counts'])
    return row

def fixed_bias(mode,budget):
    configurations=[(0,4,256)] if mode=='smoke' else [(j,r,32768) for j in range(8) for r in (4,5,6,7,8,24)]
    rows=[]
    for j,r,n in configurations:
        if budget():break
        row=worker('bias',j,r,n,0,budget)
        row.update(bounds(row['counts'][0],row['executed'],48));rows.append(row)
        if row['status']!='completed':break
    executed=sum(x['executed'] for x in rows);requested=sum(x[2] for x in configurations)
    return dict(status='completed' if executed==requested else 'incomplete',requested=requested,executed=executed,settings=rows,
                input_delta='L=0000000080000000, R=0',output_bit=1,output_mask='serialized byte0 & 02',
                multiple_tests=48,fixture_rule='bias-key/tweak(j,0,0); bias-plain(j,r,i)',
                interpretation='Hoeffding bound under independent Bernoulli approximation; no trail/hull proof or CI p-value gate')
EXPERIMENTS.append(('fixed-four-round-differential',fixed_bias))
