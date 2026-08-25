#pragma once
#include "core/LabState.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QPlainTextEdit;

class LabPanel : public QWidget {
  Q_OBJECT
public:
  explicit LabPanel(QWidget* parent = 0);
  void setState(const LabState& s);
  LabState state() const;
  void setDeviceDescription(const QString& text);
  void setCbText(const QString& text);
signals:
  void changed();
  void reloadShaders();
private slots:
  void onEdited();
  void onReload();
private:
  void loadWidgets(const LabState& s);

  LabState m_state;
  bool m_block;
  QLabel* m_device;
  QComboBox* m_variant;
  QComboBox* m_fill;
  QComboBox* m_cull;
  QCheckBox* m_depth;
  QPlainTextEdit* m_cb;
  QPlainTextEdit* m_cbHex;
};
