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

constexpr static size_t CHUNK_VOXELS_TOTAL = CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE;

struct Location {
    ivec3 pos{}; // position within a chunk, 0..CHUNK_SIZE exclusive
    ivec2 chunkPos{}; // Y is Z. the level is a single chunk tall, minecraft style

    constexpr Location(const ivec3 pos, const ivec2 chunkPos) : pos(pos), chunkPos(chunkPos) {}

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr Location(const ivec3 globalPos) { // NOLINT(*-explicit-constructor)
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

    [[nodiscard]] constexpr ivec3 getGlobalPos() const {
        const auto offset = chunkPos * CHUNK_SIZE;
        return {pos.x + offset.x, pos.y, pos.z + offset.y};
    }

    [[nodiscard]] constexpr static Location fromGlobalPos(const ivec3 &globalPos) {
        return {globalPos};
    }

    [[nodiscard]] constexpr static Location fromRealGlobalPos(const vec3 &globalPos) {
        return fromGlobalPos(glm::floor(globalPos));
    }
};

class Chunk {
private:
    Voxel::Id voxels[CHUNK_VOXELS_TOTAL] = {};
public:
    std::unique_ptr<raylib::Mesh> mesh;
    bool meshDirty = true;
    bool hidden = false;

    [[nodiscard]] Voxel::Id* getVoxels();

    [[nodiscard]] static bool isVoxelInBounds(ivec3 pos);
    [[nodiscard]] bool isVoxelSolid(ivec3 pos) const;
    [[nodiscard]] Voxel::Id getVoxel(ivec3 pos) const;
    void setVoxel(ivec3 pos, Voxel::Id voxel);

    // this probably shouldn't be static
    // but for that Chunk needs to know it's position...
    [[nodiscard]] static AABB getAabb(ivec2 chunkPos);
};

class Level {
private:
    std::unordered_map<ivec2, std::unique_ptr<Chunk>> chunks;
public:
    std::mutex mutex;

    std::deque<ivec2> dirtyChunksQueue;
    std::queue<std::pair<ivec2, std::unique_ptr<raylib::Mesh>>> chunksReadyToSwapMeshQueue;

    [[nodiscard]] std::unordered_map<ivec2, std::unique_ptr<Chunk>>& getChunks() { return chunks; }

    [[nodiscard]] bool hasChunk(ivec2 chunkPos) const;
    void createChunk(ivec2 chunkPos);
    void removeChunk(ivec2 chunkPos);
    void genChunk(ivec2 chunkPos);

    // nullptr if it doesn't exist
    Chunk* getChunk(ivec2 chunkPos);

    [[nodiscard]] static bool isVoxelInBounds(Location loc);
    [[nodiscard]] bool isVoxelSolid(Location loc);

    [[nodiscard]] Voxel::Id getVoxel(Location loc);
    [[nodiscard]] Voxel::Id getVoxelOrAir(Location loc);
    void setVoxel(Location loc, Voxel::Id voxel);

    static constexpr AABB getVoxelAABB(const ivec3 pos) {
        return {static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z), static_cast<float>(pos.x + 1), static_cast<float>(pos.y + 1), static_cast<float>(pos.z + 1)};
    }
    std::vector<AABB> getCubes(const AABB &box);

    void markChunkDirty(ivec2 chunkPos);
    void markVoxelDirty(const Location &loc);
    void markVoxelDirtyAndNeighbours(const Location &loc);
};


#endif //VOXELGAME_LEVEL_HPP