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
    GpuConstantsTable::Desc("camerapos", GpuConstantsType::kVector4,
                            offsetof(vs_cbuffer, camerapos), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);

  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("dirlight", offsetof(ps_cbuffer, light),
                            sizeof(lord::DirLight), 1),
    GpuConstantsTable::Desc("spotlight", offsetof(ps_cbuffer, spotlight),
                            sizeof(lord::SpotLight), 1),
  };
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

void SdkMeshEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void SdkMeshEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void SdkMeshEffect::SetSpotLight(const SpotLight& value) {
  spot_light_ = value;
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

void SdkMeshEffect::UseTexture(azer::Renderer* renderer) {
  renderer->UseTexture(kPixelStage, 0, diffusemap_.get());
  renderer->UseTexture(kPixelStage, 1, normalmap_.get());
  renderer->UseTexture(kPixelStage, 2, specularmap_.get());
}

// class SdkMeshMaterialEffectAdapter
SdkMeshMaterialEffectAdapter::SdkMeshMaterialEffectAdapter() {}
EffectAdapterKey SdkMeshMaterialEffectAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(SdkMeshMaterial).name());
}

void SdkMeshMaterialEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const SdkMeshEffect* provider = (const SdkMeshEffect*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
}


// class CameraProviderSdkMeshEffectProvider
CameraProviderSdkMeshEffectProvider::CameraProviderSdkMeshEffectProvider() {}
EffectAdapterKey CameraProviderSdkMeshEffectProvider::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(SdkMeshMaterial).name());
}

void CameraProviderSdkMeshEffectProvider::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(CameraProvider));
  const SdkMeshMaterial* provider = (const SdkMeshMaterial*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  
}
