#include "app/MainWindow.h"
#include "teach/JsonIo.h"
#include "teach/Transforms.h"
#include "teach/TutorialScript.h"
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
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
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
    , m_demoBar(0)
    , m_demoBarStep(0)
    , m_demoBarTitle(0)
    , m_demoBarBody(0)
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

  // Tutorial strip lives in the central column so QMainWindow dock sizing
  // cannot collapse it to height 0 (resizeDocks will not revive a 0-height dock).
  QWidget* central = new QWidget(this);
  QVBoxLayout* centralLay = new QVBoxLayout(central);
  centralLay->setContentsMargins(0, 0, 0, 0);
  centralLay->setSpacing(0);
  centralLay->addWidget(m_viewport, 1);
  QFrame* tutorialStrip = new QFrame(central);
  tutorialStrip->setObjectName(QString::fromUtf8("tutorialStrip"));
  tutorialStrip->setMinimumHeight(128);
  QVBoxLayout* stripLay = new QVBoxLayout(tutorialStrip);
  stripLay->setContentsMargins(8, 4, 8, 6);
  stripLay->setSpacing(0);
  stripLay->addWidget(m_tutorial);
  centralLay->addWidget(tutorialStrip, 0);
  setCentralWidget(central);

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

  QTimer::singleShot(0, this, [this, dockTransforms, dockObject, dockMatrix]() {
    resizeDocks({dockTransforms, dockObject}, {300, 300}, Qt::Horizontal);
    resizeDocks({dockMatrix}, {380}, Qt::Horizontal);
  });

  QToolBar* toolbar = addToolBar(QString::fromUtf8("Teach"));
  toolbar->setObjectName(QString::fromUtf8("toolbarTeach"));
  m_majorAction = toolbar->addAction(QString());
  QAction* resetAction = toolbar->addAction(QString::fromUtf8("\xE9\x87\x8D\xE7\xBD\xAE"));
  syncMajorActionText();

  m_demoBar = new QWidget(toolbar);
  m_demoBar->setObjectName(QString::fromUtf8("demoBar"));
  QHBoxLayout* demoLay = new QHBoxLayout(m_demoBar);
  demoLay->setContentsMargins(8, 2, 8, 2);
  demoLay->setSpacing(8);
  m_demoBarStep = new QLabel(m_demoBar);
  m_demoBarStep->setObjectName(QString::fromUtf8("demoBarStep"));
  m_demoBarTitle = new QLabel(m_demoBar);
  m_demoBarTitle->setObjectName(QString::fromUtf8("demoBarTitle"));
  m_demoBarBody = new QLabel(m_demoBar);
  m_demoBarBody->setObjectName(QString::fromUtf8("demoBarBody"));
  m_demoBarBody->setMaximumWidth(360);
  demoLay->addWidget(m_demoBarStep);
  demoLay->addWidget(m_demoBarTitle);
  demoLay->addWidget(m_demoBarBody, 1);
  toolbar->addWidget(m_demoBar);

  auto addShellTool = [this, toolbar](const char* tipUtf8) {
    const QString tip = QString::fromUtf8(tipUtf8);
    QAction* a = toolbar->addAction(tip);
    a->setToolTip(tip + QString::fromUtf8(" (\xE6\x9C\xAA\xE5\xAE\x9E\xE7\x8E\xB0)"));
    connect(a, &QAction::triggered, this, [this, tip]() {
      statusBar()->showMessage(
          tip + QString::fromUtf8(" — \xE6\x9C\xAA\xE5\xAE\x9E\xE7\x8E\xB0"), 1500);
    });
  };
  toolbar->addSeparator();
  addShellTool("Select");
  addShellTool("Pan");
  addShellTool("Orbit");
  addShellTool("Dolly");
  toolbar->addSeparator();
  addShellTool("Solid");
  addShellTool("Wire");
  addShellTool("Track");
  toolbar->addSeparator();
  addShellTool("Cube");
  addShellTool("Sphere");
  addShellTool("Cylinder");
  toolbar->addSeparator();
  addShellTool("Record");
  addShellTool("Perf");
  addShellTool("Shot");

  QMenu* fileMenu = menuBar()->addMenu(QString::fromUtf8("\xE6\x96\x87\xE4\xBB\xB6"));
  QAction* importAction = fileMenu->addAction(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5"));
  QAction* exportAction = fileMenu->addAction(QString::fromUtf8("\xE5\xAF\xBC\xE5\x87\xBA"));

  QMenu* viewMenu = menuBar()->addMenu(QString::fromUtf8("\xE8\xA7\x86\xE5\x9B\xBE"));
  viewMenu->addAction(dockTransforms->toggleViewAction());
  viewMenu->addAction(dockObject->toggleViewAction());
  viewMenu->addAction(dockMatrix->toggleViewAction());
  viewMenu->addAction(dockTracker->toggleViewAction());
  viewMenu->addAction(dockDebug->toggleViewAction());

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
  refreshDemoBar();
}

void MainWindow::refreshDemoBar() {
  if (!m_demoBarStep || !m_demoBarTitle || !m_demoBarBody) {
    return;
  }
  const TutorialStep step = tutorialStepAt(m_teaching.tutorialStep);
  m_demoBarStep->setText(QString::fromUtf8("%1 / %2")
                             .arg(m_teaching.tutorialStep + 1)
                             .arg(tutorialStepCount()));
  m_demoBarTitle->setText(QString::fromUtf8(step.title));
  m_demoBarBody->setText(QString::fromUtf8(step.body));
  m_demoBarBody->setToolTip(QString::fromUtf8(step.body));
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
    m_tutorial->setStepIndex(m_teaching.tutorialStep);
    m_tutorial->setDemoPlaying(true);
    syncTeaching();
    m_demoTimer->start();
    return;
  }
  m_demoTimer->stop();
  m_teaching = t;
  m_teaching.demoPlaying = false;
  m_tutorial->setDemoPlaying(false);
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
  m_tutorial->setDemoPlaying(false);
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
  m_tutorial->setDemoPlaying(false);
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
    m_tutorial->setDemoPlaying(false);
    return;
  }
  if (!m_demo.tick(0.016f, &m_teaching)) {
    m_teaching.demoPlaying = false;
    m_demoTimer->stop();
    m_tutorial->setDemoPlaying(false);
  } else {
    m_tutorial->setDemoPlaying(true);
  }
  m_tutorial->setStepIndex(m_teaching.tutorialStep);
  syncTeaching();
}

void MainWindow::refreshBoard() {
  using namespace DirectX;
  const TeachingState& t = m_teaching;
  float aspect = t.aspect;
  if (t.aspectFollowViewport && m_viewport->surfaceHeight() > 0) {
    aspect = static_cast<float>(m_viewport->surfaceWidth()) /
             static_cast<float>(m_viewport->surfaceHeight());
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
  m_board->setDemoFocus(demoMatrixFocus(t.tutorialStep, t.demoPlaying));

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
  if (t.aspectFollowViewport && m_viewport->surfaceHeight() > 0) {
    aspect = static_cast<float>(m_viewport->surfaceWidth()) /
             static_cast<float>(m_viewport->surfaceHeight());
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
