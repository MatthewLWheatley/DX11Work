cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vOutputColor;
}

Texture2D ShaderTexture : register(t0);
SamplerState SamplerLinear : register(s0);

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = input.Pos; // Pass through position
    output.Tex = input.Tex; // Pass through texture coordinate
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    return ShaderTexture.Sample(SamplerLinear, input.Tex);
}