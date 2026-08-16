cbuffer LineCB : register(b0) {
  float4x4 viewProj;
  float4 colorMul;
};

struct VSIn {
  float3 pos : POSITION;
  float4 col : COLOR;
};

struct VSOut {
  float4 pos : SV_POSITION;
  float4 col : COLOR;
};

VSOut vs_main(VSIn i) {
  VSOut o;
  o.pos = mul(float4(i.pos, 1.0), viewProj);
  o.col = i.col * colorMul;
  return o;
}

float4 ps_main(VSOut i) : SV_TARGET {
  return i.col;
}
