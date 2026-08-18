//
// Created by penggrin on 21.03.2026.
//

#ifndef VOXELGAME_AABB_HPP
#define VOXELGAME_AABB_HPP

#include <format>
#include <glm/glm.hpp>

#include "utils/utils.hpp"

class Entity;

struct HitResult
{
    bool hit = false;
    int face;
    glm::vec3 pos;
};

struct AABB
{
public:
    glm::vec3 a;
    glm::vec3 b;

    constexpr AABB()
        : a{glm::vec3{0}}, b{glm::vec3{0}} { }

    constexpr AABB(const float x0, const float y0, const float z0, const float x1, const float y1, const float z1)
        : a{glm::vec3{x0, y0, z0}}, b{glm::vec3{x1, y1, z1}} { }

    constexpr AABB(const glm::vec3 a, const glm::vec3 b)
        : AABB{a.x, a.y, a.z, b.x, b.y, b.z} { }

    AABB(const AABB& other) = default;
    AABB& operator=(const AABB& other) = delete;
    AABB(AABB&& other) noexcept = delete;
    AABB& operator=(AABB&& other) noexcept = delete;

    constexpr AABB* set(const float x0, const float y0, const float z0, const float x1, const float y1, const float z1) {
        a = glm::vec3{x0, y0, z0}; b = glm::vec3{x1, y1, z1};
        return this;
    }

    [[nodiscard]] AABB expand(glm::vec3 by) const;

    template <Axis A>
    [[nodiscard]] float clipCollide(const AABB &c, float delta) const {
        constexpr auto i = static_cast<glm::length_t>(A);

        constexpr auto B = static_cast<glm::length_t>(nextAxis(A));
        constexpr auto C = static_cast<glm::length_t>(prevAxis(A));
        if (c.b[B] <= a[B] || c.a[B] >= b[B])
            return delta;
        if (c.b[C] <= a[C] || c.a[C] >= b[C])
            return delta;

        if (delta > 0 && c.b[i] <= a[i]) {
            if (const float max = a[i] - c.b[i]; max < delta)
                delta = max;
        }
        if (delta < 0 && c.a[i] >= b[i]) {
            if (const float max = b[i] - c.a[i]; max > delta)
                delta = max;
        }

        return delta;
    }

    [[nodiscard]] bool intersects(const AABB &c) const;

    template <Axis axis>
    constexpr AABB* move(const float delta) {
        constexpr auto i = static_cast<glm::length_t>(axis);
        a[i] += delta; b[i] += delta;
        return this;
    }

    [[nodiscard]] HitResult clip(vec3 a_, vec3 b_) const;

    [[nodiscard]] constexpr glm::vec3 area() const {
        return {b.x - a.x, b.y - a.y, b.z - a.z};
    }
private:
    template <Axis A>
    [[nodiscard]] bool contains(const glm::vec3 v) const {
        constexpr auto B = static_cast<glm::length_t>(nextAxis(A));
        constexpr auto C = static_cast<glm::length_t>(prevAxis(A));

        return v[B] >= a[B] && v[B] <= b[B] &&
               v[C] >= a[C] && v[C] <= b[C];
    }

public:
    [[nodiscard]] std::string toString() const {
        return std::format("AABB({:.2f}, {:.2f}, {:.2f} to {:.2f}, {:.2f}, {:.2f})", a.x, a.y, a.z, b.x, b.y, b.z);
    }
};

#endif //VOXELGAME_AABB_HPP