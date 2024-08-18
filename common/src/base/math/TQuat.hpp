#pragma once

#include <globals.hpp>

namespace cal {

    struct Vec3;
    struct DVec3;
    struct Matrix;

    struct Quat
    {
        Quat() {}
        Quat(const Vec3& axis, float angle);
        Quat(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

        void fromEuler(const Vec3& euler);
        Vec3 toEuler() const;
        float toYaw() const;
        void set(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
        void conjugate();
        Quat conjugated() const;
        Matrix toMatrix() const;

        Vec3 rotate(const Vec3& v) const;
        DVec3 rotate(const DVec3& v) const;
        Quat operator*(const Quat& q) const;
        Quat operator-() const;
        Quat operator+(const Quat& q) const;
        Quat operator*(float m) const;
        Vec3 operator*(const Vec3& q) const;

        static Quat vec3ToVec3(const Vec3& a, const Vec3& b);

        float x, y, z, w;

        static const Quat IDENTITY;
    };

    struct DualQuat {
        Quat r;
        Quat d;
    };

}