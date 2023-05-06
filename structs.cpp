#include "structs.h"

namespace structs {
    std::vector<Mesh> objlMeshToCustomMesh(const std::vector<objl::Mesh>& objlMeshes) 
    {
        std::vector<Mesh> customMeshes;

        for (const objl::Mesh& objlMesh : objlMeshes) 
        {
            std::vector<Vertex> customVertices;
            std::vector<unsigned int> customIndices;

            for (const objl::Vertex& objlVertex : objlMesh.Vertices) 
            {
                glm::vec3 position(objlVertex.Position.X, objlVertex.Position.Y, objlVertex.Position.Z);
                glm::vec3 normal(objlVertex.Normal.X, objlVertex.Normal.Y, objlVertex.Normal.Z);
                glm::vec2 textureCoordinate(objlVertex.TextureCoordinate.X, objlVertex.TextureCoordinate.Y);
                customVertices.emplace_back(position, normal, textureCoordinate);
            }

            customIndices = objlMesh.Indices;

            Mesh customMesh(customVertices, customIndices);
            customMesh.meshName = objlMesh.MeshName;

            const objl::Material& objlMat = objlMesh.MeshMaterial;
            Material customMaterial;
            customMaterial.name = objlMat.name;
            customMaterial.ka = glm::vec3(objlMat.Ka.X, objlMat.Ka.Y, objlMat.Ka.Z);
            customMaterial.kd = glm::vec3(objlMat.Kd.X, objlMat.Kd.Y, objlMat.Kd.Z);
            customMaterial.ks = glm::vec3(objlMat.Ks.X, objlMat.Ks.Y, objlMat.Ks.Z);
            customMaterial.ns = objlMat.Ns;
            customMaterial.ni = objlMat.Ni;
            customMaterial.d = objlMat.d;
            customMaterial.illum = objlMat.illum;
            customMaterial.kaMap = objlMat.map_Ka;
            customMaterial.kdMap = objlMat.map_Kd;
            customMaterial.ksMap = objlMat.map_Ks;
            customMaterial.nsMap = objlMat.map_Ns;
            customMaterial.dMap = objlMat.map_d;
            customMaterial.bumpMap = objlMat.map_bump;

            customMesh.meshMaterial = customMaterial;

            customMeshes.push_back(customMesh);
        }

        return customMeshes;
    }
}