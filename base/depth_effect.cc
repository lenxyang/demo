#include "demo/base/depth_effect.h"

#include "base/strings/utf_string_conversions.h"
#include "azer/render/util/shader_util.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_loader.h"
#include "demo/base/shadow_render_tree.h"

using namespace lord;
using namespace azer;
using base::UTF8ToUTF16;


const char DepthEffect::kEffectName[] = "DepthEffect";
IMPLEMENT_EFFECT_DYNCREATE(DepthEffect);

DepthEffect::DepthEffect() {
  ResourceLoader* loader = LordEnv::instance()->resource_loader();
  ResPath effect_path(UTF8ToUTF16("//data/effects.xml:depth_vertex_desc"));
  VariantResource res = LoadResource(effect_path, kResTypeVertexDesc, loader);
  CHECK(res.type == kResTypeVertexDesc);
  vertex_desc_ptr_ = res.vertex_desc;
}

DepthEffect::~DepthEffect() {}

const char* DepthEffect::GetEffectName() const {
  return kEffectName;
}
bool DepthEffect::Init(const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void DepthEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
}
void DepthEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void DepthEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void DepthEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}

void DepthEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
  }
}

EffectAdapterKey SceneRenderNodeDepthEffectAdapter::key() const {
  return std::make_pair(typeid(DepthEffect).name(), typeid(SceneRenderNode).name());
}

void SceneRenderNodeDepthEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const {
  CHECK(typeid(*e) == typeid(DepthEffect));
  CHECK(typeid(*params) == typeid(SceneRenderNode));
  const SceneRenderNode* provider = (const SceneRenderNode*)params;
  DepthEffect* effect = dynamic_cast<DepthEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}


EffectAdapterKey ShadowRenderDepthEffectAdapter::key() const {
  return std::make_pair(typeid(DepthEffect).name(),
                        typeid(ShadowDepthRenderDelegate).name());
}

void ShadowRenderDepthEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const {
  CHECK(typeid(*e) == typeid(DepthEffect));
  CHECK(typeid(*params) == typeid(ShadowDepthRenderDelegate));
  const ShadowDepthRenderDelegate* provider =
      (const ShadowDepthRenderDelegate*)params;
  DepthEffect* effect = dynamic_cast<DepthEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}
