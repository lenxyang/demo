#include "demo/base/basic_effect.h"

#include <stddef.h>

#include "base/basictypes.h"
#include "base/logging.h"

#include "azer/render/render.h"
#include "azer/render/util/effects/vertex_desc.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/ui_scene_render.h"
#include "azer/render/util/shader_util.h"

using namespace azer;
using namespace lord;

IMPLEMENT_EFFECT_DYNCREATE(BasicEffect);
const char BasicEffect::kEffectName[] = "BasicEffect";
BasicEffect::BasicEffect() {
  emission_ = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

BasicEffect::~BasicEffect() {
}

const char* BasicEffect::GetEffectName() const {
  return kEffectName;
}
bool BasicEffect::Init(azer::VertexDesc* desc, const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void BasicEffect::InitGpuConstantTable() {
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
  // generate GpuTable init for stage kPixelStage
  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("color", GpuConstantsType::kVector4,
                            offsetof(ps_cbuffer, color), 1),
    GpuConstantsTable::Desc("emission", GpuConstantsType::kVector4,
                            offsetof(ps_cbuffer, emission), 1),
    GpuConstantsTable::Desc("light", offsetof(ps_cbuffer, light),
                            sizeof(lord::DirLight), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}
void BasicEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void BasicEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void BasicEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}
void BasicEffect::SetEmission(const Vector4& value) {
  emission_ = value;
}

void BasicEffect::SetColor(const Vector4& value) {
  color_ = value;
}
void BasicEffect::SetDirLight(const lord::DirLight& value) {
  light_ = value;
}

void BasicEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
  }
  {
    GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &color_, sizeof(Vector4));
    tb->SetValue(1, &emission_, sizeof(Vector4));
    tb->SetValue(2, &light_, sizeof(lord::DirLight));
  }
}

// class BasicColorProvider
IMPLEMENT_EFFECT_PROVIDER_DYNCREATE(BasicColorProvider);
const char BasicColorProvider::kEffectProviderName[] = "BasicColorProvider";
BasicColorProvider::BasicColorProvider() {
  color_ = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
}
BasicColorProvider::~BasicColorProvider() {}
const char* BasicColorProvider::GetProviderName() const { return kEffectProviderName;}

bool BasicColorProvider::Init(const ConfigNode* node, ResourceLoadContext* ctx) {
  return true;
}

// class BasicColorEffectAdapter
BasicColorEffectAdapter::BasicColorEffectAdapter() {}

EffectAdapterKey BasicColorEffectAdapter::key() const {
  return std::make_pair(typeid(BasicEffect).name(),
                        typeid(BasicColorProvider).name());
}

void BasicColorEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(BasicEffect));
  CHECK(typeid(*params) == typeid(BasicColorProvider));
  BasicColorProvider* provider = (BasicColorProvider*)params;
  BasicEffect* effect = dynamic_cast<BasicEffect*>(e);
  effect->SetColor(provider->color());
  effect->SetEmission(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
}

// class RenderNodeBasicEffectAdapter
RenderNodeBasicEffectAdapter::RenderNodeBasicEffectAdapter() {}
EffectAdapterKey RenderNodeBasicEffectAdapter::key() const {
  return std::make_pair(typeid(BasicEffect).name(),
                        typeid(RenderNode).name());
}

void RenderNodeBasicEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(BasicEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const RenderNode* provider = (const RenderNode*)params;
  BasicEffect* effect = dynamic_cast<BasicEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
}

LordEnvNodeBasicEffectAdapter::LordEnvNodeBasicEffectAdapter() {
}

EffectAdapterKey LordEnvNodeBasicEffectAdapter::key() const {
  return std::make_pair(typeid(BasicEffect).name(),
                        typeid(LordEnvNodeDelegate).name());
}

void LordEnvNodeBasicEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(BasicEffect));
  CHECK(typeid(*params) == typeid(LordEnvNodeDelegate));
  const LordEnvNodeDelegate* provider = (const LordEnvNodeDelegate*)params;
  BasicEffect* effect = dynamic_cast<BasicEffect*>(e);
  for (auto iter = provider->lights().begin(); 
       iter != provider->lights().end();
       ++iter) {
    if ((*iter)->type() == kDirectionalLight) {
      effect->SetDirLight((*iter)->dir_light());
      break;
    }
  }
}
