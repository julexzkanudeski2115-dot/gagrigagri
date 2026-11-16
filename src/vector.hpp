#pragma once
#include <cmath>

namespace adrenaline {

struct Vector2 {
    float x = 0.f;
    float y = 0.f;
};

struct Vector3 {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }

    float dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float distance(const Vector3& o) const { return (*this - o).length(); }
};

} // namespace adrenaline
