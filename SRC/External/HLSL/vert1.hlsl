
/*// 
## compile into header file
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /H "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

## generate human readable shader assembly
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fl "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

//*/


//float4x4 g_mWVP : register(c0); // (c0–c3)

float4x4 uWorld : register(c0);
float4x4 uViewProj : register(c4);
float3 uLightPosWS : register(c8);
float3 uCameraPosWS : register(c9);
float3 g_PosMin : register(c10);
float3 g_PosMax : register(c11);
float4 cb12 : register(c12); // { x:0.00 y:1.00 z:2.00 w:0.999f}
#define zero cb12.x
#define one cb12.y
#define two cb12.z
#define point99 cb12.w



struct VS_INPUT
{
    float4 PosPack : POSITION;   // px,py,pz,uvx  (0–1)
    float4 NorPack : NORMAL;     // nx,ny,nz,uvy  (0–1)
};
struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 lightTS : COLOR0;
    float3 viewTS : COLOR1;
};
// Build a tangent from the normal only (stable, but not UV‑aligned)
float3 BuildTangent(float3 N)
{
    float3 up = abs(N.z) < point99 ? float3(zero, zero, one) : float3(zero, one, zero);
    return normalize(cross(up, N));
}
VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Unpack position (0–1 - world-space)
    float3 posWorld = g_PosMin + IN.PosPack.xyz * (g_PosMax - g_PosMin);
    // Transform position
    //OUT.Position = mul(float4(posWorld, 1.0f), g_mWVP);
    float4 posWS = mul(float4(posWorld, one), uWorld);
    OUT.Position = mul(posWS, uViewProj);
    
    // Unpack UV (already 0–1)
    OUT.TexCoord.x = IN.PosPack.w;
    OUT.TexCoord.y = IN.NorPack.w;
    
    // Unpack normal (0–1 -> -1..1)
    float3 normal = IN.NorPack.xyz * two - one;
    
    float3 N = normalize(mul(normal, (float3x3) uWorld));
    float3 T = BuildTangent(N);
    float3 B = normalize(cross(T, N));

    float3x3 TBN = float3x3(T, B, N);

    float3 L = normalize(uLightPosWS);
    float3 V = normalize(uCameraPosWS);
    
    OUT.lightTS = mul(TBN, L);
    OUT.viewTS = mul(TBN, V);

    return OUT;
}






