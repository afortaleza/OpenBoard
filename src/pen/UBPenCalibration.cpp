#include "UBPenCalibration.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>

UBPenCalibration::UBPenCalibration(QWidget *parent)
    : QMainWindow(parent)
{
    // Set window flags for borderless full-screen
    //setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Make window transparent
    // setAttribute(Qt::WA_TranslucentBackground);

    // Get primary screen size and set geometry
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    setGeometry(screenGeometry);
}

UBPenCalibration::~UBPenCalibration()
{
}

void UBPenCalibration::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Draw semi-transparent black background
    painter.setBrush(QColor(0, 0, 0, 128)); // 50% opacity (255/2 = 128)
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // Set pen for crosses
    painter.setPen(QPen(Qt::white, 2));

    // Calculate dimensions
    int w = width();
    int h = height();

    // First cross (10% from bottom left)
    int cross1_x = w * 0.1;
    int cross1_y = h * 0.9;  // 10% from bottom = 90% from top
    int crossSize = std::min(w, h) * 0.1;  // Cross size is 10% of smaller dimension

    // Draw first cross
    painter.drawLine(cross1_x - crossSize/2, cross1_y, cross1_x + crossSize/2, cross1_y);
    painter.drawLine(cross1_x, cross1_y - crossSize/2, cross1_x, cross1_y + crossSize/2);

    // Second cross (center)
    int cross2_x = w / 2;
    int cross2_y = h / 2;

    // Draw second cross
    painter.drawLine(cross2_x - crossSize/2, cross2_y, cross2_x + crossSize/2, cross2_y);
    painter.drawLine(cross2_x, cross2_y - crossSize/2, cross2_x, cross2_y + crossSize/2);
}
