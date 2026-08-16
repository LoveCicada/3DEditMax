cbuffer FrameCB : register(b0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float4 shadingMode;
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

float4 ps_checker(VSOut i) : SV_TARGET {
  float2 f = frac(i.uv * 8.0);
  float c = (f.x < 0.5) != (f.y < 0.5) ? 0.0 : 1.0;
  return float4(c, c, c, 1.0);
}

float4 ps_main(VSOut i) : SV_TARGET {
  float mode = shadingMode.x;
  if (mode < 0.5) {
    return float4(0.55, 0.62, 0.75, 1.0);
  }
  if (mode < 1.5) {
    float3 c = 0.35 + 0.65 * saturate(i.nrm * 0.5 + 0.5);
    return float4(c, 1.0);
  }
  return ps_checker(i);
}
