#include "LabelEditorDialog.h"
#include "ui_LabelEditorDialog.h"
#include <LabelEditorModel.h>

#include <QAbstractTableModel>

#include <QtComboBoxCoupling.h>
#include <QtLineEditCoupling.h>
#include <QtListWidgetCoupling.h>
#include <QtSliderCoupling.h>
#include <QtSpinBoxCoupling.h>
#include <QtDoubleSpinBoxCoupling.h>
#include <QtWidgetArrayCoupling.h>
#include <QtCheckBoxCoupling.h>
#include <QtPushButtonCoupling.h>

#include <QtWidgetActivator.h>

LabelEditorDialog::LabelEditorDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::LabelEditorDialog)
{
  ui->setupUi(this);
}

LabelEditorDialog::~LabelEditorDialog()
{
  delete ui;
}

void LabelEditorDialog::SetModel(LabelEditorModel *model)
{
  // Store the model
  m_Model = model;

  // Couple widgets to the model
  makeCoupling(ui->listLabels, m_Model->GetCurrentLabelModel());

  // Coupling for the description of the current label. Override the default
  // signal for this widget.
  makeCoupling(ui->inLabelDescription,
               m_Model->GetCurrentLabelDescriptionModel());

  // Coupling for the ID of the current label
  makeCoupling(ui->inLabelId,
               m_Model->GetCurrentLabelIdModel(),
               QtCouplingOptions(NULL,false));

  // Opacity (there are two controls)
  makeCoupling(ui->inLabelOpacitySlider,
               m_Model->GetCurrentLabelOpacityModel());

  makeCoupling(ui->inLabelOpacitySpinner,
               m_Model->GetCurrentLabelOpacityModel());

  // Visibility checkboxes
  makeArrayCoupling(ui->inVisibleAll, ui->inVisible3D,
                    m_Model->GetCurrentLabelHiddenStateModel());

  // Color button
  makeCoupling(ui->btnLabelColor,
               m_Model->GetCurrentLabelColorModel());

  // Set up activations
  activateOnFlag(ui->grpSelectedLabel, m_Model,
                 LabelEditorModel::UIF_EDITABLE_LABEL_SELECTED);
  activateOnFlag(ui->btnDuplicate, m_Model,
                 LabelEditorModel::UIF_EDITABLE_LABEL_SELECTED);
  activateOnFlag(ui->btnDelete, m_Model,
                 LabelEditorModel::UIF_EDITABLE_LABEL_SELECTED);
}

void LabelEditorDialog::on_btnClose_clicked()
{
  this->close();
}

#include <QMessageBox>

void LabelEditorDialog::on_btnNew_clicked()
{
  // Create a new label in the first slot after the currently selected
  if(!m_Model->MakeNewLabel(false))
    {
    QMessageBox::information(
          this,
          "ITK-SNAP: Label Insertion Failed",
          "There is no room to add a new label. Delete some existing labels.");
    }
}

void LabelEditorDialog::on_btnDuplicate_clicked()
{
  // Create a new label in the first slot after the currently selected
  if(!m_Model->MakeNewLabel(true))
    {
    QMessageBox::information(
          this,
          "ITK-SNAP: Label Insertion Failed",
          "There is no room to add a new label. Delete some existing labels.");
    }
}

void LabelEditorDialog::on_btnDelete_clicked()
{
  // Check if the deletion is going to affect the segmentation
  if(m_Model->IsLabelDeletionDestructive())
    {
    QMessageBox mb(this);
    QString text = QString(
          "Label %1 is used in the current segmentation. If you delete this "
          "label, the voxels that currently are assigned label %1 will be "
          "assigned the clear label (label 0). Are you sure you want to "
          "delete label %1?").arg(m_Model->GetCurrentLabelModel()->GetValue());
    mb.setText(text);
    mb.addButton("Delete Label", QMessageBox::ActionRole);
    QPushButton *bCancel = mb.addButton(QMessageBox::Cancel);
    mb.exec();

    if(mb.clickedButton() == bCancel)
      return;
    }

  // Delete the label
  m_Model->DeleteCurrentLabel();
}

void LabelEditorDialog::on_inLabelId_editingFinished()
{
  // Check if the new value is available
  LabelType newid = (LabelType) ui->inLabelId->value();
  LabelType curid = m_Model->GetCurrentLabelIdModel()->GetValue();

  // If the value is the same as the current, just ignore
  if(newid == curid)
    return;

  // Try to reassign the label
  if(!m_Model->ReassignLabelId(ui->inLabelId->value()))
    {
    // Complain
    QMessageBox::information(
          this,
          "ITK-SNAP: Label Id Change Failed",
          QString("Can not change the numerical value to %1 "
                  "because a label with that value already exists. "
                  "Delete label %1 first.").arg(ui->inLabelId->value()));

    // Set the value to the current value
    ui->inLabelId->setValue(curid);
    }
}
