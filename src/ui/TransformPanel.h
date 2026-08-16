#pragma once
#include "core/TeachingState.h"
#include <QWidget>

class QComboBox;
class QDoubleSpinBox;

class TransformPanel : public QWidget {
  Q_OBJECT
public:
  explicit TransformPanel(QWidget* parent = 0);
  void setState(const TeachingState& s);
  TeachingState state() const;
signals:
  void changed();
private slots:
  void onEdited();
  void onPresetFront();
  void onPresetSide();
  void onPresetTop();
  void onPresetIso();
private:
  QDoubleSpinBox* makeSpin(double minv, double maxv, double step, int decimals);
  void applyViewPreset(double pitch, double yaw);
  void loadWidgets(const TeachingState& s);

  TeachingState m_state;
  bool m_block;
  QDoubleSpinBox* m_posX;
  QDoubleSpinBox* m_posY;
  QDoubleSpinBox* m_posZ;
  QDoubleSpinBox* m_pitch;
  QDoubleSpinBox* m_yaw;
  QDoubleSpinBox* m_roll;
  QDoubleSpinBox* m_scaleX;
  QDoubleSpinBox* m_scaleY;
  QDoubleSpinBox* m_scaleZ;
  QDoubleSpinBox* m_camDistance;
  QDoubleSpinBox* m_camPitch;
  QDoubleSpinBox* m_camYaw;
  QComboBox* m_proj;
  QDoubleSpinBox* m_fov;
  QDoubleSpinBox* m_nearZ;
  QDoubleSpinBox* m_farZ;
};
