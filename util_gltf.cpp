// Credit to Laurent NOËL (celeborn2bealive) for the gltf loader and the rendering 
// code and his tutorial at https://gltf-viewer-tutorial.gitlab.io/ along with the
// accompanying source code at https://gitlab.com/gltf-viewer-tutorial/gltf-viewer

#include <iostream>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

#include "util_gltf.h"

namespace utilgltf
{
    bool loadGLTFfile(std::filesystem::path filePath, tinygltf::Model& model)
    {
        tinygltf::TinyGLTF loader;
        std::string error;
        std::string warning;

        std::string strPath = filePath.make_preferred().string();
        const char* cStrPath = strPath.c_str();
        bool result = false;
        if (filePath.extension() == ".gltf")
        {
            std::cout << "Loading gltf file: " << filePath.filename().string().c_str() << std::endl;
            result = loader.LoadASCIIFromFile(&model, &error, &warning, cStrPath);
        }
        else if (filePath.extension() == ".glb")
        {
            std::cout << "Loading glb file: " << filePath.filename().string().c_str() << std::endl;
            result = loader.LoadBinaryFromFile(&model, &error, &warning, cStrPath);
        }

        if (!result)
        {
            std::cerr << "Failed to parse gltf file" << std::endl;
            return false;
        }
        if (!warning.empty())
        {
            std::cout << "Warning: tinygltf warning: " << warning << std::endl;
        }
        if (!error.empty())
        {
            std::cerr << "Failed to parse gltf file, tinygltf ERR: " << error << std::endl;
            return false;
        }

        return true;
    }

