#include "TTransform.hpp"

#include "Math.hpp"
#include "TMatrix.hpp"

namespace cal {

    Transform::Transform(const DVec3& pos, const Quat& rot, Vec3 scale)
        : pos(pos)
        , rot(rot)
        , scale(scale) {}

    Transform Transform::operator*(const Transform& rhs) const { return { rot.rotate(rhs.pos * scale) + pos, rot * rhs.rot, scale * rhs.scale }; }

    Transform Transform::operator*(const LocalRigidTransform& rhs) const { return { pos + rot.rotate(rhs.pos * scale), rot * rhs.rot, scale }; }

    DVec3 Transform::transform(const Vec3& value) const { return pos + rot.rotate(value) * scale; }

    DVec3 Transform::transform(const DVec3& value) const { return pos + rot.rotate(value) * scale; }

    Vec3 Transform::transformVector(const Vec3& value) const { return rot.rotate(value) * scale; }

    RigidTransform Transform::getRigidPart() const { return { pos, rot }; }

    Transform Transform::inverted() const {
        Transform result;
        result.rot = rot.conjugated();
        result.pos = result.rot.rotate(-pos / scale);
        result.scale = Vec3(1.0f) / scale;
        return result;
    }

    LocalTransform::LocalTransform(const Vec3& pos, const Quat& rot, float scale)
        : pos(pos)
        , rot(rot)
        , scale(scale)
    {}

    LocalTransform LocalTransform::inverted() const {
        LocalTransform result;
        result.rot = rot.conjugated();
        result.pos = result.rot.rotate(-pos / scale);
        result.scale = 1.0f / scale;
        return result;
    }

    LocalTransform LocalTransform::operator*(const LocalTransform& rhs) const {
        return { pos + rot.rotate(rhs.pos * scale), rot * rhs.rot, scale };
    }

    LocalRigidTransform LocalRigidTransform::inverted() const {
        LocalRigidTransform result;
        result.rot = rot.conjugated();
        result.pos = result.rot.rotate(-pos);
        return result;
    }

    DualQuat LocalRigidTransform::toDualQuat() const {
        DualQuat res;
        res.r = rot;
        res.d = {
            0.5f * (pos.x * rot.w + pos.y * rot.z - pos.z * rot.y),
            0.5f * (-pos.x * rot.z + pos.y * rot.w + pos.z * rot.x),
            0.5f * (pos.x * rot.y - pos.y * rot.x + pos.z * rot.w),
            -0.5f * (pos.x * rot.x + pos.y * rot.y + pos.z * rot.z)
        };
        return res;
    }

    Matrix LocalRigidTransform::toMatrix() const {
        return Matrix(pos, rot);
    }

    LocalRigidTransform LocalRigidTransform::operator*(const LocalRigidTransform& rhs) const {
        return { rot.rotate(rhs.pos) + pos, rot * rhs.rot };
    }

    LocalRigidTransform LocalRigidTransform::interpolate(const LocalRigidTransform& rhs, float t) const {
        LocalRigidTransform ret;
        ret.pos = lerp(pos, rhs.pos, t);
        ret.rot = nlerp(rot, rhs.rot, t);
        return ret;
    }

    RigidTransform RigidTransform::inverted() const {
        RigidTransform result;
        result.rot = rot.conjugated();
        result.pos = result.rot.rotate(-pos);
        return result;
    }

    RigidTransform::RigidTransform(const DVec3& pos, const Quat& rot)
        : pos(pos)
        , rot(rot) {}

    RigidTransform RigidTransform::operator*(const RigidTransform& rhs) const { return { rot.rotate(rhs.pos) + pos, rot * rhs.rot }; }

    RigidTransform RigidTransform::operator*(const LocalRigidTransform& rhs) const { return { DVec3(rot.rotate(rhs.pos)) + pos, rot * rhs.rot }; }

}