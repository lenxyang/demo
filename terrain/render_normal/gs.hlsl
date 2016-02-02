#pragma pack_matrix(row_major)

cbuffer c_buffer {
  float4x4 pvw;
}


struct VsOutput {
  float4 position:SV_POSITION;
  float3 normal: NORMAL;
  float2 texcoord: TEXCOORD;
};


struct GsOutput {
  float4 position:SV_POSITION;
};

[maxvertexcount(6)]
void gs_main(triangle VsOutput v[3], inout LineStream<GsOutput> linestream) {
  linestream.RestartStrip();

  GsOutput o;
  [unroll]
  for (int i = 0; i < 3; ++i) {
    o.position = mul(pvw, v[i].position);
    linestream.Append(o);

    float4 pos = v[i].position + float4(v[i].normal, 0.0);
    o.position = mul(pvw, pos);
    linestream.Append(o);
    break;
  }
}
