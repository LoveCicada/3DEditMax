#pragma once
#include "core/LabState.h"
#include "core/TeachingState.h"
#include "core/MakeUnique.h"
#include "render/RenderThread.h"
#include <QPoint>
#include <QWidget>
#include <memory>

class QFrame;
class QLabel;
class Dx11NativeSurface;

class Dx11ViewportWidget : public QWidget {
  Q_OBJECT
  friend class Dx11NativeSurface;
public:
  explicit Dx11ViewportWidget(QWidget* parent = 0);
  ~Dx11ViewportWidget();
  void publishState(const TeachingState& t, const LabState& l);
  void reloadShaders();
  FeedbackQueue& feedback();
  int surfaceWidth() const;
  int surfaceHeight() const;
signals:
  void teachingEdited(const TeachingState& t);
protected:
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);
private:
  void startRenderer();
  void stopRenderer();
  void commitTeaching();
  void buildHud();
  void updateHud();
  void onSurfaceResized();
  void surfaceMousePress(QMouseEvent* e);
  void surfaceMouseRelease(QMouseEvent* e);
  void surfaceMouseMove(QMouseEvent* e);
  void surfaceWheel(QWheelEvent* e);
  HWND hwnd() const;
  std::unique_ptr<RenderThread> m_thread;
  TeachingState m_teaching;
  LabState m_lab;
  QPoint m_lastMouse;
  QFrame* m_hud;
  QLabel* m_demoCaption;
  Dx11NativeSurface* m_surface;
  bool m_orbiting;
  bool m_panning;
  bool m_translating;
  int m_translateAxis;
};
