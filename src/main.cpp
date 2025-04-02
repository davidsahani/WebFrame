#include <SimpleIni.h>
#include "window.hpp"
#include "widgets.hpp"
#include "webview.hpp"
#include "wndctrl.hpp"
#include "keybind_listener.hpp"
#include "utils.hpp"
#include "Log.hpp"

int main() {
    Window window(L"WebFrame", 800, 600);

    if (!window.Init()) {
        return 1;
    }

    CSimpleIniA ini;
    ini.SetUnicode(); // Use UTF-8
    constexpr char iniFilename[] = "settings.ini";

    if (ini.LoadFile(iniFilename) < 0) {
        Log::Error("Failed to load INI file: %s", iniFilename);
    }

    const ImageTextures imageTextures = Utils::LoadImageTextures(
        {
            {"BackwordButton", "icons/backward.png"},
            {"ForwardButton", "icons/forward.png"},
            {"RefreshButton", "icons/refresh.png"},
            {"SettingsButton", "icons/settings.png"},
            {"ScreenshotButton", "icons/screenshot.png"},
        },
        window.GetDevice()
    );

    const OmniBarImageTextures omnibarTextures = {
        imageTextures.Get("BackwordButton"),
        imageTextures.Get("ForwardButton"),
        imageTextures.Get("RefreshButton"),
        imageTextures.Get("SettingsButton"),
        imageTextures.Get("ScreenshotButton"),
    };

    SettingsArgs settingsArgs = {
        ini.GetValue("Webview", "Homepage", "https://www.google.com"),
        ini.GetValue("Webview", "EnvironmentOptions", ""),
        ini.GetBoolValue("Window", "Topmost", false),
        ini.GetBoolValue("Window", "Borderless", false),
        ini.GetBoolValue("Window", "ToolWindow", false),
        ini.GetBoolValue("Window", "CursorLock", false),
        ini.GetLongValue("Window", "Transparency", 255),
        ini.GetValue("HotKeys", "Quit", ""),
        ini.GetValue("HotKeys", "Visibility", ""),
        ini.GetValue("HotKeys", "ClickThrough", "")
    };

    HWND hwnd = window.GetWindowHandle();

    SettingsCallbacks settingsCallbacks = {
        [&ini](std::string url) {
            ini.SetValue("Webview", "Homepage", url.c_str());
        },
        [&ini](std::string envOptions) {
            ini.SetValue("Webview", "EnvironmentOptions", envOptions.c_str());
        },
        [&ini, &hwnd](bool topmost) {
            WndCtrl::SetWindowTopmost(hwnd, topmost);
            ini.SetBoolValue("Window", "Topmost", topmost);
        },
        [&ini, &hwnd](bool borderless) {
            WndCtrl::SetWindowBorderless(hwnd, borderless);
            ini.SetBoolValue("Window", "Borderless", borderless);
        },
        [&ini, &hwnd](bool toolWindow) {
            WndCtrl::SetToolWindowMode(hwnd, toolWindow);
            ini.SetBoolValue("Window", "ToolWindow", toolWindow);
        },
        [&ini, &hwnd](bool cursorLock) {
            WndCtrl::SetCursorShapeLock(hwnd, cursorLock);
            ini.SetBoolValue("Window", "CursorLock", cursorLock);
        },
        [&ini, &hwnd](int transparancy) {
            WndCtrl::SetTransparency(hwnd, transparancy);
            ini.SetLongValue("Window", "Transparency", transparancy);
        }
    };

    { // Apply settings on initialization
        settingsCallbacks.alwaysOnTopCallback(settingsArgs.topmost);
        settingsCallbacks.borderlessCallback(settingsArgs.borderless);
        settingsCallbacks.toolWindowCallback(settingsArgs.toolWindow);
        settingsCallbacks.cursorLockCallback(settingsArgs.cursorLock);
        settingsCallbacks.transparencyCallback(settingsArgs.transparency);
    }

    { // Install and Register keybinds
        KeybindListener::InstallHook();

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

    WebView webview(hwnd, settingsArgs.env_options);
    // Find WebView2 window handle attached to the main ImGui window
    HWND webviewHwnd = FindWindowEx(hwnd, nullptr, "Chrome_WidgetWin_0", nullptr);

    constexpr float omnibarHeight = 28.0f;
    std::string website_url = settingsArgs.website_url;
    RECT bounds = {0, 0, 0, 0};  // ImGui window bounds
    RECT wBounds = {0, 0, 0, 0}; // WebView window bounds

    window.OnResize([&webview, &hwnd, &bounds, &omnibarHeight, &webviewHwnd, &wBounds]() {
        GetClientRect(hwnd, &bounds);
        bounds.top += static_cast<LONG>(omnibarHeight + 4.8f);
        webview.Resize(bounds);
        // Update WebView window bounds
        GetClientRect(webviewHwnd, &wBounds);
    });

    webview.OnInitialized([&window, &webview, &website_url]() {
        webview.Navigate(website_url);
        window.TriggerResizeCallback();
    });

    webview.OnUrlChange([&website_url](std::string url) {
        website_url = url;
    });

    bool showSettings = false;   // settings visibility flag
    bool showScreenshot = false; // screenshot visibility flag
    ScreenshotImage screenshotImage;

    auto createScreenshot = [&window, &screenshotImage](bool value) {
        if (value) {
            if (!Utils::CaptureScreenshot(screenshotImage.buffer, screenshotImage.width, screenshotImage.height)) {
                Log::Error("Failed to capture screenshot");
                return;
            }
            screenshotImage.textureView = Utils::CreateDx11TextureBGRA(
                screenshotImage.buffer.data(), screenshotImage.width,
                screenshotImage.height, window.GetDevice()
            );
        } else if (screenshotImage.textureView != nullptr) {
            screenshotImage.textureView->Release();
            screenshotImage.textureView = nullptr;
        }
    };

    ScreenshotCallbacks screenshotCallbacks = {
        [&screenshotImage]() {
            if (!Utils::CopyImageToClipboard(screenshotImage.buffer, screenshotImage.width, screenshotImage.height)) {
                Log::Error("Failed to copy screenshot to clipboard");
            }
        }
    };

    OmniBarCallbacks omnibarCallbacks = {
        [&webview]() { webview.GoBack(); },
        [&webview]() { webview.GoForward(); },
        [&webview]() { webview.Reload(); },
        [&webview](std::string url) { webview.Navigate(url); },
        [&showScreenshot, &createScreenshot]() {
            showScreenshot = !showScreenshot;
            createScreenshot(showScreenshot);
        },
        [&showSettings]() { showSettings = !showSettings; }
    };

    const ImGuiIO &io = ImGui::GetIO();

    while (window.IsRunning()) {
        if (!window.Draw()) continue;

        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                                 ImGuiWindowFlags_NoCollapse;
        // Set position at the top-left corner
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        // Set full width and fixed height
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, omnibarHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f)); // Slightly lighter gray background

        ImGui::Begin("OmniBar", nullptr, windowFlags);
        Widgets::OmniBar(website_url, omnibarTextures, omnibarCallbacks);
        ImGui::End(); // End OmniBar

        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(2);

        static bool clipped = false;
        static bool initialScreenshotSizeSet = false;
        static RECT screenshotClipRect = RECT{0, 0, 0, 0};
        static RECT settingsClipRect = RECT{0, 0, 0, 0};

        if (showScreenshot) {
            if (!initialScreenshotSizeSet) {
                // Center horizontally
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2, omnibarHeight + 10), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
                // Set initial window width and height
                ImGui::SetNextWindowSize(ImVec2(600, 391));
                initialScreenshotSizeSet = true;
            }
            ImGui::Begin("Screenshot", &showScreenshot, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
            Widgets::Screenshot(screenshotImage, screenshotCallbacks);
            { // Update clipping rectangle and reset clipped flag on window move or resize
                const ImVec2 pos = ImGui::GetWindowPos();
                const ImVec2 size = ImGui::GetWindowSize();

                screenshotClipRect = RECT{
                    static_cast<LONG>(pos.x), static_cast<LONG>(pos.y),
                    static_cast<LONG>(pos.x + size.x),
                    static_cast<LONG>(pos.y + size.y)
                };

                static RECT prevRect = RECT{0, 0, 0, 0};
                static RECT prevBounds = RECT{0, 0, 0, 0};
                const bool rectChanged = memcmp(&screenshotClipRect, &prevRect, sizeof(RECT)) != 0;
                const bool boundsChanged = memcmp(&bounds, &prevBounds, sizeof(RECT)) != 0;

                if (rectChanged || boundsChanged) {
                    prevRect = screenshotClipRect;
                    prevBounds = bounds;
                    clipped = false;
                }
            }
            ImGui::End(); // End Settings
        }

        if (showSettings) {
            constexpr float settingsWidth = 500.0F;
            constexpr float spacing = omnibarHeight + 4.5F;
            constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            // Set position at the top-left corner
            ImGui::SetNextWindowPos(ImVec2(min(io.DisplaySize.x, io.DisplaySize.x - settingsWidth), spacing + 0.5F));
            // // Set full width and fixed height
            ImGui::SetNextWindowSize(ImVec2(settingsWidth, io.DisplaySize.y - spacing));

            ImGui::Begin("Settings", &showSettings, flags);
            Widgets::Settings(settingsArgs, settingsCallbacks);
            { // Update clipping rectangle and reset clipped flag on window move
                const ImVec2 pos = ImGui::GetWindowPos();
                const ImVec2 size = ImGui::GetWindowSize();

                settingsClipRect = RECT{
                    static_cast<LONG>(pos.x), static_cast<LONG>(pos.y),
                    static_cast<LONG>(pos.x + size.x),
                    static_cast<LONG>(pos.y + size.y)
                };

                static RECT prevRect = RECT{0, 0, 0, 0};

                if (memcmp(&settingsClipRect, &prevRect, sizeof(RECT)) != 0) {
                    prevRect = settingsClipRect;
                    clipped = false;
                }
            }
            ImGui::End(); // End Settings
        }

        if ((showSettings || showScreenshot) && !clipped) {
            // Create full region covering the child window
            HRGN fullRegion = CreateRectRgn(wBounds.top, wBounds.left, wBounds.right, wBounds.bottom);
            const LONG heightDelta = bounds.bottom - wBounds.bottom;

            if (showSettings && showScreenshot) {
                // Create clipping region for the screenshot window
                screenshotClipRect.top -= heightDelta;
                screenshotClipRect.bottom -= heightDelta;
                const HRGN screenshotTransparentRegion = CreateRectRgn(
                    screenshotClipRect.left, screenshotClipRect.top,
                    screenshotClipRect.right, screenshotClipRect.bottom
                );
                CombineRgn(fullRegion, fullRegion, screenshotTransparentRegion, RGN_DIFF);
                // Create clipping region for the settings window
                settingsClipRect.top -= heightDelta;
                settingsClipRect.bottom -= heightDelta;
                const HRGN settingsTransparentRegion = CreateRectRgn(
                    settingsClipRect.left, settingsClipRect.top,
                    settingsClipRect.right, settingsClipRect.bottom
                );
                CombineRgn(fullRegion, fullRegion, settingsTransparentRegion, RGN_DIFF);
                // Apply the updated clipping region
                SetWindowRgn(webviewHwnd, fullRegion, TRUE);
                // Cleanup
                DeleteObject(screenshotTransparentRegion);
                DeleteObject(settingsTransparentRegion);

            } else if (showSettings) {
                // Create a region for the new clipping area
                settingsClipRect.top -= heightDelta;
                settingsClipRect.bottom -= heightDelta;
                const HRGN transparentRegion = CreateRectRgn(
                    settingsClipRect.left, settingsClipRect.top,
                    settingsClipRect.right, settingsClipRect.bottom
                );
                CombineRgn(fullRegion, fullRegion, transparentRegion, RGN_DIFF);
                // Apply the updated clipping region
                SetWindowRgn(webviewHwnd, fullRegion, TRUE);
                // Cleanup
                DeleteObject(transparentRegion);

            } else if (showScreenshot) {
                // Create a region for the new clipping area
                screenshotClipRect.top -= heightDelta;
                screenshotClipRect.bottom -= heightDelta;
                const HRGN transparentRegion = CreateRectRgn(
                    screenshotClipRect.left, screenshotClipRect.top,
                    screenshotClipRect.right, screenshotClipRect.bottom
                );
                CombineRgn(fullRegion, fullRegion, transparentRegion, RGN_DIFF);
                // Apply the updated clipping region
                SetWindowRgn(webviewHwnd, fullRegion, TRUE);
                // Cleanup
                DeleteObject(transparentRegion);
            }
            // Cleanup
            DeleteObject(fullRegion);
            clipped = true;
        }

        static bool prevShowScreenshot = showScreenshot;
        static bool prevShowSettings = showSettings;

        if (prevShowSettings != showSettings || prevShowScreenshot != showScreenshot) {
            prevShowSettings = showSettings;
            prevShowScreenshot = showScreenshot;

            if (!showSettings && !showScreenshot) {
                // Apply the full WebView2 window region
                SetWindowRgn(webviewHwnd, nullptr, TRUE);
            }
            if (!showScreenshot) {
                createScreenshot(false);
                initialScreenshotSizeSet = false;
            }
            clipped = false;
        }

        window.Render();
    }

    KeybindListener::UninstallHook();
    return ini.SaveFile(iniFilename);
}