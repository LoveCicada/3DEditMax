#pragma once
#include "core/LabState.h"
#include "core/TeachingState.h"
#include "core/MakeUnique.h"
#include "render/RenderThread.h"
#include <QPoint>
#include <QWidget>
#include <memory>

class QFrame;

class Dx11ViewportWidget : public QWidget {
  Q_OBJECT
public:
  explicit Dx11ViewportWidget(QWidget* parent = 0);
  ~Dx11ViewportWidget();
  void publishState(const TeachingState& t, const LabState& l);
  void reloadShaders();
  FeedbackQueue& feedback();
signals:
  void teachingEdited(const TeachingState& t);
protected:
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);
  void resizeEvent(QResizeEvent* e);
  void paintEvent(QPaintEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void wheelEvent(QWheelEvent* e);
  QPaintEngine* paintEngine() const;
private:
  void startRenderer();
  void stopRenderer();
  void commitTeaching();
  void buildAxisLegend();
  HWND hwnd() const;
  std::unique_ptr<RenderThread> m_thread;
  TeachingState m_teaching;
  LabState m_lab;
  QPoint m_lastMouse;
  QFrame* m_axisLegend;
  bool m_orbiting;
  bool m_panning;
};
