//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

Texture2D txDiffuse : register(t0);
Texture2D txNormalMap : register(t1);
SamplerState samLinear : register(s0);
SamplerState samNormalMap : register(s1);

#define MAX_LIGHTS 1
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _Material
{
	float4  Emissive;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Ambient;        // 16 bytes
							//------------------------------------(16 byte boundary)
	float4  Diffuse;        // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Specular;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float   SpecularPower;  // 4 bytes
	bool    UseTexture;     // 4 bytes
	float2  Padding;        // 8 bytes
							//----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b1)
{
	_Material Material;
};

struct Light
{
	float4      Position;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Direction;              // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Color;                  // 16 bytes
										//----------------------------------- (16 byte boundary)
	float       SpotAngle;              // 4 bytes
	float       ConstantAttenuation;    // 4 bytes
	float       LinearAttenuation;      // 4 bytes
	float       QuadraticAttenuation;   // 4 bytes
										//----------------------------------- (16 byte boundary)
	int         LightType;              // 4 bytes
	bool        Enabled;                // 4 bytes
	int2        Padding;                // 8 bytes
										//----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b2)
{
	float4 EyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4 GlobalAmbient;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
}; 

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

float4 DoDiffuse(float3 lightDir, float3 norm, float4 lightColor, float4 materialDiffuse)
{
    float NdotL = max(dot(norm, lightDir), 0.0);
    return NdotL * lightColor * materialDiffuse;
}

float4 DoSpecular(float3 lightDir, float3 norm, float3 viewDir, float4 lightColor, float4 materialSpecular, float specularPower)
{
    float3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularPower);
    return spec * lightColor * materialSpecular;
}

float DoAttenuation(Light light, float d)
{
	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

LightingResult DoPointLight(Light light, float3 vertexToEye, float4 vertexPos, float3 N, float3 Tangent, float3 Bitangent)
{
    LightingResult result;
    result.Diffuse = float4(0, 0, 0, 0);
    result.Specular = float4(0, 0, 0, 0);

    // Calculate light direction
    float3 lightDir = normalize(light.Position.xyz - vertexPos.xyz);

    // Calculate attenuation
    float attenuation = DoAttenuation(light, length(light.Position.xyz - vertexPos.xyz));

    // Calculate diffuse contribution
    float diffuseFactor = max(dot(N, lightDir), 0.0f);
    result.Diffuse = light.Color * Material.Diffuse * diffuseFactor * attenuation;

    // Calculate specular contribution
    float3 viewDir = normalize(vertexToEye);
    float3 reflectDir = reflect(-lightDir, N);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), Material.SpecularPower);
    result.Specular = light.Color * Material.Specular * specularFactor * attenuation;

    return result;
}

LightingResult ComputeLighting(float3 worldPos, float3 N)
{
    LightingResult result = { float4(0, 0, 0, 1), float4(0, 0, 0, 1) };
    float3 viewDir = normalize(EyePosition - worldPos);

    // Accumulate contributions from all lights
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        if (!Lights[i].Enabled)
            continue;

        float3 lightDir;
        float attenuation = 1.0; // Start with no attenuation

        if (Lights[i].LightType == POINT_LIGHT)
        {
            lightDir = normalize(Lights[i].Position.xyz - worldPos);
            float distance = length(Lights[i].Position.xyz - worldPos);
            attenuation = 1.0 / (Lights[i].ConstantAttenuation + Lights[i].LinearAttenuation * distance + Lights[i].QuadraticAttenuation * distance * distance);
        }
        else if (Lights[i].LightType == DIRECTIONAL_LIGHT)
        {
            lightDir = normalize(-Lights[i].Direction.xyz);
        }
        // Add spot light calculation here if needed

        // Calculate diffuse and specular contributions
        float4 diffuse = DoDiffuse(lightDir, N, Lights[i].Color, Material.Diffuse)*2;
        float4 specular = DoSpecular(lightDir, N, viewDir, Lights[i].Color, Material.Specular, Material.SpecularPower);

        // Apply attenuation for point lights
        result.Diffuse += diffuse * attenuation;
        result.Specular += specular * attenuation;
    }

    // Apply global ambient light
    result.Diffuse += Material.Ambient * GlobalAmbient;

    return result;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    // Transform positions to clip space
    output.Pos = mul(mul(mul(input.Pos, World), View), Projection);
    // Transform normals to world space
    output.Norm = mul(input.Norm, (float3x3) World);
    // Pass through texture coordinates
    output.Tex = input.Tex;
    // Calculate and pass tangent and bitangent vectors
    output.Tangent = mul(input.Tangent, (float3x3) World);
    output.Bitangent = mul(input.Bitangent, (float3x3) World);
    // Calculate world position for lighting
    output.WorldPos = mul(input.Pos, World).xyz;

    return output;
}

//--------------------------------------------------------------------------------------
// Vertex Shader 2
//--------------------------------------------------------------------------------------
PS_INPUT VS2(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.WorldPos = output.Pos;
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

	// multiply the normal by the world transform (to go from model space to world space)
    output.Norm = mul(float4(input.Norm, 0), World).xyz;

    output.Tex = input.Tex;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_TARGET
{
    // Sample the normal map and convert to [-1,1]
    float3 normalMap = txNormalMap.Sample(samNormalMap, input.Tex).xyz *2.0f -1.0f;
    // Construct TBN matrix
    input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Norm) * input.Norm);
    
    float3x3 TBN = float3x3(normalize(input.Tangent), normalize(input.Bitangent), normalize(input.Norm));
    // Transform normal to world space
    // In Pixel Shader
    float3 normal = normalize(mul(normalMap, TBN));
    //normal = normalize(input.Norm);
    //normal = float3(0, 0, 1);

    // Compute lighting
    LightingResult lit = ComputeLighting(input.WorldPos, normal);

    // Sample texture color
    float4 texColor = txDiffuse.Sample(samLinear, input.Tex);
    if (!Material.UseTexture)
    {
        texColor = float4(1, 1, 1, 1);
    }
    
    float4 emissive = Material.Emissive;
    float4 ambient = Material.Ambient * GlobalAmbient;
    float4 diffuse = Material.Diffuse * lit.Diffuse;
    float4 specular = Material.Specular * lit.Specular;
    
    float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;
    
    return finalColor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader 2
//--------------------------------------------------------------------------------------
float4 PS2(PS_INPUT IN) : SV_TARGET
{
    LightingResult lit = ComputeLighting(IN.WorldPos, normalize(IN.Norm));

    float4 texColor = { 1, 1, 1, 1 };

    float4 emissive = Material.Emissive;
    float4 ambient = Material.Ambient * GlobalAmbient;
    float4 diffuse = Material.Diffuse * lit.Diffuse;
    float4 specular = Material.Specular * lit.Specular;

    if (Material.UseTexture)
    {
        texColor = txDiffuse.Sample(samLinear, IN.Tex);
    }

    float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

    return finalColor;
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}

