//
// Created by penggrin on 21.03.2026.
//

#ifndef VOXELGAME_AUDIO_HPP
#define VOXELGAME_AUDIO_HPP
#include <string>
#include <glm/glm.hpp>

class GameAudio {
private:
    struct Impl;
    Impl* impl = nullptr;
public:
    GameAudio();
    ~GameAudio();

    void updateListener(const glm::vec3& pos, const glm::vec3 &dir) const;

    void cacheSounds(const std::string& path, const std::string& prefix) const;

    [[nodiscard]] void* getSound(const std::string& prefix) const;
    void playSound(const std::string& prefix) const;
    void playSound(const std::string& prefix, const glm::vec3& pos) const;
};

#endif //VOXELGAME_AUDIO_HPP