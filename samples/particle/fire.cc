#include <memory>

#include "azer/render/geometry.h"
#include "base/rand_util.h"_
#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/base.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

#pragma pack(push, 1)
struct Particle {
  float initpos[3];
  float velocity[3];
  float size[2];
  float age;
  uint32 type;
};
#pragma pack(pop)

class StreamOutEffect : public Effect {
 public:
  static const char kEffectName[];
  StreamOutEffect() {}
  ~StreamOutEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(VertexDesc* desc, const Shaders& sources) override {
    DCHECK(sources.size() == kRenderPipelineStageNum);
    DCHECK(desc);
    vertex_desc_ = desc;
    InitShaders(sources);
    InitGpuConstantTable();
    return true;
  }

#pragma pack(push, 4)
  struct gs_cbuffer {
    Vector3 initpos;
    float game_time;
  };
#pragma pack(pop)

  void SetInitPos(const Vector3& value) {initpos_ = value;}
  void SetGameTime(float t) { game_time_ = t;}
  void SetRandomTex(Texture* tex) { random_tex_ = tex;}
 protected:
  void ApplyGpuConstantTable(Renderer* renderer) override {
    {
      GpuConstantsTable* tb = gpu_table_[(int)kGeometryStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &initpos_, sizeof(initpos_));
      tb->SetValue(1, &game_time_, sizeof(game_time_));
    }
  }
  void InitGpuConstantTable() {
    RenderSystem* rs = RenderSystem::Current();
    // generate GpuTable init for stage kVertexStage
    GpuConstantsTable::Desc gs_table_desc[] = {
      GpuConstantsTable::Desc("initpos", GpuConstantsType::kVector3,
                              offsetof(gs_cbuffer, initpos), 1),
      GpuConstantsTable::Desc("game_time", GpuConstantsType::kFloat,
                              offsetof(gs_cbuffer, game_time), 1),
    };
    gpu_table_[kGeometryStage] = rs->CreateGpuConstantsTable(
        arraysize(gs_table_desc), gs_table_desc);
  }

  void UseTexture(azer::Renderer* renderer) override {
    renderer->UseTexture(kGeometryStage, 0, random_tex_);
  }

  Vector3 initpos_;
  float game_time_;
  TexturePtr random_tex_;
  DISALLOW_COPY_AND_ASSIGN(StreamOutEffect);
};

class FireEffect : public Effect {
 public:
  static const char kEffectName[];
  FireEffect() {}
  ~FireEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(VertexDesc* desc, const Shaders& sources) override {
    DCHECK(sources.size() == kRenderPipelineStageNum);
    DCHECK(desc);
    vertex_desc_ = desc;
    InitShaders(sources);
    InitGpuConstantTable();
    return true;
  }

#pragma pack(push, 4)
  struct gs_cbuffer {
    Matrix4 pv;
    Vector3 eyepos;
  };
#pragma pack(pop)

  void SetEyepos(const Vector3& value) {eyepos_ = value;}
  void SetTexture(Texture* tex) { texture_ = tex;}
  void SetPV(const Matrix4& pv) { pv_ = pv;}
 protected:
  void ApplyGpuConstantTable(Renderer* renderer) override {
    {
      GpuConstantsTable* tb = gpu_table_[(int)kGeometryStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &pv_, sizeof(pv_));
      tb->SetValue(1, &eyepos_, sizeof(eyepos_));
    }
  }
  void InitGpuConstantTable() {
    RenderSystem* rs = RenderSystem::Current();
    // generate GpuTable init for stage kVertexStage
    GpuConstantsTable::Desc gs_table_desc[] = {
      GpuConstantsTable::Desc("pv", GpuConstantsType::kMatrix4,
                              offsetof(gs_cbuffer, pv), 1),
      GpuConstantsTable::Desc("eyepos", GpuConstantsType::kVector3,
                              offsetof(gs_cbuffer, eyepos), 1),
    };
    gpu_table_[kGeometryStage] = rs->CreateGpuConstantsTable(
        arraysize(gs_table_desc), gs_table_desc);
  }

  void UseTexture(azer::Renderer* renderer) override {
    renderer->UseTexture(kPixelStage, 0, texture_);
  }

  Matrix4 pv_;
  Vector3 eyepos_;
  TexturePtr texture_;
  DISALLOW_COPY_AND_ASSIGN(FireEffect);
};
const char FireEffect::kEffectName[] = "FireEffect";

