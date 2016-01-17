#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/parallax_occlusion_mapping/effect.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class MyRenderWindow : public lord::FrameWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::FrameWindow(rect) {}
  SceneNodePtr InitScene() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  RenderNodePtr render_root_;
  RenderNodePtr bvolumn_root_;
  scoped_ptr<UISceneRender> tree_render_;
  scoped_ptr<FileSystem> fsystem_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(LordEnv::InitEnv(argc, argv));

  LordEnv* env = LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new lord::sandbox::MaterialEffectAdapter);
  adapterctx->RegisteAdapter(new lord::sandbox::RenderNodeEffectAdapter);
  adapterctx->RegisteAdapter(new lord::sandbox::LordEnvNodeDelegateEffectAdapter);

  gfx::Rect init_bounds(0, 0, 800, 600);
  MyRenderWindow* window(new MyRenderWindow(init_bounds));
  nelf::ResourceBundle* bundle = LordEnv::instance()->resource_bundle();
  window->SetWindowIcon(*bundle->GetImageSkiaNamed(IDR_ICON_CAPTION_RULE));
  window->SetShowIcon(true);
  window->Init();
  window->Show();

  lord::ObjectControlToolbar* toolbar =
      new lord::ObjectControlToolbar(window, window->GetInteractive());
  window->GetRenderLoop()->Run();
  return 0;
}

SceneNodePtr MyRenderWindow::InitScene() {
  LordEnv* env = LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new NativeFileSystem(FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());
  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);
  ResPath respath(UTF8ToUTF16("//parallax_occlusion_mapping/scene.xml"));
  VariantResource res = resloader->Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  tree_render_.reset(new UISceneRender);
  tree_render_->Init(root, &camera());
  
  return root;
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  tree_render_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  tree_render_->Render(renderer);
}
