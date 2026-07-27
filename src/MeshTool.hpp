//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHTOOL_HPP
#define VOXELGAME_MESHTOOL_HPP
#include <raylib.h>

#include "utils/utils.hpp"

class MeshTool {
private:
    std::vector<vec3> normals;
    std::vector<ivec3> vertices;
    std::vector<vec2> uvs;
    std::vector<glm::u8vec4> colors;
public:
    void clear() {
        vertices.clear();
        normals.clear();
        uvs.clear();
        colors.clear();
    }

    void reserve(const size_t quads) {
        vertices.reserve(quads * 6);
        normals.reserve(quads * 6);
        uvs.reserve(quads * 6);
        colors.reserve(quads * 6);
    }

    void addQuad(const ivec3 a, const ivec3 b, const ivec3 c, const ivec3 d, const vec2 uvA, const vec2 uvB, const vec2 uvC, const vec2 uvD, const vec3 normal, const glm::vec4 light) {
        const ivec3 verts[4] = {a, b, c, d};
        const vec2 texCoords[4] = {uvA, uvB, uvC, uvD};
        const uint8_t vertLight[4] = {
            static_cast<uint8_t>(light[0] * 255.0f),
            static_cast<uint8_t>(light[1] * 255.0f),
            static_cast<uint8_t>(light[2] * 255.0f),
            static_cast<uint8_t>(light[3] * 255.0f)
        };

        static constexpr int defaultIndices[6] = {0, 1, 2, 0, 2, 3};
        static constexpr int flippedIndices[6] = {1, 2, 3, 0, 1, 3};

        const int* indices = (light[0] + light[2] < light[1] + light[3]) ? flippedIndices : defaultIndices;

        for (int i = 0; i < 6; ++i) {
            const int idx = indices[i];
            normals.push_back(normal);
            vertices.push_back(verts[idx]);
            uvs.push_back(texCoords[idx]);
            colors.push_back(glm::u8vec4(vertLight[idx], vertLight[idx], vertLight[idx], 255));
        }
    }

    void exportToMesh(Mesh* mesh) const {
        mesh->vertexCount = vertices.size();
        mesh->vertices = static_cast<float*>(MemAlloc(vertices.size() * 3 * sizeof(float)));
        for (size_t i = 0; i < vertices.size(); i++) {
            mesh->vertices[i * 3 + 0] = vertices[i].x;
            mesh->vertices[i * 3 + 1] = vertices[i].y;
            mesh->vertices[i * 3 + 2] = vertices[i].z;
        }
        mesh->triangleCount = vertices.size() / 3;
        mesh->normals = static_cast<float*>(MemAlloc(normals.size() * 3 * sizeof(float)));
        for (size_t i = 0; i < normals.size(); i++) {
            mesh->normals[i * 3 + 0] = normals[i].x;
            mesh->normals[i * 3 + 1] = normals[i].y;
            mesh->normals[i * 3 + 2] = normals[i].z;
        }
        mesh->texcoords = static_cast<float*>(MemAlloc(uvs.size() * 2 * sizeof(float)));
        for (size_t i = 0; i < uvs.size(); i++) {
            mesh->texcoords[i * 2 + 0] = uvs[i].x;
            mesh->texcoords[i * 2 + 1] = uvs[i].y;
        }
        mesh->colors = static_cast<unsigned char*>(MemAlloc(colors.size() * 4 * sizeof(unsigned char)));
        for (size_t i = 0; i < colors.size(); i++) {
            mesh->colors[i * 4 + 0] = colors[i].r;
            mesh->colors[i * 4 + 1] = colors[i].g;
            mesh->colors[i * 4 + 2] = colors[i].b;
            mesh->colors[i * 4 + 3] = colors[i].a;
        }
    }
};

#endif //VOXELGAME_MESHTOOL_HPP