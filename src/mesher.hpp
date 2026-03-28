//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHER_HPP
#define VOXELGAME_MESHER_HPP

#include <chrono>

#include <spdlog/spdlog.h>
#include "MeshTool.hpp"
#include "voxeldata.hpp"
#include "utils/utils.hpp"

constexpr int TEXTURE_ATLAS_ITEM_SIZE = 16; // X and Y
constexpr int TEXTURE_ATLAS_PIXEL_SIZE = 256; // X and Y

constexpr ivec3 CUBE_VERTICES[8] = {
    {0, 0, 0},
    {1, 0, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
};

struct Side {
    int v0, v1, v2, v3;
    ivec3 normal;
};

constexpr Side SIDE_FRONT  = {4, 5, 6, 7, { 0,  0,  1}};
constexpr Side SIDE_BACK   = {1, 0, 3, 2, { 0,  0, -1}};
constexpr Side SIDE_LEFT   = {0, 4, 7, 3, {-1,  0,  0}};
constexpr Side SIDE_RIGHT  = {5, 1, 2, 6, { 1,  0,  0}};
constexpr Side SIDE_TOP    = {7, 6, 2, 3, { 0,  1,  0}};
constexpr Side SIDE_BOTTOM = {0, 1, 5, 4, { 0, -1,  0}};

constexpr Side CUBE_SIDES[6] = {SIDE_TOP, SIDE_BOTTOM, SIDE_LEFT, SIDE_RIGHT, SIDE_BACK, SIDE_FRONT};

inline void addFace(MeshTool& meshTool, const Side &side, const ivec2 atlasOffset, const ivec3 offset) {
    const ivec3 a = CUBE_VERTICES[side.v0] + offset;
    const ivec3 b = CUBE_VERTICES[side.v1] + offset;
    const ivec3 c = CUBE_VERTICES[side.v2] + offset;
    const ivec3 d = CUBE_VERTICES[side.v3] + offset;

    static constexpr float uvSize = 1.0f / TEXTURE_ATLAS_ITEM_SIZE;

    const vec2 uvOffset = static_cast<vec2>(atlasOffset) * uvSize;
    constexpr float inset = 1.0f / TEXTURE_ATLAS_PIXEL_SIZE * 0.01f; // hacky way to prevent texture bleeding

    const vec2 uvA = {uvOffset.x + inset,            uvOffset.y + uvSize - inset};
    const vec2 uvB = {uvOffset.x + uvSize - inset,   uvOffset.y + uvSize - inset};
    const vec2 uvC = {uvOffset.x + uvSize - inset,   uvOffset.y + inset};
    const vec2 uvD = {uvOffset.x + inset,            uvOffset.y + inset};

    meshTool.addQuad(a, b, c, d, uvA, uvB, uvC, uvD, vec3(side.normal));
}

inline void addVoxel(MeshTool& meshTool, Level* level, const Voxel::Id id, const ivec2& chunkPos, const ivec3& pos) {
    for (int i = 0; i < sizeof(CUBE_SIDES) / sizeof(Side); ++i) {
        const auto sideNormal = CUBE_SIDES[i].normal;
        const auto globalPos = Location::fromGlobalPos(ivec3{pos.x + chunkPos.x * CHUNK_SIZE + sideNormal.x, pos.y + sideNormal.y, pos.z + chunkPos.y * CHUNK_SIZE + sideNormal.z});
        if (!level->hasChunk(globalPos.chunkPos))
            continue;
        if (level->isVoxelSolid(globalPos))
            continue;

        addFace(meshTool, CUBE_SIDES[i], VOXEL_ATLAS_OFFSETS[id][i], pos);
    }
}

[[noreturn]] inline void chunkerThread(Level* level) {
    spdlog::info("chunkerThread started");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    MeshTool meshTool;
    while (true) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // std::this_thread::yield();
        // std::this_thread::sleep_for(std::chrono::duration<float>(1));
        if (level->dirtyChunksQueue.empty())
            continue;
        spdlog::info("chunkerThread runninnggggg");

        const auto& [chunkPos, chunk] = level->dirtyChunksQueue.front();
        spdlog::info("{} {}", chunkPos.x, chunkPos.y);

#ifdef DEBUG
        auto t0 = std::chrono::high_resolution_clock::now();
#endif

        meshTool.clear();

        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < LEVEL_HEIGHT; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    const ivec3 pos = {x, y, z};
                    const Voxel::Id voxel = chunk->getVoxel(pos);
                    if (voxel == Voxel::AIR)
                        continue;

                    addVoxel(meshTool, level, voxel, chunkPos, pos);
                }
            }
        }

        auto newMesh = std::make_unique<raylib::Mesh>();
        newMesh->boneMatrices = nullptr; // raylib-cpp issue #344
        meshTool.exportToMesh(newMesh.get());

        level->chunksReadyToSwapMeshQueue.emplace(chunk, std::move(newMesh));

#ifdef DEBUG
        auto t1 = std::chrono::high_resolution_clock::now();
        spdlog::debug("{} ms", (t1-t0).count() / static_cast<double>(1'000'000));
#endif

        chunk->meshDirty = false;
        spdlog::debug("dirty chunk cleaned");

        level->dirtyChunksQueue.pop();
    }
}

inline void startChunkerThread(Level* level) {
    std::thread thread(chunkerThread, level);
    thread.detach();
}

#endif //VOXELGAME_MESHER_HPP
