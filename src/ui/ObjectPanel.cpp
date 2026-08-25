#include "ui/ObjectPanel.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QVBoxLayout>

namespace {

void makeCheckableGroup(QGroupBox* box) {
  box->setCheckable(true);
  box->setChecked(true);
  QObject::connect(box, &QGroupBox::toggled, box, [box](bool checked) {
    const QList<QWidget*> children =
        box->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children) {
      child->setVisible(checked);
    }
  });
}

}  // namespace

ObjectPanel::ObjectPanel(QWidget* parent)
    : QWidget(parent)
    , m_state(teachingStateDefault())
    , m_block(false)
    , m_mesh(0)
    , m_layout(0)
    , m_shading(0) {
  QVBoxLayout* root = new QVBoxLayout(this);

  QGroupBox* meshBox = new QGroupBox(QString::fromUtf8("Mesh"), this);
  meshBox->setObjectName(QString::fromUtf8("sectionObject"));
  QHBoxLayout* meshRow = new QHBoxLayout(meshBox);
  QRadioButton* cube = new QRadioButton(QString::fromUtf8("Cube"), meshBox);
  QRadioButton* sphere = new QRadioButton(QString::fromUtf8("Sphere"), meshBox);
  QRadioButton* cyl = new QRadioButton(QString::fromUtf8("Cylinder"), meshBox);
  m_mesh = new QButtonGroup(this);
  m_mesh->addButton(cube, MeshCube);
  m_mesh->addButton(sphere, MeshSphere);
  m_mesh->addButton(cyl, MeshCylinder);
  meshRow->addWidget(cube);
  meshRow->addWidget(sphere);
  meshRow->addWidget(cyl);
  makeCheckableGroup(meshBox);
  root->addWidget(meshBox);

  QGroupBox* scene = new QGroupBox(QString::fromUtf8("Scene"), this);
  QFormLayout* form = new QFormLayout(scene);
  m_layout = new QComboBox(scene);
  m_layout->addItem(QString::fromUtf8("One"));
  m_layout->addItem(QString::fromUtf8("Three"));
  m_shading = new QComboBox(scene);
  m_shading->addItem(QString::fromUtf8("Solid"));
  m_shading->addItem(QString::fromUtf8("Normal"));
  m_shading->addItem(QString::fromUtf8("Checker"));
  m_shading->addItem(QString::fromUtf8("Wire"));
  form->addRow(QString::fromUtf8("Layout"), m_layout);
  form->addRow(QString::fromUtf8("Shading"), m_shading);
  makeCheckableGroup(scene);
  root->addWidget(scene);
  root->addStretch(1);

  connect(m_mesh, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
          this, &ObjectPanel::onEdited);
  connect(m_layout, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &ObjectPanel::onEdited);
  connect(m_shading, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &ObjectPanel::onEdited);

  m_block = true;
  loadWidgets(m_state);
  m_block = false;
}

void ObjectPanel::setState(const TeachingState& s) {
  m_state = s;
  const bool was = m_block;
  m_block = true;
  loadWidgets(s);
  m_block = was;
}

TeachingState ObjectPanel::state() const {
  TeachingState s = m_state;
  const int meshId = m_mesh->checkedId();
  if (meshId == MeshSphere) {
    s.objects[0].mesh = MeshSphere;
  } else if (meshId == MeshCylinder) {
    s.objects[0].mesh = MeshCylinder;
  } else {
    s.objects[0].mesh = MeshCube;
  }
  s.layout = (m_layout->currentIndex() == 1) ? LayoutThree : LayoutOne;
  const int shade = m_shading->currentIndex();
  if (shade == ShadeNormal) {
    s.shading = ShadeNormal;
  } else if (shade == ShadeChecker) {
    s.shading = ShadeChecker;
  } else if (shade == ShadeWire) {
    s.shading = ShadeWire;
  } else {
    s.shading = ShadeSolid;
  }
  return s;
}

void ObjectPanel::onEdited() {
  if (m_block) {
    return;
  }
  emit changed();
}

void ObjectPanel::loadWidgets(const TeachingState& s) {
  int meshId = MeshCube;
  if (s.objects[0].mesh == MeshSphere) {
    meshId = MeshSphere;
  } else if (s.objects[0].mesh == MeshCylinder) {
    meshId = MeshCylinder;
  }
  if (QAbstractButton* b = m_mesh->button(meshId)) {
    b->setChecked(true);
  }
  m_layout->setCurrentIndex(s.layout == LayoutThree ? 1 : 0);
  int shade = 0;
  if (s.shading == ShadeNormal) {
    shade = 1;
  } else if (s.shading == ShadeChecker) {
    shade = 2;
  } else if (s.shading == ShadeWire) {
    shade = 3;
  }
  m_shading->setCurrentIndex(shade);
}
