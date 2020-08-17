cbuffer ConstantBuffer : register(b0)
{
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float3 eyePos;
  float pad;
};

struct VS_IN
{
  float3 pos : position;
  float3 normal : normal;
  float2 tex : texcoord;
};

struct VS_OUT
{
  float4 pos : sv_position;
  float3 tex : texcoord;
};

VS_OUT VS(VS_IN input)
{
  VS_OUT vout;

  vout.pos = mul(float4(input.pos, 1.0f), worldViewProj).xyww;
  vout.tex = input.pos;

  return vout;
}