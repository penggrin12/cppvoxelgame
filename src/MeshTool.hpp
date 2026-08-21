//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHTOOL_HPP
#define VOXELGAME_MESHTOOL_HPP

#include <raylib.h>

class MeshTool {
private:
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::u8vec4> colors;

    int vertexCount = 0;
public:
    inline constexpr MeshTool() {
        constexpr auto size = CHUNK_SIZE * LEVEL_HEIGHT * CHUNK_SIZE * 6 * 6;
        vertices.resize(size);
        normals.resize(size);
        uvs.resize(size);
        colors.resize(size);
    }

    inline void clear() {
        vertexCount = 0;
    }

    inline void addQuad(const glm::vec3 a, const glm::vec3 b, const glm::vec3 c, const glm::vec3 d, const glm::vec2 uvA, const glm::vec2 uvB, const glm::vec2 uvC, const glm::vec2 uvD, const vec3 normal, const glm::vec4 light) {
        const glm::vec3 verts[4] = {a, b, c, d};
        const glm::vec2 texCoords[4] = {uvA, uvB, uvC, uvD};
        const uint8_t vertLight[4] = {
            static_cast<uint8_t>(light[0] * 255.0f),
            static_cast<uint8_t>(light[1] * 255.0f),
            static_cast<uint8_t>(light[2] * 255.0f),
            static_cast<uint8_t>(light[3] * 255.0f)
        };

        static constexpr int defaultIndices[6] = {0, 1, 2, 0, 2, 3};
        static constexpr int flippedIndices[6] = {1, 2, 3, 0, 1, 3};

        const int *indices = (light[0] + light[2] < light[1] + light[3]) ? flippedIndices : defaultIndices;

        for (int i = 0; i < 6; ++i) {
            const int idx = indices[i];
            normals[vertexCount] = normal;
            vertices[vertexCount] = verts[idx];
            uvs[vertexCount] = texCoords[idx];
            colors[vertexCount] = glm::u8vec4{vertLight[idx], vertLight[idx], vertLight[idx], 255};

            vertexCount++;
        }
    }

    inline void exportToMesh(Mesh* mesh) const {
        mesh->vertexCount = vertexCount;
        mesh->vertices = static_cast<float*>(MemAlloc(vertexCount * 3 * sizeof(float)));
        for (size_t i = 0; i < vertexCount; i++) {
            mesh->vertices[i * 3 + 0] = vertices[i].x;
            mesh->vertices[i * 3 + 1] = vertices[i].y;
            mesh->vertices[i * 3 + 2] = vertices[i].z;
        }
        mesh->triangleCount = vertexCount / 3;
        mesh->normals = static_cast<float*>(MemAlloc(vertexCount * 3 * sizeof(float)));
        for (size_t i = 0; i < vertexCount; i++) {
            mesh->normals[i * 3 + 0] = normals[i].x;
            mesh->normals[i * 3 + 1] = normals[i].y;
            mesh->normals[i * 3 + 2] = normals[i].z;
        }
        mesh->texcoords = static_cast<float*>(MemAlloc(vertexCount * 2 * sizeof(float)));
        for (size_t i = 0; i < vertexCount; i++) {
            mesh->texcoords[i * 2 + 0] = uvs[i].x;
            mesh->texcoords[i * 2 + 1] = uvs[i].y;
        }
        mesh->colors = static_cast<unsigned char*>(MemAlloc(vertexCount * 4 * sizeof(unsigned char)));
        for (size_t i = 0; i < vertexCount; i++) {
            mesh->colors[i * 4 + 0] = colors[i].r;
            mesh->colors[i * 4 + 1] = colors[i].g;
            mesh->colors[i * 4 + 2] = colors[i].b;
            mesh->colors[i * 4 + 3] = colors[i].a;
        }
    }
};

#endif //VOXELGAME_MESHTOOL_HPP