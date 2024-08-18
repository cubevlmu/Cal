#include "Math.hpp"

#include "system/SysTimer.hpp"
#include "TMatrix.hpp"

namespace cal {

    float dot(const Vec4& op1, const Vec4& op2) {
        return op1.x * op2.x + op1.y * op2.y + op1.z * op2.z + op1.w * op2.w;
    }

    float dot(const Vec3& op1, const Vec3& op2) {
        return op1.x * op2.x + op1.y * op2.y + op1.z * op2.z;
    }

    float dot(const Vec2& op1, const Vec2& op2) {
        return op1.x * op2.x + op1.y * op2.y;
    }

    Vec3 cross(const Vec3& op1, const Vec3& op2) {
        return Vec3(op1.y * op2.z - op1.z * op2.y, op1.z * op2.x - op1.x * op2.z, op1.x * op2.y - op1.y * op2.x);
    }

    DVec3 cross(const DVec3& op1, const DVec3& op2) {
        return DVec3(op1.y * op2.z - op1.z * op2.y, op1.z * op2.x - op1.x * op2.z, op1.x * op2.y - op1.y * op2.x);
    }

    float easeInOut(float t) {
        float scaled_t = t * 2;
        if (scaled_t < 1) {
            return 0.5f * scaled_t * scaled_t;
        }
        --scaled_t;
        return -0.5f * (scaled_t * (scaled_t - 2) - 1);
    }

    float degreesToRadians(float angle) {
        return angle * PI / 180.0f;
    }

    double degreesToRadians(double angle) {
        return angle * PI / 180.0;
    }

    float degreesToRadians(int angle) {
        return angle * PI / 180.0f;
    }

    float radiansToDegrees(float angle) {
        return angle / PI * 180.0f;
    }

    Vec2 degreesToRadians(const Vec2& v) {
        return Vec2(degreesToRadians(v.x), degreesToRadians(v.y));
    }

    Vec2 radiansToDegrees(const Vec2& v) {
        return Vec2(radiansToDegrees(v.x), radiansToDegrees(v.y));
    }

    Vec3 degreesToRadians(const Vec3& v) {
        return Vec3(degreesToRadians(v.x), degreesToRadians(v.y), degreesToRadians(v.z));
    }

    Vec3 radiansToDegrees(const Vec3& v) {
        return Vec3(radiansToDegrees(v.x), radiansToDegrees(v.y), radiansToDegrees(v.z));
    }

    float angleDiff(float a, float b) {
        float delta = a - b;
        delta = fmodf(delta, PI * 2);
        if (delta > PI) return -PI * 2 + delta;
        if (delta < -PI) return PI * 2 + delta;
        return delta;
    }

    RandomGenerator::RandomGenerator(u32 u, u32 v) : u(u), v(v) {}

    u32 RandomGenerator::rand() {
        u = 36969 * (u & 65535) + (u >> 16);
        v = 18000 * (v & 65535) + (v >> 16);
        return (u << 16) + v;
    }

    static RandomGenerator initRandomGenerator() {
        // TODO improve entropy
        thread_local u64 dummy;
        const u64 seed = uintptr(&dummy) ^ platform::Timer::getRawTimestamp();
        RandomGenerator res(u32(seed), u32(seed >> 32));
        return res;
    }

    static thread_local RandomGenerator rg = initRandomGenerator();

    u32 rand() {
        return rg.rand();
    }

    u64 randGUID() {
        return (u64(rand()) << 32) + u64(rand());
    }


    u32 rand(u32 from_incl, u32 to_incl) {
        return from_incl + rand() % (to_incl - from_incl + 1);
    }


    float randFloat() {
        u32 i = rand();
        return float(i * 2.328306435996595e-10);
    }

    float RandomGenerator::randFloat(float from, float to) {
        return from + float((to - from) * (rand() * 2.328306435996595e-10));
    }

    float RandomGenerator::randFloat() {
        return float(rand() * 2.328306435996595e-10);
    }

