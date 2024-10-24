cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
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

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

	// Change the position vector to 4 components for matrix calculations
    input.position.w = 1.0f;

	// Transform position from object space to clip space
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Pass texture coordinates to pixel shader
    output.tex = input.tex;
    
	// Transform normal, tangent, and bitangent vectors to world space
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.tangent = mul(input.tangent, (float3x3) worldMatrix);
    output.bitangent = mul(input.bitangent, (float3x3) worldMatrix);

    // Normalize the vectors
    output.normal = normalize(output.normal);
    output.tangent = normalize(output.tangent);
    output.bitangent = normalize(output.bitangent);

	// Calculate the world position of the vertex
    worldPosition = mul(input.position, worldMatrix);

	// Calculate the view direction (camera to the vertex) and normalize it
    output.viewDirection = normalize(cameraPosition.xyz - worldPosition.xyz);

    return output;
}
