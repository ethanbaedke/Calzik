// Vertex Shader
struct VS_INPUT
{
    float4 pos : POSITION;
    float4 norm : NORMAL;
    float4 tang : TANGENT;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 norm : NORMAL;
    float4 tang : TANGENT;
    float2 uv : TEXCOORD;
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

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(proj, view), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, input.pos);
    output.worldPos = mul(world, input.pos);
    output.norm = mul(world, float4(input.norm.xyz, 0.0f));
    output.tang = mul(world, float4(input.tang.xyz, 0.0f));
    output.uv = input.uv;
    return output;
}

struct LightData
{
    float4 lightPos;
    float4 lightColor;
};
cbuffer FrameConstants : register(b1)
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
    float4 tang : TANGENT;
    float2 uv : TEXCOORD;
};

SamplerState InputSampler : register(s0);
Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.norm.xyz);
    
    // Normal mapping
    if (normTexFlag != 0)
    {
        float3 tangent = normalize(input.tang.xyz);
        float3 bitangent = cross(tangent, normal);
        float3x3 tangentSpaceMat = float3x3(tangent.x, bitangent.x, normal.x, tangent.y, bitangent.y, normal.y, tangent.z, bitangent.z, normal.z);
        normal = (NormalTexture.Sample(InputSampler, input.uv).xyz * 2.0f) - 1.0f;
        normal = mul(tangentSpaceMat, normal);
    }
    
    float3 reflectAlbedo = float3(0.2f, 0.2f, 0.2f); // Default value is ambient light
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
    
    float3 textureSample = float3(1.0f, 1.0f, 1.0f);
    if (diffTexFlag != 0)
    {
        textureSample = DiffuseTexture.Sample(InputSampler, input.uv).xyz;
    }
    
    return float4(textureSample * reflectAlbedo, 1.0f);
}