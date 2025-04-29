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
    float4 lightViewPos : TEXCOORD1;
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

struct LightData
{
    uint lightType;
    float4 lightPos;
    float4 lightColor;
    float4x4 lightViewMat;
    float4x4 lightProjMat;
};
cbuffer FrameConstants : register(b1)
{
    float4 eyeWorldPosition;
    LightData lights[5];
}

VS_OUTPUT VSMain(VS_INPUT input)
{
    float4x4 worldViewProj = mul(mul(proj, view), world);
    
    VS_OUTPUT output;
    output.pos = mul(worldViewProj, input.pos);
    output.worldPos = mul(world, input.pos);
    output.lightViewPos = mul(mul(mul(lights[0].lightProjMat, lights[0].lightViewMat), world), input.pos);
    output.norm = mul(world, float4(input.norm.xyz, 0.0f));
    output.tang = mul(world, float4(input.tang.xyz, 0.0f));
    output.uv = input.uv;
    return output;
}

// Pixel Shader
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float4 lightViewPos : TEXCOORD1;
    float4 norm : NORMAL;
    float4 tang : TANGENT;
    float2 uv : TEXCOORD;
};

SamplerState InputSampler : register(s0);
SamplerState InputShadowSampler : register(s1);
Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D ShadowMapTexture : register(t2);

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
    
    float3 reflectAlbedo = float3(0.25f, 0.25f, 0.25f); // Default value is ambient light
    for (int i = 0; i < 5; i++)
    {
        float2 shadowMapTexCoord;
        shadowMapTexCoord.x = input.lightViewPos.x / input.lightViewPos.w / 2.0f + 0.5f;
        shadowMapTexCoord.y = -input.lightViewPos.y / input.lightViewPos.w / 2.0f + 0.5f;
        
        if (shadowMapTexCoord.x < 0.0f || shadowMapTexCoord.x > 1.0f || shadowMapTexCoord.y < 0.0f || shadowMapTexCoord.y > 1.0f)
            continue;
        
        float sampleDepthValue = ShadowMapTexture.Sample(InputShadowSampler, shadowMapTexCoord).r;
        float depthDist = length(input.worldPos.xyz - lights[i].lightPos.xyz);
        float trueDepthValue = min(1.0f, depthDist / 100.0f);
        
        if (trueDepthValue > sampleDepthValue + 0.002f)
            continue;
        
        // Diffuse
        float3 toLight = normalize(lights[i].lightPos.xyz - input.worldPos.xyz);
        float3 diffuse = lights[i].lightColor.xyz * max(0, dot(normal, toLight));
        
        // Specular
        float3 toEye = normalize(eyeWorldPosition.xyz - input.worldPos.xyz);
        float3 reflectionVec = normalize((2.0f * max(0, dot(normal, toLight)) * normal) - toLight);
        float3 specular = lights[i].lightColor.xyz * pow(max(0, dot(reflectionVec, toEye)), 2.0f);
        
        reflectAlbedo = reflectAlbedo + diffuse + specular;
    }
    
    float3 textureSample = float3(1.0f, 1.0f, 1.0f);
    if (diffTexFlag != 0)
    {
        textureSample = DiffuseTexture.Sample(InputSampler, input.uv).xyz;
    }
    
    return float4(textureSample * reflectAlbedo, 1.0f);
}