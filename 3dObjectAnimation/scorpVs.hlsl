cbuffer ConstantBuffer : register(b0)
{
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float4x4 rotation;
  float3 eyePos;
  float pad;
  float4x4 boneTransforms[96];
};

struct VS_IN
{
  float3 pos : position;
  float3 normal : normal;
  float2 tex : texcoord;
  float3 boneWeights : weights;
  uint4 boneIndices : boneindices;
};

struct VS_OUT
{
  float4 posH : sv_position;
  float3 posW : position;
  float3 normal : normal;
  float2 tex : texcoord;
  uint4 boneIndices : boneindices;
};

VS_OUT VS(VS_IN input)
{
  VS_OUT vout;

  float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  weights[0] = input.boneWeights.x;
  weights[1] = input.boneWeights.y;
  weights[2] = input.boneWeights.z;
  weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

  float3 posL = float3(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < 4; ++i) {
    posL += weights[i] * mul(float4(input.pos, 1.0f), boneTransforms[input.boneIndices[i]]).xyz;
  }

  input.pos = posL;

  vout.posH = mul(float4(input.pos, 1.0f), worldViewProj);
  vout.posW = vout.posH.xyz;
  vout.normal = input.normal;
  vout.tex = input.tex;
  vout.boneIndices = input.boneIndices;

  return vout;
}