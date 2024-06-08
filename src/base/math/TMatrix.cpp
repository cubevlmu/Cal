#include "TMatrix.hpp"

#include "Math.hpp"
#include "system/SIMD.hpp"

namespace cal {

    const Matrix Matrix::IDENTITY(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), Vec4(0, 0, 0, 1));

    Matrix::Matrix(const Vec3& pos, const Quat& rot) {
        *this = rot.toMatrix();
        setTranslation(pos);
    }

    void Matrix::lookAt(const Vec3& eye, const Vec3& at, const Vec3& up) {
        *this = Matrix::IDENTITY;
        Vec3 f = normalize((eye - at));
        Vec3 r = normalize(cross(up, f));
        Vec3 u = cross(f, r);
        setXVector(r);
        setYVector(u);
        setZVector(f);
        transpose();
        setTranslation(Vec3(-dot(r, eye), -dot(u, eye), -dot(f, eye)));
    }

    void Matrix::fromEuler(float yaw, float pitch, float roll) {
        float sroll = sinf(roll);
        float croll = cosf(roll);
        float spitch = sinf(pitch);
        float cpitch = cosf(pitch);
        float syaw = sinf(yaw);
        float cyaw = cosf(yaw);

        columns[0].x = sroll * spitch * syaw + croll * cyaw;
        columns[0].y = sroll * cpitch;
        columns[0].z = sroll * spitch * cyaw - croll * syaw;
        columns[0].w = 0.0f;
        columns[1].x = croll * spitch * syaw - sroll * cyaw;
        columns[1].y = croll * cpitch;
        columns[1].z = croll * spitch * cyaw + sroll * syaw;
        columns[1].w = 0.0f;
        columns[2].x = cpitch * syaw;
        columns[2].y = -spitch;
        columns[2].z = cpitch * cyaw;
        columns[2].w = 0.0f;
        columns[3].x = 0.0f;
        columns[3].y = 0.0f;
        columns[3].z = 0.0f;
        columns[3].w = 1.0f;
    }

    Matrix Matrix::rotationX(float angle) {
        Matrix m = IDENTITY;
        float c = cosf(angle);
        float s = sinf(angle);

        m.columns[1].y = m.columns[2].z = c;
        m.columns[2].y = -s;
        m.columns[1].z = s;

        return m;
    }

    Matrix Matrix::rotationY(float angle) {
        Matrix m = IDENTITY;
        float c = cosf(angle);
        float s = sinf(angle);

        m.columns[0].x = m.columns[2].z = c;
        m.columns[2].x = s;
        m.columns[0].z = -s;

        return m;
    }

    Matrix Matrix::rotationZ(float angle) {
        Matrix m = IDENTITY;
        float c = cosf(angle);
        float s = sinf(angle);

        m.columns[0].x = m.columns[1].y = c;
        m.columns[1].x = -s;
        m.columns[0].y = s;

        return m;
    }

    void Matrix::setOrtho(float left, float right, float bottom, float top, float z_near, float z_far, bool reversed_z) {
        *this = IDENTITY;
        columns[0].x = 2 / (right - left);
        columns[1].y = 2 / (top - bottom);
        columns[3].x = (right + left) / (left - right);
        columns[3].y = (top + bottom) / (bottom - top);
        if (reversed_z) {
            columns[2].z = 1 / (z_far - z_near);
            columns[3].z = z_far / (z_far - z_near);
        }
        else {
            columns[2].z = -1 / (z_far - z_near);
            columns[3].z = z_near / (z_near - z_far);
        }
    }

    void Matrix::setPerspective(float fov, float ratio, float near_plane, float far_plane, bool reversed_z) {
        *this = Matrix::IDENTITY;
        float f = 1 / tanf(fov * 0.5f);
        columns[0].x = f / ratio;
        columns[1].y = f;
        columns[3].w = 0;
        columns[2].w = -1.0f;

        if (reversed_z) {
            columns[2].z = 0;
            columns[3].z = near_plane;
        }
        else {
            columns[2].z = -1;
            columns[3].z = -near_plane;
        }
    }

    void Matrix::decompose(Vec3& position, Quat& rotation, float& scale) const {
        position = getTranslation();
        scale = length(getXVector());
        Matrix tmp = *this;
        tmp.multiply3x3(1 / scale);
        rotation = tmp.getRotation();
    }

    Matrix Matrix::operator*(float rhs) const {
        Matrix out;
        out.columns[0].x = columns[0].x * rhs;
        out.columns[0].y = columns[0].y * rhs;
        out.columns[0].z = columns[0].z * rhs;
        out.columns[0].w = columns[0].w * rhs;

        out.columns[1].x = columns[1].x * rhs;
        out.columns[1].y = columns[1].y * rhs;
        out.columns[1].z = columns[1].z * rhs;
        out.columns[1].w = columns[1].w * rhs;

        out.columns[2].x = columns[2].x * rhs;
        out.columns[2].y = columns[2].y * rhs;
        out.columns[2].z = columns[2].z * rhs;
        out.columns[2].w = columns[2].w * rhs;

        out.columns[3].x = columns[3].x * rhs;
        out.columns[3].y = columns[3].y * rhs;
        out.columns[3].z = columns[3].z * rhs;
        out.columns[3].w = columns[3].w * rhs;

        return out;
    }

    Matrix Matrix::operator+(const Matrix& rhs) const {
        Matrix out;

        out.columns[0].x = columns[0].x + rhs.columns[0].x;
        out.columns[0].y = columns[0].y + rhs.columns[0].y;
        out.columns[0].z = columns[0].z + rhs.columns[0].z;
        out.columns[0].w = columns[0].w + rhs.columns[0].w;

        out.columns[1].x = columns[1].x + rhs.columns[1].x;
        out.columns[1].y = columns[1].y + rhs.columns[1].y;
        out.columns[1].z = columns[1].z + rhs.columns[1].z;
        out.columns[1].w = columns[1].w + rhs.columns[1].w;

        out.columns[2].x = columns[2].x + rhs.columns[2].x;
        out.columns[2].y = columns[2].y + rhs.columns[2].y;
        out.columns[2].z = columns[2].z + rhs.columns[2].z;
        out.columns[2].w = columns[2].w + rhs.columns[2].w;

        out.columns[3].x = columns[3].x + rhs.columns[3].x;
        out.columns[3].y = columns[3].y + rhs.columns[3].y;
        out.columns[3].z = columns[3].z + rhs.columns[3].z;
        out.columns[3].w = columns[3].w + rhs.columns[3].w;

        return out;
    }

    Matrix Matrix::operator*(const Matrix& rhs) const {
        Matrix out;

        const float4 a = f4Load(&columns[0].x);
        const float4 b = f4Load(&columns[1].x);
        const float4 c = f4Load(&columns[2].x);
        const float4 d = f4Load(&columns[3].x);

        float4 t1 = f4Splat(rhs.columns[0].x);
        float4 t2 = f4Mul(a, t1);
        t1 = f4Splat(rhs.columns[0].y);
        t2 = f4Add(f4Mul(b, t1), t2);
        t1 = f4Splat(rhs.columns[0].z);
        t2 = f4Add(f4Mul(c, t1), t2);
        t1 = f4Splat(rhs.columns[0].w);
        t2 = f4Add(f4Mul(d, t1), t2);

        f4Store(&out.columns[0].x, t2);

        t1 = f4Splat(rhs.columns[1].x);
        t2 = f4Mul(a, t1);
        t1 = f4Splat(rhs.columns[1].y);
        t2 = f4Add(f4Mul(b, t1), t2);
        t1 = f4Splat(rhs.columns[1].z);
        t2 = f4Add(f4Mul(c, t1), t2);
        t1 = f4Splat(rhs.columns[1].w);
        t2 = f4Add(f4Mul(d, t1), t2);

        f4Store(&out.columns[1].x, t2);

        t1 = f4Splat(rhs.columns[2].x);
        t2 = f4Mul(a, t1);
        t1 = f4Splat(rhs.columns[2].y);
        t2 = f4Add(f4Mul(b, t1), t2);
        t1 = f4Splat(rhs.columns[2].z);
        t2 = f4Add(f4Mul(c, t1), t2);
        t1 = f4Splat(rhs.columns[2].w);
        t2 = f4Add(f4Mul(d, t1), t2);

        f4Store(&out.columns[2].x, t2);

        t1 = f4Splat(rhs.columns[3].x);
        t2 = f4Mul(a, t1);
        t1 = f4Splat(rhs.columns[3].y);
        t2 = f4Add(f4Mul(b, t1), t2);
        t1 = f4Splat(rhs.columns[3].z);
        t2 = f4Add(f4Mul(c, t1), t2);
        t1 = f4Splat(rhs.columns[3].w);
        t2 = f4Add(f4Mul(d, t1), t2);

        f4Store(&out.columns[3].x, t2);

        return out;
    }

    void Matrix::normalizeScale() {
        Vec3 scale = { 1 / length(Vec3(columns[0].x, columns[1].x, columns[2].x)), 1 / length(Vec3(columns[0].y, columns[1].y, columns[2].y)), 1 / length(Vec3(columns[0].z, columns[1].z, columns[2].z)) };

        columns[0].x *= scale.x;
        columns[1].x *= scale.x;
        columns[2].x *= scale.x;

        columns[0].y *= scale.y;
        columns[1].y *= scale.y;
        columns[2].y *= scale.y;

        columns[0].z *= scale.z;
        columns[1].z *= scale.z;
        columns[2].z *= scale.z;
    }

    Quat Matrix::getRotation() const {
        Quat rot;
        float tr = columns[0].x + columns[1].y + columns[2].z;

        if (tr > 0) {
            float t = tr + 1.0f;
            float s = 1 / sqrtf(t) * 0.5f;

            rot.w = s * t;
            rot.z = (columns[0].y - columns[1].x) * s;
            rot.y = (columns[2].x - columns[0].z) * s;
            rot.x = (columns[1].z - columns[2].y) * s;
        }
        else if ((columns[0].x > columns[1].y) && (columns[0].x > columns[2].z)) {
            float t = 1.0f + columns[0].x - columns[1].y - columns[2].z;
            float s = 1 / sqrtf(t) * 0.5f;
            rot.x = s * t;
            rot.y = (columns[0].y + columns[1].x) * s;
            rot.z = (columns[0].z + columns[2].x) * s;
            rot.w = (columns[1].z - columns[2].y) * s;
        }
        else if (columns[1].y > columns[2].z) {
            float t = 1.0f + columns[1].y - columns[0].x - columns[2].z;
            float s = 1 / sqrtf(t) * 0.5f;
            rot.w = (columns[2].x - columns[0].z) * s;
            rot.x = (columns[0].y + columns[1].x) * s;
            rot.y = s * t;
            rot.z = (columns[1].z + columns[2].y) * s;
        }
        else {
            float t = 1.0f + columns[2].z - columns[0].x - columns[1].y;
            float s = 1 / sqrtf(t) * 0.5f;
            rot.w = (columns[0].y - columns[1].x) * s;
            rot.x = (columns[2].x + columns[0].z) * s;
            rot.y = (columns[2].y + columns[1].z) * s;
            rot.z = s * t;
        }
        return rot;
    }

    void Matrix::transpose() {
        float tmp = columns[1].x;
        columns[1].x = columns[0].y;
        columns[0].y = tmp;

        tmp = columns[2].y;
        columns[2].y = columns[1].z;
        columns[1].z = tmp;

        tmp = columns[2].x;
        columns[2].x = columns[0].z;
        columns[0].z = tmp;

        tmp = columns[3].x;
        columns[3].x = columns[0].w;
        columns[0].w = tmp;

        tmp = columns[3].y;
        columns[3].y = columns[1].w;
        columns[1].w = tmp;

        tmp = columns[3].z;
        columns[3].z = columns[2].w;
        columns[2].w = tmp;
    }

    void Matrix::multiply3x3(const Vec3& scale) {
        columns[0].x *= scale.x;
        columns[0].y *= scale.x;
        columns[0].z *= scale.x;
        columns[1].x *= scale.y;
        columns[1].y *= scale.y;
        columns[1].z *= scale.y;
        columns[2].x *= scale.z;
        columns[2].y *= scale.z;
        columns[2].z *= scale.z;
    }

    void Matrix::multiply3x3(float scale) {
        columns[0].x *= scale;
        columns[0].y *= scale;
        columns[0].z *= scale;
        columns[1].x *= scale;
        columns[1].y *= scale;
        columns[1].z *= scale;
        columns[2].x *= scale;
        columns[2].y *= scale;
        columns[2].z *= scale;
    }

    Vec3 Matrix::transformPoint(const Vec3& rhs) const {
        return Vec3(columns[0].x * rhs.x + columns[1].x * rhs.y + columns[2].x * rhs.z + columns[3].x,
            columns[0].y * rhs.x + columns[1].y * rhs.y + columns[2].y * rhs.z + columns[3].y,
            columns[0].z * rhs.x + columns[1].z * rhs.y + columns[2].z * rhs.z + columns[3].z);
    }

    Vec3 Matrix::transformVector(const Vec3& rhs) const {
        return Vec3(columns[0].x * rhs.x + columns[1].x * rhs.y + columns[2].x * rhs.z,
            columns[0].y * rhs.x + columns[1].y * rhs.y + columns[2].y * rhs.z,
            columns[0].z * rhs.x + columns[1].z * rhs.y + columns[2].z * rhs.z);
    }

    void Matrix::setIdentity() {
        columns[0].x = 1;
        columns[0].y = 0;
        columns[0].z = 0;
        columns[0].w = 0;
        columns[1].x = 0;
        columns[1].y = 1;
        columns[1].z = 0;
        columns[1].w = 0;
        columns[2].x = 0;
        columns[2].y = 0;
        columns[2].z = 1;
        columns[2].w = 0;
        columns[3].x = 0;
        columns[3].y = 0;
        columns[3].z = 0;
        columns[3].w = 1;
    }
}