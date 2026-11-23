"""Keep raw bit counts; derive individual bounds rather than storing 20,480 dictionaries."""
import hashlib
import json
import sys
from pathlib import Path

def compact(source,target):
    raw=Path(source).read_bytes();doc=json.loads(raw)
    for row in doc.get('experiments',{}).get('bounded-related-tweak-screening',{}).get('settings',[]):
        bounds=row.pop('bit_bounds',None)
        if bounds:
            row['minimum_log10_p_upper']=min(b['log10_p_upper'] for b in bounds)
            row['minimum_log10_p_adjusted']=min(b['log10_p_adjusted'] for b in bounds)
    doc['artifact_transform']=dict(raw_json_sha256=hashlib.sha256(raw).hexdigest(),
        rule='All 128 raw counts retained. Individual Hoeffding logs are derived by research/experiments.py:bounds(k,n,20480). Per-setting minima retained.')
    Path(target).write_text(json.dumps(doc,separators=(',',':'),allow_nan=False)+'\n')
if __name__=='__main__':compact(*sys.argv[1:])
