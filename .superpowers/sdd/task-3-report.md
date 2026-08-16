# Task 3 Report: Queues and snapshot buffer (TDD)

## Status

**DONE**

## Summary

Added mutex-protected `CommandQueue` and `FeedbackQueue`, plus a double-buffered `SnapshotBuffer`. Tests were written first (RED: missing `CommandQueue.h`), then implementations were added (GREEN: CTest 1/1 passed). Local commit only.

## Files Created

| File | Purpose |
|------|---------|
| `src/render/CommandQueue.h` | `CommandType`, `RenderCommand`, `CommandQueue` |
| `src/render/CommandQueue.cpp` | Mutex `push` / `tryPop` |
| `src/render/FeedbackQueue.h` | `FeedbackKind`, `FeedbackItem`, `FeedbackQueue` |
| `src/render/FeedbackQueue.cpp` | Mutex `push` / `tryPop` |
| `src/core/SnapshotBuffer.h` | Double-buffer `publish` / `consume` |
| `src/core/SnapshotBuffer.cpp` | Dirty-flag swap of `StateSnapshot` |
| `tests/test_queues.cpp` | `runQueueTests()` (empty pop, stop cmd, error feedback, snapshot fields) |

## Files Modified

| File | Change |
|------|--------|
| `CMakeLists.txt` | Added `test_queues.cpp` + three impl `.cpp` files to `3deditmax_tests` |
| `tests/test_main.cpp` | Declares and calls `runQueueTests()` before the failure check |

## Files Not Modified

- App target `3DEditMax` — brief says add impl sources to tests and later the app.
- Core POD headers from Task 2 — consumed as-is.

## TDD Evidence

### RED (Step 1–2)

Wrote `tests/test_queues.cpp` and wired `runQueueTests()` **before** any queue/buffer sources existed. Added only `tests/test_queues.cpp` to `3deditmax_tests`, then built with MSVC.

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build
```

**Result:** compile failed (exit 1).

```
E:\code\private\3DEditMax\tests\test_queues.cpp(2): fatal error C1083:
  cannot open include file: "render/CommandQueue.h": No such file or directory
ninja: build stopped: subcommand failed.
```

Failure reason: feature missing (headers/types not present), not a test typo.

Brief Step 2 expected a **link** error (`CommandQueue` / `SnapshotBuffer` not found). Actual RED was **C1083** because production headers were not written yet (correct TDD order). A link error would have required shipping headers first.

### GREEN (Step 3–4)

Implemented the six sources verbatim from the brief and added the three impl `.cpp` files to `3deditmax_tests`.

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build
ctest --test-dir build --output-on-failure
```

**Result:**

| Artifact / Test | Result |
|-----------------|--------|
| `3deditmax_tests.exe` linked | PASS |
| CTest `3deditmax_tests` | **Passed** (1/1, 0.05 s) |
| `100% tests passed, 0 tests failed out of 1` | PASS |

Existing dummy `TEST_CHECK(1 == 1)` still runs first.

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `vcvars64.bat`)
- Generator: existing `build/` Ninja Debug (not Clang)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64` (needed for CMake reconfigure of the app target)

Reconfigure with `-G "Ninja"` was skipped after a quoting failure under `cmd /c`; the existing Ninja Debug tree was reused and CMake re-ran from the changed `CMakeLists.txt`.

## Commit

| SHA | Subject |
|-----|---------|
| `7c025b9` | Add thread-safe command, feedback, and snapshot queues. |

Local commit only; not pushed (per task constraints). Not amended.

## Self-Review

### Requirements coverage

- [x] `CommandType` / `RenderCommand` / `CommandQueue::push` / `tryPop` verbatim
- [x] `FeedbackKind` / `FeedbackItem` / `FeedbackQueue::push` / `tryPop` verbatim
- [x] `SnapshotBuffer::publish` / `consume` double-buffer with `m_dirty`
- [x] Consumes `StateSnapshot` / `stateSnapshotDefault()`
- [x] Tests cover empty pop, `CmdStop`, `FbError` + `"boom"`, `camDistance == 9.f`, `viewportW == 640`
- [x] New `.cpp` files added to `3deditmax_tests`
- [x] TDD: watched RED, then GREEN
- [x] C++11, mutex queues, no `std::make_unique` / `std::optional`

### Concerns

1. **App target does not link these sources yet.** Only `3deditmax_tests` compiles them. A later render-thread task must add them to `3DEditMax`.
2. **`CommandQueue.h` includes `<windows.h>`.** Fine for `src/render`; UI panels (except the viewport) must still avoid it.
3. **`tryPop` does not null-check `out`.** Matches the brief; callers must pass a valid pointer.
4. **No concurrent stress tests.** Mutex is present; tests are single-threaded.
5. **`SnapshotBuffer::consume` returns the last front if never published.** Constructor seeds both sides with `stateSnapshotDefault()`; tests always publish first.

### Deviations from brief

- Step 2 expected a link error; RED was C1083 missing header (TDD-correct).
- Step 4 `cmake -S . -B build -G "Ninja"` quoting failed under nested `cmd /c`; reused existing Ninja Debug `build/`.
- Step 5 `git push origin main` omitted (task + global constraints: local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

Queues and snapshot buffer are ready for `RenderThread` / UI poll consumers.
