/*// 
## compile into header file
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /H "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.hlsl"

## generate human readable shader assembly
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe /Fl "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.txt" "C:\Users\ct770\Downloads\CODENAME_MEXP\vs26 xbox test\Original Xbox Game1\SRC\External\HLSL\font.hlsl"

## get commands list
C:\Users\ct770\Downloads\CODENAME_MEXP\rxdk\RXDK\xbox\bin\xfxc.exe

//*/
float4 cb0 : register(c0);
float4 cb1 : register(c1);
float4 cb2 : register(c2);
float4 text_color : register(c3); 

#define glyphWidthPixels cb0.x
#define glyphHeightPixels cb0.y
#define tileWidthUV cb0.z
#define tileHeightUV cb0.w

#define tilesPerRow cb1.x
#define one cb1.y
#define highest_char_bit cb1.z  // == 128
#define max_char cb1.w          // == 255

#define zero cb2.x
#define screenScaleX cb2.y              // can change per line
#define screenScaleY cb2.z              // can change per line
#define glyphWidthPixelsSpacer cb2.w    // can change per line
#define screen_y_offset text_color.w   



struct VS_INPUT
{
    float2 PosPack : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Color : COLOR0; // note ensure alpha vertex color blending is disabled, or it could have weird effects
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    // normalized to 255 range
    float packed = input.PosPack.x * max_char;
    float packed_2 = input.PosPack.y * max_char;
    
    // extract char position & glyph index, %= 128
    float charIndex = fmod(packed, highest_char_bit); 
    float glyphIndex = fmod(packed_2, highest_char_bit); 
    // extract corner IDs, /= 128
    float cornerX = floor(packed / highest_char_bit); 
    float cornerY = floor(packed_2 / highest_char_bit); 
    float2 cornerOffset = float2(cornerX, cornerY);
    
    // find default corner pixel position
    float2 basePos = float2(charIndex * glyphWidthPixelsSpacer, screen_y_offset);

    // apply current corner offset & apply scale ratio
    float2 pixelPos = basePos + cornerOffset * float2(glyphWidthPixels, glyphHeightPixels);

    // convert from pixel offsets to screen space
    float2 clipPos = pixelPos * float2(screenScaleX, screenScaleY) - float2(one, one);

    o.Position = float4(clipPos, zero, one);
    
    float tileX = fmod(glyphIndex, tilesPerRow);
    float tileY = floor(glyphIndex / tilesPerRow);

    float2 tileOrigin = float2(tileX * tileWidthUV, tileY * tileHeightUV);

    // flip texture upside down
    float inverted_y = one - cornerY;
    float2 inverted_cornerOffset = float2(cornerX, inverted_y);
    
    o.TexCoord = tileOrigin + inverted_cornerOffset * float2(tileWidthUV, tileHeightUV);
    o.Color = text_color.xyz;
    return o;
}
