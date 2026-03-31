#include "Graphics.h"

#include "../../External/HLSL/font.h" 
#include "../../External/HLSL/fontPS.h" 

#include "../../External/HLSL/vert1.h" 
#include "../../External/HLSL/vert1PS.h" 

#include "../../External/HLSL/gizmoVS.h" 
#include "../../External/HLSL/gizmoPS.h" 

#include "../../External/Blender/all_meshes_export.h" 
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D8             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8 g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER8 g_p2VB = NULL; // Buffer to hold vertices
LPDIRECT3DINDEXBUFFER8  g_pIB = NULL; // Buffer to hold indices
LPDIRECT3DINDEXBUFFER8  g_p2IB = NULL; // Buffer to hold indices

LPDIRECT3DDEVICE8 Graphics_GetD3D() {
    return g_pd3dDevice;
}



static LPDIRECT3DTEXTURE8 s_smokeTex = NULL;
static LPDIRECT3DTEXTURE8 s_smoke1Tex = NULL;
static LPDIRECT3DTEXTURE8 s_smoke2Tex = NULL;
static void LoadSmokeTexture()
{
    if (s_smokeTex || !g_pd3dDevice) return;


    if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, "D:\\test\\font32_64.dds", &s_smokeTex))) {
        Log("failed to load font texture", log_red);
    }
    if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, "D:\\test\\BricksDiff.dds", &s_smoke1Tex))) {
        Log("failed to load grass diffuse texture", log_red);
    }
    if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, "D:\\test\\BricksNormInvert.dds", &s_smoke2Tex))) {
        Log("failed to load grass normal texture", log_red);
    }
}

//-----------------------------------------------------------------------------
// Name: InitVB()
// Desc: Creates a vertex buffer and fills it with our vertices. The vertex
//       buffer is basically just a chunk of memory that holds vertices. After
//       creating it, we must Lock()/Unlock() it to fill it.
//-----------------------------------------------------------------------------
D3DXMATRIX triangle_mat;
HRESULT InitVB()
{

    // setup vertex buffers

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(sizeof(g_Cube_Vertices), 0, 0, 0, &g_pVB)))
        return E_FAIL;

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(sizeof(g_Cube001_Vertices), 0, 0, 0, &g_p2VB)))
        return E_FAIL;

    void* vert_buffer;
    if (FAILED(g_pVB->Lock(0, 0, (BYTE**)&vert_buffer, 0)))
        return E_FAIL;
    memcpy(vert_buffer, g_Cube_Vertices, sizeof(g_Cube_Vertices));
    g_pVB->Unlock();

    if (FAILED(g_p2VB->Lock(0, 0, (BYTE**)&vert_buffer, 0)))
        return E_FAIL;
    memcpy(vert_buffer, g_Cube001_Vertices, sizeof(g_Cube001_Vertices));
    g_p2VB->Unlock();


    // then do index buffers

    if (FAILED(g_pd3dDevice->CreateIndexBuffer(sizeof(g_Cube_Indices), 0, D3DFMT_UNKNOWN, 0, &g_pIB)))
        return E_FAIL;

    if (FAILED(g_pd3dDevice->CreateIndexBuffer(sizeof(g_Cube001_Indices), 0, D3DFMT_UNKNOWN, 0, &g_p2IB)))
        return E_FAIL;

    void* index_buffer;
    if (FAILED(g_pIB->Lock(0, 0, (BYTE**)&index_buffer, 0)))
        return E_FAIL;
    memcpy(index_buffer, g_Cube_Indices, sizeof(g_Cube_Indices));
    g_pIB->Unlock();

    if (FAILED(g_p2IB->Lock(0, 0, (BYTE**)&index_buffer, 0)))
        return E_FAIL;
    memcpy(index_buffer, g_Cube001_Indices, sizeof(g_Cube001_Indices));
    g_p2IB->Unlock();


    D3DXMatrixIdentity(&triangle_mat);

    LoadSmokeTexture();

    Log("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._:@[]\"!-+=*", log_white); 
    Log("HELLO WORLD !! 12345: ", log_green, true, (void*)12345678); // BC614E
    Log("green numbers: ", log_green, true, (void*)0x34128967);
    Log("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._:@[]\"!-+=*", log_white);
    Log("red symbols: ABC0123456789._:@[]\"!-+=*", log_red);
    Log("string join test", log_green, false, "2nd half");

    return S_OK;
}
DWORD gizmo_vsHandle = 0;
DWORD gizmo_psHandle = 0;

