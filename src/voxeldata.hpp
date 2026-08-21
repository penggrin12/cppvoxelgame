//
// Created by penggrin on 16.03.2026.
//

#ifndef VOXELGAME_VOXELDATA_HPP
#define VOXELGAME_VOXELDATA_HPP

#include "utils/utils.hpp"

namespace Voxel {
    enum Type {
        FULL,
        VEGETATION,
        TYPE_MAX
    };

    enum Id : uint8_t {
#define VOXEL(name, id, ...) name = id,
#include "../voxels.def"

#undef VOXEL
        ID_MAX
    };

    constexpr bool isSolid(const Id id) {
        return id != AIR && id != TALL_GRASS;
    }

    constexpr bool isRenderable(const Id id) {
        return id > AIR;
    }
}

constexpr ivec2 VOXEL_ATLAS_OFFSETS[Voxel::Id::ID_MAX][6] = {
#define VOXEL(_, __, ___, ...) __VA_ARGS__,
#include "../voxels.def"
#undef VOXEL
};

constexpr Voxel::Type VOXEL_TYPES[Voxel::Id::ID_MAX] = {
#define VOXEL(_, __, type, ...) Voxel::Type::type,
#include "../voxels.def"
#undef VOXEL
};

#endif //VOXELGAME_VOXELDATA_HPP