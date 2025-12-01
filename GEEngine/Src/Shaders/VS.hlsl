cbuffer staticMeshBuffer
{
    float4x4 W;
    float4x4 VP;
};


struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};

float3x3 inverse(float3x3 m) {
    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    float b01 = a22 * a11 - a12 * a21;
    float b11 = -a22 * a10 + a12 * a20;
    float b21 = a21 * a10 - a11 * a20;

    float det = a00 * b01 + a01 * b11 + a02 * b21;
    
    if (abs(det) < 1e-6)
        det = 1e-6;

    float invDet = 1.0 / det;

    float3x3 r;

    r[0][0] = b01 * invDet;
    r[0][1] = (-a22 * a01 + a02 * a21) * invDet;
    r[0][2] = (a12 * a01 - a02 * a11) * invDet;

    r[1][0] = b11 * invDet;
    r[1][1] = (a22 * a00 - a02 * a20) * invDet;
    r[1][2] = (-a12 * a00 + a02 * a10) * invDet;

    r[2][0] = b21 * invDet;
    r[2][1] = (-a21 * a00 + a01 * a20) * invDet;
    r[2][2] = (a11 * a00 - a01 * a10) * invDet;

    return r;
}

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = mul(input.Pos, W);
    output.Pos = mul(output.Pos, VP);
    
    // output.Normal = mul(input.Normal, (float3x3) W);
    output.Normal = mul(input.Normal, transpose(inverse((float3x3) W)));

    output.Tangent = mul(input.Tangent, (float3x3) W);
    output.TexCoords = input.TexCoords;
    return output;
}

