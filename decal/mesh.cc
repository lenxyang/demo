#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "demo/base/base.h"

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
  std::vector<MeshPtr> meshes_;
  scoped_refptr<WorldProvider> world_provider_;
  scoped_refptr<CameraProvider> camera_provider_;
  scoped_refptr<LightProvider> light_provider_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  adapterctx->RegisteAdapter(new TexMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new RenderNodeTexEffectAdapter);
  adapterctx->RegisteAdapter(new LordEnvNodeDelegateTexEffectAdapter);
  adapterctx->RegisteAdapter(new SdkMeshMaterialEffectAdapter);
  adapterctx->RegisteAdapter(new CameraProviderSdkMeshAdapter);
  adapterctx->RegisteAdapter(new WorldProviderSdkMeshAdapter);
  adapterctx->RegisteAdapter(new LightProviderSdkMeshAdapter);

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
  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
  base::FilePath root(UTF8ToUTF16("demo"));
  scoped_ptr<FileSystem> fs(new NativeFileSystem(root));
  env->SetFileSystem(fs.Pass());

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
  light_provider_ = new LightProvider;
  light_provider_->SetDirLight(dirlight);
  light_provider_->SetSpotLight(spotlight);

  world_provider_ = new WorldProvider;

  Vector3 camera_pos(0.0f, 3.0f, 2.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);
  camera_provider_ = new CameraProvider(&camera());
  ResPath modelpath(UTF8ToUTF16("//data/sdkmesh/Helmet.sdkmesh"));
  SdkMeshData meshdata(env->file_system());;
  CHECK(meshdata.LoadFromFile(modelpath));
  CHECK(meshdata.CreateMesh(&meshes_, adapterctx));
  for (auto iter = meshes_.begin(); iter != meshes_.end(); ++iter) {
    (*iter)->AddProvider(camera_provider_);
    (*iter)->AddProvider(light_provider_);
    (*iter)->AddProvider(world_provider_);
  }

  SetClearColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  Radians rad(3.14f * 0.5f * args.delta().InSecondsF());
  world_provider_->mutable_holder()->yaw(rad);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  for (auto iter = meshes_.begin(); iter != meshes_.end(); ++iter) {
    (*iter)->Render(renderer);
  }
}
