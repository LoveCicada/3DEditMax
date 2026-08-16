# Task 15 Report: Phase 1–4 acceptance

## Status

**DONE_WITH_CONCERNS**

## Summary

Rebuilt the existing Ninja Debug tree with MSVC `vcvars64` and ran `ctest --test-dir build --output-on-failure`: **1/1 PASS**. Short exe smoke: `3DEditMax` window appeared, `CloseMainWindow` joined with no leftover process. PIX / interactive visual items were not run by a human; those checklist rows are SKIPPED (logic covered by CTest where a unit test exists). README build block was Release-only and omitted `vcvars64` + CTest; aligned to the finished Debug lab-shell commands. Local commit only; not pushed.

## Checklist

| # | Item | Result | Evidence / why |
|---|------|--------|----------------|
| CTest | `ctest --test-dir build --output-on-failure` | **PASS** | `3deditmax_tests` 1/1, 0.02 s, 0 failed. |
| 1 | Column-major board vs PIX `worldViewProj` (or TrackPoint NDC vs screen) | **PARTIAL** | CTest: `formatMatrix4(MajorColumn)` puts translation in the last column; `TrackPoint` identity NDC matches model. **SKIPPED:** PIX/Graphics Debugger CB dump and on-screen pixel vs NDC. |
| 2 | JSON save/load restores TRS and camera | **PASS** (unit) | `teachingToJson` / `teachingFromJson` round-trip `camDistance` and `objects[0].trs.pos[0]`; empty/`{}` rejected. **SKIPPED:** File-dialog UI click. |
| 3 | Tutorial 8 steps all apply | **PASS** (unit) | `tutorialStepCount() == 8`; step 1 yaw 35°; step 2 pitch 40°. UI `TutorialPanel::applyState` → `onTutorialApply` exists. **SKIPPED:** Clicking all 8 Apply buttons in the running app. |
| 4 | Demo starts/stops | **PASS** (unit) | `DemoPlayer::start` + `tick(0)` stays playing; `tick(4)` yaw 90°; next `tick(4)` returns false and yaw 0°. **SKIPPED:** Toolbar start/stop in the GUI. |
| 5 | Three objects + four shadings | **PARTIAL** | CTest: cube 36 indices; sphere/cylinder builders non-empty. Defaults have 3 object slots; `LayoutThree` + `ShadeSolid/Normal/Checker/Wire` exist. **SKIPPED:** Visual three-mesh / four-shade pass. |
| 6 | Frustum tracks near/far | **SKIPPED** (visual) | `BuildProjection` uses `nearZ`/`farZ`; `DebugDraw` unprojects LH clip corners through current P. No CTest for edge motion; not clicked in-app. |
| 7 | Lab fill/cull/depth | **PARTIAL** | CTest: defaults Solid/Back/depth-on/`unlit`; `labEffectiveFillMode` wire override; cull clamp; shadeX from variant vs teaching. **SKIPPED:** Combo clicks in the lab dock. |
| 8 | Close joins the render thread | **PASS** | CTest: `RenderThread::start` + `CmdStop` + `requestStopAndJoin` returns. Smoke: window title `3DEditMax`, `CloseMainWindow` true, process gone (`LEFTOVER=none`). Viewport dtor/`stopRenderer` calls `requestStopAndJoin`. |

## Files Modified

| File | Change |
|------|--------|
| `README.md` | Build block: `vcvars64`, Ninja Debug, `ctest`; note Clang vs MSVC Qt; `%QT_DIR%\bin` on PATH to run the exe |

## Files Not Modified

- App / render / tests — no acceptance bugs found that required a code fix
- Plan / spec — self-review table only

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- Generator: existing `build/` Ninja, `CMAKE_BUILD_TYPE=Debug`

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build
ctest --test-dir build --output-on-failure
```

```
[vcvarsall.bat] Environment initialized for: 'x64'
ninja: no work to do.
1/1 Test #1: 3deditmax_tests ..................   Passed    0.02 sec
100% tests passed, 0 tests failed out of 1
Total Test time (real) =   0.03 sec
```

**Exe smoke** (`build\3DEditMax.exe`, `PATH` += `%QT_DIR%\bin`, cwd `build`):

```
aliveAfterStart=True
title=3DEditMax
closeMainWindow=True
exitedClean=True
exitCode=3
LEFTOVER=none
```

## Commit

| SHA | Subject |
|-----|---------|
| `a357156` | Align README with the finished lab-shell build. |

Local commit only; not pushed. Not amended. `.superpowers/` remains untracked (report only).

## Self-Review

### Requirements coverage

- [x] Step 1 CTest all PASS (fresh this session)
- [x] Step 2 checklist documented pass / partial / skipped with evidence
- [x] Step 3 README updated because build instructions had drifted (Release, no vcvars, no ctest)
- [x] No empty commit; no push; no amend
- [x] No code fix required

### Concerns

1. **No PIX / human visual pass.** Items 1 (GPU CB dump), 5–7 (meshes, frustum silhouette, RS/DSS look) stay SKIPPED. Task 12 already noted same-camera frustum is mostly screen-edge lines.
2. **Smoke exit code 3.** Process exited and did not linger; Qt `exec()` return is not asserted as 0. First smoke attempt left PID 17992 (script error); it was killed before the successful run.
3. **JSON CTest does not assert every TRS/camera field** — only `camDistance` and object-0 `pos[0]`. Serializer writes the rest (`nearZ`/`farZ`/yaw/etc.).
4. **Tutorial CTest samples steps 0–2 only**, not apply of steps 3–7 (those states exist in `tutorialStepAt`).

### Deviations from brief

- Did not `git push origin main` (task constraint: local commit only, stay on `feat/dx11-lab-teach`).
- Manual Step 2 is CTest + short close-smoke, not a human PIX/UI walkthrough.

## Follow-up: four whole-branch Important findings

Local commit `c9e7dfc` (`Fix four whole-branch review findings.`). Not pushed. Not amended.

| # | Finding | Fix |
|---|---------|-----|
| 1 | Pump `ID3D11InfoQueue` | Debug-only `attachInfoQueue` after device create; `GetMessage`/`ClearStoredMessages` after init and each Present; map severity to `FbLog`/`FbWarn`/`FbError`. QI no-op without the debug layer. |
| 2 | Tracker vs MeshBuild cube | `teachingStateDefault().trackModel` and TrackerPanel presets are ±0.5. CTest: default point matches a cube vertex; identity-world `TrackPoint` is `(0.5,0.5,0.5)`. |
| 3 | Failed lab-variant compile every Present | `m_triedVariant` records the last requested name; compile once per name. Failure keeps last good PSO. `reloadShaders` clears tried so a file change retries. |
| 4 | Camera pitch spinbox | `TransformPanel` cam pitch range ±89 (orbit / Top preset already 89). Object pitch stays ±180. |

### Re-test (MSVC vcvars64)

- Branch: `feat/dx11-lab-teach`
- MSVC 14.33 via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- Incremental CTest first failed (`onCorner`) because stale test objs still had old inline `teachingStateDefault` (1,1,1). Clean-rebuild of `3deditmax_tests` then PASS. Also `--clean-first` `3DEditMax`.

```
1/1 Test #1: 3deditmax_tests ..................   Passed    0.07 sec
100% tests passed, 0 tests failed out of 1
Total Test time (real) =   0.08 sec
```
