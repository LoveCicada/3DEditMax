### Task 1: Stop drag flicker (UI sync only on release)

Plan: docs/superpowers/plans/2026-08-16-viewport-pan-layout.md Task 1

- Drag: publishState only; NO teachingEdited during drag
- Release: emit teachingEdited once
- Keep no-op resize + matrix min height
- Commit: Stop matrix board refresh during viewport drag to prevent flicker.
- Local commit only; no push
- C++11; do not change render-thread / teaching state machine
