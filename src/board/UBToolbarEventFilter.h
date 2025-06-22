#ifndef UBTOOLBAREVENTFILTER_H
#define UBTOOLBAREVENTFILTER_H

#include <QObject>
#include <QPoint>
#include <QTimer>

class UBToolbarEventFilter : public QObject {
    Q_OBJECT
public:
    explicit UBToolbarEventFilter(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QPointF mPreviousPoint;
    QPoint dragStartPosition;
    bool dragLogged; // Tracks if drag has been logged
    QTimer *mMinimizeTimer;
};

#endif // UBTOOLBAREVENTFILTER_H
