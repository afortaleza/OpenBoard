#include "UBToolbarEventFilter.h"
#include <QMouseEvent>
#include <QWidget>
#include <QApplication>
#include <QDebug>
#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBPenBoardConfiguration.h"
#include "gui/UBMainWindow.h"

UBToolbarEventFilter::UBToolbarEventFilter(QObject* parent)
    : QObject(parent), dragLogged(false), mMinimizeTimer(NULL)
{
    mMinimizeTimer = new QTimer();
    mMinimizeTimer->setSingleShot(true);
    connect(mMinimizeTimer, &QTimer::timeout, this, []() {
        if (!UBPenBoardConfiguration::isBoardMinimized()) {
            UBPenBoardConfiguration::minimizeBoard();
        }
    });
}

bool UBToolbarEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

    if (event->type() == QEvent::MouseMove && mouseEvent->buttons() & Qt::LeftButton) {
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if (widget && (mouseEvent->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
            dragLogged = true;
            if (!UBApplication::mainWindow->actionHand->isChecked())
                UBApplication::mainWindow->actionHand->trigger();

            QPointF eventPosition = mouseEvent->position();
            qreal dx = eventPosition.x () - mPreviousPoint.x ();
            qreal dy = eventPosition.y () - mPreviousPoint.y ();
            UBApplication::boardController->handScroll(dx, dy);
            mPreviousPoint = eventPosition;

            return true; // Consume the event
        }
    } else if (event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
        mPreviousPoint = static_cast<QMouseEvent*>(event)->position();
        dragStartPosition = static_cast<QMouseEvent*>(event)->pos();
    } else if  (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
        if (!dragLogged) {
            if (UBPenBoardConfiguration::isBoardMinimized()) {
                UBPenBoardConfiguration::maximizeBoard();
                mMinimizeTimer->start(3000); // Start 3-second timer
            }
            else {
                UBPenBoardConfiguration::minimizeBoard();
                mMinimizeTimer->stop(); // Stop timer if minimizing manually
            }

            QApplication::processEvents();
            UBPenBoardConfiguration::mSkipCentering = false;
        }
        else {
            dragLogged = false;
        }
    }

    return QObject::eventFilter(obj, event);
}
