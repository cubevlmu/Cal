#pragma once

#include "globals.hpp"

namespace cal {

    struct Vec2;
    struct Vec3;
    struct DVec3;
    struct Matrix;

    struct IVec2 {
        IVec2() {}
        explicit IVec2(i32 i) : x(i), y(i) {}
        explicit IVec2(const Vec2& rhs);
        IVec2(i32 x, i32 y) : x(x), y(y) {}
        IVec2 operator +(const IVec2& v) const
        {
            return IVec2(x + v.x, y + v.y);
        }
        IVec2 operator -(const IVec2& v) const
        {
            return IVec2(x - v.x, y - v.y);
        }
        IVec2 operator *(i32 i) const
        {
            return IVec2(x * i, y * i);
        }
        IVec2 operator / (i32 rhs) const
        {
            return IVec2(x / rhs, y / rhs);
        }
        IVec2 operator / (const IVec2& rhs) const
        {
            return IVec2(x / rhs.x, y / rhs.y);
        }
        inline Vec2 operator / (const Vec2& rhs) const;
        bool operator !=(const IVec2& rhs) const
        {
            return rhs.x != x || rhs.y != y;
        }

        i32 x;
        i32 y;
    };


    struct IVec3 {
        IVec3() {}
        IVec3(i32 x, i32 y, i32 z)
            : x(x), y(y), z(z)
        {}
        explicit IVec3(const DVec3& rhs);
        explicit IVec3(const Vec3& rhs);


        IVec3 operator +(const IVec3& v) const
        {
            return IVec3(x + v.x, y + v.y, z + v.z);
        }
        bool operator==(const IVec3& rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }
        DVec3 operator *(double i) const;
        IVec2 xy() const
        {
            return { x, y };
        }

        i32 x;
        i32 y;
        i32 z;
    };


    struct IVec4 {
        IVec4() {}
        IVec4(i32 x, i32 y, i32 z, i32 w)
            : x(x), y(y), z(z), w(w)
        {}
        IVec4(i32 v)
            : x(v), y(v), z(v), w(v)
        {}
        IVec4(const IVec2& a, const IVec2& b)
            : x(a.x), y(a.y), z(b.x), w(b.y)
        {}

        void operator += (const IVec4& rhs);
        IVec2 xy() const
        {
            return { x,y };
        }
        IVec2 zw() const
        {
            return { z,w };
        }

        i32 x, y, z, w;
    };


    struct DVec2 {
        DVec2() {}
        DVec2(double x, double y)
            : x(x), y(y) {}
        DVec2 operator -(const DVec2& rhs) const
        {
            return { x - rhs.x, y - rhs.y };
        }
        DVec2 operator +(const DVec2& rhs) const
        {
            return { x + rhs.x, y + rhs.y };
        }
        DVec2 operator *(double rhs) const
        {
            return { x * rhs, y * rhs };
        }


        double x, y;
    };


    struct Vec2 {
        Vec2() {}

        explicit Vec2(const IVec2& rhs);
        explicit Vec2(const DVec2& rhs);
        explicit Vec2(float a);
        Vec2(float a, float b);

        float& operator[](u32 i);
        float operator[](u32 i) const;
        bool operator==(const Vec2& rhs) const;
        bool operator!=(const Vec2& rhs) const;
        void operator/=(float rhs);
        void operator*=(float f);

        Vec2 operator *(const Vec2& v) const;
        Vec2 operator /(const Vec2& v) const;
        Vec2 operator *(float f) const;
        Vec2 operator /(float f) const;
        Vec2 operator +(const Vec2& v) const;
        Vec2 operator +(float v) const;
        void operator +=(const Vec2& v);
        Vec2 operator -(const Vec2& v) const;
        Vec2 operator -(float f) const;
        Vec2 operator -() const;
        Vec2 ortho() const { return { y, -x }; }

        float x, y;

        static const Vec2 MAX;
        static const Vec2 MIN;
        static const Vec2 ZERO;
    };



