#pragma once
#include "core/TeachingState.h"
#include "teach/Transforms.h"
#include <QWidget>

class QDoubleSpinBox;
class QLabel;

class TrackerPanel : public QWidget {
  Q_OBJECT
public:
  explicit TrackerPanel(QWidget* parent = 0);
  void setState(const TeachingState& s);
  TeachingState state() const;
  void setTrackResult(const TrackResult& tr);
signals:
  void changed();
private slots:
  void onEdited();
  void onPreset();
private:
  QDoubleSpinBox* makeSpin();
  void loadWidgets(const TeachingState& s);

  TeachingState m_state;
  bool m_block;
  QDoubleSpinBox* m_x;
  QDoubleSpinBox* m_y;
  QDoubleSpinBox* m_z;
  QLabel* m_world;
  QLabel* m_view;
  QLabel* m_clip;
  QLabel* m_ndc;
};
