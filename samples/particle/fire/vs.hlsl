#pragma pack_matrix(row_major)

#include "demo/samples/particle/particle.hlsl"

struct VsOutput {
  float3 pos :  POSITION;
  float2 size:  SIZE;
  float4 color: COLOR;
  uint   type:  TYPE;
};


VsOutput vs_main(Particle input) {
  VsOutput o;
  o.pos = input.initpos + input.velocity * input.age;
  float opacity = 1.0f - smoothstep(0.0f, 1.0f, input.age / 1.0f);
  o.color  = float4(1.0f, 1.0f, 1.0f, opacity);
  o.type = input.type;
  o.size = input.size;
  return o;
}
