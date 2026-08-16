#include "app/MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStyleFactory>

static void applyAppStyleSheet(QApplication& app) {
  app.setStyle(QStyleFactory::create(QString::fromUtf8("Fusion")));
  const QString qssPath =
      QCoreApplication::applicationDirPath() + QString::fromUtf8("/app.qss");
  QFile qssFile(qssPath);
  if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    app.setStyleSheet(QString::fromUtf8(qssFile.readAll()));
  }
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  applyAppStyleSheet(app);
  MainWindow w;
  w.show();
  return app.exec();
}
