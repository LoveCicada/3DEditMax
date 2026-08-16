#include "render/RenderThread.h"
#include <chrono>

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
  int frames = 0;
  for (;;) {
    RenderCommand cmd;
    bool stop = false;
#ifndef D3DEDITMAX_NO_D3D
    bool allowPresent = true;
#endif
    while (m_commands.tryPop(&cmd)) {
      if (cmd.type == CmdInit) {
        m_hwnd = cmd.hwnd;
        m_w = cmd.width;
        m_h = cmd.height;
#ifndef D3DEDITMAX_NO_D3D
        allowPresent = m_renderer.initialize(cmd.hwnd, cmd.width, cmd.height, true, &m_feedback);
#endif
      } else if (cmd.type == CmdResize) {
        m_w = cmd.width;
        m_h = cmd.height;
#ifndef D3DEDITMAX_NO_D3D
        if (!m_renderer.resize(cmd.width, cmd.height)) {
          allowPresent = false;
        }
#endif
      } else if (cmd.type == CmdReloadShader) {
#ifndef D3DEDITMAX_NO_D3D
        m_renderer.reloadShaders(&m_feedback);
#endif
      } else if (cmd.type == CmdStop) {
#ifndef D3DEDITMAX_NO_D3D
        m_renderer.shutdown();
#endif
        stop = true;
      }
    }
    if (stop) {
      break;
    }
#ifndef D3DEDITMAX_NO_D3D
    if (allowPresent && m_renderer.initialized() && !m_renderer.dead() && m_renderer.viewsValid()) {
      const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
      m_renderer.render(m_snapshots.consume());
      const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
      ++frames;
      if (frames % 30 == 0) {
        FeedbackItem fps;
        fps.kind = FbFps;
        fps.text = "";
        fps.ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
        m_feedback.push(fps);
      }
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#else
    (void)m_snapshots.consume();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
  }
}
