#pragma once

#include "TQuat.hpp"
#include "TVector.hpp"

namespace cal {

    struct LocalRigidTransform {
        LocalRigidTransform inverted() const;
        LocalRigidTransform operator*(const LocalRigidTransform& rhs) const;
        Matrix toMatrix() const;
        DualQuat toDualQuat() const;
        LocalRigidTransform interpolate(const LocalRigidTransform& rhs, float t) const;
        Vec3 pos;
        Quat rot;
    };


    struct RigidTransform {
        RigidTransform() {}
        RigidTransform(const DVec3& pos, const Quat& rot);

        RigidTransform inverted() const;
        RigidTransform operator*(const RigidTransform& rhs) const;
        RigidTransform operator*(const LocalRigidTransform& rhs) const;

        Quat rot;
        DVec3 pos;
    };


    struct LocalTransform {
        LocalTransform() {}
        LocalTransform(const Vec3& pos, const Quat& rot, float scale);

        LocalTransform inverted() const;

        LocalTransform operator*(const LocalTransform& rhs) const;

        Vec3 pos;
        Quat rot;
        float scale;
    };


    struct Transform {
        Transform() {}
        Transform(const DVec3& pos, const Quat& rot, Vec3 scale);

        Transform inverted() const;

        Transform operator*(const Transform& rhs) const;
        Transform operator*(const LocalRigidTransform& rhs) const;
        DVec3 transform(const Vec3& value) const;
        Vec3 transformVector(const Vec3& value) const;
        DVec3 transform(const DVec3& value) const;
        RigidTransform getRigidPart() const;

        DVec3 pos;
        Quat rot;
        Vec3 scale;

        static const Transform IDENTITY;
    };

}