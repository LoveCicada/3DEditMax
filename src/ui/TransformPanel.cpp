#include "ui/TransformPanel.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

TransformPanel::TransformPanel(QWidget* parent)
    : QWidget(parent)
    , m_state(teachingStateDefault())
    , m_block(false)
    , m_posX(0)
    , m_posY(0)
    , m_posZ(0)
    , m_pitch(0)
    , m_yaw(0)
    , m_roll(0)
    , m_scaleX(0)
    , m_scaleY(0)
    , m_scaleZ(0)
    , m_camDistance(0)
    , m_camPitch(0)
    , m_camYaw(0)
    , m_proj(0)
    , m_fov(0)
    , m_nearZ(0)
    , m_farZ(0) {
  QVBoxLayout* root = new QVBoxLayout(this);

  QGroupBox* world = new QGroupBox(QString::fromUtf8("World"), this);
  world->setObjectName(QString::fromUtf8("sectionWorld"));
  QFormLayout* worldForm = new QFormLayout(world);
  m_posX = makeSpin(-50.0, 50.0, 0.1, 1);
  m_posY = makeSpin(-50.0, 50.0, 0.1, 1);
  m_posZ = makeSpin(-50.0, 50.0, 0.1, 1);
  m_pitch = makeSpin(-180.0, 180.0, 1.0, 0);
  m_yaw = makeSpin(-180.0, 180.0, 1.0, 0);
  m_roll = makeSpin(-180.0, 180.0, 1.0, 0);
  m_scaleX = makeSpin(0.01, 10.0, 0.01, 2);
  m_scaleY = makeSpin(0.01, 10.0, 0.01, 2);
  m_scaleZ = makeSpin(0.01, 10.0, 0.01, 2);
  worldForm->addRow(QString::fromUtf8("Pos X"), m_posX);
  worldForm->addRow(QString::fromUtf8("Pos Y"), m_posY);
  worldForm->addRow(QString::fromUtf8("Pos Z"), m_posZ);
  worldForm->addRow(QString::fromUtf8("Pitch"), m_pitch);
  worldForm->addRow(QString::fromUtf8("Yaw"), m_yaw);
  worldForm->addRow(QString::fromUtf8("Roll"), m_roll);
  worldForm->addRow(QString::fromUtf8("Scale X"), m_scaleX);
  worldForm->addRow(QString::fromUtf8("Scale Y"), m_scaleY);
  worldForm->addRow(QString::fromUtf8("Scale Z"), m_scaleZ);
  root->addWidget(world);

  QGroupBox* view = new QGroupBox(QString::fromUtf8("View"), this);
  view->setObjectName(QString::fromUtf8("sectionView"));
  QFormLayout* viewForm = new QFormLayout(view);
  m_camDistance = makeSpin(0.5, 50.0, 0.1, 1);
  m_camPitch = makeSpin(-89.0, 89.0, 1.0, 0);
  m_camYaw = makeSpin(-180.0, 180.0, 1.0, 0);
  viewForm->addRow(QString::fromUtf8("Distance"), m_camDistance);
  viewForm->addRow(QString::fromUtf8("Pitch"), m_camPitch);
  viewForm->addRow(QString::fromUtf8("Yaw"), m_camYaw);
  QHBoxLayout* presets = new QHBoxLayout();
  QPushButton* front = new QPushButton(QString::fromUtf8("Front"), view);
  QPushButton* side = new QPushButton(QString::fromUtf8("Side"), view);
  QPushButton* top = new QPushButton(QString::fromUtf8("Top"), view);
  QPushButton* iso = new QPushButton(QString::fromUtf8("Iso"), view);
  presets->addWidget(front);
  presets->addWidget(side);
  presets->addWidget(top);
  presets->addWidget(iso);
  viewForm->addRow(presets);
  root->addWidget(view);

  QGroupBox* proj = new QGroupBox(QString::fromUtf8("Projection"), this);
  proj->setObjectName(QString::fromUtf8("sectionProjection"));
  QFormLayout* projForm = new QFormLayout(proj);
  m_proj = new QComboBox(proj);
  m_proj->addItem(QString::fromUtf8("Perspective"));
  m_proj->addItem(QString::fromUtf8("Ortho"));
  m_fov = makeSpin(10.0, 120.0, 1.0, 0);
  m_nearZ = makeSpin(0.01, 10.0, 0.01, 2);
  m_farZ = makeSpin(1.0, 1000.0, 1.0, 1);
  projForm->addRow(QString::fromUtf8("Mode"), m_proj);
  projForm->addRow(QString::fromUtf8("FOV"), m_fov);
  projForm->addRow(QString::fromUtf8("Near"), m_nearZ);
  projForm->addRow(QString::fromUtf8("Far"), m_farZ);
  root->addWidget(proj);
  root->addStretch(1);

  connect(front, &QPushButton::clicked, this, &TransformPanel::onPresetFront);
  connect(side, &QPushButton::clicked, this, &TransformPanel::onPresetSide);
  connect(top, &QPushButton::clicked, this, &TransformPanel::onPresetTop);
  connect(iso, &QPushButton::clicked, this, &TransformPanel::onPresetIso);
  connect(m_proj, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &TransformPanel::onEdited);

  m_block = true;
  loadWidgets(m_state);
  m_block = false;
}

