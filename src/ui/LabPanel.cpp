#include "ui/LabPanel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

LabPanel::LabPanel(QWidget* parent)
    : QWidget(parent)
    , m_state(labStateDefault())
    , m_block(false)
    , m_device(0)
    , m_variant(0)
    , m_fill(0)
    , m_cull(0)
    , m_depth(0)
    , m_cb(0) {
  QVBoxLayout* root = new QVBoxLayout(this);

  QGroupBox* deviceBox = new QGroupBox(QString::fromUtf8("Device"), this);
  deviceBox->setObjectName(QString::fromUtf8("labCardDevice"));
  QVBoxLayout* deviceLay = new QVBoxLayout(deviceBox);
  m_device = new QLabel(QString::fromUtf8("(no device)"), deviceBox);
  m_device->setWordWrap(true);
  deviceLay->addWidget(m_device);
  root->addWidget(deviceBox);

  QGroupBox* shaderBox = new QGroupBox(QString::fromUtf8("Shader"), this);
  shaderBox->setObjectName(QString::fromUtf8("labCardShader"));
  QHBoxLayout* shaderRow = new QHBoxLayout(shaderBox);
  m_variant = new QComboBox(shaderBox);
  m_variant->addItem(QString::fromUtf8("unlit"));
  m_variant->addItem(QString::fromUtf8("normal"));
  m_variant->addItem(QString::fromUtf8("checker"));
  QPushButton* reload = new QPushButton(QString::fromUtf8("Reload"), shaderBox);
  shaderRow->addWidget(m_variant, 1);
  shaderRow->addWidget(reload);
  root->addWidget(shaderBox);

  QGroupBox* rsBox = new QGroupBox(QString::fromUtf8("Rasterizer"), this);
  rsBox->setObjectName(QString::fromUtf8("labCardRaster"));
  QFormLayout* rsForm = new QFormLayout(rsBox);
  m_fill = new QComboBox(rsBox);
  m_fill->addItem(QString::fromUtf8("Solid"));
  m_fill->addItem(QString::fromUtf8("Wire"));
  m_cull = new QComboBox(rsBox);
  m_cull->addItem(QString::fromUtf8("None"));
  m_cull->addItem(QString::fromUtf8("Back"));
  m_cull->addItem(QString::fromUtf8("Front"));
  m_depth = new QCheckBox(QString::fromUtf8("On"), rsBox);
  rsForm->addRow(QString::fromUtf8("Fill"), m_fill);
  rsForm->addRow(QString::fromUtf8("Cull"), m_cull);
  rsForm->addRow(QString::fromUtf8("Depth"), m_depth);
  root->addWidget(rsBox);

  QGroupBox* cbBox = new QGroupBox(QString::fromUtf8("CB"), this);
  cbBox->setObjectName(QString::fromUtf8("labCardCb"));
  QVBoxLayout* cbLay = new QVBoxLayout(cbBox);
  m_cb = new QPlainTextEdit(cbBox);
  m_cb->setReadOnly(true);
  QFont font = m_cb->font();
  font.setFamily(QString::fromUtf8("Consolas"));
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  m_cb->setFont(font);
  m_cb->setLineWrapMode(QPlainTextEdit::NoWrap);
  m_cb->setTabChangesFocus(true);
  cbLay->addWidget(m_cb);
  QLabel* hexTitle = new QLabel(QString::fromUtf8("CB hex (shell)"), cbBox);
  QPlainTextEdit* hex = new QPlainTextEdit(cbBox);
  hex->setObjectName(QString::fromUtf8("labCbHex"));
  hex->setReadOnly(true);
  hex->setMaximumHeight(72);
  hex->setPlainText(QString::fromUtf8(
      "00 01 02 03  04 05 06 07\n08 09 0A 0B  0C 0D 0E 0F\n— placeholder —"));
  cbLay->addWidget(hexTitle);
  cbLay->addWidget(hex);
  root->addWidget(cbBox, 1);

  connect(m_variant, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &LabPanel::onEdited);
  connect(m_fill, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &LabPanel::onEdited);
  connect(m_cull, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &LabPanel::onEdited);
  connect(m_depth, &QCheckBox::toggled, this, &LabPanel::onEdited);
  connect(reload, &QPushButton::clicked, this, &LabPanel::onReload);

  m_block = true;
  loadWidgets(m_state);
  m_block = false;
}

void LabPanel::setState(const LabState& s) {
  m_state = s;
  const bool was = m_block;
  m_block = true;
  loadWidgets(s);
  m_block = was;
}

LabState LabPanel::state() const {
  LabState s = m_state;
  const int vi = m_variant->currentIndex();
  if (vi == 1) {
    labSetShaderVariant(&s, "normal");
  } else if (vi == 2) {
    labSetShaderVariant(&s, "checker");
  } else {
    labSetShaderVariant(&s, "unlit");
  }
  s.fillMode = (m_fill->currentIndex() == 1) ? 2 : 3;
  const int ci = m_cull->currentIndex();
  if (ci == 0) {
    s.cullMode = 1;
  } else if (ci == 2) {
    s.cullMode = 2;
  } else {
    s.cullMode = 3;
  }
  s.depthEnable = m_depth->isChecked();
  return s;
}

void LabPanel::setDeviceDescription(const QString& text) {
  if (text.isEmpty()) {
    m_device->setText(QString::fromUtf8("(no device)"));
    return;
  }
  m_device->setText(text);
}

void LabPanel::setCbText(const QString& text) {
  m_cb->setPlainText(text);
}

void LabPanel::onEdited() {
  if (m_block) {
    return;
  }
  emit changed();
}

void LabPanel::onReload() {
  emit reloadShaders();
}

void LabPanel::loadWidgets(const LabState& s) {
  int vi = 0;
  if (std::strcmp(s.shaderVariant, "normal") == 0) {
    vi = 1;
  } else if (std::strcmp(s.shaderVariant, "checker") == 0) {
    vi = 2;
  }
  m_variant->setCurrentIndex(vi);
  m_fill->setCurrentIndex(s.fillMode == 2 ? 1 : 0);
  int ci = 1;
  if (s.cullMode == 1) {
    ci = 0;
  } else if (s.cullMode == 2) {
    ci = 2;
  }
  m_cull->setCurrentIndex(ci);
  m_depth->setChecked(s.depthEnable);
}
