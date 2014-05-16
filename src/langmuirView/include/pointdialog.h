#ifndef POINTDIALOG_H
#define POINTDIALOG_H

#include "pointcloud.h"

#include <QDialog>

namespace Ui {
class PointDialog;
}

class LangmuirViewer;

class PointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PointDialog(LangmuirViewer &viewer, QWidget *parent = 0);
    ~PointDialog();

public slots:
    void init();
    void update();

    void updateComboBoxElectrons(PointCloud::Mode mode);
    void updateComboBoxDefects(PointCloud::Mode mode);
    void updateComboBoxHoles(PointCloud::Mode mode);

    void updateSpinBoxElectrons(float d);
    void updateSpinBoxDefects(float d);
    void updateSpinBoxHoles(float d);

    void on_comboBoxElectrons_currentTextChanged(const QString & text);
    void on_comboBoxDefects_currentTextChanged(const QString & text);
    void on_comboBoxHoles_currentTextChanged(const QString & text);

    void on_spinBoxElectrons_valueChanged(double d);
    void on_spinBoxDefects_valueChanged(double d);
    void on_spinBoxHoles_valueChanged(double d);

private:
    Ui::PointDialog *ui;
    LangmuirViewer& m_viewer;
};

#endif // POINTDIALOG_H
