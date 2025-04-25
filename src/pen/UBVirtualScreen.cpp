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
    primaryScreenHeight = primaryScreenWidth = 0;

    // Board max bounds
    x_max = y_max = 0;
}

void UBVirtualScreen::setPrimaryScreenDimensions(int width, int height) {
    primaryScreenWidth = width;
    primaryScreenHeight = height;
}

void UBVirtualScreen::setSecondaryScreenDimensions(int width, int height)
{
    secondaryScreenWidth = width;
    secondaryScreenHeight = height;
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
    qx = (double)primaryScreenWidth / v_width;
    qy = (double)primaryScreenHeight / v_height;

    // Set board bounds
    x_max = p0x + v_width;
    y_max = p0y + v_height;
}

void UBVirtualScreen::dotToMouse(int x, int y, int pX, int pY) {
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
                UBMouseOperations::DragStart(x, y);
            }
        } else {
            UBMouseOperations::Drag(x, y, primaryScreenWidth, primaryScreenHeight);
        }
    }
    else if (UBApplication::penController->penTipStatus == PenUp)
    {
        if (UBMouseOperations::IsDragging) {
            UBMouseOperations::DragEnd();
        } else {
            UBMouseOperations::LeftClick(x, y);
        }
    }
}

std::tuple<int, int> UBVirtualScreen::getPrimaryScreenDot(int pX, int pY) {
    if ((pX > p0x && pX < x_max) &&
        (pY > p0y && pY < y_max)) {
        return std::tuple<int, int>{
            (pX - p0x) * qx,
            (pY - p0y) * qy
        };
    }

    return std::tuple<int, int>{-1, -1};
}

std::tuple<int, int> UBVirtualScreen::getSecondaryScreenDot(QRect vRect, int pX, int pY)
{
    int pRectX = pX - vRect.x();
    int pRectY = pY - vRect.y();

    qreal propWidth = static_cast<qreal>(secondaryScreenWidth) / vRect.width();
    qreal propHeight = static_cast<qreal>(secondaryScreenWidth) / vRect.height();

    return std::tuple<int, int> { pRectX * propWidth, pRectY * propHeight };
}

bool UBVirtualScreen::enteredDragMode(int pX, int pY) {
    return (std::abs(down_px - pX) > drag_threshold ||
            std::abs(down_py - pY) > drag_threshold);
}