DWORD s_vsHandle = 0;
DWORD s_psHandle = 0;
const DWORD s_vsDecl[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION,  D3DVSDT_PBYTE4),
    D3DVSD_REG(D3DVSDE_NORMAL,   D3DVSDT_PBYTE4),
    D3DVSD_REG(D3DVSDE_DIFFUSE,   D3DVSDT_PBYTE4),
    D3DVSD_END()
};

DWORD s_vs2Handle = 0;
DWORD s_ps2Handle = 0;
const DWORD s_vsDecl2[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION,  D3DVSDT_PBYTE2),
    D3DVSD_END()
};

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D()
{
    // Create the D3D object.
    if (NULL == (g_pD3D = Direct3DCreate8(D3D_SDK_VERSION)))
        return E_FAIL;

    // Set up the structure used to create the D3DDevice.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.BackBufferWidth = 640;
    d3dpp.BackBufferHeight = 480;
    d3dpp.BackBufferFormat = D3DFMT_LIN_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;

    // Create the Direct3D device.
    if (FAILED(g_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &d3dpp, &g_pd3dDevice)))
        return E_FAIL;


    if (FAILED(g_pd3dDevice->CreateVertexShader(s_vsDecl, dwGizmoVSVertexShader, &gizmo_vsHandle, 0)))
        return E_FAIL;
    if (FAILED(g_pd3dDevice->CreateVertexShader(s_vsDecl, dwVert1VertexShader, &s_vsHandle, 0)))
        return E_FAIL;
    if (FAILED(g_pd3dDevice->CreateVertexShader(s_vsDecl2, dwFontVertexShader, &s_vs2Handle, 0)))
        return E_FAIL;

    if ((g_pd3dDevice->CreatePixelShader((D3DPIXELSHADERDEF*)&dwGizmoPSPixelShader, &gizmo_psHandle)))
        return E_FAIL;

    if ((g_pd3dDevice->CreatePixelShader((D3DPIXELSHADERDEF*)&dwVert1psPixelShader, &s_psHandle)))
        return E_FAIL;

    if ((g_pd3dDevice->CreatePixelShader((D3DPIXELSHADERDEF*)&dwFontPSPixelShader, &s_ps2Handle)))
        return E_FAIL;


    // Initialize the vertex buffer
    return InitVB();
}



