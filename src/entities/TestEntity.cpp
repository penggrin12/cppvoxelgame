//
// Created by penggrin on 21.03.2026.
//

#include "TestEntity.hpp"

#include "../phys/raycast.hpp"
#include "../utils/random.hpp"

void TestEntity::init() {
    setSize(0.6f, 1.8f);
}

void TestEntity::logic() {
    vel.y -= 25.0f * GetFrameTime();

    if (onGround) {
        vel.x = rng::range(-1.0f, 1.0f) * 4.0f;
        vel.z = rng::range(-1.0f, 1.0f) * 4.0f;
        vel.y = 8.0f;
    }

    move(vel * GetFrameTime());
}

void TestEntity::draw() {
    debugDrawAABB(*getAabb());
}
