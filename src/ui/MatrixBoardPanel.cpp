#include "ui/MatrixBoardPanel.h"
#include <QFont>
#include <QFontMetrics>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

static QPlainTextEdit* makeMatrixEdit(QWidget* parent) {
  QPlainTextEdit* edit = new QPlainTextEdit(parent);
  edit->setReadOnly(true);
  QFont font = edit->font();
  font.setFamily(QString::fromUtf8("Consolas"));
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  edit->setFont(font);
  edit->setLineWrapMode(QPlainTextEdit::NoWrap);
  edit->setTabChangesFocus(true);
  const QFontMetrics fm(edit->font());
  edit->setMinimumHeight(fm.lineSpacing() * 4 + 28);
  return edit;
}

MatrixBoardPanel::MatrixBoardPanel(QWidget* parent)
    : QWidget(parent)
    , m_order(MajorColumn)
    , m_mw(0)
    , m_mv(0)
    , m_mp(0)
    , m_mvp(0) {
  DirectX::XMStoreFloat4x4(&m_w, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&m_v, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&m_p, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&m_wvp, DirectX::XMMatrixIdentity());

  QVBoxLayout* root = new QVBoxLayout(this);
  QGroupBox* gw = new QGroupBox(QString::fromUtf8("M_W"), this);
  QGroupBox* gv = new QGroupBox(QString::fromUtf8("M_V"), this);
  QGroupBox* gp = new QGroupBox(QString::fromUtf8("M_P"), this);
  QGroupBox* gm = new QGroupBox(QString::fromUtf8("MVP"), this);
  QVBoxLayout* lw = new QVBoxLayout(gw);
  QVBoxLayout* lv = new QVBoxLayout(gv);
  QVBoxLayout* lp = new QVBoxLayout(gp);
  QVBoxLayout* lm = new QVBoxLayout(gm);
  m_mw = makeMatrixEdit(gw);
  m_mv = makeMatrixEdit(gv);
  m_mp = makeMatrixEdit(gp);
  m_mvp = makeMatrixEdit(gm);
  lw->addWidget(m_mw);
  lv->addWidget(m_mv);
  lp->addWidget(m_mp);
  lm->addWidget(m_mvp);
  root->addWidget(gw);
  root->addWidget(gv);
  root->addWidget(gp);
  root->addWidget(gm);
  refresh();
}

void MatrixBoardPanel::setMajorOrder(MajorOrder order) {
  m_order = order;
  refresh();
}

void MatrixBoardPanel::setMatrices(const DirectX::XMFLOAT4X4& w,
                                  const DirectX::XMFLOAT4X4& v,
                                  const DirectX::XMFLOAT4X4& p,
                                  const DirectX::XMFLOAT4X4& wvp) {
  m_w = w;
  m_v = v;
  m_p = p;
  m_wvp = wvp;
  refresh();
}

void MatrixBoardPanel::setMatrices(const DirectX::XMFLOAT4X4& w,
                                  const DirectX::XMFLOAT4X4& v,
                                  const DirectX::XMFLOAT4X4& p,
                                  const DirectX::XMFLOAT4X4& wvp,
                                  MajorOrder order) {
  m_order = order;
  setMatrices(w, v, p, wvp);
}

void MatrixBoardPanel::refresh() {
  fillBlock(m_mw, m_w);
  fillBlock(m_mv, m_v);
  fillBlock(m_mp, m_p);
  fillBlock(m_mvp, m_wvp);
}

void MatrixBoardPanel::fillBlock(QPlainTextEdit* edit, const DirectX::XMFLOAT4X4& m) {
  char lines[4][64];
  formatMatrix4(m, m_order, lines);
  QString text;
  for (int i = 0; i < 4; ++i) {
    if (i > 0) {
      text += QChar::fromLatin1('\n');
    }
    text += QString::fromUtf8(lines[i]);
  }
  edit->setPlainText(text);
}
