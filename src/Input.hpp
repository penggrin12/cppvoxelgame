//
// Created by penggrin on 22.03.2026.
//

#ifndef VOXELGAME_INPUT_HPP
#define VOXELGAME_INPUT_HPP
#include <raylib-cpp.hpp>
#include <glm/vec2.hpp>

#include "utils/utils.hpp"

class GameInput {
public:
    static bool keyDown(const KeyboardKey key) { return raylib::Keyboard::IsKeyDown(key); }
    static float getAxis(const KeyboardKey a, const KeyboardKey b) { return keyDown(b) - keyDown(a); }
    static glm::vec2 getVec(const KeyboardKey x1, const KeyboardKey y1, const KeyboardKey x2, const KeyboardKey y2) { return ::normalize(glm::vec2(getAxis(x1, y1), getAxis(x2, y2))); }
};

#endif //VOXELGAME_INPUT_HPP