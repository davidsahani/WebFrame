#pragma once
#include <string>
#include <vector>
#include <functional>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#define CALL_IF_VALID(fn, ...)   \
    do {                         \
        if (fn) fn(__VA_ARGS__); \
    } while (0)

struct OmniBarImageTextures {
    ImTextureID backward_button;
    ImTextureID forward_button;
    ImTextureID refresh_button;
    ImTextureID settings_button;
    ImTextureID screenshot_button;
};

struct OmniBarCallbacks {
    std::function<void()> backwardButtonCallback;
    std::function<void()> forwardButtonCallback;
    std::function<void()> refreshButtonCallback;
    std::function<void(std::string)> urlCallback;
    std::function<void()> screenshotButtonCallback;
    std::function<void()> settingsButtonCallback;
};

struct SettingsArgs {
    std::string website_url;
    std::string env_options;
    bool topmost;
    bool borderless;
    bool toolWindow;
    bool cursorLock;
    int transparency;
    std::string quitHotKey;
    std::string visibilityHotKey;
    std::string clickThroughHotKey;
};

struct SettingsCallbacks {
    std::function<void(std::string)> websiteUrlCallback;
    std::function<void(std::string)> envOptionsCallback;
    std::function<void(bool)> alwaysOnTopCallback;
    std::function<void(bool)> borderlessCallback;
    std::function<void(bool)> toolWindowCallback;
    std::function<void(bool)> cursorLockCallback;
    std::function<void(int)> transparencyCallback;
};

struct ScreenshotImage {
    int width;
    int height;
    ID3D11ShaderResourceView *textureView = nullptr;
};

struct ScreenshotCallbacks {
    std::function<void()> copyToClipboardCallback;
};

namespace Widgets {

void OmniBar(std::string &url, const OmniBarImageTextures &textures, const OmniBarCallbacks &callbacks);
void Settings(SettingsArgs &args, const SettingsCallbacks &callbacks);
void Screenshot(const ScreenshotImage &screenshotImage, const ScreenshotCallbacks &callbacks);
bool InputText(const char *label, std::string &str);
bool InputTextWithHint(const char *label, const char *hint, std::string &str);

} // namespace Widgets