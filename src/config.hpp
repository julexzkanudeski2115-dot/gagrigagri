#pragma once
#include <algorithm>
#include <fstream>
#include <string>

namespace adrenaline {

struct EspSettings {
    bool enabled = true;
    bool boxes = true;
    bool names = true;
    bool health = true;
    bool distance = true;
    bool headDot = false;
    bool snaplines = false;
    bool teamCheck = true;
    float maxDistance = 2000.f;
    float boxColor[4] = { 0.20f, 0.90f, 1.00f, 1.00f };
    float teamColor[4] = { 0.30f, 1.00f, 0.40f, 1.00f };
};

struct AimbotSettings {
    bool enabled = false;
    bool teamCheck = true;
    bool visibleCheck = false;
    bool holdKey = true;
    int aimKey = 0x02; // VK_RBUTTON
    float fov = 120.f;
    float smooth = 4.f;
    float sensitivity = 1.0f;
    bool targetHead = true;
    float maxDistance = 1000.f;
};

struct MenuSettings {
    int toggleKey = 0x2D; // VK_INSERT
};

struct Config {
    EspSettings esp;
    AimbotSettings aimbot;
    MenuSettings menu;
    bool streamProof = false;

    void save(const std::string& path) const {
        std::ofstream file(path);
        if (!file) {
            return;
        }
        file << "[esp]\n"
             << "enabled=" << esp.enabled << "\n"
             << "boxes=" << esp.boxes << "\n"
             << "names=" << esp.names << "\n"
             << "health=" << esp.health << "\n"
             << "distance=" << esp.distance << "\n"
             << "headDot=" << esp.headDot << "\n"
             << "snaplines=" << esp.snaplines << "\n"
             << "teamCheck=" << esp.teamCheck << "\n"
             << "maxDistance=" << esp.maxDistance << "\n"
             << "boxColor=" << esp.boxColor[0] << "," << esp.boxColor[1] << "," << esp.boxColor[2] << "," << esp.boxColor[3] << "\n"
             << "teamColor=" << esp.teamColor[0] << "," << esp.teamColor[1] << "," << esp.teamColor[2] << "," << esp.teamColor[3] << "\n"
             << "[aimbot]\n"
             << "aimbotEnabled=" << aimbot.enabled << "\n"
             << "aimbotTeamCheck=" << aimbot.teamCheck << "\n"
             << "aimbotVisibleCheck=" << aimbot.visibleCheck << "\n"
             << "aimbotHoldKey=" << aimbot.holdKey << "\n"
             << "aimKey=" << aimbot.aimKey << "\n"
             << "aimFov=" << aimbot.fov << "\n"
             << "aimSmooth=" << aimbot.smooth << "\n"
             << "aimSensitivity=" << aimbot.sensitivity << "\n"
             << "aimTargetHead=" << aimbot.targetHead << "\n"
             << "aimMaxDistance=" << aimbot.maxDistance << "\n"
             << "[menu]\n"
             << "toggleKey=" << menu.toggleKey << "\n"
             << "streamProof=" << streamProof << "\n";
    }

    void load(const std::string& path) {
        std::ifstream file(path);
        if (!file) {
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            const size_t separator = line.find('=');
            if (separator == std::string::npos) {
                continue;
            }
            const std::string key = line.substr(0, separator);
            const std::string value = line.substr(separator + 1);

            auto asBool = [](const std::string& v) { return v == "1" || v == "true"; };
            auto asInt = [](const std::string& v) { return std::stoi(v); };
            auto asFloat = [](const std::string& v) { return std::stof(v); };
            auto asColor = [](const std::string& v, float out[4]) {
                std::string token = v;
                for (int i = 0; i < 4; ++i) {
                    const size_t comma = token.find(',');
                    out[i] = std::stof(token.substr(0, comma));
                    if (comma == std::string::npos) {
                        break;
                    }
                    token = token.substr(comma + 1);
                }
            };

            try {
                if (key == "enabled") { esp.enabled = asBool(value); }
                else if (key == "boxes") { esp.boxes = asBool(value); }
                else if (key == "names") { esp.names = asBool(value); }
                else if (key == "health") { esp.health = asBool(value); }
                else if (key == "distance") { esp.distance = asBool(value); }
                else if (key == "headDot") { esp.headDot = asBool(value); }
                else if (key == "snaplines") { esp.snaplines = asBool(value); }
                else if (key == "teamCheck") { esp.teamCheck = asBool(value); }
                else if (key == "maxDistance") { esp.maxDistance = asFloat(value); }
                else if (key == "boxColor") { asColor(value, esp.boxColor); }
                else if (key == "teamColor") { asColor(value, esp.teamColor); }
                else if (key == "aimbotEnabled") { aimbot.enabled = asBool(value); }
                else if (key == "aimbotTeamCheck") { aimbot.teamCheck = asBool(value); }
                else if (key == "aimbotVisibleCheck") { aimbot.visibleCheck = asBool(value); }
                else if (key == "aimbotHoldKey") { aimbot.holdKey = asBool(value); }
                else if (key == "aimKey") { aimbot.aimKey = asInt(value); }
                else if (key == "aimFov") { aimbot.fov = asFloat(value); }
                else if (key == "aimSmooth") { aimbot.smooth = asFloat(value); }
                else if (key == "aimSensitivity") { aimbot.sensitivity = asFloat(value); }
                else if (key == "aimTargetHead") { aimbot.targetHead = asBool(value); }
                else if (key == "aimMaxDistance") { aimbot.maxDistance = asFloat(value); }
                else if (key == "toggleKey") { menu.toggleKey = asInt(value); }
                else if (key == "streamProof") { streamProof = asBool(value); }
            } catch (...) {
                // ignore malformed values, keep defaults
            }
        }
    }
};

} // namespace adrenaline
