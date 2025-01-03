#ifndef UBVIRTUALDESKTOP_H
#define UBVIRTUALDESKTOP_H

#include <QGraphicsItem>
#include <QImage>
#include <QObject>

class UBVirtualDesktop : public QObject, public QGraphicsItem
{
    Q_OBJECT  // Make sure this class is a QObject to support event handling

public:
    // Constructor that accepts width and height of the virtual screen
    UBVirtualDesktop(int width, int height);

    // Override boundingRect() to define the area of the item
    QRectF boundingRect() const override;

    // Override paint() to render the secondary screen content
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    // Override the timerEvent to handle timer updates for screen capture
    void timerEvent(QTimerEvent *event) override;

private:
    int m_width;          // Width of the virtual desktop (secondary screen)
    int m_height;         // Height of the virtual desktop (secondary screen)
    QImage m_screenImage; // To store the image of the secondary screen
    int m_timerId;        // Timer ID for the timer event

    // Capture the secondary screen and store it as an image
    void captureSecondaryScreen();
};


#endif // UBVIRTUALDESKTOP_H
