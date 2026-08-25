#include "ui/TutorialPanel.h"
#include "teach/TutorialScript.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

TutorialPanel::TutorialPanel(QWidget* parent)
    : QWidget(parent)
    , m_index(0)
    , m_demoPlaying(false)
    , m_title(0)
    , m_body(0)
    , m_step(0)
    , m_prev(0)
    , m_next(0)
    , m_demo(0) {
  setMinimumHeight(110);
  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(4);

  QHBoxLayout* row = new QHBoxLayout();
  m_prev = new QPushButton(QString::fromUtf8("\xE4\xB8\x8A\xE4\xB8\x80\xE6\xAD\xA5"), this);
  m_next = new QPushButton(QString::fromUtf8("\xE4\xB8\x8B\xE4\xB8\x80\xE6\xAD\xA5"), this);
  m_demo = new QPushButton(QString::fromUtf8("\xE6\xBC\x94\xE7\xA4\xBA"), this);
  m_demo->setObjectName(QString::fromUtf8("btnDemo"));
  m_step = new QLabel(this);
  row->addWidget(m_prev);
  row->addWidget(m_next);
  row->addWidget(m_demo);
  row->addStretch(1);
  row->addWidget(m_step);
  root->addLayout(row);

  m_title = new QLabel(this);
  QFont titleFont = m_title->font();
  titleFont.setBold(true);
  m_title->setFont(titleFont);
  m_body = new QLabel(this);
  m_body->setWordWrap(true);
  root->addWidget(m_title);
  root->addWidget(m_body);

  connect(m_prev, &QPushButton::clicked, this, &TutorialPanel::onPrev);
  connect(m_next, &QPushButton::clicked, this, &TutorialPanel::onNext);
  connect(m_demo, &QPushButton::clicked, this, &TutorialPanel::onDemo);

  refresh();
}

void TutorialPanel::setStepIndex(int index) {
  const int n = tutorialStepCount();
  if (index < 0) {
    index = 0;
  }
  if (n > 0 && index >= n) {
    index = n - 1;
  }
  m_index = index;
  refresh();
}

void TutorialPanel::setDemoPlaying(bool playing) {
  m_demoPlaying = playing;
  refresh();
}

void TutorialPanel::onPrev() {
  if (m_demoPlaying || m_index <= 0) {
    return;
  }
  --m_index;
  refresh();
  emit applyState(tutorialStepAt(m_index).state);
}

void TutorialPanel::onNext() {
  if (m_demoPlaying || m_index + 1 >= tutorialStepCount()) {
    return;
  }
  ++m_index;
  refresh();
  emit applyState(tutorialStepAt(m_index).state);
}

void TutorialPanel::onDemo() {
  if (m_demoPlaying) {
    TeachingState s = tutorialStepAt(m_index).state;
    s.demoPlaying = false;
    emit applyState(s);
    return;
  }
  TeachingState s = tutorialStepAt(m_index).state;
  s.demoPlaying = true;
  emit applyState(s);
}

void TutorialPanel::refresh() {
  const TutorialStep step = tutorialStepAt(m_index);
  m_title->setText(QString::fromUtf8(step.title));
  m_body->setText(QString::fromUtf8(step.body));
  m_step->setText(QString::fromUtf8("%1 / %2").arg(m_index + 1).arg(tutorialStepCount()));
  m_prev->setEnabled(!m_demoPlaying && m_index > 0);
  m_next->setEnabled(!m_demoPlaying && m_index + 1 < tutorialStepCount());
  if (m_demoPlaying) {
    m_demo->setText(QString::fromUtf8("\xE5\x81\x9C\xE6\xAD\xA2"));
  } else {
    m_demo->setText(QString::fromUtf8("\xE6\xBC\x94\xE7\xA4\xBA"));
  }
}
