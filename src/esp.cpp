#include "esp.hpp"

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>

namespace adrenaline {

namespace {

ImU32 toColor(const float rgba[4]) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(rgba[0], rgba[1], rgba[2], rgba[3]));
}

void drawOutlinedText(ImDrawList* drawList, const ImVec2& position, const char* text, ImU32 color) {
    const ImU32 shadow = IM_COL32(0, 0, 0, 200);
    drawList->AddText(ImVec2(position.x + 1.f, position.y + 1.f), shadow, text);
    drawList->AddText(ImVec2(position.x - 1.f, position.y + 1.f), shadow, text);
    drawList->AddText(ImVec2(position.x + 1.f, position.y - 1.f), shadow, text);
    drawList->AddText(ImVec2(position.x - 1.f, position.y - 1.f), shadow, text);
    drawList->AddText(position, color, text);
}

// Projects the eight corners of an axis-aligned world box and returns the
// screen-space bounding rectangle.
bool projectBox(const Game& game, const Vector3& center, const Vector3& halfExtents, ImVec2& min, ImVec2& max) {
    min = ImVec2(FLT_MAX, FLT_MAX);
    max = ImVec2(-FLT_MAX, -FLT_MAX);

    for (int i = 0; i < 8; ++i) {
        const Vector3 corner{
            center.x + ((i & 1) ? halfExtents.x : -halfExtents.x),
            center.y + ((i & 2) ? halfExtents.y : -halfExtents.y),
            center.z + ((i & 4) ? halfExtents.z : -halfExtents.z),
        };
        Vector2 screen{};
        if (!game.worldToScreen(corner, screen)) {
            return false;
        }
        min.x = std::min(min.x, screen.x);
        min.y = std::min(min.y, screen.y);
        max.x = std::max(max.x, screen.x);
        max.y = std::max(max.y, screen.y);
    }
    return true;
}

} // namespace

void drawEsp(ImDrawList* drawList, const Game& game, const Config& config) {
    if (!drawList || !config.esp.enabled) {
        return;
    }

    const ImU32 boxColor = toColor(config.esp.boxColor);
    const ImU32 teamColor = toColor(config.esp.teamColor);
    const ImVec2 screenCenter(game.screenWidth() * 0.5f, game.screenHeight() * 0.5f);

    for (const Player& player : game.players()) {
        if (!player.valid || player.isLocal) {
            continue;
        }
        if (config.esp.teamCheck && player.isTeammate) {
            continue;
        }
        if (player.distance > config.esp.maxDistance) {
            continue;
        }

        // Character box: ~5 studs tall, ~3 studs wide, centred slightly above the root.
        const Vector3 center = player.rootPosition + Vector3{ 0.f, 1.0f, 0.f };
        const Vector3 halfExtents{ 1.5f, 3.0f, 1.5f };

        ImVec2 boxMin;
        ImVec2 boxMax;
        if (!projectBox(game, center, halfExtents, boxMin, boxMax)) {
            continue;
        }

        const float boxWidth = boxMax.x - boxMin.x;
        const float boxHeight = boxMax.y - boxMin.y;
        if (boxWidth < 1.f || boxHeight < 1.f || boxWidth > 4000.f || boxHeight > 4000.f) {
            continue;
        }

        const ImU32 color = player.isTeammate ? teamColor : boxColor;

        if (config.esp.snaplines) {
            drawList->AddLine(screenCenter, ImVec2((boxMin.x + boxMax.x) * 0.5f, boxMax.y), color, 1.0f);
        }

        if (config.esp.boxes) {
            drawList->AddRect(ImVec2(boxMin.x - 1.f, boxMin.y - 1.f), ImVec2(boxMax.x + 1.f, boxMax.y + 1.f), IM_COL32(0, 0, 0, 180), 0.f, 0, 1.0f);
            drawList->AddRect(boxMin, boxMax, color, 0.f, 0, 1.5f);
        }

        if (config.esp.health) {
            const float fraction = std::clamp(player.health / player.maxHealth, 0.f, 1.f);
            const float barX = boxMin.x - 6.f;
            const float barTop = boxMin.y;
            const float barBottom = boxMax.y;
            const float barHeight = barBottom - barTop;
            const ImU32 healthColor = IM_COL32(
                static_cast<int>(255.f * (1.f - fraction)),
                static_cast<int>(255.f * fraction),
                40, 255);

            drawList->AddRectFilled(ImVec2(barX, barTop), ImVec2(barX + 3.f, barBottom), IM_COL32(0, 0, 0, 180));
            drawList->AddRectFilled(ImVec2(barX, barBottom - barHeight * fraction), ImVec2(barX + 3.f, barBottom), healthColor);
        }

        if (config.esp.headDot) {
            Vector2 headScreen{};
            if (game.worldToScreen(player.headPosition, headScreen)) {
                drawList->AddCircleFilled(ImVec2(headScreen.x, headScreen.y), 3.f, color, 12);
            }
        }

        const float fontSize = ImGui::GetFontSize();
        float textY = boxMin.y - fontSize - 2.f;

        if (config.esp.names) {
            const char* label = player.displayName.empty() ? player.name.c_str() : player.displayName.c_str();
            const ImVec2 textSize = ImGui::CalcTextSize(label);
            drawOutlinedText(drawList, ImVec2((boxMin.x + boxMax.x) * 0.5f - textSize.x * 0.5f, textY), label, color);
            textY -= fontSize;
        }

        if (config.esp.distance) {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "%.0fm  %.0fhp", player.distance, player.health);
            const ImVec2 textSize = ImGui::CalcTextSize(buffer);
            drawOutlinedText(drawList, ImVec2((boxMin.x + boxMax.x) * 0.5f - textSize.x * 0.5f, textY), buffer, IM_COL32(230, 230, 230, 255));
        }
    }
}

} // namespace adrenaline
