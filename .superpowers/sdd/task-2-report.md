# Task 2 Report: State PODs and make_unique

## Status

**DONE**

## Summary

Added four header-only core state types: C++11 `make_unique`, teaching/lab POD defaults, and a combined `StateSnapshot`. Headers compile under C++11; existing CTest harness still passes. No CMake or test changes (next task will wire snapshot tests).

## Files Created

| File | Purpose |
|------|---------|
| `src/core/MakeUnique.h` | C++11 `make_unique` polyfill via `std::unique_ptr` + `new` |
| `src/core/TeachingState.h` | Enums, `TransformTRS`, `TeachingObject`, `TeachingState`, `transformIdentity()`, `teachingStateDefault()` |
| `src/core/LabState.h` | `LabState` POD + `labStateDefault()` (`"unlit"`, D3D11-ish fill/cull ints) |
| `src/core/StateSnapshot.h` | `StateSnapshot` + `stateSnapshotDefault()` (viewport 1×1) |

## Files Not Modified

- `CMakeLists.txt` — headers not listed as sources; no change required.
- `tests/test_main.cpp` — snapshot tests deferred to next task (per brief).

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (via `vcvars64.bat`)
- Standard: C++11 project setting; ad-hoc compile check used `/std:c++14` (still valid for C++11 code)

**Compile verification (ad-hoc, not committed)**

Temporary TU including all four headers + `stateSnapshotDefault()` / `make_unique<int>(42)` compiled and linked successfully.

**Existing CTest**

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

| Test | Result |
|------|--------|
| `3deditmax_tests` | **Passed** (1/1) |

No new runtime tests added (per brief Step 2).

## Commit

| SHA | Subject |
|-----|---------|
| `d5e269e` | Add teaching and lab state snapshots. |

Local commit only; not pushed (per task constraints).

## Self-Review

### Requirements coverage

- [x] `make_unique` template in `src/core/MakeUnique.h` (no `std::make_unique`)
- [x] `TeachingState` with `objects[3]`, enums, camera/projection fields, `teachingStateDefault()` defaults verbatim
- [x] `LabState` with `shaderVariant[32]`, fill/cull/depth/debug defaults verbatim
- [x] `StateSnapshot` struct + `stateSnapshotDefault()`
- [x] No D3D or Qt includes in these headers
- [x] No Scene/Node types
- [x] CMake unchanged

### Concerns

1. **Headers not in CMake graph:** Compile verified via ad-hoc TU only; CI will not compile them until a `.cpp` includes them (planned next task).
2. **No snapshot unit tests yet:** Defaults are untested at runtime until Task 3+ adds assertions.

### Deviations from brief

- Step 3 `git push origin main` omitted intentionally (task + global constraints: commit locally only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

State PODs and `make_unique` are ready for consumers (render loop, snapshot tests, UI bindings).
