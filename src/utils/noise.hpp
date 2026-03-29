//
// Created by penggrin on 22.03.2026.
//

#ifndef VOXELGAME_NOISE_HPP
#define VOXELGAME_NOISE_HPP
#include <ctime>
#include <glm/vec2.hpp>
#include "fastnoiselite/FastNoiseLite.h"

namespace noise {
    inline FastNoiseLite fastNoise{static_cast<int>(std::time(nullptr))};

    inline float at(const float x, const float y) {
        return fastNoise.GetNoise(x, y);
    }
    inline float at(const glm::vec2& pos) {
        return noise::at(pos.x, pos.y);
    }
    inline float at(const int x, const int y) {
        return noise::at(static_cast<float>(x), static_cast<float>(y));
    }
    inline float at(const glm::ivec2& pos) {
        return noise::at(pos.x, pos.y);
    }
}


#endif //VOXELGAME_NOISE_HPP
