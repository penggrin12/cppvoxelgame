//
// Created by penggrin on 21.03.2026.
//

#ifndef VOXELGAME_AABB_HPP
#define VOXELGAME_AABB_HPP
#include <format>

#include "../utils/utils.hpp"

class Entity;

struct HitResult
{
    bool hit = false;
    int face;
    vec3 pos;

    [[nodiscard]] constexpr bool isHit() const { return hit; }
};

class AABB
{
public:
    float x0, y0, z0;
    float x1, y1, z1;

    constexpr AABB() : x0(0), y0(0), z0(0), x1(1), y1(1), z1(1) {}

    constexpr AABB(const float x0, const float y0, const float z0, const float x1, const float y1, const float z1) {
        this->set(x0, y0, z0, x1, y1, z1);
    }

    constexpr AABB* set(const float x0, const float y0, const float z0, const float x1, const float y1, const float z1) {
        this->x0 = x0; this->y0 = y0; this->z0 = z0;
        this->x1 = x1; this->y1 = y1; this->z1 = z1;
        return this;
    }

    [[nodiscard]] constexpr AABB expand(const vec3 by) const {
        float _x0 = x0;
        float _y0 = y0;
        float _z0 = z0;
        float _x1 = x1;
        float _y1 = y1;
        float _z1 = z1;

        if (by.x < 0) _x0 += by.x;
        if (by.x > 0) _x1 += by.x;

        if (by.y < 0) _y0 += by.y;
        if (by.y > 0) _y1 += by.y;

        if (by.z < 0) _z0 += by.z;
        if (by.z > 0) _z1 += by.z;

        return {_x0, _y0, _z0, _x1, _y1, _z1};
    }

    [[nodiscard]] constexpr AABB grow(const float xa, const float ya, const float za) const {
        return {x0 - xa, y0 - ya, z0 - za, x1 + xa, y1 + ya, z1 + za};
    }

    [[nodiscard]] constexpr AABB cloneMove(const float xa, const float ya, const float za) const {
        return {x0 + xa, y0 + ya, z0 + za, x1 + xa, y1 + ya, z1 + za};
    }

    [[nodiscard]] constexpr float clipXCollide(const AABB &c, float xa) const {
        if (c.y1 <= y0 || c.y0 >= y1) return xa;
        if (c.z1 <= z0 || c.z0 >= z1) return xa;

        if (xa > 0 && c.x1 <= x0) {
            const float max = x0 - c.x1;
            if (max < xa) xa = max;
        }
        if (xa < 0 && c.x0 >= x1) {
            const float max = x1 - c.x0;
            if (max > xa) xa = max;
        }

        return xa;
    }

    [[nodiscard]] constexpr float clipYCollide(const AABB &c, float ya) const {
        if (c.x1 <= x0 || c.x0 >= x1) return ya;
        if (c.z1 <= z0 || c.z0 >= z1) return ya;

        if (ya > 0 && c.y1 <= y0) {
            const float max = y0 - c.y1;
            if (max < ya) ya = max;
        }
        if (ya < 0 && c.y0 >= y1) {
            const float max = y1 - c.y0;
            if (max > ya) ya = max;
        }

        return ya;
    }

    [[nodiscard]] constexpr float clipZCollide(const AABB &c, float za) const {
        if (c.x1 <= x0 || c.x0 >= x1) return za;
        if (c.y1 <= y0 || c.y0 >= y1) return za;

        if (za > 0 && c.z1 <= z0) {
            const float max = z0 - c.z1;
            if (max < za) za = max;
        }
        if (za < 0 && c.z0 >= z1) {
            const float max = z1 - c.z0;
            if (max > za) za = max;
        }

        return za;
    }

    [[nodiscard]] constexpr bool intersects(const AABB &c) const {
        if (c.x1 <= x0 || c.x0 >= x1) return false;
        if (c.y1 <= y0 || c.y0 >= y1) return false;
        if (c.z1 <= z0 || c.z0 >= z1) return false;
        return true;
    }

    [[nodiscard]] constexpr bool intersectsInner(const AABB &c) const {
        if (c.x1 < x0 || c.x0 > x1) return false;
        if (c.y1 < y0 || c.y0 > y1) return false;
        if (c.z1 < z0 || c.z0 > z1) return false;
        return true;
    }

    constexpr AABB* move(const float xa, const float ya, const float za) {
        x0 += xa;
        y0 += ya;
        z0 += za;
        x1 += xa;
        y1 += ya;
        z1 += za;
        return this;
    }

