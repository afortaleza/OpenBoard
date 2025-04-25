#ifndef VIRTUAL_SCREEN_HPP
#define VIRTUAL_SCREEN_HPP

#include <tuple>
#include <QPoint>
#include <QRect>

class UBVirtualScreen {
public:
    UBVirtualScreen();

    void calibrate();
    void setPrimaryScreenDimensions(int width, int height);
    void setSecondaryScreenDimensions(int width, int height);
    void calibrationSetFirstPoint(int pX, int pY);
    void calibrationSetSecondPoint(int pX, int pY);
    void setCalibration();
    void dotToMouse(int x, int y, int pX, int pY);
    std::tuple<int, int> getPrimaryScreenDot(int pX, int pY);
    std::tuple<int, int> getSecondaryScreenDot(QRect vRect, int pX, int pY);
    bool connected;
    bool hasSecondaryScreen;

private:
    int p1x;
    int p1y;
    int p2x;
    int p2y;
    int p0x;
    int p0y;
    double qx;
    double qy;
    int v_width;
    int v_height;
    int down_px;
    int down_py;
    int drag_threshold;
    int primaryScreenHeight;
    int primaryScreenWidth;
    int secondaryScreenHeight;
    int secondaryScreenWidth;


    // Board max bounds
    int x_max;
    int y_max;

    // Private methods
    bool enteredDragMode(int pX, int pY);
};

#endif // VIRTUAL_SCREEN_HPP
