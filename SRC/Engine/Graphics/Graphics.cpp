#include "Graphics.h"

#include "../../External/HLSL/vert1.h" 
#include "../../External/HLSL/font.h" 
#include "../../External/Blender/all_meshes_export.h" 
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D8             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE8       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER8 g_pVB = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER8 g_p2VB = NULL; // Buffer to hold vertices
LPDIRECT3DINDEXBUFFER8  g_pIB = NULL; // Buffer to hold indices
LPDIRECT3DINDEXBUFFER8  g_p2IB = NULL; // Buffer to hold indices

int test_font_count = 0;
LPDIRECT3DVERTEXBUFFER8 test_font_vb = 0;


LPDIRECT3DVERTEXBUFFER8 ConstructStringBuffer(char* string) {
    if (!string) return 0;

    char buffer[512];
    int i = 0;
    // i max value of 63

    while (char c = string[i]) {

        if (c >= 0x30) {
            if (c < 0x40) {
                c -= 22;
                goto submit;
            }
            else if ((c & ~0x20) >= 0x41 && (c & ~0x20) < 0x5B) {
                c -= 0x41;
                goto submit;
            }
        }
        switch (c) {
        case 0x20: // space 
            break;
        case 0x21: // !
            c = 43;
            break;
        case 0x22: // "
            c = 42;
            break;
        case 0x28: // (
        case 0x5b: // [
            c = 40;
            break;
        case 0x29: // )
        case 0x5d: // ]
            c = 41;
            break;
        case 0x2a: // *
            c = 47;
            break;
        case 0x2b: // +
            c = 45;
            break;
        case 0x2d: // -
            c = 44;
            break;
        case 0x2c: // ,
        case 0x2e: // .
            c = 36;
            break;
        case 0x3a: // :
            c = 38;
            break;
        case 0x3d: // =
            c = 46;
            break;
        case 0x5f: // _
            c = 37;
            break;
        default: // any unsupported characters
            c = 39;
            break;
        }

    submit:

        // create for vertices
        buffer[i * 8    ] = (char)i | 0b0'0000000;
        buffer[i * 8 + 1] = c       | 0b0'0000000;
        buffer[i * 8 + 2] = (char)i | 0b0'0000000;
        buffer[i * 8 + 3] = c       | 0b1'0000000;
        buffer[i * 8 + 4] = (char)i | 0b1'0000000;
        buffer[i * 8 + 5] = c       | 0b1'0000000;
        buffer[i * 8 + 6] = (char)i | 0b1'0000000;
        buffer[i * 8 + 7] = c       | 0b0'0000000;



        i += 1;
        if (i == 64) break;
    }


    LPDIRECT3DVERTEXBUFFER8 result_vb = NULL;

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(i * 8, 0, 0, 0, &result_vb))) {

    }

    void* vert_buffer;
    if (FAILED(result_vb->Lock(0, 0, (BYTE**)&vert_buffer, 0))) {

    }
    memcpy(vert_buffer, buffer, i * 8);
    result_vb->Unlock();

    test_font_count = i;
    return result_vb;
}



static LPDIRECT3DTEXTURE8 s_smokeTex = NULL;
static void LoadSmokeTexture()
{
    if (s_smokeTex || !g_pd3dDevice) return;

    const char* p0 = "D:\\test\\grass.dds";
    const char* p1 = "D:\\test\\font32.dds";

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

    test_font_vb = ConstructStringBuffer("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._:@[]\"!-+=*");

    return S_OK;
}

DWORD s_vsHandle = 0;
const DWORD s_vsDecl[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION,  D3DVSDT_D3DCOLOR),
    D3DVSD_REG(D3DVSDE_NORMAL,   D3DVSDT_D3DCOLOR),
    D3DVSD_END()
};

