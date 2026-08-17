#include "ui/Dx11ViewportWidget.h"
#include "core/StateSnapshot.h"
#include "teach/Transforms.h"
#include "teach/TutorialScript.h"
#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEngine>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <windows.h>

class Dx11NativeSurface : public QWidget {
public:
  explicit Dx11NativeSurface(Dx11ViewportWidget* owner)
      : QWidget(owner)
      , m_owner(owner) {
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::NoContextMenu);
    setMinimumSize(320, 140);
  }

protected:
  void paintEvent(QPaintEvent*) {}
  QPaintEngine* paintEngine() const { return 0; }
  void resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    if (m_owner) {
      m_owner->onSurfaceResized();
    }
  }
  void mousePressEvent(QMouseEvent* e) {
    if (m_owner) {
      m_owner->surfaceMousePress(e);
    }
  }
  void mouseReleaseEvent(QMouseEvent* e) {
    if (m_owner) {
      m_owner->surfaceMouseRelease(e);
    }
  }
  void mouseMoveEvent(QMouseEvent* e) {
    if (m_owner) {
      m_owner->surfaceMouseMove(e);
    }
  }
  void wheelEvent(QWheelEvent* e) {
    if (m_owner) {
      m_owner->surfaceWheel(e);
    }
  }

private:
  Dx11ViewportWidget* m_owner;
};

Dx11ViewportWidget::Dx11ViewportWidget(QWidget* parent)
    : QWidget(parent)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault())
    , m_lastMouse(0, 0)
    , m_hud(0)
    , m_demoCaption(0)
    , m_surface(0)
    , m_orbiting(false)
    , m_panning(false)
    , m_translating(false)
    , m_translateAxis(-1) {
  setMinimumSize(320, 180);
  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(0);
  buildHud();
  m_surface = new Dx11NativeSurface(this);
  root->addWidget(m_hud);
  root->addWidget(m_surface, 1);
}

Dx11ViewportWidget::~Dx11ViewportWidget() {
  stopRenderer();
}

void Dx11ViewportWidget::publishState(const TeachingState& t, const LabState& l) {
  m_teaching = t;
  m_lab = l;
  updateHud();
  if (!m_thread) {
    return;
  }
  StateSnapshot snap = stateSnapshotDefault();
  snap.teaching = m_teaching;
  snap.lab = m_lab;
  snap.viewportW = surfaceWidth();
  snap.viewportH = surfaceHeight();
  m_thread->snapshots().publish(snap);
}

void Dx11ViewportWidget::reloadShaders() {
  if (!m_thread) {
    return;
  }
  RenderCommand cmd;
  cmd.type = CmdReloadShader;
  cmd.hwnd = hwnd();
  cmd.width = surfaceWidth();
  cmd.height = surfaceHeight();
  m_thread->commands().push(cmd);
}

FeedbackQueue& Dx11ViewportWidget::feedback() {
  if (m_thread) {
    return m_thread->feedback();
  }
  static FeedbackQueue empty;
  return empty;
}

int Dx11ViewportWidget::surfaceWidth() const {
  return m_surface ? m_surface->width() : width();
}

int Dx11ViewportWidget::surfaceHeight() const {
  return m_surface ? m_surface->height() : height();
}

void Dx11ViewportWidget::showEvent(QShowEvent* e) {
  QWidget::showEvent(e);
  if (m_surface) {
    m_surface->winId();
  }
  startRenderer();
}

void Dx11ViewportWidget::hideEvent(QHideEvent* e) {
  stopRenderer();
  QWidget::hideEvent(e);
}

void Dx11ViewportWidget::onSurfaceResized() {
  if (!m_thread || !m_surface) {
    return;
  }
  m_thread->resizeOnOwnerThread(surfaceWidth(), surfaceHeight());
  publishState(m_teaching, m_lab);
}

void Dx11ViewportWidget::surfaceMousePress(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton && !m_panning && !m_translating) {
    m_lastMouse = e->pos();
    const int axis = m_teaching.demoPlaying
        ? -1
        : hitWorldAxisHandle(m_teaching, static_cast<float>(m_lastMouse.x()),
                             static_cast<float>(m_lastMouse.y()),
                             static_cast<float>(surfaceWidth()),
                             static_cast<float>(surfaceHeight()));
    if (axis >= 0) {
      m_translating = true;
      m_translateAxis = axis;
    } else {
      m_orbiting = true;
    }
    m_surface->grabMouse();
    m_surface->setFocus(Qt::MouseFocusReason);
  } else if (e->button() == Qt::RightButton && !m_orbiting && !m_translating) {
    m_lastMouse = e->pos();
    m_panning = true;
    m_surface->grabMouse();
    m_surface->setFocus(Qt::MouseFocusReason);
  }
}

void Dx11ViewportWidget::surfaceMouseRelease(QMouseEvent* e) {
  if (e->button() == Qt::LeftButton && (m_orbiting || m_translating)) {
    m_orbiting = false;
    m_translating = false;
    m_translateAxis = -1;
    m_surface->releaseMouse();
    publishState(m_teaching, m_lab);
    emit teachingEdited(m_teaching);
  } else if (e->button() == Qt::RightButton && m_panning) {
    m_panning = false;
    m_surface->releaseMouse();
    publishState(m_teaching, m_lab);
    emit teachingEdited(m_teaching);
  }
}

