#include "demo/base/textured_effect.h"

#include "base/strings/utf_string_conversions.h"
#include "lordaeron/env.h"
#include "lordaeron/resource/resource_util.h"
#include "lordaeron/resource/resource_loader.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/ui_scene_render.h"
#include "demo/base/resource_util.h"
#include "demo/base/sdkmesh.h"
#include "demo/base/sdkmesh_effect.h"

using namespace lord;
using namespace azer;

using base::UTF8ToUTF16;

IMPLEMENT_EFFECT_DYNCREATE(TexturedEffect);
const char TexturedEffect::kEffectName[] = "TexturedEffect";
TexturedEffect::TexturedEffect()
    : ambient_scalar_(0.01f),
      specular_scalar_(1.0f),
      alpha_(1.0f) {
}

TexturedEffect::~TexturedEffect() {
}

const char* TexturedEffect::GetEffectName() const {
  return kEffectName;
}
bool TexturedEffect::Init(VertexDesc* desc, const azer::Shaders& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());

  DCHECK(desc);
  vertex_desc_ = desc;
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void TexturedEffect::InitGpuConstantTable() {
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
    GpuConstantsTable::Desc("alpha", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, alpha), 1),
    GpuConstantsTable::Desc("pad2", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, pad2), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}
void TexturedEffect::InitTechnique(const azer::Shaders& sources) {
  InitShaders(sources);
}

void TexturedEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void TexturedEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}
void TexturedEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void TexturedEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void TexturedEffect::SetPointLight(const PointLight& value) {
  point_light_ = value;
}

void TexturedEffect::SetSpotLight(const SpotLight& value) {
  spot_light_ = value;
}

void TexturedEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
    tb->SetValue(2, &camerapos_, sizeof(Vector4));
  }
  {
    GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &dir_light_, sizeof(lord::DirLight));
    tb->SetValue(1, &point_light_, sizeof(lord::PointLight));
    tb->SetValue(2, &spot_light_, sizeof(lord::SpotLight));
    tb->SetValue(3, &ambient_scalar_, sizeof(float));
    tb->SetValue(4, &specular_scalar_, sizeof(float));
    tb->SetValue(5, &alpha_, sizeof(float));
    tb->SetValue(6, &specular_scalar_, sizeof(float));
  }
}

void TexturedEffect::UseTexture(Renderer* renderer) {
  renderer->BindTexture(kPixelStage, 0, diffuse_map_.get());
}

// class RenderNodeTexturedEffectAdapter
RenderNodeTexEffectAdapter::RenderNodeTexEffectAdapter() {}
EffectAdapterKey RenderNodeTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(RenderNode).name());
}

void RenderNodeTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const RenderNode* provider = (const RenderNode*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

LordEnvNodeDelegateTexEffectAdapter::LordEnvNodeDelegateTexEffectAdapter() {
}

EffectAdapterKey LordEnvNodeDelegateTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(LordEnvNodeDelegate).name());
}

void LordEnvNodeDelegateTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(LordEnvNodeDelegate));
  const LordEnvNodeDelegate* provider = (const LordEnvNodeDelegate*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  for (auto iter = provider->lights().begin(); 
       iter != provider->lights().end();
       ++iter) {
    lord::Light* light = iter->get();
    if (light->type() == kDirectionalLight) {
      effect->SetDirLight(light->dir_light());
    } else if (light->type() == kPointLight) {
      effect->SetPointLight(light->point_light());
    } else if (light->type() == kSpotLight) {
      effect->SetSpotLight(light->spot_light());
    }
  }
}

// class SdkMeshMaterialTexEffectAdapter
SdkMeshMaterialTexEffectAdapter::SdkMeshMaterialTexEffectAdapter() {}
EffectAdapterKey SdkMeshMaterialTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(SdkMeshMaterial).name());
}

void SdkMeshMaterialTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(SdkMeshMaterial));
  const SdkMeshMaterial* provider = (const SdkMeshMaterial*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->set_diffuse_texture(provider->diffusemap());
  // effect->SetNormalMap(provider->normalmap());
  // effect->SetSpecularMap(provider->specularmap());
}

// class CameraProviderTexturedEffectProvider
CameraProviderTexEffectAdapter::CameraProviderTexEffectAdapter() {}
EffectAdapterKey CameraProviderTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(CameraProvider).name());
}

void CameraProviderTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(CameraProvider));
  const CameraProvider* provider = (const CameraProvider*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->SetPV(provider->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->GetCameraPos(), 1.0f));
}


// class CameraProviderTexturedEffectProvider
WorldProviderTexEffectAdapter::WorldProviderTexEffectAdapter() {}
EffectAdapterKey WorldProviderTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(WorldProvider).name());
}

void WorldProviderTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(WorldProvider));
  const WorldProvider* provider = (const WorldProvider*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->SetWorld(provider->world());
}


// class LightProviderTexturedEffectProvider
LightProviderTexEffectAdapter::LightProviderTexEffectAdapter() {}
EffectAdapterKey LightProviderTexEffectAdapter::key() const {
  return std::make_pair(typeid(TexturedEffect).name(),
                        typeid(LightProvider).name());
}

void LightProviderTexEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(TexturedEffect));
  CHECK(typeid(*params) == typeid(LightProvider));
  const LightProvider* provider = (const LightProvider*)params;
  TexturedEffect* effect = dynamic_cast<TexturedEffect*>(e);
  effect->SetSpotLight(provider->spot_light());
  effect->SetDirLight(provider->dir_light());
}

