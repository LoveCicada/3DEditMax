#include "ui/TrackerPanel.h"
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

QString format4(const DirectX::XMFLOAT4& v) {
  return QString::asprintf("%.3f  %.3f  %.3f  %.3f",
                           static_cast<double>(v.x),
                           static_cast<double>(v.y),
                           static_cast<double>(v.z),
                           static_cast<double>(v.w));
}

}  // namespace

TrackerPanel::TrackerPanel(QWidget* parent)
    : QWidget(parent)
    , m_state(teachingStateDefault())
    , m_block(false)
    , m_x(0)
    , m_y(0)
    , m_z(0)
    , m_world(0)
    , m_view(0)
    , m_clip(0)
    , m_ndc(0) {
  QVBoxLayout* root = new QVBoxLayout(this);

  QGroupBox* model = new QGroupBox(QString::fromUtf8("Model"), this);
  model->setObjectName(QString::fromUtf8("sectionTracker"));
  QFormLayout* form = new QFormLayout(model);
  m_x = makeSpin();
  m_y = makeSpin();
  m_z = makeSpin();
  QLabel* lx = new QLabel(QString::fromUtf8("X"), model);
  lx->setObjectName(QString::fromUtf8("axisLabelX"));
  QLabel* ly = new QLabel(QString::fromUtf8("Y"), model);
  ly->setObjectName(QString::fromUtf8("axisLabelY"));
  QLabel* lz = new QLabel(QString::fromUtf8("Z"), model);
  lz->setObjectName(QString::fromUtf8("axisLabelZ"));
  form->addRow(lx, m_x);
  form->addRow(ly, m_y);
  form->addRow(lz, m_z);
  root->addWidget(model);

  QGroupBox* corners = new QGroupBox(QString::fromUtf8("Cube corners"), this);
  QGridLayout* grid = new QGridLayout(corners);
  static const float kCorner[8][3] = {
      {0.5f, 0.5f, 0.5f},   {0.5f, 0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, -0.5f},
      {-0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f},
  };
  for (int i = 0; i < 8; ++i) {
    const QString text = QString::asprintf("%+.1f %+.1f %+.1f",
                                           static_cast<double>(kCorner[i][0]),
                                           static_cast<double>(kCorner[i][1]),
                                           static_cast<double>(kCorner[i][2]));
    QPushButton* b = new QPushButton(text, corners);
    b->setProperty("cx", kCorner[i][0]);
    b->setProperty("cy", kCorner[i][1]);
    b->setProperty("cz", kCorner[i][2]);
    connect(b, &QPushButton::clicked, this, &TrackerPanel::onPreset);
    grid->addWidget(b, i / 2, i % 2);
  }
  root->addWidget(corners);

  QGroupBox* spaces = new QGroupBox(QString::fromUtf8("Spaces"), this);
  QFormLayout* spaceForm = new QFormLayout(spaces);
  m_world = new QLabel(spaces);
  m_view = new QLabel(spaces);
  m_clip = new QLabel(spaces);
  m_ndc = new QLabel(spaces);
  m_world->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_view->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_clip->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_ndc->setTextInteractionFlags(Qt::TextSelectableByMouse);
  spaceForm->addRow(QString::fromUtf8("World"), m_world);
  spaceForm->addRow(QString::fromUtf8("View"), m_view);
  spaceForm->addRow(QString::fromUtf8("Clip"), m_clip);
  spaceForm->addRow(QString::fromUtf8("NDC"), m_ndc);
  root->addWidget(spaces);
  root->addStretch(1);

  m_block = true;
  loadWidgets(m_state);
  m_block = false;
}

QDoubleSpinBox* TrackerPanel::makeSpin() {
  QDoubleSpinBox* s = new QDoubleSpinBox(this);
  s->setRange(-10.0, 10.0);
  s->setSingleStep(0.1);
  s->setDecimals(2);
  connect(s, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
          this, &TrackerPanel::onEdited);
  return s;
}

void TrackerPanel::setState(const TeachingState& s) {
  m_state = s;
  const bool was = m_block;
  m_block = true;
  loadWidgets(s);
  m_block = was;
}

TeachingState TrackerPanel::state() const {
  TeachingState s = m_state;
  s.trackModel[0] = static_cast<float>(m_x->value());
  s.trackModel[1] = static_cast<float>(m_y->value());
  s.trackModel[2] = static_cast<float>(m_z->value());
  return s;
}

void TrackerPanel::setTrackResult(const TrackResult& tr) {
  m_world->setText(format4(tr.world));
  m_view->setText(format4(tr.view));
  m_clip->setText(format4(tr.clip));
  m_ndc->setText(format4(tr.ndc));
}

void TrackerPanel::onEdited() {
  if (m_block) {
    return;
  }
  emit changed();
}

void TrackerPanel::onPreset() {
  QPushButton* b = qobject_cast<QPushButton*>(sender());
  if (!b) {
    return;
  }
  const bool was = m_block;
  m_block = true;
  m_x->setValue(b->property("cx").toDouble());
  m_y->setValue(b->property("cy").toDouble());
  m_z->setValue(b->property("cz").toDouble());
  m_block = was;
  if (!m_block) {
    emit changed();
  }
}

void TrackerPanel::loadWidgets(const TeachingState& s) {
  m_x->setValue(s.trackModel[0]);
  m_y->setValue(s.trackModel[1]);
  m_z->setValue(s.trackModel[2]);
}
