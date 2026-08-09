#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include <tracy/Tracy.hpp>

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

int main(int argc, char **argv) {
    spdlog::set_pattern("[%H:%M:%S:%e] [%s:%!:%#] [%l] %v");

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    SetTraceLogCallback(raylibLog);

    cxxopts::Options options{"voxelgame", "One line description of MyProgram"};
    options.add_options()
        ("h,help", "Display this help message")
        ("v,vsync", "Enable VSync")
        ;

    cxxopts::ParseResult args;

    try {
        args = options.parse(argc, argv);
    } catch (cxxopts::exceptions::exception &e) {
        SPDLOG_CRITICAL(e.what());
        return EXIT_FAILURE;
    }

    if (args.contains("help")) {
        printf("%s\n", options.help().c_str());
        return EXIT_SUCCESS;
    }

    if (args.contains("vsync"))
        raylib::Window::SetConfigFlags(FLAG_VSYNC_HINT);
    raylib::Window window(1200, 800, "voxel game");
    // window.SetTargetFPS(60);

    Game _game(window);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    game = &_game;

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(raylib::Color::Black());

        game->logic();
        game->draw();

        game->debugStats.frame++;
        window.EndDrawing();
        FrameMark;
    }

    // window closes in raylib::Window destructor

    return EXIT_SUCCESS;
}
