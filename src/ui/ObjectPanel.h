#pragma once
#include "core/TeachingState.h"
#include <QWidget>

class QButtonGroup;
class QComboBox;

class ObjectPanel : public QWidget {
  Q_OBJECT
public:
  explicit ObjectPanel(QWidget* parent = 0);
  void setState(const TeachingState& s);
  TeachingState state() const;
signals:
  void changed();
private slots:
  void onEdited();
private:
  void loadWidgets(const TeachingState& s);

  TeachingState m_state;
  bool m_block;
  QButtonGroup* m_mesh;
  QComboBox* m_layout;
  QComboBox* m_shading;
};
