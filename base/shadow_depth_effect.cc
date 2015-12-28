#include "demo/base/shadow_depth_effect.h"

#include "base/strings/utf_string_conversions.h"
#include "azer/render/util/shader_util.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_loader.h"

using namespace lord;
using namespace azer;
using base::UTF8ToUTF16;

IMPLEMENT_EFFECT_DYNCREATE(ShadowDepthEffect);
const char ShadowDepthEffect::kEffectName[] = "ShadowDepthEffect";
ShadowDepthEffect::ShadowDepthEffect() {
  ResourceLoader* loader = LordEnv::instance()->resource_loader();
  ResPath effect_path(UTF8ToUTF16("//data/effects.xml:depth_vertex_desc"));
  VariantResource res = LoadResource(effect_path, kResTypeVertexDesc, loader);
  CHECK(res.type == kResTypeVertexDesc);
  vertex_desc_ptr_ = res.vertex_desc;
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

EffectAdapterKey SceneRenderNodeDepthEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowDepthEffect).name(),
                        typeid(lord::SceneRenderNode).name());
}

void SceneRenderNodeDepthEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const {
  CHECK(typeid(*e) == typeid(ShadowDepthEffect));
  CHECK(typeid(*params) == typeid(lord::SceneRenderNode));
  const lord::SceneRenderNode* provider = (const lord::SceneRenderNode*)params;
  ShadowDepthEffect* effect = dynamic_cast<ShadowDepthEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
}
