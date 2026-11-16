#include "game.hpp"

#include <algorithm>
#include <cmath>

namespace adrenaline {

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr size_t kMaxChildren = 4096;
constexpr size_t kMaxStringLength = 256;
} // namespace

bool Game::init(const std::wstring& processName) {
    return memory_.attach(processName);
}

void Game::shutdown() {
    memory_.detach();
    players_.clear();
}

std::string Game::readStdString(uintptr_t address) {
    if (!address) {
        return {};
    }
    // MSVC std::string: _Bx[16] | _Mysize(0x10) | _Myres(0x18)
    const size_t length = memory_.read<size_t>(address + 0x10);
    const size_t capacity = memory_.read<size_t>(address + 0x18);
    if (length == 0 || length > kMaxStringLength) {
        return {};
    }
    if (capacity >= 16) {
        const uintptr_t data = memory_.read<uintptr_t>(address);
        return memory_.readString(data, length);
    }
    return memory_.readString(address, length);
}

std::vector<uintptr_t> Game::getChildren(uintptr_t instance) {
    std::vector<uintptr_t> children;
    if (!instance) {
        return children;
    }
    const uintptr_t begin = memory_.read<uintptr_t>(instance + offsets_.ChildrenStart);
    const uintptr_t end = memory_.read<uintptr_t>(instance + offsets_.ChildrenEnd);
    if (!begin || !end || end <= begin) {
        return children;
    }
    size_t count = (end - begin) / sizeof(uintptr_t);
    if (count == 0 || count > kMaxChildren) {
        return children;
    }
    std::vector<uintptr_t> buffer(count);
    if (!memory_.readRaw(begin, buffer.data(), count * sizeof(uintptr_t))) {
        return children;
    }
    children.reserve(count);
    for (const uintptr_t child : buffer) {
        if (child) {
            children.push_back(child);
        }
    }
    return children;
}

std::string Game::getClassName(uintptr_t instance) {
    const uintptr_t descriptor = memory_.read<uintptr_t>(instance + offsets_.ClassDescriptor);
    if (!descriptor) {
        return {};
    }
    return readStdString(descriptor + offsets_.ClassDescriptorToName);
}

std::string Game::getName(uintptr_t instance) {
    return readStdString(instance + offsets_.Name);
}

uintptr_t Game::findFirstChild(uintptr_t instance, const std::string& name) {
    for (const uintptr_t child : getChildren(instance)) {
        if (getName(child) == name) {
            return child;
        }
    }
    return 0;
}

uintptr_t Game::findFirstChildOfClass(uintptr_t instance, const std::string& className) {
    for (const uintptr_t child : getChildren(instance)) {
        if (getClassName(child) == className) {
            return child;
        }
    }
    return 0;
}

uintptr_t Game::getDataModel() {
    if (!fakeDataModel_) {
        const std::optional<uintptr_t> instruction = memory_.scan(offsets_.fakeDataModelSignature);
        if (!instruction) {
            return 0;
        }
        // mov rcx, [rip + rel32]  ->  48 8B 0D <rel32>
        const int32_t relative = memory_.read<int32_t>(*instruction + 3);
        fakeDataModel_ = *instruction + 7 + relative;
    }
    const uintptr_t fake = memory_.read<uintptr_t>(fakeDataModel_);
    if (!fake) {
        return 0;
    }
    return memory_.read<uintptr_t>(fake + offsets_.FakeDataModelToDataModel);
}

