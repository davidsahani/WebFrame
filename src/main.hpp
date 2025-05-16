#include <SimpleIni.h>
#include <memory> // For std::unique_ptr
#include "window.hpp"
#include "widgets.hpp"
#include "webview.hpp"
#include "wndctrl.hpp"
#include "keybind_listener.hpp"
#include "string_utils.hpp"
#include "utils.hpp"
#include "Log.hpp"

inline std::unique_ptr<CSimpleIniA> LoadConfig(const char *filename) {
    auto ini = std::make_unique<CSimpleIniA>();
    ini->SetUnicode(); // Use UTF-8 encoding

    if (ini->LoadFile(filename) < 0) {
        Log::Error("Failed to load INI file: %s", filename);
    }
    return ini;
}

inline OmniBarImageTextures LoadOmniBarImageTextures(ID3D11Device *device) {
    const std::vector<ImTextureID> imageTextures = Utils::LoadImageTextures(
        {
            {"icons/backward.png"},
            {"icons/forward.png"},
            {"icons/refresh.png"},
            {"icons/settings.png"},
            {"icons/screenshot.png"},
        },
        device
    );

    return {
        .backward_button = imageTextures.at(0),
        .forward_button = imageTextures.at(1),
        .refresh_button = imageTextures.at(2),
        .settings_button = imageTextures.at(3),
        .screenshot_button = imageTextures.at(4),
    };
}

inline WindowParams GetWindowParams(const std::unique_ptr<CSimpleIniA> &ini) {
    constexpr int default_width = 800;
    constexpr int default_height = 600;
    constexpr int default_posX = 100;
    constexpr int default_posY = 100;

    constexpr char default_windowName[] = "WebFrame";
    const std::string default_windowSize = std::to_string(default_width) + ", " + std::to_string(default_height);
    const std::string default_windowPos = std::to_string(default_posX) + ", " + std::to_string(default_posY);

    const std::string windowName = ini->GetValue("Window", "Name", default_windowName);
    const std::string windowSize = ini->GetValue("Window", "Size", default_windowSize.c_str());
    const std::string windowPos = ini->GetValue("Window", "Position", default_windowPos.c_str());

    std::vector<std::string> windowSizes = StringUtils::Split(windowSize, ',');
    std::vector<std::string> windowPositions = StringUtils::Split(windowPos, ',');

    if (windowSizes.size() != 2) windowSizes = {std::to_string(default_width), std::to_string(default_height)};
    if (windowPositions.size() != 2) windowPositions = {std::to_string(default_posX), std::to_string(default_posY)};

    return {
        windowName, // Application Window Name
        StringUtils::TryParseInt(windowPositions[0], default_posX),
        StringUtils::TryParseInt(windowPositions[1], default_posY),
        StringUtils::TryParseInt(windowSizes[0], default_width),
        StringUtils::TryParseInt(windowSizes[1], default_height)
    };
}

inline void SaveWindowPosition(const std::unique_ptr<CSimpleIniA> &ini, const RECT &winRect) {
    if (ini->GetBoolValue("Window", "RestorePosition", false)) {
        const LONG width = winRect.right - winRect.left;
        const LONG height = winRect.bottom - winRect.top;
        const LONG posX = winRect.left;
        const LONG posY = winRect.top;

        const std::string size = std::to_string(width) + ", " + std::to_string(height);
        const std::string pos = std::to_string(posX) + ", " + std::to_string(posY);

        ini->SetValue("Window", "Size", size.c_str());
        ini->SetValue("Window", "Position", pos.c_str());
    }
}

inline SettingsArgs GetSettingsArgs(const std::unique_ptr<CSimpleIniA> &ini) {
    return {
        .website_url = ini->GetValue("Webview", "Homepage", "https://www.google.com"),
        .env_options = ini->GetValue("Webview", "EnvironmentOptions", ""),
        .topmost = ini->GetBoolValue("WindowProps", "Topmost", false),
        .borderless = ini->GetBoolValue("WindowProps", "Borderless", false),
        .toolWindow = ini->GetBoolValue("WindowProps", "ToolWindow", false),
        .cursorLock = ini->GetBoolValue("WindowProps", "CursorLock", false),
        .transparency = ini->GetLongValue("WindowProps", "Transparency", 255),
        .quitHotKey = ini->GetValue("HotKeys", "Quit", ""),
        .visibilityHotKey = ini->GetValue("HotKeys", "Visibility", ""),
        .clickThroughHotKey = ini->GetValue("HotKeys", "ClickThrough", "")
    };
}

