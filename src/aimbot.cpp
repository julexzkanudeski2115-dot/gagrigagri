#include "aimbot.hpp"

#include <windows.h>

#include <cfloat>
#include <cmath>
#include <cstdlib>

namespace adrenaline {

namespace {

bool isKeyDown(int virtualKey) {
    return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

void moveMouseRelative(float dx, float dy) {
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dx = static_cast<LONG>(std::lround(dx));
    input.mi.dy = static_cast<LONG>(std::lround(dy));
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &input, sizeof(INPUT));
}

} // namespace

void tickAimbot(const Game& game, const Config& config) {
    const AimbotSettings& settings = config.aimbot;
    if (!settings.enabled || settings.smooth <= 0.f) {
        return;
    }

    if (settings.holdKey && !isKeyDown(settings.aimKey)) {
        return;
    }

    const float screenCenterX = game.screenWidth() * 0.5f;
    const float screenCenterY = game.screenHeight() * 0.5f;
    const float fovRadius = settings.fov * 0.5f;

    const Player* best = nullptr;
    float bestDistance = FLT_MAX;

    for (const Player& player : game.players()) {
        if (!player.valid || player.isLocal) {
            continue;
        }
        if (settings.teamCheck && player.isTeammate) {
            continue;
        }
        if (player.distance > settings.maxDistance || player.health <= 0.f) {
            continue;
        }

        const Vector3 aimPoint = settings.targetHead ? player.headPosition : player.rootPosition;
        Vector2 screen{};
        if (!game.worldToScreen(aimPoint, screen)) {
            continue;
        }

        const float dx = screen.x - screenCenterX;
        const float dy = screen.y - screenCenterY;
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance > fovRadius || distance >= bestDistance) {
            continue;
        }

        bestDistance = distance;
        best = &player;
    }

    if (!best) {
        return;
    }

    const Vector3 aimPoint = settings.targetHead ? best->headPosition : best->rootPosition;
    Vector2 screen{};
    if (!game.worldToScreen(aimPoint, screen)) {
        return;
    }

    const float deltaX = screen.x - screenCenterX;
    const float deltaY = screen.y - screenCenterY;

    // Smoothing divides the raw delta; sensitivity scales it back for the game's
    // mouse sensitivity. Values are in raw mouse counts.
    const float stepX = (deltaX / settings.smooth) * settings.sensitivity;
    const float stepY = (deltaY / settings.smooth) * settings.sensitivity;

    if (std::fabs(stepX) < 0.5f && std::fabs(stepY) < 0.5f) {
        return;
    }

    moveMouseRelative(stepX, stepY);
}

} // namespace adrenaline
