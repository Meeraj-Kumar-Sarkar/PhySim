#include "model/ObjLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>

namespace
{

    struct IndexTuple
    {
        int v = 0;
        int vt = 0;
        int vn = 0;
        bool operator==(const IndexTuple &o) const
        {
            return v == o.v && vt == o.vt && vn == o.vn;
        }
    };

    struct IndexTupleHash
    {
        std::size_t operator()(const IndexTuple &t) const noexcept
        {
            std::size_t h = 1469598103934665603ull;
            auto mix = [&](int x)
            {
                h ^= static_cast<std::size_t>(x);
                h *= 1099511628211ull;
            };
            mix(t.v);
            mix(t.vt);
            mix(t.vn);
            return h;
        }
    };

    // Parse "v", "v/vt", "v//vn", "v/vt/vn"
    IndexTuple parseFaceVertex(const std::string &token)
    {
        IndexTuple t;
        int field = 0;
        std::string num;
        auto flush = [&]()
        {
            if (!num.empty())
            {
                const int value = std::stoi(num);
                if (field == 0)
                    t.v = value;
                if (field == 1)
                    t.vt = value;
                if (field == 2)
                    t.vn = value;
            }
            field++;
            num.clear();
        };
        for (char c : token)
        {
            if (c == '/')
                flush();
            else
                num.push_back(c);
        }
        flush();
        return t;
    }

    // OBJ is 1-based. Negative indices are relative from the end.
    int resolve(int idx, int count)
    {
        if (idx > 0)
            return idx - 1;
        if (idx < 0)
            return count + idx;
        return -1;
    }

} // namespace

bool ObjLoader::load(const std::string &path, Mesh &outMesh)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "ObjLoader: cannot open " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::unordered_map<IndexTuple, uint32_t, IndexTupleHash> uniqueVerts;

    bool hadNormals = false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v")
        {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn")
        {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
            hadNormals = true;
        }
        else if (tag == "vt")
        {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (tag == "f")
        {
            std::vector<IndexTuple> face;
            std::string token;
            while (ss >> token)
                face.push_back(parseFaceVertex(token));
            if (face.size() < 3)
                continue;

            for (size_t i = 1; i + 1 < face.size(); ++i)
            {
                IndexTuple tri[3] = {face[0], face[i], face[i + 1]};
                for (const auto &t : tri)
                {
                    auto it = uniqueVerts.find(t);
                    if (it != uniqueVerts.end())
                    {
                        outMesh.indices.push_back(it->second);
                        continue;
                    }

                    Vertex vtx{};
                    int pi = resolve(t.v, (int)positions.size());
                    if (pi >= 0 && pi < (int)positions.size())
                        vtx.position = positions[pi];

                    int ni = resolve(t.vn, (int)normals.size());
                    if (ni >= 0 && ni < (int)normals.size())
                        vtx.normal = normals[ni];

                    int ti = resolve(t.vt, (int)uvs.size());
                    if (ti >= 0 && ti < (int)uvs.size())
                        vtx.uv = uvs[ti];

                    const uint32_t newIdx = (uint32_t)outMesh.vertices.size();
                    outMesh.vertices.push_back(vtx);
                    outMesh.indices.push_back(newIdx);
                    uniqueVerts.emplace(t, newIdx);
                }
            }
        }
        // mtllib, usemtl, o, g, s are ignored for now
    }

    if (!hadNormals)
        outMesh.computeNormals();
    outMesh.computeBounds();

    std::cout << "ObjLoader: " << path
              << "  (" << outMesh.vertices.size() << " verts, "
              << outMesh.indices.size() / 3 << " tris)\n";
    return true;
}