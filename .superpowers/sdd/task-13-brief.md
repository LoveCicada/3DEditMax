### Task 13: Tutorial, demo, JSON (Phase 3)

**Files:**
- Create: `src/teach/TutorialScript.h/.cpp`
- Create: `src/teach/DemoPlayer.h/.cpp`
- Create: `src/teach/JsonIo.h/.cpp`
- Create: `src/ui/TutorialPanel.h/.cpp`
- Create: `tests/test_tutorial.cpp`
- Create: `tests/test_json.cpp`
- Modify: `MainWindow` bottom dock, File menu 瀵煎叆/瀵煎嚭
- Modify: `CMakeLists.txt`, `tests/test_main.cpp`

**Interfaces:**
- `struct TutorialStep { const char* title; const char* body; TeachingState state; };`
- `int tutorialStepCount()` returns `8`
- `TutorialStep tutorialStepAt(int index)` 鈥?index 0..7
- Steps in order: 鍧愭爣绌洪棿, World, View, Projection, MVP, 杩戣繙骞抽潰, 涓夌墿浣撳鐓? DX11 CB 瀵圭収. Each step writes a distinctive `TeachingState` (e.g. step World sets yaw=35; step View sets camPitch=40).
- `bool teachingToJson(const TeachingState& s, std::string* out)`
- `bool teachingFromJson(const std::string& in, TeachingState* out)`
- JSON keys (ASCII): `layout`, `shading`, `camDistance`, `camPitchDeg`, `camYawDeg`, `proj`, `fovDeg`, `aspect`, `aspectFollowViewport`, `nearZ`, `farZ`, `trackModel`[3], `objects` array of `{mesh, pos, pitchDeg, yawDeg, rollDeg, scale}`
- Implement a minimal writer/parser (no third-party): writer is straightforward; parser accepts the exact shape the writer emits (whitespace optional). Reject unknown required-field absence with `false`.
- `DemoPlayer`: `void start(const TeachingState& from)`; `bool tick(float dtSec, TeachingState* io)` 鈥?8 seconds, lerp `camYawDeg` 0鈫?0 and back; returns false when finished
- `TutorialPanel`: prev/next, show title+body, Demo button; emits `applyState(TeachingState)`
- File menu: `QFileDialog` read/write UTF-8 json via `JsonIo`

- [ ] **Step 1: Tests** 鈥?`tutorialStepCount()==8`; JSON default state round-trip `camDistance` and `objects[0].trs.pos[0]`
- [ ] **Step 2: FAIL then implement TutorialScript + JsonIo**
- [ ] **Step 3: DemoPlayer + TutorialPanel + menus**
- [ ] **Step 4: MainWindow `QTimer` 16ms while `m_teaching.demoPlaying`** calls `tick`, publishes, stops when false
- [ ] **Step 5: CTest PASS + manual script/demo/json**
- [ ] **Step 6: Commit and push**

```bash
git add src/teach/TutorialScript.h src/teach/TutorialScript.cpp src/teach/DemoPlayer.h src/teach/DemoPlayer.cpp src/teach/JsonIo.h src/teach/JsonIo.cpp src/ui/TutorialPanel.h src/ui/TutorialPanel.cpp tests/test_tutorial.cpp tests/test_json.cpp tests/test_main.cpp src/app/MainWindow.h src/app/MainWindow.cpp CMakeLists.txt
git commit -m "Add tutorial steps, demo playback, and teaching JSON."
git push origin main
```

---