inline SettingsCallbacks GetSettingsCallbacks(const std::unique_ptr<CSimpleIniA> &ini, const HWND &hwnd) {
    // clang-format off
    return {
        .websiteUrlCallback = [&ini](std::string url) {
            ini->SetValue("Webview", "Homepage", url.c_str());
        },
        .envOptionsCallback = [&ini](std::string envOptions) {
            ini->SetValue("Webview", "EnvironmentOptions", envOptions.c_str());
        },
        .alwaysOnTopCallback = [&ini, &hwnd](bool topmost) {
            WndCtrl::SetWindowTopmost(hwnd, topmost);
            ini->SetBoolValue("WindowProps", "Topmost", topmost);
        },
        .borderlessCallback = [&ini, &hwnd](bool borderless) {
            WndCtrl::SetWindowBorderless(hwnd, borderless);
            ini->SetBoolValue("WindowProps", "Borderless", borderless);
        },
        .toolWindowCallback = [&ini, &hwnd](bool toolWindow) {
            WndCtrl::SetToolWindowMode(hwnd, toolWindow);
            ini->SetBoolValue("WindowProps", "ToolWindow", toolWindow);
        },
        .cursorLockCallback = [&ini, &hwnd](bool cursorLock) {
            WndCtrl::SetCursorShapeLock(hwnd, cursorLock);
            ini->SetBoolValue("WindowProps", "CursorLock", cursorLock);
        },
        .transparencyCallback = [&ini, &hwnd](int transparancy) {
            WndCtrl::SetTransparency(hwnd, transparancy);
            ini->SetLongValue("WindowProps", "Transparency", transparancy);
        }
    };
    // clang-format on
}

inline void ApplyInitialSettings(const SettingsCallbacks &settingsCallbacks, const SettingsArgs &settingsArgs) {
    settingsCallbacks.alwaysOnTopCallback(settingsArgs.topmost);
    settingsCallbacks.borderlessCallback(settingsArgs.borderless);
    settingsCallbacks.toolWindowCallback(settingsArgs.toolWindow);
    settingsCallbacks.cursorLockCallback(settingsArgs.cursorLock);
    settingsCallbacks.transparencyCallback(settingsArgs.transparency);
}

inline void RegisterKeybinds(const SettingsArgs &settingsArgs, Window &window, const HWND &hwnd) {
    KeybindListener::RegisterKeybind(settingsArgs.quitHotKey, [&window]() {
        window.StopRunning();
    });
    KeybindListener::RegisterKeybind(settingsArgs.visibilityHotKey, [&hwnd]() {
        WndCtrl::ToggleWindowVisibility(hwnd);
    });
    KeybindListener::RegisterKeybind(settingsArgs.clickThroughHotKey, [&hwnd]() {
        static bool enable = false;
        WndCtrl::EnableClickThrough(hwnd, enable = !enable);
    });
}

class ScreenshotManager {
  public:
    ScreenshotManager(ID3D11Device *device) : device(device) {}
    ~ScreenshotManager() { Release(); }

    void Capture() {
        Release();

        if (!Utils::CaptureScreenshot(buffer, width, height)) {
            Log::Error("Failed to capture screenshot");
            return;
        }
        textureView = Utils::CreateDx11TextureBGRA(
            buffer.data(), width,
            height, device
        );
    }

    void Capture(bool value) {
        if (value)
            Capture();
        else
            Release();
    }

    void CopyToClipboard() {
        if (textureView == nullptr) {
            Log::Error("No screenshot captured");
            return;
        }

        if (!Utils::CopyImageToClipboard(buffer, width, height)) {
            Log::Error("Failed to copy screenshot to clipboard");
        }
    }

    void Release() {
        if (textureView != nullptr) {
            textureView->Release();
            textureView = nullptr;
        }
    }

    ScreenshotImage GetImage() const {
        return {
            .width = width,
            .height = height,
            .textureView = textureView
        };
    }

  private:
    std::vector<BYTE> buffer;
    int width;
    int height;
    ID3D11Device *device = nullptr;
    ID3D11ShaderResourceView *textureView = nullptr;
};