#include "TQuat.hpp"

#include "Math.hpp"
#include "TMatrix.hpp"

namespace cal {

    const Quat Quat::IDENTITY = { 0, 0, 0, 1 };


    Quat::Quat(const Vec3& axis, float angle)
    {
        float half_angle = angle * 0.5f;
        float s = sinf(half_angle);
        w = cosf(half_angle);
        x = axis.x * s;
        y = axis.y * s;
        z = axis.z * s;
    }


    Quat Quat::vec3ToVec3(const Vec3& v0, const Vec3& v1)
    {
        const Vec3 from = normalize(v0);
        const Vec3 to = normalize(v1);

        float cos_angle = dot(from, to);
        Vec3 half;

        if (cos_angle > -1.0005f && cos_angle < -0.9995f) {
            Vec3 n = Vec3(0, from.z, -from.y);
            if (squaredLength(n) < 0.01) {
                n = Vec3(from.y, -from.x, 0);
            }
            n = normalize(n);
            return Quat(n, PI);
        }
        else
            half = normalize(from + to);

        // http://physicsforgames.blogspot.sk/2010/03/quaternion-tricks.html
        return Quat(
            from.y * half.z - from.z * half.y,
            from.z * half.x - from.x * half.z,
            from.x * half.y - from.y * half.x,
            dot(from, half));
    }


    void Quat::fromEuler(const Vec3& euler)
    {
        ASSERT(euler.x >= -HALF_PI && euler.x <= HALF_PI);
        float ex = euler.x * 0.5f;
        float ey = euler.y * 0.5f;
        float ez = euler.z * 0.5f;
        float sinX = sinf(ex);
        float cosX = cosf(ex);
        float sinY = sinf(ey);
        float cosY = cosf(ey);
        float sinZ = sinf(ez);
        float cosZ = cosf(ez);

        w = cosY * cosX * cosZ + sinY * sinX * sinZ;
        x = cosY * sinX * cosZ + sinY * cosX * sinZ;
        y = sinY * cosX * cosZ - cosY * sinX * sinZ;
        z = cosY * cosX * sinZ - sinY * sinX * cosZ;
    }


    Vec3 Quat::toEuler() const
    {
        // from urho3d
        float check = 2.0f * (w * x - y * z);

        if (check < -0.999999f) {
            return Vec3(-PI * 0.5f, 0.0f, -atan2f(2.0f * (x * z - w * y), 1.0f - 2.0f * (y * y + z * z)));
        }

        if (check > 0.999999f) {
            return Vec3(PI * 0.5f, 0.0f, atan2f(2.0f * (x * z - w * y), 1.0f - 2.0f * (y * y + z * z)));
        }

        return Vec3((float)asin(check),
            atan2f(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y)),
            atan2f(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z)));
    }


    float Quat::toYaw() const {
        float check = 2.0f * (w * x - y * z);

        if (check < -0.999999f) return 0.f;
        if (check > 0.999999f) return 0.f;

        return atan2f(2.f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y));
    }


    void Quat::conjugate()
    {
        w = -w;
    }


    Quat Quat::conjugated() const
    {
        return Quat(x, y, z, -w);
    }

    Vec3 slerp(const Vec3& a, const Vec3& b, float t) {
        float d = dot(a, b);
        d = clamp(d, -1.f, 1.f);
        const float s = acosf(d) * t;
        const Vec3 r = normalize(b - a * d);
        return a * cosf(s) + r * sinf(s);
    }

    Quat nlerp(const Quat& q1, const Quat& q2, float t)
    {
        Quat res;
        float inv = 1.0f - t;
        if (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w < 0) t = -t;
        float ox = q1.x * inv + q2.x * t;
        float oy = q1.y * inv + q2.y * t;
        float oz = q1.z * inv + q2.z * t;
        float ow = q1.w * inv + q2.w * t;
        float l = 1 / sqrtf(ox * ox + oy * oy + oz * oz + ow * ow);
        ox *= l;
        oy *= l;
        oz *= l;
        ow *= l;
        res.x = ox;
        res.y = oy;
        res.z = oz;
        res.w = ow;
        return res;
    }


    Quat Quat::operator*(const Quat& rhs) const
    {
        return Quat(w * rhs.x + rhs.w * x + y * rhs.z - rhs.y * z,
            w * rhs.y + rhs.w * y + z * rhs.x - rhs.z * x,
            w * rhs.z + rhs.w * z + x * rhs.y - rhs.x * y,
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }


    Quat Quat::operator-() const
    {
        return Quat(x, y, z, -w);
    }


    Quat Quat::operator+(const Quat& q) const
    {
        return Quat(x + q.x, y + q.y, z + q.z, w + q.w);
    }


    Quat Quat::operator*(float m) const
    {
        return Quat(x * m, y * m, z * m, w * m);
    }


    Vec3 Quat::operator*(const Vec3& q) const
    {
        return rotate(q);
    }


    Matrix Quat::toMatrix() const
    {
        float fx = x + x;
        float fy = y + y;
        float fz = z + z;
        float fwx = fx * w;
        float fwy = fy * w;
        float fwz = fz * w;
        float fxx = fx * x;
        float fxy = fy * x;
        float fxz = fz * x;
        float fyy = fy * y;
        float fyz = fz * y;
        float fzz = fz * z;

        Matrix mtx;
        mtx.columns[0].x = 1.0f - (fyy + fzz);
        mtx.columns[1].x = fxy - fwz;
        mtx.columns[2].x = fxz + fwy;
        mtx.columns[0].y = fxy + fwz;
        mtx.columns[1].y = 1.0f - (fxx + fzz);
        mtx.columns[2].y = fyz - fwx;
        mtx.columns[0].z = fxz - fwy;
        mtx.columns[1].z = fyz + fwx;
        mtx.columns[2].z = 1.0f - (fxx + fyy);

        mtx.columns[3].x = mtx.columns[3].y = mtx.columns[3].z = mtx.columns[0].w = mtx.columns[1].w = mtx.columns[2].w = 0;
        mtx.columns[3].w = 1;
        return mtx;
    }

}