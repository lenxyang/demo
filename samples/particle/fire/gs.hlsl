#pragma pack_matrix(row_major)

#include "demo/samples/particle/particle.hlsl"

cbuffer cbPerFrame {
  float4x4 pv;
  float3 eyepos;
};

struct VsOutput {
  float3 pos :  POSITION;
  float2 size:  SIZE;
  float4 color: COLOR;
  uint   type:  TYPE;
};

struct GsOutput {
  float4 pos: SV_POSITION;
  float4 color:    COLOR;
  float2 tex: TEXCOORD;
};

[maxvertexcount(6)]
void gs_main(point VsOutput vin[1], inout TriangleStream<GsOutput> stream) {
  if (vin[0].type != PT_EMITTER) {
    float3 look = normalize(eyepos.xyz - vin[0].pos);
    float3 right = normalize(cross(float3(0, 1, 0), look));
    float3 up = cross(look, right);
    
    float half_width = 0.5f * vin[0].size.x;
    float half_height = 0.5f * vin[0].size.y;
    GsOutput o[4];
    o[0].pos = float4(vin[0].pos - half_width * right + half_height * up, 1.0);
    o[1].pos = float4(vin[0].pos - half_width * right - half_height * up, 1.0);
    o[2].pos = float4(vin[0].pos + half_width * right - half_height * up, 1.0);
    o[3].pos = float4(vin[0].pos + half_width * right + half_height * up, 1.0);
    o[0].pos = mul(pv, o[0].pos);
    o[1].pos = mul(pv, o[1].pos);
    o[2].pos = mul(pv, o[2].pos);
    o[3].pos = mul(pv, o[3].pos);
    o[0].tex = float2(0.0f, 0.0f);
    o[1].tex = float2(1.0f, 0.0f);
    o[2].tex = float2(1.0f, 1.0f);
    o[3].tex = float2(0.0f, 1.0f);
    o[0].color = vin[0].color;
    o[1].color = vin[0].color;
    o[2].color = vin[0].color;
    o[3].color = vin[0].color;
    
    stream.Append(o[0]);
    stream.Append(o[1]);
    stream.Append(o[3]);
    stream.Append(o[3]);
    stream.Append(o[1]);
    stream.Append(o[2]);
  }
}
