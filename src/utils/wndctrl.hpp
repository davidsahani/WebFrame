#pragma once
#include <windows.h>
#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")

namespace WndCtrl {

// Set window always on top
void SetWindowTopmost(HWND hwnd, bool alwaysOnTop) {
    const HWND topmost = alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(hwnd, topmost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// Set window transparency (0 = fully transparent, 255 = fully opaque)
void SetTransparency(HWND hwnd, BYTE alpha) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

// Set the window as a tool window (hides from Alt+Tab)
void SetToolWindowMode(HWND hwnd, bool enable) {
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (enable) {
        exStyle |= WS_EX_TOOLWINDOW; // Removes from Alt+Tab
        exStyle &= ~WS_EX_APPWINDOW; // Prevents taskbar entry
    } else {
        exStyle &= ~WS_EX_TOOLWINDOW;
        exStyle |= WS_EX_APPWINDOW;
    }
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
}

// Modify window style to remove the default border and title bar
void SetWindowBorderless(HWND hwnd, bool enable) {
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (enable) {
        style &= ~(WS_CAPTION | WS_THICKFRAME); // Remove title bar and borders
    } else {
        style |= (WS_CAPTION | WS_THICKFRAME); // Restore default title and window borders
    }
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
}

// Enable click-through (mouse clicks pass through the window)
void EnableClickThrough(HWND hwnd, bool enable) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (enable) {
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    } else {
        SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
    }
}

void ToggleWindowVisibility(HWND hwnd) {
    if (IsWindowVisible(hwnd)) {
        ShowWindow(hwnd, SW_HIDE);
        return;
    }
    // If hidden, bring to front and show
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE); // Restore if minimized
    }

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    const bool topmost = (exStyle & WS_EX_TOPMOST) != 0;

    if (!topmost) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    ShowWindow(hwnd, SW_SHOW); // Show the window

    if (!topmost) {
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

LRESULT CALLBACK CursorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (uMsg == WM_SETCURSOR) {
        SetCursor(LoadCursor(NULL, IDC_ARROW)); // Force arrow cursor shape
        return 0;                               // Allow normal window behavior
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// Toggle forced cursor shape (true = lock to arrow, false = allow normal cursor changes)
BOOL SetCursorShapeLock(HWND hwnd, bool enable) {
    return (enable) ? SetWindowSubclass(hwnd, CursorWindowProc, 0, 0)
                    : RemoveWindowSubclass(hwnd, CursorWindowProc, 0);
}

}; // namespace WndCtrl