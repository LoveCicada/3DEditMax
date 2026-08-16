#pragma once
#include "core/LabState.h"
#include "core/MatrixFormat.h"
#include "core/TeachingState.h"
#include <QMainWindow>

class DebugLogPanel;
class Dx11ViewportWidget;
class MatrixBoardPanel;
class QAction;
class QTimer;
class TransformPanel;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = 0);
private slots:
  void onPollFeedback();
  void onTransformsChanged();
  void onTeachingEdited(const TeachingState& t);
  void onToggleMajor();
  void onReset();
private:
  void refreshBoard();
  void syncMajorActionText();

  Dx11ViewportWidget* m_viewport;
  TransformPanel* m_transforms;
  MatrixBoardPanel* m_board;
  DebugLogPanel* m_log;
  QTimer* m_poll;
  QAction* m_majorAction;
  TeachingState m_teaching;
  LabState m_lab;
  MajorOrder m_major;
};
