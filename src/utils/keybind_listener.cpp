#include "keybind_listener.hpp"
#include "string_utils.hpp"
#include <algorithm>

// Static variable definitions
HHOOK KeybindListener::keyboardHook = NULL;
std::vector<std::string> KeybindListener::pressedKeys = {};
std::unordered_set<std::string> KeybindListener::pressedKeysSet = {};
std::unordered_map<std::string, std::function<void()>> KeybindListener::keybinds = {};

// Implementation of public API
bool KeybindListener::InstallHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    return keyboardHook != NULL;
}

bool KeybindListener::UninstallHook() {
    return UnhookWindowsHookEx(keyboardHook);
}

bool KeybindListener::RegisterKeybind(std::string keybind, std::function<void()> callback) {
    std::string key = NormalizeKey(keybind);
    if (key.empty()) return false;
    keybinds[key] = callback;
    return true;
}

bool KeybindListener::UnRegisterKeybind(std::string keybind) {
    std::string key = NormalizeKey(keybind);
    if (key.empty()) return false;
    return keybinds.erase(key) != 0;
}

// Implementation of hook and event handling
LRESULT CALLBACK KeybindListener::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *kbStruct = (KBDLLHOOKSTRUCT *)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            std::string keyName = GetKeyName(kbStruct->vkCode);
            if (!keyName.empty()) {
                ProcessKeyEvent(keyName, true);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            std::string keyName = GetKeyName(kbStruct->vkCode);
            if (!keyName.empty()) {
                ProcessKeyEvent(keyName, false);
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void KeybindListener::ProcessKeyEvent(std::string key, bool pressed) {
    if (pressed) {
        // O(1) lookup in unordered_set
        if (pressedKeysSet.insert(key).second) {
            pressedKeys.push_back(key); // Only add if not already present
        }
    } else {
        if (pressedKeysSet.erase(key)) {
            // Remove key while maintaining order
            pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), key), pressedKeys.end());
        }
    }

    std::string activeKeys = StringUtils::Join(pressedKeys, '+');
    if (activeKeys.empty()) return;

    if (keybinds.find(activeKeys) != keybinds.end()) {
        keybinds[activeKeys]();
    }
}

// Implementation of key name processing
std::string KeybindListener::GetKeyName(UINT vkCode) {
    switch (vkCode) {
    case VK_LWIN:
        return "Win";
    case VK_RWIN:
        return "Right Win";
    case VK_RCONTROL:
        return "Right Ctrl";
    case VK_RMENU:
        return "Right Alt";
    case VK_PRIOR:
        return "PageUp";
    case VK_NEXT:
        return "PageDown";
    case VK_HOME:
        return "Home";
    case VK_END:
        return "End";
    case VK_INSERT:
        return "Insert";
    case VK_DELETE:
        return "Delete";
    case VK_LEFT:
        return "Left";
    case VK_RIGHT:
        return "Right";
    case VK_UP:
        return "Up";
    case VK_DOWN:
        return "Down";
    case VK_PAUSE:
        return "Pause";
    case VK_NUMLOCK:
        return "Num Lock";
    }

    char name[128];
    if (GetKeyNameTextA(MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC) << 16, name, sizeof(name)) > 0) {
        return std::string(name);
    }
    return "";
}

std::string KeybindListener::NormalizeKey(const std::string &key) {
    std::vector<std::string> tokens = StringUtils::Split(key, '+');

    for (std::string &token : tokens) {
        std::vector<std::string> parts = StringUtils::Split(token, ' ');

        for (auto &p : parts) {
            std::transform(p.begin(), p.end(), p.begin(), ::tolower);
            p[0] = std::toupper(p[0]);
        }

        token = StringUtils::Join(parts, ' ');
    }

    return StringUtils::Join(tokens, '+');
}
