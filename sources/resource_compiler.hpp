#pragma once

#include "types.hpp"
#include "tinyxml/tinyxml2.h"

#include <glm/glm.hpp>
#include <vector>

namespace resource_compiler {

void convertToMatrix(const tinyxml2::XMLElement& element, glm::mat4& mat);
void GetVertex(const tinyxml2::XMLElement* meshElement, std::vector<glm::vec3>& vertexArray);
void GetNormal(const tinyxml2::XMLElement* meshElement, std::vector<glm::vec3>& normalArray);
void GetUV(const tinyxml2::XMLElement* meshElement, std::vector<glm::vec2>& uvArray);
void GetFace(const tinyxml2::XMLElement* meshElement, std::vector<uint>& faceArray);
}