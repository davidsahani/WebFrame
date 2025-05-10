#include "widgets.hpp"

// Forward declaration
bool ToggleSwitch(const char *str_id, bool *v);

inline void AddColumnSpacing(const float &spacing) {
    ImGui::Dummy(ImVec2(spacing, 0));
    ImGui::SameLine();
}

void Widgets::Settings(SettingsArgs &args, const SettingsCallbacks &callbacks) {
    ImGui::SeparatorText("Webview Settings");

    if (ImGui::BeginTable("WebviewTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);

        // Row 1 - Homepage
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Homepage:");

        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
        if (Widgets::InputTextWithHint("##URL", "Enter website URL", args.website_url)) {
            CALL_IF_VALID(callbacks.websiteUrlCallback, args.website_url);
        }

        // Row 2 - ENV Options
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("ENV Options:");

        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
        if (Widgets::InputTextWithHint("##ENV-OPTS", "Enter environment options", args.env_options)) {
            CALL_IF_VALID(callbacks.envOptionsCallback, args.env_options);
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Window Controls");

    if (ImGui::BeginTable("WindowTable", 2, ImGuiTableFlags_SizingFixedFit)) {
        // Row 1 - Always On Top
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Always On Top");

        ImGui::TableSetColumnIndex(1);
        if (ToggleSwitch("##Topmost", &args.topmost)) {
            CALL_IF_VALID(callbacks.alwaysOnTopCallback, args.topmost);
        }

        // Row 2 - Tool Window Mode
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Lock Cursor Shape");

        ImGui::TableSetColumnIndex(1);
        if (ToggleSwitch("##CursorLock", &args.cursorLock)) {
            CALL_IF_VALID(callbacks.cursorLockCallback, args.cursorLock);
        }

        // Row 3 - Borderless Window Mode
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Borderless Window Mode");

        ImGui::TableSetColumnIndex(1);
        if (ToggleSwitch("##BorderlessMode", &args.borderless)) {
            CALL_IF_VALID(callbacks.borderlessCallback, args.borderless);
        }

        // Row 4 - Tool Window Mode
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Tool Window Mode");

        ImGui::TableSetColumnIndex(1);
        if (ToggleSwitch("##ToolWindowMode", &args.toolWindow)) {
            CALL_IF_VALID(callbacks.toolWindowCallback, args.toolWindow);
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();

    ImGui::Text("Transparency:");
    ImGui::SameLine();
    if (ImGui::SliderInt("##TSlider", &args.transparency, 50, 255, "%d")) {
        CALL_IF_VALID(callbacks.transparencyCallback, args.transparency);
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Keyboard Shortcuts");

    if (ImGui::BeginTable("KeyboardTable", 3, ImGuiTableFlags_SizingFixedFit)) {
        constexpr float columnSpacing = 15.0f;

        // Row 1 - Visibility Hotkey
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Quit Application");

        ImGui::TableSetColumnIndex(1);
        AddColumnSpacing(columnSpacing);
        ImGui::Text(args.quitHotKey.c_str());

        // Row 2 - Visibility Hotkey
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Show/Hide Window");

        ImGui::TableSetColumnIndex(1);
        AddColumnSpacing(columnSpacing);
        ImGui::Text(args.visibilityHotKey.c_str());

        // Row 3 - Passthrough Hotkey
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Allow Window ClickThrough");

        ImGui::TableSetColumnIndex(1);
        AddColumnSpacing(columnSpacing);
        ImGui::Text(args.clickThroughHotKey.c_str());

        ImGui::EndTable();
    }
}

bool ToggleSwitch(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    constexpr float height = 20.0f;
    constexpr float width = height * 1.8f;
    constexpr float radius = height * 0.5f;

    bool clicked = ImGui::InvisibleButton(str_id, ImVec2(width, height));

    if (ImGui::IsItemClicked())
        *v = !(*v);

    ImU32 col_bg = *v ? IM_COL32(0, 122, 255, 255) : IM_COL32(200, 200, 200, 255);
    ImU32 col_circle = IM_COL32(255, 255, 255, 255);

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, radius);
    draw_list->AddCircleFilled(ImVec2(p.x + (*v ? width - radius : radius), p.y + radius), radius - 2, col_circle);

    return clicked;
}