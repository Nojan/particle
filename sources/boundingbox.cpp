#pragma once

#include "boundingbox.hpp"

#include <cassert>

BoundingBox3D::BoundingBox3D() 
: mMin(FLT_MAX)
, mMax(-FLT_MAX)
{}

BoundingBox3D::BoundingBox3D(const glm::vec3& min, const glm::vec3& max)
: mMin(min)
, mMax(max)
{
    assert(Valid());
}

glm::vec3 BoundingBox3D::Min() const
{
    assert(Valid());
    return mMin;
}

glm::vec3 BoundingBox3D::Max() const
{
    assert(Valid());
    return mMax;
}

glm::vec3 BoundingBox3D::Center() const
{
    assert(Valid());
    return mMin + Extent() * 0.5f;
}

glm::vec3 BoundingBox3D::Extent() const
{
    assert(Valid());
    return mMax - mMin;
}

bool BoundingBox3D::Valid() const
{
    bool res = true;
    res = res && mMin.x <= mMax.x;
    res = res && mMin.y <= mMax.y;
    res = res && mMin.z <= mMax.z;
    return res;
}

void BoundingBox3D::Add(const glm::vec3& point)
{
    if (!Valid())
    {
        mMin = point;
        mMax = point;
    }
    else 
    {
        for (int i = 0; i < 3; ++i) {
            if (mMin[i] > point[i])
                mMin[i] = point[i];
            if (mMax[i] < point[i])
                mMax[i] = point[i];
        }
    }
}

