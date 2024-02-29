
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vOutputColor;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);


struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 WorldNorm : NORMAL;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader 2
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);

    output.Tex = input.Tex;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_TARGET
{
    return txDiffuse.Sample(samLinear, input.Tex);
}

