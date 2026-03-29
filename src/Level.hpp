//
// Created by penggrin on 16.03.2026.
//

#ifndef VOXELGAME_LEVEL_HPP
#define VOXELGAME_LEVEL_HPP
#include <memory>
#include <queue>
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "utils/utils.hpp"

#include "voxeldata.hpp"
#include "phys/AABB.hpp"
#include "utils/math.hpp"

constexpr int LEVEL_HEIGHT = 128;
constexpr int CHUNK_SIZE = 16; // squared (XZ), Y is as tall as the level

struct Location {
    ivec3 pos{}; // position within a chunk, 0..CHUNK_SIZE exclusive
    ivec2 chunkPos{}; // Y is Z. the level is a single chunk tall, minecraft style

    Location(const ivec3& pos, const ivec2& chunkPos) : pos(pos), chunkPos(chunkPos) {}

    // ReSharper disable once CppNonExplicitConvertingConstructor
    Location(const ivec3& globalPos) { // NOLINT(*-explicit-constructor)
        chunkPos = ivec2{
            floorDiv(globalPos.x, CHUNK_SIZE),
            floorDiv(globalPos.z, CHUNK_SIZE)
        };
        pos = ivec3{
            globalPos.x - chunkPos.x * CHUNK_SIZE,
            globalPos.y,
            globalPos.z - chunkPos.y * CHUNK_SIZE
        };
    }

    [[nodiscard]] ivec3 getGlobalPos() const {
        const auto offset = chunkPos * CHUNK_SIZE;
        return ivec3{pos.x + offset.x, pos.y, pos.z + offset.y};
    }

    [[nodiscard]] static Location fromGlobalPos(const ivec3& globalPos) {
        return {globalPos};
    }

    [[nodiscard]] static Location fromRealGlobalPos(const vec3& globalPos) {
        return fromGlobalPos(glm::floor(globalPos));
    }
};

class Chunk {
private:
    Voxel::Id voxels[CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE] = {};
public:
    std::unique_ptr<raylib::Mesh> mesh;
    bool meshDirty = true;

    [[nodiscard]] static bool isVoxelInBounds(const ivec3& pos);
    [[nodiscard]] bool isVoxelSolid(const ivec3& pos) const;
    [[nodiscard]] Voxel::Id getVoxel(const ivec3& pos) const;
    void setVoxel(const ivec3& pos, Voxel::Id voxel);
};

class Level {
private:
    std::unordered_map<ivec2, std::unique_ptr<Chunk>> chunks;
public:
    std::mutex mutex;

    std::queue<std::pair<ivec2, Chunk*>> dirtyChunksQueue;
    std::queue<std::pair<Chunk*, std::unique_ptr<raylib::Mesh>>> chunksReadyToSwapMeshQueue;

    [[nodiscard]] std::unordered_map<ivec2, std::unique_ptr<Chunk>>& getChunks() { return chunks; }

    [[nodiscard]] bool hasChunk(const ivec2& chunkPos);
    void createChunk(const ivec2& chunkPos);
    void removeChunk(const ivec2& chunkPos);
    void genChunk(const ivec2& chunkPos);

    // nullptr if it doesn't exist
    Chunk* getChunk(ivec2 chunkPos);

    [[nodiscard]] static bool isVoxelInBounds(const Location& loc);
    [[nodiscard]] bool isVoxelSolid(const Location& loc);

    [[nodiscard]] Voxel::Id getVoxel(const Location& loc);
    void setVoxel(const Location& loc, Voxel::Id voxel);

    std::vector<AABB> getCubes(const AABB& box);

    void markChunkDirty(const ivec2& chunkPos);
    void markChunkDirty(const ivec2 &chunkPos, Chunk* chunk);
    void markVoxelDirty(const Location& loc);
    void markVoxelDirtyAndNeighbours(const Location& loc);
};


#endif //VOXELGAME_LEVEL_HPP