    [[nodiscard]] constexpr bool intersects(float x02, float y02, float z02, float x12, float y12, float z12) const {
        if (x12 <= x0 || x02 >= x1) return false;
        if (y12 <= y0 || y02 >= y1) return false;
        if (z12 <= z0 || z02 >= z1) return false;
        return true;
    }

    [[nodiscard]] constexpr bool contains(const vec3 &p) const {
        if (p.x <= x0 || p.x >= x1) return false;
        if (p.y <= y0 || p.y >= y1) return false;
        if (p.z <= z0 || p.z >= z1) return false;
        return true;
    }

    [[nodiscard]] constexpr float getSize() const {
        const float xs = x1 - x0;
        const float ys = y1 - y0;
        const float zs = z1 - z0;
        return (xs + ys + zs) / 3.0f;
    }

    [[nodiscard]] constexpr Vector3 getA() const {
        return {x0, y0, z0};
    }

    [[nodiscard]] constexpr Vector3 getB() const {
        return {x1, y1, z1};
    }

    [[nodiscard]] AABB shrink(float xa, float ya, float za) const {
        return {x0 + xa, y0 + ya, z0 + za, x1 - xa, y1 - ya, z1 - za};
    }

    [[nodiscard]] AABB copy() const {
        return {x0, y0, z0, x1, y1, z1};
    }

    [[nodiscard]] constexpr HitResult clip(const vec3 &a, const vec3 &b) const {
        vec3 xh0, xh1, yh0, yh1, zh0, zh1;

        bool bxh0 = clipAxis<Axis::X>(a, b, x0, xh0);
        bool bxh1 = clipAxis<Axis::X>(a, b, x1, xh1);

        bool byh0 = clipAxis<Axis::Y>(a, b, y0, yh0);
        bool byh1 = clipAxis<Axis::Y>(a, b, y1, yh1);

        bool bzh0 = clipAxis<Axis::Z>(a, b, z0, zh0);
        bool bzh1 = clipAxis<Axis::Z>(a, b, z1, zh1);

        if (!bxh0 || !containsX(&xh0)) bxh0 = false;
        if (!bxh1 || !containsX(&xh1)) bxh1 = false;
        if (!byh0 || !containsY(&yh0)) byh0 = false;
        if (!byh1 || !containsY(&yh1)) byh1 = false;
        if (!bzh0 || !containsZ(&zh0)) bzh0 = false;
        if (!bzh1 || !containsZ(&zh1)) bzh1 = false;

        vec3 *closest = nullptr;

        if (bxh0 && (closest == nullptr || distSqr(a, xh0) < distSqr(a, *closest))) closest = &xh0;
        if (bxh1 && (closest == nullptr || distSqr(a, xh1) < distSqr(a, *closest))) closest = &xh1;
        if (byh0 && (closest == nullptr || distSqr(a, yh0) < distSqr(a, *closest))) closest = &yh0;
        if (byh1 && (closest == nullptr || distSqr(a, yh1) < distSqr(a, *closest))) closest = &yh1;
        if (bzh0 && (closest == nullptr || distSqr(a, zh0) < distSqr(a, *closest))) closest = &zh0;
        if (bzh1 && (closest == nullptr || distSqr(a, zh1) < distSqr(a, *closest))) closest = &zh1;

        if (closest == nullptr)
            return {};

        int face = -1;

        if (closest == &xh0) face = 4;
        if (closest == &xh1) face = 5;
        if (closest == &yh0) face = 0;
        if (closest == &yh1) face = 1;
        if (closest == &zh0) face = 2;
        if (closest == &zh1) face = 3;

        return {true, face, *closest};
    }

    constexpr vec3 area() const {
        return {x1 - x0, y1 - y0, z1 - z0};
    }

private:
    [[nodiscard]] bool containsX(const vec3 *v) const {
        return v->y >= y0 && v->y <= y1 && v->z >= z0 && v->z <= z1;
    }

    [[nodiscard]] bool containsY(const vec3 *v) const {
        return v->x >= x0 && v->x <= x1 && v->z >= z0 && v->z <= z1;
    }

    [[nodiscard]] bool containsZ(const vec3 *v) const {
        return v->x >= x0 && v->x <= x1 && v->y >= y0 && v->y <= y1;
    }

public:
    void set(const AABB &other) {
        x0 = other.x0; y0 = other.y0; z0 = other.z0;
        x1 = other.x1; y1 = other.y1; z1 = other.z1;
    }

    [[nodiscard]] std::string toString() const {
        return std::format("AABB({:.2f}, {:.2f}, {:.2f} to {:.2f}, {:.2f}, {:.2f})", x0, y0, z0, x1, y1, z1);
    }
};

#endif //VOXELGAME_AABB_HPP