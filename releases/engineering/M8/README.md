# M8 engineering checkpoint — NOT an accepted/stable release

ECEN21.g3a:158376bytes,SHA256
`a893609cb6a6757a5b20dbd253495339f3d9c2e6e1a5a13fa7c2ce01480824df`.

This is the actual freshly built/emulator-verified M8 user program,archived in
Git so it survives loss of local ignored build folders. The ZIP contains17actual
production-emulator Verilog exports,byte-compared and host-compiled/simulated.
Verify both files using SHA256SUMS. User/physical acceptance remains pending.

Tests and limits: ../../../TESTING.md.
Chinese installation/acceptance: ../../../docs/M8-USER-ACCEPTANCE.zh-CN.md.
Original engineering recovery bundle: ../../../version-archives/m8.tar.gz.
Do not replace these files silently;create a new engineering checkpoint/commit
for an acceptance fix. Mark Accepted Release only after explicit user acceptance
of that exact artifact. A new milestone approval is not acceptance.
