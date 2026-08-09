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
    vec3 vel = {0, 0, 0};
    AABB bb = {0, 0, 0, 0, 0, 0};
    Level *level;

    float bbWidth = 0.6f;
    float bbHeight = 1.8f;

public:
    bool onGround = false;

    Entity() = delete;
    explicit Entity(Level* level) : level(level) {}
    virtual ~Entity() = default;

    [[nodiscard]] AABB *getAabb() { return &bb; }
    [[nodiscard]] vec3 getPos() const { return pos; }
    [[nodiscard]] vec3 getVel() const { return vel; }

    void setSize(float width, float height);
    void setPos(const vec3 &newPos);
    void setVel(const vec3 &newVel) { vel = newVel; }

    void translate(const vec3 &by) { setPos(pos + by); }

    virtual void init() {}
    virtual void logic() {}
    virtual void draw() {}
    virtual void draw2d() {}

    void move(vec3 delta);
};


#endif //VOXELGAME_ENTITY_HPP