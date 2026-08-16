### Task 6: Viewport widget, docks, debug log (Phase 0 UI)

**Files:**
- Create: `src/ui/Dx11ViewportWidget.h`
- Create: `src/ui/Dx11ViewportWidget.cpp`
- Create: `src/ui/DebugLogPanel.h`
- Create: `src/ui/DebugLogPanel.cpp`
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `RenderThread`, `StateSnapshot` defaults
- Produces:
  - `Dx11ViewportWidget` starts thread in `showEvent`, `requestStopAndJoin` in `hideEvent` and destructor
  - `void Dx11ViewportWidget::publishState(const TeachingState&, const LabState&)`
  - `FeedbackQueue& Dx11ViewportWidget::feedback()`
  - `DebugLogPanel::appendFromQueue(FeedbackQueue&)`
  - `MainWindow` central widget + right `Debug` dock + `QTimer` 100ms poll

- [ ] **Step 1: Viewport**

`src/ui/Dx11ViewportWidget.h`:

```cpp
#pragma once
#include "core/LabState.h"
#include "core/TeachingState.h"
#include "core/MakeUnique.h"
#include "render/RenderThread.h"
#include <QWidget>
#include <memory>

class Dx11ViewportWidget : public QWidget {
  Q_OBJECT
public:
  explicit Dx11ViewportWidget(QWidget* parent = 0);
  ~Dx11ViewportWidget();
  void publishState(const TeachingState& t, const LabState& l);
  FeedbackQueue& feedback();
protected:
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);
  void resizeEvent(QResizeEvent* e);
  void paintEvent(QPaintEvent* e);
  QPaintEngine* paintEngine() const;
private:
  void startRenderer();
  void stopRenderer();
  HWND hwnd() const;
  std::unique_ptr<RenderThread> m_thread;
  TeachingState m_teaching;
  LabState m_lab;
};
```

`src/ui/Dx11ViewportWidget.cpp` (required behavior):

```cpp
setAttribute(Qt::WA_NativeWindow, true);
setAttribute(Qt::WA_DontCreateNativeAncestors, true);
setAttribute(Qt::WA_OpaquePaintEvent, true);
setAttribute(Qt::WA_NoSystemBackground, true);
setAutoFillBackground(false);
setMinimumSize(320, 180);
```

`paintEngine() const { return 0; }`  
`paintEvent`: empty (D3D owns the pixels).

`hwnd()`: `reinterpret_cast<HWND>(winId())`.

`startRenderer`:

```cpp
const QString dir = QCoreApplication::applicationDirPath() + QString::fromUtf8("/shaders");
m_thread = make_unique<RenderThread>(dir.toStdWString());
m_thread->start();
RenderCommand init;
init.type = CmdInit;
init.hwnd = hwnd();
init.width = width();
init.height = height();
m_thread->commands().push(init);
publishState(m_teaching, m_lab);
```

`publishState`: copy into members; build `StateSnapshot` with `viewportW/H = width/height`; `m_thread->snapshots().publish(snap)`.

`resizeEvent`: if thread exists, push `CmdResize` with new size, then `publishState`.

`stopRenderer`: if `m_thread`, `requestStopAndJoin()`, `m_thread.reset()`.

Call `startRenderer` from `showEvent` after `QWidget::showEvent` and `winId()`. Call `stopRenderer` from `hideEvent` before `QWidget::hideEvent`, and from destructor.

- [ ] **Step 2: DebugLogPanel**

`QPlainTextEdit` read-only. `drain(FeedbackQueue& q)`: while `tryPop`, append line `[Error] text` / `[Warn]` / `[Log]` / ignore `FbFps` in the text box (optional status later).

- [ ] **Step 3: MainWindow**

Members: `Dx11ViewportWidget* m_viewport`, `DebugLogPanel* m_log`, `QTimer* m_poll`, `TeachingState m_teaching`, `LabState m_lab`.

```cpp
setCentralWidget(m_viewport);
QDockWidget* dock = new QDockWidget(QString::fromUtf8("DX11 Lab / Debug"), this);
dock->setObjectName(QString::fromUtf8("dockDebug"));
dock->setWidget(m_log);
addDockWidget(Qt::RightDockWidgetArea, dock);
m_poll = new QTimer(this);
m_poll->setInterval(100);
connect(m_poll, &QTimer::timeout, this, &MainWindow::onPollFeedback);
m_poll->start();
```

`onPollFeedback`: `m_log->drain(m_viewport->feedback());`

Menu `瑙嗗浘` with `dock->toggleViewAction()`.

- [ ] **Step 4: Manual test**

Build Debug. Run exe from output dir (so `shaders/unlit.hlsl` exists). Expect: dark viewport, lit cube, log line `D3D11 device ready`. Resize window: no crash, cube aspect updates. Close window: process exits (thread joined). Hide/show if applicable: no double device leak.

- [ ] **Step 5: Commit and push**

```bash
git add src/ui/Dx11ViewportWidget.h src/ui/Dx11ViewportWidget.cpp src/ui/DebugLogPanel.h src/ui/DebugLogPanel.cpp src/app/MainWindow.h src/app/MainWindow.cpp CMakeLists.txt
git commit -m "Embed D3D11 swapchain in a Qt viewport with debug log polling."
git push origin main
```

---

