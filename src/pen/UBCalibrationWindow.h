#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class UBCalibrationWindow : public QMainWindow
{
    Q_OBJECT

public:
    UBCalibrationWindow(QWidget *parent = nullptr);
    ~UBCalibrationWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void drawBottomLeftCross(QPainter &painter);
    void drawCenterCross(QPainter &painter);
};

#endif // MAINWINDOW_H