    float randFloat(float from, float to) {
        return rg.randFloat(from, to);
    }

    u32 nextPow2(u32 v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }


    u32 log2(u32 v)
    {
        u32 r = (v > 0xffff) << 4; v >>= r;
        u32 shift = (v > 0xff) << 3; v >>= shift; r |= shift;
        shift = (v > 0xf) << 2; v >>= shift; r |= shift;
        shift = (v > 0x3) << 1; v >>= shift; r |= shift;
        r |= (v >> 1);
        return r;
    }


    Matrix4x3::Matrix4x3(const Matrix& rhs) {
        columns[0] = rhs.columns[0].xyz();
        columns[1] = rhs.columns[1].xyz();
        columns[2] = rhs.columns[2].xyz();
        columns[3] = rhs.columns[3].xyz();
    }

    Matrix3x4 Matrix4x3::transposed() const {
        Matrix3x4 res;
        res.columns[0] = Vec4(columns[0].x, columns[1].x, columns[2].x, columns[3].x);
        res.columns[1] = Vec4(columns[0].y, columns[1].y, columns[2].y, columns[3].y);
        res.columns[2] = Vec4(columns[0].z, columns[1].z, columns[2].z, columns[3].z);
        return res;
    }

    Matrix3x4::Matrix3x4(const Matrix& rhs) {
        columns[0] = rhs.columns[0];
        columns[1] = rhs.columns[1];
        columns[2] = rhs.columns[2];
    }

    Matrix::Matrix(const Vec4& col0, const Vec4& col1, const Vec4& col2, const Vec4& col3) {
        columns[0] = col0;
        columns[1] = col1;
        columns[2] = col2;
        columns[3] = col3;
    }

    void Matrix::setXVector(const Vec3& v) {
        columns[0].x = v.x;
        columns[0].y = v.y;
        columns[0].z = v.z;
    }

    void Matrix::setYVector(const Vec3& v) {
        columns[1].x = v.x;
        columns[1].y = v.y;
        columns[1].z = v.z;
    }

    void Matrix::setZVector(const Vec3& v) {
        columns[2].x = v.x;
        columns[2].y = v.y;
        columns[2].z = v.z;
    }


    float Matrix::determinant() const {
        return columns[0].w * columns[1].z * columns[2].y * columns[3].x - columns[0].z * columns[1].w * columns[2].y * columns[3].x - columns[0].w * columns[1].y * columns[2].z * columns[3].x +
            columns[0].y * columns[1].w * columns[2].z * columns[3].x + columns[0].z * columns[1].y * columns[2].w * columns[3].x - columns[0].y * columns[1].z * columns[2].w * columns[3].x -
            columns[0].w * columns[1].z * columns[2].x * columns[3].y + columns[0].z * columns[1].w * columns[2].x * columns[3].y + columns[0].w * columns[1].x * columns[2].z * columns[3].y -
            columns[0].x * columns[1].w * columns[2].z * columns[3].y - columns[0].z * columns[1].x * columns[2].w * columns[3].y + columns[0].x * columns[1].z * columns[2].w * columns[3].y +
            columns[0].w * columns[1].y * columns[2].x * columns[3].z - columns[0].y * columns[1].w * columns[2].x * columns[3].z - columns[0].w * columns[1].x * columns[2].y * columns[3].z +
            columns[0].x * columns[1].w * columns[2].y * columns[3].z + columns[0].y * columns[1].x * columns[2].w * columns[3].z - columns[0].x * columns[1].y * columns[2].w * columns[3].z -
            columns[0].z * columns[1].y * columns[2].x * columns[3].w + columns[0].y * columns[1].z * columns[2].x * columns[3].w + columns[0].z * columns[1].x * columns[2].y * columns[3].w -
            columns[0].x * columns[1].z * columns[2].y * columns[3].w - columns[0].y * columns[1].x * columns[2].z * columns[3].w + columns[0].x * columns[1].y * columns[2].z * columns[3].w;
    }


