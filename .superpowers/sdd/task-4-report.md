# Task 4 Report: RenderThread loop without D3D (TDD)

## Status

**DONE**

## Summary

Added a `std::thread` `RenderThread` that drains `CommandQueue`, exits on `CmdStop`, otherwise `consume()`s the snapshot and sleeps 1 ms. No D3D, no `QThread`. Tests were written first (RED: missing `RenderThread.h`), then the implementation was added (GREEN: CTest 1/1 passed in 0.05 s). Local commit only.

## Files Created

| File | Purpose |
|------|---------|
| `src/render/RenderThread.h` | `start` / `requestStopAndJoin` / queue accessors |
| `src/render/RenderThread.cpp` | `std::thread` loop; join only if `joinable()` |

## Files Modified

| File | Change |
|------|--------|
| `tests/test_queues.cpp` | Added `runRenderThreadTests()` (start, push `CmdStop`, join) |
| `tests/test_main.cpp` | Declares and calls `runRenderThreadTests()` after queue tests |
| `CMakeLists.txt` | Added `src/render/RenderThread.cpp` to `3deditmax_tests` |

## Files Not Modified

- App target `3DEditMax` — still does not link render sources (same as Task 3).
- Queue / snapshot implementations — consumed as-is.

## TDD Evidence

### RED (Step 1–2)

Wrote `runRenderThreadTests()` and wired it from `test_main.cpp` **before** `RenderThread.h` / `.cpp` existed. Built with MSVC.

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build
```

**Result:** compile failed (exit 1).

```
E:\code\private\3DEditMax\tests\test_queues.cpp(4): fatal error C1083:
  cannot open include file: "render/RenderThread.h": No such file or directory
ninja: build stopped: subcommand failed.
```

Failure reason: feature missing (header not present), not a test typo.

Brief Step 2 expected a missing `RenderThread` build error. Actual RED was **C1083** because the production header was not written yet (correct TDD order). A link error would have required shipping the header first.

### GREEN (Step 3–4)

Implemented `RenderThread.h` / `.cpp` verbatim from the brief and added `RenderThread.cpp` to `3deditmax_tests`.

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
| Returned within ~1 s (no hang) | PASS |

`requestStopAndJoin` is safe if `CmdStop` already ended the thread: it returns immediately when `!m_thread.joinable()`, otherwise pushes `CmdStop` and `join()`s. A finished-but-not-yet-joined thread remains `joinable()`, so the extra `CmdStop` is harmless and `join()` reaps it. The destructor calls `requestStopAndJoin`, so a second call after the test already joined is a no-op.

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `vcvars64.bat`)
- Generator: existing `build/` Ninja Debug (not Clang)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64` (needed for CMake reconfigure of the app target)

## Commit

| SHA | Subject |
|-----|---------|
| `4628946` | Add std::thread render loop that stops on CmdStop. |

Local commit only; not pushed (per task constraints). Not amended.

## Self-Review

### Requirements coverage

- [x] `RenderThread()` / `start()` / `requestStopAndJoin()` / accessors verbatim
- [x] Loop pops all commands; `CmdStop` exits; else `consume()` + 1 ms sleep
- [x] `std::thread` only — no `QThread` / D3D
- [x] `requestStopAndJoin` joins only if `joinable()`
- [x] `runRenderThreadTests` as specified
- [x] TDD: watched RED, then GREEN
- [x] C++11; no `std::make_unique` / `std::optional`

### Concerns

1. **App target does not link `RenderThread` yet.** Only `3deditmax_tests` compiles it. Viewport / HWND ownership comes later.
2. **`m_running` is unused by the loop.** The loop exits only on `CmdStop`; the flag is set in `start` / cleared after join. Fine for this task; later GPU work may use it.
3. **Test assertion is `TEST_CHECK(true)`.** It proves the loop returns (no hang) rather than inspecting feedback or snapshots.
4. **Unused test includes** `<chrono>` / `<thread>` match the brief; they are not used by the test body.
5. **No concurrent stress beyond start/stop.** Queue mutexes already exist from Task 3.

### Deviations from brief

- Step 2 expected a missing-`RenderThread` error; RED was C1083 missing header (TDD-correct).
- Step 5 `git push origin main` omitted (task + global constraints: local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

`RenderThread` is ready for Task 5 to own `D3D11Renderer` and handle Init/Resize/ReloadShader.
