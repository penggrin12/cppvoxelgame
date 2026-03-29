//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_ENTITY_HPP
#define VOXELGAME_ENTITY_HPP

#include "../Level.hpp"
#include "../phys/AABB.hpp"
#include "../utils/utils.hpp"


class Entity {
protected:
    vec3 pos = {0, 0, 0};
    AABB bb = {0, 0, 0, 0, 0, 0};
    Level* level;
public:
    bool onGround = false;

    Entity() = delete;
    explicit Entity(Level* level) : level(level) {}
    virtual ~Entity() = default;

    [[nodiscard]] AABB* getAabb() { return &bb; }

    [[nodiscard]] vec3 getPos() const { return pos; }
    void setPos(const vec3& newPos);

    void translate(const vec3& by) { setPos(pos + by); }

    virtual void init() {}

    virtual void logic() {}
    virtual void draw() {}
    virtual void draw2d() {}

    void move(vec3 delta);
};


#endif //VOXELGAME_ENTITY_HPP