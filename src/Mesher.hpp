//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHER_HPP
#define VOXELGAME_MESHER_HPP

#include "Level.hpp"
#include "MeshTool.hpp"


class Mesher {
private:
    class MesherCache;

    struct QuadUVs {
        vec2 a, b, c, d;
    };

    constexpr static int TEXTURE_ATLAS_ITEM_SIZE = 16; // X and Y
    constexpr static int TEXTURE_ATLAS_PIXEL_SIZE = 256; // X and Y

    constexpr static ivec3 CUBE_VERTICES[8] = {
        {0, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1}
    };

    struct Side {
        int i;
        int v0, v1, v2, v3;
        ivec3 normal;
    };

    constexpr static Side SIDE_FRONT  = {0, 4, 5, 6, 7, { 0,  0,  1}};
    constexpr static Side SIDE_BACK   = {1, 1, 0, 3, 2, { 0,  0, -1}};
    constexpr static Side SIDE_LEFT   = {2, 0, 4, 7, 3, {-1,  0,  0}};
    constexpr static Side SIDE_RIGHT  = {3, 5, 1, 2, 6, { 1,  0,  0}};
    constexpr static Side SIDE_TOP    = {4, 7, 6, 2, 3, { 0,  1,  0}};
    constexpr static Side SIDE_BOTTOM = {5, 0, 1, 5, 4, { 0, -1,  0}};

    constexpr static Side CUBE_SIDES[6] = {SIDE_TOP, SIDE_BOTTOM, SIDE_LEFT, SIDE_RIGHT, SIDE_BACK, SIDE_FRONT};

    constexpr static float AO_VALUES[4] = {0.4f, 0.6f, 0.8f, 1.0f};

    std::atomic<bool> shouldStop{false};

    std::vector<std::thread> workerThreads = {};

    constexpr static int ao(int side1, int side2, int corner);
    static glm::vec4 getAo(const Side &side, const MesherCache &cache, const Location &loc);

    static QuadUVs getQuadUVs(const glm::ivec2 &atlasOffset);
    static void addFace(MeshTool &meshTool, const MesherCache &cache, const Side &side, const glm::ivec2 &atlasOffset, const Location &loc);
    static void addVoxel(MeshTool &meshTool, const MesherCache &cache, Voxel::Id id, const Location &loc);
    static void addVegetation(MeshTool &meshTool, Voxel::Id id, const Location &loc);
public:
    // these should be private...
    // but some other code depends on them
    std::mutex mtx;
    std::condition_variable cv;

    void worker(Level* level);

    void startThread(Level* level);
    void stopThread();
};

#endif //VOXELGAME_MESHER_HPP
