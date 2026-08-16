#pragma once
#include "core/TeachingState.h"
#include <QWidget>

class QLabel;
class QPushButton;

class TutorialPanel : public QWidget {
  Q_OBJECT
public:
  explicit TutorialPanel(QWidget* parent = 0);
  void setStepIndex(int index);
signals:
  void applyState(TeachingState state);
private slots:
  void onPrev();
  void onNext();
  void onDemo();
private:
  void refresh();

  int m_index;
  QLabel* m_title;
  QLabel* m_body;
  QLabel* m_step;
  QPushButton* m_prev;
  QPushButton* m_next;
};
