#pragma once

#include <lib/egg/math/eggVector.h>

namespace EGG {

struct Quatf : public Vector3f {
    Quatf() {}

    Quatf(f32 f, Vector3f v) : w(f), Vector3f(v) {}

    Quatf(f32 f, f32 x, f32 y, f32 z) : w(f), Vector3f(Vector3f(x, y, z)) {}

    ~Quatf() {}

    friend Quatf operator*(const Quatf& q, const Vector3f& vec);
    friend Quatf operator*(const Quatf& u, const Quatf& v);

    void set(f32 fw, f32 fx, f32 fy, f32 fz);
    void set(f32 f, const Vector3f& vec);
    void setRPY(const EGG::Vector3f& rpy);
    Vector3f calcRPY();
    void setRPY(f32 roll, f32 pitch, f32 yaw);
    void setAxisRotation(const Vector3f&, f32);
    f32 norm();
    void normalise();
    Quatf conjugate();
    Quatf inverse();
    Vector3f rotateVector(const Vector3f&);
    Vector3f rotateVectorInv(const Vector3f&);
    void slerpTo(const Quatf&, f32, Quatf& out) const;
    void limitSlerpTo(const Quatf&, f32, f32, Quatf& out) const;
    void makeVectorRotationLimit(Vector3f&, Vector3f&, f32);
    void makeVectorRotation(Vector3f&, Vector3f&);

    f32 dot(const Quatf& q) const {
        return w * w + q.x * q.x + q.y * q.y + q.z * q.z;
    }

    f32 length() const {
        return Math<f32>::sqrt(dot(*this));
    }

    void multScalar(f32 s) {
        w *= s;
        x *= s;
        y *= s;
        z *= s;
    }

    void setUnit() {
        set(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // union {
    //     Vector3f v;
    //     struct {
    //         f32 x, y, z;
    //     };
    // };
    f32 w;
};

} // namespace EGG