//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHER_HPP
#define VOXELGAME_MESHER_HPP

#include "Level.hpp"
#include "MeshTool.hpp"

constexpr int TEXTURE_ATLAS_ITEM_SIZE = 16; // X and Y
constexpr int TEXTURE_ATLAS_PIXEL_SIZE = 256; // X and Y

constexpr ivec3 CUBE_VERTICES[8] = {
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
    int v0, v1, v2, v3;
    ivec3 normal;
};

constexpr Side SIDE_FRONT  = {4, 5, 6, 7, { 0,  0,  1}};
constexpr Side SIDE_BACK   = {1, 0, 3, 2, { 0,  0, -1}};
constexpr Side SIDE_LEFT   = {0, 4, 7, 3, {-1,  0,  0}};
constexpr Side SIDE_RIGHT  = {5, 1, 2, 6, { 1,  0,  0}};
constexpr Side SIDE_TOP    = {7, 6, 2, 3, { 0,  1,  0}};
constexpr Side SIDE_BOTTOM = {0, 1, 5, 4, { 0, -1,  0}};

constexpr Side CUBE_SIDES[6] = {SIDE_TOP, SIDE_BOTTOM, SIDE_LEFT, SIDE_RIGHT, SIDE_BACK, SIDE_FRONT};

class Mesher {
private:
    bool shouldStop = false;
public:
    std::mutex mtx;
    std::condition_variable cv;

    static void addFace(MeshTool& meshTool, const Side &side, const glm::ivec2& atlasOffset, const glm::ivec3& offset);
    static void addVoxel(MeshTool& meshTool, Level* level, Voxel::Id id, const glm::ivec2& chunkPos, const glm::ivec3& pos);

    void chunkerThread(Level* level);
    void startThread(Level* level);
    void stopThread();
};

#endif //VOXELGAME_MESHER_HPP
