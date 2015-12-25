#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/effect_dict.h"
#include "demo/shadow/effect.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class ShadowDepthEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  ShadowDepthEffect();
  ~ShadowDepthEffect();

  const char* GetEffectName() const override;
  bool Init(const ShaderPrograms& source) override;

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value);
  void SetWorld(const Matrix4& value);
  static azer::Effect* CreateObject() { return new ShadowDepthEffect;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override;
  void InitTechnique(const ShaderPrograms& source);
  void InitGpuConstantTable();

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  DECLARE_EFFECT_DYNCREATE(ShadowDepthEffect);
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthEffect);
};

class SceneRenderNodeDepthEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderNodeDepthEffectAdapter() {}

  EffectAdapterKey key() const override {
    return std::make_pair(typeid(ShadowDepthEffect).name(),
                          typeid(SceneRenderNode).name());
  }
  void Apply(Effect* e, const EffectParamsProvider* params) const override {
    CHECK(typeid(*e) == typeid(ShadowDepthEffect));
    CHECK(typeid(*params) == typeid(SceneRenderNode));
    const SceneRenderNode* provider = (const SceneRenderNode*)params;
    ShadowDepthEffect* effect = dynamic_cast<ShadowDepthEffect*>(e);
    effect->SetWorld(provider->GetWorld());
    effect->SetPV(provider->camera()->GetProjViewMatrix());
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderNodeDepthEffectAdapter);
};

class MaterialDepthEffectAdapter : public EffectParamsAdapter {
 public:
  MaterialDepthEffectAdapter() {}

  EffectAdapterKey key() const override {
    return std::make_pair(typeid(ShadowDepthEffect).name(),
                          typeid(MaterialProvider).name());
  }
  void Apply(Effect* e, const EffectParamsProvider* params) const override {}
 private:
  DISALLOW_COPY_AND_ASSIGN(MaterialDepthEffectAdapter);
};
class SceneRenderEnvNodeDepthEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderEnvNodeDepthEffectAdapter() {}
  EffectAdapterKey key() const override {
    return std::make_pair(typeid(ShadowDepthEffect).name(),
                          typeid(SceneRenderEnvNode).name());
  }
  void Apply(Effect* e, const EffectParamsProvider* params) const override {}
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderEnvNodeDepthEffectAdapter);
};


class MyRenderWindow : public lord::SceneRenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::SceneRenderWindow(rect) {}
  SceneNodePtr OnInitScene() override;
  void OnInitUI() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  SceneRenderNodePtr render_root_;
  SceneRenderNodePtr bvolumn_root_;
  scoped_ptr<SimpleRenderTreeRenderer> tree_render_;
  scoped_ptr<FileSystem> fsystem_;
  EffectDict dict_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::Context::InitContext(argc, argv));

  lord::Context* ctx = lord::Context::instance();
  azer::EffectAdapterContext* adapterctx = ctx->GetEffectAdapterContext();
  
  adapterctx->RegisteAdapter(new SceneRenderNodeDepthEffectAdapter);
  adapterctx->RegisteAdapter(new MaterialDepthEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderEnvNodeDepthEffectAdapter);
  adapterctx->RegisteAdapter(new MaterialEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderNodeEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderEnvNodeEffectAdapter);
  
  gfx::Rect init_bounds(0, 0, 800, 600);
  MyRenderWindow* window(new MyRenderWindow(init_bounds));
  nelf::ResourceBundle* bundle = lord::Context::instance()->resource_bundle();
  window->SetWindowIcon(*bundle->GetImageSkiaNamed(IDR_ICON_CAPTION_RULE));
  window->SetShowIcon(true);
  window->Init();
  window->Show();

  lord::ObjectControlToolbar* toolbar =
      new lord::ObjectControlToolbar(window, window->GetInteractive());
  window->GetRenderLoop()->Run();
  return 0;
}

SceneNodePtr MyRenderWindow::OnInitScene() {
  Context* ctx = Context::instance();
  fsystem_.reset(new azer::NativeFileSystem(FilePath(UTF8ToUTF16("demo/shadow/"))));

  ResourceLoader resloader(fsystem_.get());
  InitDefaultLoader(&resloader);
  ResPath respath(UTF8ToUTF16("//depth_scene.xml:root"));
  VariantResource res = resloader.Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  tree_render_.reset(new SimpleRenderTreeRenderer);
  LoadSceneRenderNodeDelegateFactory factory(tree_render_.get());
  SceneRenderTreeBuilder builder(&factory);

  render_root_ = builder.Build(root.get(), &camera());
  tree_render_->SetSceneNode(render_root_.get());
  LOG(ERROR) << "\n" << render_root_->DumpTree();
  
  return root;
}

void MyRenderWindow::OnInitUI() {
  gfx::Rect bounds(300, 360);
  nelf::TabbedWindow* wnd = CreateSceneTreeViewWindow(bounds, root(), this);
  wnd->Dock(nelf::kDockLeft);

  SceneNodeInspectorWindow* inspector = new SceneNodeInspectorWindow(bounds, this);
  inspector->Init();
  GetInteractive()->AddObserver(inspector->inspector_pane());
  inspector->Show();
  mutable_camera()->reset(Vector3(0.0f, 8.0f, 12.0f), Vector3(0.0f, 0.0f, 0.0f),
                          Vector3(0.0f, 1.0f, 0.0f));
  inspector->Dock(nelf::kDockLeft);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  tree_render_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  tree_render_->Render(renderer);
}


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

IMPLEMENT_EFFECT_DYNCREATE(ShadowDepthEffect);
const char ShadowDepthEffect::kEffectName[] = "ShadowDepthEffect";
ShadowDepthEffect::ShadowDepthEffect() {
  VertexDescPtr desc(new VertexDesc(kVertexDescArray, arraysize(kVertexDescArray)));
  vertex_desc_ptr_ = desc;
}

ShadowDepthEffect::~ShadowDepthEffect() {
}

const char* ShadowDepthEffect::GetEffectName() const {
  return kEffectName;
}
bool ShadowDepthEffect::Init(const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  InitTechnique(sources);
  InitGpuConstantTable();
  return true;
}

void ShadowDepthEffect::InitGpuConstantTable() {
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
void ShadowDepthEffect::InitTechnique(const ShaderPrograms& sources) {
  InitShaders(sources);
}

void ShadowDepthEffect::SetPV(const Matrix4& value) {
  pv_ = value;
}
void ShadowDepthEffect::SetWorld(const Matrix4& value) {
  world_ = value;
}
void ShadowDepthEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
  }
}


