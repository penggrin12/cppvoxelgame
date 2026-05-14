//
// Created by penggrin on 29.03.2026.
//

#ifndef VOXELGAME_STORAGE_HPP
#define VOXELGAME_STORAGE_HPP
#include <glm/vec2.hpp>

#include "Level.hpp"

class Storage {
public:
    virtual ~Storage() = default;

    [[nodiscard]] virtual bool hasChunk(const glm::ivec2& chunkPos) = 0;
    virtual void loadChunk(const glm::ivec2& chunkPos, Chunk* chunk) = 0;
    virtual void saveChunk(const glm::ivec2& chunkPos, Chunk* chunk) = 0;
};

#endif //VOXELGAME_STORAGE_HPP
