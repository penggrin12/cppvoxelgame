//
// Created by penggrin on 29.03.2026.
//

#include "Entity.hpp"

void Entity::setPos(const vec3 &newPos) {
    constexpr float bbWidth = 0.6f;
    constexpr float bbHeight = 1.8f;
    constexpr float heightOffset = 0.0f / 16.0f; // ???

    pos = newPos;
    constexpr float w = bbWidth / 2;
    constexpr float h = bbHeight;
    bb.set(pos.x - w, pos.y - heightOffset, pos.z - w, pos.x + w, pos.y - heightOffset + h, pos.z + w);
}

void Entity::move(vec3 delta) {
    std::vector<AABB> aabbs = level->getCubes(bb.expand(delta));

    // y
    auto hasTouchedFloor = false;
    for (AABB& box : aabbs) {
        const auto yClipped = box.clipYCollide(bb, delta.y);
        if (delta.y < 0 && abs(yClipped) < 0.0001)
            hasTouchedFloor = true;
        delta.y = yClipped;
    }
    onGround = hasTouchedFloor;
    bb.move(0, delta.y, 0);

    // x
    for (AABB& box : aabbs)
        delta.x = box.clipXCollide(bb, delta.x);
    bb.move(delta.x, 0, 0);

    // z
    for (AABB& box : aabbs)
        delta.z = box.clipZCollide(bb, delta.z);
    bb.move(0, 0, delta.z);

    pos.x = (bb.x0 + bb.x1) * 0.5f;
    pos.y = bb.y0;
    pos.z = (bb.z0 + bb.z1) * 0.5f;
}
