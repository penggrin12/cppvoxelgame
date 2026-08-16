//
// Created by penggrin on 16.03.2026.
//

#ifndef VOXELGAME_VOXELDATA_HPP
#define VOXELGAME_VOXELDATA_HPP

#include "utils/utils.hpp"

namespace Voxel {
    enum Id : uint8_t {
#define VOXEL(name, id, ...) name = id,
#include "../voxels.def"

#undef VOXEL
        MAX
    };

    constexpr bool isSolid(const Id id) {
        return id != AIR && id != TALL_GRASS;
    }

    constexpr bool isRenderable(const Id id) {
        return id > AIR;
    }
}

constexpr ivec2 VOXEL_ATLAS_OFFSETS[Voxel::MAX][6] = {
#define VOXEL(_, __, ...) __VA_ARGS__,
#include "../voxels.def"

#undef VOXEL
};

#endif //VOXELGAME_VOXELDATA_HPP