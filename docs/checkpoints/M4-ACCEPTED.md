# M4 accepted stable checkpoint

Checkpoint ID: `M4-accepted`. Recorded November12,2026. Status: engineering
verification passed; user explicitly reported real **Casio fx-CG50** acceptance
passed and formally approved closure of M4. M5 implementation remains unapproved.
Phase1/M2A/M2B/M3 are also user-accepted; original evidence is preserved.

## Frozen program and provenance

- User program: [archived ECEN21.g3a](../../.phase1/milestones/M4/artifacts/ECEN21.g3a).
- Size: **93,804 bytes**.
- SHA-256: `165a5a2230b075352046e0f666b9b9d981d47b99c69af06443373fcadefcff6f`.
- Accepted build inputs: [77-file source manifest](../../.phase1/milestones/M4/source-manifest.json).
- Complete source copy: `.phase1/milestones/M4/build-source/` (app/common/exact engine).
- Original build/verifier helpers and delivery records: `.phase1/milestones/M4/reproduction/`.
- Build logs, ELF, package dumps,host/target reports,screenshots and immutable
  original engineering verification: `.phase1/milestones/M4/`.
- Acceptance report: [acceptance.json](../../.phase1/milestones/M4/acceptance.json).
- Portable local bundle: [M4-accepted.tar.gz](../../.phase1/checkpoints/M4-accepted.tar.gz).
- Machine-readable checkpoint and bundle hash: [M4-ACCEPTED.json](M4-ACCEPTED.json).

This is a **local archive checkpoint**, not a Git release/tag or remote backup.
The repository still has uncommitted/untracked project work. Git HEAD
`12d6b4e1086746534636c3bb87703deeef5734a8` identifies the upstream baseline only,
not the ECEN21 source snapshot. Hashes plus frozen build inputs are the checkpoint
authority. No Git commit/tag was invented or created. Generated `.phase1/`
archives remain ignored; retain the bundle separately if a remote backup is
wanted. No public release is implied (that belongs to M9).

## Preservation and restoration

The archive was created without rebuilding or changing the accepted binary.
All77 build-manifest hashes match current inputs, and the originalM2A/M2B/M3
artifacts remain intact. M3 computational/UI files are unchanged. New product
planning lives in documents only; noM5 implementation or architectural migration
was performed. The bundle contains the frozen milestone plus current planning
records; a member hash manifest permits integrity verification.

To reinstall on the calculator, use the archived user file with the existing
[M4 Chinese installation/acceptance guide](../M4-USER-ACCEPTANCE.zh-CN.md).
Do not install EC21Test.g3a. Keep future builds in new milestone folders instead
of overwriting this checkpoint. Reproduce source builds using the archived helper
with its frozen build-source inputs in a separate scratch checkout (copy the
archivedbuild-source apps/common/experiments into a scratch root and the archived
reproduction/scripts helpers into its scripts directory); the current
workspace's generic build alias will eventually build a newer source version.
The existing pinned ARM64 image/native emulator requirements still apply.

## Evidence limits

Acceptance is the user's hardware report, not agent-operated hardware testing.
No device OS version, per-step photos, numerical latency, peak stack/memory or
long-run benchmark was supplied; those remain unmeasured. Original reports may
say 'user acceptance pending' because they record the earlier engineering handoff;
`acceptance.json` and currentPLAN/PROGRESS/TESTING separately record later closure.
Historical verification.json/final-review.json are intentionally not rewritten.

## P0 remote-safe recovery mirror

Original accepted source/binary hashes are preserved in `version-archives/m4.tar.gz`
and its index. The byte-identical legacy bundle remains locally at its original
path;Finder.DS_Store metadata prevents uploading that raw package. Current remote
sync is recorded in tools/versioning/P0-REPORT.json,not inferred from the original
closure record.
