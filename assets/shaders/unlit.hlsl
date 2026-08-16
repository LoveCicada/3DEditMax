cbuffer FrameCB : register(b0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
  float4 shadingMode;
  float4 baseColor;
  float4 emissive;
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
  o.nrm = mul(i.nrm, (float3x3)world);
  o.uv = i.uv;
  return o;
}

float3 lambertLit(float3 albedo, float3 nrm, float3 emit) {
  float3 N = normalize(nrm);
  float3 L = normalize(float3(5.0, 10.0, 5.0));
  float diff = saturate(dot(N, L));
  return albedo * (0.4 + 0.8 * diff) + emit;
}

float4 ps_checker(VSOut i) : SV_TARGET {
  float2 f = frac(i.uv * 8.0);
  float c = (f.x < 0.5) != (f.y < 0.5) ? 0.0 : 1.0;
  float3 dark = float3(0.173, 0.373, 0.588);
  float3 lite = float3(0.949, 0.949, 0.949);
  float3 albedo = lerp(dark, lite, c);
  return float4(lambertLit(albedo, i.nrm, emissive.rgb), baseColor.a);
}

float4 ps_main(VSOut i) : SV_TARGET {
  float mode = shadingMode.x;
  if (mode < 0.5) {
    return float4(lambertLit(baseColor.rgb, i.nrm, emissive.rgb), baseColor.a);
  }
  if (mode < 1.5) {
    float3 c = 0.35 + 0.65 * saturate(normalize(i.nrm) * 0.5 + 0.5);
    return float4(c, baseColor.a);
  }
  return ps_checker(i);
}
