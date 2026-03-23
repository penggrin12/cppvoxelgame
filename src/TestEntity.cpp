//
// Created by penggrin on 21.03.2026.
//

#include "TestEntity.hpp"

#include "phys/raycast.hpp"
#include "utils/random.hpp"

void TestEntity::logic() {
    move(normalize(vec3{rng::range(-1.0f, 1.0f), -1.0f, rng::range(-1.0f, 1.0f)}) * 15.0f * GetFrameTime());
}

void TestEntity::draw() {
    debugDrawAABB(*getAabb());
}
