//
// Created by penggrin on 16.08.2026.
//

#ifndef VOXELGAME_DEBUGDRAW_HPP
#define VOXELGAME_DEBUGDRAW_HPP

#include "Level.hpp"
#include "utils.hpp"

inline void debugDrawVoxel(const Location &loc, const Color color = raylib::Color::Green()) {
    DrawCubeWiresV(glm2rl(vec3(loc.getGlobalPos()) + vec3{0.5f, 0.5f, 0.5f}), {1, 1, 1}, color);
}

inline void debugDrawAABB(const AABB &aabb, const Color color = raylib::Color::Green()) {
    DrawCubeWires(glm2rl(vec3(aabb.x0 + aabb.x1, aabb.y0 + aabb.y1, aabb.z0 + aabb.z1) * 0.5f),
                  aabb.x1 - aabb.x0, aabb.y1 - aabb.y0, aabb.z1 - aabb.z0, color);

}

#endif //VOXELGAME_DEBUGDRAW_HPP
