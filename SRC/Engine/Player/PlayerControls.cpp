#include "PlayerControls.h"


void MoveCamera(camera& cam)
{
    int lx, ly, rx, ry;
    Input_GetSticks(lx, ly, rx, ry);

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

