cbuffer MatrixBuffer
{
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

    // Remove the translation component from the view matrix
    matrix rotationOnlyView = viewMatrix;
    rotationOnlyView[3][0] = 0.0f;
    rotationOnlyView[3][1] = 0.0f;
    rotationOnlyView[3][2] = 0.0f;

    // Scale the skybox position to the edge of the view frustum
    float3 scaledPosition = position * 300.0f;
    float4 worldPosition = float4(scaledPosition, 1.0f);

    // Apply rotation-only view matrix and projection matrix
    output.position = mul(worldPosition, rotationOnlyView);
    output.position = mul(output.position, projectionMatrix);

    // Pass through texture coordinates
    output.texCoords = texCoord;

    return output;
}
