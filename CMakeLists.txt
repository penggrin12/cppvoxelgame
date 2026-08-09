cmake_minimum_required(VERSION 3.28)

project(voxelgame)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

function(FETCH NAME REPO TAG)
    FetchContent_Declare(${NAME}
        GIT_REPOSITORY ${REPO}
        GIT_TAG ${TAG}
        GIT_SHALLOW 1
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(${NAME})
endfunction()

FETCH(miniaudio https://github.com/mackron/miniaudio.git master)
FETCH(glm https://github.com/g-truc/glm.git master)
FETCH(raylib https://github.com/raysan5/raylib.git master)
FETCH(raylib_cpp https://github.com/RobLoach/raylib-cpp.git master)
FETCH(spdlog https://github.com/gabime/spdlog.git v1.x)
FETCH(zlib https://github.com/madler/zlib.git master)
FETCH(gzip_hpp https://github.com/LimiNode/gzip-hpp.git main)
FETCH(cxxopts https://github.com/jarro2783/cxxopts.git v3.3.1)
FETCH(tracy https://github.com/wolfpld/tracy.git master)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/*.h"
    "lib/*.cpp"
    "lib/*.h"
)
add_executable(voxelgame ${GAME_SOURCES})

target_include_directories(voxelgame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_compile_definitions(voxelgame PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:TRACY_ENABLE>
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

target_link_libraries(voxelgame PRIVATE
    miniaudio
    glm
    raylib
    raylib_cpp
    spdlog::spdlog
    zlibstatic
    gzip_hpp
    cxxopts
    TracyClient
)

add_custom_target(copy_res ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:voxelgame>/res
        COMMENT "Copying res directory"
)
add_dependencies(voxelgame copy_res)
