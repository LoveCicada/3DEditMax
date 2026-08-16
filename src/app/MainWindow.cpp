#include "app/MainWindow.h"
#include "teach/JsonIo.h"
#include "teach/Transforms.h"
#include "ui/DebugLogPanel.h"
#include "ui/Dx11ViewportWidget.h"
#include "ui/LabPanel.h"
#include "ui/MatrixBoardPanel.h"
#include "ui/ObjectPanel.h"
#include "ui/TrackerPanel.h"
#include "ui/TransformPanel.h"
#include "ui/TutorialPanel.h"
#include <QAction>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <DirectXMath.h>
#include <string>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewport(0)
    , m_transforms(0)
    , m_objects(0)
    , m_tracker(0)
    , m_board(0)
    , m_labPanel(0)
    , m_log(0)
    , m_tutorial(0)
    , m_poll(0)
    , m_demoTimer(0)
    , m_majorAction(0)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault())
    , m_major(MajorColumn) {
  setWindowTitle(QString::fromUtf8("3DEditMax"));
  resize(1280, 720);

  m_viewport = new Dx11ViewportWidget(this);
  m_transforms = new TransformPanel(this);
  m_objects = new ObjectPanel(this);
  m_tracker = new TrackerPanel(this);
  m_board = new MatrixBoardPanel(this);
  m_labPanel = new LabPanel(this);
  m_log = new DebugLogPanel(this);
  m_tutorial = new TutorialPanel(this);
  setCentralWidget(m_viewport);

  QDockWidget* dockTransforms = new QDockWidget(QString::fromUtf8("\xE5\x8F\x98\xE6\x8D\xA2"), this);
  dockTransforms->setObjectName(QString::fromUtf8("dockTransforms"));
  dockTransforms->setWidget(m_transforms);
  addDockWidget(Qt::LeftDockWidgetArea, dockTransforms);

  QDockWidget* dockObject = new QDockWidget(QString::fromUtf8("\xE7\x89\xA9\xE4\xBD\x93"), this);
  dockObject->setObjectName(QString::fromUtf8("dockObject"));
  dockObject->setWidget(m_objects);
  addDockWidget(Qt::LeftDockWidgetArea, dockObject);
  splitDockWidget(dockTransforms, dockObject, Qt::Vertical);

  QDockWidget* dockMatrix = new QDockWidget(QString::fromUtf8("\xE7\x9F\xA9\xE9\x98\xB5\xE7\x9C\x8B\xE6\x9D\xBF"), this);
  dockMatrix->setObjectName(QString::fromUtf8("dockMatrix"));
  dockMatrix->setWidget(m_board);
  addDockWidget(Qt::RightDockWidgetArea, dockMatrix);

  QDockWidget* dockTracker = new QDockWidget(QString::fromUtf8("Tracker"), this);
  dockTracker->setObjectName(QString::fromUtf8("dockTracker"));
  dockTracker->setWidget(m_tracker);
  addDockWidget(Qt::RightDockWidgetArea, dockTracker);
  splitDockWidget(dockMatrix, dockTracker, Qt::Vertical);

  QWidget* labDockInner = new QWidget(this);
  QVBoxLayout* labDockLay = new QVBoxLayout(labDockInner);
  labDockLay->setContentsMargins(0, 0, 0, 0);
  labDockLay->addWidget(m_labPanel, 1);
  labDockLay->addWidget(m_log, 1);

  QDockWidget* dockDebug = new QDockWidget(QString::fromUtf8("DX11 Lab / Debug"), this);
  dockDebug->setObjectName(QString::fromUtf8("dockDebug"));
  dockDebug->setWidget(labDockInner);
  addDockWidget(Qt::RightDockWidgetArea, dockDebug);
  splitDockWidget(dockMatrix, dockDebug, Qt::Vertical);

  QDockWidget* dockTutorial = new QDockWidget(QString::fromUtf8("\xE6\x95\x99\xE7\xA8\x8B"), this);
  dockTutorial->setObjectName(QString::fromUtf8("dockTutorial"));
  dockTutorial->setWidget(m_tutorial);
  addDockWidget(Qt::BottomDockWidgetArea, dockTutorial);

  QToolBar* toolbar = addToolBar(QString::fromUtf8("Teach"));
  toolbar->setObjectName(QString::fromUtf8("toolbarTeach"));
  m_majorAction = toolbar->addAction(QString());
  QAction* resetAction = toolbar->addAction(QString::fromUtf8("\xE9\x87\x8D\xE7\xBD\xAE"));
  syncMajorActionText();

  QMenu* fileMenu = menuBar()->addMenu(QString::fromUtf8("\xE6\x96\x87\xE4\xBB\xB6"));
  QAction* importAction = fileMenu->addAction(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5"));
  QAction* exportAction = fileMenu->addAction(QString::fromUtf8("\xE5\xAF\xBC\xE5\x87\xBA"));

  QMenu* viewMenu = menuBar()->addMenu(QString::fromUtf8("\xE8\xA7\x86\xE5\x9B\xBE"));
  viewMenu->addAction(dockTransforms->toggleViewAction());
  viewMenu->addAction(dockObject->toggleViewAction());
  viewMenu->addAction(dockMatrix->toggleViewAction());
  viewMenu->addAction(dockTracker->toggleViewAction());
  viewMenu->addAction(dockDebug->toggleViewAction());
  viewMenu->addAction(dockTutorial->toggleViewAction());

  connect(m_transforms, &TransformPanel::changed, this, &MainWindow::onTransformsChanged);
  connect(m_objects, &ObjectPanel::changed, this, &MainWindow::onObjectsChanged);
  connect(m_tracker, &TrackerPanel::changed, this, &MainWindow::onTrackerChanged);
  connect(m_viewport, &Dx11ViewportWidget::teachingEdited, this, &MainWindow::onTeachingEdited);
  connect(m_tutorial, &TutorialPanel::applyState, this, &MainWindow::onTutorialApply);
  connect(m_labPanel, &LabPanel::changed, this, &MainWindow::onLabChanged);
  connect(m_labPanel, &LabPanel::reloadShaders, m_viewport, &Dx11ViewportWidget::reloadShaders);
  connect(m_majorAction, &QAction::triggered, this, &MainWindow::onToggleMajor);
  connect(resetAction, &QAction::triggered, this, &MainWindow::onReset);
  connect(importAction, &QAction::triggered, this, &MainWindow::onImportJson);
  connect(exportAction, &QAction::triggered, this, &MainWindow::onExportJson);

  m_tutorial->setStepIndex(m_teaching.tutorialStep);
  syncTeaching();

  m_poll = new QTimer(this);
  m_poll->setInterval(100);
  connect(m_poll, &QTimer::timeout, this, &MainWindow::onPollFeedback);
  m_poll->start();

  m_demoTimer = new QTimer(this);
  m_demoTimer->setInterval(16);
  connect(m_demoTimer, &QTimer::timeout, this, &MainWindow::onDemoTick);
}

void MainWindow::onPollFeedback() {
  m_log->drain(m_viewport->feedback());
  if (!m_log->adapterName().isEmpty()) {
    m_labPanel->setDeviceDescription(m_log->adapterName());
  }
}

void MainWindow::syncTeaching() {
  m_transforms->setState(m_teaching);
  m_objects->setState(m_teaching);
  m_tracker->setState(m_teaching);
  m_viewport->publishState(m_teaching, m_lab);
  refreshBoard();
  refreshTracker();
}

void MainWindow::onTransformsChanged() {
  m_teaching = m_transforms->state();
  syncTeaching();
}

void MainWindow::onObjectsChanged() {
  m_teaching = m_objects->state();
  syncTeaching();
}

void MainWindow::onTrackerChanged() {
  m_teaching = m_tracker->state();
  syncTeaching();
}

void MainWindow::onTeachingEdited(const TeachingState& t) {
  m_teaching = t;
  m_transforms->setState(m_teaching);
  m_objects->setState(m_teaching);
  m_tracker->setState(m_teaching);
  refreshBoard();
  refreshTracker();
}

void MainWindow::onLabChanged() {
  m_lab = m_labPanel->state();
  m_viewport->publishState(m_teaching, m_lab);
}

void MainWindow::onTutorialApply(const TeachingState& t) {
  if (t.demoPlaying) {
    m_teaching.demoPlaying = true;
    m_demo.start(m_teaching);
    m_demo.tick(0.f, &m_teaching);
    syncTeaching();
    m_demoTimer->start();
    return;
  }
  m_demoTimer->stop();
  m_teaching = t;
  m_teaching.demoPlaying = false;
  syncTeaching();
}

void MainWindow::onToggleMajor() {
  m_major = (m_major == MajorColumn) ? MajorRow : MajorColumn;
  syncMajorActionText();
  m_board->setMajorOrder(m_major);
  refreshBoard();
}

void MainWindow::onReset() {
  m_demoTimer->stop();
  m_teaching = teachingStateDefault();
  m_lab = labStateDefault();
  m_labPanel->setState(m_lab);
  m_tutorial->setStepIndex(0);
  syncTeaching();
}

void MainWindow::onImportJson() {
  const QString path = QFileDialog::getOpenFileName(
      this,
      QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5"),
      QString(),
      QString::fromUtf8("JSON (*.json)"));
  if (path.isEmpty()) {
    return;
  }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this,
                         QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5"),
                         QString::fromUtf8("JSON \xE6\x97\xA0\xE6\x95\x88"));
    return;
  }
  const QByteArray bytes = file.readAll();
  const std::string in(bytes.constData(), static_cast<size_t>(bytes.size()));
  TeachingState s;
  if (!teachingFromJson(in, &s)) {
    QMessageBox::warning(this,
                         QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5"),
                         QString::fromUtf8("JSON \xE6\x97\xA0\xE6\x95\x88"));
    return;
  }
  m_demoTimer->stop();
  s.demoPlaying = false;
  m_teaching = s;
  syncTeaching();
}

