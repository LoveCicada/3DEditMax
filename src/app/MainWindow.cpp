#include "app/MainWindow.h"
#include "teach/Transforms.h"
#include "ui/DebugLogPanel.h"
#include "ui/Dx11ViewportWidget.h"
#include "ui/MatrixBoardPanel.h"
#include "ui/TransformPanel.h"
#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QToolBar>
#include <DirectXMath.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewport(0)
    , m_transforms(0)
    , m_board(0)
    , m_log(0)
    , m_poll(0)
    , m_majorAction(0)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault())
    , m_major(MajorColumn) {
  setWindowTitle(QString::fromUtf8("3DEditMax"));
  resize(1280, 720);

  m_viewport = new Dx11ViewportWidget(this);
  m_transforms = new TransformPanel(this);
  m_board = new MatrixBoardPanel(this);
  m_log = new DebugLogPanel(this);
  setCentralWidget(m_viewport);

  QDockWidget* dockTransforms = new QDockWidget(QString::fromUtf8("\xE5\x8F\x98\xE6\x8D\xA2"), this);
  dockTransforms->setObjectName(QString::fromUtf8("dockTransforms"));
  dockTransforms->setWidget(m_transforms);
  addDockWidget(Qt::LeftDockWidgetArea, dockTransforms);

  QDockWidget* dockMatrix = new QDockWidget(QString::fromUtf8("\xE7\x9F\xA9\xE9\x98\xB5\xE7\x9C\x8B\xE6\x9D\xBF"), this);
  dockMatrix->setObjectName(QString::fromUtf8("dockMatrix"));
  dockMatrix->setWidget(m_board);
  addDockWidget(Qt::RightDockWidgetArea, dockMatrix);

  QDockWidget* dockDebug = new QDockWidget(QString::fromUtf8("DX11 Lab / Debug"), this);
  dockDebug->setObjectName(QString::fromUtf8("dockDebug"));
  dockDebug->setWidget(m_log);
  addDockWidget(Qt::RightDockWidgetArea, dockDebug);
  splitDockWidget(dockMatrix, dockDebug, Qt::Vertical);

  QToolBar* toolbar = addToolBar(QString::fromUtf8("Teach"));
  toolbar->setObjectName(QString::fromUtf8("toolbarTeach"));
  m_majorAction = toolbar->addAction(QString());
  QAction* resetAction = toolbar->addAction(QString::fromUtf8("\xE9\x87\x8D\xE7\xBD\xAE"));
  syncMajorActionText();

  QMenu* viewMenu = menuBar()->addMenu(QString::fromUtf8("\xE8\xA7\x86\xE5\x9B\xBE"));
  viewMenu->addAction(dockTransforms->toggleViewAction());
  viewMenu->addAction(dockMatrix->toggleViewAction());
  viewMenu->addAction(dockDebug->toggleViewAction());

  connect(m_transforms, &TransformPanel::changed, this, &MainWindow::onTransformsChanged);
  connect(m_viewport, &Dx11ViewportWidget::teachingEdited, this, &MainWindow::onTeachingEdited);
  connect(m_majorAction, &QAction::triggered, this, &MainWindow::onToggleMajor);
  connect(resetAction, &QAction::triggered, this, &MainWindow::onReset);

  m_transforms->setState(m_teaching);
  m_viewport->publishState(m_teaching, m_lab);
  refreshBoard();

  m_poll = new QTimer(this);
  m_poll->setInterval(100);
  connect(m_poll, &QTimer::timeout, this, &MainWindow::onPollFeedback);
  m_poll->start();
}

void MainWindow::onPollFeedback() {
  m_log->drain(m_viewport->feedback());
}

void MainWindow::onTransformsChanged() {
  m_teaching = m_transforms->state();
  m_viewport->publishState(m_teaching, m_lab);
  refreshBoard();
}

void MainWindow::onTeachingEdited(const TeachingState& t) {
  m_teaching = t;
  m_transforms->setState(m_teaching);
  refreshBoard();
}

void MainWindow::onToggleMajor() {
  m_major = (m_major == MajorColumn) ? MajorRow : MajorColumn;
  syncMajorActionText();
  m_board->setMajorOrder(m_major);
}

void MainWindow::onReset() {
  m_teaching = teachingStateDefault();
  m_transforms->setState(m_teaching);
  m_viewport->publishState(m_teaching, m_lab);
  refreshBoard();
}

void MainWindow::refreshBoard() {
  using namespace DirectX;
  const TeachingState& t = m_teaching;
  float aspect = t.aspect;
  if (t.aspectFollowViewport && m_viewport->height() > 0) {
    aspect = static_cast<float>(m_viewport->width()) /
             static_cast<float>(m_viewport->height());
  }
  const XMMATRIX W = BuildWorld(t.objects[0].trs);
  const XMMATRIX V = BuildView(t.camDistance, t.camPitchDeg, t.camYawDeg);
  const XMMATRIX P = BuildProjection(t, aspect);
  const XMMATRIX WVP = W * V * P;
  XMFLOAT4X4 w;
  XMFLOAT4X4 v;
  XMFLOAT4X4 p;
  XMFLOAT4X4 wvp;
  XMStoreFloat4x4(&w, W);
  XMStoreFloat4x4(&v, V);
  XMStoreFloat4x4(&p, P);
  XMStoreFloat4x4(&wvp, WVP);
  m_board->setMatrices(w, v, p, wvp, m_major);
}

void MainWindow::syncMajorActionText() {
  if (m_major == MajorColumn) {
    m_majorAction->setText(QString::fromUtf8("\xE5\x88\x97\xE4\xB8\xBB\xE5\xBA\x8F"));
  } else {
    m_majorAction->setText(QString::fromUtf8("\xE8\xA1\x8C\xE4\xB8\xBB\xE5\xBA\x8F"));
  }
}
