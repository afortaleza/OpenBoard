#ifndef UBGRAPHICSVIRTUALDESKTOPITEMDELEGATE_H
#define UBGRAPHICSVIRTUALDESKTOPITEMDELEGATE_H

#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsVirtualDesktopItem.h"

class UBGraphicsVirtualDesktopItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsVirtualDesktopItemDelegate(UBGraphicsVirtualDesktopItem* pDelegated, QObject * parent = 0);

    protected slots:
        virtual void remove(bool canUndo = false);
};

#endif // UBGRAPHICSVIRTUALDESKTOPITEMDELEGATE_H
