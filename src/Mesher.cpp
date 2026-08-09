//
// Created by penggrin on 29.03.2026.
//

#include "Mesher.hpp"

constexpr int Mesher::ao(const int side1, const int side2, const int corner) {
    if (side1 + side2 == 2)
        return 0;
    return 3 - (side1 + side2 + corner);
}

glm::vec4 Mesher::getAo(const Side &side, Level *level, const Location &loc) {
    glm::vec4 aos;
    const int vertices[4] = {side.v0, side.v1, side.v2, side.v3};

    for (int i = 0; i < 4; ++i) {
        glm::ivec3 vertex = CUBE_VERTICES[vertices[i]];

        glm::ivec3 D = vertex * 2 - glm::ivec3(1, 1, 1);
        glm::ivec3 T = D - side.normal;

        glm::ivec3 side1Offset = T;
        glm::ivec3 side2Offset = T;

        if (T.x != 0) {
            if (T.y != 0) {
                side1Offset.y = 0; side2Offset.x = 0;
            } else {
                side1Offset.z = 0; side2Offset.x = 0;
            }
        } else {
            side1Offset.z = 0; side2Offset.y = 0;
        }

        Voxel::Id side1 = level->getVoxelOrAir(loc.getGlobalPos() + side.normal + side1Offset);
        Voxel::Id side2 = level->getVoxelOrAir(loc.getGlobalPos() + side.normal + side2Offset);
        Voxel::Id corner = level->getVoxelOrAir(loc.getGlobalPos() + side.normal + T);

        int aoVal = ao(
            Voxel::isSolid(side1) ? 1 : 0,
            Voxel::isSolid(side2) ? 1 : 0,
            Voxel::isSolid(corner) ? 1 : 0
        );

        aos[i] = AO_VALUES[aoVal];
    }

    return aos;
}

void Mesher::addFace(MeshTool &meshTool, Level *level, const Side &side, const ivec2 &atlasOffset, const Location &loc) {
    const ivec3 a = CUBE_VERTICES[side.v0] + loc.pos;
    const ivec3 b = CUBE_VERTICES[side.v1] + loc.pos;
    const ivec3 c = CUBE_VERTICES[side.v2] + loc.pos;
    const ivec3 d = CUBE_VERTICES[side.v3] + loc.pos;

    static constexpr float uvSize = 1.0f / TEXTURE_ATLAS_ITEM_SIZE;

    const vec2 uvOffset = static_cast<vec2>(atlasOffset) * uvSize;
    constexpr float inset = 1.0f / TEXTURE_ATLAS_PIXEL_SIZE * 0.01f; // hacky way to prevent texture bleeding

    const vec2 uvA = {uvOffset.x + inset,            uvOffset.y + uvSize - inset};
    const vec2 uvB = {uvOffset.x + uvSize - inset,   uvOffset.y + uvSize - inset};
    const vec2 uvC = {uvOffset.x + uvSize - inset,   uvOffset.y + inset};
    const vec2 uvD = {uvOffset.x + inset,            uvOffset.y + inset};

    const glm::vec4 ao = getAo(side, level, loc);
    // glm::vec4 ao = glm::vec4(1);

    meshTool.addQuad(a, b, c, d, uvA, uvB, uvC, uvD, vec3(side.normal), ao);
}

void Mesher::addVoxel(MeshTool &meshTool, Level *level, const Voxel::Id id, const Location &loc) {
    for (int i = 0; i < sizeof(CUBE_SIDES) / sizeof(Side); ++i) {
        const auto sideNormal = CUBE_SIDES[i].normal;
        // const auto sideLoc = Location::fromGlobalPos(ivec3{pos.x + chunkPos.x * CHUNK_SIZE + sideNormal.x, pos.y + sideNormal.y, pos.z + chunkPos.y * CHUNK_SIZE + sideNormal.z});
        const auto sideLoc = Location::fromGlobalPos(loc.getGlobalPos() + sideNormal);
        assert(level->hasChunk(sideLoc.chunkPos));
        if (level->isVoxelSolid(sideLoc))
            continue;

        addFace(meshTool, level, CUBE_SIDES[i], VOXEL_ATLAS_OFFSETS[id][i], loc);
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

                    addVoxel(meshTool, level, voxel, Location(pos, chunkPos));
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
    thread = std::thread(&Mesher::chunkerThread, this, level);
}

void Mesher::stopThread() {
    shouldStop = true;
    cv.notify_all();
    thread.join();
}
