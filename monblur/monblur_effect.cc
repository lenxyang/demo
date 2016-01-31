#include "demo/monblur/monblur_effect.h"

#include "demo/base/common_provider.h"
#include "azer/render/util/shader_util.h"

using namespace azer;

// class MonblurEffect
const char MonblurEffect::kEffectName[] = "MonblurEffect";
IMPLEMENT_EFFECT_DYNCREATE(MonblurEffect);
MonblurEffect::MonblurEffect() {
  world_ = Matrix4::kIdentity;
}
bool MonblurEffect::Init(VertexDesc* desc, const azer::Shaders& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitShaders(sources);
  InitGpuConstantTable();
  return true;
}
void MonblurEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc gs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), kMaxStep),
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, world), kMaxStep),
    GpuConstantsTable::Desc("camerapos", GpuConstantsType::kVector4,
                            offsetof(vs_cbuffer, camerapos), 1),
  };
  gpu_table_[kGeometryStage] = rs->CreateGpuConstantsTable(
      arraysize(gs_table_desc), gs_table_desc);

  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("dirlight", offsetof(ps_cbuffer, dirlight),
                            sizeof(lord::DirLight), 1),
    GpuConstantsTable::Desc("spotlight", offsetof(ps_cbuffer, spotlight),
                            sizeof(lord::SpotLight), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}

void MonblurEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    GpuConstantsTable* tb = gpu_table_[(int)kGeometryStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, pv_, sizeof(Matrix4) * arraysize(pv_));
    tb->SetValue(1, world_, sizeof(Matrix4) * arraysize(world_));
    tb->SetValue(2, camerapos_, sizeof(Vector4));
  }

  {
    GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &dir_light_, sizeof(lord::DirLight));
    tb->SetValue(1, &spot_light_, sizeof(lord::SpotLight));
  }
}

void MonblurEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void MonblurEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void MonblurEffect::SetSpotLight(const lord::SpotLight& value) {
  spot_light_ = value;
}

void MonblurEffect::SetPV(const azer::Matrix4& value) {
  for (int i = 0; i < kMaxStep - 1; ++i) {
    pv_[i + 1] - pv_[i];
  }
  pv_[0] = value;
}

void MonblurEffect::SetWorld(const azer::Matrix4& value) {
  for (int i = 0; i < kMaxStep - 1; ++i) {
    world_[i + 1] - world_[i];
  }
  world_[0] = value;
}

scoped_refptr<MonblurEffect> CreateMonblurEffect() {
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
  CHECK(LoadShaderAtStage(kPixelStage, "demo/base/hlsl/sdkmesh.hlsl.ps", &s));
  CHECK(LoadShaderAtStage(kVertexStage, "demo/base/hlsl/sdkmesh.hlsl.vs", &s));
  scoped_refptr<MonblurEffect> ptr(new MonblurEffect);
  ptr->Init(desc, s);
  return ptr;
}

void MonblurEffect::UseTexture(azer::Renderer* renderer) {
  renderer->UseTexture(kPixelStage, 0, diffusemap_.get());
  renderer->UseTexture(kPixelStage, 1, normalmap_.get());
  renderer->UseTexture(kPixelStage, 2, specularmap_.get());
}

// class MonblurMaterialEffectAdapter
MonblurMaterialEffectAdapter::MonblurMaterialEffectAdapter() {}
EffectAdapterKey MonblurMaterialEffectAdapter::key() const {
  return std::make_pair(typeid(MonblurEffect).name(),
                        typeid(SdkMeshMaterial).name());
}

void MonblurMaterialEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MonblurEffect));
  CHECK(typeid(*params) == typeid(SdkMeshMaterial));
  const SdkMeshMaterial* provider = (const SdkMeshMaterial*)params;
  MonblurEffect* effect = dynamic_cast<MonblurEffect*>(e);
  effect->SetDiffuseMap(provider->diffusemap());
  effect->SetNormalMap(provider->normalmap());
  effect->SetSpecularMap(provider->specularmap());
}

// class RenderNodeMonblurEffectAdapter
using namespace lord;
RenderNodeMonblurEffectAdapter::RenderNodeMonblurEffectAdapter() {}
EffectAdapterKey RenderNodeMonblurEffectAdapter::key() const {
  return std::make_pair(typeid(MonblurEffect).name(),
                        typeid(RenderNode).name());
}

void RenderNodeMonblurEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MonblurEffect));
  CHECK(typeid(*params) == typeid(RenderNode));
  const RenderNode* provider = (const RenderNode*)params;
  MonblurEffect* effect = dynamic_cast<MonblurEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

LordEnvNodeDelegateMonblurEffectAdapter::LordEnvNodeDelegateMonblurEffectAdapter() {}
EffectAdapterKey LordEnvNodeDelegateMonblurEffectAdapter::key() const {
  return std::make_pair(typeid(MonblurEffect).name(),
                        typeid(LordEnvNodeDelegate).name());
}

void LordEnvNodeDelegateMonblurEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MonblurEffect));
  CHECK(typeid(*params) == typeid(LordEnvNodeDelegate));
  const LordEnvNodeDelegate* provider = (const LordEnvNodeDelegate*)params;
  MonblurEffect* effect = dynamic_cast<MonblurEffect*>(e);
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

