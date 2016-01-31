#include "demo/base/shadowmap_effect.h"

#include "base/strings/utf_string_conversions.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_util.h"
#include "lordaeron/resource/resource_loader.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/ui_scene_render.h"
#include "demo/base/resource_util.h"
#include "demo/base/effected_scene_render.h"

using namespace lord;
using namespace azer;

using base::UTF8ToUTF16;

IMPLEMENT_EFFECT_DYNCREATE(ShadowMapEffect);
const char ShadowMapEffect::kEffectName[] = "ShadowMapEffect";
ShadowMapEffect::ShadowMapEffect() {
}

ShadowMapEffect::~ShadowMapEffect() {
}

const char* ShadowMapEffect::GetEffectName() const {
  return kEffectName;
}
bool ShadowMapEffect::Init(VertexDesc* desc, const azer::Shaders& sources) {
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
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, world), 1),
    GpuConstantsTable::Desc("spotlight_pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, spotlight_pv), 1),
    GpuConstantsTable::Desc("camerapos", GpuConstantsType::kVector4,
                            offsetof(vs_cbuffer, camerapos), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
  // generate GpuTable init for stage kPixelStage
  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("light", offsetof(ps_cbuffer, light),
                            sizeof(lord::DirLight), 1),
    GpuConstantsTable::Desc("pointlight", offsetof(ps_cbuffer, pointlight),
                            sizeof(lord::PointLight), 1),
    GpuConstantsTable::Desc("spotlight", offsetof(ps_cbuffer, spotlight),
                            sizeof(lord::SpotLight), 1),
    GpuConstantsTable::Desc("ambient_scalar", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, ambient_scalar), 1),
    GpuConstantsTable::Desc("specular_scalar", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, specular_scalar), 1),
    GpuConstantsTable::Desc("pad1", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, pad1), 1),
    GpuConstantsTable::Desc("pad2", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, pad2), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}
void ShadowMapEffect::InitTechnique(const azer::Shaders& sources) {
  InitShaders(sources);
}

void ShadowMapEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void ShadowMapEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}
void ShadowMapEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void ShadowMapEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void ShadowMapEffect::SetPointLight(const PointLight& value) {
  point_light_ = value;
}

void ShadowMapEffect::SetSpotLight(const SpotLight& value) {
  spot_light_ = value;
}

void ShadowMapEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
    tb->SetValue(2, &spotlight_pvw_, sizeof(Matrix4));
    tb->SetValue(3, &camerapos_, sizeof(Vector4));
  }
  {
    GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &dir_light_, sizeof(lord::DirLight));
    tb->SetValue(1, &point_light_, sizeof(lord::PointLight));
    tb->SetValue(2, &spot_light_, sizeof(lord::SpotLight));
    tb->SetValue(3, &ambient_scalar_, sizeof(float));
    tb->SetValue(4, &specular_scalar_, sizeof(float));
    tb->SetValue(5, &specular_scalar_, sizeof(float));
    tb->SetValue(6, &specular_scalar_, sizeof(float));
  }
}

void ShadowMapEffect::UseTexture(Renderer* renderer) {
  renderer->UseTexture(kPixelStage, 0, diffuse_map_.get());
  renderer->UseTexture(kPixelStage, 1, spotlight_shadowmap_.get());
}

// class RenderNodeShadowMapEffectAdapter
RenderNodeShadowMapEffectAdapter::RenderNodeShadowMapEffectAdapter() {}
EffectAdapterKey RenderNodeShadowMapEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowMapEffect).name(),
                        typeid(RenderNode).name());
}

void RenderNodeShadowMapEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(ShadowMapEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const RenderNode* provider = (const RenderNode*)params;
  ShadowMapEffect* effect = dynamic_cast<ShadowMapEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

EffectedEnvNodeDelegateShadowMapEffectAdapter
::EffectedEnvNodeDelegateShadowMapEffectAdapter() {}

EffectAdapterKey EffectedEnvNodeDelegateShadowMapEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowMapEffect).name(),
                        typeid(EffectedEnvNodeDelegate).name());
}

void EffectedEnvNodeDelegateShadowMapEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(ShadowMapEffect));
  CHECK(typeid(*params) == typeid(EffectedEnvNodeDelegate));
  const EffectedEnvNodeDelegate* provider = (const EffectedEnvNodeDelegate*)params;
  ShadowMapEffect* effect = dynamic_cast<ShadowMapEffect*>(e);
  for (int32 index = 0; index < provider->light_count(); ++index) {
    auto data = provider->light_data_at(index);
    Light* light = data->light;
    if (light->type() == kDirectionalLight) {
      effect->SetDirLight(light->dir_light());
    } else if (light->type() == kPointLight) {
      effect->SetPointLight(light->point_light());
    } else if (light->type() == kSpotLight) {
      effect->SetSpotLight(light->spot_light());
      effect->SetSpotLightShadowMap(provider->GetLightShadowmap(index));
      effect->SetSpotLightPV(data->camera.GetProjViewMatrix());
    }
  }
}
