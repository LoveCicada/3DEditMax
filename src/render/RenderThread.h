#pragma once
#include "core/SnapshotBuffer.h"
#include "render/CommandQueue.h"
#include "render/FeedbackQueue.h"
#ifndef D3DEDITMAX_NO_D3D
#include "render/D3D11Renderer.h"
#endif
#include <atomic>
#include <string>
#include <thread>

class RenderThread {
public:
  RenderThread();
  explicit RenderThread(const std::wstring& shaderDir);
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
#ifndef D3DEDITMAX_NO_D3D
  D3D11Renderer m_renderer;
#endif
  HWND m_hwnd;
  int m_w;
  int m_h;
  std::wstring m_shaderDir;
  std::thread m_thread;
  std::atomic<bool> m_running;
};
