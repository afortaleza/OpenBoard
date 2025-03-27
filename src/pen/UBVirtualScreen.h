#ifndef VIRTUAL_SCREEN_HPP
#define VIRTUAL_SCREEN_HPP

#include <tuple>

enum class CalibrationStatus { NOT_CALIBRATED, CALIBRATING_P1, CALIBRATING_P2, CALIBRATED };
enum class DotType { PEN_MOVE, PEN_UP };

class VirtualScreen {
public:
    // Static method to get the singleton instance
    static VirtualScreen& getInstance() {
        static VirtualScreen instance; // Created only once, thread-safe in C++11
        return instance;
    }

    // Public methods
    void calibrate();
    void setScreenDimensions(int width, int height);
    void calibrationSetFirstPoint(int pX, int pY);
    void calibrationSetSecondPoint(int pX, int pY);
    bool isInvertedAxis();
    void setCalibration();
    void dotToMouse(int pX, int pY);

    // Public variables (replacing getters and setters)
    bool connected;
    CalibrationStatus calibrationStatus;

    // Delete copy constructor and assignment operator to prevent copying
    VirtualScreen(const VirtualScreen&) = delete;
    VirtualScreen& operator=(const VirtualScreen&) = delete;

private:
    // Private constructor to prevent direct instantiation
    VirtualScreen();

    int p1x_;
    int p1y_;
    int p2x_;
    int p2y_;
    int p0x_;
    int p0y_;
    double qx_;
    double qy_;
    int v_width_;
    int v_height_;
    int down_px_;
    int down_py_;
    int drag_threshold_;
    int screen_height_;
    int screen_width_;

    // Private methods
    std::tuple<int, int> getComputerScreenDot(int pX, int pY);
    bool enteredDragMode(int pX, int pY);
};

#endif // VIRTUAL_SCREEN_HPP
