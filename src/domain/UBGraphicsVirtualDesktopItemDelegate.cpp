#include "../gui/UBMainWindow.h"
#include "../core/UBApplication.h"
#include "../board/UBBoardController.h"
#include "UBGraphicsVirtualDesktopItemDelegate.h"


UBGraphicsVirtualDesktopItemDelegate::UBGraphicsVirtualDesktopItemDelegate(UBGraphicsVirtualDesktopItem *pDelegated, QObject *parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_COMMON
                                                     | GF_RESPECT_RATIO
                                                     | GF_TOOLBAR_USED)
{

}

void UBGraphicsVirtualDesktopItemDelegate::remove(bool canUndo)
{
    UBApplication::mainWindow->actionVirtualDesktop->setChecked(false);
    UBGraphicsItemDelegate::remove(false);
}
