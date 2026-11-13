#!/usr/bin/env python3
"""Verify recovery snapshots or restore into a NEW EMPTY directory only."""
from pathlib import Path,PurePosixPath
import argparse,hashlib,io,json,tarfile
ROOT=Path(__file__).resolve().parents[1]
sha=lambda data:hashlib.sha256(data).hexdigest()
def read_archive(path):
    entries={}
    with tarfile.open(path,'r:gz') as tar:
        for member in tar.getmembers():
            p=PurePosixPath(member.name)
            if p.is_absolute() or '..' in p.parts or member.issym() or member.islnk():raise RuntimeError('Unsafe archive path')
            if member.isdir():continue
            if not member.isfile() or member.name in entries or member.size>50_000_000:raise RuntimeError('Unsupported archive member')
            entries[member.name]=tar.extractfile(member).read()
    return entries

def validate(version):
    path=ROOT/version['archive']
    if sha(path.read_bytes())!=version['archive_sha256']:raise RuntimeError('Archive checksum: '+version['id'])
    entries=read_archive(path);members=json.loads(entries.pop('_recovery-members.json'))
    if set(entries)!=set(members):raise RuntimeError('Member set mismatch')
    for name,data in entries.items():
        if sha(data)!=members[name]:raise RuntimeError('Member checksum: '+name)
    manifest=json.loads(entries['source-manifest.json'])
    if len(manifest)!=version['source_inputs'] or sha(entries['source-manifest.json'])!=version['source_manifest_sha256']:raise RuntimeError('Source manifest mismatch')
    for name,digest in manifest.items():
        if sha(entries['build-source/'+name])!=digest:raise RuntimeError('Source checksum: '+name)
    if sha(entries[version['artifact']])!=version['artifact_sha256']:raise RuntimeError('User binary checksum')
    return entries

def main():
    p=argparse.ArgumentParser();p.add_argument('--restore',metavar='ID');p.add_argument('--destination',type=Path);args=p.parse_args()
    index=json.loads((ROOT/'version-archives/index.json').read_text())
    versions=index['versions']
    if args.restore:
        versions=[v for v in versions if v['id']==args.restore]
        if len(versions)!=1 or args.destination is None:raise SystemExit('Specify a valid ID and --destination')
        target=args.destination.resolve()
        if target==ROOT or ROOT in target.parents and '.git' in target.parts:raise SystemExit('Refuse repository/Git metadata destination')
        if target.exists() and any(target.iterdir()):raise SystemExit('Destination must be new or empty; no overwrite')
        entries=validate(versions[0]);target.mkdir(parents=True,exist_ok=True)
        for name,data in entries.items():
            path=target/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
            if path.suffix=='.sh':path.chmod(0o755)
        print('Restored',args.restore,'to',target,'; source root:',target/'build-source')
        return
    for v in versions:validate(v)
    release=ROOT/'releases/engineering/M8'
    for line in (release/'SHA256SUMS').read_text().splitlines():
        digest,name=line.split();assert sha((release/name).read_bytes())==digest,name
    print('PASS:',len(versions),'recovery archives/source manifests/binaries; curated M4 source/binary; M8 release hashes')
if __name__=='__main__':main()
