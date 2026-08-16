//
// Created by penggrin on 22.03.2026.
//

#ifndef VOXELGAME_RANDOM_HPP
#define VOXELGAME_RANDOM_HPP

#include <cmath>
#include <concepts>
#include <random>

namespace rng {
    [[nodiscard]] inline int rand() {
        return std::rand();
    }

    [[nodiscard]] inline bool randBool() {
        constexpr auto midpoint = RAND_MAX / 2;
        return rand() > midpoint;
    }

    [[nodiscard]] inline float randFloat() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // inclusive to
    [[nodiscard]] inline int range(const int from, const int to) {
        return rand() % (to - from + 1) + from;
    }

    // inclusive to
    [[nodiscard]] inline int range(const int to) {
        return rng::range(0, to);
    }

    // inclusive to
    template <std::floating_point T, int precision = 1000>
    [[nodiscard]] T range(const T from, const T to) {
        return static_cast<T>(rng::range(std::lround(from * static_cast<float>(precision)), std::lround(to * static_cast<float>(precision)))) / static_cast<float>(precision);
    }

    // inclusive to
    template <std::floating_point T, int precision = 1000>
    [[nodiscard]] T range(const T to) {
        return rng::range<T, precision>(0.0f, to);
    }
}

#endif //VOXELGAME_RANDOM_HPP