"""Compare deterministic outcomes; ignore runtime and source-manifest differences."""
import json
import sys

def compare(a,b):
    assert a['status']==b['status']=='completed'
    assert set(a['experiments'])==set(b['experiments'])
    for name,x in a['experiments'].items():
        y=b['experiments'][name]
        for field in ('requested','executed','rank','basis','positive_control_successes','current_schedule_successes','verified_second_plaintexts','failures'):
            assert x.get(field)==y.get(field),(name,field)
        if 'settings' in x:
            assert len(x['settings'])==len(y['settings'])
            for s,t in zip(x['settings'],y['settings']):
                for field in ('key_index','rounds','delta_index','requested','executed','counts','rate','log10_p_upper','log10_p_adjusted'):
                    assert s.get(field)==t.get(field),(name,field)
if __name__=='__main__':
    compare(json.load(open(sys.argv[1])),json.load(open(sys.argv[2])))
    print('All deterministic research outcomes and statistical counts are identical')
