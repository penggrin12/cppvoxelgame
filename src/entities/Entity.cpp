//
// Created by penggrin on 29.03.2026.
//

#include "Entity.hpp"

#include "tracy/Tracy.hpp"

void Entity::setSize(float width, float height) {
    bbWidth = width;
    bbHeight = height;
    setPos(pos);
}

void Entity::setPos(const vec3 &newPos) {
    pos = newPos;
    const float w = bbWidth / 2.0f;
    const float h = bbHeight;
    bb.set(pos.x - w, pos.y, pos.z - w, pos.x + w, pos.y + h, pos.z + w);
}

void Entity::move(vec3 delta) { ZoneScoped;
    std::vector<AABB> aabbs = level->getCubes(bb.expand(delta));

    const vec3 originalDelta = delta;

    for (AABB& box : aabbs) {
        delta.y = box.clipYCollide(bb, delta.y);
    }
    bb.move(0, delta.y, 0);

    for (AABB& box : aabbs) {
        delta.x = box.clipXCollide(bb, delta.x);
    }
    bb.move(delta.x, 0, 0);

    for (AABB& box : aabbs) {
        delta.z = box.clipZCollide(bb, delta.z);
    }
    bb.move(0, 0, delta.z);

    onGround = (originalDelta.y < 0.0f && delta.y > originalDelta.y);

    if (delta.x != originalDelta.x) vel.x = 0.0f;
    if (delta.y != originalDelta.y) vel.y = 0.0f;
    if (delta.z != originalDelta.z) vel.z = 0.0f;

    pos.x = (bb.x0 + bb.x1) * 0.5f;
    pos.y = bb.y0;
    pos.z = (bb.z0 + bb.z1) * 0.5f;
}
