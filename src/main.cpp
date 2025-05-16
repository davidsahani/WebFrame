#include "main.hpp"

int main() {
    const char iniFilename[] = "settings.ini";
    std::unique_ptr<CSimpleIniA> ini = LoadConfig(iniFilename);
    const WindowParams windowParams = GetWindowParams(ini);

    Window window(windowParams);

    if (!window.Init()) {
        Log::Critical("Failed to initialize window");
        return 1;
    }

    const HWND hwnd = window.GetWindowHandle();

    RECT winRect = {
        windowParams.posX, windowParams.posY,
        windowParams.posX + windowParams.width,
        windowParams.posY + windowParams.height
    };
    window.OnMove([&winRect, &hwnd]() {
        GetWindowRect(hwnd, &winRect);
    });

    // Set Window name for MessageBox
    Log::SetWindowName(windowParams.windowName);
    // Set Log filename, if empty MessageBox will be used.
    Log::SetLogFile(ini->GetValue("Logging", "Filename", ""));

    SettingsArgs settingsArgs = GetSettingsArgs(ini);
    SettingsCallbacks settingsCallbacks = GetSettingsCallbacks(ini, hwnd);
    ApplyInitialSettings(settingsCallbacks, settingsArgs);
    // Install and Register keybinds
    KeybindListener::InstallHook();
    RegisterKeybinds(settingsArgs, window, hwnd);

    WebView webview(hwnd, settingsArgs.env_options);
    // Find WebView2 window handle attached to the main ImGui window
    HWND webviewHwnd = FindWindowEx(hwnd, nullptr, "Chrome_WidgetWin_0", nullptr);

    constexpr float omniBarHeight = 28.0f;
    std::string website_url = settingsArgs.website_url;
    RECT bounds = {0, 0, 0, 0};  // ImGui window bounds
    RECT wBounds = {0, 0, 0, 0}; // WebView window bounds

    window.OnResize([&webview, &hwnd, &bounds, &omniBarHeight, &webviewHwnd, &wBounds]() {
        GetClientRect(hwnd, &bounds);
        bounds.top += static_cast<LONG>(omniBarHeight + 4.8f);
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

    ScreenshotManager screenshotManager(window.GetDevice());
    ScreenshotCallbacks screenshotCallbacks = {
        [&screenshotManager]() { screenshotManager.CopyToClipboard(); }
    };

    const OmniBarImageTextures omniBarTextures =
        LoadOmniBarImageTextures(window.GetDevice());

    bool showSettings = false;   // settings visibility flag
    bool showScreenshot = false; // screenshot visibility flag

    OmniBarCallbacks omniBarCallbacks = {
        [&webview]() { webview.GoBack(); },
        [&webview]() { webview.GoForward(); },
        [&webview]() { webview.Reload(); },
        [&webview](std::string url) { webview.Navigate(url); },
        [&showScreenshot, &screenshotManager]() {
            showScreenshot = !showScreenshot;
            screenshotManager.Capture(showScreenshot);
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
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, omniBarHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f)); // Slightly lighter gray background

        ImGui::Begin("OmniBar", nullptr, windowFlags);
        Widgets::OmniBar(website_url, omniBarTextures, omniBarCallbacks);
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
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2, omniBarHeight + 10), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
                // Set initial window width and height
                ImGui::SetNextWindowSize(ImVec2(600, 391));
                initialScreenshotSizeSet = true;
            }
            ImGui::Begin("Screenshot", &showScreenshot, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
            Widgets::Screenshot(screenshotManager.GetImage(), screenshotCallbacks);
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
            constexpr float spacing = omniBarHeight + 4.5F;
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
                screenshotManager.Release();
                initialScreenshotSizeSet = false;
            }
            clipped = false;
        }

        window.Render();
    }

    KeybindListener::UninstallHook();
    SaveWindowPosition(ini, winRect);
    return ini->SaveFile(iniFilename);
}