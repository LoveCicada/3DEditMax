# Pan/Layout Task 4 — Verify

## Automated
- `cmake --build build` — ninja: no work to do
- `ctest --test-dir build --output-on-failure --timeout 30` — **PASS** (1/1)

## Checklist

| Item | Status |
|------|--------|
| Drag: no teachingEdited until release (Task 1) | Present (`826adae`) |
| camTarget + BuildView + RMB pan (Task 2) | Present (`bd9396f`) |
| Dock defaults + collapsible groups (Task 3 + fix) | Present (`15e6959`, `1846d28`) |
| Interactive orbit/pan smoke | Deferred to human |

## Commits on feat/viewport-pan-layout (after plan commit 45ba068)

1. `826adae` Stop matrix board refresh during viewport drag…
2. `bd9396f` Add camera target and right-button pan…
3. `15e6959` Loosen lab shell docks…
4. `1846d28` Fix TransformPanel collapsible groups…

No extra README commit required.
