#include "ui/Dx11ViewportWidget.h"
#include "core/StateSnapshot.h"
#include <QCoreApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHideEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <windows.h>

Dx11ViewportWidget::Dx11ViewportWidget(QWidget* parent)
    : QWidget(parent)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault())
    , m_lastMouse(0, 0)
    , m_axisLegend(0)
    , m_orbiting(false) {
  setAttribute(Qt::WA_NativeWindow, true);
  setAttribute(Qt::WA_DontCreateNativeAncestors, true);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAutoFillBackground(false);
  setMinimumSize(320, 180);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  buildAxisLegend();
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

void Dx11ViewportWidget::reloadShaders() {
  if (!m_thread) {
    return;
  }
  RenderCommand cmd;
  cmd.type = CmdReloadShader;
  cmd.hwnd = hwnd();
  cmd.width = width();
  cmd.height = height();
  m_thread->commands().push(cmd);
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
  if (m_axisLegend) {
    m_axisLegend->raise();
    m_axisLegend->show();
  }
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
  m_thread->resizeOnOwnerThread(width(), height());
  publishState(m_teaching, m_lab);
}

void Dx11ViewportWidget::paintEvent(QPaintEvent*) {
}

void Dx11ViewportWidget::mousePressEvent(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton) {
    m_lastMouse = e->pos();
    m_orbiting = true;
    grabMouse();
    setFocus(Qt::MouseFocusReason);
  }
  QWidget::mousePressEvent(e);
}

void Dx11ViewportWidget::mouseReleaseEvent(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton && m_orbiting) {
    m_orbiting = false;
    releaseMouse();
    publishState(m_teaching, m_lab);
    emit teachingEdited(m_teaching);
  }
  QWidget::mouseReleaseEvent(e);
}

void Dx11ViewportWidget::mouseMoveEvent(QMouseEvent* e) {
  if (e->buttons() & Qt::LeftButton) {
    const QPoint p = e->pos();
    const float dx = static_cast<float>(p.x() - m_lastMouse.x());
    const float dy = static_cast<float>(p.y() - m_lastMouse.y());
    m_lastMouse = p;
    applyOrbitDrag(&m_teaching, dx, dy);
    commitTeaching();
  }
  QWidget::mouseMoveEvent(e);
}

void Dx11ViewportWidget::wheelEvent(QWheelEvent* e) {
  const int delta = e->angleDelta().y();
  if (delta != 0) {
    applyDollyWheel(&m_teaching, delta);
    commitTeaching();
    e->accept();
    return;
  }
  QWidget::wheelEvent(e);
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
  m_thread->initOnOwnerThread(hwnd(), width(), height());
  publishState(m_teaching, m_lab);
}

void Dx11ViewportWidget::stopRenderer() {
  if (!m_thread) {
    return;
  }
  m_thread->shutdownOnOwnerThread();
  m_thread->requestStopAndJoin();
  m_thread.reset();
}

void Dx11ViewportWidget::commitTeaching() {
  publishState(m_teaching, m_lab);
  if (!m_orbiting) {
    emit teachingEdited(m_teaching);
  }
}

void Dx11ViewportWidget::buildAxisLegend() {
  m_axisLegend = new QFrame(this);
  m_axisLegend->setObjectName(QString::fromUtf8("axisLegend"));
  m_axisLegend->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  m_axisLegend->setStyleSheet(QString::fromUtf8(
      "QFrame#axisLegend {"
      "  background-color: rgba(10, 10, 24, 184);"
      "  border: 1px solid rgba(97, 218, 251, 71);"
      "  border-radius: 6px;"
      "}"
      "QLabel#axisLegendTitle {"
      "  color: #61dafb;"
      "  font-size: 11px;"
      "  font-weight: 700;"
      "}"
      "QLabel#axisLegendName { color: #ccd6f6; font-size: 11px; }"
      "QLabel#axisLegendDesc { color: #8892b0; font-size: 10px; }"));

  QVBoxLayout* root = new QVBoxLayout(m_axisLegend);
  root->setContentsMargins(10, 8, 10, 8);
  root->setSpacing(6);

  QLabel* title = new QLabel(QString::fromUtf8("\xE5\x9D\x90\xE6\xA0\x87\xE8\xBD\xB4\xE5\x9B\xBE\xE4\xBE\x8B"), m_axisLegend);
  title->setObjectName(QString::fromUtf8("axisLegendTitle"));
  root->addWidget(title);

  QGridLayout* grid = new QGridLayout();
  grid->setContentsMargins(0, 0, 0, 0);
  grid->setHorizontalSpacing(8);
  grid->setVerticalSpacing(5);
  grid->setColumnStretch(2, 1);

  const char* names[3] = {
      "X \xE8\xBD\xB4",
      "Y \xE8\xBD\xB4",
      "Z \xE8\xBD\xB4",
  };
  const char* descs[3] = {
      "World / Local \xC2\xB7 \xE7\xBA\xA2\xE8\x89\xB2 \xC2\xB7 \xE5\xB7\xA6\xE5\x8F\xB3\xE6\x96\xB9\xE5\x90\x91",
      "World / Local \xC2\xB7 \xE7\xBB\xBF\xE8\x89\xB2 \xC2\xB7 \xE4\xB8\x8A\xE4\xB8\x8B\xE6\x96\xB9\xE5\x90\x91",
      "World / Local \xC2\xB7 \xE8\x93\x9D\xE8\x89\xB2 \xC2\xB7 \xE5\x89\x8D\xE5\x90\x8E\xE6\x96\xB9\xE5\x90\x91",
  };
  const char* swatches[3] = { "#ff4444", "#44ff44", "#4444ff" };

  for (int i = 0; i < 3; ++i) {
    QLabel* swatch = new QLabel(m_axisLegend);
    swatch->setFixedSize(10, 10);
    swatch->setStyleSheet(QString::fromUtf8(
        "background-color: %1; border-radius: 5px; border: 1px solid rgba(255,255,255,20);")
                              .arg(QString::fromUtf8(swatches[i])));
    QLabel* name = new QLabel(QString::fromUtf8(names[i]), m_axisLegend);
    name->setObjectName(QString::fromUtf8("axisLegendName"));
    QLabel* desc = new QLabel(QString::fromUtf8(descs[i]), m_axisLegend);
    desc->setObjectName(QString::fromUtf8("axisLegendDesc"));
    grid->addWidget(swatch, i, 0, Qt::AlignVCenter);
    grid->addWidget(name, i, 1, Qt::AlignVCenter);
    grid->addWidget(desc, i, 2, Qt::AlignVCenter);
  }
  root->addLayout(grid);

  m_axisLegend->adjustSize();
  m_axisLegend->move(12, 10);
  m_axisLegend->raise();
  m_axisLegend->show();
}

HWND Dx11ViewportWidget::hwnd() const {
  return reinterpret_cast<HWND>(winId());
}
