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

float4 main(VS_OUTPUT input) : SV_Target
{
    int2 texCoords = int2(input.Position.xy); // Convert to int2 for Load function
    float4 hdrColor = HDRTexture.Load(texCoords, 4); // Sample with sample index 0

    // Calculate the luminance of the HDR color
    float luminance = CalculateLuminance(hdrColor.rgb);

    // Apply Reinhard tone mapping with exposure adjustment
    float3 toneMappedColor = hdrColor.rgb * Exposure; // Adjust for exposure
    toneMappedColor /= (toneMappedColor + float3(1.0, 1.0, 1.0)); // Reinhard normalization

    // Use the scene's average luminance for scaling
    toneMappedColor /= (AverageLuminance + 1e-5); // Avoid division by zero

    // Apply maximum luminance threshold
    toneMappedColor = min(toneMappedColor, MaxLuminance);

    // Use burn to control clamping and prevent overexposure
    toneMappedColor = saturate(toneMappedColor / Burn);

    // Return the final color with tone mapping applied
    return float4(toneMappedColor, hdrColor.a);
}
