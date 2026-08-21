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

constexpr int LEVEL_HEIGHT = 256;
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
    std::mutex mutex;
    struct MainThreadLock {
        std::unique_lock<std::mutex> lock;
        explicit MainThreadLock(Level* level) {
            level->mainThreadWaiting.store(true, std::memory_order_relaxed);
            lock = std::unique_lock(level->mutex);
            level->mainThreadWaiting.store(false, std::memory_order_relaxed);
        }
    };

    struct WorkerThreadLock {
        std::unique_lock<std::mutex> lock;
        explicit WorkerThreadLock(Level* level) : lock(level->mutex, std::defer_lock) {
            while (true) {
                while (level->mainThreadWaiting.load(std::memory_order_relaxed)) {
                    std::this_thread::yield();
                }
                if (!lock.try_lock()) {
                    std::this_thread::yield();
                    continue;
                }
                if (level->mainThreadWaiting.load(std::memory_order_relaxed)) {
                    lock.unlock();
                    std::this_thread::yield();
                    continue;
                }
                break;
            }
        }
    };
public:

    std::atomic<bool> mainThreadWaiting{false};

    std::deque<ivec2> dirtyChunksQueue;
    std::queue<std::pair<ivec2, std::unique_ptr<raylib::Mesh>>> chunksReadyToSwapMeshQueue;

    MainThreadLock main_thread_lock() { return MainThreadLock{this}; }
    WorkerThreadLock worker_thread_lock() { return WorkerThreadLock{this}; }

    [[nodiscard]] std::unordered_map<ivec2, std::unique_ptr<Chunk>>& getChunks() { return chunks; }

    [[nodiscard]] bool hasChunk(ivec2 chunkPos) const;
    void createChunk(ivec2 chunkPos);
    void removeChunk(ivec2 chunkPos);
    void genChunk(ivec2 chunkPos);

    // nullptr if it doesn't exist
    Chunk* getChunk(ivec2 chunkPos);

    [[nodiscard]] static bool isVoxelInBounds(const Location &loc);
    [[nodiscard]] bool isVoxelSolid(const Location &loc);

    [[nodiscard]] Voxel::Id getVoxel(const Location &loc);
    [[nodiscard]] Voxel::Id getVoxelOrAir(const Location &loc);
    void setVoxel(const Location &loc, Voxel::Id voxel);

    static constexpr AABB getVoxelAABB(const ivec3 pos) {
        return {static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z), static_cast<float>(pos.x + 1), static_cast<float>(pos.y + 1), static_cast<float>(pos.z + 1)};
    }
    std::vector<AABB> getCubes(const AABB &box);

    void markChunkDirty(ivec2 chunkPos);
    void markVoxelDirty(const Location &loc);
    void markVoxelDirtyAndNeighbours(const Location &loc);
};


#endif //VOXELGAME_LEVEL_HPP