#include "ui/Dx11ViewportWidget.h"
#include "core/StateSnapshot.h"
#include <QCoreApplication>
#include <QHideEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <windows.h>

Dx11ViewportWidget::Dx11ViewportWidget(QWidget* parent)
    : QWidget(parent)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault()) {
  setAttribute(Qt::WA_NativeWindow, true);
  setAttribute(Qt::WA_DontCreateNativeAncestors, true);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAutoFillBackground(false);
  setMinimumSize(320, 180);
}

Dx11ViewportWidget::~Dx11ViewportWidget() {
  stopRenderer();
}

void Dx11ViewportWidget::publishState(const TeachingState& t, const LabState& l) {
  m_teaching = t;
  m_lab = l;
  if (!m_thread) {
    return;
  }
  StateSnapshot snap = stateSnapshotDefault();
  snap.teaching = m_teaching;
  snap.lab = m_lab;
  snap.viewportW = width();
  snap.viewportH = height();
  m_thread->snapshots().publish(snap);
}

FeedbackQueue& Dx11ViewportWidget::feedback() {
  if (m_thread) {
    return m_thread->feedback();
  }
  static FeedbackQueue empty;
  return empty;
}

void Dx11ViewportWidget::showEvent(QShowEvent* e) {
  QWidget::showEvent(e);
  winId();
  startRenderer();
}

void Dx11ViewportWidget::hideEvent(QHideEvent* e) {
  stopRenderer();
  QWidget::hideEvent(e);
}

void Dx11ViewportWidget::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);
  if (!m_thread) {
    return;
  }
  RenderCommand cmd;
  cmd.type = CmdResize;
  cmd.hwnd = hwnd();
  cmd.width = width();
  cmd.height = height();
  m_thread->commands().push(cmd);
  publishState(m_teaching, m_lab);
}

void Dx11ViewportWidget::paintEvent(QPaintEvent*) {
}

QPaintEngine* Dx11ViewportWidget::paintEngine() const {
  return 0;
}

void Dx11ViewportWidget::startRenderer() {
  if (m_thread) {
    return;
  }
  const QString dir = QCoreApplication::applicationDirPath() + QString::fromUtf8("/shaders");
  m_thread = ::make_unique<RenderThread>(dir.toStdWString());
  m_thread->start();
  RenderCommand init;
  init.type = CmdInit;
  init.hwnd = hwnd();
  init.width = width();
  init.height = height();
  m_thread->commands().push(init);
  publishState(m_teaching, m_lab);
}

void Dx11ViewportWidget::stopRenderer() {
  if (!m_thread) {
    return;
  }
  m_thread->requestStopAndJoin();
  m_thread.reset();
}

HWND Dx11ViewportWidget::hwnd() const {
  return reinterpret_cast<HWND>(winId());
}
