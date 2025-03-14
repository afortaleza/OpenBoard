#include "UBGraphicsVirtualDesktop.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include "domain/UBGraphicsItemDelegate.h"


UBGraphicsVirtualDesktop::UBGraphicsVirtualDesktop()
    : m_timerId(0)
{
    // Get the list of available screens
    QList<QScreen *> screens = QGuiApplication::screens();

    // Check if we have more than one screen, assuming secondary screen is at index 1
    if (screens.size() > 1) {
        QScreen *secondaryScreen = screens.at(1);

        // Get the scaled size of the secondary screen
        QSize scaledSize = secondaryScreen->size();

        // Get the scaling factor (device pixel ratio)
        qreal devicePixelRatio = secondaryScreen->devicePixelRatio();

        // Calculate the original (logical) resolution by dividing by the scaling factor
        QSize originalSize(scaledSize.width() * devicePixelRatio, scaledSize.height() * devicePixelRatio);

        // Set the virtual desktop size to 1/3 of the original screen width and height
        m_width = originalSize.width() / 3;
        m_height = originalSize.height() / 3;

        setPos(m_width * -0.5, m_height * -0.5);
    }
    else {
        // If only one screen, set the virtual desktop size to 50% of the primary screen's width and height
        QScreen *primaryScreen = screens.at(0);
        QSize screenSize = primaryScreen->size();

        m_width = screenSize.width() / 3;
        m_height = screenSize.height() / 3;
    }

    // Delegate setup with flags
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_SCALABLE_Y_AXIS | GF_RESPECT_RATIO));

    // Set the data layer types
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));

    // Capture the content of the secondary screen when the object is created
    captureSecondaryScreen();

    // Start the timer for periodic screen updates at ~30 FPS
    m_timerId = startTimer(200);

    // Flags for item geometry change and selection
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);  // Optional: Enables tracking of position changes
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
        QRectF target = this->boundingRect();
        QRectF source(0, 0, m_width * 3, m_height * 3);

        painter->drawImage(target, m_screenImage, source);
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
