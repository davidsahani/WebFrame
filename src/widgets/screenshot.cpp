#include "widgets.hpp"

void Widgets::Screenshot(const ScreenshotImage &screenshotImage, const ScreenshotCallbacks &callbacks) {
    // Get available size inside the window (excluding padding)
    const ImVec2 availableSize = ImGui::GetContentRegionAvail();

    // Get original image dimensions
    const float imgWidth = static_cast<float>(screenshotImage.width);
    const float imgHeight = static_cast<float>(screenshotImage.height);

    // Calculate aspect-ratio-preserving size
    const float scale = min(availableSize.x / imgWidth, availableSize.y / imgHeight);
    const ImVec2 newSize = ImVec2(imgWidth * scale, imgHeight * scale);

    if (screenshotImage.textureView != nullptr) {
        ImGui::Image((ImTextureID)(intptr_t)screenshotImage.textureView, newSize);
    } else {
        // Center text within the reserved (newSize) image space
        const ImVec2 spacingY = {newSize.x, newSize.y / 2.2f};
        ImGui::Dummy(spacingY);
        ImGui::Dummy({newSize.x / 3.0f, 0.0f});
        ImGui::SameLine();
        ImGui::Text("Screenshot not available");
        ImGui::Dummy(spacingY);
    }

    static const ImGuiIO &io = ImGui::GetIO();

    ImGui::Text("Coordinates x: %.f, y: %.f", io.MousePos.x, io.MousePos.y);
    // Move the "Copy to Clipboard" button to the right
    constexpr float buttonWidth = 130.0f;
    const float cursorPosX = availableSize.x + ImGui::GetCursorPosX() - buttonWidth;
    ImGui::SameLine(cursorPosX);

    if (ImGui::Button("Copy to Clipboard", ImVec2(buttonWidth, 0))) {
        CALL_IF_VALID(callbacks.copyToClipboardCallback);
    }
}