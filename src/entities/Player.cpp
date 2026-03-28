//
// Created by penggrin on 15.03.2026.
//

#include <format>

#include "Player.hpp"

#include <ranges>

#include "../Game.hpp"
#include "../audio/audio.hpp"
#include "../phys/raycast.hpp"

constexpr int renderDist = 4;

void Player::iWantToSeeNearbyChunks() const {
    std::lock_guard lock(level->mutex);

    const Location myLoc = Location::fromGlobalPos(getPos());

    for (int cx = myLoc.chunkPos.x - renderDist; cx < myLoc.chunkPos.x + renderDist; ++cx) {
        for (int cy = myLoc.chunkPos.y - renderDist; cy < myLoc.chunkPos.y + renderDist; ++cy) {
            const auto chunkPos = ivec2{cx, cy};
            if (!level->hasChunk(chunkPos)) {
                level->createChunk(chunkPos);
                level->genChunk(chunkPos);
            }
        }
    }

    std::vector<ivec2> keysCopy;
    for (const auto& k : level->getChunks() | std::views::keys)
        keysCopy.push_back(k);

    for (const auto& chunkPos: keysCopy) {
        const auto dist = distChebyshev(myLoc.chunkPos, chunkPos);
        if ((dist < renderDist - 1) && (level->getChunk(chunkPos)->mesh == nullptr)) {
            level->markChunkDirty(chunkPos);
        }
        if (dist <= renderDist + 1)
            continue;
        level->removeChunk(chunkPos);
    }
}

void Player::init() {
    camera = raylib::Camera3D(
        glm2rl(getPos() + vec3(0, 2, 0)),
        glm2rl(getPos() + vec3(0, 2, -1)),
        raylib::Vector3{0, 1, 0},
        90.0f,
        CAMERA_PERSPECTIVE
    );
}

void Player::draw() {
    debugDrawAABB(*this->getAabb());
}

void Player::draw2d() {
    const auto vec = GameInput::getVec(KEY_A, KEY_D, KEY_W, KEY_S);
    game->DrawTextB(std::format("{} {}", vec.x, vec.y), 8, 170, 12, raylib::Color::White());
    const auto aabb = getAabb();
    game->DrawTextB(std::format("onGround: {}\n{}\naabb area: {} {} {}\n\neye pos: {} {} {}\ndir: {} {} {}", onGround, aabb->toString(), aabb->area().x, aabb->area().y, aabb->area().z, getEyePos().x, getEyePos().y, getEyePos().z, getDir().x, getDir().y, getDir().z), 8, 195, 12, raylib::Color::White());
}

void Player::logic() {
    // setPos(rl2glm(camera.position));

    // camera.position = glm2rl(getPos());

    if (game->debugStats.frame % 30 == 0)
        iWantToSeeNearbyChunks();

    game->getAudio().updateListener(getEyePos(), getDir());
    movement();

    if (!raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT) && !raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_RIGHT))
        return;

    const auto ray = raycast(level, getEyePos(), getDir(), 4.5f);
    if (!ray.hit)
        return;

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT)) {
        level->setVoxel(ray.pos, Voxel::AIR);
        level->markVoxelDirtyAndNeighbours(ray.pos);
        game->getAudio().playSound("dirt.break", vec3(ray.pos) + vec3{0.5f, 0.5, 0.5f});
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_RIGHT)) {
        const auto targetVoxelPos = ray.pos + ray.normal;
        if (!level->isVoxelInBounds(targetVoxelPos))
            return;
        if (level->getVoxel(targetVoxelPos) > Voxel::AIR)
            return;

        level->setVoxel(targetVoxelPos, Voxel::STONE);
        level->markVoxelDirtyAndNeighbours(targetVoxelPos);
        game->getAudio().playSound("dirt.break", vec3(ray.pos) + vec3{0.5f, 0.5, 0.5f});
    }
}

void Player::movement() {
    const auto move = GameInput::getVec(KEY_A, KEY_D, KEY_W, KEY_S);
    const auto look = GetMouseDelta();

    const raylib::Vector3 oldCamPos = camera.position;

    camera.Update(
        glm2rl(vec3{-move.y, move.x, 0.0f} * 5.0f * GetFrameTime()),
        glm2rl(vec3{look.x, look.y, 0.0f}),
        0.0f
    );

    vec3 delta = rl2glm(camera.position) - rl2glm(oldCamPos);
    delta.y += -5.0f * GetFrameTime(); // gravity

    if (raylib::Keyboard::IsKeyPressed(KEY_SPACE) && onGround)
        delta.y = 1.1f;

    this->move(delta);

    const vec3 eyePos = getEyePos();
    const vec3 correction = eyePos - rl2glm(camera.position);

    camera.position = glm2rl(eyePos);

    camera.target.x += correction.x;
    camera.target.y += correction.y;
    camera.target.z += correction.z;
}

raylib::Camera3D& Player::getCamera() {
    return this->camera;
}


