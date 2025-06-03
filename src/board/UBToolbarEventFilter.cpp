#include "UBToolbarEventFilter.h"
#include <QMouseEvent>
#include <QWidget>
#include <QApplication>
#include <QDebug>
#include "board/UBBoardController.h"
#include "core/UBApplication.h";
#include "gui/UBMainWindow.h"

UBToolbarEventFilter::UBToolbarEventFilter(QObject* parent)
    : QObject(parent), dragLogged(false)
{
}

bool UBToolbarEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

    if (event->type() == QEvent::MouseMove && mouseEvent->buttons() & Qt::LeftButton) {
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if (widget && (mouseEvent->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
            qDebug() << widget->objectName();
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
    }

    return QObject::eventFilter(obj, event);
}