typedef scoped_refptr<StreamOutEffect> StreamOutEffectPtr;
StreamOutEffectPtr CreateStreamOutEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"INITPOS", 0, kVec3},
    {"VELOCITY", 0, kVec3},
    {"SIZE", 0, kVec2},
    {"AGE", 0, kFloat},
    {"TYPE", 0, kUint},
  };
  Shaders shaders;
  LoadStageShader(kVertexStage, "demo/samples/particle/fire/so.vs.hlsl", &shaders);
  LoadStageShader(kGeometryStage, "demo/samples/particle/fire/so.gs.hlsl", &shaders);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  StreamOutEffectPtr ptr(new StreamOutEffect);
  ptr->Init(desc, shaders);
  return ptr;
}

typedef scoped_refptr<FireEffect> FireEffectPtr;
FireEffectPtr CreateFireEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"INITPOS", 0, kVec3},
    {"VELOCITY", 0, kVec3},
    {"SIZE", 0, kVec2},
    {"AGE", 0, kFloat},
    {"TYPE", 0, kUint},
  };
  Shaders shaders;
  LoadStageShader(kVertexStage, "demo/samples/particle/fire/vs.hlsl", &shaders);
  LoadStageShader(kGeometryStage, "demo/samples/particle/fire/gs.hlsl", &shaders);
  LoadStageShader(kPixelStage, "demo/samples/particle/fire/ps.hlsl", &shaders);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  FireEffectPtr ptr(new FireEffect);
  ptr->Init(desc, shaders);
  return ptr;
}

class MyRenderWindow : public lord::RenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) 
      : lord::RenderWindow(rect),
        wichi_(0) {
  }
  void OnInit() override;
  void OnUpdateFrame(const FrameArgs& args) override;
  void OnRenderFrame(const FrameArgs& args, Renderer* renderer) override;
 private:
  VertexBufferPtr initvb_;
  VertexBufferPtr particlevb_[2];
  StreamOutEffectPtr streamout_effect_;
  FireEffectPtr effect_;
  TexturePtr texture_;
  TexturePtr randomtex_;
  RasterizerStatePtr state_;
  int32 which_;
  DISALLOW_COPY_AND_ASSIGN(MyRenderWindow);
};

int main(int argc, char* argv[]) {
  CHECK(lord::LordEnv::InitEnv(argc, argv));
  lord::LordEnv* env = lord::LordEnv::instance();
  scoped_ptr<FileSystem> fs(new NativeFileSystem(FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  EffectAdapterContext* adapterctx = env->GetEffectAdapterContext();
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
  FileSystem* fs = env->file_system();
  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  Vector3 camera_pos(0.0f, 0.0f, 20.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateFireEffect();
  Particle particle;
  particle.initpos[0] = 0.0f;
  particle.initpos[1] = 0.0f;
  particle.initpos[2] = 0.0f;
  particle.velocity[0] = 0.0f;
  particle.velocity[1] = 0.0f;
  particle.velocity[2] = 0.0f;
  particle.size[0] = 2.0f;
  particle.size[1] = 2.0f;
  particle.type = 1;
  SlotVertexDataPtr vdata(new SlotVertexData(effect_->vertex_desc(), 1));
  memcpy(vdata->pointer(), &particle, sizeof(particle));
  VertexBuffer::Options vbopt;
  vbopt.target = kBindTargetVertexBuffer | kBindTargetStreamOut;
  initvb_ = rs->CreateVertexBuffer(vbopt, vdata);
  particlevb_[0] = rs->CreateVertexBuffer(vbopt, vdata);
  particlevb_[1] = rs->CreateVertexBuffer(vbopt, vdata);
  ResPath respath(UTF8ToUTF16("//samples/particle/tex/flare0.dds"));
  texture_ = Load2DTexture(respath, fs);
  state_ = rs->CreateRasterizerState();
  state_->SetCullingMode(kCullNone);

  // init random vertex
  ::base::PseudoRandom random(0);
  const int32 kRandomWidth = 1024;
  ImageDataPtr data(new ImageData(kRandomWidth, 1, kRGBAnf));
  Vector4* cur = data->data();
  for (int i = 0; i < RandomWidth; ++i, ++cur) {
    float x = ::base::RandDouble();
    float y = ::base::RandDouble();
    float z = ::base::RandDouble();
    float w = 0.0f;
    *cur = Vector4(x, y, z, w);
  }
  ImagePtr img(new Image(data, kTex2D));
  Texture::Options texopt;
  texopt.size = gfx::Size(kRandomWidth, 1);
  texopt.format = kRGBAf;
  randomtex_ = rs->CreateTexture(texopt, img);
  streamout_effect_ = CreateStreamOutEffect();
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  renderer->SetRasterizerState(state_);

  which_ ^= 1;
  int cur = which_;
  int prev = which_ ^ 1;
  
  
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetEyepos(camera().position());
  effect_->SetTexture(texture_);
  renderer->UseEffect(effect_);
  initvb_->Draw(renderer);
}
