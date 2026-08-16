#pragma once
#include <QString>
#include <QWidget>

class FeedbackQueue;
class QPlainTextEdit;

class DebugLogPanel : public QWidget {
  Q_OBJECT
public:
  explicit DebugLogPanel(QWidget* parent = 0);
  void drain(FeedbackQueue& q);
  void appendFromQueue(FeedbackQueue& q);
  QString adapterName() const;
private:
  QPlainTextEdit* m_edit;
  QString m_adapter;
};
