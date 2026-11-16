#pragma once
#include "memory.hpp"
#include "offsets.hpp"
#include "vector.hpp"

#include <string>
#include <vector>

namespace adrenaline {

struct Player {
    uintptr_t address = 0;
    std::string name;
    std::string displayName;
    std::string team;
    Vector3 rootPosition{};
    Vector3 headPosition{};
    float health = 0.f;
    float maxHealth = 100.f;
    float distance = 0.f;
    bool isLocal = false;
    bool isTeammate = false;
    bool valid = false;
};

class Game {
public:
    bool init(const std::wstring& processName);
    void shutdown();

    bool update();

    const std::vector<Player>& players() const { return players_; }
    uintptr_t localPlayer() const { return localPlayer_; }
    bool isAttached() const { return memory_.isAttached(); }
    DWORD pid() const { return memory_.pid(); }

    bool worldToScreen(const Vector3& world, Vector2& out) const;

    int screenWidth() const { return screenWidth_; }
    int screenHeight() const { return screenHeight_; }
    void setScreenSize(int width, int height) { screenWidth_ = width; screenHeight_ = height; }

    const Vector3& cameraPosition() const { return cameraPosition_; }

private:
    uintptr_t getDataModel();
    std::vector<uintptr_t> getChildren(uintptr_t instance);
    std::string getClassName(uintptr_t instance);
    std::string getName(uintptr_t instance);
    uintptr_t findFirstChild(uintptr_t instance, const std::string& name);
    uintptr_t findFirstChildOfClass(uintptr_t instance, const std::string& className);
    std::string readStdString(uintptr_t address);

    Memory memory_;
    Offsets offsets_{};
    std::vector<Player> players_;

    uintptr_t fakeDataModel_ = 0;
    uintptr_t localPlayer_ = 0;
    uintptr_t camera_ = 0;

    Vector3 cameraPosition_{};
    Vector3 cameraRight_{};
    Vector3 cameraUp_{};
    Vector3 cameraLook_{};
    float cameraFov_ = 1.2217305f; // 70 degrees in radians

    int screenWidth_ = 1920;
    int screenHeight_ = 1080;
};

} // namespace adrenaline
