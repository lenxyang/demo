#include "demo/base/sdkmesh_effect.h"

using namespace azer;

// class SdkMeshEffect
const char SdkMeshEffect::kEffectName[] = "SdkMeshEffect";
SdkMeshEffect::SdkMeshEffect() {
  world_ = Matrix4::kIdentity;
}
bool SdkMeshEffect::Init(VertexDesc* desc, const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitShaders(sources);
  InitGpuConstantTable();
  return true;
}
void SdkMeshEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, world), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
}

void SdkMeshEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
  }
}

scoped_refptr<SdkMeshEffect> CreateSdkMeshEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec3},
    {"NORMAL", 0, kVec3},
    {"TEXCOORD", 0, kVec2},
    {"TANGENT", 0, kVec3},
  };
  Effect::ShaderPrograms shaders;
  shaders.resize(kRenderPipelineStageNum);
  shaders[kVertexStage].path = "effect.vs";
  shaders[kVertexStage].stage = kVertexStage;
  shaders[kVertexStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "struct VsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "}\n;"
      "struct VSInput {\n"
      "  float3 position:POSITION;\n"
      "  float3 normal:NORMAL;\n"
      "  float2 texcoord:TEXCOORD;\n"
      "  float3 tangent:TANGENT;\n"
      "};\n"
      "cbuffer c_buffer {\n"
      "  float4x4 pvw;"
      "  float4x4 world;"
      "};"
      "VsOutput vs_main(VSInput input) {\n"
      "VsOutput o;"
      "o.position = mul(pvw, float4(input.position, 1.0));"
      "return o;"
      "}";
  shaders[kPixelStage].path = "effect.ps";
  shaders[kPixelStage].stage = kPixelStage;
  shaders[kPixelStage].code = "#pragma pack_matrix(row_major)\n"
      "struct VsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "};\n"
      "float4 ps_main(VsOutput o):SV_TARGET {\n"
      "  return color;"
      "}\n";
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  scoped_refptr<SdkMeshEffect> ptr(new SdkMeshEffect);
  ptr->Init(desc, shaders);
  return ptr;
}
