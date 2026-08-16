#pragma once
#include <windows.h>
#include <mutex>
#include <queue>

enum CommandType {
  CmdInit = 0,
  CmdResize = 1,
  CmdReloadShader = 2,
  CmdStop = 3
};

struct RenderCommand {
  CommandType type;
  HWND hwnd;
  int width;
  int height;
};

class CommandQueue {
public:
  void push(const RenderCommand& cmd);
  bool tryPop(RenderCommand* out);
private:
  std::mutex m_mu;
  std::queue<RenderCommand> m_q;
};
