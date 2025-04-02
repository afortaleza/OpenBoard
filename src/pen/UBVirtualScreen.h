#ifndef VIRTUAL_SCREEN_HPP
#define VIRTUAL_SCREEN_HPP

#include <tuple>

class UBVirtualScreen {
public:
    UBVirtualScreen();

    void calibrate();
    void setScreenDimensions(int width, int height);
    void calibrationSetFirstPoint(int pX, int pY);
    void calibrationSetSecondPoint(int pX, int pY);
    void setCalibration();
    void dotToMouse(int pX, int pY);
    bool connected;

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
    int screen_height;
    int screen_width;

    // Board max bounds
    int x_max;
    int y_max;

    // Private methods
    std::tuple<int, int> getComputerScreenDot(int pX, int pY);
    bool enteredDragMode(int pX, int pY);
};

#endif // VIRTUAL_SCREEN_HPP
