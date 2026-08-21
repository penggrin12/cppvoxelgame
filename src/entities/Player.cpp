//
// Created by penggrin on 15.03.2026.
//

#include <format>

#include "Player.hpp"

#include <ranges>

#include "../Game.hpp"
#include "../audio/audio.hpp"
#include "../phys/raycast.hpp"
#include "tracy/Tracy.hpp"

void Player::iWantToSeeNearbyChunks() const { ZoneScoped;
    auto lock = level->main_thread_lock();

    const Location myLoc = Location::fromRealGlobalPos(getPos());
    const auto paddedRenderDist = config.renderDist + 1;

    for (int cx = myLoc.chunkPos.x - paddedRenderDist; cx <= myLoc.chunkPos.x + paddedRenderDist; ++cx) {
        for (int cy = myLoc.chunkPos.y - paddedRenderDist; cy <= myLoc.chunkPos.y + paddedRenderDist; ++cy) {
            const auto chunkPos = ivec2{cx, cy};
            if (level->hasChunk(chunkPos))
                continue;

            level->createChunk(chunkPos);
            level->genChunk(chunkPos);
        }
    }

    std::vector<ivec2> keysCopy;
    for (const auto &k : level->getChunks() | std::views::keys)
        keysCopy.push_back(k);

    for (const auto &chunkPos: keysCopy) {
        const auto dist = distChebyshev(myLoc.chunkPos, chunkPos);
        if (dist <= config.renderDist) {
            if (level->getChunk(chunkPos)->mesh == nullptr)
                level->markChunkDirty(chunkPos);
            continue;
        }

        // keep a strip of unmeshed chunks to correctly mesh closer ones
        if (dist == config.renderDist + 1) {
            level->getChunk(chunkPos)->mesh = nullptr;
            continue;
        }

        SPDLOG_INFO("removing {},{}. dist is {}", chunkPos.x, chunkPos.y, dist);
        level->removeChunk(chunkPos);
    }
}

glm::vec3 Player::getDir() const {
    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front.y = std::sin(glm::radians(pitch));
    front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    return glm::normalize(front);
}

void Player::init() {
    setSize(0.6f, 1.8f);
    yaw = -90.0f;
    pitch = 0.0f;

    vec3 eyePos = getEyePos();
    camera = raylib::Camera3D(
        glm2rl(eyePos),
        glm2rl(eyePos + getDir()),
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
    game->DrawTextB(std::format("{:.2f} {:.2f}", vec.x, vec.y), 8, 170, 12, raylib::Color::White());
    const auto aabb = getAabb();

    auto dir = getDir();
    auto eye = getEyePos();
    game->DrawTextB(std::format("onGround: {}\n{}\naabb area: {:.2f} {:.2f} {:.2f}\n\neye pos: {:.2f} {:.2f} {:.2f}\ndir: {:.2f} {:.2f} {:.2f}",
        onGround, aabb->toString(), aabb->area().x, aabb->area().y, aabb->area().z,
        eye.x, eye.y, eye.z, dir.x, dir.y, dir.z),
        8, 195, 12, raylib::Color::White());
}

void Player::tryPlace(const RaycastHit &ray) const {
    const auto targetVoxelPos = ray.pos + ray.normal;
    if (!Level::isVoxelInBounds(targetVoxelPos))
        return;
    if (level->getVoxel(targetVoxelPos) > Voxel::AIR)
        return;

    // cant place voxels inside of yourself
    if (Level::getVoxelAABB(targetVoxelPos).intersects(bb))
        return;

    level->setVoxel(targetVoxelPos, Voxel::STONE);
    level->markVoxelDirtyAndNeighbours(targetVoxelPos);
    game->getAudio().playSound("dirt.break", vec3(ray.pos) + vec3{0.5f, 0.5, 0.5f});
}

[[nodiscard]] bool Player::isChunkInFrustum(const ivec2 &chunkPos) const {
    const auto chunkAABB = Chunk::getAabb(chunkPos);
    return frustum.AABBoxIn(glm2rl(chunkAABB.a), glm2rl(chunkAABB.b));
}

void Player::frustumCulling() { ZoneScoped;
    frustum.Extract();

    for (auto &chunkPair: level->getChunks()) {
        chunkPair.second->hidden = !isChunkInFrustum(chunkPair.first);
    }
}

void Player::logic() { ZoneScoped;
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

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_RIGHT))
        tryPlace(ray);
}

void Player::movement() { ZoneScoped;
    const auto look = GetMouseDelta();
    const float sensitivity = 0.15f;

    yaw += look.x * sensitivity;
    pitch -= look.y * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front = getDir();

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 flatFront = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), right));

    const auto move = GameInput::getVec(KEY_A, KEY_D, KEY_W, KEY_S);
    const float speed = GameInput::keyDown(KEY_LEFT_SHIFT) ? 9.0f : 4.5f;

    glm::vec3 targetVel = (flatFront * -move.y) + (right * move.x);
    if (glm::length(targetVel) > 0.0f) {
        targetVel = glm::normalize(targetVel) * speed;
    }

    vel.x = targetVel.x;
    vel.z = targetVel.z;

    vel.y -= 25.0f * GetFrameTime();
    if (onGround && raylib::Keyboard::IsKeyDown(KEY_SPACE)) {
        vel.y = 8.5f;
    }

    vec3 delta = vel * GetFrameTime();
    this->move(delta);

    const vec3 eyePos = getEyePos();
    camera.position = glm2rl(eyePos);
    camera.target = glm2rl(eyePos + front);
}
