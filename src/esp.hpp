#pragma once
#include "config.hpp"
#include "game.hpp"

struct ImDrawList;

namespace adrenaline {

void drawEsp(ImDrawList* drawList, const Game& game, const Config& config);

} // namespace adrenaline
