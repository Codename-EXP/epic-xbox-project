#pragma once
#include <xtl.h>
#include "../Objects/Transform.h"

struct camera {
#define MOVE_SPEED 0.000001f
#define MAX_LOOK_ANGLE 1.396263f
#define MIN_LOOK_ANGLE -1.48353f
    transform_camera transform;
    void move_relative(D3DXVECTOR3& pos_change, float yaw_change, float pitch_change);
    void init();
};


