//
// Created by penggrin on 15.03.2026.
//

#include "Game.hpp"

#include <format>
#include <memory>

#include "mesher.hpp"
#include "TestEntity.hpp"
#include "phys/raycast.hpp"
#include "utils/noise.hpp"

raylib::Mesh mesh;

void genLevel(Level* level) {
    // const auto noise = raylib::Image(GenImagePerlinNoise(32, 32, 0, 0, 1.0f));
    // const SimplexNoise noise{};

    for (int x = -4; x < 4; ++x) {
        for (int y = -4; y < 4; ++y) {
            const auto pos = ivec2{x, y};
            if (level->hasChunk(pos))
                continue;
            level->createChunk(pos);
        }
    }

    for (int x = 0; x < 32; ++x) {
        for (int z = 0; z < 32; ++z) {
            // const auto height = static_cast<int>(std::floor(noise.GetColor(x, z).r / 255.0 * 16));
            const auto height = static_cast<int>((noise::at(x, z) / 2.0f + 0.5f) * 16);
            // printf("%f\n", );
            for (int y = 0; y < 32; ++y) {
                if (height < y)
                    continue;

                level->setVoxel(ivec3(x, y, z), height == y ? Voxel::GRASS : Voxel::DIRT);
            }
        }
    }
}

void debugRay(RaycastHit& ray) {
    raylib::DrawText(std::format("{} {} {} {}", ray.hit, ray.normal.x, ray.normal.y, ray.normal.z), 64, 150, 20, raylib::Color::White());
}

void Game::initResources() {
    getAudio().cacheSounds("./res/sounds", "dirt.break");

    terrainTexture = raylib::Texture("res/textures/terrain.png");
    const auto terrainShader = LoadShader("res/shaders/terrain.vs.glsl", "res/shaders/terrain.fs.glsl");

    terrainMaterial = raylib::Material();
    terrainMaterial.SetTexture(MATERIAL_MAP_ALBEDO, terrainTexture);
    terrainMaterial.SetShader(terrainShader);
}

Game::Game(raylib::Window& window) : window(window) {
    initResources();

    curLevel = std::make_unique<Level>();

    std::unique_ptr<Entity> player = std::make_unique<Player>(curLevel.get());
    player->setPos({8, 20, 8});
    spdlog::debug("player: {}", addEntity(player));

    DisableCursor();

    genLevel(curLevel.get());
}

Game::~Game() = default;

void Game::logic() {
    for (const auto& entity: entities)
        entity->logic();

    if (raylib::Keyboard::IsKeyPressed(KEY_E)) {
        std::unique_ptr<Entity> ent = std::make_unique<TestEntity>(curLevel.get());
        ent->setPos(getPlayer().getPos());
        addEntity(ent);
    }
}

void Game::draw() {
    auto camera = getPlayer().getCamera();
    camera.BeginMode();

    // printf("%f %f %f\n", camera.position.x, camera.position.y, camera.position.z);

    DrawSphere(Vector3(), 0.1f, raylib::Color::Blue());
    // DrawTriangle3D(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(1, 0, 1), raylib::Color::Red());

    // drawCube(vec3(0, 5, 0));

    rebuildDirtyChunks(curLevel.get());
    for (auto& [chunksPos, chunk]: curLevel->getChunks()) {
        if (chunk->mesh == nullptr) {
            spdlog::warn("oh no nullptr mesh");
            continue;
        }

        chunk->mesh->Draw(terrainMaterial, raylib::Matrix::Translate(chunksPos.x * CHUNK_SIZE, 0, chunksPos.y * CHUNK_SIZE));
    }

    for (const auto& entity: entities)
        entity->draw();

    DrawLine3D({camera.position.x, camera.position.y - 1.0f, camera.position.z}, camera.target, raylib::Color::Red());

    auto ray = raycast(curLevel.get(), getPlayer().getEyePos(), getPlayer().getDir(), 4.0f);

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

    auto drawText = [&posY](const std::string& text)
    {
        const auto height = (std::count(text.begin(), text.end(), '\n') + 1) * 22;
        raylib::DrawText(text, 2, posY, 20, raylib::Color::White());
        posY += height;
    };

    const auto playerPos = getPlayer().getPos();

    window.DrawFPS(2, 2);
    drawText(std::format("T: {}, Q: {}", debugStats.tris, debugStats.tris / 2));
    drawText(std::format("X: {:.2f}\nY: {:.2f}\nZ: {:.2f}", playerPos.x, playerPos.y, playerPos.z));
    drawText(std::format("C: {}, {}", floorDiv(playerPos.x, CHUNK_SIZE), floorDiv(playerPos.z, CHUNK_SIZE)));
}
