#pragma once
#include <xtl.h>
#include "../Objects/Camera.h"
#include "../Logging/Log.h"

HRESULT InitD3D();
void Render();
void SetViewFromCamera(camera& cam);
LPDIRECT3DDEVICE8 Graphics_GetD3D();