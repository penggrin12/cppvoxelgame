//
// Created by penggrin on 29.03.2026.
//

#include "Mesher.hpp"

#include "tracy/Tracy.hpp"
#include "utils/random.hpp"


class MesherCache {
public:
    Voxel::Id voxels[3][3][CHUNK_VOXELS_TOTAL];
    bool exists[3][3];
    ivec2 center;

    void update(Level* level, const ivec2 centerPos) {
        center = centerPos;
        std::lock_guard lock(level->mutex);
        for(int x = -1; x <= 1; ++x) {
            for(int z = -1; z <= 1; ++z) {
                Chunk* c = level->getChunk(centerPos + ivec2(x, z));
                if(c) {
                    exists[x+1][z+1] = true;
                    std::memcpy(voxels[x+1][z+1], c->getVoxels(), CHUNK_VOXELS_TOTAL * sizeof(Voxel::Id));
                } else {
                    exists[x+1][z+1] = false;
                }
            }
        }
    }

    [[nodiscard]] Voxel::Id getVoxelOrAir(const Location& loc) const {
        if (loc.pos.y < 0 || loc.pos.y >= LEVEL_HEIGHT) return Voxel::AIR;
        ivec2 offset = loc.chunkPos - center;
        if (offset.x >= -1 && offset.x <= 1 && offset.y >= -1 && offset.y <= 1) {
            if (exists[offset.x+1][offset.y+1]) {
                size_t index = loc.pos.x + loc.pos.z * CHUNK_SIZE + loc.pos.y * CHUNK_SIZE * CHUNK_SIZE;
                return voxels[offset.x+1][offset.y+1][index];
            }
        }
        return Voxel::AIR;
    }
};

constexpr int Mesher::ao(const int side1, const int side2, const int corner) {
    if (side1 + side2 == 2)
        return 0;
    return 3 - (side1 + side2 + corner);
}

glm::vec4 Mesher::getAo(const Side &side, const MesherCache &cache, const Location &loc) {
    glm::vec4 aos;
    const int vertices[4] = {side.v0, side.v1, side.v2, side.v3};

    for (int i = 0; i < 4; ++i) {
        glm::ivec3 vertex = CUBE_VERTICES[vertices[i]];
        glm::ivec3 D = vertex * 2 - glm::ivec3(1, 1, 1);
        glm::ivec3 T = D - side.normal;
        glm::ivec3 side1Offset = T, side2Offset = T;

        if (T.x != 0) {
            if (T.y != 0) { side1Offset.y = 0; side2Offset.x = 0; } else { side1Offset.z = 0; side2Offset.x = 0; }
        } else {
            side1Offset.z = 0; side2Offset.y = 0;
        }

        Voxel::Id side1 = cache.getVoxelOrAir(loc.getGlobalPos() + side.normal + side1Offset);
        Voxel::Id side2 = cache.getVoxelOrAir(loc.getGlobalPos() + side.normal + side2Offset);
        Voxel::Id corner = cache.getVoxelOrAir(loc.getGlobalPos() + side.normal + T);

        int aoVal = ao(Voxel::isSolid(side1) ? 1 : 0, Voxel::isSolid(side2) ? 1 : 0, Voxel::isSolid(corner) ? 1 : 0);
        aos[i] = AO_VALUES[aoVal];
    }
    return aos;
}

QuadUVs Mesher::getQuadUVs(const glm::ivec2 &atlasOffset) {
    static constexpr float uvSize = 1.0f / TEXTURE_ATLAS_ITEM_SIZE;
    const vec2 uvOffset = static_cast<vec2>(atlasOffset) * uvSize;
    constexpr float inset = 1.0f / TEXTURE_ATLAS_PIXEL_SIZE * 0.01f; // hacky way to prevent texture bleeding

    return {
        {uvOffset.x + inset,          uvOffset.y + uvSize - inset},
        {uvOffset.x + uvSize - inset, uvOffset.y + uvSize - inset},
        {uvOffset.x + uvSize - inset, uvOffset.y + inset},
        {uvOffset.x + inset,          uvOffset.y + inset}
    };
}

void Mesher::addFace(MeshTool &meshTool, const MesherCache &cache, const Side &side, const ivec2 &atlasOffset, const Location &loc) {
    const ivec3 a = CUBE_VERTICES[side.v0] + loc.pos;
    const ivec3 b = CUBE_VERTICES[side.v1] + loc.pos;
    const ivec3 c = CUBE_VERTICES[side.v2] + loc.pos;
    const ivec3 d = CUBE_VERTICES[side.v3] + loc.pos;

    const auto [uvA, uvB, uvC, uvD] = getQuadUVs(atlasOffset);
    const glm::vec4 ao = getAo(side, cache, loc);
    meshTool.addQuad(a, b, c, d, uvA, uvB, uvC, uvD, vec3(side.normal), ao);
}

