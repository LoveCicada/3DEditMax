cbuffer SpriteCB : register(b0) {
  float4x4 viewProj;
  float4 colorMul;
};

Texture2D spriteTex : register(t0);
SamplerState spriteSamp : register(s0);

struct VSIn {
  float3 pos : POSITION;
  float2 uv  : TEXCOORD0;
  float4 col : COLOR;
};

struct VSOut {
  float4 pos : SV_POSITION;
  float2 uv  : TEXCOORD0;
  float4 col : COLOR;
};

VSOut vs_main(VSIn i) {
  VSOut o;
  o.pos = mul(float4(i.pos, 1.0), viewProj);
  o.uv = i.uv;
  o.col = i.col * colorMul;
  return o;
}

float4 ps_main(VSOut i) : SV_TARGET {
  float4 tex = spriteTex.Sample(spriteSamp, i.uv);
  float4 outc = tex * i.col;
  clip(outc.a - 0.02);
  return outc;
}
