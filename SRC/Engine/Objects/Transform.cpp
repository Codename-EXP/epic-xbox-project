
#include "Transform.h"

inline void transform_pos::update() {
    D3DXMatrixTranslation(&transform, pos.x, pos.y, pos.z);
}
inline void transform_pos_yaw::update() {
    D3DXMATRIX r, m;
    D3DXMatrixRotationY(&r, yaw);
    D3DXMatrixTranslation(&m, pos.x, pos.y, pos.z);
    transform = r * m;
}
inline void transform_pos_rot::update() {
    D3DXMATRIX r, m;
    D3DXMatrixRotationYawPitchRoll(&r, yaw, pitch, roll);
    D3DXMatrixTranslation(&m, pos.x, pos.y, pos.z);
    transform = r * m;
}
inline void transform_pos_rot_scale::update() {
    D3DXMATRIX s, r, m;
    D3DXMatrixScaling(&s, scale.x, scale.y, scale.z);
    D3DXMatrixRotationYawPitchRoll(&r, yaw, pitch, roll);
    D3DXMatrixTranslation(&m, pos.x, pos.y, pos.z);
    transform = s * r * m;
}