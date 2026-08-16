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
