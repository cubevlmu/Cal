#include "TVector.hpp"

#include "Math.hpp"
#include <cfloat>

namespace cal {

    const Vec2 Vec2::MAX = Vec2(FLT_MAX);
    const Vec2 Vec2::MIN = Vec2(-FLT_MAX);
    const Vec2 Vec2::ZERO = Vec2(0);

    const Vec3 Vec3::MAX = Vec3(FLT_MAX);
    const Vec3 Vec3::MIN = Vec3(-FLT_MAX);
    const Vec3 Vec3::ZERO = Vec3(0);

    const Vec4 Vec4::MAX = Vec4(FLT_MAX);
    const Vec4 Vec4::MIN = Vec4(-FLT_MAX);
    const Vec4 Vec4::ZERO = Vec4(0);



    Vec2::Vec2(const IVec2& rhs)
        : x(float(rhs.x))
        , y(float(rhs.y)) {}

    Vec2::Vec2(const DVec2& rhs)
        : x(float(rhs.x))
        , y(float(rhs.y)) {}

    Vec2::Vec2(float a)
        : x(a)
        , y(a) {}

    Vec2::Vec2(float a, float b)
        : x(a)
        , y(b) {}

    float& Vec2::operator[](u32 i) {
        ASSERT(i < 2);
        return (&x)[i];
    }

    float Vec2::operator[](u32 i) const {
        ASSERT(i < 2);
        return (&x)[i];
    }