bool Game::update() {
    if (!memory_.isAttached()) {
        return false;
    }

    const uintptr_t dataModel = getDataModel();
    if (!dataModel) {
        return false;
    }

    const uintptr_t workspace = findFirstChildOfClass(dataModel, "Workspace");
    const uintptr_t playersService = findFirstChildOfClass(dataModel, "Players");
    if (!workspace || !playersService) {
        return false;
    }

    // ---- Camera ----
    camera_ = findFirstChildOfClass(workspace, "Camera");
    if (camera_) {
        float rotation[9]{};
        if (memory_.readRaw(camera_ + offsets_.CameraRotation, rotation, sizeof(rotation))) {
            cameraRight_ = { rotation[0], rotation[3], rotation[6] };
            cameraUp_    = { rotation[1], rotation[4], rotation[7] };
            cameraLook_  = { -rotation[2], -rotation[5], -rotation[8] };
        }
        cameraPosition_ = memory_.read<Vector3>(camera_ + offsets_.CameraPosition);

        float fov = memory_.read<float>(camera_ + offsets_.CameraFov);
        if (fov > 3.2f) {
            fov *= kPi / 180.f; // stored in degrees
        }
        if (fov > 0.01f && fov < 3.1f) {
            cameraFov_ = fov;
        }
    }

    localPlayer_ = memory_.read<uintptr_t>(playersService + offsets_.PlayersToLocalPlayer);
    const uintptr_t localTeam = localPlayer_ ? memory_.read<uintptr_t>(localPlayer_ + offsets_.PlayerToTeam) : 0;

    // ---- Players ----
    std::vector<Player> fresh;
    for (const uintptr_t playerAddress : getChildren(playersService)) {
        if (getClassName(playerAddress) != "Player") {
            continue;
        }

        Player player;
        player.address = playerAddress;
        player.name = getName(playerAddress);
        player.displayName = readStdString(playerAddress + offsets_.PlayerToDisplayName);
        if (player.displayName.empty()) {
            player.displayName = player.name;
        }

        const uintptr_t team = memory_.read<uintptr_t>(playerAddress + offsets_.PlayerToTeam);
        player.isTeammate = (team != 0 && team == localTeam);
        if (team) {
            player.team = getName(team);
        }
        player.isLocal = (playerAddress == localPlayer_);

        const uintptr_t character = memory_.read<uintptr_t>(playerAddress + offsets_.PlayerToCharacter);
        if (!character) {
            fresh.push_back(player);
            continue;
        }

        const uintptr_t humanoid = findFirstChildOfClass(character, "Humanoid");
        const uintptr_t rootPart = findFirstChild(character, "HumanoidRootPart");
        const uintptr_t headPart = findFirstChild(character, "Head");
        if (!rootPart) {
            fresh.push_back(player);
            continue;
        }

        const uintptr_t rootPrimitive = memory_.read<uintptr_t>(rootPart + offsets_.BasePartToPrimitive);
        if (!rootPrimitive) {
            fresh.push_back(player);
            continue;
        }
        player.rootPosition = memory_.read<Vector3>(rootPrimitive + offsets_.PrimitiveToPosition);

        if (headPart) {
            const uintptr_t headPrimitive = memory_.read<uintptr_t>(headPart + offsets_.BasePartToPrimitive);
            if (headPrimitive) {
                player.headPosition = memory_.read<Vector3>(headPrimitive + offsets_.PrimitiveToPosition);
            }
        }
        if (player.headPosition.y == 0.f) {
            player.headPosition = player.rootPosition + Vector3{ 0.f, 1.5f, 0.f };
        }

        if (humanoid) {
            player.health = memory_.read<float>(humanoid + offsets_.HumanoidToHealth);
            player.maxHealth = memory_.read<float>(humanoid + offsets_.HumanoidToMaxHealth);
            if (player.maxHealth <= 0.f) {
                player.maxHealth = 100.f;
            }
        } else {
            player.health = 100.f;
            player.maxHealth = 100.f;
        }

        player.distance = cameraPosition_.distance(player.rootPosition);
        player.valid = true;
        fresh.push_back(player);
    }

    players_ = std::move(fresh);
    return true;
}

bool Game::worldToScreen(const Vector3& world, Vector2& out) const {
    const Vector3 diff = world - cameraPosition_;
    const float x = diff.dot(cameraRight_);
    const float y = diff.dot(cameraUp_);
    const float z = diff.dot(cameraLook_);
    if (z <= 0.01f) {
        return false;
    }

    const float tanHalfFov = std::tan(cameraFov_ * 0.5f);
    if (tanHalfFov <= 0.0001f) {
        return false;
    }

    const float focal = (static_cast<float>(screenHeight_) * 0.5f) / tanHalfFov;
    out.x = static_cast<float>(screenWidth_) * 0.5f + (x * focal) / z;
    out.y = static_cast<float>(screenHeight_) * 0.5f - (y * focal) / z;
    return true;
}

} // namespace adrenaline
