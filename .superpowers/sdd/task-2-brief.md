### Task 2: State PODs and make_unique

**Files:**
- Create: `src/core/MakeUnique.h`
- Create: `src/core/TeachingState.h`
- Create: `src/core/LabState.h`
- Create: `src/core/StateSnapshot.h`

**Interfaces:**
- Consumes: none
- Produces:
  - `template<typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&...)`
  - `TeachingState teachingStateDefault()`
  - `LabState labStateDefault()`
  - `struct StateSnapshot { TeachingState teaching; LabState lab; int viewportW; int viewportH; }`

- [ ] **Step 1: Write headers**

`src/core/MakeUnique.h`:

```cpp
#pragma once
#include <memory>
#include <utility>

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

`src/core/TeachingState.h`:

```cpp
#pragma once

enum MeshId {
  MeshCube = 0,
  MeshSphere = 1,
  MeshCylinder = 2
};

enum LayoutMode {
  LayoutOne = 0,
  LayoutThree = 1
};

enum ShadingMode {
  ShadeSolid = 0,
  ShadeNormal = 1,
  ShadeChecker = 2,
  ShadeWire = 3
};

enum ProjMode {
  ProjPerspective = 0,
  ProjOrtho = 1
};

struct TransformTRS {
  float pos[3];
  float pitchDeg;
  float yawDeg;
  float rollDeg;
  float scale[3];
};

struct TeachingObject {
  MeshId mesh;
  TransformTRS trs;
};

struct TeachingState {
  TeachingObject objects[3];
  LayoutMode layout;
  ShadingMode shading;
  float camDistance;
  float camPitchDeg;
  float camYawDeg;
  ProjMode proj;
  float fovDeg;
  float aspect;
  bool aspectFollowViewport;
  float nearZ;
  float farZ;
  float trackModel[3];
  int tutorialStep;
  bool demoPlaying;
};

inline TransformTRS transformIdentity() {
  TransformTRS t;
  t.pos[0] = 0.f; t.pos[1] = 0.f; t.pos[2] = 0.f;
  t.pitchDeg = 0.f; t.yawDeg = 0.f; t.rollDeg = 0.f;
  t.scale[0] = 1.f; t.scale[1] = 1.f; t.scale[2] = 1.f;
  return t;
}

inline TeachingState teachingStateDefault() {
  TeachingState s;
  for (int i = 0; i < 3; ++i) {
    s.objects[i].mesh = MeshCube;
    s.objects[i].trs = transformIdentity();
    s.objects[i].trs.pos[0] = (i - 1) * 2.5f;
  }
  s.layout = LayoutOne;
  s.shading = ShadeSolid;
  s.camDistance = 5.f;
  s.camPitchDeg = 20.f;
  s.camYawDeg = 45.f;
  s.proj = ProjPerspective;
  s.fovDeg = 60.f;
  s.aspect = 16.f / 9.f;
  s.aspectFollowViewport = true;
  s.nearZ = 0.1f;
  s.farZ = 100.f;
  s.trackModel[0] = 1.f;
  s.trackModel[1] = 1.f;
  s.trackModel[2] = 1.f;
  s.tutorialStep = 0;
  s.demoPlaying = false;
  return s;
}
```

`src/core/LabState.h`:

```cpp
#pragma once
#include <cstring>

struct LabState {
  char shaderVariant[32];
  int fillMode;
  int cullMode;
  bool depthEnable;
  bool wantDebugLayer;
};

inline LabState labStateDefault() {
  LabState s;
  std::memset(s.shaderVariant, 0, sizeof(s.shaderVariant));
  const char* name = "unlit";
  for (int i = 0; name[i] && i < 31; ++i) {
    s.shaderVariant[i] = name[i];
  }
  s.fillMode = 3;  /* D3D11_FILL_SOLID */
  s.cullMode = 3;  /* D3D11_CULL_BACK */
  s.depthEnable = true;
  s.wantDebugLayer = true;
  return s;
}
```

`src/core/StateSnapshot.h`:

```cpp
#pragma once
#include "core/TeachingState.h"
#include "core/LabState.h"

struct StateSnapshot {
  TeachingState teaching;
  LabState lab;
  int viewportW;
  int viewportH;
};

inline StateSnapshot stateSnapshotDefault() {
  StateSnapshot s;
  s.teaching = teachingStateDefault();
  s.lab = labStateDefault();
  s.viewportW = 1;
  s.viewportH = 1;
  return s;
}
```

- [ ] **Step 2: No runtime test required beyond compile** 鈥?next task includes snapshot in tests.

- [ ] **Step 3: Commit and push**

```bash
git add src/core/MakeUnique.h src/core/TeachingState.h src/core/LabState.h src/core/StateSnapshot.h
git commit -m "Add teaching and lab state snapshots."
git push origin main
```

---

