#pragma once
#include <mutex>
#include <queue>
#include <string>

enum FeedbackKind {
  FbLog = 0,
  FbError = 1,
  FbWarn = 2,
  FbFps = 3,
  FbDeviceLost = 4,
  FbDeviceOk = 5
};

struct FeedbackItem {
  FeedbackKind kind;
  std::string text;
  float ms;
};

class FeedbackQueue {
public:
  void push(const FeedbackItem& item);
  bool tryPop(FeedbackItem* out);
private:
  std::mutex m_mu;
  std::queue<FeedbackItem> m_q;
};
