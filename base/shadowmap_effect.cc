#include "demo/base/shadowmap_effect.h"

#include "base/strings/utf_string_conversions.h"
#include "azer/render/util/shader_util.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_loader.h"
#include "demo/base/shadow_render_tree.h"

using namespace lord;
using namespace azer;
using base::UTF8ToUTF16;

const char ShadowMapEffect::kEffectName[] = "ShadowMapEffect";
IMPLEMENT_EFFECT_DYNCREATE(ShadowMapEffect);

ShadowMapEffect::ShadowMapEffect() {
}

ShadowMapEffect::~ShadowMapEffect() {}

const char* ShadowMapEffect::GetEffectName() const {
  return kEffectName;
}

bool ShadowMapEffect::Init(azer::VertexDesc* desc, const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void ShadowMapEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
}
void ShadowMapEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void ShadowMapEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void ShadowMapEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}

void ShadowMapEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
  }
}

EffectAdapterKey SceneRenderNodeShadowMapEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowMapEffect).name(), typeid(SceneRenderNode).name());
}

void SceneRenderNodeShadowMapEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const {
  CHECK(typeid(*e) == typeid(ShadowMapEffect));
  CHECK(typeid(*params) == typeid(SceneRenderNode));
  const SceneRenderNode* provider = (const SceneRenderNode*)params;
  ShadowMapEffect* effect = dynamic_cast<ShadowMapEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}
