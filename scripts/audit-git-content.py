#!/usr/bin/env python3
"""Conservative high-confidence secret scan; reports paths/rules, NEVER values."""
from pathlib import Path,PurePosixPath
import argparse,io,json,re,subprocess,tarfile,zipfile
ROOT=Path(__file__).resolve().parents[1]
RULES={
'private-key':rb'-----BEGIN (?:RSA |EC |DSA |OPENSSH )?PRIVATE KEY-----',
'github-token':rb'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})\b',
'aws-access-key':rb'\b(?:AKIA|ASIA)[A-Z0-9]{16}\b',
'slack-token':rb'\bxox[baprs]-[A-Za-z0-9-]{20,}\b',
'stripe-live-key':rb'\bsk_live_[A-Za-z0-9]{20,}\b',
'google-api-key':rb'\bAIza[A-Za-z0-9_-]{35}\b',
'credential-assignment':rb'''(?i)(?:password|client_secret|access_token|api_key)\s*[:=]\s*["']([A-Za-z0-9_./+=-]{16,})["']''',
}
PATTERNS={name:re.compile(pattern) for name,pattern in RULES.items()}
BANNED={'.netrc','.pypirc','.npmrc','id_rsa','id_ed25519','credentials.json','token.json','.DS_Store'}
def scan(name,data,findings,nested=False):
    parts=PurePosixPath(name).parts
    if any(p in BANNED or p=='.env' or p.startswith('.env.') and p not in ('.env.example','.env.sample') for p in parts):findings.append({'path':name,'rule':'private-file-name'})
    for rule,pattern in PATTERNS.items():
        if pattern.search(data):findings.append({'path':name,'rule':rule})
    if nested:return
    if name.endswith('.tar.gz'):
        with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as tar:
            for m in tar.getmembers():
                if m.issym() or m.islnk():findings.append({'path':name,'rule':'archive-link'})
                if m.isfile():scan(name+'!/'+m.name,tar.extractfile(m).read(),findings,True)
    elif name.endswith('.zip'):
        with zipfile.ZipFile(io.BytesIO(data)) as z:
            for member in z.namelist():
                if not member.endswith('/'):scan(name+'!/'+member,z.read(member),findings,True)

def main():
    p=argparse.ArgumentParser();p.add_argument('--staged',action='store_true');args=p.parse_args()
    cmd=['git','diff','--cached','--name-only','--diff-filter=ACM','-z'] if args.staged else ['git','ls-files','--cached','--others','--exclude-standard','-z']
    names=[n for n in subprocess.check_output(cmd,cwd=ROOT).decode().split('\0') if n];findings=[]
    for name in names:
        data=subprocess.check_output(['git','show',':'+name],cwd=ROOT) if args.staged else (ROOT/name).read_bytes()
        if name.startswith('.phase1/') or any(p.endswith('.dSYM') for p in Path(name).parts) or Path(name).suffix in ('.pyc','.o','.vvp'):findings.append({'path':name,'rule':'build-cache'})
        scan(name,data,findings)
    print(json.dumps({'files_scanned':len(names),'findings':findings,'limit':'High-confidence rules, not a guarantee against every possible secret.'},indent=2))
    raise SystemExit(bool(findings))
if __name__=='__main__':main()
