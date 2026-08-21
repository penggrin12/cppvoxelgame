//
// Created by penggrin on 29.03.2026.
//

#include <filesystem>
#include <fstream>
#include <tracy/Tracy.hpp>

#include "BasicStorage.hpp"

const auto BASE_DIR = std::filesystem::path("saves/");

std::filesystem::path BasicStorage::getChunkPath(const glm::ivec2 &chunkPos) const {
    return CHUNKS_SAVE_DIR / std::format("{}.{}.bin", chunkPos.x, chunkPos.y);
}

BasicStorage::BasicStorage(const std::string &saveName) : BASE_SAVE_DIR(BASE_DIR / saveName), CHUNKS_SAVE_DIR(BASE_SAVE_DIR / "chunks") {
    if (!std::filesystem::exists(CHUNKS_SAVE_DIR))
        std::filesystem::create_directories(CHUNKS_SAVE_DIR);
}

bool BasicStorage::hasChunk(const glm::ivec2 &chunkPos) { ZoneScoped;
    return std::filesystem::exists(getChunkPath(chunkPos));
}

void BasicStorage::loadChunk(const glm::ivec2 &chunkPos, Chunk *chunk) { ZoneScoped;
    const auto filePath = getChunkPath(chunkPos);
    const auto fileSize = std::filesystem::file_size(filePath);

    thread_local std::array<char, CHUNK_VOXELS_TOTAL> buff;

    std::ifstream file;
    file.open(filePath, std::ios::binary | std::ios::in);
    file.read(buff.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    std::string dataOut;
    decomp.decompress(dataOut, buff.data(), fileSize);

    memcpy(chunk->getVoxels(), dataOut.c_str(), CHUNK_VOXELS_TOTAL);
}

void BasicStorage::saveChunk(const glm::ivec2 &chunkPos, Chunk *chunk) { ZoneScoped;
    std::string dataOut;
    comp.compress(dataOut, reinterpret_cast<char*>(chunk->getVoxels()), CHUNK_VOXELS_TOTAL);

    std::ofstream file;
    file.open(getChunkPath(chunkPos), std::ios::binary | std::ios::in);
    file.write(dataOut.c_str(), static_cast<std::streamsize>(dataOut.size()));
    file.close();
}
