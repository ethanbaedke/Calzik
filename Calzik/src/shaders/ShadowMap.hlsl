struct VS_INPUT
{
    float4 pos : POSITION;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
};

// Constant buffer
cbuffer RenderItemConstants : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
    
    int diffTexFlag;
    int normTexFlag;
}

cbuffer ShadowMapConstants : register(b2)
{
    float4 lightPos;
    float4x4 lightView;
    float4x4 lightProj;
}

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(lightProj, lightView), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, input.pos);
    output.worldPos = mul(world, input.pos);

    return output;
}

// Pixel Shader
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float dist = length(input.worldPos.xyz - lightPos.xyz);
    float factor = min(1.0f, dist / 100.0f);
    
    return float4(factor, factor, factor, 1.0f);
}