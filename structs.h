#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "obj_loader.h"

namespace structs 
{
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoordinate;

        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoordinate)
            : position(position), normal(normal), textureCoordinate(textureCoordinate) {}
    };

    struct Material 
    {
        Material() : ns(0.0f), ni(0.0f), d(0.0f), illum(0) {}

        std::string name;
        glm::vec3 ka;
        glm::vec3 kd;
        glm::vec3 ks;
        float ns;
        float ni;
        float d;
        int illum;
        std::string kaMap;
        std::string kdMap;
        std::string ksMap;
        std::string nsMap;
        std::string dMap;
        std::string bumpMap;
    };

    struct Mesh 
    {
        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
            : vertices(vertices), indices(indices) {}

        std::string meshName;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Material meshMaterial;
    };

    std::vector<Mesh> objlMeshToCustomMesh(const std::vector<objl::Mesh>& objlMeshes);
};
