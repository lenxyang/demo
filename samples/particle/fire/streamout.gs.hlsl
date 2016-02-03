#pragma pack_matrix(row_major)

#include "demo/samples/particle/particle.hlsl"

cbuffer cbPerFrame {
  float3 emitpos;
  float time_step;
  float global_time;
};

Texture1D random_tex : register(t0);
SamplerState linear_sampler {
  Filter = MIN_MAG_MIP_LINEAR;
  AddressU = WRAP;
  AddressV = WRAP;
};

float3 RandUnitVec3(float offset) {
  float u = (gGameTime + offset);
  float3 v = random_tex.SampleLevel(linear_sampler, u, 0).xyz;
  return normalize(v);
}

[maxvertexcount(2)]
void stream_outgs(point Particle vin[1], inout PointStream<Particle> stream) {
  vin[0].age += time_step;
  if (vin[0].type == PT_EMITTER) {
    if (vin[0].age > 0.0005f) {
      float3 random = RandUintVec3(0.0f);
      random.x *= 0.5f;
      random.z *= 0.5f;

      Particle p;
      p.initpos = emitpos;
      p.velocity = 4.0f * random;
      p.size = float2(3.0f, 3.0f);
      p.Age = 0.0f;
      p.Type = PT_FLARE;

      stream.Append(p);
      vin[0].age = 0.0f;
    }

    // always keep the emitter
    stream.Append(vin[0]);
  } else {
    if (vin[0].age <= 1.0) {
      stream.Append(vin[0]);
    }
  }
}
