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
  OverlayPtr overlay_;
  TexturePtr texture1_;
  TexturePtr texture2_;
  TexturePtr output_;
  GpuComputeTaskPtr task_;
  GpuComputeTaskDispatcherPtr dispatcher_;
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

const static char *cs_shader = ""
    "#pragma pack_matrix(row_major)\n"
    "Texture2D<float4> input1 : register(t0);"
    "Texture2D<float4> input2 : register(t1);"
    "RWTexture2D<float4> output : register(u0);"
    "[numthreads(16, 16, 1)]\n"
    "void cs_main(int3 dtid : SV_DispatchThreadID,"
    "             int gidx :SV_GroupIndex) {"
    "  int2 xy = int2(dtid.x, dtid.y);"
    "  output[xy] = (input1[xy] + input2[xy]) * 0.5f;"
    "}";

void MyRenderWindow::OnInit() {
  RenderSystem* rs = RenderSystem::Current();
  LordEnv* env = LordEnv::instance();
  scoped_ptr<FileSystem> fs(new NativeFileSystem(FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResPath tex1path(AZER_LITERAL("//data/media/flare.dds"));
  ResPath tex2path(AZER_LITERAL("//data/media/flarealpha.dds"));
  texture1_ = Load2DTexture(tex1path, env->file_system());
  texture2_ = Load2DTexture(tex2path, env->file_system());
  overlay_ = rs->CreateOverlay();

  Texture::Options opt = texture2_->options();
  opt.target |= kBindTargetUnorderedAccess;
  output_ = rs->CreateTexture(opt);
  dispatcher_ = rs->CreateDispatcher();
  ShaderInfo info;
  info.path = "vertadd.cs";
  info.code = cs_shader;
  info.stage = kComputeStage;
  task_ = new GpuComputeTask(info);
  task_->SetInputCount(2);
  task_->SetOutputCount(1);
  task_->SetInputTexture(0, texture1_);
  task_->SetInputTexture(1, texture2_);
  task_->SetOutputTexture(0, output_);
  
  Vector3 camera_pos(0.0f, 0.0f, 5.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  GpuTaskParams params;
  params.thread_group_x = 32;
  params.thread_group_y = 32;
  params.thread_group_z = 1;
  dispatcher_->Dispatch(task_.get(), params);

  LordEnv* context = LordEnv::instance();
  BlendingPtr blending = context->GetDefaultBlending();
  renderer->UseBlending(blending.get(), 0);
  overlay_->SetTexture(texture1_);
  overlay_->SetBounds(gfx::RectF(-0.75f, -0.25f, 0.3f, 0.3f));
  overlay_->Render(renderer);

  overlay_->SetTexture(texture2_);
  overlay_->SetBounds(gfx::RectF(-0.40f, -0.25f, 0.3f, 0.3f));
  overlay_->Render(renderer);

  overlay_->SetTexture(output_);
  overlay_->SetBounds(gfx::RectF(-0.05f, -0.25f, 0.3f, 0.3f));
  overlay_->Render(renderer);
  renderer->ResetBlending();
}
