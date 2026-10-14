#include "model/Ray.h"

#include <algorithm>
#include <limits>

bool rayAabb(const Ray &ray, const glm::vec3 &minB, const glm::vec3 &maxB, float &tHit)
{
    float tmin = -std::numeric_limits<float>::infinity();
    float tmax = std::numeric_limits<float>::infinity();

    for (int i = 0; i < 3; i++)
    {
        if (std::abs(ray.dir[i] < 1e-8f))
        {
            if (ray.origin[i] < minB[i] || ray.origin[i] > maxB[i])
                return false;
        }
        else
        {
            const float inv = 1.0f / ray.dir[i];
            float t1 = (minB[i] - ray.origin[i]) * inv;
            float t2 = (maxB[i] - ray.origin[i]) * inv;
            if (t1 > t2)
                std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax)
                return false;
        }
    }
    tHit = (tmin >= 0.0f) ? tmin : tmax;
    return tHit >= 0.0f;
}