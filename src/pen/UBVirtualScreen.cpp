#include "UBVirtualScreen.h"
#include "UBMouseOperations.h"
#include "UBPenController.h"
#include <cmath>

// Private constructor implementation
UBVirtualScreen::UBVirtualScreen() {
    calibrationStatus = CalibrationStatus::NOT_CALIBRATED;
    drag_threshold_ = 100;
    connected = false;
    p1x_ = p1y_ = p2x_ = p2y_ = 0;
    p0x_ = p0y_ = 0;
    qx_ = qy_ = 0.0;
    v_width_ = v_height_ = 0;
    down_px_ = down_py_ = 0;
    screen_height_ = screen_width_ = 0;
}

void UBVirtualScreen::calibrate() {
    calibrationStatus = CalibrationStatus::CALIBRATING_P1;
}

void UBVirtualScreen::setScreenDimensions(int width, int height) {
    screen_width_ = width;
    screen_height_ = height;
}

void UBVirtualScreen::calibrationSetFirstPoint(int pX, int pY) {
    p1x_ = pX;
    p1y_ = pY;
    calibrationStatus = CalibrationStatus::CALIBRATING_P2;
}

void UBVirtualScreen::calibrationSetSecondPoint(int pX, int pY) {
    p2x_ = pX;
    p2y_ = pY;
    setCalibration();
    calibrationStatus = CalibrationStatus::CALIBRATED;
}

bool UBVirtualScreen::isInvertedAxis() {
    bool screenIsWide = (screen_width_ / screen_height_) >= 1;
    bool paperIsWide = (std::abs(p1x_ - p2x_) / std::abs(p1y_ - p2y_)) >= 1;
    return screenIsWide && !paperIsWide;
}

void UBVirtualScreen::setCalibration() {
    // Calculate virtual screen width and height using the offset p1
    int offsetWidth = (p2x_ - p1x_) * 2;
    int offsetHeight = (p1y_ - p2y_) * 2;

    // Calculate 10% of vWidth and vHeight
    int tenPercentOfWidth = static_cast<int>(std::round(static_cast<double>(offsetWidth) / 8));
    int tenPercentOfHeight = static_cast<int>(std::round(static_cast<double>(offsetHeight) / 8));

    // Calculate vWidth and vHeight adding 20% to compensate for offset
    v_width_ = offsetWidth + (tenPercentOfWidth * 2);
    v_height_ = offsetHeight + (tenPercentOfHeight * 2);

    // Remove the offset from p1
    p1x_ = p1x_ - tenPercentOfWidth;
    p1y_ = p1y_ + tenPercentOfHeight;

    // Set virtual screen p0 (origin)
    p0x_ = p1x_;
    p0y_ = p1y_ - v_height_;

    // Calculate proportion quotients between real and virtual screen
    qx_ = static_cast<double>(screen_width_) / v_width_;
    qy_ = static_cast<double>(screen_height_) / v_height_;
}

void UBVirtualScreen::dotToMouse(int pX, int pY) {
    switch (UBPenController::getInstance()->penStatus) {
    case PenDown:
        down_px_ = pX;
        down_py_ = pY;
        UBMouseOperations::IsDragging = false;
        break;
    case PenMove:
        if (!UBMouseOperations::IsDragging) {
            if (enteredDragMode(pX, pY)) {
                auto [screenX, screenY] = getComputerScreenDot(pX, pY);
                UBMouseOperations::DragStart(screenX, screenY);
            }
        } else {
            auto [screenX, screenY] = getComputerScreenDot(pX, pY);
            UBMouseOperations::Drag(screenX, screenY, screen_width_, screen_height_);
        }
        break;
    case PenUp:
        if (UBMouseOperations::IsDragging) {
            UBMouseOperations::DragEnd();
        } else {
            auto [screenX, screenY] = getComputerScreenDot(pX, pY);
            UBMouseOperations::LeftClick(screenX, screenY);
        }
        break;
    }
}

std::tuple<int, int> UBVirtualScreen::getComputerScreenDot(int pX, int pY) {
    if ((pX > p0x_ && pX < p0x_ + v_width_) &&
        (pY > p0y_ && pY < p0y_ + v_height_)) {
        int pScreenX = static_cast<int>((pX - p0x_) * qx_);
        int pScreenY = static_cast<int>((pY - p0y_) * qy_);
        return std::make_tuple(pScreenX, pScreenY);
    }
    return std::make_tuple(-1, -1);
}

bool UBVirtualScreen::enteredDragMode(int pX, int pY) {
    return (std::abs(down_px_ - pX) > drag_threshold_ ||
            std::abs(down_py_ - pY) > drag_threshold_);
}
