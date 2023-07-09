// Credit to Laurent NOËL (celeborn2bealive) for the gltf loader and the rendering 
// code and his tutorial at https://gltf-viewer-tutorial.gitlab.io/ along with the
// accompanying source code at https://gitlab.com/gltf-viewer-tutorial/gltf-viewer

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <filesystem>

#include "tiny_gltf.h"

namespace utilgltf
{
    // A range of indices in a vector containing Vertex Array Objects, replace with pair (firstIndex, nbrElements)?
    struct VAOrange
    {
        GLsizei begin; // Index of first element in vertexArrayObjects
        GLsizei count; // Number of elements in range
    };

    bool loadGLTFfile(std::filesystem::path filePath, tinygltf::Model& model);
    std::vector<GLuint> createTextureObjects(const tinygltf::Model& model);
    std::vector<GLuint> createVBOs(const tinygltf::Model& model);
    std::vector<GLuint> createVAOs(const tinygltf::Model &model, const std::vector<GLuint>& VBOs, std::vector<VAOrange>& meshToVertexArrays);
    glm::mat4 getLocalToWorldMatrix(const tinygltf::Node& node, const glm::mat4& parentMatrix);
    void computeSceneBounds(const tinygltf::Model &model, glm::vec3 &bboxMin, glm::vec3 &bboxMax);
}
