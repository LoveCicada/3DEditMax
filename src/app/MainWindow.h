#pragma once
#include "core/LabState.h"
#include "core/MatrixFormat.h"
#include "core/TeachingState.h"
#include "teach/DemoPlayer.h"
#include <QMainWindow>

class DebugLogPanel;
class Dx11ViewportWidget;
class LabPanel;
class MatrixBoardPanel;
class ObjectPanel;
class QAction;
class QTimer;
class TrackerPanel;
class TransformPanel;
class TutorialPanel;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = 0);
private slots:
  void onPollFeedback();
  void onTransformsChanged();
  void onObjectsChanged();
  void onTrackerChanged();
  void onTeachingEdited(const TeachingState& t);
  void onTutorialApply(const TeachingState& t);
  void onLabChanged();
  void onToggleMajor();
  void onReset();
  void onImportJson();
  void onExportJson();
  void onDemoTick();
private:
  void syncTeaching();
  void refreshBoard();
  void refreshTracker();
  void syncMajorActionText();

  Dx11ViewportWidget* m_viewport;
  TransformPanel* m_transforms;
  ObjectPanel* m_objects;
  TrackerPanel* m_tracker;
  MatrixBoardPanel* m_board;
  LabPanel* m_labPanel;
  DebugLogPanel* m_log;
  TutorialPanel* m_tutorial;
  QTimer* m_poll;
  QTimer* m_demoTimer;
  QAction* m_majorAction;
  DemoPlayer m_demo;
  TeachingState m_teaching;
  LabState m_lab;
  MajorOrder m_major;
};
