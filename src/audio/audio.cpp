//
// Created by penggrin on 21.03.2026.
//

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER

#define MINIAUDIO_IMPLEMENTATION
#include <format>
#include <memory>
#include <miniaudio.h>
#include <ranges>
#include <raylib-cpp/Functions.hpp>

#include "audio.hpp"

#include <spdlog/spdlog.h>


struct GameAudio::Impl {
    ma_engine engine = {};
    std::unordered_map<std::string, std::vector<std::unique_ptr<ma_sound>>> cachedSounds = {};
};

GameAudio::GameAudio() {
    impl = new Impl();

    ma_result result = ma_engine_init(nullptr, &impl->engine);
    if (result != MA_SUCCESS) {
        throw std::runtime_error(std::format("failed to init miniaudio: {}", static_cast<int>(result)));
    }

    SPDLOG_INFO("miniaudio initialized");
}

GameAudio::~GameAudio() {
    if (impl == nullptr || impl->engine.pDevice == nullptr)
        return;

    for (auto& sounds: impl->cachedSounds | std::views::values)
        for (auto& sound : sounds)
            ma_sound_uninit(sound.get());

    ma_engine_uninit(&impl->engine);

    delete impl;
}

void GameAudio::updateListener(const glm::vec3 &pos, const glm::vec3 &dir) const {
    ma_engine_listener_set_position(&impl->engine, 0, pos.x, pos.y, pos.z);
    ma_engine_listener_set_direction(&impl->engine, 0, dir.x, dir.y, dir.z);
}

void GameAudio::cacheSounds(const std::string &path, const std::string &prefix) const {
    for (const auto& filePath: raylib::LoadDirectoryFiles(path)) {
        auto fileName = raylib::GetFileName(filePath);
        if (!fileName.starts_with(prefix))
            continue;
        SPDLOG_DEBUG("found audio file for prefix %s: %s", prefix.c_str(), fileName.c_str());

        impl->cachedSounds[prefix].push_back(std::make_unique<ma_sound>());
        ma_result result = ma_sound_init_from_file(&impl->engine, filePath.c_str(), 0, nullptr, nullptr, impl->cachedSounds[prefix].back().get());
        if (result != MA_SUCCESS) {
            throw std::runtime_error(std::format("failed to load audio: {}", static_cast<int>(result)).c_str());
        }
    }
}

[[nodiscard]] void* GameAudio::getSound(const std::string &prefix) const {
    return impl->cachedSounds[prefix][rand() % impl->cachedSounds[prefix].size()].get();
}

void GameAudio::playSound(const std::string &prefix) const {
    const auto sound = static_cast<ma_sound*>(getSound(prefix));

    ma_sound_set_spatialization_enabled(sound, MA_FALSE);
    ma_sound_start(sound);
}

void GameAudio::playSound(const std::string &prefix, const glm::vec3& pos) const {
    const auto sound = static_cast<ma_sound*>(getSound(prefix));

    ma_sound_set_spatialization_enabled(sound, MA_TRUE);
    ma_sound_set_position(sound, pos.x, pos.y, pos.z);
    ma_sound_start(sound);
}
