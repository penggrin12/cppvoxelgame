//
// Created by penggrin on 21.03.2026.
//

#ifndef VOXELGAME_TESTENTITY_HPP
#define VOXELGAME_TESTENTITY_HPP
#include "Entity.hpp"


class TestEntity final : public Entity {
public:
    TestEntity() = delete;
    explicit TestEntity(Level* level) : Entity(level) {}

    void logic() override;
    void draw() override;
};


#endif //VOXELGAME_TESTENTITY_HPP