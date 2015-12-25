#include "demo/base/shadow_depth_effect.h"

#include "azer/render/util/shader_util.h"

using namespace lord;
using namespace azer;

namespace {
// class TexPosNormalVertex
const VertexDesc::Desc kVertexDescArray[] = {
  {"POSITION", 0, kVec4, 0, 0, false},
};
}  // namespace

IMPLEMENT_EFFECT_DYNCREATE(ShadowDepthEffect);
const char ShadowDepthEffect::kEffectName[] = "ShadowDepthEffect";
ShadowDepthEffect::ShadowDepthEffect() {
  vertex_desc_ptr_ = new VertexDesc(kVertexDescArray, arraysize(kVertexDescArray));
  Effect::ShaderPrograms shaders;
  CHECK(LoadShaderAtStage(kVertexStage, 
                          "demo/base/hlsl/shadow_depth.hlsl.vs",
                          &shaders));
  CHECK(LoadShaderAtStage(kPixelStage, 
                          "demo/base/hlsl/shadow_depth.hlsl.ps",
                          &shaders));
  Init(shaders);
}

ShadowDepthEffect::~ShadowDepthEffect() {}

const char* ShadowDepthEffect::GetEffectName() const {
  return kEffectName;
}
bool ShadowDepthEffect::Init(const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void ShadowDepthEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
}
void ShadowDepthEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void ShadowDepthEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void ShadowDepthEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}

void ShadowDepthEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
  }
}
