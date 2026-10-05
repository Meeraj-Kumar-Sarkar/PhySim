#pragma once

#include <model/Mesh.h>
#include <string>

class ObjLoader
{
public:
    static bool load(const std::string &path, Mesh &outMesh);
};