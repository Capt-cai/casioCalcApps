# Version control and recovery — P0

Feature development is paused by the user. M9 has not started. This P0 task
protects existing work; it does not implement or accept a new product milestone.

## Initial audit

Before this task the normal local/main and origin/main both pointed to
`12d6b4e1086746534636c3bb87703deeef5734a8`,with60original-project commits.
ECEN21 sources/tests/docs/scripts were untracked;there were no normal ECEN21
milestone commits or tags. Codex internal refs pointed to working-tree objects,
not a sequence of milestone commits. They are not substituted for Git history.
The60original commits are preserved;no rebase/reset/backdating or force push.

Origin is the personal fork `https://github.com/Capt-cai/casioCalcApps.git`.
The original author is `joserodpt/casioCalcApps`;upstream is read-only and its
push URL is DISABLED. Initial noninteractive GitHub API authentication failed401;
browser reauthentication was requested. Actual final sync state is recorded in
PROGRESS/TESTING and the delivery report,not inferred from public ls-remote.

`.gitignore` correctly excluded runtime caches but also hid all `.phase1` recovery
snapshots. Current sources were untracked rather than wrongly ignored. Curated
`version-archives/` now protects all original source manifests/binaries/evidence;
`releases/engineering/M8/` protects the actual user binary and Verilog examples.
Secrets/private-key files/compiler caches remain excluded. Original M4 legacy
bundle contains.DS_Store;it remains unchanged locally. Its curated recovery copy
preserves all source and user-binary hashes without Finder metadata.

## Recoverability and status

See [version index](../version-archives/index.json) and
[archive instructions](../version-archives/README.md). Ten snapshots:Phase1,
M2A,M2B,originalM3,M4,M5,M6,custom-width,M7,M8. They were recovered NOW,not forged
into old commits. All source inputs/member/user-artifact hashes verified and all
ten restored into independent empty directories. Important distinction:

- Phase1 software accepted;physical execution not inferred.
- Original M2A-M4:explicit user-reported hardware acceptance confirmed in this P0
  request;no per-step/OS/performance observations invented.
- Custom-width follow-up:pending,separate from accepted original M3.
- M5/M6:engineering passed;hardware explicitly deferred.
- M7/M8:engineering passed;user/hardware acceptance pending.
- M9:not started/unapproved;all new features currently paused.

M8 engineering tag references the P0 recovery commit and current M8 source,not an
accepted/stable release. No historical M2A-M7 milestone commits are manufactured.
Historical archive documents remain immutable even when they record then-pending
acceptance;the index/root records provide later user reports.

## Ongoing workflow for every future session

1. ReadAGENTS/PLAN/PROGRESS/TESTING;inspect status/log/branch/remotes/tags,then fetch
   verified personal origin and compare ancestry before remote writes.
2. After milestone authorization,use its own `codex/<milestone>-<topic>` branch.
   Keep user edits;create a tested commit after a meaningful coherent change.
3. Before significant refactoring,make a recoverable commit/tag if appropriate;
   never use destructive reset,force push or amend/rebase shared history.
4. Engineering verification ->annotated engineering checkpoint with source and
   artifact hashes,test results and acceptance-pending status. Explicit user
   hardware acceptance of that exact version ->Accepted Release only then.
5. Run content/archive checks,inspect staged diff,commit and auto-push the current
   authorized milestone branch/new checkpoint tags to verified personal origin.
   On auth failure,unexpected URL,conflict/non-FF,stop remote writes and report.
   Do not silently change ownership,visibility or overwrite remote work.
6. P0 first-baseline promotion of personal main may be a fast-forward only after
   verifying it still contains only the observed upstream history. Future main
   promotion needs explicit authorized scope;routine work pushes milestone branch.
7. Delivery reports must state branch,code/checkpoint commit,test status,actual
   remote sync and unresolved risk. No unattended timer performs commits/pushes.

After cloning,run `bash scripts/setup-git-workflow.sh` to install repository-local
hooks (Git intentionally does not auto-enable hooks in a fresh clone). AGENTS
requires this check/setup at session start. Hooks are defense in depth:pre-commit
scans staged blobs/archives for high-confidence secrets;pre-push restricts target,
blocks remote deletion/tag replacement/non-fast-forward history. Rules do not
promise detection of every possible credential;manual staged review remains needed.

## Recovery without modifying the current worktree

```sh
python3 scripts/verify-version-archives.py
python3 scripts/verify-version-archives.py --restore m8 --destination /tmp/ecen21-m8-recovery
```

Use a NEW empty destination. The script refuses to overwrite a nonempty directory.
Restored `build-source/` contains the exact source inputs for that checkpoint.
Restored `artifacts/` contains its original binaries and relevant engineering
files;do not install EC21Test/EC21Bench instead of the user ECEN21 program.

An independent Git bundle is also created locally under `.phase1/version-control/`
for full current branch/tag history. Keep a copy on another device for protection
against local disk loss;only verified remote pushes are called remote backups.
A bundle cannot include its own checksum in its contained commit;its external
sidecar identifies the exact included refs/hashes. Remote restore is checked
independently after successful push. Pinned Docker/emulator external availability
is separate from Git source/binary recovery;large tool installations are not
committed as caches.

## Verified P0 outcome

Code checkpoint840a04d5f9ff9669767d87e9b4f4574eb17d38cd is remotely backed up on
personal main andcodex/p0-m8-version-backup,withtagengineering/m8-recovered-p0.
Authentication401 was resolved by user-completed GitHub browser OAuth;Capt-cai
hasADMIN on the personal fork. A fresh independent GitHub clone recovered every
141M8input,all10archives and the exact user binary. Documentation follow-up
commits do not alter the immutable code checkpoint. No accepted/stable M8 release
was created. Current details:tools/versioning/P0-REPORT.json.
