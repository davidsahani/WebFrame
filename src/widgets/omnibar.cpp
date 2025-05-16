#include "widgets.hpp"
#include <d3d11.h>
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#define BTN_WIDTH(B) ((B).size.x + (B).padding.x)

struct ButtonProperties {
    ImVec2 size;
    ImVec2 padding;
};

struct Button {
    static constexpr ButtonProperties Arrow = {{30, 30}, {0, 1}};
    static constexpr ButtonProperties Refresh = {{25, 25}, {5, 3.5}};
    static constexpr ButtonProperties Screenshot = {{25, 25}, {5, 3.5}};
    static constexpr ButtonProperties Settings = {{22, 22}, {5, 3.5}};
};

constexpr float CombinedWidth =
    BTN_WIDTH(Button::Arrow) * 2 +
    BTN_WIDTH(Button::Refresh) +
    BTN_WIDTH(Button::Screenshot) +
    BTN_WIDTH(Button::Settings) +
    46.0f; // MysteryPadding

void Widgets::OmniBar(std::string &url, const OmniBarImageTextures &textures, const OmniBarCallbacks &callbacks) {
    const float full_width = ImGui::GetContentRegionAvail().x; // Get available width
    const float input_width = full_width - CombinedWidth;      // Remaining space for address bar

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.12f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Button::Arrow.padding);

    if (ImGui::ImageButton("BackwardButton", textures.backward_button, Button::Arrow.size)) {
        CALL_IF_VALID(callbacks.backwardButtonCallback);
    }
    ImGui::SameLine();
    if (ImGui::ImageButton("ForwardButton", textures.forward_button, Button::Arrow.size)) {
        CALL_IF_VALID(callbacks.forwardButtonCallback);
    }

    ImGui::PopStyleVar();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Button::Refresh.padding);

    ImGui::SameLine();
    if (ImGui::ImageButton("RefreshButton", textures.refresh_button, Button::Refresh.size)) {
        CALL_IF_VALID(callbacks.refreshButtonCallback);
    }

    ImGui::PopStyleVar();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(input_width);
    // Address bar
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);                    // Rounded corners
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 5));             // Padding inside the bar
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f)); // Dark gray background
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));                   // White text

        ImGui::PushID("AddressBar");

        if (InputTextWithHint("##URL", "Enter website URL: ", url)) {
            CALL_IF_VALID(callbacks.urlCallback, url);
        }

        if (ImGui::IsItemActive()) {
            // Get item rectangle
            ImRect inputRect = ImGui::GetCurrentContext()->LastItemData.Rect;

            // If left clicking outside, clear focus
            if (!inputRect.Contains(ImGui::GetMousePos()) && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
                ImGui::ClearActiveID();
            }
        }

        ImGui::PopID();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    } // Address bar End

    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Button::Screenshot.padding);

    if (ImGui::ImageButton("ScreenshotButton", textures.screenshot_button, Button::Screenshot.size)) {
        CALL_IF_VALID(callbacks.screenshotButtonCallback);
    }

    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Button::Settings.padding);

    if (ImGui::ImageButton("SettingsButton", textures.settings_button, Button::Settings.size)) {
        CALL_IF_VALID(callbacks.settingsButtonCallback);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3); // Restore previous style
}