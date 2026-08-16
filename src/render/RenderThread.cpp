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
