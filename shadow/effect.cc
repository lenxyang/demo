#include "demo/shadow/effect.h"

#include <stddef.h>

#include "base/basictypes.h"
#include "base/logging.h"

#include "azer/base/image.h"
#include "azer/render/render.h"
#include "azer/render/util/effects/vertex_desc.h"
#include "azer/render/util/shader_util.h"
#include "lordaeron/resource/resource_loader.h"
#include "demo/base/resource_util.h"
#include "lordaeron/scene/scene_node.h"
#include "lordaeron/scene/scene_render_tree.h"

using namespace azer;
using base::UTF8ToUTF16;

namespace lord {
namespace sandbox {

namespace {
// class TexPosNormalVertex
const VertexDesc::Desc kVertexDescArray[] = {
  {"POSITION", 0, kVec4, 0, 0, false},
  {"NORMAL",   0, kVec4, 1, 0, false},
  {"BINORMAL", 0, kVec4, 1, 0, false},
  {"TANGENT",  0, kVec4, 1, 0, false},
  {"TEXCOORD", 0, kVec2, 1, 0, false},
};
}  // namespace

IMPLEMENT_EFFECT_DYNCREATE(MyEffect);
const char MyEffect::kEffectName[] = "MyEffect";
MyEffect::MyEffect() {
  VertexDescPtr desc(new VertexDesc(kVertexDescArray, arraysize(kVertexDescArray)));
  vertex_desc_ptr_ = desc;
}

MyEffect::~MyEffect() {
}

const char* MyEffect::GetEffectName() const {
  return kEffectName;
}
bool MyEffect::Init(const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void MyEffect::InitGpuConstantTable() {
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
    GpuConstantsTable::Desc("pad1", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, pad1), 1),
    GpuConstantsTable::Desc("pad2", GpuConstantsType::kFloat,
                            offsetof(ps_cbuffer, pad2), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}
void MyEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void MyEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void MyEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}
void MyEffect::SetCameraPos(const Vector4& value) {
  camerapos_ = value;
}

void MyEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void MyEffect::SetPointLight(const PointLight& value) {
  point_light_ = value;
}

void MyEffect::SetSpotLight(const SpotLight& value) {
  spot_light_ = value;
}

void MyEffect::ApplyGpuConstantTable(Renderer* renderer) {
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
    tb->SetValue(5, &specular_scalar_, sizeof(float));
    tb->SetValue(6, &specular_scalar_, sizeof(float));
  }
}

void MyEffect::UseTexture(Renderer* renderer) {
  renderer->UseTexture(kPixelStage, 0, diffuse_map_.get());
  renderer->UseTexture(kPixelStage, 1, nmh_map_.get());
}

// class MaterialProvider
IMPLEMENT_EFFECT_PROVIDER_DYNCREATE(MaterialProvider);
const char MaterialProvider::kEffectProviderName[] = "MaterialProvider";
MaterialProvider::MaterialProvider() {
}

bool MaterialProvider::Init(const azer::ConfigNode* node, ResourceLoadContext* ctx) {
  CHECK(node->GetChildTextAsFloat("ambient", &ambient_scalar_));
  CHECK(node->GetChildTextAsFloat("specular", &specular_scalar_));
  CHECK(node->GetChildTextAsVec4("emission", &emission_));
  std::string diffusemap_path, nmhmap_path;
  CHECK(node->GetChildText("diffusemap", &diffusemap_path));
  CHECK(node->GetChildText("nmhmap", &nmhmap_path));
  diffuse_map_ = Load2DTexture(ResPath(UTF8ToUTF16(diffusemap_path)), ctx->filesystem);
  nmh_map_ = Load2DTexture(ResPath(UTF8ToUTF16(nmhmap_path)), ctx->filesystem);
  return true;
}

const char* MaterialProvider::GetProviderName() const {
  return kEffectParamsProviderName;
}

// class MaterialEffectAdapter
MaterialEffectAdapter::MaterialEffectAdapter() {}

EffectAdapterKey MaterialEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(MaterialProvider).name());
}

void MaterialEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(MaterialProvider));
  MaterialProvider* provider = (MaterialProvider*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  effect->set_ambient_scalar(provider->ambient_scalar());
  effect->set_specular_scalar(provider->specular_scalar());
  effect->set_diffuse_texture(provider->diffuse_map());
  effect->set_nmh_texture(provider->nmh_map());
}

// class SceneRenderNodeMyEffectAdapter
SceneRenderNodeEffectAdapter::SceneRenderNodeEffectAdapter() {}
EffectAdapterKey SceneRenderNodeEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(SceneRenderNode).name());
}

void SceneRenderNodeEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(SceneRenderNode));
  const SceneRenderNode* provider = (const SceneRenderNode*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->camera()->GetProjViewMatrix());
  effect->SetCameraPos(Vector4(provider->camera()->position(), 1.0f));
}

SceneRenderEnvNodeEffectAdapter::SceneRenderEnvNodeEffectAdapter() {
}

EffectAdapterKey SceneRenderEnvNodeEffectAdapter::key() const {
  return std::make_pair(typeid(MyEffect).name(),
                        typeid(SceneRenderEnvNode).name());
}

void SceneRenderEnvNodeEffectAdapter::Apply(
    Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(MyEffect));
  CHECK(typeid(*params) == typeid(SceneRenderEnvNode));
  const SceneRenderEnvNode* provider = (const SceneRenderEnvNode*)params;
  MyEffect* effect = dynamic_cast<MyEffect*>(e);
  for (auto iter = provider->lights().begin(); 
       iter != provider->lights().end();
       ++iter) {
    Light* light = iter->get();
    if (light->type() == kDirectionalLight) {
      effect->SetDirLight(light->dir_light());
    } else if (light->type() == kPointLight) {
      effect->SetPointLight(light->point_light());
    } else if (light->type() == kSpotLight) {
      effect->SetSpotLight(light->spot_light());
    }
  }
}
}  // namespace sandbox
}  // namespace lord
