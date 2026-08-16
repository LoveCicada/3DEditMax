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
