//
// Created by penggrin on 20.03.2026.
//

#ifndef VOXELGAME_RAYCAST_HPP
#define VOXELGAME_RAYCAST_HPP

#include "../Level.hpp"
#include "../utils/utils.hpp"
#include "utils/debugdraw.hpp"

struct RaycastHit {
    bool hit;
    ivec3 pos;
    ivec3 normal;
};

inline RaycastHit raycast(Level* level, const vec3 &origin, const vec3 &dir, const float maxDistance) {
    RaycastHit result{};
    result.hit = false;

    ivec3 pos = {floori(origin.x), floori(origin.y), floori(origin.z)};

    const ivec3 step = {
        dir.x > 0 ? 1 : dir.x < 0 ? -1 : 0,
        dir.y > 0 ? 1 : dir.y < 0 ? -1 : 0,
        dir.z > 0 ? 1 : dir.z < 0 ? -1 : 0
    };

    const vec3 tDelta = {
        dir.x != 0 ? std::abs(1.0f / dir.x) : std::numeric_limits<float>::infinity(),
        dir.y != 0 ? std::abs(1.0f / dir.y) : std::numeric_limits<float>::infinity(),
        dir.z != 0 ? std::abs(1.0f / dir.z) : std::numeric_limits<float>::infinity()
    };

    // distance to first voxel boundary
    const vec3 nextVoxelBoundary = {
        step.x > 0 ? std::floor(origin.x) + 1.0f : std::floor(origin.x),
        step.y > 0 ? std::floor(origin.y) + 1.0f : std::floor(origin.y),
        step.z > 0 ? std::floor(origin.z) + 1.0f : std::floor(origin.z)
    };

    vec3 tMax = {
        dir.x != 0 ? (nextVoxelBoundary.x - origin.x) / dir.x : std::numeric_limits<float>::infinity(),
        dir.y != 0 ? (nextVoxelBoundary.y - origin.y) / dir.y : std::numeric_limits<float>::infinity(),
        dir.z != 0 ? (nextVoxelBoundary.z - origin.z) / dir.z : std::numeric_limits<float>::infinity()
    };

    float distance = 0.0f;
    ivec3 normal = {0, 0, 0};

    while (distance <= maxDistance) {
        debugDrawVoxel(pos);
        if (level->isVoxelSolid(pos)) {
            result.hit = true;
            result.pos = pos;
            result.normal = normal;
            return result;
        }

        if (tMax.x <= tMax.y && tMax.x <= tMax.z) {
            pos.x += step.x;
            distance = tMax.x;
            tMax.x += tDelta.x;
            normal = {-step.x, 0, 0};
        }
        else if (tMax.y <= tMax.z) {
            pos.y += step.y;
            distance = tMax.y;
            tMax.y += tDelta.y;
            normal = {0, -step.y, 0};
        }
        else {
            pos.z += step.z;
            distance = tMax.z;
            tMax.z += tDelta.z;
            normal = {0, 0, -step.z};
        }
    }

    return result;
}

#endif //VOXELGAME_RAYCAST_HPP