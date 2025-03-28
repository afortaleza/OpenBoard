#ifndef MOUSE_OPERATIONS_HPP
#define MOUSE_OPERATIONS_HPP

#include <windows.h>

class UBMouseOperations {

public:
    static void LeftClick(int screenX, int screenY);
    static void DragStart(int screenX, int screenY);
    static void Drag(int screenX, int screenY, int screenWidth, int screenHeight);
    static void DragEnd();
    static void MouseMove(int screenX, int screenY);

    // Static getter for isDragging
    static bool IsDragging;
};

#endif // MOUSE_OPERATIONS_HPP
