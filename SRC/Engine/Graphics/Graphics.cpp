#include "Graphics.h"


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D8             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER8 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER8 g_p2VB = NULL; // Buffer to hold vertices

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    float x, y, z; // The vertex position
    DWORD color;   // The vertex color
    float u, v;
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)


static LPDIRECT3DTEXTURE8 s_smokeTex = NULL;
static void LoadSmokeTexture()
{
    if (s_smokeTex || !g_pd3dDevice) return;

    const char* p0 = "D:\\test\\grass.dds";
    const char* p1 = "D:\\chrome_diff.dds";

    if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, p1, &s_smokeTex))) {

        if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice, p0, &s_smokeTex))) {

            for (;;) {
                p0++;
            }
        }
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
    // Initialize three vertices for rendering a triangle
    CUSTOMVERTEX g_Vertices[] =
    {
        // Use opaque colors (0xAARRGGBB) and place triangle at z = -1 in world
        {  0.0f, -1.1547f, -1.0f, 0xFFFF0000, 0.0f, 1.0f }, // x, y, z, color
        { -1.0f,  0.5777f, -1.0f, 0xFF00FFFF, 1.0f, 0.0f },
        {  1.0f,  0.5777f, -1.0f, 0xFF00FF00, 0.0f, 0.0f },
    };
    // Initialize three vertices for rendering a square
    CUSTOMVERTEX g_2Vertices[] =
    {
        {  5.0f, -1.0f,  5.0f, 0xFFFF0000, 0.0f, 0.0f }, // x, y, z, color
        {  5.0f, -1.0f, -5.0f, 0xFF00FF00, 0.0f, 1.0f },
        { -5.0f, -1.0f,  5.0f, 0xFF00FFFF, 1.0f, 0.0f },


        { -5.0f, -1.0f,  5.0f, 0xFF00FFFF, 1.0f, 0.0f }, // x, y, z, color
        {  5.0f, -1.0f, -5.0f, 0xFF00FF00, 0.0f, 1.0f },
        { -5.0f, -1.0f, -5.0f, 0xFFFF0000, 1.0f, 1.0f },
    };

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. We also
    // specify the FVF, so the vertex buffer knows what data it contains.
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
        D3DUSAGE_WRITEONLY,
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED, &g_pVB)))
        return E_FAIL;

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX),
        D3DUSAGE_WRITEONLY,
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED, &g_p2VB)))
        return E_FAIL;

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required because the
    // vertex buffer may still be in use by the GPU. This can happen if the
    // CPU gets ahead of the GPU. The GPU could still be rendering the previous
    // frame.

    CUSTOMVERTEX* pVertices;
    if (FAILED(g_pVB->Lock(0, 0, (BYTE**)&pVertices, 0)))
        return E_FAIL;
    memcpy(pVertices, g_Vertices, 3 * sizeof(CUSTOMVERTEX));
    g_pVB->Unlock();

    CUSTOMVERTEX* p2Vertices;
    if (FAILED(g_p2VB->Lock(0, 0, (BYTE**)&p2Vertices, 0)))
        return E_FAIL;
    memcpy(p2Vertices, g_2Vertices, 6 * sizeof(CUSTOMVERTEX));
    g_p2VB->Unlock();

    D3DXMatrixIdentity(&triangle_mat);

    LoadSmokeTexture();

    return S_OK;
}



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
void Render()
{
    // Clear the backbuffer to a blue color
    g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
        D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0L);

    // Begin the scene
    g_pd3dDevice->BeginScene();



    D3DXMATRIX matRotate;
    D3DXMatrixRotationYawPitchRoll(&matRotate, 0.0f, 0.0f, 0.02f);
    D3DXMatrixMultiply(&triangle_mat, &triangle_mat, &matRotate);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &triangle_mat);

    //// Ensure viewport is set and device is ready
    //D3DVIEWPORT8 vp;
    //vp.X = 0; vp.Y = 0; vp.Width = 640; vp.Height = 480; vp.MinZ = 0.0f; vp.MaxZ = 1.0f;
    //g_pd3dDevice->SetViewport(&vp);

    // Set FVF and stream source
    g_pd3dDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX);
    g_pd3dDevice->SetStreamSource(0, g_pVB, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetTexture(0, s_smokeTex);

    // Ensure common render states for solid geometry
    //g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    //g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    //g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    //g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    //// Bind texture if available; otherwise rely on vertex diffuse color
    //if (s_smokeTex) {
    //    // Modulate texture with vertex color
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    //} else {
    //    g_pd3dDevice->SetTexture(0, NULL);
    //    // Use vertex diffuse color only so geometry is visible without a texture
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    //    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    //}
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


    //g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    //g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    //g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);




        g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);


    D3DXMATRIX quad_mat;
    D3DXMatrixIdentity(&quad_mat);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &quad_mat);
    g_pd3dDevice->SetStreamSource(0, g_p2VB, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    g_pd3dDevice->SetTexture(0, NULL);
    // End the scene
    g_pd3dDevice->EndScene();
    // Present the backbuffer contents to the display
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

//static void SetupSmokeStates()
//{
//
//    g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
//
//    g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//    g_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//    g_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//
//    g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//    g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//    g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
//
//    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
//    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
//    g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
//}
//
//static void EndSmokeStates()
//{
//    g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
//}