void MainWindow::onExportJson() {
  const QString path = QFileDialog::getSaveFileName(
      this,
      QString::fromUtf8("\xE5\xAF\xBC\xE5\x87\xBA"),
      QString(),
      QString::fromUtf8("JSON (*.json)"));
  if (path.isEmpty()) {
    return;
  }
  std::string json;
  if (!teachingToJson(m_teaching, &json)) {
    return;
  }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QMessageBox::warning(this,
                         QString::fromUtf8("\xE5\xAF\xBC\xE5\x87\xBA"),
                         QString::fromUtf8("JSON \xE6\x97\xA0\xE6\x95\x88"));
    return;
  }
  file.write(json.data(), static_cast<qint64>(json.size()));
}

void MainWindow::onDemoTick() {
  if (!m_teaching.demoPlaying) {
    m_demoTimer->stop();
    return;
  }
  if (!m_demo.tick(0.016f, &m_teaching)) {
    m_teaching.demoPlaying = false;
    m_demoTimer->stop();
  }
  syncTeaching();
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
  const XMMATRIX V = BuildView(t.camDistance, t.camPitchDeg, t.camYawDeg, t.camTarget);
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

  char lines[4][64];
  QString cb;
  const char* titles[4] = { "W", "V", "P", "WVP" };
  const XMFLOAT4X4* mats[4] = { &w, &v, &p, &wvp };
  for (int mi = 0; mi < 4; ++mi) {
    if (mi > 0) {
      cb += QChar::fromLatin1('\n');
    }
    cb += QString::fromUtf8(titles[mi]);
    cb += QChar::fromLatin1('\n');
    formatMatrix4(*mats[mi], m_major, lines);
    for (int i = 0; i < 4; ++i) {
      cb += QString::fromUtf8(lines[i]);
      cb += QChar::fromLatin1('\n');
    }
  }
  m_labPanel->setCbText(cb);
}

void MainWindow::refreshTracker() {
  using namespace DirectX;
  const TeachingState& t = m_teaching;
  float aspect = t.aspect;
  if (t.aspectFollowViewport && m_viewport->height() > 0) {
    aspect = static_cast<float>(m_viewport->width()) /
             static_cast<float>(m_viewport->height());
  }
  const XMMATRIX W = BuildWorld(t.objects[0].trs);
  const XMMATRIX V = BuildView(t.camDistance, t.camPitchDeg, t.camYawDeg, t.camTarget);
  const XMMATRIX P = BuildProjection(t, aspect);
  const XMFLOAT3 model(t.trackModel[0], t.trackModel[1], t.trackModel[2]);
  m_tracker->setTrackResult(TrackPoint(model, W, V, P));
}

void MainWindow::syncMajorActionText() {
  if (m_major == MajorColumn) {
    m_majorAction->setText(QString::fromUtf8("\xE5\x88\x97\xE4\xB8\xBB\xE5\xBA\x8F"));
  } else {
    m_majorAction->setText(QString::fromUtf8("\xE8\xA1\x8C\xE4\xB8\xBB\xE5\xBA\x8F"));
  }
}
