// Vertex Shader
struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// Constant buffer
cbuffer ConstantBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(proj, view), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, float4(input.pos, 1.0f));
    output.uv = input.uv;
    return output;
}

// Pixel Shader
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

SamplerState InputSampler : register(s0);
Texture2D InputTexture : register(t0);

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return InputTexture.Sample(InputSampler, input.uv);
}