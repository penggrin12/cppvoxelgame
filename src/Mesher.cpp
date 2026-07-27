//
// Created by penggrin on 29.03.2026.
//

#include "Mesher.hpp"

void Mesher::addFace(MeshTool& meshTool, const Side &side, const ivec2& atlasOffset, const ivec3& offset) {
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

void Mesher::addVoxel(MeshTool &meshTool, Level *level, const Voxel::Id id, const ivec2 &chunkPos, const ivec3 &pos) {
    for (int i = 0; i < sizeof(CUBE_SIDES) / sizeof(Side); ++i) {
        const auto sideNormal = CUBE_SIDES[i].normal;
        const auto sideLoc = Location::fromGlobalPos(ivec3{pos.x + chunkPos.x * CHUNK_SIZE + sideNormal.x, pos.y + sideNormal.y, pos.z + chunkPos.y * CHUNK_SIZE + sideNormal.z});
        assert(level->hasChunk(sideLoc.chunkPos));
        if (level->isVoxelSolid(sideLoc))
            continue;

        addFace(meshTool, CUBE_SIDES[i], VOXEL_ATLAS_OFFSETS[id][i], pos);
    }
}

void Mesher::chunkerThread(Level *level) {
    SPDLOG_INFO("chunkerThread started");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    MeshTool meshTool;
    while (!shouldStop) {
        std::unique_lock chunkerLock(mtx);
        cv.wait(chunkerLock, [&] {
            return shouldStop || !level->dirtyChunksQueue.empty();
        });
        if (shouldStop) break;

        SPDLOG_INFO("chunkerThread running");

        const auto [chunkPos, chunk] = level->dirtyChunksQueue.front();
        level->dirtyChunksQueue.pop();
        chunkerLock.unlock();
        SPDLOG_INFO("{} {}", chunkPos.x, chunkPos.y);

#ifdef DEBUG
        auto t0 = std::chrono::high_resolution_clock::now();
#endif

        meshTool.clear();

        std::lock_guard lock(level->mutex);
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
        meshTool.exportToMesh(newMesh.get());

        level->chunksReadyToSwapMeshQueue.emplace(chunk, std::move(newMesh));

#ifdef DEBUG
        auto t1 = std::chrono::high_resolution_clock::now();
        SPDLOG_DEBUG("{} ms", (t1-t0).count() / static_cast<double>(1'000'000));
#endif

        chunk->meshDirty = false;
        SPDLOG_DEBUG("dirty chunk cleaned");
    }
}

void Mesher::startThread(Level *level) {
    std::thread(&Mesher::chunkerThread, this, level).detach();
}

void Mesher::stopThread() {
    shouldStop = true;
}
