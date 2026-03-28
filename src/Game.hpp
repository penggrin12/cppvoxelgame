//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_GAME_HPP
#define VOXELGAME_GAME_HPP

#include <memory>

#include "Input.hpp"
#include "Level.hpp"
#include "audio/audio.hpp"
#include "entities/Entity.hpp"
#include "entities/Player.hpp"

struct DebugStats {
    uint64_t frame = 0;
    int tris = 0;
};

class Game;

struct GameResources {
    raylib::Font font;
    raylib::Texture terrainTexture;
    raylib::Material terrainMaterial;

    explicit GameResources(Game*);
};

class Game {
private:
    GameAudio audio;
    GameInput input;
    raylib::Window& window;

    std::unique_ptr<Level> curLevel;

    std::vector<std::unique_ptr<Entity>> entities;

    void drawDebug();
public:
    GameResources res;
    DebugStats debugStats;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    explicit Game(raylib::Window&);
    ~Game();

    [[nodiscard]] Level* getCurrLevel() const { return curLevel.get(); }

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

    void DrawTextB(const std::string& text, const int posX, const int posY, const int fontSize, const raylib::Color color = raylib::Color::White()) const {
        DrawTextEx(res.font, text, raylib::Vector2{static_cast<float>(posX), static_cast<float>(posY)}, fontSize, 0, color);
    }
};

inline Game* game = nullptr;

#endif //VOXELGAME_GAME_HPP