    Matrix Matrix::inverted() const {
        Matrix mtx;
        float d = determinant();
        if (d == 0) return *this;
        d = 1.0f / d;

        mtx.columns[0] = Vec4(d * (columns[1].z * columns[2].w * columns[3].y - columns[1].w * columns[2].z * columns[3].y + columns[1].w * columns[2].y * columns[3].z -
            columns[1].y * columns[2].w * columns[3].z - columns[1].z * columns[2].y * columns[3].w + columns[1].y * columns[2].z * columns[3].w),
            d * (columns[0].w * columns[2].z * columns[3].y - columns[0].z * columns[2].w * columns[3].y - columns[0].w * columns[2].y * columns[3].z + columns[0].y * columns[2].w * columns[3].z +
                columns[0].z * columns[2].y * columns[3].w - columns[0].y * columns[2].z * columns[3].w),
            d * (columns[0].z * columns[1].w * columns[3].y - columns[0].w * columns[1].z * columns[3].y + columns[0].w * columns[1].y * columns[3].z - columns[0].y * columns[1].w * columns[3].z -
                columns[0].z * columns[1].y * columns[3].w + columns[0].y * columns[1].z * columns[3].w),
            d * (columns[0].w * columns[1].z * columns[2].y - columns[0].z * columns[1].w * columns[2].y - columns[0].w * columns[1].y * columns[2].z + columns[0].y * columns[1].w * columns[2].z +
                columns[0].z * columns[1].y * columns[2].w - columns[0].y * columns[1].z * columns[2].w));
        mtx.columns[1] = Vec4(d * (columns[1].w * columns[2].z * columns[3].x - columns[1].z * columns[2].w * columns[3].x - columns[1].w * columns[2].x * columns[3].z +
            columns[1].x * columns[2].w * columns[3].z + columns[1].z * columns[2].x * columns[3].w - columns[1].x * columns[2].z * columns[3].w),
            d * (columns[0].z * columns[2].w * columns[3].x - columns[0].w * columns[2].z * columns[3].x + columns[0].w * columns[2].x * columns[3].z - columns[0].x * columns[2].w * columns[3].z -
                columns[0].z * columns[2].x * columns[3].w + columns[0].x * columns[2].z * columns[3].w),
            d * (columns[0].w * columns[1].z * columns[3].x - columns[0].z * columns[1].w * columns[3].x - columns[0].w * columns[1].x * columns[3].z + columns[0].x * columns[1].w * columns[3].z +
                columns[0].z * columns[1].x * columns[3].w - columns[0].x * columns[1].z * columns[3].w),
            d * (columns[0].z * columns[1].w * columns[2].x - columns[0].w * columns[1].z * columns[2].x + columns[0].w * columns[1].x * columns[2].z - columns[0].x * columns[1].w * columns[2].z -
                columns[0].z * columns[1].x * columns[2].w + columns[0].x * columns[1].z * columns[2].w));
        mtx.columns[2] = Vec4(d * (columns[1].y * columns[2].w * columns[3].x - columns[1].w * columns[2].y * columns[3].x + columns[1].w * columns[2].x * columns[3].y -
            columns[1].x * columns[2].w * columns[3].y - columns[1].y * columns[2].x * columns[3].w + columns[1].x * columns[2].y * columns[3].w),
            d * (columns[0].w * columns[2].y * columns[3].x - columns[0].y * columns[2].w * columns[3].x - columns[0].w * columns[2].x * columns[3].y + columns[0].x * columns[2].w * columns[3].y +
                columns[0].y * columns[2].x * columns[3].w - columns[0].x * columns[2].y * columns[3].w),
            d * (columns[0].y * columns[1].w * columns[3].x - columns[0].w * columns[1].y * columns[3].x + columns[0].w * columns[1].x * columns[3].y - columns[0].x * columns[1].w * columns[3].y -
                columns[0].y * columns[1].x * columns[3].w + columns[0].x * columns[1].y * columns[3].w),
            d * (columns[0].w * columns[1].y * columns[2].x - columns[0].y * columns[1].w * columns[2].x - columns[0].w * columns[1].x * columns[2].y + columns[0].x * columns[1].w * columns[2].y +
                columns[0].y * columns[1].x * columns[2].w - columns[0].x * columns[1].y * columns[2].w));
        mtx.columns[3] = Vec4(d * (columns[1].z * columns[2].y * columns[3].x - columns[1].y * columns[2].z * columns[3].x - columns[1].z * columns[2].x * columns[3].y +
            columns[1].x * columns[2].z * columns[3].y + columns[1].y * columns[2].x * columns[3].z - columns[1].x * columns[2].y * columns[3].z),
            d * (columns[0].y * columns[2].z * columns[3].x - columns[0].z * columns[2].y * columns[3].x + columns[0].z * columns[2].x * columns[3].y - columns[0].x * columns[2].z * columns[3].y -
                columns[0].y * columns[2].x * columns[3].z + columns[0].x * columns[2].y * columns[3].z),
            d * (columns[0].z * columns[1].y * columns[3].x - columns[0].y * columns[1].z * columns[3].x - columns[0].z * columns[1].x * columns[3].y + columns[0].x * columns[1].z * columns[3].y +
                columns[0].y * columns[1].x * columns[3].z - columns[0].x * columns[1].y * columns[3].z),
            d * (columns[0].y * columns[1].z * columns[2].x - columns[0].z * columns[1].y * columns[2].x + columns[0].z * columns[1].x * columns[2].y - columns[0].x * columns[1].z * columns[2].y -
                columns[0].y * columns[1].x * columns[2].z + columns[0].x * columns[1].y * columns[2].z));
        return mtx;
    }

