
#include <xtl.h>
#include "Engine/Objects/Transform.h"
#include "Engine/Input/input.h"
#include "Engine/Player/PlayerControls.h"
#include "Engine/Objects/Camera.h"
#include "Engine/Graphics/Graphics.h"

camera main_camera;



//-----------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
void __cdecl main()
{
    // Initialize Direct3D
    if (FAILED(InitD3D()))
        return;

    Input_Init();
    main_camera.init();


    while (TRUE)
    {
        Input_Poll();

        MoveCamera(main_camera);
        //SetViewFromCamera(main_camera);
        PositionCamera(main_camera);
        // Render the scene
        Render();

    }
    //int c = connect(1,NULL,3);
}



