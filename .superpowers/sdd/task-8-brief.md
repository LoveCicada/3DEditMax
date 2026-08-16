### Task 8: Transform unit tests (Phase 1 math)

**Files:**
- Create: `tests/test_transforms.cpp`
- Create: `src/teach/TrackPoint.h`  鈥?keep tracker types next to transforms
- Modify: `src/teach/Transforms.h` add `TrackResult TrackPoint(...)`
- Modify: `src/teach/Transforms.cpp`
- Modify: `tests/test_main.cpp`
- Modify: `CMakeLists.txt` 鈥?tests link nothing extra if Transforms.cpp already listed; add `test_transforms.cpp`

**Interfaces:**
- Produces:
  - `struct TrackResult { DirectX::XMFLOAT4 model, world, view, clip, ndc; };`
  - `TrackResult TrackPoint(DirectX::XMFLOAT3 pModel, DirectX::FXMMATRIX w, DirectX::FXMMATRIX v, DirectX::FXMMATRIX p)`

- [ ] **Step 1: Failing tests**

```cpp
#include "test_harness.h"
#include "teach/Transforms.h"
#include <DirectXMath.h>
#include <cmath>

static bool near4(float a, float b) {
  return fabsf(a - b) < 1e-4f;
}

void runTransformTests() {
  TransformTRS id = transformIdentity();
  DirectX::XMMATRIX w = BuildWorld(id);
  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, w);
  TEST_CHECK(near4(m._11, 1.f) && near4(m._22, 1.f) && near4(m._33, 1.f) && near4(m._44, 1.f));

  TransformTRS t = transformIdentity();
  t.pos[0] = 2.f;
  DirectX::XMStoreFloat4x4(&m, BuildWorld(t));
  TEST_CHECK(near4(m._41, 2.f)); /* row-major store: translation in _41,_42,_43 */

  TeachingState s = teachingStateDefault();
  DirectX::XMMATRIX P = BuildProjection(s, 1.f);
  (void)P;
  TEST_CHECK(s.nearZ < s.farZ);

  DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
  TrackResult tr = TrackPoint(DirectX::XMFLOAT3(1.f, 2.f, 3.f), I, I, I);
  TEST_CHECK(near4(tr.world.x, 1.f) && near4(tr.world.y, 2.f) && near4(tr.world.z, 3.f));
  TEST_CHECK(near4(tr.ndc.x, 1.f) && near4(tr.ndc.w, 1.f));
}
```

Note: `XMStoreFloat4x4` is row-major in memory as `_11.._14` first row; DirectXMath documentation: `_41,_42,_43` is the translation row of the stored float4x4. The test above matches that store layout. Display code in Task 9 must document this vs GPU column-major: formatters print either `_11,_21,_31,_41` as first displayed column (column-major view) or `_11,_12,_13,_14` as first row (row-major view).

- [ ] **Step 2: Run 鈥?FAIL on `TrackPoint` missing**

- [ ] **Step 3: Implement TrackPoint**

```cpp
struct TrackResult {
  DirectX::XMFLOAT4 model;
  DirectX::XMFLOAT4 world;
  DirectX::XMFLOAT4 view;
  DirectX::XMFLOAT4 clip;
  DirectX::XMFLOAT4 ndc;
};

inline TrackResult TrackPoint(DirectX::XMFLOAT3 pModel,
                              DirectX::FXMMATRIX w,
                              DirectX::FXMMATRIX v,
                              DirectX::FXMMATRIX p) {
  using namespace DirectX;
  TrackResult r;
  const XMVECTOR m = XMVectorSet(pModel.x, pModel.y, pModel.z, 1.f);
  const XMVECTOR wo = XMVector4Transform(m, w);
  const XMVECTOR vi = XMVector4Transform(wo, v);
  const XMVECTOR cl = XMVector4Transform(vi, p);
  XMStoreFloat4(&r.model, m);
  XMStoreFloat4(&r.world, wo);
  XMStoreFloat4(&r.view, vi);
  XMStoreFloat4(&r.clip, cl);
  const float cw = XMVectorGetW(cl);
  XMVECTOR nd = cl;
  if (fabsf(cw) > 1e-8f) {
    nd = XMVectorScale(cl, 1.f / cw);
  }
  XMStoreFloat4(&r.ndc, nd);
  return r;
}
```

Put `TrackResult` / `TrackPoint` in `Transforms.h` (no extra header required if you prefer one file; do not create a second unused header).

- [ ] **Step 4: CTest PASS**

- [ ] **Step 5: Commit and push**

```bash
git add src/teach/Transforms.h src/teach/Transforms.cpp tests/test_transforms.cpp tests/test_main.cpp CMakeLists.txt
git commit -m "Test world/view/projection builders and point tracking."
git push origin main
```

---

