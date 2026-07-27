//
// Created by penggrin on 15.03.2026.
//

#include "Game.hpp"

#include <format>
#include <memory>

#include "Mesher.hpp"
#include "entities/TestEntity.hpp"
#include "phys/raycast.hpp"
#include "utils/noise.hpp"

raylib::Mesh mesh;

GameResources::GameResources(Game* game) {
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
    curLevel = std::make_unique<Level>();

    mesher.startThread(curLevel.get());

    std::unique_ptr<Entity> player = std::make_unique<Player>(curLevel.get());
    player->setPos({8, 20, 8});
    addEntity(player);

    DisableCursor();
}

Game::~Game() {
    mesher.stopThread();

    for (auto& [chunkPos, chunk]: curLevel->getChunks()) {
        getStorage().saveChunk(chunkPos, chunk.get());
    }
}

void Game::logic() {
    for (const auto& entity: entities)
        entity->logic();

    if (raylib::Keyboard::IsKeyPressed(KEY_E)) {
        std::unique_ptr<Entity> ent = std::make_unique<TestEntity>(curLevel.get());
        ent->setPos(getPlayer().getPos());
        addEntity(ent);
    }

    while (!curLevel->chunksReadyToSwapMeshQueue.empty()) {
        auto& [chunk, newMesh] = curLevel->chunksReadyToSwapMeshQueue.front();
        newMesh->Upload();
        chunk->mesh = std::move(newMesh);
        curLevel->chunksReadyToSwapMeshQueue.pop();
    }
}

void Game::draw() {
    auto camera = getPlayer().getCamera();
    camera.BeginMode();

    const auto eyePos = getPlayer().getEyePos();

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    res.skyMesh.Draw(res.skyMaterial, raylib::Matrix::Identity().Translate(eyePos.x, eyePos.y, eyePos.z));
    rlEnableDepthMask();
    rlEnableBackfaceCulling();

    for (auto& [chunksPos, chunk]: curLevel->getChunks()) {
        if (chunk->mesh == nullptr) {
            // SPDLOG_WARN("oh no nullptr mesh");
            continue;
        }

        chunk->mesh->Draw(res.terrainMaterial, raylib::Matrix::Translate(chunksPos.x * CHUNK_SIZE, 0, chunksPos.y * CHUNK_SIZE));
    }

    for (const auto& entity: entities)
        entity->draw();

    DrawLine3D({camera.position.x, camera.position.y - 1.0f, camera.position.z}, camera.target, raylib::Color::Red());

    auto ray = raycast(curLevel.get(), eyePos, getPlayer().getDir(), 4.0f);

    const auto playerPos = getPlayer().getPos();
    const auto cubes = curLevel->getCubes(AABB(playerPos.x - 0.5f, playerPos.y - 2, playerPos.z - 0.5f, playerPos.x + 0.5f, playerPos.y + 0, playerPos.z + 0.5f));
    for (const auto& cube: cubes) {
        debugDrawAABB(cube);
    }

    camera.EndMode();

    for (const auto& entity: entities)
        entity->draw2d();

    debugRay(ray);

    drawDebug();
}

void Game::drawDebug() {
    int posY = 22;

    auto drawText = [&posY, this](const std::string& text, const raylib::Color color = raylib::Color::White())
    {
        const auto height = (std::count(text.begin(), text.end(), '\n') + 1) * 22;
        raylib::DrawTextEx(res.font, text, raylib::Vector2{2, static_cast<float>(posY)}, 12, 0, color);
        posY += height;
    };

    const auto playerPos = getPlayer().getPos();

    window.DrawFPS(2, 2);
    drawText(std::format("T : {}, Q: {}", debugStats.tris, debugStats.tris / 2));
    drawText(std::format("X : {:.2f}\nY: {:.2f}\nZ: {:.2f}", playerPos.x, playerPos.y, playerPos.z), raylib::Color::Green());
    drawText(std::format("C : {}, {}", floorDiv(playerPos.x, CHUNK_SIZE), floorDiv(playerPos.z, CHUNK_SIZE)), raylib::Color::SkyBlue());
    drawText(std::format("CU: {}", curLevel->dirtyChunksQueue.size()), raylib::Color::SkyBlue());
}
