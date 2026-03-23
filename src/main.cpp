#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "raylib-cpp.hpp"


void raylibLog(const int logLevel, const char *text, va_list args) {
    std::string formattedText;
    va_list tmp_args;
    va_copy(tmp_args, args);
    const int len = vsnprintf(nullptr, 0, text, tmp_args);
    va_end(tmp_args);
    formattedText.resize(len);
    vsnprintf(formattedText.data(), len + 1, text, args);

    switch (logLevel) {
        case LOG_TRACE:
            spdlog::trace(formattedText);
            break;
        case LOG_DEBUG:
            spdlog::debug(formattedText);
            break;
        case LOG_INFO:
            spdlog::info(formattedText);
            break;
        case LOG_WARNING:
            spdlog::warn(formattedText);
            break;
        case LOG_ERROR:
            spdlog::error(formattedText);
            break;
        case LOG_FATAL:
            spdlog::critical(formattedText);
            break;
        default:
            break;
    }
}

int main() {
    spdlog::set_pattern("[%H:%M:%S:%e] [%l] %v");

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SetTraceLogCallback(raylibLog);

    raylib::Window window(800, 600, "voxel game");
    // window.SetTargetFPS(60);

    Game _game(window);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    game = &_game;

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(raylib::Color::Black());

        game->logic();
        game->draw();

        window.EndDrawing();
    }

    // window closes in raylib::Window destructor

    return 0;
}
