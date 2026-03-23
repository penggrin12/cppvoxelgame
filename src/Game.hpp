//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_GAME_HPP
#define VOXELGAME_GAME_HPP

#include <memory>

#include "Entity.hpp"
#include "Input.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "audio/audio.hpp"

struct DebugStats {
    int tris = 0;
};

class Game {
private:
    raylib::Texture terrainTexture;
    raylib::Material terrainMaterial;

    GameAudio audio;
    GameInput input;
    raylib::Window& window;

    std::unique_ptr<Level> curLevel;

    std::vector<std::unique_ptr<Entity>> entities;

    void initResources();
    void drawDebug();
public:
    DebugStats debugStats;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    explicit Game(raylib::Window&);
    ~Game();

    GameAudio& getAudio() { return audio; }
    GameInput& getInput() { return input; }

    size_t addEntity(std::unique_ptr<Entity>& entity) {
        entities.push_back(std::move(entity));
        entities.back()->init();
        return entities.size() - 1;
    }
    [[nodiscard]] Entity* getEntity(const size_t index) const { return entities[index].get(); }

    [[nodiscard]] Player& getPlayer() const { return dynamic_cast<Player&>(*entities[0]); }

    void logic();
    void draw();
};

inline Game* game = nullptr;

#endif //VOXELGAME_GAME_HPP