void SetViewFromCamera(camera& cam)
{
    // Build forward vector from yaw + pitch
    D3DXVECTOR3 forward;
    forward.x = cosf(cam.transform.pitch) * sinf(cam.transform.yaw);
    forward.y = sinf(cam.transform.pitch);
    forward.z = cosf(cam.transform.pitch) * cosf(cam.transform.yaw);

    // Look-at target = eye + forward
    D3DXVECTOR3 at = cam.transform.pos + forward;

    // Standard world up
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

    // Build view matrix
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH(&matView, &cam.transform.pos, &at, &up);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    // proj
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(90.0f), 640.0f / 480.0f, 0.1f, 50.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
void Render(camera& main_camera)
{
    // Clear the backbuffer to a blue color
    g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
        D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0L);

    // Begin the scene
    g_pd3dDevice->BeginScene();



    //D3DXMATRIX matRotate;
    //D3DXMatrixRotationYawPitchRoll(&matRotate, 0.0f, 0.0f, 0.02f);
    //D3DXMatrixMultiply(&triangle_mat, &triangle_mat, &matRotate);
    //g_pd3dDevice->SetTransform(D3DTS_WORLD, &triangle_mat);


    // Set FVF and stream source
    g_pd3dDevice->SetVertexShader(s_vsHandle);
    g_pd3dDevice->SetPixelShader(s_psHandle);
    //g_pd3dDevice->SetStreamSource(0, g_pVB, 8);
    //g_pd3dDevice->SetIndices(g_pIB, 0);
    g_pd3dDevice->SetTexture(0, s_smoke2Tex);
    g_pd3dDevice->SetTexture(1, s_smoke1Tex);

    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);




    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    //
    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    // disable subsequent stages
    //g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    //g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    // Ensure simple lighting settings
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00FFFFFF);
    g_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);


    //g_pd3dDevice->GetTransform(D3DTS_WORLD, &world);
    //g_pd3dDevice->GetTransform(D3DTS_VIEW, &view);
    //g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);
    //wvp = world * view * proj;
    //g_pd3dDevice->SetVertexShaderConstant(0, &wvp, 4);
    //g_pd3dDevice->SetVertexShaderConstant(4, g_Cube_PosMin, 1);
    //g_pd3dDevice->SetVertexShaderConstant(5, g_Cube_PosMax, 1);
    //g_pd3dDevice->SetVertexShaderConstant(6, c6, 1);


    //g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    //g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,0, 0, g_Cube_IndexCount);

    D3DXMATRIX quad_mat;
    static float quad_uppies = 0.0f;

    quad_uppies += 0.01f;
    //quad_uppies += 0.00f;


    //D3DXMatrixTranslation(&quad_mat, 0.0f, quad_uppies, 0.0f);
    D3DXMatrixRotationYawPitchRoll(&quad_mat, quad_uppies, 0, 0);
    //D3DXMatrixIdentity(&quad_mat);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &quad_mat);
    g_pd3dDevice->SetStreamSource(0, g_p2VB, 12);
    g_pd3dDevice->SetIndices(g_p2IB, 0);




    D3DXMATRIX world, view, proj, wvp;
    g_pd3dDevice->GetTransform(D3DTS_VIEW, &view);
    g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);


    // compute wvp as you already do

    wvp = view * proj;

    float c12[4] = { 0.0f, 1.0f, 2.0f, 0.5f,};
    float c9[4] = { main_camera.transform.pos.x, main_camera.transform.pos.y, main_camera.transform.pos.z, 1.0f }; // camera


    
    static float light_angle = 0.0f;
    light_angle += 0.00f;
    //light_angle += 0.01f;
    float radius = 2.2f;
    float lx = cosf(light_angle) * radius;
    float lz = sinf(light_angle) * radius;
    float ly = 2.f;  
    float c8[4] = { lx,ly,lz, 0.0f };

    g_pd3dDevice->SetVertexShaderConstant(0, &quad_mat, 4);
    g_pd3dDevice->SetVertexShaderConstant(4, &wvp, 4);
    g_pd3dDevice->SetVertexShaderConstant(8, c8, 1);
    g_pd3dDevice->SetVertexShaderConstant(9, c9, 1);
    g_pd3dDevice->SetVertexShaderConstant(10, g_Cube001_PosMin, 1);
    g_pd3dDevice->SetVertexShaderConstant(11, g_Cube001_PosMax, 1);
    g_pd3dDevice->SetVertexShaderConstant(12, c12, 1);

    float ps_c0[4] = { 0.9f, 0.9f, 0.9f, 1.0f, };
    float ps_c1[4] = { 0.1f, 0.1f, 0.1f, 1.0f, };
    g_pd3dDevice->SetPixelShaderConstant(0, ps_c0, 1);
    g_pd3dDevice->SetPixelShaderConstant(1, ps_c1, 1);


    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, g_Cube001_IndexCount / 3);






    // draw light gizmo
    {
        g_pd3dDevice->SetVertexShader(gizmo_vsHandle);
        g_pd3dDevice->SetPixelShader(gizmo_psHandle);

        D3DXMATRIX gizmo_mat;
        D3DXMatrixTranslation(&gizmo_mat, lx, ly, lz);
        //D3DXMatrixIdentity(&gizmo_mat);
        g_pd3dDevice->SetTransform(D3DTS_WORLD, &gizmo_mat);
        g_pd3dDevice->SetStreamSource(0, g_pVB, 12);
        g_pd3dDevice->SetIndices(g_pIB, 0);

        D3DXMATRIX wvp = gizmo_mat * view * proj;
        D3DXMATRIX wvpT3;
        D3DXMatrixTranspose(&wvpT3, &wvp);

        float gizmo_c6[4] = { 1.0f, 2.0f, 0,0};

        g_pd3dDevice->SetVertexShaderConstant(0, &wvpT3, 4);
        g_pd3dDevice->SetVertexShaderConstant(4, g_Cube_PosMin, 1);
        g_pd3dDevice->SetVertexShaderConstant(5, g_Cube_PosMax, 1);
        g_pd3dDevice->SetVertexShaderConstant(6, gizmo_c6, 1);

        g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, g_Cube_IndexCount / 3);
    }


    // render console log
    {
        g_pd3dDevice->SetTexture(0, s_smokeTex);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 10);     // 0-255 alpha threshold
        g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

        g_pd3dDevice->SetVertexShader(s_vs2Handle);
        g_pd3dDevice->SetPixelShader(s_ps2Handle);
        Log_Render();
    }



    g_pd3dDevice->SetPixelShader(0);
    g_pd3dDevice->SetTexture(0, NULL);

    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    // End the scene
    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

