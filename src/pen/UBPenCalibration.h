#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class UBPenCalibration : public QMainWindow
{
    Q_OBJECT

public:
    UBPenCalibration(QWidget *parent = nullptr);
    ~UBPenCalibration();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // MAINWINDOW_H
