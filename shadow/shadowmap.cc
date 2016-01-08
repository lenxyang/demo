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
      : lord::FrameWindow(rect),
        default_renderer_(true),
        switching_(false) {
  }

  SceneNodePtr InitScene() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
  void switch_renderer() {
    default_renderer_ = !default_renderer_;
    switching_ = true;
  }
 private:
  RenderNodePtr bvolumn_root_;
  scoped_ptr<EffectedSceneRenderer> effected_render_;
  scoped_ptr<UISceneRenderer> scene_render_;
  scoped_ptr<ShadowDepthRenderer> depth_render_;
  EffectDict dict_;
  bool default_renderer_;
  bool switching_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

class RendererToolbar : public nelf::Toolbar,
                        public views::ButtonListener {
 public:
  RendererToolbar(MyRenderWindow* render_window)
      : nelf::Toolbar(render_window),
        render_window_(render_window) {
    LordEnv* context = LordEnv::instance();
    int32 toolbar_id = IDR_ICON_TOOLBAR_LAYERS;

    using views::BoxLayout;
    views::View* contents = new views::View;
    contents->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 0, 1, 0));
    nelf::ResourceBundle* bundle = context->resource_bundle();
    int32 id = toolbar_id;
    const gfx::ImageSkia* img = bundle->GetImageSkiaNamed(id);
    button_ = new nelf::ToggleButton(*img);
    button_->SetInsets(gfx::Insets(1, 1, 1, 1));
    button_->SetImageLabelSpacing(0);
    button_->set_listener(this);
    button_->set_tag(id);
    button_->SetMinSize(gfx::Size(32, 32));
    button_->SetTooltipText(::base::UTF8ToUTF16("This is tooltip"));
    contents->AddChildView(button_);
    SetContents(contents);
  }
  ~RendererToolbar() {
  }
 private:
  void ButtonPressed(views::Button* sender, const ui::Event& event) override {
    render_window_->switch_renderer();
  }
  MyRenderWindow* render_window_;
  nelf::ToggleButton* button_;
  DISALLOW_COPY_AND_ASSIGN(RendererToolbar);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));

  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new RenderNodeShadowMapEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateShadowMapEffectAdapter);
  adapterctx->RegisteAdapter(new ShadowMapMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new ShadowMapDepthEffectAdapter);
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new RenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateTexEffectAdapter);

  gfx::Rect init_bounds(0, 0, 800, 600);
  MyRenderWindow* window(new MyRenderWindow(init_bounds));
  nelf::ResourceBundle* bundle = lord::LordEnv::instance()->resource_bundle();
  window->SetWindowIcon(*bundle->GetImageSkiaNamed(IDR_ICON_CAPTION_RULE));
  window->SetShowIcon(true);
  window->Init();
  window->Show();

  RendererToolbar* toolbar = new RendererToolbar(window);
  toolbar->Float();
  toolbar->Dock(0, 1);
  window->GetRenderLoop()->Run();
  return 0;
}

SceneNodePtr MyRenderWindow::InitScene() {
  LordEnv* env = LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);
  ResPath respath(UTF8ToUTF16("//shadow/scene.xml:scene"));
  VariantResource res = resloader->Load(respath);
  SceneNodePtr root = res.scene;
  CHECK(root.get()) << "Failed to init scene";

  effected_render_.reset(new EffectedSceneRenderer);
  effected_render_->Init(root, &camera());
  LOG(ERROR) << effected_render_->root()->DumpTree();

  scene_render_.reset(new UISceneRenderer);
  scene_render_->Init(root, &camera());
  LOG(ERROR) << scene_render_->root()->DumpTree();

  return root;
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  if (default_renderer_) {
    scene_render_->Update(args);
  } else {
    effected_render_->Update(args);
  }
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  if (switching_) {
    switching_ = false;
    return;
  }
  if (default_renderer_) {
    scene_render_->Render(renderer);
  } else {
    effected_render_->Render(renderer);
  }
}