DWORD s_vs2Handle = 0;
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


    if (FAILED(g_pd3dDevice->CreateVertexShader(s_vsDecl, dwVert1VertexShader, &s_vsHandle, 0)))
        return E_FAIL;
    if (FAILED(g_pd3dDevice->CreateVertexShader(s_vsDecl2, dwFontVertexShader, &s_vs2Handle, 0)))
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



    //D3DXMATRIX matRotate;
    //D3DXMatrixRotationYawPitchRoll(&matRotate, 0.0f, 0.0f, 0.02f);
    //D3DXMatrixMultiply(&triangle_mat, &triangle_mat, &matRotate);
    //g_pd3dDevice->SetTransform(D3DTS_WORLD, &triangle_mat);


    // Set FVF and stream source
    g_pd3dDevice->SetVertexShader(s_vsHandle);
    //g_pd3dDevice->SetStreamSource(0, g_pVB, 8);
    //g_pd3dDevice->SetIndices(g_pIB, 0);
    g_pd3dDevice->SetTexture(0, s_smokeTex);

    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);




    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    // disable subsequent stages
    g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    // Ensure simple lighting settings
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00FFFFFF);

    D3DXMATRIX world, view, proj, wvp;
    //g_pd3dDevice->GetTransform(D3DTS_WORLD, &world);
    //g_pd3dDevice->GetTransform(D3DTS_VIEW, &view);
    //g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);
    //wvp = world * view * proj;
    //g_pd3dDevice->SetVertexShaderConstant(0, &wvp, 4);
    float c6[4] = { 1.0f, 2.0f, 0.0f, 0.0f };
    //g_pd3dDevice->SetVertexShaderConstant(4, g_Cube_PosMin, 1);
    //g_pd3dDevice->SetVertexShaderConstant(5, g_Cube_PosMax, 1);
    //g_pd3dDevice->SetVertexShaderConstant(6, c6, 1);


    //g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    //g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,0, 0, g_Cube_IndexCount);

    D3DXMATRIX quad_mat;
    D3DXMatrixIdentity(&quad_mat);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &quad_mat);
    g_pd3dDevice->SetStreamSource(0, g_p2VB, 8);
    g_pd3dDevice->SetIndices(g_p2IB, 0);




    g_pd3dDevice->GetTransform(D3DTS_VIEW, &view);
    g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);


    // compute wvp as you already do

    wvp = quad_mat * view * proj;
    // transpose for shader constant layout
    D3DXMATRIX wvpT;
    D3DXMatrixTranspose(&wvpT, &wvp);
    g_pd3dDevice->SetVertexShaderConstant(0, &wvpT, 4);
    g_pd3dDevice->SetVertexShaderConstant(4, g_Cube001_PosMin, 1);
    g_pd3dDevice->SetVertexShaderConstant(5, g_Cube001_PosMax, 1);
    g_pd3dDevice->SetVertexShaderConstant(6, c6, 1);



    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 0, g_Cube001_IndexCount / 3);

    g_pd3dDevice->SetIndices(0, 0);




    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    //g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    float c1[4] =
    {
        4.0f,   // glyph width in pixels
        5.0f,  // glyph height in pixels
        2.0f / 640.0f,  // screenScaleX (convert pixels → clip space)
        2.0f / 480.0f   // screenScaleY
    };
    float c2[4] =
    {
        4.0f / 32.0f,   // tileWidthUV
        5.0f / 32.0f,   // tileHeightUV
        8.0f,          // tilesPerRow
        6.0f // glyphWidthPixelsSpacer
    };
    g_pd3dDevice->SetVertexShader(s_vs2Handle);
    g_pd3dDevice->SetStreamSource(0, test_font_vb, 2);
    float c3[4] = { 0.0f, 1.0f, 128.0f, 255.0f };
    float c4[4] = { 0.1f, 1.0f, 0.0f, 1.0f };
    g_pd3dDevice->SetVertexShaderConstant(0, c1, 4);
    g_pd3dDevice->SetVertexShaderConstant(1, c2, 1);
    g_pd3dDevice->SetVertexShaderConstant(2, c3, 1);
    g_pd3dDevice->SetVertexShaderConstant(3, c4, 1);
    g_pd3dDevice->DrawPrimitive(D3DPT_QUADLIST, 0, test_font_count);



    g_pd3dDevice->SetTexture(0, NULL);


    // End the scene
    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

