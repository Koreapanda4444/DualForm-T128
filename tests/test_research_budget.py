import json
import subprocess
import sys
import tempfile
import time
import signal
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    out=Path(tmp)/'result.json'
    common=[sys.executable,str(root/'research/run.py'),'--mode','full','--output',str(out),'--compiler',sys.argv[3],'--openssl',sys.argv[4],'--vector-runner',sys.argv[1],'--worker',sys.argv[2]]
    subprocess.run([*common,'--budget','0'],check=True)
    doc=json.loads(out.read_text());assert doc['status']=='incomplete' and not doc['experiments']
    subprocess.run([*common,'--budget','.15'],check=True)
    doc=json.loads(out.read_text());assert doc['status']=='incomplete' and doc['experiments']
    assert any(e.get('executed',0)>0 for e in doc['experiments'].values())
    if sys.platform!='win32':
        out.unlink();proc=subprocess.Popen([*common,'--budget','1800'])
        for _ in range(100):
            if out.exists():break
            time.sleep(.01)
        proc.send_signal(signal.SIGTERM);proc.wait(timeout=30)
        doc=json.loads(out.read_text());assert doc['status']=='incomplete'
        assert any(e.get('executed',0)>0 for e in doc['experiments'].values())
print('Zero/short budgets and supported signal interruption preserve status and executed samples')
