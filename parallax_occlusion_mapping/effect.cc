#include "demo/parallax_occlusion_mapping/effect.h"

#include <stddef.h>

#include "base/basictypes.h"
#include "base/logging.h"

#include "azer/base/image.h"
#include "azer/render/render.h"
#include "azer/render/util/effects/vertex_desc.h"
#include "azer/render/util/shader_util.h"
#include "demo/base/resource_util.h"

using namespace azer;
using base::UTF8ToUTF16;

namespace lord {
namespace sandbox {
const char MyEffect::kEffectName[] = "MyEffect";
MyEffect::MyEffect(VertexDescPtr desc) 
    : Effect(RenderSystem::Current()) {
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
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, world), 1),
    GpuConstantsTable::Desc("camerapos", GpuConstantsType::kVector4,
                            offsetof(vs_cbuffer, camerapos), 1),
  };
  gpu_table_[kVertexStage] = render_system_->CreateGpuConstantsTable(
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
  gpu_table_[kPixelStage] = render_system_->CreateGpuConstantsTable(
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

namespace {
// class TexPosNormalVertex
const VertexDesc::Desc kVertexDesc[] = {
  {"POSITION", 0, kVec4, 0},
  {"NORMAL",   0, kVec4, 0},
  {"BINORMAL", 0, kVec4, 0},
  {"TANGENT",  0, kVec4, 0},
  {"TEXCOORD", 0, kVec2, 0},
};
}  // namespace

MyEffectPtr CreateMyEffect() {
  Effect::ShaderPrograms shaders;
  CHECK(LoadShaderAtStage(kVertexStage, 
                          "demo/parallax_occlusion_mapping/effect.hlsl.vs",
                          &shaders));
  CHECK(LoadShaderAtStage(kPixelStage, 
                          "demo/parallax_occlusion_mapping/effect.hlsl.ps",
                          &shaders));
  
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  MyEffectPtr ptr(new MyEffect(desc));
  ptr->Init(shaders);
  return ptr;
}

const char MaterialProvider::kEffectParamsProviderName[] = "MaterialProvider";
MaterialProvider::MaterialProvider() {
}

const char* MaterialProvider::name() const { return kEffectParamsProviderName;}
void MaterialProvider::InitFromConfigNode(ConfigNode* config, FileSystem* fs) {
  CHECK(config->GetChildTextAsFloat("ambient", &ambient_scalar_));
  CHECK(config->GetChildTextAsFloat("specular", &specular_scalar_));
  CHECK(config->GetChildTextAsVec4("emission", &emission_));
  std::string diffusemap_path, nmhmap_path;
  CHECK(config->GetChildText("diffusemap", &diffusemap_path));
  CHECK(config->GetChildText("nmhmap", &nmhmap_path));
  diffuse_map_ = Load2DTexture(ResPath(UTF8ToUTF16(diffusemap_path)), fs);
  nmh_map_ = Load2DTexture(ResPath(UTF8ToUTF16(nmhmap_path)), fs);
}
}  // namespace sandbox
}  // namespace lord
