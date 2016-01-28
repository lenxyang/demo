#include "demo/detailmap/detailmap_effect.h"

#include "azer/render/util/shader_util.h"

using namespace azer;
const char DetailmapEffect::kEffectName[] = "DetailmapEffect";
DetailmapEffect::DetailmapEffect() {
  world_ = Matrix4::kIdentity;
}
DetailmapEffect::~DetailmapEffect() {}
bool DetailmapEffect::Init(azer::VertexDesc* desc, const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitShaders(sources);
  InitGpuConstantTable();
  return true;
}

void DetailmapEffect::ApplyGpuConstantTable(azer::Renderer* renderer)  {
  {
    GpuConstantsTable* tb = gpu_table_[(int)kDomainStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pv_, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
    tb->SetValue(2, &eyepos_, sizeof(Vector4));
  }
  {
    GpuConstantsTable* tb = gpu_table_[(int)kHullStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &edge_, sizeof(Vector4));
  }

  {
    GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &dir_light_, sizeof(lord::DirLight));
    tb->SetValue(1, &spot_light_, sizeof(lord::SpotLight));
  }
}
void DetailmapEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc ds_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(ds_cbuffer, pvw), 1),
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(ds_cbuffer, world), 1),
    GpuConstantsTable::Desc("eyepos", GpuConstantsType::kVector4,
                            offsetof(ds_cbuffer, eyepos), 1),
  };
  gpu_table_[kDomainStage] = rs->CreateGpuConstantsTable(
      arraysize(ds_table_desc), ds_table_desc);

  // generate GpuTable init for stage kPixelStage
  GpuConstantsTable::Desc hs_table_desc[] = {
    GpuConstantsTable::Desc("edge", GpuConstantsType::kVector4,
                            offsetof(hs_cbuffer, edge), 1),
  };
  gpu_table_[kHullStage] = rs->CreateGpuConstantsTable(
      arraysize(hs_table_desc), hs_table_desc);

  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("dirlight", offsetof(ps_cbuffer, dirlight),
                            sizeof(lord::DirLight), 1),
    GpuConstantsTable::Desc("spotlight", offsetof(ps_cbuffer, spotlight),
                            sizeof(lord::SpotLight), 1),
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}

void DetailmapEffect::SetDirLight(const lord::DirLight& value) {
  dir_light_ = value;
}

void DetailmapEffect::SetSpotLight(const lord::SpotLight& value) {
  spot_light_ = value;
}

void DetailmapEffect::UseTexture(azer::Renderer* renderer) {
  renderer->UseTexture(kPixelStage, 0, diffusemap_.get());
  renderer->UseTexture(kPixelStage, 1, nmmap_.get());
}

DetailmapEffectPtr CreateDetailmapEffect() {
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec3},
    {"NORMAL", 0, kVec3},
    {"TEXCOORD", 0, kVec2},
    {"TANGENT", 0, kVec3},
  };
  Effect::ShaderPrograms s;
  s.resize(kRenderPipelineStageNum);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  CHECK(LoadShaderAtStage(kVertexStage, "demo/detailmap/detailmap.vs.hlsl", &s));
  CHECK(LoadShaderAtStage(kHullStage, "demo/detailmap/detailmap.hs.hlsl", &s));
  CHECK(LoadShaderAtStage(kDomainStage, "demo/detailmap/detailmap.ds.hlsl", &s));
  CHECK(LoadShaderAtStage(kPixelStage, "demo/detailmap/detailmap.ps.hlsl", &s));
  scoped_refptr<DetailmapEffect> ptr(new DetailmapEffect);
  ptr->Init(desc, s);
  return ptr;
}
