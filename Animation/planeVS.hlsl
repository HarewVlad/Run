cbuffer ConstantBuffer : register(b0)
{
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float3 eyePos;
  float pad;
};

cbuffer ConstantBufferLightning : register(b1)
{
  float3 direction;
  float pad1;
  float4 ambient;
  float4 diffuse;
}

struct VS_IN
{
  float3 pos : position;
  float3 normal : normal;
  float2 tex : texcoord;
};

struct VS_OUT
{
  float4 posH : sv_position;
  float3 posW : position;
  float3 normal : normal;
  float2 tex : texcoord;
};

VS_OUT VS(VS_IN input)
{
  VS_OUT vout;

  vout.posH = mul(float4(input.pos, 1.0f), worldViewProj);
  vout.posW = vout.posH.xyz;
  vout.normal = input.normal;
  vout.tex = input.tex;

  return vout;
}

