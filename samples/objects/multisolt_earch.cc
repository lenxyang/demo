#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/material.h"
#include "demo/base/textured_effect.h"
#include "demo/base/resource_util.h"

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
  scoped_refptr<TexturedEffect> effect_;
  TexturePtr earthmap_;
  EntityPtr earch_;
  SpotLight spotlight_;
  lord::DirLight dirlight_;
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
  RenderSystem* rs = RenderSystem::Current();
  LordEnv* env = LordEnv::instance();
  scoped_ptr<FileSystem> fs(new NativeFileSystem(FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  Vector3 camera_pos(0.0f, 0.0f, 5.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  // earchmap
  // class ColoredPosNormalVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec3},
    {"NORMAL", 0, kVec3},
    {"TEXCOORD", 1, kVec2},
  };
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  effect_ = new TexturedEffect;
  Effect::ShaderPrograms programs;
  CHECK(LoadShaderAtStage(kVertexStage, "demo/base/hlsl/tex.hlsl.vs", &programs));
  CHECK(LoadShaderAtStage(kPixelStage, "demo/base/hlsl/tex.hlsl.ps", &programs));
  CHECK(effect_->Init(desc, programs));
  ResPath earthmap_path(AZER_LITERAL("//data/media/earth.dds"));
  earthmap_ = Load2DTexture(earthmap_path, env->file_system());
  GeoSphereParams params;
  earch_ = CreateSphereEntity(desc, params, Matrix4::kIdentity);

  spotlight_.diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  spotlight_.ambient = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  spotlight_.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  spotlight_.position = Vector4(-3.0, 3.0f, 0.0f, 1.0f);
  spotlight_.directional = Vector4(1.0f, -1.0f, 0.0f, 0.0f);
  spotlight_.phi = cos(Degree(45.0f));
  spotlight_.theta = cos(Degree(30.0f));
  spotlight_.range = 30.0f;
  spotlight_.falloff = 0.5f;
  spotlight_.enable = 1.0f;

  dirlight_.ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  dirlight_.diffuse = Vector4(0.5f, 0.5f, 0.4f, 1.0f);
  dirlight_.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  dirlight_.directional = Vector4(1.0f, -1.0f, -1.0f, 0.0f);
  dirlight_.enable = 1.0f;
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetWorld(Matrix4::kIdentity);
  effect_->SetCameraPos(Vector4(camera().position(), 1.0f));
  effect_->SetDirLight(dirlight_);
  effect_->SetSpotLight(spotlight_);
  effect_->set_ambient_scalar(0.01f);
  effect_->set_specular_scalar(0.8f);
  effect_->set_diffuse_texture(earthmap_);
  renderer->UseEffect(effect_);
  earch_->DrawIndex(renderer);
}
