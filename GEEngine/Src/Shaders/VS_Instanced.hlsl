cbuffer staticMeshBuffer : register(b0)
{
    float4x4 VP;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
   
    float4x4 World : INSTANCE;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPos = mul(float4(input.Pos, 1.0f), input.World);
    output.Pos = mul(worldPos, VP);

    output.Normal = mul(input.Normal, (float3x3) input.World);
    output.Tangent = mul(input.Tangent, (float3x3) input.World);
    output.TexCoords = input.TexCoords;
    return output;
}