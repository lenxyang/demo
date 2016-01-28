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
  TexturePtr diffusemap_;
  TexturePtr nmmap_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new RenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateTexEffectAdapter);

  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

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
  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  Vector3 camera_pos(0.0f, 8.0f, 8.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateDetailmapEffect();
  GeoPlaneParams params;
  params.row = 1;
  params.column = 1;
  params.row_width = 10.0f;
  params.column_width = 10.0f;
  entity_ = CreatePlaneEntity(effect_->vertex_desc(), params, Matrix4::kIdentity);
  entity_->set_primitive(kControlPoint3);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
  state_->SetCullingMode(kCullNone);

  ResPath texpath(UTF8ToUTF16("//data/media/rocks.dds"));
  diffusemap_ = Load2DTexture(texpath, env->file_system());
  ResPath nmtexpath(UTF8ToUTF16("//data/media/rocks_NM_height.dds"));
  nmmap_ = Load2DTexture(nmtexpath, env->file_system());

  
  SpotLight spotlight;
  spotlight.diffuse = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  spotlight.ambient = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  spotlight.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  spotlight.position = Vector4(-3.0, 3.0f, 0.0f, 1.0f);
  spotlight.directional = Vector4(1.0f, -1.0f, 0.0f, 0.0f);
  spotlight.phi = cos(Degree(45.0f));
  spotlight.theta = cos(Degree(30.0f));
  spotlight.range = 30.0f;
  spotlight.falloff = 0.5f;
  spotlight.enable = 1.0f;
  
  lord::DirLight dirlight;
  dirlight.ambient = Vector4(0.1f, 0.1f, 0.1f, 0.1f);
  dirlight.diffuse = Vector4(0.5f, 0.5f, 0.4f, 1.0f);
  dirlight.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  dirlight.directional = Vector4(1.0f, -1.0f, -1.0f, 0.0f);
  dirlight.enable = 1.0f;
  effect_->SetSpotLight(spotlight);
  effect_->SetDirLight(dirlight);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  Matrix4 world = Matrix4::kIdentity;
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetWorld(world);
  effect_->SetEdgeInside(Vector4(3.0f, 3.0f, 3.0f, 1.0f));
  effect_->SetEyePos(Vector4(camera().position(), 1.0f));
  effect_->SetDiffuseMap(diffusemap_);
  effect_->SetNMMap(nmmap_);
  renderer->UseEffect(effect_);
  // renderer->SetRasterizerState(state_);
  entity_->DrawIndex(renderer);
}
