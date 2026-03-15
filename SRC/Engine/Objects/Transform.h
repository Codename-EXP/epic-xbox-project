#pragma once
#include <xtl.h>


// NOTE: we update the matrix of a transform after moving it, so we dont have to recalculate an objects matrix each render
//       this also supports our multiple 

// NOTE: all transform related data must be contained here
//       this is so we can manage accordingly when migrating to fixed u32 based positions

struct transform_pos {
    inline void update();
    D3DXVECTOR3 pos;
    D3DXMATRIX transform;
};
struct transform_pos_yaw {
    inline void update();
    D3DXVECTOR3 pos;
    float yaw;
    D3DXMATRIX transform;
};
struct transform_pos_rot {
    inline void update();
    D3DXVECTOR3 pos;
    float yaw;
    float pitch;
    float roll;
    D3DXMATRIX transform;
};
struct transform_pos_rot_scale {
    inline void update();
    D3DXVECTOR3 pos;
    float yaw;
    float pitch;
    float roll;
    D3DXVECTOR3 scale;
    D3DXMATRIX transform;
};

// other types of transforms used by cameras and other gizmos
struct transform_camera {
    D3DXVECTOR3 pos;
    float yaw;
    float pitch;
};