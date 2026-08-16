# Task 1 Report: CMake app skeleton

## Status

**DONE**

## Summary

Implemented the CMake/Qt 5.15.2 application skeleton with an empty `MainWindow` (1280×720, title `3DEditMax`) and a CTest harness with a passing dummy check. Configured, built, and tested successfully on Windows with MSVC x64 + Ninja.

## Files Created

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | C++11 project; Qt5 Widgets via `QT_DIR`; targets `3DEditMax` and `3deditmax_tests` |
| `src/main.cpp` | `QApplication` entry; shows `MainWindow` |
| `src/app/MainWindow.h` | `QMainWindow` subclass with `Q_OBJECT` |
| `src/app/MainWindow.cpp` | Sets title and 1280×720 size |
| `tests/test_harness.h` | `TEST_CHECK` macro; `testFailureCount()` (no C++17 inline vars) |
| `tests/test_main.cpp` | Harness runner with `TEST_CHECK(1 == 1)` |

## Files Not Modified

- `README.md` — build block already matches Ninja generator; no change needed.

## TDD / Harness

Step 2 (harness) was implemented before Step 3 (app). The test target builds independently of Qt and runs under CTest.

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64\bin` (CMake stripped `bin` → kit root)
- Generator: Ninja, `CMAKE_BUILD_TYPE=Debug`
- Compiler: MSVC 14.33 (via `vcvars64.bat`); default shell Clang cannot link MSVC Qt libs

**Commands**

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64\bin
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build -C Debug --output-on-failure
```

**Results**

| Artifact / Test | Result |
|-----------------|--------|
| Configure (Qt5 Widgets found) | PASS |
| `build/3DEditMax.exe` | Built |
| `build/3deditmax_tests.exe` | Built |
| CTest `3deditmax_tests` | **Passed** (1/1, 0.11 s) |

**Manual GUI check:** Not performed in this session (headless agent). Window behavior matches spec by code review (`setWindowTitle`, `resize(1280, 720)`).

## Commit

| SHA | Subject |
|-----|---------|
| `8f499a0` | Add CMake Qt skeleton and CTest harness. |

Local commit only; not pushed (per task constraints).

## Self-Review

### Requirements coverage

- [x] `CMAKE_CXX_STANDARD 11`, extensions OFF
- [x] Qt 5.15 Widgets via `QT_DIR` with trailing-slash and `bin` stripping
- [x] `MainWindow` constructor `explicit MainWindow(QWidget* parent = 0)`
- [x] Executable target `3DEditMax`
- [x] CTest target `3deditmax_tests` with harness (C++11-safe counter)
- [x] No `std::make_unique` / `std::optional`

### Concerns

1. **MSVC required:** CMake in the default dev shell picks Clang; linking Qt fails unless `vcvars64.bat` is invoked first. README could note this in a later task.
2. **GUI not visually verified:** Executable builds; manual launch recommended to confirm window title/size.

### Deviations from brief

- Step 5 `git push` omitted intentionally (task + global constraints: commit locally only).

## Next Task Readiness

Empty window + CTest harness are in place. Task 2+ can add render/UI on top of this skeleton.
