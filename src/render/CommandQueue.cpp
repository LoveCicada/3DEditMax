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