    std::vector<GLuint> createTextureObjects(const tinygltf::Model& model)
    {
        std::vector<GLuint> textureObjects(model.textures.size(), 0);

        // default sampler:
        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#texturesampler
        // "When undefined, a sampler with repeat wrapping and auto filtering should
        // be used."
        tinygltf::Sampler defaultSampler;
        defaultSampler.minFilter = GL_LINEAR;
        defaultSampler.magFilter = GL_LINEAR;
        defaultSampler.wrapS = GL_REPEAT;
        defaultSampler.wrapT = GL_REPEAT;
        // defaultSampler.wrapR = GL_REPEAT; tinygltf doesn't use wrapR

        glActiveTexture(GL_TEXTURE0);

        glGenTextures(GLsizei(model.textures.size()), textureObjects.data());
        for (size_t i = 0; i < model.textures.size(); ++i) 
        {
            const auto &texture = model.textures[i];
            assert(texture.source >= 0);
            const auto &image = model.images[texture.source];

            const auto &sampler =
                texture.sampler >= 0 ? model.samplers[texture.sampler] : defaultSampler;
            glBindTexture(GL_TEXTURE_2D, textureObjects[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0,
                GL_RGBA, image.pixel_type, image.image.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                sampler.minFilter != -1 ? sampler.minFilter : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                sampler.magFilter != -1 ? sampler.magFilter : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, sampler.wrapR); wrapR not used by tinygltf

            if (sampler.minFilter == GL_NEAREST_MIPMAP_NEAREST ||
                sampler.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
                sampler.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
                sampler.minFilter == GL_LINEAR_MIPMAP_LINEAR)
            {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        return textureObjects;
    }

    std::vector<GLuint> createVBOs(const tinygltf::Model& model)
    {
        std::vector<GLuint> VBOs(model.buffers.size(), 0);

        glGenBuffers(GLsizei(model.buffers.size()), VBOs.data());
        for (size_t i = 0; i < model.buffers.size(); ++i) 
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
            glBufferStorage(GL_ARRAY_BUFFER, model.buffers[i].data.size(),
                model.buffers[i].data.data(), 0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return VBOs;
    }

    std::vector<GLuint> createVAOs(const tinygltf::Model& model, const std::vector<GLuint>& VBOs, std::vector<VAOrange>& meshToVertexArrays)
    {
        std::vector<GLuint> VAOs; // We don't know the size yet

        // For each mesh of model we keep its range of VAOs
        meshToVertexArrays.resize(model.meshes.size());

        const GLuint VERTEX_ATTRIB_POSITION_IDX = 0;
        const GLuint VERTEX_ATTRIB_NORMAL_IDX = 1;
        const GLuint VERTEX_ATTRIB_TEXCOORD0_IDX = 2;

        for (size_t i = 0; i < model.meshes.size(); ++i) 
        {
            const auto &mesh = model.meshes[i];

            auto &VAOrange = meshToVertexArrays[i];
            VAOrange.begin =
                GLsizei(VAOs.size()); // Range for this mesh will be at
                                                    // the end of VAOs
            VAOrange.count =
                GLsizei(mesh.primitives.size()); // One VAO for each primitive

            // Add enough elements to store our VAOs identifiers
            VAOs.resize(
                VAOs.size() + mesh.primitives.size());

            glGenVertexArrays(VAOrange.count, &VAOs[VAOrange.begin]);
            for (size_t pIdx = 0; pIdx < mesh.primitives.size(); ++pIdx)
            {
                const auto vao = VAOs[VAOrange.begin + pIdx];
                const auto &primitive = mesh.primitives[pIdx];
                glBindVertexArray(vao);
                { // POSITION attribute
                    // scope, so we can declare const variable with the same name on each
                    // scope
                    const auto iterator = primitive.attributes.find("POSITION");
                    if (iterator != end(primitive.attributes)) {
                    const auto accessorIdx = (*iterator).second;
                    const auto &accessor = model.accessors[accessorIdx];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto bufferIdx = bufferView.buffer;

                    glEnableVertexAttribArray(VERTEX_ATTRIB_POSITION_IDX);
                    assert(GL_ARRAY_BUFFER == bufferView.target);
                    // Theorically we could also use bufferView.target, but it is safer
                    // Here it is important to know that the next call
                    // (glVertexAttribPointer) use what is currently bound
                    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIdx]);

                    // tinygltf converts strings type like "VEC3, "VEC2" to the number of
                    // components, stored in accessor.type
                    const auto byteOffset = accessor.byteOffset + bufferView.byteOffset;
                    glVertexAttribPointer(VERTEX_ATTRIB_POSITION_IDX, accessor.type,
                        accessor.componentType, GL_FALSE, GLsizei(bufferView.byteStride),
                        (const GLvoid *)byteOffset);
                    }
                }
                // todo Refactor to remove code duplication (loop over "POSITION",
                // "NORMAL" and their corresponding VERTEX_ATTRIB_*)
                { // NORMAL attribute
                    const auto iterator = primitive.attributes.find("NORMAL");
                    if (iterator != end(primitive.attributes)) {
                    const auto accessorIdx = (*iterator).second;
                    const auto &accessor = model.accessors[accessorIdx];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto bufferIdx = bufferView.buffer;

                    glEnableVertexAttribArray(VERTEX_ATTRIB_NORMAL_IDX);
                    assert(GL_ARRAY_BUFFER == bufferView.target);
                    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIdx]);
                    glVertexAttribPointer(VERTEX_ATTRIB_NORMAL_IDX, accessor.type,
                        accessor.componentType, GL_FALSE, GLsizei(bufferView.byteStride),
                        (const GLvoid *)(accessor.byteOffset + bufferView.byteOffset));
                    }
                }
                { // TEXCOORD_0 attribute
                    const auto iterator = primitive.attributes.find("TEXCOORD_0");
                    if (iterator != end(primitive.attributes)) {
                    const auto accessorIdx = (*iterator).second;
                    const auto &accessor = model.accessors[accessorIdx];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto bufferIdx = bufferView.buffer;

                    glEnableVertexAttribArray(VERTEX_ATTRIB_TEXCOORD0_IDX);
                    assert(GL_ARRAY_BUFFER == bufferView.target);
                    glBindBuffer(GL_ARRAY_BUFFER, VBOs[bufferIdx]);
                    glVertexAttribPointer(VERTEX_ATTRIB_TEXCOORD0_IDX, accessor.type,
                        accessor.componentType, GL_FALSE, GLsizei(bufferView.byteStride),
                        (const GLvoid *)(accessor.byteOffset + bufferView.byteOffset));
                    }
                }
                // Index array if defined
                if (primitive.indices >= 0) 
                {
                    const auto accessorIdx = primitive.indices;
                    const auto &accessor = model.accessors[accessorIdx];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto bufferIdx = bufferView.buffer;

                    assert(GL_ELEMENT_ARRAY_BUFFER == bufferView.target);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                        VBOs[bufferIdx]); // Binding the index buffer to
                                                // GL_ELEMENT_ARRAY_BUFFER while the VAO
                                                // is bound is enough to tell OpenGL we
                                                // want to use that index buffer for that
                                                // VAO
                }
            }
        }
        glBindVertexArray(0);

        std::clog << "Number of VAOs: " << VAOs.size() << std::endl;

