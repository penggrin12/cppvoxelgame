//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_PLAYER_HPP
#define VOXELGAME_PLAYER_HPP
#include "Entity.hpp"


class Player final : public Entity {
private:
    raylib::Camera3D camera;
public:
    Player() = delete;

    explicit Player(Level *level) : Entity(level) {}

    void init() override;

    void logic() override;
    void draw() override;
    void draw2d() override;

    void movement();

    [[nodiscard]] raylib::Camera3D& getCamera();
    [[nodiscard]] glm::vec3 getEyePos() const { return getPos() + vec3{0, 1.8, 0}; }

    glm::vec3 getDir() { return glm::normalize(rl2glm(getCamera().target) - getEyePos()); }
};


#endif //VOXELGAME_PLAYER_HPP