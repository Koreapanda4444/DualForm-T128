import json
import subprocess
import sys
sys.path.insert(0,str(__import__('pathlib').Path(__file__).resolve().parents[1]/'tests'))
import reference as ref
for j,r,n in [(0,4,256),(3,8,128),(7,24,64)]:
    k=ref.sample('bias-key',j,0,0,32);t=ref.sample('bias-tweak',j,0,0,16);sk=ref.schedule(k,t)
    count=0
    for i in range(n):
        p=ref.sample('bias-plain',j,r,i,16);p2=bytearray(p);p2[3]^=128
        c=ref.encrypt(sk,p,r);c2=ref.encrypt(sk,p2,r);count+=((c[0]^c2[0])>>1)&1
    row=json.loads(subprocess.check_output([sys.argv[1],'bias',str(j),str(r),str(n),'0','60','v1'],text=True))
    assert row['counts']==[count] and row['executed']==n
print('C worker sample/bit counts match independent Python oracle')
