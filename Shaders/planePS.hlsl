SamplerState s;
Texture2D t;

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

struct VS_OUT
{
  float4 posH : sv_position;
  float3 posW : position;
  float3 normal : normal;
  float2 tex : texcoord;
};

float4 PS(VS_OUT input) : SV_Target
{
  input.normal = normalize(input.normal);

  float4 objectDiffuse = t.Sample(s, input.tex);

  float4 finalColor = objectDiffuse * ambient;
  finalColor += saturate(dot(direction, input.normal) * diffuse * objectDiffuse);

  return float4(finalColor.xyz, objectDiffuse.a);
}