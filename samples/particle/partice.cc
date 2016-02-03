#include "demo/samples/particle/partice.h"

#include "base/logging.h"
#include "azer/render/render.h"

using namespace azer;

const char ParticleEffect::kEffectName[] = "ParticleEffect";

ParticleEffect::ParticleEffect() {
  world_ = Matrix4::kIdentity;
  color_ = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
}

ParticleEffect::~ParticleEffect() {
}

bool ParticleEffect::Init(VertexDesc* desc, const Shaders& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitShaders(sources);
  InitGpuConstantTable();
  return true;
}

void ParticleEffect::ApplyGpuConstantTable(Renderer* renderer) {
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
  }
}
void ParticleEffect::InitGpuConstantTable() {
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
  };
  gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
}

typedef scoped_refptr<ParticleEffect> ParticleEffectPtr;
ParticleEffectPtr CreateParticleEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec4},
  };
  Shaders shaders;
  LoadStageShader(kVertexStage, "demo/samples/particle/vs.hlsl", &shaders);
  LoadStageShader(kGeometryStage, "demo/samples/particle/gs.hlsl", &shaders);
  LoadStageShader(kPixelStage, "demo/samples/particle/ps.hlsl", &shaders);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  TessEffectPtr ptr(new TessEffect);
  ptr->Init(desc, shaders);
  return ptr;
}

// class Particle
Particle::Particle() {
}

void Particle::SetVertexBuffer(uint32 index, VertexBuffer* vb) {
  DCHECK_LT(index, arraysize(vbs_));
  vbs_[index] = vb;
}

void Particle::Render(Renderer* renderer) {
}
