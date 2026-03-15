#pragma once
#include <xtl.h>
#define MAX_PORTS 4
#define ANALOG_THRESHOLD 30       // 0..255 analog-button threshold
enum
{
    BTN_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
    BTN_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
    BTN_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
    BTN_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,

    BTN_START = XINPUT_GAMEPAD_START,
    BTN_BACK = XINPUT_GAMEPAD_BACK,
    BTN_LTHUMB = XINPUT_GAMEPAD_LEFT_THUMB,
    BTN_RTHUMB = XINPUT_GAMEPAD_RIGHT_THUMB,

    // Synthetic analog-button digital flags
    BTN_A = 0x1000,
    BTN_B = 0x2000,
    BTN_X = 0x4000,
    BTN_Y = 0x8000,
};
void Input_Init();
void Input_Poll();

// Returns OR of all controller button masks (BTN_* flags above).
WORD Input_GetButtons();

//   lx,ly = left stick   (-32768 .. 32767)
//   rx,ry = right stick  (-32768 .. 32767)
void Input_GetSticks(int& lx, int& ly, int& rx, int& ry);
