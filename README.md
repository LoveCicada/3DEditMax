# 3DEditMax

C++11 / Qt 5.15.2 / Direct3D 11 实验台：用真实 DX11 视口学习 World / View / Projection 矩阵，并逐步观察 Device、Shader、Constant Buffer 与光栅状态。

第一期不做 3ds Max 式编辑器。设计见 [`docs/superpowers/specs/2026-08-16-dx11-lab-teach-design.md`](docs/superpowers/specs/2026-08-16-dx11-lab-teach-design.md)，实现计划见 [`docs/superpowers/plans/2026-08-16-dx11-lab-teach.md`](docs/superpowers/plans/2026-08-16-dx11-lab-teach.md)，界面示意图见 [`docs/ui/`](docs/ui/)。

## 构建（实现阶段）

Windows + MSVC x64。先 `call vcvars64.bat`（默认 shell 的 Clang 链不上这套 Qt），再设 `QT_DIR` 指向 Qt 5.15.2 kit（含 `lib\cmake\Qt5`）。

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

运行 `build\3DEditMax.exe` 时把 `%QT_DIR%\bin` 加入 `PATH`。

## 许可

私人学习项目。
