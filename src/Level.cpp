//
// Created by penggrin on 16.03.2026.
//

#include <cassert>

#include "Level.hpp"

#include "voxeldata.hpp"

/// Chunk

[[nodiscard]] constexpr static size_t indexVoxel(const int x, const int y, const int z) {
    assert(x >= 0 && x < CHUNK_SIZE);
    assert(y >= 0 && y < LEVEL_HEIGHT);
    assert(z >= 0 && z < CHUNK_SIZE);

    size_t const index = x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
    assert(index < CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE);

    return index;
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

[[nodiscard]] Voxel::Id Level::getVoxel(const Location& loc) {
    const auto chunk = getChunk(loc.chunkPos);
    ASSERT_AND_RETURN(chunk != nullptr, Voxel::AIR)
    return chunk->getVoxel(loc.pos);
}

void Level::setVoxel(const Location& loc, const Voxel::Id voxel) {
    const auto chunk = getChunk(loc.chunkPos);
    ASSERT_AND_RETURN(chunk != nullptr,)
    chunk->setVoxel(loc.pos, voxel);
}

constexpr AABB getVoxelAABB(const ivec3& pos) {
    return AABB(pos.x, pos.y, pos.z, pos.x + 1, pos.y + 1, pos.z + 1);
}

void addAABBs(const ivec3& pos, const AABB& box, std::vector<AABB>& boxes) {
    const AABB aabb = getVoxelAABB(pos);
    if (box.intersects(aabb))
        boxes.push_back(aabb);
}

std::vector<AABB> Level::getCubes(const AABB& box) {
    std::vector<AABB> boxes = {};

    const ivec3& a = {floori(box.x0), floori(box.y0), floori(box.z0)};
    const ivec3& b = {floori(box.x1 + 1), floori(box.y1 + 1), floori(box.z1 + 1)};

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
