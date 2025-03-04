// Vertex Shader
struct VS_INPUT
{
    float4 pos : POSITION;
    float4 norm : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 norm : NORMAL;
    float2 uv : TEXCOORD;
};

// Constant buffer
cbuffer RenderItemConstants : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
}

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(proj, view), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, input.pos);
    output.worldPos = mul(world, input.pos);
    output.norm = mul(world, float4(input.norm.xyz, 0.0f));
    output.uv = input.uv;
    return output;
}

struct LightData
{
    float4 lightPos;
    float4 lightColor;
};
cbuffer FrameConstants : register(b0)
{
    float4 eyeWorldPosition;
    LightData lights[5];
}

// Pixel Shader
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 norm : NORMAL;
    float2 uv : TEXCOORD;
};

SamplerState InputSampler : register(s0);
Texture2D InputTexture : register(t0);

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.norm.xyz);
    
    float3 reflectAlbedo = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 5; i++)
    {
        // Diffuse
        float3 toLight = normalize(lights[i].lightPos.xyz - input.worldPos.xyz);
        float3 diffuse = lights[i].lightColor.xyz * max(0, dot(normal, toLight));
        
        // Specular
        float3 toEye = normalize(eyeWorldPosition.xyz - input.worldPos.xyz);
        float3 reflectionVec = normalize((2.0f * max(0, dot(normal, toLight)) * normal) - toLight);
        float3 specular = lights[i].lightColor.xyz * pow(max(0, dot(reflectionVec, toEye)), 64.0f);
        
        reflectAlbedo = reflectAlbedo + diffuse + specular;
    }
    
    float3 textureSample = InputTexture.Sample(InputSampler, input.uv);
    
    return float4(textureSample * reflectAlbedo, 1.0f);
}