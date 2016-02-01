#include <memory>

#include "lordaeron/sandbox/sandbox.h"
#include "demo/base/textured_effect.h"
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
  EntityPtr wall_entity_;
  TexturePtr wall_tex_;
  EntityPtr mirror_entity_;
  TexturePtr ground_tex_;
  EntityPtr ground_entity_;
  TexturePtr mirror_tex_;
  Plane mirror_plane_;
  std::vector<MeshPtr> meshes_;
  scoped_refptr<WorldProvider> world_provider_;
  scoped_refptr<CameraProvider> camera_provider_;
  scoped_refptr<LightProvider> light_provider_;
  BlendingPtr blending_;

  RasterizerStatePtr rasterizer_state_;
  RasterizerStatePtr reflect_raster_state_;
  DepthStencilStatePtr mirror_state_;
  DepthStencilStatePtr reflect_state_;
  scoped_refptr<TexturedEffect> tex_effect_;
  scoped_ptr<CameraEventListener> listener_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  base::FilePath root(UTF8ToUTF16("demo"));
  scoped_ptr<FileSystem> fs(new NativeFileSystem(root));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  azer::EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
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

  ResPath modelpath(UTF8ToUTF16("//data/sdkmesh/dwarf/dwarf.sdkmesh"));
  SdkMeshData meshdata(env->file_system());;
  CHECK(meshdata.LoadFromFile(modelpath));
  CHECK(meshdata.CreateMesh(&meshes_, adapterctx));
  ResPath texeffect_path(UTF8ToUTF16("//data/effects.xml:tex_effect"));
  VariantResource res = LoadResource(texeffect_path, kResTypeEffect,
                                     env->resource_loader());
  tex_effect_ = (TexturedEffect*)res.effect.get();
  rasterizer_state_ = rs->CreateRasterizerState();
  rasterizer_state_->SetCullingMode(kCullNone);
  reflect_raster_state_ = rs->CreateRasterizerState();
  reflect_raster_state_->SetFrontFace(kClockwise);

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
  tex_effect_->SetSpotLight(spotlight);
  tex_effect_->SetDirLight(dirlight);

  Vector3 camera_pos(-10.0f, 8.0f, 10.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);
  listener_.reset(new CameraEventListener(mutable_camera()));
  view()->AddEventListener(listener_.get());

  light_provider_ = new LightProvider;
  light_provider_->SetDirLight(dirlight);
  light_provider_->SetSpotLight(spotlight);
  world_provider_ = new WorldProvider;
  camera_provider_ = new CameraProvider(&camera());
  for (auto iter = meshes_.begin(); iter != meshes_.end(); ++iter) {
    (*iter)->AddProvider(camera_provider_);
    (*iter)->AddProvider(light_provider_);
    (*iter)->AddProvider(world_provider_);
  }

  // create mirror
  GeoPlaneParams params;
  params.row = 10.0;
  params.column = 10.0;
  params.row_width = 1.0f;
  params.column_width = 1.0f;
  Matrix4 wall_mat = Scale(1.0f, 0.5f, 1.0f) * Translate(0.0f, 5.0f, -5.0f)
      * RotateX(Degree(-90.0f));
  wall_entity_ = CreatePlaneEntity(tex_effect_->vertex_desc(), params, wall_mat);
  wall_entity_->set_primitive(kTriangleList);

  Matrix4 mat = Matrix4::kIdentity;
  params.row = 10.0;
  params.column = 10.0;
  params.row_width = 1.0f;
  params.column_width = 1.0f;
  ground_entity_ = CreatePlaneEntity(tex_effect_->vertex_desc(), params, mat);
  ground_entity_->set_primitive(kTriangleList);

  mat = Scale(1.0f, 0.5f, 1.0f) * Translate(0.0f, 5.0f, -4.99f)
      * RotateX(Degree(-90.0f));
  params.row = 8.0;
  params.column = 8.0;
  params.row_width = 1.0f;
  params.column_width = 1.0f;
  mirror_entity_ = CreatePlaneEntity(tex_effect_->vertex_desc(), params, mat);
  mirror_entity_->set_primitive(kTriangleList);
  mirror_plane_.reset(Vector3(0.0f, 5.0f, -4.99f),  Vector3(3.0f, 0.0f, -4.99f), 
                      Vector3(-3.0f, 0.0f, -4.99f));

  ResPath walltex_path(UTF8ToUTF16("//data/media/brickwall.dds"));
  ResPath mirror_path(UTF8ToUTF16("//data/media/ice.dds"));
  ResPath ground_path(UTF8ToUTF16("//data/media/checkboard.dds"));
  wall_tex_ = Load2DTexture(walltex_path, env->file_system());
  mirror_tex_ = Load2DTexture(mirror_path, env->file_system());
  ground_tex_ = Load2DTexture(ground_path, env->file_system());
  SetClearColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));

  StencilOperStruct oper;
  oper.failed_op = kStencilOperKeep;
  oper.pass_op = kStencilOperReplace;
  oper.depth_failed_op = kStencilOperKeep;
  oper.stencil_func = kCompareFuncAlways;
  mirror_state_ = rs->CreateDepthStencilState();
  mirror_state_->EnableStencil(true);
  mirror_state_->EnableDepthTest(true);
  mirror_state_->SetDepthWriteMask(0x00);
  mirror_state_->SetStencilMask(0xff, 0xff);
  mirror_state_->SetFrontFaceOper(oper);
  mirror_state_->SetBackFaceOper(oper);

  oper.failed_op = kStencilOperKeep;
  oper.pass_op = kStencilOperKeep;
  oper.depth_failed_op = kStencilOperKeep;
  oper.stencil_func = kCompareFuncEqual;
  reflect_state_ = rs->CreateDepthStencilState();
  reflect_state_->EnableStencil(true);
  reflect_state_->EnableDepthTest(true);
  reflect_state_->SetDepthWriteMask(0xff);
  reflect_state_->SetStencilMask(0xff, 0xff);
  reflect_state_->SetFrontFaceOper(oper);
  reflect_state_->SetBackFaceOper(oper);

  // blending
  Blending::Desc blend_desc;
  blend_desc.src = Blending::kSrcAlpha;
  blend_desc.dest = Blending::kSrcInvAlpha;
  blend_desc.oper = Blending::kAdd;
  blend_desc.src_alpha = Blending::kOne;
  blend_desc.dest_alpha = Blending::kZero;
  blend_desc.alpha_oper = Blending::kAdd;
  blend_desc.mask = Blending::kWriteColor;
  blending_ = rs->CreateBlending(blend_desc);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  listener_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  // draw scene
  // -- draw mirror and wall
  {
    ScopedRasterizerState scoped_state(renderer, rasterizer_state_);
    tex_effect_->SetPV(camera().GetProjViewMatrix());
    tex_effect_->SetCameraPos(Vector4(camera().position(), 1.0f));
    tex_effect_->SetWorld(Matrix4::kIdentity);
    tex_effect_->set_diffuse_texture(wall_tex_);
    renderer->UseEffect(tex_effect_);
    wall_entity_->Render(renderer);

    tex_effect_->set_diffuse_texture(ground_tex_);
    renderer->UseEffect(tex_effect_);
    ground_entity_->Render(renderer);

    // draw mirror
    {
      ScopedDepthStencilState scoped_state(renderer, mirror_state_);
      tex_effect_->set_diffuse_texture(mirror_tex_);
      renderer->UseEffect(tex_effect_);
      mirror_entity_->Render(renderer);
    }

    {
      ScopedRasterizerState scoped_raster_state(renderer, reflect_raster_state_);
      ScopedDepthStencilState scoped_depth_state(renderer, reflect_state_);
      world_provider_->SetWorld(MirrorTrans(mirror_plane_));
      for (auto iter = meshes_.begin(); iter != meshes_.end(); ++iter) {
        (*iter)->Render(renderer);
      }
    }

    // draw mirror
    {
      scoped_ptr<ScopedResetBlending> autoblending_;
      autoblending_.reset(new ScopedResetBlending(renderer));
      renderer->UseBlending(blending_.get(), 0);
      world_provider_->SetWorld(Matrix4::kIdentity);
      tex_effect_->set_alpha(0.6f);
      tex_effect_->set_diffuse_texture(mirror_tex_);
      renderer->UseEffect(tex_effect_);
      mirror_entity_->Render(renderer);
      tex_effect_->set_alpha(1.0f);
    }
  }
  
  /*
  for (auto iter = meshes_.begin(); iter != meshes_.end(); ++iter) {
    (*iter)->Render(renderer);
  }
  */
}
