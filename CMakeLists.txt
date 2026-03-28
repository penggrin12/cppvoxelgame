# generator must be "MinGW Makefiles" if not using MSVC for whatever reason

cmake_minimum_required(VERSION 4.0)

project(voxelgame)

set(CMAKE_CXX_STANDARD 26)

add_compile_definitions(
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

find_package(glfw3 CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(
        voxelgame
        src/main.cpp
        src/Game.hpp
        src/Game.cpp
        src/entities/Entity.hpp
        src/entities/Player.cpp
        src/entities/Player.hpp
        src/mesher.hpp
        src/utils/utils.hpp
        src/MeshTool.hpp
        src/Level.cpp
        src/Level.hpp
        src/voxeldata.hpp
        src/phys/raycast.hpp
        src/phys/AABB.hpp
        src/entities/TestEntity.cpp
        src/entities/TestEntity.hpp
        src/audio/audio.hpp
        src/audio/audio.cpp
        src/Input.hpp
        lib/rcamera.h
        src/utils/random.hpp
        src/utils/math.hpp
        src/utils/noise.hpp
        lib/fastnoiselite/FastNoiseLite.h
)

target_include_directories(voxelgame PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib
)

#target_compile_definitions(voxelgame PRIVATE "$<$<CONFIG:_DEBUG>:_DEBUG>")
target_link_libraries(voxelgame PRIVATE raylib glfw winmm spdlog::spdlog)

add_custom_target(copy_res ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:voxelgame>/res
        COMMENT "Copying res directory"
)
add_dependencies(voxelgame copy_res)
