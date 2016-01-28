#include <memory>

#include "demo/base/base.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class MyRenderWindow : public lord::FrameWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) 
      : lord::FrameWindow(rect) {
  }

  SceneNodePtr InitScene() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  RenderNodePtr bvolumn_root_;
  scoped_ptr<UISceneRender> scene_render_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());
  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);
  resloader->RegisterSpecialLoader(new SdkMeshSpecialLoader);
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new RenderNodeShadowMapEffectAdapter);
  adapterctx->RegisteAdapter(new EffectedEnvNodeDelegateShadowMapEffectAdapter);
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new RenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateTexEffectAdapter);
  adapterctx->RegisteAdapter(new SdkMeshMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new RenderNodeSdkMeshEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateSdkMeshEffectAdapter);
  

  gfx::Rect init_bounds(0, 0, 800, 600);
  MyRenderWindow* window(new MyRenderWindow(init_bounds));
  nelf::ResourceBundle* bundle = lord::LordEnv::instance()->resource_bundle();
  window->SetWindowIcon(*bundle->GetImageSkiaNamed(IDR_ICON_CAPTION_RULE));
  window->SetShowIcon(true);
  window->Init();
  window->Show();

  window->GetRenderLoop()->Run();
  return 0;
}

SceneNodePtr MyRenderWindow::InitScene() {
  LordEnv* env = LordEnv::instance();
  ResourceLoader* resloader = env->resource_loader();  
  ResPath respath(UTF8ToUTF16("//monblur/scene.xml:scene"));
  VariantResource res = resloader->Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  Vector3 camera_pos(0.0f, 3.0f, -6.0f);
  Vector3 lookat(0.0f, 3.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  scene_render_.reset(new UISceneRender);
  scene_render_->Init(root, &camera());
  LOG(ERROR) << scene_render_->root()->DumpTree();
  return root;
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  SceneNode* fan = root()->GetNode("//scene/node/fan");
  fan->roll(Radians(3.14f * 0.5f * args.delta().InSecondsF()));
  scene_render_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  scene_render_->Render(renderer);
}

