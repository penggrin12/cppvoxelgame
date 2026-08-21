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

    for (AABB& box : aabbs)
        delta.y = box.clipCollide<Axis::Y>(bb, delta.y);
    bb.move<Axis::Y>(delta.y);

    for (AABB& box : aabbs)
        delta.x = box.clipCollide<Axis::X>(bb, delta.x);
    bb.move<Axis::X>(delta.x);

    for (AABB& box : aabbs)
        delta.z = box.clipCollide<Axis::Z>(bb, delta.z);
    bb.move<Axis::Z>(delta.z);

    onGround = (originalDelta.y < 0.0f && delta.y > originalDelta.y);

    if (delta.x != originalDelta.x) vel.x = 0.0f;
    if (delta.y != originalDelta.y) vel.y = 0.0f;
    if (delta.z != originalDelta.z) vel.z = 0.0f;

    pos.x = (bb.a.x + bb.b.x) * 0.5f;
    pos.y = bb.a.y;
    pos.z = (bb.a.z + bb.b.z) * 0.5f;
}
