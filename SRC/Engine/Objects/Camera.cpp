#include "Camera.h"


void camera::move_relative(D3DXVECTOR3& pos_change, float yaw_change, float pitch_change) {

    // Forward vector (XZ plane)
    float fx = sinf(transform.yaw);
    float fz = cosf(transform.yaw);

    // Right vector (perpendicular)
    float rxv = cosf(transform.yaw);
    float rzv = -sinf(transform.yaw);

    // forward/back
    transform.pos.x += fx * (pos_change.x * MOVE_SPEED);
    transform.pos.z += fz * (pos_change.x * MOVE_SPEED);

    // strafe
    transform.pos.x += fz * (pos_change.z * MOVE_SPEED);
    transform.pos.z += -fx * (pos_change.z * MOVE_SPEED);

    // up/down
    transform.pos.y += pos_change.y * MOVE_SPEED;

    // yaw
    transform.yaw += yaw_change;

    // pitch
    transform.pitch += pitch_change;
    if (transform.pitch > MAX_LOOK_ANGLE)
        transform.pitch = MAX_LOOK_ANGLE;
    else
    if (transform.pitch < MIN_LOOK_ANGLE)
        transform.pitch = MIN_LOOK_ANGLE;

}

void camera::init() {
    // Place camera back along -Z so it looks toward +Z where scene geometry is placed
    transform.pos = { 0.0f, 0.0f, -7.0f };
    transform.pitch = 0.0f;
    transform.yaw = 0.0f;
}

