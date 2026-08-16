#include "app/MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
  setWindowTitle(QString::fromUtf8("3DEditMax"));
  resize(1280, 720);
}
