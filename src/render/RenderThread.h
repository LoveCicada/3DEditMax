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
