// Constant buffer
cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

// Vertex Shader
struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(proj, view), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, float4(input.pos, 1.0f));
    output.color = input.color;
    return output;
}

// Pixel Shader
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.color;
}