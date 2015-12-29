#include "demo/base/base.h"

using views::Widget;
using namespace azer;
using namespace lord;

class MyRenderWindow : public lord::SceneRenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::SceneRenderWindow(rect) {}
  SceneNodePtr OnInitScene() override;
  void OnInitUI() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  scoped_ptr<UISceneRenderer> tree_render_;
  azer::RendererPtr depth_renderer_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));

  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new ShadowMapDepthEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderNodeDepthEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderEnvNodeTexEffectAdapter);

  gfx::Rect init_bounds(0, 0, 800, 600);
  MyRenderWindow* window(new MyRenderWindow(init_bounds));
  nelf::ResourceBundle* bundle = lord::LordEnv::instance()->resource_bundle();
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
  LordEnv* env = LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);
  ResPath respath(UTF8ToUTF16("//shadow/depth_scene.xml:scene"));
  VariantResource res = resloader->Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  tree_render_.reset(new UISceneRenderer);
  tree_render_->Init(root, &camera());
  LOG(ERROR) << "\n" << tree_render_->root()->DumpTree();

  RenderSystem* rs = RenderSystem::Current();
  Texture::Options opt;
  opt.target = (Texture::BindTarget)
      (Texture::kShaderResource | Texture::kRenderTarget);
  opt.format = kRGBAf;
  opt.size = gfx::Size(800, 800);
  depth_renderer_ = rs->CreateRenderer(opt);
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
  depth_renderer_->Use();
  depth_renderer_->Clear(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
  depth_renderer_->ClearDepthAndStencil();
  tree_render_->Render(depth_renderer_);

  renderer->Use();
  tree_render_->Render(renderer);
}
