#pragma once
#include "core/LabState.h"
#include "core/TeachingState.h"
#include <QMainWindow>

class DebugLogPanel;
class Dx11ViewportWidget;
class QTimer;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = 0);
private slots:
  void onPollFeedback();
private:
  Dx11ViewportWidget* m_viewport;
  DebugLogPanel* m_log;
  QTimer* m_poll;
  TeachingState m_teaching;
  LabState m_lab;
};
