#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <windows.h>

class KeybindListener {
  public:
    // Public API
    static bool InstallHook();
    static bool UninstallHook();
    static bool RegisterKeybind(std::string keybind, std::function<void()> callback);
    static bool UnRegisterKeybind(std::string keybind);

  private:
    // Global hook handle
    static HHOOK keyboardHook;
    static std::unordered_map<std::string, std::function<void()>> keybinds;
    static std::vector<std::string> pressedKeys;           // Maintains order
    static std::unordered_set<std::string> pressedKeysSet; // Fast lookup

    // Hook and event handling
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static void ProcessKeyEvent(std::string key, bool pressed);

    // Key name processing
    static std::string GetKeyName(UINT vkCode);
    static std::string NormalizeKey(const std::string &key);
};
