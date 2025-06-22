#ifndef UBPENBOARDCONFIGURATION_H
#define UBPENBOARDCONFIGURATION_H

class UBPenBoardConfiguration
{
public:
    UBPenBoardConfiguration();
    static void hideToolbarActions();
    static void hideStylusActions();
    static void initComponents();

    static void minimizeBoard();
    static void startMinimized();
    static void maximizeBoard();
    static bool isBoardMinimized();
    static void setBoardMinimized(bool boardMinimized);

    static bool mSkipCentering;
    static const int sizeBarMinimized = 20;
    static const int sizeBarMaximized = 50;
private:
    static bool mBoardMinimized;
};

#endif // UBPENBOARDCONFIGURATION_H
