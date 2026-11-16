#include "menu.hpp"

#include <imgui.h>

namespace adrenaline {

namespace {

struct KeyOption {
    const char* label;
    int virtualKey;
};

const KeyOption kKeyOptions[] = {
    { "Right Mouse", VK_RBUTTON },
    { "Left Mouse", VK_LBUTTON },
    { "Middle Mouse", VK_MBUTTON },
    { "Shift", VK_SHIFT },
    { "Control", VK_CONTROL },
    { "Alt", VK_MENU },
    { "Caps Lock", VK_CAPITAL },
    { "E", 'E' },
    { "Q", 'Q' },
    { "X", 'X' },
    { "C", 'C' },
    { "V", 'V' },
    { "F", 'F' },
    { "G", 'G' },
};

const char* keyName(int virtualKey) {
    for (const KeyOption& option : kKeyOptions) {
        if (option.virtualKey == virtualKey) {
            return option.label;
        }
    }
    return "Unknown";
}

void drawKeySelector(const char* id, int& virtualKey) {
    if (ImGui::BeginCombo(id, keyName(virtualKey))) {
        for (const KeyOption& option : kKeyOptions) {
            const bool selected = (option.virtualKey == virtualKey);
            if (ImGui::Selectable(option.label, selected)) {
                virtualKey = option.virtualKey;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

} // namespace

void drawMenu(Config& config, const Game& game, bool& open) {
    if (!open) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(420.f, 520.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(60.f, 60.f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("adrenaline.win", &open, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImGui::Text("adrenaline.win");
    ImGui::SameLine();
    ImGui::TextDisabled("| %s", game.isAttached() ? "attached" : "detached");
    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox("Enabled", &config.esp.enabled);
            ImGui::Checkbox("Boxes", &config.esp.boxes);
            ImGui::Checkbox("Names", &config.esp.names);
            ImGui::Checkbox("Health bar", &config.esp.health);
            ImGui::Checkbox("Distance / HP text", &config.esp.distance);
            ImGui::Checkbox("Head dot", &config.esp.headDot);
            ImGui::Checkbox("Snaplines", &config.esp.snaplines);
            ImGui::Checkbox("Team check", &config.esp.teamCheck);
            ImGui::SliderFloat("Max distance", &config.esp.maxDistance, 100.f, 5000.f, "%.0f");
            ImGui::ColorEdit4("Enemy color", config.esp.boxColor);
            ImGui::ColorEdit4("Team color", config.esp.teamColor);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Checkbox("Enabled", &config.aimbot.enabled);
            ImGui::Checkbox("Team check", &config.aimbot.teamCheck);
            ImGui::Checkbox("Visible check", &config.aimbot.visibleCheck);
            ImGui::Checkbox("Hold key", &config.aimbot.holdKey);
            ImGui::Checkbox("Target head", &config.aimbot.targetHead);
            drawKeySelector("Aim key", config.aimbot.aimKey);
            ImGui::SliderFloat("FOV", &config.aimbot.fov, 10.f, 800.f, "%.0f px");
            ImGui::SliderFloat("Smooth", &config.aimbot.smooth, 1.f, 20.f, "%.1f");
            ImGui::SliderFloat("Sensitivity", &config.aimbot.sensitivity, 0.1f, 5.f, "%.2f");
            ImGui::SliderFloat("Max distance", &config.aimbot.maxDistance, 100.f, 3000.f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Misc")) {
            drawKeySelector("Menu key", config.menu.toggleKey);
            ImGui::Checkbox("Stream proof", &config.streamProof);
            if (ImGui::Button("Save config")) {
                config.save("adrenaline.ini");
            }
            ImGui::SameLine();
            if (ImGui::Button("Load config")) {
                config.load("adrenaline.ini");
            }
            ImGui::Separator();
            ImGui::TextDisabled("Players tracked: %d", static_cast<int>(game.players().size()));
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace adrenaline
