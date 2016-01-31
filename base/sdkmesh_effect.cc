#include "demo/base/sdkmesh_effect.h"

#include "demo/base/common_provider.h"

using namespace azer;

// class SdkMeshEffect
const char SdkMeshEffect::kEffectName[] = "SdkMeshEffect";
SdkMeshEffect::SdkMeshEffect() {
  world_ = Matrix4::kIdentity;
}
bool SdkMeshEffect::Init(VertexDesc* desc, const azer::Shaders& sources) {
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
    GpuConstantsTable::Desc("dirlight", offsetof(ps_cbuffer, dirlight),
                            sizeof(lord::DirLight), 1),
    GpuConstantsTable::Desc("spotlight", offsetof(ps_cbuffer, spotlight),
                            sizeof(lord::SpotLight), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}

void SdkMeshEffect::ApplyGpuConstantTable(Renderer* renderer) {
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
    tb->SetValue(0, &dir_light_, sizeof(lord::DirLight));
    tb->SetValue(1, &spot_light_, sizeof(lord::SpotLight));
  }
}

void SdkMeshEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void SdkMeshEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void SdkMeshEffect::SetSpotLight(const lord::SpotLight& value) {
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
  azer::Shaders s;
  s.resize(kRenderPipelineStageNum);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  CHECK(LoadStageShader(kPixelStage, "demo/base/hlsl/sdkmesh.ps.hlsl", &s));
  CHECK(LoadStageShader(kVertexStage, "demo/base/hlsl/sdkmesh.vs.hlsl", &s));
  scoped_refptr<SdkMeshEffect> ptr(new SdkMeshEffect);
  ptr->Init(desc, s);
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
  CHECK(typeid(*params) == typeid(SdkMeshMaterial));
  const SdkMeshMaterial* provider = (const SdkMeshMaterial*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  effect->SetDiffuseMap(provider->diffusemap());
  effect->SetNormalMap(provider->normalmap());
  effect->SetSpecularMap(provider->specularmap());
}


// class CameraProviderSdkMeshEffectProvider
CameraProviderSdkMeshAdapter::CameraProviderSdkMeshAdapter() {}
EffectAdapterKey CameraProviderSdkMeshAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(CameraProvider).name());
}

void CameraProviderSdkMeshAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(CameraProvider));
  const CameraProvider* provider = (const CameraProvider*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  effect->SetPV(provider->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->GetCameraPos(), 1.0f));
}


// class CameraProviderSdkMeshEffectProvider
WorldProviderSdkMeshAdapter::WorldProviderSdkMeshAdapter() {}
EffectAdapterKey WorldProviderSdkMeshAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(WorldProvider).name());
}

void WorldProviderSdkMeshAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(WorldProvider));
  const WorldProvider* provider = (const WorldProvider*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  effect->SetWorld(provider->world());
}


// class LightProviderSdkMeshEffectProvider
LightProviderSdkMeshAdapter::LightProviderSdkMeshAdapter() {}
EffectAdapterKey LightProviderSdkMeshAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(LightProvider).name());
}

void LightProviderSdkMeshAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(LightProvider));
  const LightProvider* provider = (const LightProvider*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  effect->SetSpotLight(provider->spot_light());
  effect->SetDirLight(provider->dir_light());
}


// class RenderNodeSdkMeshEffectAdapter
using namespace lord;
RenderNodeSdkMeshEffectAdapter::RenderNodeSdkMeshEffectAdapter() {}
EffectAdapterKey RenderNodeSdkMeshEffectAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(RenderNode).name());
}

void RenderNodeSdkMeshEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const RenderNode* provider = (const RenderNode*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

LordEnvNodeDelegateSdkMeshEffectAdapter::LordEnvNodeDelegateSdkMeshEffectAdapter() {}
EffectAdapterKey LordEnvNodeDelegateSdkMeshEffectAdapter::key() const {
  return std::make_pair(typeid(SdkMeshEffect).name(),
                        typeid(LordEnvNodeDelegate).name());
}

void LordEnvNodeDelegateSdkMeshEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(SdkMeshEffect));
  CHECK(typeid(*params) == typeid(LordEnvNodeDelegate));
  const LordEnvNodeDelegate* provider = (const LordEnvNodeDelegate*)params;
  SdkMeshEffect* effect = dynamic_cast<SdkMeshEffect*>(e);
  for (auto iter = provider->lights().begin(); 
       iter != provider->lights().end();
       ++iter) {
    lord::Light* light = iter->get();
    if (light->type() == kDirectionalLight) {
      effect->SetDirLight(light->dir_light());
    } else if (light->type() == kSpotLight) {
      effect->SetSpotLight(light->spot_light());
    }
  }
}

