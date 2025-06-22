#include "UBPenBoardConfiguration.h"
#include "gui/UBMainWindow.h"
#include "UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

bool UBPenBoardConfiguration::mBoardMinimized = true;
bool UBPenBoardConfiguration::mSkipCentering = false;


UBPenBoardConfiguration::UBPenBoardConfiguration() {}

void UBPenBoardConfiguration::hideToolbarActions()
{
    // Main Menu
    UBApplication::mainWindow->actionWeb->setVisible(false);
    UBApplication::mainWindow->actionCheckUpdate->setVisible(false);
    UBApplication::mainWindow->actionMultiScreen->setVisible(false);
    UBApplication::mainWindow->actionHintsAndTips->setVisible(false);
    UBApplication::mainWindow->actionOpenTutorial->setVisible(false);
    UBApplication::mainWindow->actionHideApplication->setVisible(false);

    UBApplication::mainWindow->actionStylus->setVisible(false);
}

void UBPenBoardConfiguration::hideStylusActions()
{
    // Hide Stylus Palette items
    UBApplication::mainWindow->actionPointer->setVisible(false);
    UBApplication::mainWindow->actionVirtualKeyboard->setVisible(false);
    UBApplication::mainWindow->actionSnap->setVisible(false);
    UBApplication::mainWindow->actionPlay->setVisible(false);
}

void UBPenBoardConfiguration::initComponents()
{
    UBApplication::mainWindow->boardToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    hideToolbarActions();
    hideStylusActions();
    startMinimized();
}

void UBPenBoardConfiguration::minimizeBoard()
{
    mSkipCentering = true;
    startMinimized();
}

void UBPenBoardConfiguration::startMinimized()
{
    setBoardMinimized(true);
    UBApplication::mainWindow->boardToolBar->setFixedHeight(sizeBarMinimized);
    for (QAction* action : UBApplication::mainWindow->boardToolBar->actions()) {
        action->setVisible(false);
    }
}

void UBPenBoardConfiguration::maximizeBoard()
{
    mSkipCentering = true;
    setBoardMinimized(false);
    UBApplication::mainWindow->boardToolBar->setFixedHeight(sizeBarMaximized);
    for (QAction* action : UBApplication::mainWindow->boardToolBar->actions()) {
        action->setVisible(true);
    }
    hideToolbarActions();
}

bool UBPenBoardConfiguration::isBoardMinimized()
{
    return mBoardMinimized;
}

void UBPenBoardConfiguration::setBoardMinimized(bool boardMinimized)
{
    mBoardMinimized = boardMinimized;
}
