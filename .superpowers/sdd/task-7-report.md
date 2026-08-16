# Task 7 Report: Phase 0 acceptance

## Status

**DONE** (checklist run in this environment; two bugs fixed and committed)

## Summary

Ran the Phase 0 manual checklist against a Debug `build/3DEditMax.exe` on this machine (screenshot + UI Automation + process check). Fixed View-menu mojibake and a missing-cube WVP upload bug. Local commit only.

## Checklist

| # | Item | Result | Evidence / why |
|---|------|--------|----------------|
| 1 | Debug build starts without a message box crash | **PASS** | Process `3DEditMax` created a main HWND titled `3DEditMax`; no owned `#32770` dialog. |
| 2 | Central client area is D3D clear color, not Qt gray | **PASS** | Viewport samples RGB(20,25,36) ≈ clear `{0.08, 0.10, 0.14}`; no Qt gray (180+). Screenshot `build/phase0_check/shot_initial.png`. |
| 3 | Cube visible, not a full-screen triangle glitch | **PASS** (after fix) | First run: sliver triangle (HLSL column-major vs row-major `XMMATRIX`). After `XMMatrixTranspose` on FrameCB upload: three-face shaded cube on dark clear. Pixel hit RGB(172,89,172) = −Y unlit tint. |
| 4 | Debug log shows device ready; Warn if debug runtime missing | **PASS** | Dock text `[Log] D3D11 device ready`. No `[Warn] D3D11 debug runtime missing` (debug layer present). |
| 5 | Minimize then restore: present resumes | **PASS** | After `SW_MINIMIZE` / `SW_RESTORE`, process stayed up; post-restore shot still D3D clear + cube sample. `hideEvent` stops the thread; `showEvent` re-inits (log can show ready twice). |
| 6 | Closing the window leaves no leftover process | **PASS** | `WM_CLOSE` → `WaitForExit` exit code 0; `Get-Process 3DEditMax` empty. |

Skipped: none. Interactive Task Manager click was replaced by the same process query.

## Files Modified

| File | Change |
|------|--------|
| `src/app/MainWindow.cpp` | View menu `QString::fromUtf8("\xE8\xA7\x86\xE5\x9B\xBE")` (视图), source-charset safe |
| `CMakeLists.txt` | MSVC `/utf-8` on `3DEditMax` |
| `src/render/D3D11Renderer.cpp` | `XMMatrixTranspose` before `XMStoreFloat4x4` for W/V/P/WVP |

## Files Not Modified

- Viewport / dock / log / RenderThread — start, present, and join already satisfied the checklist
- `unlit.hlsl` — `mul(float4, float4x4)` kept; CPU upload now matches HLSL column-major

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- Exe: `build/3DEditMax.exe` with `build/shaders/unlit.hlsl`

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake -S . -B build
cmake --build build --target 3DEditMax 3deditmax_tests
ctest --test-dir build --output-on-failure
```

**Result:** link exit 0; CTest `3deditmax_tests` **Passed** (1/1, 0.02 s). GUI script: start, UIA menu/log, screenshot pixels, minimize/restore, clean exit.

## Commit

| SHA | Subject |
|-----|---------|
| `d6c7d19` | Fix View-menu UTF-8 and transpose WVP for HLSL. |

Local commit only; not pushed (per task constraints). Not amended.

## Self-Review

### Requirements coverage

- [x] Checklist items 1–6 executed with evidence
- [x] View menu displays 视图 (UIA name match), not 瑙嗗浘
- [x] Cube bug fixed in owning file (`D3D11Renderer.cpp`)
- [x] Menu encoding fixed in owning file (`MainWindow.cpp`) plus `/utf-8`
- [x] No push / no amend

### Concerns

1. **`hideEvent` still tears down D3D.** Minimize joins the render thread; restore `CmdInit`s again. Present resumes, but the dock can show a second `D3D11 device ready`. Fine for Phase 0; later hide/show churn may drop unread feedback.
2. **Default object 0 is at x = −2.5** (`teachingStateDefault` three-slot layout). Cube is left-of-center, not framed. Not a glitch.
3. **Plan comment was wrong.** Task 5 said `XMStoreFloat4x4` alone matches `mul(float4, float4x4)`. HLSL default column-major packing requires a CPU transpose (or `row_major` in the cbuffer).
4. **Checklist is automated, not a human at Task Manager.** Same signals (HWND, pixels, UIA log, process table).

### Deviations from brief

- Menu string uses UTF-8 hex escapes instead of a raw `"视图"` literal so MSVC source charset cannot mojibake it; `/utf-8` added as belt-and-suspenders.
- Cube fix was not in the brief; required for item 3.

## Next Task Readiness

Phase 0 window / thread / cube / log is accepted on this machine. Task 8 can start Phase 1 math tests.
