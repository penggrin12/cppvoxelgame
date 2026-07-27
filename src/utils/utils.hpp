//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_UTILS_HPP
#define VOXELGAME_UTILS_HPP

// needed to include glm/gtx/hash.hpp
#define GLM_ENABLE_EXPERIMENTAL

#include <raylib-cpp.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#define ASSERT_AND_RETURN(expression, returnValue) { assert(expression); if (!(expression)) return returnValue; }
#define ASSERT_AND_RETURN_VOID(expression) ASSERT_AND_RETURN(expression,)

using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;

using vec2 = glm::vec2;
using vec3 = glm::vec3;

enum class Axis : int {
    X = 0,
    Y,
    Z
};

// clangd REALLY wants these here
template<typename T, glm::qualifier Q>
struct std::hash<glm::vec<1, T, Q> >;
template<typename T, glm::qualifier Q>
struct std::hash<glm::vec<2, T, Q> >;
template<typename T, glm::qualifier Q>
struct std::hash<glm::vec<3, T, Q> >;
template<typename T, glm::qualifier Q>
struct std::hash<glm::vec<4, T, Q> >;

template <Axis axis>
[[nodiscard]] constexpr bool clipAxis(const glm::vec3 &start, const glm::vec3 &end, const float target, glm::vec3 &result) {
    constexpr int i = static_cast<int>(axis);
    const float deltaAxis = end[i] - start[i];

    if (glm::abs(deltaAxis) < 0.000001f)
        return false;

    const float d = (target - start[i]) / deltaAxis;

    if (d < 0.0f || d > 1.0f)
        return false;

    result = glm::mix(start, end, d);
    return true;
}

[[nodiscard]] constexpr float distSqr(const glm::vec3 &a, const glm::vec3 &b) {
    const vec3 diff = a - b;
    return glm::dot(diff, diff);
}

template <typename T>
constexpr T normalize(const T &vec) {
    if (glm::length(vec) <= 0.00001)
        return glm::zero<T>();
    return glm::normalize(vec);
}

// cant be constexpr on msvc until <https://github.com/RobLoach/raylib-cpp/issues/383> resolves
#if defined(_MSC_VER) && !defined(__clang__)
#  define CONSTEXPR_IF_SUPPORTED
#else
#  define CONSTEXPR_IF_SUPPORTED constexpr
#endif

inline CONSTEXPR_IF_SUPPORTED raylib::Vector3 glm2rl(const glm::vec3 &v) { return {v.x, v.y, v.z}; }
inline CONSTEXPR_IF_SUPPORTED glm::vec3 rl2glm(const raylib::Vector3& v) { return {v.x, v.y, v.z}; }

#undef CONSTEXPR_IF_SUPPORTED

#endif //VOXELGAME_UTILS_HPP