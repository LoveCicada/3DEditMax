#pragma once
#include <QWidget>

class FeedbackQueue;
class QPlainTextEdit;

class DebugLogPanel : public QWidget {
  Q_OBJECT
public:
  explicit DebugLogPanel(QWidget* parent = 0);
  void drain(FeedbackQueue& q);
  void appendFromQueue(FeedbackQueue& q);
private:
  QPlainTextEdit* m_edit;
};
