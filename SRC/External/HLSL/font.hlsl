/*// 
## compile into header file
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /H "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.hlsl"

## generate human readable shader assembly
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fl "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.hlsl"

## get commands list
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe

//*/
float4 const_values : register(c0);
//     const_values.x = glyphWidthPixels
//     const_values.y = glyphHeightPixels
//     const_values.z = screenScaleX
//     const_values.w = screenScaleY

float4 atlas_values : register(c1);
//     atlas_values.x = tileWidthUV
//     atlas_values.y = tileHeightUV
//     atlas_values.z = tilesPerRow
//     atlas_values.w = glyphWidthPixelsSpacer 

float4 const_values_2 : register(c2);
//     const_values_2.x = 0.0
//     const_values_2.y = 1.0
//     const_values_2.z = 128.0 (2^7) — used as divisor for corner decode
//     const_values_2.w = 255.0  (byte → float scale)

float4 text_color : register(c3);

struct VS_INPUT
{
    float2 PosPack : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    // normalized to 255 range
    float packed = input.PosPack.x * const_values_2.w;
    float packed_2 = input.PosPack.y * const_values_2.w;
    
    float charIndex = fmod(packed, const_values_2.z); // % 128
    float glyphIndex = fmod(packed_2, const_values_2.z); // % 128
    
    float cornerX = floor(packed / const_values_2.z); // /128
    float cornerY = floor(packed_2 / const_values_2.z); // /128
    float2 cornerOffset = float2(cornerX, cornerY);
    
    float2 basePos = float2(charIndex * atlas_values.w, const_values_2.x);

    float2 pixelPos = basePos + cornerOffset * float2(const_values.x, const_values.y);

    float2 clipPos = pixelPos * float2(const_values.z, const_values.w) - float2(const_values_2.y, const_values_2.y);

    o.Position = float4(clipPos, const_values_2.x, const_values_2.y);
    
    // causing issues ??
    float tileX = fmod(glyphIndex, atlas_values.z);
    float tileY = floor(glyphIndex / atlas_values.z);

    float2 tileOrigin = float2(tileX * atlas_values.x, tileY * atlas_values.y);

    float inverted_y = const_values_2.y - cornerY;
    float2 inverted_cornerOffset = float2(cornerX, inverted_y);
    
    
    o.TexCoord = tileOrigin + inverted_cornerOffset * float2(atlas_values.x, atlas_values.y);
    o.Color = text_color;
    return o;
}
