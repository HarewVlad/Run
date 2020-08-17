SamplerState s;
TextureCube tc;

struct VS_OUT
{
  float4 pos : sv_position;
  float3 tex : texcoord;
};

float4 PS(VS_OUT input) : sv_target
{
  return tc.Sample(s, input.tex);
}