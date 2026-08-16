#include "app/MainWindow.h"
#include "ui/DebugLogPanel.h"
#include "ui/Dx11ViewportWidget.h"
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewport(0)
    , m_log(0)
    , m_poll(0)
    , m_teaching(teachingStateDefault())
    , m_lab(labStateDefault()) {
  setWindowTitle(QString::fromUtf8("3DEditMax"));
  resize(1280, 720);

  m_viewport = new Dx11ViewportWidget(this);
  m_log = new DebugLogPanel(this);
  setCentralWidget(m_viewport);

  QDockWidget* dock = new QDockWidget(QString::fromUtf8("DX11 Lab / Debug"), this);
  dock->setObjectName(QString::fromUtf8("dockDebug"));
  dock->setWidget(m_log);
  addDockWidget(Qt::RightDockWidgetArea, dock);

  QMenu* viewMenu = menuBar()->addMenu(QString::fromUtf8("视图"));
  viewMenu->addAction(dock->toggleViewAction());

  m_poll = new QTimer(this);
  m_poll->setInterval(100);
  connect(m_poll, &QTimer::timeout, this, &MainWindow::onPollFeedback);
  m_poll->start();
}

void MainWindow::onPollFeedback() {
  m_log->drain(m_viewport->feedback());
}
