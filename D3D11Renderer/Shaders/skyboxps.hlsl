// Pixel Shader (skyboxps.hlsl)
Texture2D<float4> panoramaTexture : register(t0); // Single panoramic texture
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoords : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // Sample the panorama texture using the input texture coordinates
    float4 color = pow(panoramaTexture.Sample(SampleType, input.texCoords), 2.2);

    return color;
}
