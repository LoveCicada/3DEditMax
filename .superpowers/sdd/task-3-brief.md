### Task 3: Queues and snapshot buffer (TDD)

**Files:**
- Create: `src/render/CommandQueue.h`
- Create: `src/render/CommandQueue.cpp`
- Create: `src/render/FeedbackQueue.h`
- Create: `src/render/FeedbackQueue.cpp`
- Create: `src/core/SnapshotBuffer.h`
- Create: `src/core/SnapshotBuffer.cpp`
- Create: `tests/test_queues.cpp`
- Modify: `CMakeLists.txt` (add the new cpp files to `3deditmax_tests` and later the app)
- Modify: `tests/test_main.cpp` (call `runQueueTests`)

**Interfaces:**
- Consumes: `StateSnapshot`
- Produces:
  - `enum CommandType { CmdInit, CmdResize, CmdReloadShader, CmdStop };`
  - `struct RenderCommand { CommandType type; HWND hwnd; int width; int height; };`
  - `void CommandQueue::push(const RenderCommand&)`
  - `bool CommandQueue::tryPop(RenderCommand* out)`
  - `enum FeedbackKind { FbLog, FbError, FbWarn, FbFps, FbDeviceLost, FbDeviceOk };`
  - `struct FeedbackItem { FeedbackKind kind; std::string text; float ms; };`
  - `void FeedbackQueue::push(const FeedbackItem&)`
  - `bool FeedbackQueue::tryPop(FeedbackItem* out)`
  - `void SnapshotBuffer::publish(const StateSnapshot&)`
  - `StateSnapshot SnapshotBuffer::consume()`

- [ ] **Step 1: Write failing tests**

`tests/test_queues.cpp`:

```cpp
#include "test_harness.h"
#include "render/CommandQueue.h"
#include "render/FeedbackQueue.h"
#include "core/SnapshotBuffer.h"

void runQueueTests() {
  CommandQueue cq;
  RenderCommand c;
  TEST_CHECK(cq.tryPop(&c) == false);
  RenderCommand in;
  in.type = CmdStop;
  in.hwnd = 0;
  in.width = 0;
  in.height = 0;
  cq.push(in);
  TEST_CHECK(cq.tryPop(&c) == true);
  TEST_CHECK(c.type == CmdStop);

  FeedbackQueue fq;
  FeedbackItem f;
  TEST_CHECK(fq.tryPop(&f) == false);
  FeedbackItem fin;
  fin.kind = FbError;
  fin.text = "boom";
  fin.ms = 1.5f;
  fq.push(fin);
  TEST_CHECK(fq.tryPop(&f) == true);
  TEST_CHECK(f.kind == FbError);
  TEST_CHECK(f.text == "boom");

  SnapshotBuffer buf;
  StateSnapshot a = stateSnapshotDefault();
  a.teaching.camDistance = 9.f;
  a.viewportW = 640;
  buf.publish(a);
  StateSnapshot b = buf.consume();
  TEST_CHECK(b.teaching.camDistance == 9.f);
  TEST_CHECK(b.viewportW == 640);
}
```

`tests/test_main.cpp` add `void runQueueTests();` and call it from `main` before the failure check.

- [ ] **Step 2: Run tests 鈥?expect FAIL** (missing symbols)

```bat
cmake --build build
```

Expected: link error `CommandQueue` / `SnapshotBuffer` not found.

- [ ] **Step 3: Implement queues**

`src/render/CommandQueue.h`:

```cpp
#pragma once
#include <windows.h>
#include <mutex>
#include <queue>

enum CommandType {
  CmdInit = 0,
  CmdResize = 1,
  CmdReloadShader = 2,
  CmdStop = 3
};

struct RenderCommand {
  CommandType type;
  HWND hwnd;
  int width;
  int height;
};

class CommandQueue {
public:
  void push(const RenderCommand& cmd);
  bool tryPop(RenderCommand* out);
private:
  std::mutex m_mu;
  std::queue<RenderCommand> m_q;
};
```

`src/render/CommandQueue.cpp`:

```cpp
#include "render/CommandQueue.h"

void CommandQueue::push(const RenderCommand& cmd) {
  std::lock_guard<std::mutex> lock(m_mu);
  m_q.push(cmd);
}

bool CommandQueue::tryPop(RenderCommand* out) {
  std::lock_guard<std::mutex> lock(m_mu);
  if (m_q.empty()) {
    return false;
  }
  *out = m_q.front();
  m_q.pop();
  return true;
}
```

`src/render/FeedbackQueue.h`:

```cpp
#pragma once
#include <mutex>
#include <queue>
#include <string>

enum FeedbackKind {
  FbLog = 0,
  FbError = 1,
  FbWarn = 2,
  FbFps = 3,
  FbDeviceLost = 4,
  FbDeviceOk = 5
};

struct FeedbackItem {
  FeedbackKind kind;
  std::string text;
  float ms;
};

class FeedbackQueue {
public:
  void push(const FeedbackItem& item);
  bool tryPop(FeedbackItem* out);
private:
  std::mutex m_mu;
  std::queue<FeedbackItem> m_q;
};
```

`src/render/FeedbackQueue.cpp`:

```cpp
#include "render/FeedbackQueue.h"

void FeedbackQueue::push(const FeedbackItem& item) {
  std::lock_guard<std::mutex> lock(m_mu);
  m_q.push(item);
}

bool FeedbackQueue::tryPop(FeedbackItem* out) {
  std::lock_guard<std::mutex> lock(m_mu);
  if (m_q.empty()) {
    return false;
  }
  *out = m_q.front();
  m_q.pop();
  return true;
}
```

`src/core/SnapshotBuffer.h`:

```cpp
#pragma once
#include "core/StateSnapshot.h"
#include <mutex>

class SnapshotBuffer {
public:
  SnapshotBuffer();
  void publish(const StateSnapshot& snap);
  StateSnapshot consume();
private:
  std::mutex m_mu;
  StateSnapshot m_front;
  StateSnapshot m_back;
  bool m_dirty;
};
```

`src/core/SnapshotBuffer.cpp`:

```cpp
#include "core/SnapshotBuffer.h"

SnapshotBuffer::SnapshotBuffer()
    : m_front(stateSnapshotDefault())
    , m_back(stateSnapshotDefault())
    , m_dirty(false) {}

void SnapshotBuffer::publish(const StateSnapshot& snap) {
  std::lock_guard<std::mutex> lock(m_mu);
  m_back = snap;
  m_dirty = true;
}

StateSnapshot SnapshotBuffer::consume() {
  std::lock_guard<std::mutex> lock(m_mu);
  if (m_dirty) {
    m_front = m_back;
    m_dirty = false;
  }
  return m_front;
}
```

Add the six new `.cpp` files to `3deditmax_tests` in `CMakeLists.txt`. Tests may link nothing else.

- [ ] **Step 4: Rebuild and run tests**

```bat
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected: PASS.

- [ ] **Step 5: Commit and push**

```bash
git add src/render/CommandQueue.h src/render/CommandQueue.cpp src/render/FeedbackQueue.h src/render/FeedbackQueue.cpp src/core/SnapshotBuffer.h src/core/SnapshotBuffer.cpp tests/test_queues.cpp tests/test_main.cpp CMakeLists.txt
git commit -m "Add thread-safe command, feedback, and snapshot queues."
git push origin main
```

---