QDoubleSpinBox* TransformPanel::makeSpin(double minv, double maxv, double step, int decimals) {
  QDoubleSpinBox* s = new QDoubleSpinBox(this);
  s->setRange(minv, maxv);
  s->setSingleStep(step);
  s->setDecimals(decimals);
  connect(s, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
          this, &TransformPanel::onEdited);
  return s;
}

void TransformPanel::setState(const TeachingState& s) {
  m_state = s;
  const bool was = m_block;
  m_block = true;
  loadWidgets(s);
  m_block = was;
}

TeachingState TransformPanel::state() const {
  TeachingState s = m_state;
  s.objects[0].trs.pos[0] = static_cast<float>(m_posX->value());
  s.objects[0].trs.pos[1] = static_cast<float>(m_posY->value());
  s.objects[0].trs.pos[2] = static_cast<float>(m_posZ->value());
  s.objects[0].trs.pitchDeg = static_cast<float>(m_pitch->value());
  s.objects[0].trs.yawDeg = static_cast<float>(m_yaw->value());
  s.objects[0].trs.rollDeg = static_cast<float>(m_roll->value());
  s.objects[0].trs.scale[0] = static_cast<float>(m_scaleX->value());
  s.objects[0].trs.scale[1] = static_cast<float>(m_scaleY->value());
  s.objects[0].trs.scale[2] = static_cast<float>(m_scaleZ->value());
  s.camDistance = static_cast<float>(m_camDistance->value());
  s.camPitchDeg = static_cast<float>(m_camPitch->value());
  s.camYawDeg = static_cast<float>(m_camYaw->value());
  s.proj = (m_proj->currentIndex() == 1) ? ProjOrtho : ProjPerspective;
  s.fovDeg = static_cast<float>(m_fov->value());
  s.nearZ = static_cast<float>(m_nearZ->value());
  s.farZ = static_cast<float>(m_farZ->value());
  return s;
}

void TransformPanel::onEdited() {
  if (m_block) {
    return;
  }
  emit changed();
}

void TransformPanel::onPresetFront() {
  applyViewPreset(0.0, 0.0);
}

void TransformPanel::onPresetSide() {
  applyViewPreset(0.0, 90.0);
}

void TransformPanel::onPresetTop() {
  applyViewPreset(89.0, 0.0);
}

void TransformPanel::onPresetIso() {
  applyViewPreset(20.0, 45.0);
}

void TransformPanel::applyViewPreset(double pitch, double yaw) {
  const bool was = m_block;
  m_block = true;
  m_camPitch->setValue(pitch);
  m_camYaw->setValue(yaw);
  m_block = was;
  if (!m_block) {
    emit changed();
  }
}

void TransformPanel::loadWidgets(const TeachingState& s) {
  const TransformTRS& trs = s.objects[0].trs;
  m_posX->setValue(trs.pos[0]);
  m_posY->setValue(trs.pos[1]);
  m_posZ->setValue(trs.pos[2]);
  m_pitch->setValue(trs.pitchDeg);
  m_yaw->setValue(trs.yawDeg);
  m_roll->setValue(trs.rollDeg);
  m_scaleX->setValue(trs.scale[0]);
  m_scaleY->setValue(trs.scale[1]);
  m_scaleZ->setValue(trs.scale[2]);
  m_camDistance->setValue(s.camDistance);
  m_camPitch->setValue(s.camPitchDeg);
  m_camYaw->setValue(s.camYawDeg);
  m_proj->setCurrentIndex(s.proj == ProjOrtho ? 1 : 0);
  m_fov->setValue(s.fovDeg);
  m_nearZ->setValue(s.nearZ);
  m_farZ->setValue(s.farZ);
}
