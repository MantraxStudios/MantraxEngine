#pragma once
#include <cmath>

// =====================
// VECTOR 2
// =====================
struct Vector2 {
    float x, y;

    Vector2(float x = 0, float y = 0): x(x), y(y) {}

    // Operadores básicos
    Vector2 operator+(const Vector2& v) const { return {x+v.x, y+v.y}; }
    Vector2 operator-(const Vector2& v) const { return {x-v.x, y-v.y}; }
    Vector2 operator*(float s) const { return {x*s, y*s}; }
    Vector2 operator/(float s) const { return {x/s, y/s}; }
    Vector2 operator*(const Vector2& v) const { return {x*v.x, y*v.y}; } // componente a componente

    Vector2& operator+=(const Vector2& v){ x+=v.x; y+=v.y; return *this; }
    Vector2& operator-=(const Vector2& v){ x-=v.x; y-=v.y; return *this; }
    Vector2& operator*=(float s){ x*=s; y*=s; return *this; }
    Vector2& operator/=(float s){ x/=s; y/=s; return *this; }

    float dot(const Vector2& v) const { return x*v.x + y*v.y; }
    float length() const { return std::sqrt(x*x + y*y); }
    Vector2 normalized() const {
        float len = length();
        return len > 0 ? (*this)*(1.0f/len) : Vector2();
    }
};

// Multiplicación float * Vector2
inline Vector2 operator*(float s, const Vector2& v) { return v * s; }

// =====================
// VECTOR 3
// =====================
struct Vector3 {
    float x, y, z;

    Vector3(float x = 0, float y = 0, float z = 0): x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    Vector3 operator*(float s) const { return {x*s, y*s, z*s}; }
    Vector3 operator/(float s) const { return {x/s, y/s, z/s}; }
    Vector3 operator*(const Vector3& v) const { return {x*v.x, y*v.y, z*v.z}; }

    Vector3& operator+=(const Vector3& v){ x+=v.x; y+=v.y; z+=v.z; return *this; }
    Vector3& operator-=(const Vector3& v){ x-=v.x; y-=v.y; z-=v.z; return *this; }
    Vector3& operator*=(float s){ x*=s; y*=s; z*=s; return *this; }
    Vector3& operator/=(float s){ x/=s; y/=s; z/=s; return *this; }

    float dot(const Vector3& v) const { return x*v.x + y*v.y + z*v.z; }

    Vector3 cross(const Vector3& v) const {
        return {
            y*v.z - z*v.y,
            z*v.x - x*v.z,
            x*v.y - y*v.x
        };
    }

    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vector3 normalized() const {
        float len = length();
        return len > 0 ? (*this)*(1.0f/len) : Vector3();
    }
};

inline Vector3 operator*(float s, const Vector3& v) { return v * s; }

// =====================
// QUATERNION
// =====================
struct Quaternion {
    float w, x, y, z;

    Quaternion(float w=1, float x=0, float y=0, float z=0)
        : w(w), x(x), y(y), z(z) {}

    static Quaternion fromAxisAngle(const Vector3& axis, float angleRad) {
        float half = angleRad * 0.5f;
        float s = std::sin(half);
        return { std::cos(half), axis.x*s, axis.y*s, axis.z*s };
    }

    Quaternion normalized() const {
        float len = std::sqrt(w*w + x*x + y*y + z*z);
        return {w/len, x/len, y/len, z/len};
    }

    Quaternion operator*(const Quaternion& q) const {
        return {
            w*q.w - x*q.x - y*q.y - z*q.z,
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w
        };
    }

    Vector3 rotate(const Vector3& v) const {
        Quaternion qv(0, v.x, v.y, v.z);
        Quaternion inv(w, -x, -y, -z);
        Quaternion res = (*this) * qv * inv;
        return {res.x, res.y, res.z};
    }
};
