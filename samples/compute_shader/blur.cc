#include <memory>

#include "demo/base/base.h"
#include "demo/monblur/monblur_effect.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class MyRenderWindow : public lord::RenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) 
      : lord::RenderWindow(rect) {
  }

  void OnInit() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
 private:
  RenderNodePtr bvolumn_root_;
  scoped_ptr<UISceneRender> scene_render_;
  OverlayPtr overlay_;
  SceneNodePtr root_;
  TexturePtr horzblur_;
  TexturePtr vertblur_;
  RendererPtr renderer_;
  GpuComputeTaskPtr horztask_;
  GpuComputeTaskPtr verttask_;
  GpuComputeTaskDispatcherPtr dispatcher_;
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

const static char *cs_horzblur_shader = ""
    "#pragma pack_matrix(row_major)\n"
    "float weights[] = {0.0545, 0.2442, 0.4026, 0.2442, 0.0545};\n"
    "Texture2D<float4> input : register(t0);"
    "RWTexture2D<float4> output : register(u0);"
    "[numthreads(1, 10, 1)]\n"
    "void cs_main(int3 dtid : SV_DispatchThreadID,"
    "             int gidx :SV_GroupIndex) {"
    "  int2 xy = int2(dtid.x, dtid.y);\n"
    "  const int N = 800;\n"
    "  float count = 5;\n"
    "  if (dtid.x == 0) count - 2;\n"
    "  if (dtid.x == 1) count - 1;\n"
    "  if (dtid.x == N - 2) count - 1;\n"
    "  if (dtid.x == N - 1) count - 2;\n"
    "  float4 v;\n"
    "  v  = input[int2(dtid.x - 2, dtid.y)] * weights[0];\n"
    "  v += input[int2(dtid.x - 1, dtid.y)] * weights[1];\n"
    "  v += input[int2(dtid.x - 0, dtid.y)] * weights[2];\n"
    "  v += input[int2(dtid.x + 1, dtid.y)] * weights[3];\n"
    "  v += input[int2(dtid.x + 2, dtid.y)] * weights[4];\n"
    "  output[xy] = v / count;\n"
    "  output[xy] = input[xy];"
    "}";

void MyRenderWindow::OnInit() {
  LordEnv* env = LordEnv::instance();
  ResourceLoader* resloader = env->resource_loader();  
  ResPath respath(UTF8ToUTF16("//samples/compute_shader/blur.xml:scene"));
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
  root_ = root;

  RenderSystem* rs = RenderSystem::Current();
  overlay_ = rs->CreateOverlay();
  overlay_->SetBounds(gfx::RectF(-1.0f, -1.0f, 2.0f, 2.0f));
  Texture::Options opt;
  opt.size = gfx::Size(800, 600);
  opt.target = kBindTargetRenderTarget | kBindTargetShaderResource;
  renderer_ = rs->CreateRenderer(opt);

  opt.target = kBindTargetUnorderedAccess | kBindTargetShaderResource;
  vertblur_ = rs->CreateTexture(opt);
  horzblur_ = rs->CreateTexture(opt);
  dispatcher_ = rs->CreateDispatcher();
  ShaderInfo horzinfo;
  horzinfo.path = "horz.cs";
  horzinfo.code = cs_horzblur_shader;
  horzinfo.stage = kComputeStage;
  horztask_ = new GpuComputeTask(horzinfo);
  horztask_->SetInputCount(1);
  horztask_->SetOutputCount(1);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
  SceneNode* fan = root_->GetNode("//scene/node/fan");
  fan->roll(Radians(3.14f * 0.5f * args.delta().InSecondsF()));
  scene_render_->Update(args);
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  renderer_->Use();
  renderer_->Clear(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
  renderer_->ClearDepthAndStencil();
  scene_render_->Render(renderer);

  renderer->Use();
  GpuTaskParams params;
  params.thread_group_x = 800;
  params.thread_group_y = 60;
  params.thread_group_z = 1;
  horztask_->SetInputTexture(0, renderer_->GetRenderTarget(0)->GetTexture());
  horztask_->SetOutputTexture(0, horzblur_);
  dispatcher_->Dispatch(horztask_, params);

  overlay_->SetTexture(horzblur_);
  overlay_->Render(renderer);
}


