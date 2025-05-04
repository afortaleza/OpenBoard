#include "UBGraphicsVirtualDesktopItem.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include "domain/UBGraphicsItemDelegate.h"
#include "../core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "../pen/UBPenController.h"
#include "UBGraphicsVirtualDesktopItemDelegate.h"

UBGraphicsVirtualDesktopItem::UBGraphicsVirtualDesktopItem()
    : m_timerId(0)
{
    setDelegate(new UBGraphicsVirtualDesktopItemDelegate(this));

    QScreen* mainWindowScreen = UBApplication::mainWindow->screen();

    // Get the list of available screens
    QList<QScreen *> screens = QGuiApplication::screens();
    QScreen *secondaryScreen = nullptr;

    for (QScreen* screen : screens) {
        if (screen != mainWindowScreen) {
            secondaryScreen = screen;
            break;
        }
    }

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

    // Delegate setup with flags
    // setDelegate(new UBGraphicsItemDelegate(this, 0, GF_SCALABLE_ALL_AXIS | GF_RESPECT_RATIO));

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

UBGraphicsVirtualDesktopItem::~UBGraphicsVirtualDesktopItem()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

QRectF UBGraphicsVirtualDesktopItem::boundingRect() const
{
    // The bounding rectangle of the virtual desktop is simply the area of the secondary screen
    return QRectF(0, 0, m_width, m_height);
}

void UBGraphicsVirtualDesktopItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Paint the captured secondary screen content (QImage)
    if (!m_screenImage.isNull()) {
        QRectF target = this->boundingRect();
        QRectF source(0, 0, m_width * 3, m_height * 3);

        painter->drawImage(target, m_screenImage, source);
    }
}

void UBGraphicsVirtualDesktopItem::setVirtualDesktopRect() const
{
    // Step 1: Get the bounding rectangle in scene coordinates
    QRectF bounding = boundingRect(); // Local coordinates (0, 0, m_width, m_height)

    // Map top-left and bottom-right corners to scene coordinates
    QPointF topLeftScene = mapToScene(bounding.topLeft());
    QPointF bottomRightScene = mapToScene(bounding.bottomRight());

    // Step 2: Get the QGraphicsScene
    QGraphicsScene *scene = this->scene();
    if (!scene) {
        return; // Return empty QRect if no scene is associated
    }

    // Step 3: Get the QGraphicsView(s) associated with the scene
    QList<QGraphicsView *> views = scene->views();
    if (views.isEmpty()) {
        return; // Return empty QRect if no views are associated
    }

    // Use the first view (assumes the item is displayed in the first QGraphicsView)
    QGraphicsView *view = views.first();

    // Step 4: Map scene coordinates to view coordinates
    QPoint topLeftView = view->mapFromScene(topLeftScene);
    QPoint bottomRightView = view->mapFromScene(bottomRightScene);

    // Step 5: Map view coordinates to global (screen) coordinates
    QPoint topLeftScreen = view->mapToGlobal(topLeftView);
    QPoint bottomRightScreen = view->mapToGlobal(bottomRightView);

    // Step 6: Construct the QRect with top-left and bottom-right points
    auto rect = QRect(topLeftScreen, bottomRightScreen);

    UBApplication::penController->virtualDesktopRect = rect;
}

void UBGraphicsVirtualDesktopItem::captureSecondaryScreen()
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

void UBGraphicsVirtualDesktopItem::timerEvent(QTimerEvent *event)
{
    // Check if the event is the one from the timer (to capture screen content periodically)
    if (event->timerId() == m_timerId) {
        // Re-capture the secondary screen periodically (for 30 FPS updates)
        captureSecondaryScreen();

        // Trigger a repaint to update the screen content
        update();
    }
}

QVariant UBGraphicsVirtualDesktopItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // Handle position changes
    if (change == GraphicsItemChange::ItemTransformHasChanged) {
        this->setVirtualDesktopRect();
    }

    return Delegate()->itemChange(change, value);
}
