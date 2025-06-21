#include "UBPenBoardConfiguration.h"
#include "gui/UBMainWindow.h"
#include "UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "gui/UBStylusPalette.h"

UBPenBoardConfiguration::UBPenBoardConfiguration() {}

void UBPenBoardConfiguration::hideComponents()
{
    // Main Menu
    UBApplication::mainWindow->actionWeb->setVisible(false);
    UBApplication::mainWindow->actionCheckUpdate->setVisible(false);
    UBApplication::mainWindow->actionMultiScreen->setVisible(false);
    UBApplication::mainWindow->actionHintsAndTips->setVisible(false);
    UBApplication::mainWindow->actionOpenTutorial->setVisible(false);
    UBApplication::mainWindow->actionHideApplication->setVisible(false);

    UBApplication::mainWindow->actionStylus->setVisible(false);

    UBApplication::mainWindow->boardToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Hide Stylus Palette items
    UBApplication::mainWindow->actionPointer->setVisible(false);
    UBApplication::mainWindow->actionVirtualKeyboard->setVisible(false);
    UBApplication::mainWindow->actionSnap->setVisible(false);
    UBApplication::mainWindow->actionPlay->setVisible(false);
}
