#!/usr/bin/env python3
"""Recover existing disk snapshots NOW; never manufacture milestone Git history."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,gzip,hashlib,io,json,tarfile
ROOT=Path(__file__).resolve().parents[1]
SOURCES={'phase1':'.phase1/toolchain','m2a':'.phase1/milestones/M2A','m2b':'.phase1/milestones/M2B','m3':'.phase1/milestones/M3','m4':'.phase1/milestones/M4','m5':'.phase1/milestones/M5-engineering','m6':'.phase1/milestones/M6-engineering','custom-width':'.phase1/milestones/custom-width-engineering','m7':'.phase1/milestones/M7-engineering','m8':'.phase1/ecen21-m8'}
sha=lambda b:hashlib.sha256(b).hexdigest()
def selected(relative):
    parts=relative.parts
    if any(x in ('__pycache__','.DS_Store') or x.endswith('.dSYM') for x in parts):return False
    if any(x=='build' or x.startswith('build-') for x in parts[1:]) and parts[0]!='build-source':return False
    if relative.suffix in ('.pyc','.o','.a','.vvp'):return False
    if parts[0]=='build-source':return True
    if parts[0]=='artifacts':return True
    if len(parts)==1:return relative.name!='image-inspect.json' and relative.suffix in ('.json','.sha256','.script')
    if parts[0] in ('verification','logs','reproduction','closure-records'):
        return relative.suffix in ('.png','.script','.json','.txt','.log','.md','.sh','.py','.sha256','.v')
    if parts[0]=='host-tests':return relative.suffix in ('.log','.sh')
    return False

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--create',action='store_true',required=True);parser.parse_args()
    dest=ROOT/'version-archives';dest.mkdir(exist_ok=True)
    index={'schema':1,'recovered_at_utc':datetime.now(timezone.utc).isoformat(),'historical_milestone_git_commits_existed':False,'note':'Recovered existing engineering snapshots in the P0 task. These are not backdated commits. Accepted historical versions are indexed separately from pending engineering snapshots.','versions':[]}
    for key,location in SOURCES.items():
        original=ROOT/location
        manifest=json.loads((original/'source-manifest.json').read_text())
        for name,digest in manifest.items():
            p=original/'build-source'/name
            if not p.is_file() or sha(p.read_bytes())!=digest:raise RuntimeError('Corrupt historical input: '+key+'/'+name)
        entries={}
        for path in sorted(original.rglob('*')):
            if path.is_symlink():raise RuntimeError('Unexpected symlink: '+str(path))
            if path.is_file() and selected(path.relative_to(original)):entries[str(path.relative_to(original))]=path.read_bytes()
        if key=='m8':
            for name in ('AGENTS.md','PLAN.md','PROGRESS.md','TESTING.md','PRD.md'):
                entries['recovered-records/'+name]=(ROOT/name).read_bytes()
            for path in (ROOT/'docs').rglob('*'):
                if path.is_file() and path.suffix in ('.md','.json'):entries['recovered-records/'+str(path.relative_to(ROOT))]=path.read_bytes()
        members={name:sha(data) for name,data in entries.items()}
        entries['_recovery-members.json']=(json.dumps(members,indent=2,sort_keys=True)+'\n').encode()
        archive=dest/(key+'.tar.gz')
        if archive.exists():raise RuntimeError('Refuse to replace archive: '+str(archive))
        with archive.open('wb') as raw:
            with gzip.GzipFile(filename='',fileobj=raw,mode='wb',mtime=0) as zipped:
                with tarfile.open(fileobj=zipped,mode='w') as tar:
                    for name,data in sorted(entries.items()):
                        info=tarfile.TarInfo(name);info.size=len(data);info.mode=0o755 if name.endswith('.sh') else 0o644;info.mtime=0
                        tar.addfile(info,io.BytesIO(data))
        artifact='artifacts/Logic.g3a' if key=='phase1' else 'artifacts/ECEN21.g3a'
        if artifact not in entries:raise RuntimeError('Missing user binary: '+key)
        accepted=key in ('phase1','m2a','m2b','m3','m4')
        index['versions'].append({'id':key,'original_local_snapshot':location,'archive':str(archive.relative_to(ROOT)),'archive_sha256':sha(archive.read_bytes()),'archive_bytes':archive.stat().st_size,'source_inputs':len(manifest),'members':len(members),'source_manifest_sha256':sha(entries['source-manifest.json']),'artifact':artifact,'artifact_sha256':sha(entries[artifact]),'artifact_bytes':len(entries[artifact]),'engineering':'passed','user_acceptance':'passed (user-reported)' if accepted else 'deferred' if key in ('m5','m6') else 'pending','physical_acceptance':'not inferred from software checks' if key=='phase1' else 'passed (explicit user report; not agent-observed)' if accepted else 'deferred' if key in ('m5','m6') else 'pending'})
    (dest/'index.json').write_text(json.dumps(index,indent=2)+'\n')
    print('Created',len(index['versions']),'recovery archives; no historical Git commits created')
if __name__=='__main__':main()
