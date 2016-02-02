#include <memory>

#include "azer/render/geometry.h"
#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/material.h"
#include "demo/base/textured_effect.h"
#include "demo/terrain/tile.h"
#include "demo/terrain/util.h"

using base::FilePath;
using base::UTF8ToUTF16;
using views::Widget;
using lord::SceneNodePtr;
using lord::SceneNode;
using namespace azer;
using namespace lord;

class NormalRenderEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  NormalRenderEffect() { world_ = Matrix4::kIdentity;  }
  ~NormalRenderEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const Shaders& sources) override {
    DCHECK(sources.size() == kRenderPipelineStageNum);
    DCHECK(!sources[kVertexStage].code.empty());
    DCHECK(!sources[kPixelStage].code.empty());
    DCHECK(desc);
    vertex_desc_ = desc;
    InitShaders(sources);
    InitGpuConstantTable();
    return true;
  }

#pragma pack(push, 4)
  struct gs_cbuffer {
    azer::Matrix4 pvw;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetHeightmap(Texture* tex) { heightmap_ = tex;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override {
    {
      GpuConstantsTable* tb = gpu_table_[(int)kGeometryStage].get();
      Matrix4 pvw = pv_ * world_;
      DCHECK(tb != NULL);
      tb->SetValue(0, &pv_, sizeof(Matrix4));
    }
  }

  void UseTexture(azer::Renderer* renderer) override {
    renderer->UseTexture(kVertexStage, 0, heightmap_);
  }
  void InitGpuConstantTable() {
    RenderSystem* rs = RenderSystem::Current();
    // generate GpuTable init for stage kVertexStage
    GpuConstantsTable::Desc gs_table_desc[] = {
      GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                              offsetof(gs_cbuffer, pvw), 1),
    };
    gpu_table_[kGeometryStage] = rs->CreateGpuConstantsTable(
        arraysize(gs_table_desc), gs_table_desc);
  }

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  TexturePtr heightmap_;
  DISALLOW_COPY_AND_ASSIGN(NormalRenderEffect);
};
const char NormalRenderEffect::kEffectName[] = "NormalRenderEffect";

typedef scoped_refptr<NormalRenderEffect> NormalRenderEffectPtr;
NormalRenderEffectPtr CreateNormalRenderEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec4},
    {"TEXCOORD", 0, kVec2},
  };
  Shaders shaders;
  LoadStageShader(kVertexStage, "demo/terrain/render_normal/vs.hlsl", &shaders);
  LoadStageShader(kGeometryStage, "demo/terrain/render_normal/gs.hlsl", &shaders);
  LoadStageShader(kPixelStage, "demo/terrain/render_normal/ps.hlsl", &shaders);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  NormalRenderEffectPtr ptr(new NormalRenderEffect);
  ptr->Init(desc, shaders);
  return ptr;
}

class TessEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  TessEffect() {
    world_ = Matrix4::kIdentity;
    color_ = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
  }
  ~TessEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const Shaders& sources) override {
    DCHECK(sources.size() == kRenderPipelineStageNum);
    DCHECK(!sources[kVertexStage].code.empty());
    DCHECK(!sources[kPixelStage].code.empty());
    DCHECK(desc);
    vertex_desc_ = desc;
    InitShaders(sources);
    InitGpuConstantTable();
    return true;
  }

