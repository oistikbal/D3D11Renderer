struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT main(uint id : SV_VertexID)
{
    VS_OUTPUT output;

    // Calculate screen-space positions for a full-screen quad
    float2 positions[4] =
    {
        float2(-1.0f, 1.0f), // Top-left
        float2(1.0f, 1.0f), // Top-right
        float2(-1.0f, -1.0f), // Bottom-left
        float2(1.0f, -1.0f) // Bottom-right
    };

    // Set position and texture coordinates based on vertex ID
    output.Position = float4(positions[id], 0.0f, 1.0f);
    output.TexCoord = float2((positions[id].x + 1.0f) * 0.5f, (1.0f - positions[id].y) * 0.5f);

    return output;
}
