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

// Helper function to calculate luminance
float CalculateLuminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

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

    // Calculate the luminance of the HDR color
    float luminance = CalculateLuminance(hdrColor.rgb);

    // Apply exposure adjustment
    float3 color = hdrColor.rgb * Exposure;

    // Tone mapping using ACES
    color = ACESFilm(color);

    // Apply gamma correction (assuming sRGB gamma of 2.2)
    color = pow(color, 1.0 / 2.2);

    // Cap luminance and control clamping
    color = min(color, MaxLuminance);
    color = saturate(color / Burn);

    // Return the final color with tone mapping and gamma correction applied
    return float4(color, hdrColor.a);
}
