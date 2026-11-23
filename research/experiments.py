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
