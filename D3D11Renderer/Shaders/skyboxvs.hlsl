// Vertex Shader (skyboxvs.hlsl)
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoords : TEXCOORD0;
};

VS_OUTPUT main(float3 position : POSITION, float2 texCoord : TEXCOORD0)
{
    VS_OUTPUT output;

    // Apply transformations (view and projection)
    float3 scaledPosition = position * 300.0f;
    float4 worldPosition = float4(scaledPosition, 1.0f);
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Pass through texture coordinates
    output.texCoords = texCoord;

    return output;
}
