#include "MBody.h"


void MBody::UpdateBody(float deltaTime) {

    if (BodyType == TypeBody::bStatic || isSleeping)
        return;

    if (UseGravity) {
        const Vector3 gravity(0.0f, -9.81f, 0.0f);
        Velocity += gravity * deltaTime;
    }

    Position += Velocity * deltaTime;

    float angularSpeed = AngularVelocity.length();
    if (angularSpeed > 0.0f) {
        Vector3 axis = AngularVelocity.normalized();
        float angle = angularSpeed * deltaTime;

        Quaternion spin = Quaternion::fromAxisAngle(axis, angle);
        Rotation = (spin * Rotation).normalized();
    }
}