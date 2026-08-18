//
// Created by penggrin on 18.08.2026.
//

#include "AABB.hpp"

#include "utils/utils.hpp"

bool AABB::intersects(const AABB &c) const {
    if (c.b.x <= a.x || c.a.x >= b.x) return false;
    if (c.b.y <= a.y || c.a.y >= b.y) return false;
    if (c.b.z <= a.z || c.a.z >= b.z) return false;
    return true;
}

HitResult AABB::clip(const vec3 a_, const vec3 b_) const {
    glm::vec3 xh0, xh1, yh0, yh1, zh0, zh1;

    bool bxh0 = clipAxis<Axis::X>(a_, b_, a.x, xh0);
    bool bxh1 = clipAxis<Axis::X>(a_, b_, b.x, xh1);

    bool byh0 = clipAxis<Axis::Y>(a_, b_, a.y, yh0);
    bool byh1 = clipAxis<Axis::Y>(a_, b_, b.y, yh1);

    bool bzh0 = clipAxis<Axis::Z>(a_, b_, a.z, zh0);
    bool bzh1 = clipAxis<Axis::Z>(a_, b_, b.z, zh1);

    if (!bxh0 || !contains<Axis::X>(xh0)) bxh0 = false;
    if (!bxh1 || !contains<Axis::X>(xh1)) bxh1 = false;
    if (!byh0 || !contains<Axis::Y>(yh0)) byh0 = false;
    if (!byh1 || !contains<Axis::Y>(yh1)) byh1 = false;
    if (!bzh0 || !contains<Axis::Z>(zh0)) bzh0 = false;
    if (!bzh1 || !contains<Axis::Z>(zh1)) bzh1 = false;

    glm::vec3 *closest = nullptr;

    if (bxh0 && (closest == nullptr || distSqr(a_, xh0) < distSqr(a_, *closest))) closest = &xh0;
    if (bxh1 && (closest == nullptr || distSqr(a_, xh1) < distSqr(a_, *closest))) closest = &xh1;
    if (byh0 && (closest == nullptr || distSqr(a_, yh0) < distSqr(a_, *closest))) closest = &yh0;
    if (byh1 && (closest == nullptr || distSqr(a_, yh1) < distSqr(a_, *closest))) closest = &yh1;
    if (bzh0 && (closest == nullptr || distSqr(a_, zh0) < distSqr(a_, *closest))) closest = &zh0;
    if (bzh1 && (closest == nullptr || distSqr(a_, zh1) < distSqr(a_, *closest))) closest = &zh1;

    if (closest == nullptr)
        return {.hit = false};

    int face = -1;

    if (closest == &xh0) face = 4;
    if (closest == &xh1) face = 5;
    if (closest == &yh0) face = 0;
    if (closest == &yh1) face = 1;
    if (closest == &zh0) face = 2;
    if (closest == &zh1) face = 3;

    return {.hit = true, .face = face, .pos = *closest};
}

AABB AABB::expand(const glm::vec3 by) const {
    auto _a = a;
    auto _b = b;

    if (by.x < 0) _a.x += by.x;
    if (by.x > 0) _b.x += by.x;

    if (by.y < 0) _a.y += by.y;
    if (by.y > 0) _b.y += by.y;

    if (by.z < 0) _a.z += by.z;
    if (by.z > 0) _b.z += by.z;

    return {_a, _b};
}
