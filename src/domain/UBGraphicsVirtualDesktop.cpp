#include "UBGraphicsVirtualDesktop.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include "domain/UBGraphicsItemDelegate.h"


UBGraphicsVirtualDesktop::UBGraphicsVirtualDesktop(int width, int height)
    : m_width(width), m_height(height), m_timerId(0)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                                        | GF_FLIPPABLE_ALL_AXIS
                                                        | GF_REVOLVABLE
                                                        | GF_RESPECT_RATIO
                                                        | GF_TOOLBAR_USED));
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly



    // Capture the content of the secondary screen when the object is created
    captureSecondaryScreen();

    // Start the timer for periodic screen updates at ~30 FPS
    m_timerId = startTimer(200);

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);  // Optional: Enables tracking of position changes
    // Set the item to be selectable
    setFlag(QGraphicsItem::ItemIsSelectable);  // Make the item selectable
}

QRectF UBGraphicsVirtualDesktop::boundingRect() const
{
    // The bounding rectangle of the virtual desktop is simply the area of the secondary screen
    return QRectF(0, 0, m_width, m_height);
}

void UBGraphicsVirtualDesktop::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Paint the captured secondary screen content (QImage)
    if (!m_screenImage.isNull()) {
        // Scale the screen content to fit the size of the virtual desktop
        painter->drawImage(0, 0, m_screenImage.scaled(m_width, m_height, Qt::KeepAspectRatio));
    }
}

void UBGraphicsVirtualDesktop::captureSecondaryScreen()
{
    // Get the list of available screens
    QList<QScreen *> screens = QGuiApplication::screens();

    // Check if we have more than one screen, assuming secondary screen is at index 1
    if (screens.size() > 1) {
        QScreen *secondaryScreen = screens.at(1);

        // Grab the content of the secondary screen (use grabWindow() to get the screen content)
        m_screenImage = secondaryScreen->grabWindow(0).toImage();
    }
}

void UBGraphicsVirtualDesktop::timerEvent(QTimerEvent *event)
{
    // Check if the event is the one from the timer (to capture screen content periodically)
    if (event->timerId() == m_timerId) {
        // Re-capture the secondary screen periodically (for 30 FPS updates)
        captureSecondaryScreen();

        // Trigger a repaint to update the screen content
        update();
    }
}

QVariant UBGraphicsVirtualDesktop::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return Delegate()->itemChange(change, value);
}
