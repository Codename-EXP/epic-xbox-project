
/*// 
## compile into header file
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /H "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

## generate human readable shader assembly
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fl "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\vert1.hlsl"

//*/



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
#define point5 cb12.w



struct VS_INPUT
{
    float4 PosPack : POSITION;   // px,py,pz,uvx  (0–1)
    float4 NorPack : NORMAL;     // nx,ny,nz,uvy  (0–1)
    float4 TanPack : Diffuse;    // tx,ty,tz,PAD  (0–1)
};
struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 lightTS : COLOR0;
    float3 viewTS : COLOR1;
};
VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    // Unpack position (0–1 - world-space)
    float3 posWorld = g_PosMin + IN.PosPack.xyz * (g_PosMax - g_PosMin);
    // Transform position
    //OUT.Position = mul(float4(posWorld, 1.0f), g_mWVP);
    float4 posWS = mul(uWorld, float4(posWorld, one));
    OUT.Position = mul(uViewProj, posWS);
    
    // Unpack UV (already 0–1)
    OUT.TexCoord = float2(IN.PosPack.w, IN.NorPack.w);
    
    // Unpack normal (0–1 -> -1..1)
    float3 N = IN.NorPack.xyz * two - one;
    float3 T = IN.TanPack.xyz * two - one;
    
    //N = normalize(mul(N, (float3x3) uWorld));
    //T = normalize(mul(T, (float3x3) uWorld));
    N = normalize(N);
    T = normalize(T);
    
    // Orthogonalize tangent (Gram–Schmidt)
    //T = normalize(T - N * dot(N, T));
    
    //float3 B = normalize(cross(T, N));
    float handedness = IN.TanPack.w * two - one;
    float3 B = normalize(cross(T, N) * handedness);
    

    float3x3 TBN = float3x3(T, B, N);
    
    float3 L = normalize(uLightPosWS - posWS.xyz);
    float3 V = normalize(uCameraPosWS - posWS.xyz);
    
    OUT.lightTS = normalize(mul(TBN, L)) * point5 + point5;
    OUT.viewTS = normalize(mul(V, TBN)) * point5 + point5;
    return OUT;
}


// Build a tangent from the normal only (stable, but not UV‑aligned)
//float3 BuildTangent(float3 N)
//{
//    float3 up = abs(N.z) < point99 ? float3(zero, zero, one) : float3(zero, one, zero);
//    return normalize(cross(up, N));
//}
