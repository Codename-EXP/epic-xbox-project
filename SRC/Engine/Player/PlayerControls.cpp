#include "PlayerControls.h"


static const float DEADZONE = 0.18f;
static const float MAX_STICK = 32768.0f;
static const float DZ_SQ = DEADZONE * DEADZONE;

inline void apply_circular_deadzone(float& x, float& y)
{
    float nx = x / MAX_STICK;
    float ny = y / MAX_STICK;

    float r2 = nx * nx + ny * ny;

    // Inside deadzone → zero
    if (r2 < DZ_SQ) {
        x = 0.0f;
        y = 0.0f;
        return;
    }
    return; 
}


void MoveCamera(camera& cam)
{
    int lx_i, ly_i, rx_i, ry_i;
    float lx, ly, rx, ry;
    Input_GetSticks(lx_i, ly_i, rx_i, ry_i);
    // perform deadzoning calcs

    lx = lx_i;
    ly = ly_i;
    rx = rx_i;
    ry = ry_i;
    apply_circular_deadzone(lx, ly);
    apply_circular_deadzone(rx, ry);

    // 

    D3DXVECTOR3 movement_vec;
    movement_vec.x = ly;
    movement_vec.z = lx;
    WORD buttons = Input_GetButtons();
    if (buttons & BTN_A) movement_vec.y = 32767;
    else
    if (buttons & BTN_B) movement_vec.y = -32768;
    else
    movement_vec.y = 0;

    cam.move_relative(movement_vec, rx * CAMERA_SENSITIVITY_X, ry * CAMERA_SENSITIVITY_Y);
}

