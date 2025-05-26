#include "UBPenBoardConfiguration.h"
#include "gui/UBMainWindow.h"
#include "UBApplication.h"

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
}
