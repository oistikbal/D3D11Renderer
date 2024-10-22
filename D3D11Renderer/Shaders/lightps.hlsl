Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
SamplerState SampleType : register(s0);

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 viewDirection : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection;
    float4 specular;
    float3 normalTangentSpace;
    float3 normalWorldSpace;

	// Sample the diffuse texture
    textureColor = diffuseMap.Sample(SampleType, input.tex);

	// Set default output color to ambient light
    color = ambientColor;

	// Sample the normal map and transform it from [0, 1] to [-1, 1]
    normalTangentSpace = normalMap.Sample(SampleType, input.tex).xyz * 2.0f - 1.0f;

	// Construct the TBN matrix
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));

	// Transform the normal from tangent space to world space
    normalWorldSpace = mul(normalTangentSpace, TBN);

	// Normalize the result
    normalWorldSpace = normalize(normalWorldSpace);

	// Invert the light direction for lighting calculations
    lightDir = -normalize(lightDirection);

	// Calculate the diffuse light intensity (N·L)
    lightIntensity = saturate(dot(normalWorldSpace, lightDir));

    if (lightIntensity > 0.0f)
    {
        // Add the diffuse component
        color += diffuseColor * lightIntensity;

        // Calculate reflection vector based on light intensity, normal, and light direction
        reflection = normalize(2.0f * lightIntensity * normalWorldSpace - lightDir);

		// Calculate the specular intensity using reflection vector, view direction, and specular power
        float specIntensity = pow(saturate(dot(reflection, normalize(input.viewDirection))), specularPower);

		// Sample the specular map and scale by specular intensity
        specular = specularMap.Sample(SampleType, input.tex) * specIntensity;

		// Add the specular color component to the final color
        color = saturate(color + specular * specularColor);
    }

	// Combine the final color with the texture color
    color = color * textureColor;

    return color;
}