    Matrix Matrix::fastInverted() const {
        Matrix ret;
        ret.columns[0].x = columns[0].x;
        ret.columns[1].y = columns[1].y;
        ret.columns[2].z = columns[2].z;

        ret.columns[1].x = columns[0].y;
        ret.columns[0].y = columns[1].x;

        ret.columns[1].z = columns[2].y;
        ret.columns[2].y = columns[1].z;

        ret.columns[0].z = columns[2].x;
        ret.columns[2].x = columns[0].z;

        float x = -columns[3].x;
        float y = -columns[3].y;
        float z = -columns[3].z;
        ret.columns[3].x = x * ret.columns[0].x + y * ret.columns[1].x + z * ret.columns[2].x;
        ret.columns[3].y = x * ret.columns[0].y + y * ret.columns[1].y + z * ret.columns[2].y;
        ret.columns[3].z = x * ret.columns[0].z + y * ret.columns[1].z + z * ret.columns[2].z;

        ret.columns[0].w = 0;
        ret.columns[1].w = 0;
        ret.columns[2].w = 0;
        ret.columns[3].w = 1;

        return ret;
    }

    void Matrix::copy3x3(const Matrix& mtx) {
        columns[0].x = mtx.columns[0].x;
        columns[0].y = mtx.columns[0].y;
        columns[0].z = mtx.columns[0].z;

        columns[1].x = mtx.columns[1].x;
        columns[1].y = mtx.columns[1].y;
        columns[1].z = mtx.columns[1].z;

        columns[2].x = mtx.columns[2].x;
        columns[2].y = mtx.columns[2].y;
        columns[2].z = mtx.columns[2].z;
    }

    void Matrix::translate(const Vec3& t) {
        columns[3].x += t.x;
        columns[3].y += t.y;
        columns[3].z += t.z;
    }

    void Matrix::translate(float x, float y, float z) {
        columns[3].x += x;
        columns[3].y += y;
        columns[3].z += z;
    }

    void Matrix::setTranslation(const Vec3& t) {
        columns[3].x = t.x;
        columns[3].y = t.y;
        columns[3].z = t.z;
    }

}