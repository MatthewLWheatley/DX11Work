cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

PixelInputType MainVS(VertexInputType input)
{
    PixelInputType output;
    input.position.w = 1.0f;
    //output.position = mul(input.position, worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);
    output.position = input.position; // Directly use the input position
    output.tex = input.tex;
    
    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;


float4 MainPS(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    return textureColor;
}
