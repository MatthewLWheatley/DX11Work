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
    output.Pos = input.Pos;//mul(input.Pos, World);
    output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    float2 xy = input.Tex.xy;
    xy.x = input.Tex.x/2 - input.Pos.x;
    xy.y = -input.Tex.y/2 - input.Pos.y;
    //return float4(xy, 0.0, 1.0);
    return ShaderTexture.Sample(SamplerLinear,xy);
}