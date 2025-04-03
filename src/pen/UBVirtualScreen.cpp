#include "UBVirtualScreen.h"
#include "UBPenController.h"
#include "../core/UBApplication.h"
#include <cmath>
#include "UBMouseOperations.h"

// Private constructor implementation
UBVirtualScreen::UBVirtualScreen() {
    drag_threshold = 100;
    p1x = p1y = p2x = p2y = 0;
    p0x = p0y = 0;
    qx = qy = 0.0;
    v_width = v_height = 0;
    down_px = down_py = 0;
    screen_height = screen_width = 0;

    // Board max bounds
    x_max = y_max = 0;
}

void UBVirtualScreen::setScreenDimensions(int width, int height) {
    screen_width = width;
    screen_height = height;
}

void UBVirtualScreen::calibrationSetFirstPoint(int pX, int pY) {
    p1x = pX;
    p1y = pY;
}

void UBVirtualScreen::calibrationSetSecondPoint(int pX, int pY) {
    p2x = pX;
    p2y = pY;
    setCalibration();
}

void UBVirtualScreen::setCalibration() {
    // Calculate virtual screen width and height using the offset p1
    int offsetWidth = (p2x - p1x) * 2;
    int offsetHeight = (p1y - p2y) * 2;

    // Calculate 10% of vWidth and vHeight
    int tenPercentOfWidth = offsetWidth / 8;
    int tenPercentOfHeight = offsetHeight / 8;

    // Calculate vWidth and vHeight adding 20% to compensate for offset
    v_width = offsetWidth + (tenPercentOfWidth * 2);
    v_height = offsetHeight + (tenPercentOfHeight * 2);

    // Remove the offset from p1
    p1x = p1x - tenPercentOfWidth;
    p1y = p1y + tenPercentOfHeight;

    // Set virtual screen p0 (origin)
    p0x = p1x;
    p0y = p1y - v_height;

    // Calculate proportion quotients between real and virtual screen
    qx = (double)screen_width / v_width;
    qy = (double)screen_height / v_height;

    // Set board bounds
    x_max = p0x + v_width;
    y_max = p0y + v_height;
}

void UBVirtualScreen::dotToMouse(int pX, int pY) {
    if (UBApplication::penController->penTipStatus == PenDown)
    {
        down_px = pX;
        down_py = pY;
        UBMouseOperations::IsDragging = false;
    }
    else if (UBApplication::penController->penTipStatus == PenMove)
    {
        if (!UBMouseOperations::IsDragging) {
            if (enteredDragMode(pX, pY)) {
                auto [x, y] = getComputerScreenDot(pX, pY);
                UBMouseOperations::DragStart(x, y);
            }
        } else {
            auto [x, y] = getComputerScreenDot(pX, pY);
            UBMouseOperations::Drag(x, y, screen_width, screen_height);
        }
    }
    else if (UBApplication::penController->penTipStatus == PenUp)
    {
        if (UBMouseOperations::IsDragging) {
            UBMouseOperations::DragEnd();
        } else {
            auto [x, y] = getComputerScreenDot(pX, pY);
            UBMouseOperations::LeftClick(x, y);
        }
    }
}

std::tuple<int, int> UBVirtualScreen::getComputerScreenDot(int pX, int pY) {
    if ((pX > p0x && pX < x_max) &&
        (pY > p0y && pY < y_max)) {
        return std::tuple<int, int>{
            (pX - p0x) * qx,
            (pY - p0y) * qy
        };
    }

    return std::tuple<int, int>{-1, -1};
}

bool UBVirtualScreen::enteredDragMode(int pX, int pY) {
    return (std::abs(down_px - pX) > drag_threshold ||
            std::abs(down_py - pY) > drag_threshold);
}
