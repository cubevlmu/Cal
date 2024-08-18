#pragma once

#include "TVector.hpp"
#include "TQuat.hpp"

namespace cal {

    struct alignas(16) Matrix {
        static Matrix rotationX(float angle);
        static Matrix rotationY(float angle);
        static Matrix rotationZ(float angle);

        Matrix() {}

        Matrix(const Vec4& col0, const Vec4& col1, const Vec4& col2, const Vec4& col3);
        Matrix(const Vec3& pos, const Quat& rot);

        void decompose(Vec3& position, Quat& rotation, float& scale) const;

        float operator[](int index) const { return (&columns[0].x)[index]; }
        float& operator[](int index) { return (&columns[0].x)[index]; }

        Matrix operator*(const Matrix& rhs) const;
        Matrix operator+(const Matrix& rhs) const;
        Matrix operator*(float rhs) const;
        Vec4 operator*(const Vec4& rhs) const { return columns[0] * rhs.x + columns[1] * rhs.y + columns[2] * rhs.z + columns[3] * rhs.w; }

        Vec3 getZVector() const { return columns[2].xyz(); }
        Vec3 getYVector() const { return columns[1].xyz(); }
        Vec3 getXVector() const { return columns[0].xyz(); }

        void setXVector(const Vec3& v);
        void setYVector(const Vec3& v);
        void setZVector(const Vec3& v);

        float determinant() const;
        Matrix inverted() const;
        // orthonormal
        Matrix fastInverted() const;
        void copy3x3(const Matrix& mtx);
        void translate(const Vec3& t);
        void translate(float x, float y, float z);
        void setTranslation(const Vec3& t);

        void setOrtho(float left, float right, float bottom, float top, float z_near, float z_far, bool reversed_z);
        void setPerspective(float fov, float ratio, float near_plane, float far_plane, bool invert_z);
        void fromEuler(float yaw, float pitch, float roll);
        void lookAt(const Vec3& eye, const Vec3& at, const Vec3& up);
        void normalizeScale();

        Vec3 getTranslation() const { return columns[3].xyz(); }
        Quat getRotation() const;
        void transpose();
        Vec3 transformPoint(const Vec3& pos) const;
        Vec3 transformVector(const Vec3& pos) const;
        void multiply3x3(float scale);
        void multiply3x3(const Vec3& scale);
        void setIdentity();

        Vec4 columns[4];

        static const Matrix IDENTITY;
    };

    struct Matrix3x4 {
        Matrix3x4() {}
        explicit Matrix3x4(const Matrix& rhs);
        Vec4 columns[3];
    };

    struct Matrix4x3 {
        Matrix4x3() {}
        explicit Matrix4x3(const Matrix& rhs);
        Matrix3x4 transposed() const;

        Vec3 columns[4];
    };

}