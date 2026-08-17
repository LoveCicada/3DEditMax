#pragma once
#include "core/MatrixFormat.h"
#include "teach/TutorialScript.h"
#include <QWidget>

class QGroupBox;
class QPlainTextEdit;

class MatrixBoardPanel : public QWidget {
  Q_OBJECT
public:
  explicit MatrixBoardPanel(QWidget* parent = 0);
  void setMajorOrder(MajorOrder order);
  void setMatrices(const DirectX::XMFLOAT4X4& w,
                   const DirectX::XMFLOAT4X4& v,
                   const DirectX::XMFLOAT4X4& p,
                   const DirectX::XMFLOAT4X4& wvp);
  void setMatrices(const DirectX::XMFLOAT4X4& w,
                   const DirectX::XMFLOAT4X4& v,
                   const DirectX::XMFLOAT4X4& p,
                   const DirectX::XMFLOAT4X4& wvp,
                   MajorOrder order);
  void setDemoFocus(DemoMatrixFocus focus);
private:
  void refresh();
  void fillBlock(QPlainTextEdit* edit, const DirectX::XMFLOAT4X4& m);
  void applyHot(QGroupBox* box, bool hot);

  MajorOrder m_order;
  DirectX::XMFLOAT4X4 m_w;
  DirectX::XMFLOAT4X4 m_v;
  DirectX::XMFLOAT4X4 m_p;
  DirectX::XMFLOAT4X4 m_wvp;
  QGroupBox* m_gw;
  QGroupBox* m_gv;
  QGroupBox* m_gp;
  QGroupBox* m_gm;
  QPlainTextEdit* m_mw;
  QPlainTextEdit* m_mv;
  QPlainTextEdit* m_mp;
  QPlainTextEdit* m_mvp;
};
