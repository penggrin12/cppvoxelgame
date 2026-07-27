//
// Created by penggrin on 16.03.2026.
//

#include <cassert>

#include "Level.hpp"

#include <ranges>

#include "Game.hpp"
#include "Mesher.hpp"
#include "voxeldata.hpp"
#include "utils/noise.hpp"

/// Chunk

[[nodiscard]] constexpr static size_t indexVoxel(const int x, const int y, const int z) {
    assert(x >= 0 && x < CHUNK_SIZE);
    assert(y >= 0 && y < LEVEL_HEIGHT);
    assert(z >= 0 && z < CHUNK_SIZE);

    size_t const index = x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
    assert(index < CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE);

    return index;
}

Voxel::Id* Chunk::getVoxels() {
    return voxels;
}

bool Chunk::isVoxelInBounds(const ivec3 &pos) {
    if (pos.x < 0 || pos.x >= CHUNK_SIZE) return false;
    if (pos.y < 0 || pos.y >= LEVEL_HEIGHT) return false;
    if (pos.z < 0 || pos.z >= CHUNK_SIZE) return false;
    return true;
}

bool Chunk::isVoxelSolid(const ivec3 &pos) const {
    assert(isVoxelInBounds(pos));
    return getVoxel(pos) > Voxel::AIR;
}

[[nodiscard]] Voxel::Id Chunk::getVoxel(const ivec3 &pos) const {
    assert(isVoxelInBounds(pos));
    return voxels[indexVoxel(pos.x, pos.y, pos.z)];
}

void Chunk::setVoxel(const ivec3 &pos, const Voxel::Id voxel) {
    assert(isVoxelInBounds(pos));
    voxels[indexVoxel(pos.x, pos.y, pos.z)] = voxel;
}

/// Level

bool Level::hasChunk(const ivec2 &chunkPos) const {
    return chunks.contains(chunkPos);
}

void Level::createChunk(const ivec2 &chunkPos) {
    ASSERT_AND_RETURN_VOID(!hasChunk(chunkPos));
    chunks[chunkPos] = std::make_unique<Chunk>();
    // dirtyChunksQueue.emplace(chunkPos, getChunk(chunkPos));
}

void Level::removeChunk(const ivec2 &chunkPos) {
    chunks[chunkPos].reset();
    chunks.erase(chunkPos);
}

void Level::genChunk(const ivec2 &chunkPos) {
    Chunk *chunk = getChunk(chunkPos);
    ASSERT_AND_RETURN_VOID(chunk != nullptr)

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // const auto height = static_cast<int>(std::floor(noise.GetColor(x, z).r / 255.0 * 16));
            const auto height = static_cast<int>((noise::at(x + chunkPos.x * CHUNK_SIZE, z + chunkPos.y * CHUNK_SIZE) / 2.0f + 0.5f) * 64);
            // printf("%f\n", );
            for (int y = 0; y < LEVEL_HEIGHT; ++y) {
                if (height < y)
                    continue;

                chunk->setVoxel(ivec3(x, y, z), height == y ? Voxel::GRASS : Voxel::DIRT);
            }
        }
    }
}

Chunk* Level::getChunk(const ivec2 chunkPos) {
    if (!hasChunk(chunkPos))
        return nullptr;
    return chunks[chunkPos].get();
}

bool Level::isVoxelInBounds(const Location &loc) {
    // return Chunk::isVoxelInBounds(loc.pos);

    // TODO: should it check if a chunk at this loc exists?

    return loc.pos.y >= 0 && loc.pos.y < LEVEL_HEIGHT;
}

bool Level::isVoxelSolid(const Location &loc) {
    if (!isVoxelInBounds(loc))
        return false;

    const auto chunk = getChunk(loc.chunkPos);
    ASSERT_AND_RETURN(chunk != nullptr, false)
    return chunk->isVoxelSolid(loc.pos);
}

[[nodiscard]] Voxel::Id Level::getVoxel(const Location &loc) {
    const auto chunk = getChunk(loc.chunkPos);
    ASSERT_AND_RETURN(chunk != nullptr, Voxel::AIR)
    return chunk->getVoxel(loc.pos);
}

[[nodiscard]] Voxel::Id Level::getVoxelOrAir(const Location &loc) {
    if (!isVoxelInBounds(loc))
        return Voxel::AIR;
    return getVoxel(loc);
}

void Level::setVoxel(const Location &loc, const Voxel::Id voxel) {
    const auto chunk = getChunk(loc.chunkPos);
    ASSERT_AND_RETURN_VOID(chunk != nullptr)
    chunk->setVoxel(loc.pos, voxel);
}

constexpr AABB getVoxelAABB(const ivec3 &pos) {
    return AABB(pos.x, pos.y, pos.z, pos.x + 1, pos.y + 1, pos.z + 1);
}

void addAABBs(const ivec3 &pos, const AABB &box, std::vector<AABB>& boxes) {
    const AABB aabb = getVoxelAABB(pos);
    if (box.intersects(aabb))
        boxes.push_back(aabb);
}

std::vector<AABB> Level::getCubes(const AABB &box) {
    std::vector<AABB> boxes = {};

    const ivec3 &a = {floori(box.x0), floori(box.y0), floori(box.z0)};
    const ivec3 &b = {floori(box.x1 + 1), floori(box.y1 + 1), floori(box.z1 + 1)};

    for (int x = a.x; x < b.x; x++) {
        for (int z = a.z; z < b.z; z++) {
            // if (hasChunkAt(x, Level::DEPTH / 2, z)) {
            for (int y = a.y - 1; y < b.y; y++) {
                if (y < 0 || y >= LEVEL_HEIGHT)
                    continue;

                const ivec3 pos = {x, y, z};
                const auto voxel = getVoxel(pos);
                if (voxel <= Voxel::AIR)
                    continue;
                addAABBs(pos, box, boxes);
            }
            // }
        }
    }

    return boxes;
}

void Level::markChunkDirty(const ivec2 &chunkPos) {
    markChunkDirty(chunkPos, getChunk(chunkPos));
}

void Level::markChunkDirty(const ivec2 &chunkPos, Chunk *chunk) {
    {
        std::lock_guard lock(game->getMesher().mtx);
;
        for (const auto &otherChunkPos: dirtyChunksQueue._Get_container() | std::views::keys) {
            if (otherChunkPos == chunkPos)
                return;
        }

        dirtyChunksQueue.emplace(chunkPos, chunk);
    }
    game->getMesher().cv.notify_one();
}

void Level::markVoxelDirty(const Location &loc) {
    markChunkDirty(loc.chunkPos);
}

void Level::markVoxelDirtyAndNeighbours(const Location &loc) {
    std::queue<ivec2> q;
    q.push(loc.chunkPos);

    if (loc.pos.x == 0) q.push(loc.chunkPos + ivec2{-1, 0});
    if (loc.pos.x == CHUNK_SIZE - 1) q.push(loc.chunkPos + ivec2{1, 0});

    if (loc.pos.z == 0) q.push(loc.chunkPos + ivec2{0, -1});
    if (loc.pos.z == CHUNK_SIZE - 1) q.push(loc.chunkPos + ivec2{0, 1});

    // {
    //     std::lock_guard lock(chunkerMtx);
    //     while (!q.empty()) {
    //         dirtyChunksQueue.emplace(q.front(), getChunk(q.front()));
    //         q.pop();
    //     }
    // }
    // chunkerCv.notify_one();

    while (!q.empty()) {
        markChunkDirty(q.front());
        q.pop();
    }
}
