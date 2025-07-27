#pragma once
#include <cmath>
#include <algorithm>

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


// Funciones auxiliares simplificadas
class MMaths {
public:
    static Vector3 RotateVectorByQuaternion(const Vector3& v, const Quaternion& q) {
        Vector3 qvec = Vector3(q.x, q.y, q.z);
        Vector3 cross1 = CrossProduct(qvec, v);
        Vector3 cross2 = CrossProduct(qvec, cross1 + v * q.w);
        return v + cross2 * 2.0f;
    }

    static Vector3 GetUprightTorque(const Quaternion& rotation) {
        // Calcular torque para enderezar el objeto (orientar hacia arriba)
        Vector3 currentUp = RotateVectorByQuaternion(Vector3(0.0f, 1.0f, 0.0f), rotation);
        Vector3 desiredUp = Vector3(0.0f, 1.0f, 0.0f);

        Vector3 axis = CrossProduct(currentUp, desiredUp);
        float angle = acos(std::max(-1.0f, std::min(1.0f, DotProduct(currentUp, desiredUp))));

        return axis * angle;
    }

    static Vector3 CrossProduct(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static float DotProduct(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static float Length(const Vector3& v) {
        return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    static Vector3 GetRotatedBoundingBox(const Vector3& originalSize, const Quaternion& rotation) {

        Vector3 vertices[8] = {
            Vector3(-originalSize.x, -originalSize.y, -originalSize.z),
            Vector3(originalSize.x, -originalSize.y, -originalSize.z),
            Vector3(-originalSize.x,  originalSize.y, -originalSize.z),
            Vector3(originalSize.x,  originalSize.y, -originalSize.z),
            Vector3(-originalSize.x, -originalSize.y,  originalSize.z),
            Vector3(originalSize.x, -originalSize.y,  originalSize.z),
            Vector3(-originalSize.x,  originalSize.y,  originalSize.z),
            Vector3(originalSize.x,  originalSize.y,  originalSize.z)
        };

        Vector3 minBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        Vector3 maxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (int i = 0; i < 8; i++) {
            Vector3 rotatedVertex = RotateVectorByQuaternion(vertices[i], rotation);

            minBounds.x = std::min(minBounds.x, rotatedVertex.x);
            minBounds.y = std::min(minBounds.y, rotatedVertex.y);
            minBounds.z = std::min(minBounds.z, rotatedVertex.z);

            maxBounds.x = std::max(maxBounds.x, rotatedVertex.x);
            maxBounds.y = std::max(maxBounds.y, rotatedVertex.y);
            maxBounds.z = std::max(maxBounds.z, rotatedVertex.z);
        }

        return Vector3(
            (maxBounds.x - minBounds.x) * 0.5f,
            (maxBounds.y - minBounds.y) * 0.5f,
            (maxBounds.z - minBounds.z) * 0.5f
        );
    }
};