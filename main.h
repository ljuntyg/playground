#ifndef MAIN_H
#define MAIN_H

#include "structs.h"
#include "obj_loader.h"
#include "renderer.h"

class Main {
public:
    static std::vector<std::string> getObjFiles(const std::string& folderName);
    static std::vector<Mesh> objlMeshToCustomMesh(const std::vector<objl::Mesh>& objlMeshes);
};

#endif // MAIN_H