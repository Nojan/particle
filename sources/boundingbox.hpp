#pragma once

#include <glm/glm.hpp>

class BoundingBox3D {
public:
    BoundingBox3D();
    BoundingBox3D(const glm::vec3& min, const glm::vec3& max);

    glm::vec3 Min() const;
    glm::vec3 Max() const;
    glm::vec3 Center() const;
    glm::vec3 Extent() const;
    bool Valid() const;

    void Add(const glm::vec3& point);

private:
    glm::vec3 mMin;
    glm::vec3 mMax;
};