    struct Vec3 {
        Vec3() {}
        Vec3(const Vec2& v, float c);
        Vec3(float a, float b, float c);
        explicit Vec3(float a);
        explicit Vec3(const DVec3& rhs);
        explicit Vec3(const IVec3& rhs);

        float& operator[](u32 i);
        float operator[](u32 i) const;
        bool operator==(const Vec3& rhs) const;
        bool operator!=(const Vec3& rhs) const;
        Vec3 operator+(const Vec3& rhs) const;
        Vec3 operator-() const;
        Vec3 operator-(const Vec3& rhs) const;
        void operator+=(const Vec3& rhs);
        void operator-=(const Vec3& rhs);
        Vec3 operator*(float s) const;
        Vec3 operator*(const Vec3& rhs) const;
        Vec3 operator*(const IVec3& rhs) const;
        Vec3 operator/(const Vec3& rhs) const;
        Vec3 operator/(const IVec3& rhs) const;
        Vec3 operator-(float s) const;
        Vec3 operator/(float s) const;
        void operator/=(float rhs);
        void operator*=(float rhs);

        Vec2 xz() const;
        Vec2 yz() const;
        Vec2 xy() const;

        union {
            struct {
                float x, y, z;
            };
            struct {
                float r, g, b;
            };
        };

        static const Vec3 MAX;
        static const Vec3 MIN;
        static const Vec3 ZERO;
    };


    struct DVec3
    {
        DVec3() {}
        DVec3(float a);
        DVec3(double x, double y, double z);
        DVec3(const DVec3& rhs) = default;
        explicit DVec3(const Vec3& rhs);

        DVec3 operator-() const;
        DVec3 operator*(float rhs) const;
        DVec3 operator*(const Vec3& rhs) const;
        DVec3 operator/(float rhs) const;
        DVec3 operator/(const Vec3& rhs) const;
        DVec3 operator/(const DVec3& rhs) const;
        DVec3 operator-(const DVec3& rhs) const;
        DVec3 operator+(const DVec3& rhs) const;
        DVec3 operator-(const Vec3& rhs) const;
        DVec3 operator+(const Vec3& rhs) const;
        void operator*=(const double& rhs);
        void operator/=(const double& rhs);
        void operator+=(const DVec3& rhs);
        void operator+=(const Vec3& rhs);
        void operator-=(const Vec3& rhs);
        DVec2 xz() const;

        union {
            struct {
                double x, y, z;
            };
            struct {
                double r, g, b;
            };
        };
    };


    struct Vec4 {
        Vec4() {}

        explicit Vec4(float a);

        Vec4(float a, float b, float c, float d);
        Vec4(const Vec2& v1, const Vec2& v2);
        Vec4(const Vec3& v, float d);

        Vec2 xz() const;
        Vec2 xy() const;
        Vec3 xyz() const;
        Vec2 yz() const;
        Vec3 rgb() const;

        float& operator[](u32 i);
        float operator[](u32 i) const;
        bool operator==(const Vec4& rhs) const;
        bool operator!=(const Vec4& rhs) const;
        Vec4 operator+(const Vec4& rhs) const;
        Vec4 operator-() const;
        Vec4 operator-(const Vec4& rhs) const;

        void operator+=(const Vec4& rhs);
        void operator-=(const Vec4& rhs);
        void operator/=(float rhs);
        Vec4 operator/(float s);
        Vec4 operator/(const Vec4& v);
        Vec4 operator*(float s);
        Vec4 operator*(const Vec4& v);
        void operator*=(float rhs);

        union {
            struct {
                float x, y, z, w;
            };
            struct {
                float r, g, b, a;
            };
        };

        static const Vec4 MAX;
        static const Vec4 MIN;
        static const Vec4 ZERO;
    };


 Vec3 operator *(float f, const Vec3& v);
 Vec4 operator *(float f, const Vec4& v);
 Vec4 operator*(const Vec4& v, float s);
}