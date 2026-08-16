#pragma once
#include "core/LabState.h"
#include "core/TeachingState.h"
#include "core/MakeUnique.h"
#include "render/RenderThread.h"
#include <QWidget>
#include <memory>

class Dx11ViewportWidget : public QWidget {
  Q_OBJECT
public:
  explicit Dx11ViewportWidget(QWidget* parent = 0);
  ~Dx11ViewportWidget();
  void publishState(const TeachingState& t, const LabState& l);
  FeedbackQueue& feedback();
protected:
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);
  void resizeEvent(QResizeEvent* e);
  void paintEvent(QPaintEvent* e);
  QPaintEngine* paintEngine() const;
private:
  void startRenderer();
  void stopRenderer();
  HWND hwnd() const;
  std::unique_ptr<RenderThread> m_thread;
  TeachingState m_teaching;
  LabState m_lab;
};