    bool Vec2::operator==(const Vec2& rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    bool Vec2::operator!=(const Vec2& rhs) const {
        return x != rhs.x || y != rhs.y;
    }

    void Vec2::operator/=(float rhs) {
        *this *= 1.0f / rhs;
    }

    void Vec2::operator*=(float f) {
        x *= f;
        y *= f;
    }

    Vec4 operator*(float f, const Vec4& v) {
        return Vec4(f * v.x, f * v.y, f * v.z, f * v.w);
    }

    Vec4 operator*(const Vec4& v, float s) {
        return Vec4(v.x * s, v.y * s, v.z * s, v.w * s);
    }

    Vec3 operator*(float f, const Vec3& v) {
        return Vec3(f * v.x, f * v.y, f * v.z);
    }

    Vec2 Vec2::operator*(const Vec2& v) const {
        return Vec2(x * v.x, y * v.y);
    }
    Vec2 Vec2::operator/(const Vec2& v) const {
        return Vec2(x / v.x, y / v.y);
    }
    Vec2 Vec2::operator*(float f) const {
        return Vec2(x * f, y * f);
    }
    Vec2 Vec2::operator/(float f) const {
        return Vec2(x / f, y / f);
    }
    Vec2 Vec2::operator+(const Vec2& v) const {
        return Vec2(x + v.x, y + v.y);
    }
    Vec2 Vec2::operator+(float v) const {
        return Vec2(x + v, y + v);
    }
    void Vec2::operator+=(const Vec2& v) {
        x += v.x;
        y += v.y;
    }
    Vec2 Vec2::operator-(const Vec2& v) const {
        return Vec2(x - v.x, y - v.y);
    }
    Vec2 Vec2::operator-(float f) const {
        return Vec2(x - f, y - f);
    }
    Vec2 Vec2::operator-() const {
        return Vec2(-x, -y);
    }

    IVec2::IVec2(const Vec2& rhs)
        : x(int(rhs.x))
        , y(int(rhs.y))
    {
    }


    Vec2 IVec2::operator/(const Vec2& rhs) const { return { x / rhs.x, y / rhs.y }; }


    IVec3::IVec3(const DVec3& rhs)
        : x(int(rhs.x))
        , y(int(rhs.y))
        , z(int(rhs.z))
    {
    }


    IVec3::IVec3(const Vec3& rhs)
        : x(int(rhs.x))
        , y(int(rhs.y))
        , z(int(rhs.z))
    {
    }


    DVec3 IVec3::operator *(double i) const
    {
        return { i * x, i * y, i * z };
    }

    Vec4 lerp(const Vec4& op1, const Vec4& op2, float t) {
        const float invt = 1.0f - t;
        return {
            op1.x * invt + op2.x * t,
            op1.y * invt + op2.y * t,
            op1.z * invt + op2.z * t,
            op1.w * invt + op2.w * t
        };
    }

    Vec3 Quat::rotate(const Vec3& v) const
    {
        // nVidia SDK implementation

        Vec3 qvec(x, y, z);
        Vec3 uv = cross(qvec, v);
        Vec3 uuv = cross(qvec, uv);
        uv *= (2.0f * w);
        uuv *= 2.0f;

        return v + uv + uuv;
    }

    DVec3 Quat::rotate(const DVec3& v) const
    {
        // nVidia SDK implementation

        const DVec3 qvec(x, y, z);
        DVec3 uv = cross(qvec, v);
        DVec3 uuv = cross(qvec, uv);
        uv *= (2.0 * w);
        uuv *= 2.0;

        return v + uv + uuv;
    }

    float lerp(float a, float b, float t) {
        return a * (1 - t) + b * t;
    }

    Vec3 lerp(const Vec3& op1, const Vec3& op2, float t) {
        Vec3 res;
        const float invt = 1.0f - t;
        res.x = op1.x * invt + op2.x * t;
        res.y = op1.y * invt + op2.y * t;
        res.z = op1.z * invt + op2.z * t;
        return res;
    }


    DVec3 lerp(const DVec3& op1, const DVec3& op2, float t) {
        DVec3 res;
        const float invt = 1.0f - t;
        res.x = op1.x * invt + op2.x * t;
        res.y = op1.y * invt + op2.y * t;
        res.z = op1.z * invt + op2.z * t;
        return res;
    }


    Vec2 lerp(const Vec2& op1, const Vec2& op2, float t) {
        Vec2 res;
        const float invt = 1.0f - t;
        res.x = op1.x * invt + op2.x * t;
        res.y = op1.y * invt + op2.y * t;
        return res;
    }


    Vec4::Vec4(float a)
        : x(a)
        , y(a)
        , z(a)
        , w(a)
    {
    }

    Vec4::Vec4(float a, float b, float c, float d)
        : x(a)
        , y(b)
        , z(c)
        , w(d)
    {
    }

    Vec4::Vec4(const Vec2& v1, const Vec2& v2)
        : x(v1.x)
        , y(v1.y)
        , z(v2.x)
        , w(v2.y)
    {
    }

    Vec4::Vec4(const Vec3& v, float d)
        : x(v.x)
        , y(v.y)
        , z(v.z)
        , w(d)
    {
    }

    Vec2 Vec4::xz() const { return Vec2(x, z); }
    Vec2 Vec4::xy() const { return Vec2(x, y); }
    Vec3 Vec4::xyz() const { return Vec3(x, y, z); }
    Vec2 Vec4::yz() const { return Vec2(y, z); }
    Vec3 Vec4::rgb() const { return Vec3(x, y, z); }

    float& Vec4::operator[](u32 i) {
        ASSERT(i < 4);
        return (&x)[i];
    }

    float Vec4::operator[](u32 i) const {
        ASSERT(i < 4);
        return (&x)[i];
    }

    bool Vec4::operator==(const Vec4& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    bool Vec4::operator!=(const Vec4& rhs) const {
        return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
    }

    Vec4 Vec4::operator+(const Vec4& rhs) const {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4 Vec4::operator-() const {
        return Vec4(-x, -y, -z, -w);
    }

    Vec4 Vec4::operator-(const Vec4& rhs) const {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    void Vec4::operator+=(const Vec4& rhs)
    {
        float x = this->x;
        float y = this->y;
        float z = this->z;
        float w = this->w;
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    void Vec4::operator-=(const Vec4& rhs)
    {
        float x = this->x;
        float y = this->y;
        float z = this->z;
        float w = this->w;
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    void Vec4::operator/=(float rhs)
    {
        *this *= 1.0f / rhs;
    }

    Vec4 Vec4::operator*(const Vec4& v) { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    Vec4 Vec4::operator*(float s) { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 Vec4::operator/(const Vec4& v) { return Vec4(x / v.x, y / v.y, z / v.z, w / v.w); }
    Vec4 Vec4::operator/(float s) { return Vec4(x / s, y / s, z / s, w / s); }

    void Vec4::operator*=(float rhs)
    {
        float x = this->x;
        float y = this->y;
        float z = this->z;
        float w = this->w;
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    Vec2 normalize(const Vec2& value) {
        float x = value.x;
        float y = value.y;
        const float inv_len = 1 / sqrtf(x * x + y * y);
        x *= inv_len;
        y *= inv_len;
        return Vec2(x, y);
    }

    DVec2 normalize(const DVec2& value) {
        double x = value.x;
        double y = value.y;
        const double inv_len = 1 / sqrt(x * x + y * y);
        x *= inv_len;
        y *= inv_len;
        return DVec2(x, y);
    }

    Vec3 normalize(const Vec3& value) {
        float x = value.x;
        float y = value.y;
        float z = value.z;
        const float inv_len = 1 / sqrtf(x * x + y * y + z * z);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
        return Vec3(x, y, z);
    }

    Quat normalize(const Quat& value) {
        float x = value.x;
        float y = value.y;
        float z = value.z;
        float w = value.w;
        const float inv_len = 1 / sqrtf(x * x + y * y + z * z + w * w);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
        w *= inv_len;
        return Quat(x, y, z, w);
    }

    float length(const Vec2& value) { return sqrtf(value.x * value.x + value.y * value.y); }
    float length(const Vec3& value) { return sqrtf(value.x * value.x + value.y * value.y + value.z * value.z); }
    double length(const DVec3& value) { return sqrt(value.x * value.x + value.y * value.y + value.z * value.z); }
    float squaredLength(const Vec2& value) { return value.x * value.x + value.y * value.y; }
    float squaredLength(const Vec3& value) { return value.x * value.x + value.y * value.y + value.z * value.z; }
    double squaredLength(const DVec2& value) { return value.x * value.x + value.y * value.y; }
    double squaredLength(const DVec3& value) { return value.x * value.x + value.y * value.y + value.z * value.z; }

    float halton(u32 index, i32 base) {
        float f = 1;
        float r = 0;
        i32 current = index;
        do {
            f = f / base;
            r = r + f * (current % base);
            current = i32(floor(float(current) / base));
        } while (current > 0);
        return r;
    }

    Vec3::Vec3(const Vec2& v, float c)
        : x(v.x)
        , y(v.y)
        , z(c) {}

    Vec3::Vec3(float a)
        : x(a)
        , y(a)
        , z(a) {}

    Vec3::Vec3(float a, float b, float c)
        : x(a)
        , y(b)
        , z(c) {}

    float& Vec3::operator[](u32 i) {
        ASSERT(i < 3);
        return (&x)[i];
    }

    float Vec3::operator[](u32 i) const {
        ASSERT(i < 3);
        return (&x)[i];
    }

    bool Vec3::operator==(const Vec3& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool Vec3::operator!=(const Vec3& rhs) const {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }

    Vec3 Vec3::operator+(const Vec3& rhs) const {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec3 Vec3::operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3 Vec3::operator-(const Vec3& rhs) const {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vec3 Vec3::operator*(float s) const {
        return Vec3(x * s, y * s, z * s);
    }
    Vec3 Vec3::operator*(const Vec3& rhs) const {
        return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
    }
    Vec3 Vec3::operator*(const IVec3& rhs) const {
        return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
    }
    Vec3 Vec3::operator/(const IVec3& rhs) const {
        return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
    }
    Vec3 Vec3::operator/(const Vec3& rhs) const {
        return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
    }
    Vec3 Vec3::operator/(float s) const {
        float tmp = 1 / s;
        return Vec3(x * tmp, y * tmp, z * tmp);
    }

    Vec3 Vec3::operator-(float s) const {
        return Vec3(x - s, y - s, z - s);
    }

    void Vec3::operator/=(float rhs) {
        *this *= 1.0f / rhs;
    }

    Vec2 Vec3::xz() const { return { x, z }; }
    Vec2 Vec3::yz() const { return { y, z }; }
    Vec2 Vec3::xy() const { return { x, y }; }

    DVec3::DVec3(float a) : x(a), y(a), z(a) {}

    DVec3::DVec3(double x, double y, double z) : x(x), y(y), z(z) {}

    DVec3::DVec3(const Vec3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

    DVec3 DVec3::operator-() const { return { -x, -y, -z }; }

    DVec3 DVec3::operator*(float rhs) const { return { x * rhs, y * rhs, z * rhs }; }

    DVec3 DVec3::operator*(const Vec3& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }

    DVec3 DVec3::operator/(float rhs) const { return { x / rhs, y / rhs, z / rhs }; }

    DVec3 DVec3::operator/(const Vec3& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

    DVec3 DVec3::operator/(const DVec3& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

    DVec3 DVec3::operator-(const DVec3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }

    DVec3 DVec3::operator+(const DVec3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }

    DVec3 DVec3::operator-(const Vec3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }

    DVec3 DVec3::operator+(const Vec3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }

    void DVec3::operator*=(const double& rhs) { x *= rhs; y *= rhs; z *= rhs; }

    void DVec3::operator/=(const double& rhs) { x /= rhs; y /= rhs; z /= rhs; }

    void DVec3::operator+=(const DVec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }

    void DVec3::operator+=(const Vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }

    void DVec3::operator-=(const Vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }

    DVec2 DVec3::xz() const { return DVec2(x, z); }

    Vec3::Vec3(const DVec3& rhs)
        : x((float)rhs.x)
        , y((float)rhs.y)
        , z((float)rhs.z)
    {}

    Vec3::Vec3(const IVec3& rhs)
        : x((float)rhs.x)
        , y((float)rhs.y)
        , z((float)rhs.z)
    {}

    void Vec3::operator*=(float rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
    }

    void Vec3::operator+=(const Vec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
    }

    void Vec3::operator-=(const Vec3& rhs) {
        float x = this->x;
        float y = this->y;
        float z = this->z;
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void IVec4::operator += (const IVec4& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
    }


}