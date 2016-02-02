#pragma pack_matrix(row_major)
cbuffer c_buffer {
  float4x4 pvw;
  float4x4 world;
};
struct HSCOutput {
  float edge[4]: SV_TessFactor;
  float inside[2]:  SV_InsideTessFactor;
};
struct HsOutput {
  float4 position: POSITION;
  float2 texcoord: TEXCOORD;
};
struct DsOutput {
  float4 position: SV_POSITION;
  float2 texcoord: TEXCOORD;
};

Texture2D heightmap;
SamplerState sam_heightmap {
  Filter = MIN_MAG_LINEAR_MIP_POINT;
  AddressU = CLAMP;
  AddressV = CLAMP;
};

[domain("quad")]
 DsOutput ds_main(HSCOutput input, 
                  const OutputPatch<HsOutput, 4> quad, 
                  float2 uv : SV_DomainLocation) {
    DsOutput output;
    float3 v1 = lerp(quad[0].position.xyz, quad[1].position.xyz, uv.x);
    float3 v2 = lerp(quad[3].position.xyz, quad[2].position.xyz, uv.x);
    float2 t1 = lerp(quad[0].texcoord, quad[1].texcoord, uv.x);
    float2 t2 = lerp(quad[3].texcoord, quad[2].texcoord, uv.x);
    output.texcoord = lerp(t1, t2, uv.y);

    float3 pos = lerp(v1, v2, uv.y);
    pos.y = heightmap.SampleLevel(sam_heightmap, output.texcoord, 0).r;
    output.position = mul(pvw, float4(pos, 1.0f));
    return output;
  };
