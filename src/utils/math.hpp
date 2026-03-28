//
// Created by penggrin on 22.03.2026.
//

#ifndef VOXELGAME_MATH_HPP
#define VOXELGAME_MATH_HPP
#include <cmath>

[[nodiscard]] constexpr int floorDiv(const int a, const int b) {
    int q = a / b;
    int r = a % b;
    if (r != 0 && r < 0 != b < 0)
        --q;
    return q;
}

template <typename T>
[[nodiscard]] constexpr int floori(const T x) {
    return static_cast<int>(std::floor(x));
}

[[nodiscard]] constexpr int distChebyshev(const ivec2& a, const ivec2& b) {
    return glm::max(glm::abs(a.x - b.x), glm::abs(a.y - b.y));
}

#endif //VOXELGAME_MATH_HPP