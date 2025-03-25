#ifndef VIRTUAL_SCREEN_HPP
#define VIRTUAL_SCREEN_HPP

#include <tuple>

enum class CalibrationStatus { NOT_CALIBRATED, CALIBRATING_P1, CALIBRATING_P2, CALIBRATED };
enum class DotType { PEN_MOVE, PEN_UP };
enum class PenStatus { Down, Move, Up };

class VirtualScreen {
public:
    // Constructor
    VirtualScreen();

    // Public methods
    void Calibrate();
    void SetScreenDimensions(int width, int height);
    void CalibrationSetFirstPoint(int pX, int pY);
    void CalibrationSetSecondPoint(int pX, int pY);
    bool IsInvertedAxis();
    void SetCalibration();
    void DotToMouse(int pX, int pY);

    // Getters and setters
    bool GetConnected() const { return connected_; }
    void SetConnected(bool value) { connected_ = value; }
    CalibrationStatus GetCalibrationStatus() const { return calibration_status_; }
    void SetCalibrationStatus(CalibrationStatus status) { calibration_status_ = status; }
    PenStatus GetPenStatus() const { return pen_status_; }
    void SetPenStatus(PenStatus status) { pen_status_ = status; }

private:
    bool connected_;
    CalibrationStatus calibration_status_;
    PenStatus pen_status_;

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
    std::tuple<int, int> GetComputerScreenDot(int pX, int pY);
    bool EnteredDragMode(int pX, int pY);
};

#endif // VIRTUAL_SCREEN_HPP
