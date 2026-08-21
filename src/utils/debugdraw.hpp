//
// Created by penggrin on 16.08.2026.
//

#ifndef VOXELGAME_DEBUGDRAW_HPP
#define VOXELGAME_DEBUGDRAW_HPP

#include "Level.hpp"
#include "utils.hpp"

inline void debugDrawVoxel(const Location &loc, const Color color = raylib::Color::Green()) {
#ifndef NDEBUG
    DrawCubeWiresV(glm2rl(vec3(loc.getGlobalPos()) + vec3{0.5f, 0.5f, 0.5f}), {1, 1, 1}, color);
#endif
}

inline void debugDrawAABB(const AABB &aabb, const Color color = raylib::Color::Green()) {
#ifndef NDEBUG
    DrawCubeWires(glm2rl(vec3(aabb.a.x + aabb.b.x, aabb.a.y + aabb.b.y, aabb.a.z + aabb.b.z) * 0.5f),
                  aabb.b.x - aabb.a.x, aabb.b.y - aabb.a.y, aabb.b.z - aabb.a.z, color);
#endif
}

#endif //VOXELGAME_DEBUGDRAW_HPP
