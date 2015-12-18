#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "demo/parallax_occlusion_mapping/scene_loader.h"
#include "demo/parallax_occlusion_mapping/effect.h"
#include "demo/parallax_occlusion_mapping/effect_adapter.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;
using lord::sandbox::SimpleSceneNodeLoader;

class RendererInfoPane;

class MyRenderWindow : public lord::SceneRenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::SceneRenderWindow(rect) {}
  SceneNodePtr OnInitScene() override;
  void OnInitUI() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  sandbox::MyEffectPtr effect_;

  SceneRenderNodePtr render_root_;
  SceneRenderNodePtr bvolumn_root_;
  scoped_ptr<SimpleRenderTreeRenderer> tree_render_;
  scoped_ptr<FileSystem> fsystem_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::Context::InitContext(argc, argv));

  lord::Context* ctx = lord::Context::instance();
  azer::EffectAdapterContext* adapterctx = ctx->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new lord::sandbox::MaterialEffectAdapter);
  adapterctx->RegisteAdapter(new lord::sandbox::SceneRenderNodeEffectAdapter);
  adapterctx->RegisteAdapter(new lord::sandbox::SceneRenderEnvNodeEffectAdapter);

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
  effect_ = sandbox::CreateMyEffect();
  Context* ctx = Context::instance();
  fsystem_.reset(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/parallax_occlusion_mapping/"))));

  scoped_ptr<SceneNodeLoader> light_loader(new LightNodeLoader());
  scoped_ptr<SceneNodeLoader> env_loader(new EnvNodeLoader());
  scoped_ptr<SimpleSceneNodeLoader> node_loader(new SimpleSceneNodeLoader(
      fsystem_.get(), effect_.get()));
  SceneLoader loader(fsystem_.get());
  loader.RegisterSceneNodeLoader(node_loader.Pass());
  loader.RegisterSceneNodeLoader(env_loader.Pass());
  loader.RegisterSceneNodeLoader(light_loader.Pass());
  SceneNodePtr root = loader.Load(ResPath(UTF8ToUTF16("//scene.xml")), "//");

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
