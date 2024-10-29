Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D aoMap : register(t3);
Texture2D emissiveMap : register(t4);
Texture2D metalRoughnessMap : register(t5);
SamplerState SampleType : register(s0);

// Inside cbuffer (if required):
cbuffer LightBuffer
{
    float4 ambientColor; // Existing lighting data
    float4 diffuseColor; // Existing lighting data
    float3 lightDirection; // Existing lighting data
    float specularPower; // Existing lighting data
    float4 specularColor; // Existing lighting data
};

// Update the PixelInputType to include new texture coordinates (optional if using the same texture coordinates)
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 viewDirection : TEXCOORD1;
};

// Main pixel shader
float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float4 emissive;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float3 reflection;
    float4 specular;
    float3 normalTangentSpace;
    float3 normalWorldSpace;

    textureColor = pow(diffuseMap.Sample(SampleType, input.tex), 2.2); // Convert diffuse to linear space
    emissive = pow(emissiveMap.Sample(SampleType, input.tex), 2.2); // Convert emissive to linear space

    // Sample the AO map (ambient occlusion) and factor it into ambient lighting
    float ao = aoMap.Sample(SampleType, input.tex).r;

    // Sample the metal-roughness map and extract metalness and roughness
    float4 metalRoughness = metalRoughnessMap.Sample(SampleType, input.tex);
    float metalness = metalRoughness.r;
    float roughness = metalRoughness.g;

    // Set default output color to ambient light (factor in AO)
    color = ambientColor * ao;

    // Sample and transform the normal map
    normalTangentSpace = normalMap.Sample(SampleType, input.tex).xyz * 2.0f - 1.0f;

    // Construct the TBN matrix
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));

    // Transform the normal from tangent space to world space
    normalWorldSpace = mul(normalTangentSpace, TBN);
    normalWorldSpace = normalize(normalWorldSpace);

    // Invert the light direction for lighting calculations
    lightDir = -normalize(lightDirection);

    // Calculate diffuse light intensity
    lightIntensity = saturate(dot(normalWorldSpace, lightDir));

    if (lightIntensity > 0.0f)
    {
        // Add diffuse lighting
        color += diffuseColor * lightIntensity;

        // Calculate reflection and specular component
        reflection = normalize(2.0f * lightIntensity * normalWorldSpace - lightDir);
        float specIntensity = pow(saturate(dot(reflection, normalize(input.viewDirection))), specularPower);
        specular = specularColor * specIntensity;

        // Modulate the specular highlight by roughness
        color += specular * (1.0f - roughness);
    }

    // Combine metallic reflection and diffuse color
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), textureColor.rgb, metalness);
    color.rgb = lerp(color.rgb, F0, metalness);

    // Combine the final color with texture, ambient occlusion, and emissive
    color.rgb = saturate(color.rgb * textureColor.rgb + emissive.rgb);
    color.a = 1.0f;
    return color;
}

