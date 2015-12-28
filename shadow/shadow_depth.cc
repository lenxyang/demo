#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/effect_dict.h"
#include "demo/base/shadow_depth_effect.h"
#include "demo/base/shadow_render_tree.h"
#include "demo/base/textured_effect.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
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
  SceneRenderNodePtr render_root_;
  SceneRenderNodePtr bvolumn_root_;
  scoped_ptr<SimpleRenderTreeRenderer> tree_render_;
  scoped_ptr<ShadowDepthRenderer> depth_render_;
  EffectDict dict_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));

  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderEnvNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new SceneRenderNodeDepthEffectAdapter);

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
  ResPath respath(UTF8ToUTF16("//shadow/scene.xml"));
  VariantResource res = resloader->Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  tree_render_.reset(new SimpleRenderTreeRenderer);
  LoadSceneRenderNodeDelegateFactory factory(tree_render_.get());
  SceneRenderTreeBuilder builder(&factory);
  render_root_ = builder.Build(root.get(), &camera());
  tree_render_->SetSceneNode(render_root_.get());
  LOG(ERROR) << "\n" << render_root_->DumpTree();


  {
    SceneNode* light_node = root->GetNode("//scene/node/env/spot");
    DCHECK(light_node);
    depth_render_.reset(new ShadowDepthRenderer(resloader));
    ShadowRenderNodeDelegateFactory factory(depth_render_.get());
    SceneRenderTreeBuilder builder(&factory);
    render_root_ = builder.Build(root.get(), &camera());
    depth_render_->SetSceneNode(render_root_.get());
    depth_render_->SetLight(light_node->mutable_data()->light());
    LOG(ERROR) << "\n" << render_root_->DumpTree();
  }
  
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
  // tree_render_->Update(args);
  depth_render_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  // tree_render_->Render(renderer);
  depth_render_->Render(renderer);
}
