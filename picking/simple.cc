#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/util/picking.h"
#include "demo/base/base.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class EventListener : public nelf::EventListener {
 public:
  EventListener(Camera* camera, SdkMeshData* data) 
      : camera_(camera),
        data_(data)  {
    controller_.reset(new CameraController(camera));
  }

  void Update(const azer::FrameArgs& args) { controller_->Update(args);}

  void OnKeyPressed(const ui::KeyEvent& event) override {
    controller_->OnKeyPressed(event);
  }

  void OnKeyReleased(const ui::KeyEvent& event) override {
    controller_->OnKeyReleased(event);
  }

  void OnMousePressed(const ui::MouseEvent& event) override {
    Ray ray = lord::GetPickingRay(event.location(), gfx::Size(800, 600), camera_);
    
    controller_->OnMousePressed(event);
  }

  void OnMouseDragged(const ui::MouseEvent& event) override {
    controller_->OnMouseDragged(event);
  }

  void OnMouseReleased(const ui::MouseEvent& event) override {
    controller_->OnMouseReleased(event);
  }
 private:
  Camera* camera_;
  SdkMeshData* data_;
  scoped_ptr<CameraController> controller_;
};

class MyRenderWindow : public lord::RenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::RenderWindow(rect) {}
  void OnInit() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  EntityPtr entity_;
  SdkMeshMaterialPtr mtrl_;
  scoped_refptr<SdkMeshEffect> effect_;
  scoped_ptr<EventListener> listener_;
  scoped_ptr<SdkMeshData> data_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();

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

  ResPath modelpath(UTF8ToUTF16("//data/sdkmesh/Helmet.sdkmesh"));
  data_.reset(new SdkMeshData(env->file_system()));
  CHECK(data_->LoadFromFile(modelpath));
  entity_ = data_->CreateEntity(0, 0);
  mtrl_ = data_->CreateMaterial(0);
  effect_ = CreateSdkMeshEffect();

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

  Vector3 camera_pos(0.0f, 4.0f, 2.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  SetClearColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
  listener_.reset(new ::EventListener(mutable_camera(), data_.get()));
  view()->AddEventListener(listener_.get());
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  listener_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetCameraPos(Vector4(camera().position(), 1.0f));
  effect_->SetWorld(Matrix4::kIdentity);
  effect_->SetDiffuseMap(mtrl_->diffusemap());
  effect_->SetNormalMap(mtrl_->normalmap());
  effect_->SetSpecularMap(mtrl_->specularmap());
  renderer->UseEffect(effect_);
  entity_->Render(renderer);
}
