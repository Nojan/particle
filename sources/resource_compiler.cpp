#include "resource_compiler.hpp"

#include <cassert>

namespace resource_compiler {

void convertToMatrix(const tinyxml2::XMLElement& element, glm::mat4& mat)
{
    assert(0 == strcmp(element.Value(), "Matrix4"));
    const char* matrix_text = element.GetText();
    float matrix[16];
    sscanf(matrix_text, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &matrix[0], &matrix[1], &matrix[2], &matrix[3], &matrix[4], &matrix[5], &matrix[6], &matrix[7], &matrix[8], &matrix[9], &matrix[10], &matrix[11], &matrix[12], &matrix[13], &matrix[14], &matrix[15]);
    //mat = glm::mat4(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
    mat = glm::mat4(matrix[0], matrix[4], matrix[8], matrix[12], matrix[1], matrix[5], matrix[9], matrix[13], matrix[2], matrix[6], matrix[10], matrix[14], matrix[3], matrix[7], matrix[11], matrix[15]);
}

void GetVertex (const tinyxml2::XMLElement* meshElement, std::vector<glm::vec3>& vertexArray) {
    assert(0 == strcmp(meshElement->Value(), "Mesh"));
    const tinyxml2::XMLElement* vertexElement = meshElement->FirstChildElement("Positions");
    const int vertexCount = vertexElement->IntAttribute("num");
    assert(vertexArray.empty());
    vertexArray.resize(vertexCount);
    static const int bufferSize = 128;
    char buffer[bufferSize];
    const char* v = vertexElement->GetText();
    for(int vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
    {
        glm::vec3& vertex = vertexArray[vertexIdx];
        for (int comp = 0; comp < 3; ++comp)
        {
            int bufferIdx;
            while (isspace(*v))
            {
                ++v;
            }
            for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
            {
                if (isspace(*v))
                {
                    buffer[bufferIdx] = '\0';
                    break;
                }
                buffer[bufferIdx] = *v;
                ++v;
            }
            assert(bufferIdx < bufferSize);
            vertex[comp] = static_cast<float>(atof(buffer));
        }
    }
}

void GetNormal(const tinyxml2::XMLElement* meshElement, std::vector<glm::vec3>& normalArray) {
    assert(0 == strcmp(meshElement->Value(), "Mesh"));
    const tinyxml2::XMLElement* normalsElement = meshElement->FirstChildElement("Normals");
    const int vertexCount = normalsElement->IntAttribute("num");
    normalArray.resize(vertexCount);
    {
        static const int bufferSize = 128;
        char buffer[bufferSize];
        const char* v = normalsElement->GetText();
        for (int vertexIdx = 0; vertexIdx <vertexCount; ++vertexIdx)
        {
            glm::vec3& normal = normalArray[vertexIdx];
            for (int comp = 0; comp < 3; ++comp)
            {
                int bufferIdx;
                while (isspace(*v))
                {
                    ++v;
                }
                for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
                {
                    if (isspace(*v))
                    {
                        buffer[bufferIdx] = '\0';
                        break;
                    }
                    buffer[bufferIdx] = *v;
                    ++v;
                }
                assert(bufferIdx < bufferSize);
                normal[comp] = atof(buffer);
            }
            assert(glm::abs(glm::length(normal) - 1.f) < 0.05f);
        }
    }
}

void GetUV(const tinyxml2::XMLElement* meshElement, std::vector<glm::vec2>& uvArray) {
    assert(0 == strcmp(meshElement->Value(), "Mesh"));
    const tinyxml2::XMLElement* textureCoordElement = meshElement->FirstChildElement("TextureCoords");
    const int vertexCount = textureCoordElement->IntAttribute("num");
    uvArray.resize(vertexCount);
    {
        static const int bufferSize = 128;
        char buffer[bufferSize];
        const char* v = textureCoordElement->GetText();
        for (int vertexIdx = 0; vertexIdx <vertexCount; ++vertexIdx)
        {
            glm::vec2& uv = uvArray[vertexIdx];
            for (int comp = 0; comp < 2; ++comp)
            {
                int bufferIdx;
                while (isspace(*v))
                {
                    ++v;
                }
                for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
                {
                    if (isspace(*v))
                    {
                        buffer[bufferIdx] = '\0';
                        break;
                    }
                    buffer[bufferIdx] = *v;
                    ++v;
                }
                assert(bufferIdx < bufferSize);
                uv[comp] = atof(buffer);
            }
        }
    }
}

void GetFace(const tinyxml2::XMLElement* meshElement, std::vector<uint>& faceArray) {
    assert(0 == strcmp(meshElement->Value(), "Mesh"));
    const tinyxml2::XMLElement* faceListElement = meshElement->FirstChildElement("FaceList");
    const int vertexPerFace = 3;
    faceArray.reserve(faceListElement->IntAttribute("num")*vertexPerFace);
    for (const tinyxml2::XMLElement* faceElement = faceListElement->FirstChildElement(); faceElement != nullptr; faceElement = faceElement->NextSiblingElement())
    {
        assert(vertexPerFace == faceElement->IntAttribute("num"));
        glm::ivec3 face;
        sscanf(faceElement->GetText(),"%d %d %d", &face[0], &face[1], &face[2]);
        for (int vertexPerFaceIdx = 0; vertexPerFaceIdx < vertexPerFace; ++vertexPerFaceIdx)
        {
            faceArray.push_back(face[vertexPerFaceIdx]);
        }
    }
}

} //ressource_compiler