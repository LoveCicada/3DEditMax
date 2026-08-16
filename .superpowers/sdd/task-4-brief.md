### Task 4: RenderThread loop without D3D

**Files:**
- Create: `src/render/RenderThread.h`
- Create: `src/render/RenderThread.cpp`
- Modify: `tests/test_queues.cpp` (add `runRenderThreadTests`)
- Modify: `tests/test_main.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `CommandQueue`, `FeedbackQueue`, `SnapshotBuffer`
- Produces:
  - `RenderThread::RenderThread()`
  - `void RenderThread::start()`
  - `void RenderThread::requestStopAndJoin()`
  - `CommandQueue& RenderThread::commands()`
  - `FeedbackQueue& RenderThread::feedback()`
  - `SnapshotBuffer& RenderThread::snapshots()`
  - Loop: pop all commands; on `CmdStop` exit; otherwise `consume()` snapshot and sleep 1ms (no GPU yet)

- [ ] **Step 1: Failing test**

```cpp
#include "render/RenderThread.h"
#include <chrono>
#include <thread>

void runRenderThreadTests() {
  RenderThread rt;
  rt.start();
  RenderCommand stop;
  stop.type = CmdStop;
  stop.hwnd = 0;
  stop.width = 0;
  stop.height = 0;
  rt.commands().push(stop);
  rt.requestStopAndJoin();
  TEST_CHECK(true);
}
```

`requestStopAndJoin` must be safe if `CmdStop` already ended the thread: join only if `joinable()`.

- [ ] **Step 2: Build 鈥?expect missing `RenderThread`**

- [ ] **Step 3: Implement**

`src/render/RenderThread.h`:

```cpp
#pragma once
#include "core/SnapshotBuffer.h"
#include "render/CommandQueue.h"
#include "render/FeedbackQueue.h"
#include <atomic>
#include <thread>

class RenderThread {
public:
  RenderThread();
  ~RenderThread();
  void start();
  void requestStopAndJoin();
  CommandQueue& commands() { return m_commands; }
  FeedbackQueue& feedback() { return m_feedback; }
  SnapshotBuffer& snapshots() { return m_snapshots; }
private:
  void threadMain();
  CommandQueue m_commands;
  FeedbackQueue m_feedback;
  SnapshotBuffer m_snapshots;
  std::thread m_thread;
  std::atomic<bool> m_running;
};
```

`src/render/RenderThread.cpp`:

```cpp
#include "render/RenderThread.h"
#include <chrono>

RenderThread::RenderThread() : m_running(false) {}

RenderThread::~RenderThread() {
  requestStopAndJoin();
}

void RenderThread::start() {
  if (m_thread.joinable()) {
    return;
  }
  m_running.store(true);
  m_thread = std::thread(&RenderThread::threadMain, this);
}

void RenderThread::requestStopAndJoin() {
  if (!m_thread.joinable()) {
    return;
  }
  RenderCommand stop;
  stop.type = CmdStop;
  stop.hwnd = 0;
  stop.width = 0;
  stop.height = 0;
  m_commands.push(stop);
  m_thread.join();
  m_running.store(false);
}

void RenderThread::threadMain() {
  for (;;) {
    RenderCommand cmd;
    bool stop = false;
    while (m_commands.tryPop(&cmd)) {
      if (cmd.type == CmdStop) {
        stop = true;
      }
    }
    if (stop) {
      break;
    }
    (void)m_snapshots.consume();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
```

- [ ] **Step 4: `ctest` PASS** (test returns within ~1s, no hang)

- [ ] **Step 5: Commit and push**

```bash
git add src/render/RenderThread.h src/render/RenderThread.cpp tests/test_queues.cpp tests/test_main.cpp CMakeLists.txt
git commit -m "Add std::thread render loop that stops on CmdStop."
git push origin main
```

---

