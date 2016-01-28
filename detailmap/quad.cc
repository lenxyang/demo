#include <memory>

#include "azer/render/geometry.h"
#include "demo/base/base.h"
#include "demo/detailmap/detailmap_effect.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class MyRenderWindow : public lord::RenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::RenderWindow(rect) {}
  void OnInit() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  EntityPtr entity_;
  DetailmapEffectPtr effect_;
  RasterizerStatePtr state_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
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

  window->GetRenderLoop()->Run();
  return 0;
}

void MyRenderWindow::OnInit() {
  LordEnv* env = LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  Vector3 camera_pos(0.0f, 0.0f, 3.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateDetailmapEffect();
  Vector3 points[] = {Vector3(-0.5f,  0.5f, 0.0f),
                      Vector3(-0.5f, -0.5f, 0.0f),
                      Vector3( 0.5f, -0.5f, 0.0f),
                      Vector3( 0.5f,  0.5f, 0.0f),};
  entity_ = CreateGeoPointsList(points, (int)arraysize(points), 
                                effect_->vertex_desc(), Matrix4::kIdentity);
  entity_->set_primitive(kControlPoint4);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
  state_->SetCullingMode(kCullNone);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  Vector3 position[] = {
    Vector3(-1.0f, -1.0f, 0.0f),
    Vector3(-1.0f,  1.0f, 0.0f),
    Vector3( 1.0f,  1.0f, 0.0f),
    Vector3( 1.0f, -1.0f, 0.0f),
  };

  Vector4 edge[] = {
    Vector4(4.0f, 4.0f, 4.0f, 1.0f),
    Vector4(3.0f, 3.0f, 3.0f, 3.0f),
    Vector4(3.0f, 3.0f, 3.0f, 4.0f),
    Vector4(3.0f, 5.0f, 6.0f, 6.0f),
  };

  Vector4 inside[] = {
    Vector4(4.0f, 4.0f, 4.0f, 1.0f),
    Vector4(3.0f, 3.0f, 3.0f, 1.0f),
    Vector4(3.0f, 3.0f, 3.0f, 1.0f),
    Vector4(3.0f, 5.0f, 6.0f, 1.0f),
  };

  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
  for (uint32 i = 0; i < arraysize(position); ++i) {
    Matrix4 world = Translate(position[i]);
    effect_->SetWorld(world);
    effect_->SetEdge(edge[i]);
    effect_->SetInside(inside[i]);
    renderer->UseEffect(effect_);
    renderer->SetRasterizerState(state_);
    entity_->Draw(renderer);
  }
}
