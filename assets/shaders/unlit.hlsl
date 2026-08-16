cbuffer FrameCB : register(b0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
};

struct VSIn {
  float3 pos : POSITION;
  float3 nrm : NORMAL;
  float2 uv  : TEXCOORD0;
};

struct VSOut {
  float4 pos : SV_POSITION;
  float3 nrm : COLOR0;
  float2 uv  : TEXCOORD0;
};

VSOut vs_main(VSIn i) {
  VSOut o;
  o.pos = mul(float4(i.pos, 1.0), worldViewProj);
  o.nrm = i.nrm;
  o.uv = i.uv;
  return o;
}

float4 ps_main(VSOut i) : SV_TARGET {
  float3 c = 0.35 + 0.65 * saturate(i.nrm * 0.5 + 0.5);
  return float4(c, 1.0);
}
