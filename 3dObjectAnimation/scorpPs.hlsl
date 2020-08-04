SamplerState s;
Texture2D tex;

cbuffer ConstantBuffer : register(b0)
{
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float3 eyePos;
  float pad;
};

struct VS_OUT
{
  float4 posH : sv_position;
  float3 posW : position;
  float3 normal : normal;
  float2 tex : texcoord;
  uint4 boneIndices : boneindices;
};

float4 PS(VS_OUT input) : SV_Target
{
  return tex.Sample(s, input.tex);
}