#include "demo/shadow/shadow_render_tree.h"

using namespace lord;
using namespace azer;
namespace {
// class TexPosNormalVertex
const VertexDesc::Desc kVertexDescArray[] = {
  {"POSITION", 0, kVec4, 0, 0, false},
};
}  // namespace

IMPLEMENT_EFFECT_DYNCREATE(ShadowEffect);
const char ShadowEffect::kEffectName[] = "ShadowEffect";
ShadowEffect::ShadowEffect() {
  vertex_desc_ptr_ = new VertexDesc(kVertexDescArray, arraysize(kVertexDescArray));
}

ShadowEffect::~ShadowEffect() {}

const char* ShadowEffect::GetEffectName() const {
  return kEffectName;
}
bool ShadowEffect::Init(const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void ShadowEffect::InitGpuConstantTable() {
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
}
void ShadowEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void ShadowEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void ShadowEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}

void ShadowEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
  }
}

// class ShadowDepthRenderDelegate
const char ShadowDepthRenderDelegate::kEffectProviderName[] = 
    "ShadowDepthRenderDelegate";
ShadowDepthRenderDelegate::ShadowDepthRenderDelegate(
    lord::SceneRenderNode* node, ShadowDepthRenderer* tree_renderer)
    : lord::SceneRenderNodeDelegate(node),
      tree_renderer_(tree_renderer) {
  Init();
}

ShadowDepthRenderDelegate::~ShadowDepthRenderDelegate() {
}

void ShadowDepthRenderDelegate::Init() {
}
  
void ShadowDepthRenderDelegate::Update(const azer::FrameArgs& args) {
}

void ShadowDepthRenderDelegate::Render(azer::Renderer* renderer) {
}

// class ShadowEffectAdapter
ShadowEffectAdapter::ShadowEffectAdapter() {}
EffectAdapterKey ShadowEffectAdapter::key() const {
  return std::make_pair(typeid(ShadowEffect).name(),
                        typeid(ShadowDepthRenderDelegate).name());
}

void ShadowEffectAdapter::Apply(Effect* e, const EffectParamsProvider* params) const  {
  CHECK(typeid(*e) == typeid(ShadowEffect));
  CHECK(typeid(*params) == typeid(ShadowDepthRenderDelegate));
  ShadowDepthRenderDelegate* provider = (ShadowDepthRenderDelegate*)params;
  ShadowEffect* effect = dynamic_cast<ShadowEffect*>(e);
  effect->SetWorld(provider->GetWorld());
  effect->SetPV(provider->GetPV());
}

// class ShadowRenderNodeDelegateFactory
ShadowRenderNodeDelegateFactory::ShadowRenderNodeDelegateFactory(
    ShadowDepthRenderer* renderer)
    : tree_renderer_(renderer) {
}

scoped_ptr<lord::SceneRenderNodeDelegate> ShadowRenderNodeDelegateFactory::
CreateDelegate(lord::SceneRenderNode* node) {
  scoped_ptr<lord::SceneRenderNodeDelegate> p(
      new ShadowDepthRenderDelegate(node, tree_renderer_));
  return p.Pass();
}

// class ShadowDepthRenderer
ShadowDepthRenderer::ShadowDepthRenderer()
    : root_(NULL),
      need_update_(true) {
}

ShadowDepthRenderer::~ShadowDepthRenderer() {
}

void ShadowDepthRenderer::SetLight(lord::LightPtr light) {
  light_ = light;
  need_update_ = true;
}

void ShadowDepthRenderer::UpdateNode(SceneRenderNode* node,
                                       const azer::FrameArgs& args) {
  node->Update(args);
  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    UpdateNode(iter->get(), args);
  }
}

void ShadowDepthRenderer::Update(const azer::FrameArgs& args) {
  if (need_update_) {
    UpdateNode(root_, args);
    need_update_ = false;
  }
}

void ShadowDepthRenderer::RenderNode(SceneRenderNode* node, Renderer* renderer) {
  if (!node->GetSceneNode()->visible()) {
    return;
  }

  if (node->GetSceneNode()->type() != kLampSceneNode) {
    node->Render(renderer);
  }

  for (auto iter = node->children().begin(); 
       iter != node->children().end(); ++iter) {
    RenderNode(iter->get(), renderer);
  }
}

void ShadowDepthRenderer::Render(azer::Renderer* renderer) {
  RenderNode(root_, renderer);
}

MeshPartPtr CreateShadowMeshPtr(MeshPart* part, Effect* effect) {
  MeshPartPtr newpart(new MeshPart(effect));
  int32 entity_count = part->entity_count();
  for (int32 i = 0; i < entity_count; ++i) {
    EntityPtr e(new Entity(effect->vertex_desc()));
    e->SetVertexBuffer(part->entity_at(i)->vertex_buffer_at(0), 0);
    e->SetIndicesBuffer(part->entity_at(i)->indices_buffer());
    newpart->AddEntity(e);
  }
  return newpart;
}

MeshPtr ShadowDepthRenderer::CreateShadowMesh(MeshPtr mesh) {
  MeshPtr shadow = new Mesh(effect_);
  
  return shadow;
}
