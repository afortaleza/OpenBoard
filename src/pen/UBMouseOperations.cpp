    #include "UBMouseOperations.h"

bool UBMouseOperations::IsDragging = false;

void UBMouseOperations::LeftClick(int screenX, int screenY) {
    SetCursorPos(screenX, screenY);

    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void UBMouseOperations::DragStart(int screenX, int screenY) {
    SetCursorPos(screenX, screenY);

    IsDragging = true;

    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}

void UBMouseOperations::Drag(int screenX, int screenY, int screenWidth, int screenHeight) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input.mi.dx = static_cast<LONG>(screenX * 65535 / screenWidth);
    input.mi.dy = static_cast<LONG>(screenY * 65535 / screenHeight);
    SendInput(1, &input, sizeof(INPUT));
}

void UBMouseOperations::DragEnd() {
    IsDragging = false;

    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void UBMouseOperations::MouseMove(int screenX, int screenY) {
    SetCursorPos(screenX, screenY);
}
