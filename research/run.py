import argparse
import json
import signal
import time
from pathlib import Path
from common import manifest,write_json
import experiments
from experiments import EXPERIMENTS

def validate(doc):
    assert doc['status'] in ('completed','incomplete','error')
    assert len(doc['manifest']['core_sha256'])==64 and len(doc['manifest']['spec_sha256'])==64
    for item in doc['experiments'].values():
        assert item['status'] in ('completed','incomplete','error')
        if 'requested' in item: assert 0<=item['executed']<=item['requested']

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--mode',choices=['smoke','full'],required=True)
    p.add_argument('--budget',type=float,default=1800)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--compiler',required=True)
    p.add_argument('--openssl',required=True)
    p.add_argument('--flags',default='')
    p.add_argument('--vector-runner',required=True)
    p.add_argument('--worker',required=True)
    a=p.parse_args()
    experiments.VECTOR_RUNNER=a.vector_runner
    experiments.WORKER=a.worker
    if a.budget<0:p.error('budget must be nonnegative')
    start=time.monotonic();stop=False
    def interrupt(*_):
        nonlocal stop
        stop=True
    signal.signal(signal.SIGINT,interrupt)
    signal.signal(signal.SIGTERM,interrupt)
    def expired(): return stop or time.monotonic()-start>=a.budget
    expired.remaining=lambda: max(0,a.budget-(time.monotonic()-start))
    doc=dict(schema_version=1,mode=a.mode,status='incomplete',budget_seconds=a.budget,
             manifest=manifest(a.compiler,a.openssl,a.flags),
             sample_rule='SHAKE256(ASCII(DualForm-T128/test/)+ASCII(label)+00+LE64(j)+LE32(r)+LE64(i), n)',
             implemented_experiments=[x[0] for x in EXPERIMENTS],experiments={})
    plan=[(name,fn) for name,fn in EXPERIMENTS if a.mode=='full' or name not in experiments.FULL_ONLY]
    doc['selected_experiments']=[name for name,_ in plan]
    for name,fn in plan:
        if expired():break
        t=time.monotonic()
        try: item=fn(a.mode,expired)
        except Exception as e: item=dict(status='error',error=f'{type(e).__name__}: {e}')
        item['elapsed_seconds']=time.monotonic()-t
        doc['experiments'][name]=item
        doc['elapsed_seconds']=time.monotonic()-start
        validate(doc);write_json(a.output,doc)
        if item['status']=='error':break
    states=[x['status'] for x in doc['experiments'].values()]
    doc['status']='error' if 'error' in states else 'completed' if len(states)==len(plan) and all(x=='completed' for x in states) else 'incomplete'
    doc['elapsed_seconds']=time.monotonic()-start
    validate(doc);write_json(a.output,doc)
    print(f"research-{a.mode}: {doc['status']} ({doc['elapsed_seconds']:.3f}s), {a.output}")
    return 1 if doc['status']=='error' else 0
if __name__=='__main__':raise SystemExit(main())
