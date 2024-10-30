Texture2DMS<float4> HDRTexture : register(t0);
SamplerState SampleType : register(s0);

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer ToneMappingParams : register(b0)
{
    float Exposure; // Brightness control
    float Burn; // Control for intensity
    float AverageLuminance; // Scene's average luminance
    float MaxLuminance; // Luminance cap to prevent overexposure
    float Padding; // Padding for alignment
};

// ACES approximation function
float3 ACESFilm(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 main(VS_OUTPUT input) : SV_Target
{
    int2 texCoords = int2(input.Position.xy); // Convert to int2 for Load function
    float4 hdrColor = HDRTexture.Load(texCoords, 4); // Sample with sample index 0

    float3 color = hdrColor.rgb * Exposure;

    // Apply ACES tone mapping
    color = ACESFilm(color);

    // Convert from linear space to sRGB space with gamma correction (gamma 2.2)
    color = pow(color, 1.0 / 2.2);

    // Return the final color with tone mapping and gamma correction applied
    return float4(color, hdrColor.a);
}
