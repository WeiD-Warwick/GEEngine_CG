cbuffer animatedMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
    float4x4 bones[256];
};

struct VS_INPUT {
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint4 BoneIDs : BONEIDS;
    float4 BoneWeights : BONEWEIGHTS;
};


struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;
    
    // transform for animation
    float4x4 transform = bones[input.BoneIDs[0]] * input.BoneWeights[0];
    transform += bones[input.BoneIDs[1]] * input.BoneWeights[1];
    transform += bones[input.BoneIDs[2]] * input.BoneWeights[2];
    transform += bones[input.BoneIDs[3]] * input.BoneWeights[3];
    
    // model -> view -> projection
    float4 pos = float4(input.Pos, 1.0f);
    pos = mul(pos, transform);
    pos = mul(pos, W);
    pos = mul(pos, VP);
    output.Pos = pos;
    
    float3 n = mul(input.Normal, (float3x3) transform);
    n = mul(n, (float3x3) W);
    output.Normal = normalize(n);
    
    float3 t = mul(input.Tangent, (float3x3) transform);
    t = mul(t, (float3x3) W);
    output.Tangent = normalize(t);
    
    output.TexCoords = input.TexCoords;
    return output;
}