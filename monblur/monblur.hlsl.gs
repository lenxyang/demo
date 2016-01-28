#pragma pack_matrix(row_major)

#define kMaxTimesStep  3

cbuffer c_buffer {
   float4x4 gpv[kMaxTimesStep];
   float4x4 gworld[kMaxTimesStep];
   float4   camerapos;
};

struct VSInput {
  float3 position:POSITION;
  float3 normal:NORMAL;
  float2 texcoord: TEXCOORD0;
  float3 tang: TANGENT;
};

struct VsOutput {
  float4 position:SV_POSITION;
  float3 worldpos: WPOS;
  float3 normal:NORMAL;
  float3 viewin: VIEWIN;
  float2 texcoord: TEXCOORD0;
};

void extrude_blur_edge(VSInput v1, VSInput v2, uniform uint step, 
                       inout TriangleStream<VsOutput> stream) {
  VsOutput o1, o2;
  float4x4 world = gworld[step];
  float4x4 pvw = gpv[step] * world;
  o1.position = mul(pvw, float4(v1.position, 1.0));
  o1.worldpos = mul(world, float4(v1.position, 1.0)).xyz;
  o1.normal   = mul(world, float4(v1.normal, 0.0f)).xyz;
  o1.viewin = normalize(camerapos.xyz - o1.worldpos);
  o1.texcoord = v1.texcoord;

  o2.position = mul(pvw, float4(v2.position, 1.0));
  o2.worldpos = mul(world, float4(v2.position, 1.0)).xyz;
  o2.normal   = mul(world, float4(v2.normal, 0.0)).xyz;
  o2.viewin = normalize(camerapos.xyz - o2.worldpos);
  o2.texcoord = v2.texcoord;
  stream.Append(o1);
  stream.Append(o2);
}

void extrude_blur_edges(VSInput v1, VSInput v2, 
                       inout TriangleStream<VsOutput> stream,
                       uniform uint step) {
  for (uint i = 0; i < step; ++i) {
    extrude_blur_edge(v1, v2, i, stream);
  }
  stream.RestartStrip();
}

[maxvertexcount(18)]
void gs_main(triangle VSInput v[3], inout TriangleStream<VsOutput> stream,
             uniform bool frontside) {
  extrude_blur_edges(v[0], v[1], stream, frontside);   
  extrude_blur_edges(v[1], v[2], stream, frontside);   
  extrude_blur_edges(v[2], v[0], stream, frontside);   
}