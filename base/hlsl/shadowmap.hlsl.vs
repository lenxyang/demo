#pragma pack_matrix(row_major)

struct VsOutput {
  float4 position:SV_POSITION;
  float3 worldpos: WPOS;
  float3 normal:NORMAL;
  float3 viewin: VIEWIN;
  float2 texcoord: TEXCOORD0;
  float4 projtex: TEXCOORD1;
};

struct VSInput {
  float4 position:POSITION;
  float4 normal:NORMAL;
  float2 texcoord: TEXCOORD0;
};

cbuffer c_buffer {
   float4x4 pvw;
   float4x4 world;
   float4x4 lightpv;
   float4   camerapos;
};

VsOutput vs_main(VSInput input) {
  VsOutput o;
  o.position = mul(pvw, input.position);
  o.worldpos = mul(world, input.position).xyz;
  o.projtex  = mul(lightpv, float4(o.worldpos, 1.0));
  o.normal = normalize(mul(world, input.normal)).xyz;
  o.viewin = normalize(camerapos - o.worldpos);
  o.texcoord = input.texcoord;
  return o;
}
