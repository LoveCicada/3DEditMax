#include "render/RenderThread.h"
#include <chrono>
#include <string>

RenderThread::RenderThread() : RenderThread(std::wstring()) {}

RenderThread::RenderThread(const std::wstring& shaderDir)
#ifndef D3DEDITMAX_NO_D3D
    : m_renderer(shaderDir)
    , m_hwnd(0)
#else
    : m_hwnd(0)
#endif
    , m_w(0)
    , m_h(0)
    , m_shaderDir(shaderDir)
    , m_running(false) {}

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
  m_running.store(false);
  RenderCommand stop;
  stop.type = CmdStop;
  stop.hwnd = 0;
  stop.width = 0;
  stop.height = 0;
  m_commands.push(stop);
  m_thread.join();
}

bool RenderThread::initOnOwnerThread(HWND hwnd, int w, int h) {
  std::lock_guard<std::mutex> lock(m_d3dMu);
  m_hwnd = hwnd;
  m_w = w;
  m_h = h;
#ifndef D3DEDITMAX_NO_D3D
  const bool ok = m_renderer.initialize(hwnd, w, h, true, &m_feedback);
  if (ok) {
    const std::string adapter = m_renderer.adapterNameUtf8();
    if (!adapter.empty()) {
      FeedbackItem item;
      item.kind = FbLog;
      item.text = std::string("Adapter: ") + adapter;
      item.ms = 0.f;
      m_feedback.push(item);
    }
  }
  return ok;
#else
  (void)hwnd;
  (void)w;
  (void)h;
  return true;
#endif
}

bool RenderThread::resizeOnOwnerThread(int w, int h) {
  std::lock_guard<std::mutex> lock(m_d3dMu);
  m_w = w;
  m_h = h;
#ifndef D3DEDITMAX_NO_D3D
  if (!m_renderer.initialized()) {
    return true;
  }
  return m_renderer.resize(w, h);
#else
  (void)w;
  (void)h;
  return true;
#endif
}

void RenderThread::shutdownOnOwnerThread() {
  std::lock_guard<std::mutex> lock(m_d3dMu);
#ifndef D3DEDITMAX_NO_D3D
  m_renderer.shutdown();
#endif
  m_hwnd = 0;
  m_w = 0;
  m_h = 0;
}

void RenderThread::threadMain() {
  int frames = 0;
  for (;;) {
    if (!m_running.load()) {
      break;
    }
    RenderCommand cmd;
    bool stop = false;
    while (m_commands.tryPop(&cmd)) {
      if (cmd.type == CmdStop) {
        stop = true;
      } else if (cmd.type == CmdReloadShader) {
#ifndef D3DEDITMAX_NO_D3D
        std::lock_guard<std::mutex> lock(m_d3dMu);
        m_renderer.reloadShaders(&m_feedback);
#endif
      }
    }
    if (stop) {
      break;
    }
#ifndef D3DEDITMAX_NO_D3D
    bool presented = false;
    {
      std::lock_guard<std::mutex> lock(m_d3dMu);
      if (m_renderer.initialized() && !m_renderer.dead() && m_renderer.viewsValid()) {
        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        m_renderer.render(m_snapshots.consume());
        const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        presented = true;
        ++frames;
        if (frames % 30 == 0) {
          FeedbackItem fps;
          fps.kind = FbFps;
          fps.text = "";
          fps.ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
          m_feedback.push(fps);
        }
      }
    }
    (void)presented;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
    (void)m_snapshots.consume();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
  }
}