void Mesher::addVoxel(MeshTool &meshTool, const MesherCache &cache, const Voxel::Id id, const Location &loc) {
    if (VOXEL_TYPES[id] == Voxel::Type::VEGETATION) {
        addVegetation(meshTool, id, loc);
        return;
    }

    for (int i = 0; i < sizeof(CUBE_SIDES) / sizeof(Side); ++i) {
        const auto sideNormal = CUBE_SIDES[i].normal;
        const auto sideLoc = Location::fromGlobalPos(loc.getGlobalPos() + sideNormal);

        if (Voxel::isSolid(cache.getVoxelOrAir(sideLoc))) continue;

        addFace(meshTool, cache, CUBE_SIDES[i], VOXEL_ATLAS_OFFSETS[id][i], loc);
    }
}

void Mesher::addVegetation(MeshTool &meshTool, const Voxel::Id id, const Location &loc) {
    static constexpr int CROSS_VERTICES[2][4] = {
        {0, 5, 6, 3},
        {1, 4, 7, 2}
    };

    const auto [uvA, uvB, uvC, uvD] = getQuadUVs(VOXEL_ATLAS_OFFSETS[id][0]);

    // should maybe be dependent on location
    const vec3 posOffset = {
        rng::range(-0.2, 0.2), 0, rng::range(-0.2, 0.2)
    };
    for (const auto &[v0, v1, v2, v3] : CROSS_VERTICES) {
        const vec3 a = static_cast<vec3>(CUBE_VERTICES[v0] + loc.pos) + posOffset;
        const vec3 b = static_cast<vec3>(CUBE_VERTICES[v1] + loc.pos) + posOffset;
        const vec3 c = static_cast<vec3>(CUBE_VERTICES[v2] + loc.pos) + posOffset;
        const vec3 d = static_cast<vec3>(CUBE_VERTICES[v3] + loc.pos) + posOffset;

        // doesn't look quite right... maybe the texture makes up for it?
        // static constexpr glm::vec4 ao = {AO_VALUES[0] * 1.5f, AO_VALUES[0] * 1.5f, AO_VALUES[3], AO_VALUES[3]};
        static constexpr auto ao = glm::vec4{AO_VALUES[3]}; // no ao

        meshTool.addQuad(a, b, c, d, uvA, uvB, uvC, uvD, {0.0f, 1.0f, 0.0f}, ao);
        meshTool.addQuad(b, a, d, c, uvA, uvB, uvC, uvD, {0.0f, 1.0f, 0.0f}, ao);
    }
}

void Mesher::chunkerThread(Level *level) { ZoneScoped;
    SPDLOG_INFO("chunkerThread started");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    MeshTool meshTool;
    meshTool.reserve(CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE * 6 / 2);

    auto cache = std::make_unique<MesherCache>();

    while (!shouldStop) { ZoneScoped
        std::unique_lock chunkerLock(mtx);
        cv.wait(chunkerLock, [&] { return shouldStop || !level->dirtyChunksQueue.empty(); });
        if (shouldStop) break;

        const auto chunkPos = level->dirtyChunksQueue.front();
        ZoneNameF("%d %d", chunkPos.x, chunkPos.y);
        level->dirtyChunksQueue.pop_front();
        chunkerLock.unlock();

        meshTool.clear();

        {
            ZoneScopedN("cache->update");
            cache->update(level, chunkPos); // This briefly locks mutex, memcpy runs, then it unlocks
        }

        if (!cache->exists[1][1]) continue; // Was this chunk safely unloaded before meshing?

        for (int y = 0; y < LEVEL_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    const ivec3 pos = {x, y, z};
                    const Voxel::Id voxel = cache->voxels[1][1][x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE];

                    if (voxel == Voxel::AIR) continue;
                    addVoxel(meshTool, *cache, voxel, Location(pos, chunkPos));
                }
            }
        }

        auto newMesh = std::make_unique<raylib::Mesh>();
        meshTool.exportToMesh(newMesh.get());

        {
            std::lock_guard lock(level->mutex);
            if (level->hasChunk(chunkPos)) {
                level->chunksReadyToSwapMeshQueue.emplace(chunkPos, std::move(newMesh));
                level->getChunk(chunkPos)->meshDirty = false;
            }
        }
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
