
#pragma pack_matrix(row_major)
struct VsOutput {
float4 position:SV_POSITION;
};
struct VSInput {
float4 position:POSITION;
};
VsOutput vs_main(VSInput input) {
VsOutput o;
o.position = input.position;
return o;
};