void Dx11ViewportWidget::surfaceMouseMove(QMouseEvent* e) {
  const QPoint p = e->pos();
  const float dx = static_cast<float>(p.x() - m_lastMouse.x());
  const float dy = static_cast<float>(p.y() - m_lastMouse.y());
  const float vw = static_cast<float>(surfaceWidth());
  const float vh = static_cast<float>(surfaceHeight());
  if (m_translating && (e->buttons() & Qt::LeftButton) && m_translateAxis >= 0) {
    m_lastMouse = p;
    const float worldDelta = axisTranslateFromDrag(m_teaching, m_translateAxis, dx, dy, vw, vh);
    applyAxisTranslateDrag(&m_teaching, m_translateAxis, worldDelta);
    commitTeaching();
  } else if (m_orbiting && (e->buttons() & Qt::LeftButton)) {
    m_lastMouse = p;
    applyOrbitDrag(&m_teaching, dx, dy);
    commitTeaching();
  } else if (m_panning && (e->buttons() & Qt::RightButton)) {
    m_lastMouse = p;
    applyPanDrag(&m_teaching, dx, dy, vw, vh);
    commitTeaching();
  }
}

void Dx11ViewportWidget::surfaceWheel(QWheelEvent* e) {
  const int delta = e->angleDelta().y();
  if (delta != 0) {
    applyDollyWheel(&m_teaching, delta);
    commitTeaching();
    e->accept();
  }
}

void Dx11ViewportWidget::startRenderer() {
  if (m_thread) {
    return;
  }
  const QString dir = QCoreApplication::applicationDirPath() + QString::fromUtf8("/shaders");
  m_thread = ::make_unique<RenderThread>(dir.toStdWString());
  m_thread->start();
  m_thread->initOnOwnerThread(hwnd(), surfaceWidth(), surfaceHeight());
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
  if (!m_orbiting && !m_panning && !m_translating) {
    emit teachingEdited(m_teaching);
  }
}

void Dx11ViewportWidget::buildHud() {
  m_hud = new QFrame(this);
  m_hud->setObjectName(QString::fromUtf8("axisLegend"));
  m_hud->setStyleSheet(QString::fromUtf8(
      "QFrame#axisLegend {"
      "  background-color: #0a0a18;"
      "  border-bottom: 1px solid rgba(97, 218, 251, 71);"
      "}"
      "QLabel#axisLegendName { color: #ccd6f6; font-size: 11px; }"
      "QLabel#demoCaption {"
      "  color: #e6edf3;"
      "  font-size: 12px;"
      "  padding: 0 8px 6px 8px;"
      "}"));

  QVBoxLayout* hudLay = new QVBoxLayout(m_hud);
  hudLay->setContentsMargins(8, 6, 8, 4);
  hudLay->setSpacing(4);

  QHBoxLayout* legend = new QHBoxLayout();
  legend->setContentsMargins(0, 0, 0, 0);
  legend->setSpacing(10);
  const char* names[4] = {
      "X  \xE5\xB7\xA6\xE5\x8F\xB3",
      "Y  \xE4\xB8\x8A\xE4\xB8\x8B",
      "Z  \xE5\x89\x8D\xE5\x90\x8E",
      "\xE8\xBF\xBD\xE8\xB8\xAA\xE7\x82\xB9",
  };
  const char* swatches[4] = { "#ff4444", "#44ff44", "#4444ff", "#ff59ff" };
  for (int i = 0; i < 4; ++i) {
    QLabel* swatch = new QLabel(m_hud);
    swatch->setFixedSize(10, 10);
    swatch->setStyleSheet(QString::fromUtf8(
        "background-color: %1; border-radius: 5px;")
                              .arg(QString::fromUtf8(swatches[i])));
    QLabel* name = new QLabel(QString::fromUtf8(names[i]), m_hud);
    name->setObjectName(QString::fromUtf8("axisLegendName"));
    legend->addWidget(swatch, 0, Qt::AlignVCenter);
    legend->addWidget(name, 0, Qt::AlignVCenter);
  }
  legend->addStretch(1);
  hudLay->addLayout(legend);

  m_demoCaption = new QLabel(m_hud);
  m_demoCaption->setObjectName(QString::fromUtf8("demoCaption"));
  m_demoCaption->setWordWrap(true);
  m_demoCaption->hide();
  hudLay->addWidget(m_demoCaption);
}

void Dx11ViewportWidget::updateHud() {
  if (!m_demoCaption) {
    return;
  }
  if (m_teaching.demoPlaying) {
    const TutorialStep step = tutorialStepAt(m_teaching.tutorialStep);
    m_demoCaption->setText(QString::fromUtf8("%1 / %2  %3\n%4")
                               .arg(m_teaching.tutorialStep + 1)
                               .arg(tutorialStepCount())
                               .arg(QString::fromUtf8(step.title))
                               .arg(QString::fromUtf8(step.body)));
    m_demoCaption->show();
  } else {
    m_demoCaption->hide();
  }
}

HWND Dx11ViewportWidget::hwnd() const {
  if (!m_surface) {
    return 0;
  }
  return reinterpret_cast<HWND>(m_surface->winId());
}