#pragma pack(push, 4)
  struct ds_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
  };

  struct hs_cbuffer {
    Vector4 eyepos;
  };

  struct ps_cbuffer {
    azer::Vector4 color;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetColor(const azer::Vector4& value) {color_ = value;}
  void SetEyePos(const azer::Vector4& value) {eyepos_ = value;}
  void SetHeightmap(Texture* tex) { heightmap_ = tex;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override {
    Matrix4 pvw = std::move(pv_ * world_);
    {
      GpuConstantsTable* tb = gpu_table_[(int)kDomainStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &pvw, sizeof(Matrix4));
      tb->SetValue(1, &world_, sizeof(Matrix4));
    }
    {
      GpuConstantsTable* tb = gpu_table_[(int)kHullStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &eyepos_, sizeof(Vector4));
    }

    {
      GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &color_, sizeof(Vector4));
    }
  }

  void UseTexture(azer::Renderer* renderer) override {
    renderer->UseTexture(kVertexStage, 0, heightmap_);
    renderer->UseTexture(kDomainStage, 0, heightmap_);
  }
  void InitGpuConstantTable() {
    RenderSystem* rs = RenderSystem::Current();
    // generate GpuTable init for stage kVertexStage
    GpuConstantsTable::Desc ds_table_desc[] = {
      GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                              offsetof(ds_cbuffer, pvw), 1),
      GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                              offsetof(ds_cbuffer, world), 1),
    };
    gpu_table_[kDomainStage] = rs->CreateGpuConstantsTable(
        arraysize(ds_table_desc), ds_table_desc);

    // generate GpuTable init for stage kPixelStage
    GpuConstantsTable::Desc hs_table_desc[] = {
      GpuConstantsTable::Desc("eyepos", GpuConstantsType::kVector4,
                              offsetof(hs_cbuffer, eyepos), 1),
    };
    gpu_table_[kHullStage] = rs->CreateGpuConstantsTable(
        arraysize(hs_table_desc), hs_table_desc);

    // generate GpuTable init for stage kPixelStage
    GpuConstantsTable::Desc ps_table_desc[] = {
      GpuConstantsTable::Desc("color", GpuConstantsType::kVector4,
                              offsetof(ps_cbuffer, color), 1),
    };
    gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
        arraysize(ps_table_desc), ps_table_desc);
  }

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  Vector4 color_;
  Vector4 eyepos_;
  TexturePtr heightmap_;
  DISALLOW_COPY_AND_ASSIGN(TessEffect);
};
const char TessEffect::kEffectName[] = "TessEffect";

typedef scoped_refptr<TessEffect> TessEffectPtr;
TessEffectPtr CreateTessEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec4},
    {"TEXCOORD", 0, kVec2},
  };
  Shaders shaders;
  LoadStageShader(kVertexStage, "demo/terrain/heightmap/vs.hlsl", &shaders);
  LoadStageShader(kHullStage, "demo/terrain/heightmap/hs.hlsl", &shaders);
  LoadStageShader(kDomainStage, "demo/terrain/heightmap/ds.hlsl", &shaders);
  LoadStageShader(kPixelStage, "demo/terrain/heightmap/ps.hlsl", &shaders);
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  TessEffectPtr ptr(new TessEffect);
  ptr->Init(desc, shaders);
  return ptr;
}

class MyRenderWindow : public lord::FrameWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::FrameWindow(rect) {}
  void OnInit() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
  SceneNodePtr InitScene() { 
    return SceneNodePtr(new SceneNode);
  }

 private:
  EntityPtr entity_;
  TessEffectPtr effect_;
  NormalRenderEffectPtr normal_effect_;
  RasterizerStatePtr state_;
  TexturePtr heightmap_;
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
  LordEnv* env = LordEnv::instance();
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  Vector3 camera_pos(0.0f, 50.0f, 3.0f);
  Vector3 lookat(0.0f, 30.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateTessEffect();
  normal_effect_ = CreateNormalRenderEffect();
  entity_ = CreateQuadTile(effect_->vertex_desc(), 8, 2.0f, Matrix4::kIdentity);
  entity_->set_primitive(kControlPoint4);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
  state_->SetCullingMode(kCullNone);
  set_draw_gridline(false);

  heightmap_ = CreateHeightmapTextureFromFile("demo/data/terrain.raw", 100.0f);
  effect_->SetHeightmap(heightmap_);
  normal_effect_->SetHeightmap(heightmap_);

  spotlight_.diffuse = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  spotlight_.ambient = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  spotlight_.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  spotlight_.position = Vector4(0.0, 50.0f, 0.0f, 1.0f);
  spotlight_.directional = Vector4(1.0f, -1.0f, 0.0f, 0.0f);
  spotlight_.phi = cos(Degree(60.0f));
  spotlight_.theta = cos(Degree(45.0f));
  spotlight_.range = 300.0f;
  spotlight_.falloff = 0.5f;
  spotlight_.enable = 1.0f;
  dirlight_.ambient = Vector4(0.1f, 0.1f, 0.1f, 0.1f);
  dirlight_.diffuse = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  dirlight_.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  dirlight_.directional = Vector4(1.0f, -1.0f, -1.0f, 0.0f);
  dirlight_.enable = 1.0f;
  SetClearColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetEyePos(Vector4(camera().position(), 1.0f));
  renderer->UseEffect(effect_);
  entity_->DrawIndex(renderer);

  normal_effect_->SetPV(camera().GetProjViewMatrix());
  renderer->UseEffect(normal_effect_);
  renderer->SetRasterizerState(state_);
  entity_->DrawIndex(renderer);
}
