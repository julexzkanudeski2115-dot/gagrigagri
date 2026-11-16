#include "aimbot.hpp"
#include "config.hpp"
#include "esp.hpp"
#include "game.hpp"
#include "menu.hpp"
#include "overlay.hpp"

#include <windows.h>
#include <imgui.h>

#include <cstring>

using namespace adrenaline;

namespace {

struct WindowSearch {
    DWORD pid;
    HWND result;
};

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM param) {
    auto* search = reinterpret_cast<WindowSearch*>(param);
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);

    if (windowPid != search->pid || !IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER) != nullptr) {
        return TRUE;
    }

    wchar_t className[256]{};
    GetClassNameW(hwnd, className, 256);
    if (_wcsicmp(className, L"WINDOWSCLIENT") == 0 || GetWindowTextLengthW(hwnd) > 0) {
        search->result = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND findWindowByPid(DWORD pid) {
    WindowSearch search{ pid, nullptr };
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&search));
    return search.result;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    Config config;
    config.load("adrenaline.ini");

    Game game;
    if (!game.init(L"RobloxPlayerBeta.exe")) {
        MessageBoxW(nullptr, L"Could not find RobloxPlayerBeta.exe.\nLaunch Roblox, then run adrenaline.win.",
                    L"adrenaline.win", MB_OK | MB_ICONERROR);
        return 1;
    }

    Overlay overlay;
    if (!overlay.create(L"adrenaline.win", L"AdrenalineOverlay")) {
        MessageBoxW(nullptr, L"Overlay creation failed.", L"adrenaline.win", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND robloxWindow = findWindowByPid(game.pid());
    RECT lastRect{};
    bool menuOpen = true;
    bool insertHeld = false;
    bool streamProofApplied = false;

    overlay.run(
        [&]() {
            // --- follow the Roblox window ---
            if (!robloxWindow || !IsWindow(robloxWindow)) {
                robloxWindow = findWindowByPid(game.pid());
            }
            if (robloxWindow) {
                RECT client{};
                GetClientRect(robloxWindow, &client);
                POINT topLeft{ client.left, client.top };
                ClientToScreen(robloxWindow, &topLeft);

                const int width = client.right - client.left;
                const int height = client.bottom - client.top;
                const RECT target{ topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height };

                if (std::memcmp(&target, &lastRect, sizeof(RECT)) != 0) {
                    lastRect = target;
                    MoveWindow(overlay.hwnd(), target.left, target.top, width, height, FALSE);
                    game.setScreenSize(width, height);
                }
            }

            // --- menu toggle ---
            const bool insertDown = (GetAsyncKeyState(config.menu.toggleKey) & 0x8000) != 0;
            if (insertDown && !insertHeld) {
                menuOpen = !menuOpen;
            }
            insertHeld = insertDown;
            overlay.setClickThrough(!menuOpen);

            // --- stream proof ---
            if (config.streamProof != streamProofApplied) {
                streamProofApplied = config.streamProof;
                SetWindowDisplayAffinity(overlay.hwnd(), streamProofApplied ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
            }

            game.update();

            ImGui::GetIO().MouseDrawCursor = menuOpen;
        },
        [&]() {
            if (config.streamProof) {
                return;
            }
            drawMenu(config, game, menuOpen);
            drawEsp(ImGui::GetBackgroundDrawList(), game, config);
            tickAimbot(game, config);
        });

    config.save("adrenaline.ini");
    overlay.destroy();
    game.shutdown();
    return 0;
}
