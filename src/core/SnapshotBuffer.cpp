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
