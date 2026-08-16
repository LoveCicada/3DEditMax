# Task 13 Report: Tutorial, demo, JSON (Phase 3)

## Status

**DONE**

## Summary

Eight tutorial steps (`tutorialStepCount()==8`) apply distinctive `TeachingState` values. Bottom `TutorialPanel` has prev/next/Demo. File menu 导入/导出 round-trips teaching JSON with a minimal writer/parser (no third-party lib). `DemoPlayer` lerps `camYawDeg` 0→90→0 over 8s; MainWindow drives it with a 16ms `QTimer` while `demoPlaying`. Local commit only; not pushed.

## Files Created

| File | Change |
|------|--------|
| `src/teach/TutorialScript.h/.cpp` | `TutorialStep`, `tutorialStepCount()`, `tutorialStepAt(0..7)` |
| `src/teach/DemoPlayer.h/.cpp` | `start` / `tick`; 8s yaw 0→90→0 |
| `src/teach/JsonIo.h/.cpp` | `teachingToJson` / `teachingFromJson`; required-field reject |
| `src/ui/TutorialPanel.h/.cpp` | Prev/next/Demo; `applyState(TeachingState)`; Qt only |
| `tests/test_tutorial.cpp` | Count==8; World yaw=35; View pitch=40; demo lerp |
| `tests/test_json.cpp` | Default-state round-trip `camDistance` and `objects[0].trs.pos[0]` |

## Files Modified

| File | Change |
|------|--------|
| `src/app/MainWindow.h/.cpp` | Bottom 教程 dock; File 导入/导出; 16ms demo timer; `syncTeaching()` |
| `tests/test_main.cpp` | `runTutorialTests()` + `runJsonTests()` |
| `CMakeLists.txt` | New sources on app + tests; tests `/utf-8` |

## TDD Evidence

### RED

Stubs: `tutorialStepCount()` returned 0; `teachingToJson` returned false; `DemoPlayer::tick` returned false.

```
FAIL E:\code\private\3DEditMax\tests\test_tutorial.cpp:11: tutorialStepCount() == 8
FAIL E:\code\private\3DEditMax\tests\test_tutorial.cpp:18: nearf(world.state.objects[0].trs.yawDeg, 35.f)
FAIL E:\code\private\3DEditMax\tests\test_tutorial.cpp:22: nearf(view.state.camPitchDeg, 40.f)
FAIL E:\code\private\3DEditMax\tests\test_tutorial.cpp:29: demo.tick(0.f, &io) == true
FAIL E:\code\private\3DEditMax\tests\test_json.cpp:13: teachingToJson(s, &json) == true
FAIL E:\code\private\3DEditMax\tests\test_json.cpp:18: nearf(out.camDistance, s.camDistance)
FAIL E:\code\private\3DEditMax\tests\test_json.cpp:19: nearf(out.objects[0].trs.pos[0], s.objects[0].trs.pos[0])
```

Failure reason: feature missing, not a typo.

### GREEN

Implemented `TutorialScript`, `JsonIo`, `DemoPlayer`. Rebuilt tests + app.

```
cmake --build build --target 3deditmax_tests 3DEditMax
...
[13/13] Linking CXX executable 3DEditMax.exe
1/1 Test #1: 3deditmax_tests ..................   Passed    0.02 sec
100% tests passed, 0 tests failed out of 1
```

## Build & Test

- Branch: `feat/dx11-lab-teach`
- MSVC 14.33 via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- `cmake --build build --target 3deditmax_tests 3DEditMax` exit 0
- CTest `3deditmax_tests` **Passed** (1/1)

## Commit

| SHA | Subject |
|-----|---------|
| `55958d2` | Add tutorial steps, demo playback, and teaching JSON. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] `tutorialStepCount()==8`; steps: 坐标空间, World, View, Projection, MVP, 近远平面, 三物体对照, DX11 CB 对照
- [x] World yaw=35; View camPitch=40
- [x] JSON keys as specified; missing required field → `false`
- [x] Default-state round-trip `camDistance` and `objects[0].trs.pos[0]`
- [x] Minimal writer/parser, no third-party lib
- [x] `DemoPlayer` 8s, lerp `camYawDeg` 0→90 and back
- [x] `TutorialPanel` prev/next/Demo, `applyState`; no windows.h / D3D
- [x] File menu 导入/导出 via `QFileDialog` UTF-8 + `JsonIo`
- [x] MainWindow 16ms `QTimer` while `demoPlaying`
- [x] Chinese via `QString::fromUtf8`
- [x] C++11
- [x] Local commit only (no push)

### Concerns

1. **Manual Step 5 not run in-session.** Script clicks, Demo orbit, and File JSON were not exercised in the running app.
2. **Demo starts from current teaching, not the step snapshot.** Demo button only sets `demoPlaying`; MainWindow calls `start(m_teaching)` so orbit/panel edits are kept.
3. **JSON omits `tutorialStep` / `demoPlaying`.** Matches the key list; import does not move the tutorial index.
4. **Parser requires exactly 3 objects** and all listed fields. Extra keys are skipped.

### Deviations from brief

- Extra DemoPlayer assertions (t=0/4/8 yaw) beyond the mandated count/JSON checks.
- Tests target also gets `/utf-8`.
- Step 6 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

Tutorial, demo, and JSON are on `TeachingState`. Task 14 can add LabPanel rasterizer/depth/shader controls without changing the script/JSON path.
