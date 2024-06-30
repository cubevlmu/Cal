#pragma once

#include "base/math/TQuat.hpp"
#include "base/math/TVector.hpp"
#include <math.h>
#include <algorithm>

namespace cal {

    constexpr float PI = 3.14159265f;
    constexpr float HALF_PI = 3.14159265f * 0.5f;
    constexpr float SQRT2 = 1.41421356237f;
    constexpr float SQRT3 = 1.73205080757f;


    template <typename T> inline void swap(T& a, T& b) {
        T tmp = static_cast<T&&>(a);
        a = static_cast<T&&>(b);
        b = static_cast<T&&>(tmp);
    }

    template <typename T> inline T minimum(T a) {
        return a;
    }

    template <typename T> inline T minimum(T a, T b) {
        return a < b ? a : b;
    }

    inline Vec2 minimum(const Vec2& a, const Vec2& b) {
        return {
            minimum(a.x, b.x),
            minimum(a.y, b.y),
        };
    }

    inline DVec3 minimum(const DVec3& a, const DVec3& b) {
        return {
            minimum(a.x, b.x),
            minimum(a.y, b.y),
            minimum(a.z, b.z)
        };
    }

    inline Vec3 minimum(const Vec3& a, const Vec3& b) {
        return {
            minimum(a.x, b.x),
            minimum(a.y, b.y),
            minimum(a.z, b.z)
        };
    }

    inline Vec4 minimum(const Vec4& a, const Vec4& b) {
        return {
            minimum(a.x, b.x),
            minimum(a.y, b.y),
            minimum(a.z, b.z),
            minimum(a.w, b.w)
        };
    }

    template <typename T1, typename... T2> inline T1 minimum(T1 a, T2... b) {
        T1 min_b = minimum(b...);
        return minimum(a, min_b);
    }

    template <typename T> inline T maximum(T a) {
        return a;
    }

    template <typename T1, typename... T2> inline T1 maximum(T1 a, T2... b) {
        T1 min_b = maximum(b...);
        return a > min_b ? a : min_b;
    }

    inline Vec2 maximum(const Vec2& a, const Vec2& b) {
        return {
            maximum(a.x, b.x),
            maximum(a.y, b.y),
        };
    }

    inline IVec2 maximum(const IVec2& a, const IVec2& b) {
        return {
            maximum(a.x, b.x),
            maximum(a.y, b.y),
        };
    }

    inline DVec3 maximum(const DVec3& a, const DVec3& b) {
        return {
            maximum(a.x, b.x),
            maximum(a.y, b.y),
            maximum(a.z, b.z)
        };
    }

    inline Vec3 maximum(const Vec3& a, const Vec3& b) {
        return {
            maximum(a.x, b.x),
            maximum(a.y, b.y),
            maximum(a.z, b.z)
        };
    }

    inline Vec4 maximum(const Vec4& a, const Vec4& b) {
        return {
            maximum(a.x, b.x),
            maximum(a.y, b.y),
            maximum(a.z, b.z),
            maximum(a.w, b.w)
        };
    }

    template <typename T> inline T signum(T a) {
        return a > 0 ? (T)1 : (a < 0 ? (T)-1 : 0);
    }

    template <typename T1, typename T2, typename T3> inline T1 clamp(T1 value, T2 min_value, T3 max_value) {
        return minimum(maximum(value, min_value), max_value);
    }


    template <typename T> bool isPowOfTwo(T n)
    {
        return (n) && !(n & (n - 1));
    }

    float dot(const Vec4& op1, const Vec4& op2);
    float dot(const Vec3& op1, const Vec3& op2);
    float dot(const Vec2& op1, const Vec2& op2);
    Vec3 cross(const Vec3& op1, const Vec3& op2);
    DVec3 cross(const DVec3& op1, const DVec3& op2);

    Vec4 lerp(const Vec4& op1, const Vec4& op2, float t);
    float lerp(float a, float b, float t);
    Vec3 lerp(const Vec3& op1, const Vec3& op2, float t);
    DVec3 lerp(const DVec3& op1, const DVec3& op2, float t);
    Vec2 lerp(const Vec2& op1, const Vec2& op2, float t);
    Vec3 slerp(const Vec3& a, const Vec3& b, float t);
    Quat nlerp(const Quat& q1, const Quat& q2, float t);

    u32 nextPow2(u32 v);
    u32 log2(u32 v);
    float degreesToRadians(float angle);
    double degreesToRadians(double angle);
    float degreesToRadians(int angle);
    float radiansToDegrees(float angle);
    Vec2 radiansToDegrees(const Vec2& v);
    Vec3 radiansToDegrees(const Vec3& v);
    Vec2 degreesToRadians(const Vec2& v);
    Vec3 degreesToRadians(const Vec3& v);
    float easeInOut(float t);
    float angleDiff(float a, float b);
    u64 randGUID();
    u32 rand();
    u32 rand(u32 from, u32 to);
    float randFloat();
    float randFloat(float from, float to);
    DVec2 normalize(const DVec2& value);
    Vec2 normalize(const Vec2& value);
    Vec3 normalize(const Vec3& value);
    Quat normalize(const Quat& value);
    float length(const Vec2& value);
    float length(const Vec3& value);
    double length(const DVec3& value);
    float squaredLength(const Vec2& value);
    float squaredLength(const Vec3& value);
    double squaredLength(const DVec2& value);
    double squaredLength(const DVec3& value);
    float halton(u32 index, i32 base);


    struct RandomGenerator {
        RandomGenerator(u32 u = 521288629, u32 v = 362436069);
        u32 rand();
        float randFloat(float from, float to);
        float randFloat();
    private:
        u32 u;
        u32 v;
    };
}