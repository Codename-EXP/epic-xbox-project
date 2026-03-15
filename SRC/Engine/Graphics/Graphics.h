#pragma once
#include <xtl.h>
#include "../Objects/Camera.h"
HRESULT InitD3D();
void Render();
void SetViewFromCamera(camera& cam);