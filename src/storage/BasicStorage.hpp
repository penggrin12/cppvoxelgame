//
// Created by penggrin on 29.03.2026.
//

#ifndef VOXELGAME_BASICSTORAGE_HPP
#define VOXELGAME_BASICSTORAGE_HPP
#include <filesystem>
#include <gzip/compress.hpp>
#include <gzip/decompress.hpp>

#include "Storage.hpp"


class BasicStorage final : public Storage {
protected:
    const std::filesystem::path BASE_SAVE_DIR;
    const std::filesystem::path CHUNKS_SAVE_DIR;
    const gzip::Compressor comp;
    const gzip::Decompressor decomp;

    [[nodiscard]] std::filesystem::path getChunkPath(const glm::ivec2 &chunkPos) const;
public:
    explicit BasicStorage(const std::string &saveName);

    [[nodiscard]] bool hasChunk(const glm::ivec2 &chunkPos) override;
    void loadChunk(const glm::ivec2 &chunkPos, Chunk *chunk) override;
    void saveChunk(const glm::ivec2 &chunkPos, Chunk *chunk) override;
};



#endif //VOXELGAME_BASICSTORAGE_HPP
