//
// Created by penggrin on 15.03.2026.
//

#include "Game.hpp"

#include <format>
#include <memory>

#include "Mesher.hpp"
#include "entities/TestEntity.hpp"
#include "phys/raycast.hpp"
#include "tracy/Tracy.hpp"
#include "utils/noise.hpp"

raylib::Mesh mesh;

GameResources::GameResources(Game* game) {
    ZoneScoped;
    game->getAudio().cacheSounds("./res/sounds", "dirt.break");

    font = raylib::Font("res/PressStart2P.ttf");

    skyMaterial = raylib::Material();
    skyMaterial.SetShader(LoadShader("res/shaders/sky.vs.glsl", "res/shaders/sky.fs.glsl"));

    skyMesh = raylib::Mesh::Sphere(.5, 8, 16);

    terrainTexture = raylib::Texture("res/textures/terrain.png");
    terrainTexture.SetWrap(TEXTURE_WRAP_CLAMP);
    terrainTexture.SetFilter(TEXTURE_FILTER_POINT);
    const auto terrainShader = LoadShader("res/shaders/terrain.vs.glsl", "res/shaders/terrain.fs.glsl");

    terrainMaterial = raylib::Material();
    terrainMaterial.SetTexture(MATERIAL_MAP_ALBEDO, terrainTexture);
    terrainMaterial.SetShader(terrainShader);
}

void debugRay(RaycastHit& ray) {
    game->DrawTextB(std::format("{} {} {} {}", ray.hit, ray.normal.x, ray.normal.y, ray.normal.z), 64, 150, 12, raylib::Color::White());
}

Game::Game(raylib::Window& window) : window(window), res(this) {
    lua.open("res/scripts/game.lua");

    curLevel = std::make_unique<Level>();

    mesher.startThread(curLevel.get());

    std::unique_ptr<Entity> player = std::make_unique<Player>(curLevel.get());
    player->luaRef = lua.getRef("Player");
    player->setPos({8, 64, 8});
    addEntity(player);

    DisableCursor();

    lua.call("init");
}

Game::~Game() {
    mesher.stopThread();

    for (auto& [chunkPos, chunk]: curLevel->getChunks()) {
        getStorage().saveChunk(chunkPos, chunk.get());
    }
}

void Game::logic() {
    ZoneScoped;
    for (const auto &entity: entities) {
        ZoneScoped;
        entity->logic();
        lua.callFor(entity->luaRef, "logic", GetFrameTime());
    }

    if (raylib::Keyboard::IsKeyDown(KEY_E)) {
        std::unique_ptr<Entity> ent = std::make_unique<TestEntity>(curLevel.get());
        ent->setPos(getPlayer().getPos());
        addEntity(ent);
    }

    lua.call("logic", GetFrameTime());
}

void Game::draw() {
    ZoneScoped;
    auto camera = getPlayer().getCamera();
    camera.BeginMode();

    getPlayer().frustumCulling();

    const auto eyePos = getPlayer().getEyePos();

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    res.skyMesh.Draw(res.skyMaterial, raylib::Matrix::Translate(eyePos.x, eyePos.y, eyePos.z));
    rlEnableDepthMask();
    rlEnableBackfaceCulling();

    std::vector<std::pair<ivec2, std::unique_ptr<raylib::Mesh>>> toSwap;
    {
        auto lock = curLevel->main_thread_lock();
        while (!curLevel->chunksReadyToSwapMeshQueue.empty()) {
            toSwap.push_back(std::move(curLevel->chunksReadyToSwapMeshQueue.front()));
            curLevel->chunksReadyToSwapMeshQueue.pop();
        }
    }

    // We upload and apply to map chunks unblocked
    for (auto& [chunkPos, newMesh] : toSwap) {
        newMesh->Upload();
        if (curLevel->hasChunk(chunkPos)) {
            curLevel->getChunk(chunkPos)->mesh = std::move(newMesh);
        }
    }

    {
        ZoneScopedN("voxels");
        for (const auto& [chunksPos, chunk]: curLevel->getChunks()) {
            if (chunk->hidden)
                continue;

            if (chunk->mesh == nullptr) {
                // SPDLOG_WARN("oh no nullptr mesh");
                continue;
            }

            chunk->mesh->Draw(res.terrainMaterial, raylib::Matrix::Translate(chunksPos.x * CHUNK_SIZE, 0, chunksPos.y * CHUNK_SIZE));
        }
    }

    for (const auto& [chunksPos, chunk]: curLevel->getChunks()) {
        debugDrawAABB(Chunk::getAabb(chunksPos), chunk->mesh == nullptr ? RED : chunk->hidden ? YELLOW : GREEN);
    }

    for (const auto &entity: entities) {
        entity->draw();
        lua.callFor(entity->luaRef, "draw");
    }

    lua.call("draw");

    DrawLine3D({camera.position.x, camera.position.y - 1.0f, camera.position.z}, camera.target, raylib::Color::Red());

    auto ray = raycast(curLevel.get(), eyePos, getPlayer().getDir(), 4.0f);

    const auto playerPos = getPlayer().getPos();
    const auto cubes = curLevel->getCubes(AABB(playerPos.x - 0.5f, playerPos.y - 2, playerPos.z - 0.5f, playerPos.x + 0.5f, playerPos.y + 0, playerPos.z + 0.5f));
    for (const auto &cube: cubes) {
        debugDrawAABB(cube);
    }

    camera.EndMode();

    for (const auto &entity: entities)
        entity->draw2d();

    debugRay(ray);

    raylib::Window::DrawFPS(2, 2);
#ifndef NDEBUG
    drawDebug();
#endif
}

void Game::drawDebug() {
    ZoneScoped;
    int posY = 22;

    auto drawText = [&posY, this](const std::string &text, const raylib::Color color = raylib::Color::White())
    {
        const auto height = (std::count(text.begin(), text.end(), '\n') + 1) * 22;
        raylib::DrawTextEx(res.font, text, raylib::Vector2{2, static_cast<float>(posY)}, 12, 0, color);
        posY += height;
    };

    const auto playerPos = getPlayer().getPos();

    window.DrawFPS(2, 2);
    drawText(std::format("T : {}, Q: {}", debugStats.tris, debugStats.tris / 2));
    drawText(std::format("X : {:.2f}\nY: {:.2f}\nZ: {:.2f}", playerPos.x, playerPos.y, playerPos.z), raylib::Color::Green());
    drawText(std::format("C : {}, {}", floori(playerPos.x / CHUNK_SIZE), floori(playerPos.z / CHUNK_SIZE)), raylib::Color::SkyBlue());
    drawText(std::format("CU: {}", curLevel->dirtyChunksQueue.size()), raylib::Color::SkyBlue());
}
