//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_PLAYER_HPP
#define VOXELGAME_PLAYER_HPP
#include "Entity.hpp"
#include "phys/raycast.hpp"
#include "RLFrustum.hpp"


class Player final : public Entity {
private:
    raylib::Camera3D camera;
    RLFrustum frustum;

    float yaw = -90.0f;
    float pitch = 0.0f;

    void iWantToSeeNearbyChunks() const;
    void tryPlace(const RaycastHit &ray) const;


public:
    Player() = delete;
    explicit Player(Level *level) : Entity(level) {}

    void init() override;

    void logic() override;
    void draw() override;
    void draw2d() override;

    void movement();

    [[nodiscard]] bool isChunkInFrustum(const ivec2 &chunkPos) const;
    void frustumCulling();

    [[nodiscard]] raylib::Camera3D& getCamera() { return camera; }

    [[nodiscard]] glm::vec3 getEyePos() const { return getPos() + vec3{0, 1.62f, 0}; }
    [[nodiscard]] glm::vec3 getDir() const;
};


#endif //VOXELGAME_PLAYER_HPP