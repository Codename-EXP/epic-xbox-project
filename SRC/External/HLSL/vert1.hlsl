
/*// 

## copy paste into cmd to compile!!!
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

## compile into header file
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /H "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"



## generate human readable shader assembly
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fl "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

  
## get commands list
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe

C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fc "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"



//*/


float4x4 g_mWVP : register(c0); // (c0–c3)
float3 g_PosMin : register(c4);
float3 g_PosMax : register(c5);
float4 const_values : register(c6); // { x:1.00 y:2.00 }

struct VS_INPUT
{
    float4 PosPack : POSITION;   // px,py,pz,uvx  (0–1)
    float4 NorPack : NORMAL;     // nx,ny,nz,uvy  (0–1)
};
struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    //float3 Normal : NORMAL;
};
VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Unpack position (0–1 → world-space)
    float3 posWorld = g_PosMin + IN.PosPack.xyz * (g_PosMax - g_PosMin);
    
    // Unpack UV (already 0–1)
    OUT.TexCoord.x = IN.PosPack.w;
    OUT.TexCoord.y = IN.NorPack.w;
    
    // Unpack normal (0–1 → -1..1)
    float3 normal = IN.NorPack.xyz * const_values.y - const_values.x;
    
    // Transform position
    OUT.Position = mul(float4(posWorld, const_values.x), g_mWVP);
    
    // Pass through UV + normal
    //OUT.TexCoord = uv;
    //OUT.Normal   = normal;

    return OUT;
}
















