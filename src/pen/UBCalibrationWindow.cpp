#include "UBCalibrationWindow.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include "UBVirtualScreen.h"

UBCalibrationWindow::UBCalibrationWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    setGeometry(screenGeometry);
}

UBCalibrationWindow::~UBCalibrationWindow()
{
}

void UBCalibrationWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (UBVirtualScreen::getInstance().calibrationStatus == CalibrationStatus::CALIBRATING_P1)
    {
        drawBottomLeftCross(painter);
    }
    else if (UBVirtualScreen::getInstance().calibrationStatus == CalibrationStatus::CALIBRATING_P2)
    {
        drawCenterCross(painter);
    }
}

void UBCalibrationWindow::drawBottomLeftCross(QPainter &painter)
{
    // Clear the screen with semi-transparent background
    painter.setBrush(QColor(0, 0, 0, 128));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Set pen for cross
    painter.setPen(QPen(Qt::white, 2));

    // Calculate dimensions
    int w = width();
    int h = height();
    int crossSize = std::min(w, h) * 0.1;

    // Calculate position (10% from bottom left)
    int cross_x = w * 0.1;
    int cross_y = h * 0.9;

    // Draw cross
    painter.drawLine(cross_x - crossSize/2, cross_y, cross_x + crossSize/2, cross_y);
    painter.drawLine(cross_x, cross_y - crossSize/2, cross_x, cross_y + crossSize/2);
}

void UBCalibrationWindow::drawCenterCross(QPainter &painter)
{
    // Clear the screen with semi-transparent background
    painter.setBrush(QColor(0, 0, 0, 128));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Set pen for cross
    painter.setPen(QPen(Qt::white, 2));

    // Calculate dimensions
    int w = width();
    int h = height();
    int crossSize = std::min(w, h) * 0.1;

    // Calculate position (center)
    int cross_x = w / 2;
    int cross_y = h / 2;

    // Draw cross
    painter.drawLine(cross_x - crossSize/2, cross_y, cross_x + crossSize/2, cross_y);
    painter.drawLine(cross_x, cross_y - crossSize/2, cross_x, cross_y + crossSize/2);
}
