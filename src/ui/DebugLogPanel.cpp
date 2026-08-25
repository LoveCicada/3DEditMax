#include "ui/DebugLogPanel.h"
#include "render/FeedbackQueue.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>

DebugLogPanel::DebugLogPanel(QWidget* parent)
    : QWidget(parent) {
  m_edit = new QPlainTextEdit(this);
  m_edit->setObjectName(QString::fromUtf8("debugLog"));
  m_edit->setReadOnly(true);
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_edit);
}

void DebugLogPanel::drain(FeedbackQueue& q) {
  FeedbackItem item;
  while (q.tryPop(&item)) {
    if (item.kind == FbFps) {
      continue;
    }
    if (item.kind == FbLog && item.text.size() >= 9 && item.text.compare(0, 9, "Adapter: ") == 0) {
      m_adapter = QString::fromUtf8(item.text.c_str() + 9);
    }
    const char* tag = 0;
    if (item.kind == FbError) {
      tag = "[Error] ";
    } else if (item.kind == FbWarn) {
      tag = "[Warn] ";
    } else if (item.kind == FbLog) {
      tag = "[Log] ";
    } else {
      continue;
    }
    m_edit->appendPlainText(QString::fromUtf8(tag) + QString::fromUtf8(item.text.c_str()));
  }
}

QString DebugLogPanel::adapterName() const {
  return m_adapter;
}

void DebugLogPanel::appendFromQueue(FeedbackQueue& q) {
  drain(q);
}
