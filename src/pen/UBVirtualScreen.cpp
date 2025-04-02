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
    int tenPercentOfWidth = static_cast<int>(std::round(static_cast<double>(offsetWidth) / 8));
    int tenPercentOfHeight = static_cast<int>(std::round(static_cast<double>(offsetHeight) / 8));

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
    qx = static_cast<double>(screen_width) / v_width;
    qy = static_cast<double>(screen_height) / v_height;
}

void UBVirtualScreen::dotToMouse(int pX, int pY) {
    int screenX, screenY;
    switch (UBApplication::penController->penTipStatus) {
        case PenDown:
            qInfo() << "[Pen] Pen Down";
            /*
            down_px = pX;
            down_py = pY;
            UBMouseOperations::IsDragging = false;
            */
            break;
        case PenMove:
            qInfo() << "[PEN] Pen Move";
            std::tuple(screenX, screenY) = getComputerScreenDot(pX, pY);
            UBMouseOperations::MouseMove(screenX, screenY);
            /*
            if (!UBMouseOperations::IsDragging) {
                if (enteredDragMode(pX, pY)) {
                    qInfo() << "[PEN] Entered drag mode";
                    auto [screenX, screenY] = getComputerScreenDot(pX, pY);
                    // UBMouseOperations::DragStart(screenX, screenY);
                }
            } else {
                qInfo() << "[PEN] Dragging";
                auto [screenX, screenY] = getComputerScreenDot(pX, pY);
                // UBMouseOperations::Drag(screenX, screenY, screen_width, screen_height);
            }
            */
            break;
        case PenUp:
            qInfo() << "[PEN] Drag end";
            /*
            if (UBMouseOperations::IsDragging) {
                qInfo() << "[PEN] Drag end";
                // UBMouseOperations::DragEnd();
            } else {
                auto [screenX, screenY] = getComputerScreenDot(pX, pY);
                qInfo() << "[PEN] Left click";
                // UBMouseOperations::LeftClick(screenX, screenY);
            }
            */
            break;
    }
}

std::tuple<int, int> UBVirtualScreen::getComputerScreenDot(int pX, int pY) {
    if ((pX > p0x && pX < p0x + v_width) &&
        (pY > p0y && pY < p0y + v_height)) {
        int pScreenX = static_cast<int>((pX - p0x) * qx);
        int pScreenY = static_cast<int>((pY - p0y) * qy);
        return std::make_tuple(pScreenX, pScreenY);
    }
    return std::make_tuple(-1, -1);
}

bool UBVirtualScreen::enteredDragMode(int pX, int pY) {
    return (std::abs(down_px - pX) > drag_threshold ||
            std::abs(down_py - pY) > drag_threshold);
}
