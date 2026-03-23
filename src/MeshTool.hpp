//
// Created by penggrin on 15.03.2026.
//

#ifndef VOXELGAME_MESHTOOL_HPP
#define VOXELGAME_MESHTOOL_HPP
#include <raylib-cpp.hpp>

#include "utils/utils.hpp"

class MeshTool {
private:
    std::vector<vec3> normals;
    std::vector<ivec3> vertices;
    std::vector<vec2> uvs;
public:
    void clear() {
        vertices.clear();
        normals.clear();
        uvs.clear();
    }

    void reserve(const size_t quads) {
        vertices.reserve(quads * 6);
        normals.reserve(quads * 6);
        uvs.reserve(quads * 6);
    }

    void addQuad(const ivec3 a, const ivec3 b, const ivec3 c, const ivec3 d, const vec2 uvA, const vec2 uvB, const vec2 uvC, const vec2 uvD, const vec3 normal) {
        for (int i = 0; i < 6; ++i)
            normals.push_back(normal);

        vertices.push_back(a); vertices.push_back(b); vertices.push_back(c);
        vertices.push_back(a); vertices.push_back(c); vertices.push_back(d);

        uvs.push_back(uvA); uvs.push_back(uvB); uvs.push_back(uvC);
        uvs.push_back(uvA); uvs.push_back(uvC); uvs.push_back(uvD);
    }

    void exportToMesh(raylib::Mesh* mesh) const {
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
    }
};

#endif //VOXELGAME_MESHTOOL_HPP