        return VAOs;
    }

    glm::mat4 getLocalToWorldMatrix(const tinygltf::Node &node, const glm::mat4 &parentMatrix)
    {
        // Extract model matrix
        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#transformations
        if (!node.matrix.empty())
        {
            return parentMatrix * glm::mat4(node.matrix[0], node.matrix[1],
                                    node.matrix[2], node.matrix[3], node.matrix[4],
                                    node.matrix[5], node.matrix[6], node.matrix[7],
                                    node.matrix[8], node.matrix[9], node.matrix[10],
                                    node.matrix[11], node.matrix[12], node.matrix[13],
                                    node.matrix[14], node.matrix[15]);
        }
        const auto T = node.translation.empty()
                            ? parentMatrix
                            : glm::translate(parentMatrix,
                                glm::vec3(node.translation[0], node.translation[1],
                                    node.translation[2]));
        const auto rotationQuat =
            node.rotation.empty()
                ? glm::quat(1, 0, 0, 0)
                : glm::quat(float(node.rotation[3]), float(node.rotation[0]),
                        float(node.rotation[1]),
                        float(node.rotation[2])); // prototype is w, x, y, z
        const auto TR = T * glm::mat4_cast(rotationQuat);
        return node.scale.empty() ? TR
                                    : glm::scale(TR, glm::vec3(node.scale[0],
                                                        node.scale[1], node.scale[2]));
    }

    void computeSceneBounds(const tinygltf::Model &model, glm::vec3 &bboxMin, glm::vec3 &bboxMax)
    {
        // Compute scene bounding box
        // todo refactor with scene drawing
        // todo need a visitScene generic function that takes a accept() functor
        bboxMin = glm::vec3(std::numeric_limits<float>::max());
        bboxMax = glm::vec3(std::numeric_limits<float>::lowest());
        if (model.defaultScene >= 0) 
        {
            const std::function<void(int, const glm::mat4 &)> updateBounds = [&](int nodeIdx, const glm::mat4 &parentMatrix) 
            {
                const auto &node = model.nodes[nodeIdx];
                const glm::mat4 modelMatrix = getLocalToWorldMatrix(node, parentMatrix);
                if (node.mesh >= 0) 
                {
                    const auto &mesh = model.meshes[node.mesh];
                    for (size_t pIdx = 0; pIdx < mesh.primitives.size(); ++pIdx) 
                    {
                        const auto &primitive = mesh.primitives[pIdx];
                        const auto positionAttrIdxIt = primitive.attributes.find("POSITION");
                        if (positionAttrIdxIt == end(primitive.attributes)) 
                        {
                            continue;
                        }
                        const auto &positionAccessor = model.accessors[(*positionAttrIdxIt).second];
                        if (positionAccessor.type != 3) 
                        {
                            std::cerr << "Position accessor with type != VEC3, skipping" << std::endl;
                            continue;
                        }

                        const auto &positionBufferView = model.bufferViews[positionAccessor.bufferView];
                        const auto byteOffset = positionAccessor.byteOffset + positionBufferView.byteOffset;
                        const auto &positionBuffer = model.buffers[positionBufferView.buffer];
                        const auto positionByteStride = positionBufferView.byteStride ? positionBufferView.byteStride : 3 * sizeof(float);

                        if (primitive.indices >= 0) 
                        {
                            const auto &indexAccessor = model.accessors[primitive.indices];
                            const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
                            const auto indexByteOffset = indexAccessor.byteOffset + indexBufferView.byteOffset;
                            const auto &indexBuffer = model.buffers[indexBufferView.buffer];
                            auto indexByteStride = indexBufferView.byteStride;

                            switch (indexAccessor.componentType) 
                            {
                            default:
                            std::cerr
                                << "Primitive index accessor with bad componentType "
                                << indexAccessor.componentType << ", skipping it."
                                << std::endl;
                            continue;
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            indexByteStride =
                                indexByteStride ? indexByteStride : sizeof(uint8_t);
                            break;
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            indexByteStride =
                                indexByteStride ? indexByteStride : sizeof(uint16_t);
                            break;
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            indexByteStride =
                                indexByteStride ? indexByteStride : sizeof(uint32_t);
                            break;
                            }
                            for (size_t i = 0; i < indexAccessor.count; ++i) 
                            {
                                uint32_t index = 0;
                                switch (indexAccessor.componentType) 
                                {
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                                    index = *((const uint8_t *)&indexBuffer
                                                .data[indexByteOffset + indexByteStride * i]);
                                    break;
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                                    index = *((const uint16_t *)&indexBuffer
                                                .data[indexByteOffset + indexByteStride * i]);
                                    break;
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                                    index = *((const uint32_t *)&indexBuffer
                                                .data[indexByteOffset + indexByteStride * i]);
                                    break;
                                }
                                const auto &localPosition =
                                    *((const glm::vec3 *)&positionBuffer
                                            .data[byteOffset + positionByteStride * index]);
                                const auto worldPosition =
                                    glm::vec3(modelMatrix * glm::vec4(localPosition, 1.f));
                                bboxMin = glm::min(bboxMin, worldPosition);
                                bboxMax = glm::max(bboxMax, worldPosition);
                            }
                        } 
                        else 
                        {
                            for (size_t i = 0; i < positionAccessor.count; ++i) 
                            {
                                const auto &localPosition = *((const glm::vec3 *)&positionBuffer.data[byteOffset + positionByteStride * i]);
                                const auto worldPosition = glm::vec3(modelMatrix * glm::vec4(localPosition, 1.f));
                                bboxMin = glm::min(bboxMin, worldPosition);
                                bboxMax = glm::max(bboxMax, worldPosition);
                            }
                        }
                    }
                }

                for (const auto childNodeIdx : node.children)
                {
                    updateBounds(childNodeIdx, modelMatrix);
                }
            };

            for (const auto nodeIdx : model.scenes[model.defaultScene].nodes)
            {
                updateBounds(nodeIdx, glm::mat4(1));
            }
        }
    }
}