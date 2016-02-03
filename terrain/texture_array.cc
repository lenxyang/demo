#include <memory>

#include "azer/render/render.h"
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

class TessEffect : public azer::Effect {
 public:
  static const char kEffectName[];
  TessEffect() : ambient_scalar_(0.01f),
                 specular_scalar_(1.0f),
                 alpha_(1.0f) {
    world_ = Matrix4::kIdentity;
  }
  ~TessEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const Shaders& sources) override {
    DCHECK(sources.size() == kRenderPipelineStageNum);
    vertex_desc_ = desc;
    InitShaders(sources);
    InitGpuConstantTable();
    return true;
  }

#pragma pack(push, 4)
  struct ds_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
    azer::Vector4 eyepos;
  };

  struct hs_cbuffer {
    Vector4 eyepos;
  };

  struct ps_cbuffer {
    lord::DirLight   dirlight;
    float ambient_scalar;
    float specular_scalar;
    float alpha;
    float pad2;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetEyePos(const azer::Vector4& value) {eyepos_ = value;}
  void SetHeightmap(Texture* tex) { heightmap_ = tex;}
  void SetLayerTex(Texture* tex) { layertex_ = tex;}
  void SetDirLight(const lord::DirLight& value) { dirlight_ = value;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override {
    {
      GpuConstantsTable* tb = gpu_table_[(int)kDomainStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &pv_, sizeof(Matrix4));
      tb->SetValue(1, &world_, sizeof(Matrix4));
      tb->SetValue(2, &eyepos_, sizeof(Vector4));
    }
    {
      GpuConstantsTable* tb = gpu_table_[(int)kHullStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &eyepos_, sizeof(Vector4));
    }

    {
      GpuConstantsTable* tb = gpu_table_[(int)kPixelStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &dirlight_, sizeof(lord::DirLight));
      tb->SetValue(1, &ambient_scalar_, sizeof(float));
      tb->SetValue(2, &specular_scalar_, sizeof(float));
      tb->SetValue(3, &alpha_, sizeof(float));
      tb->SetValue(4, &specular_scalar_, sizeof(float));
    }
  }

  void UseTexture(azer::Renderer* renderer) override {
    renderer->UseTexture(kVertexStage, 0, heightmap_);
    renderer->UseTexture(kDomainStage, 0, heightmap_);
    renderer->UseTexture(kPixelStage, 0, layertex_);
  }
  void InitGpuConstantTable() {
    RenderSystem* rs = RenderSystem::Current();
    // generate GpuTable init for stage kVertexStage
    GpuConstantsTable::Desc ds_table_desc[] = {
      GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                              offsetof(ds_cbuffer, pvw), 1),
      GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                              offsetof(ds_cbuffer, world), 1),
      GpuConstantsTable::Desc("eyepos", GpuConstantsType::kVector4,
                              offsetof(ds_cbuffer, eyepos), 1),
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
      GpuConstantsTable::Desc("light", offsetof(ps_cbuffer, dirlight),
                              sizeof(lord::DirLight), 1),
      GpuConstantsTable::Desc("ambient_scalar", GpuConstantsType::kFloat,
                              offsetof(ps_cbuffer, ambient_scalar), 1),
      GpuConstantsTable::Desc("specular_scalar", GpuConstantsType::kFloat,
                              offsetof(ps_cbuffer, specular_scalar), 1),
      GpuConstantsTable::Desc("alpha", GpuConstantsType::kFloat,
                              offsetof(ps_cbuffer, alpha), 1),
      GpuConstantsTable::Desc("pad2", GpuConstantsType::kFloat,
                              offsetof(ps_cbuffer, pad2), 1),
    };
    gpu_table_[kPixelStage] = rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc);
  }

  azer::Matrix4 pv_;
  azer::Matrix4 world_;
  float ambient_scalar_;
  float specular_scalar_;
  float alpha_;
  lord::DirLight dirlight_;
  Vector4 eyepos_;
  TexturePtr heightmap_;
  TexturePtr layertex_;
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
  LoadStageShader(kVertexStage, "demo/terrain/texture/vs.hlsl", &shaders);
  LoadStageShader(kHullStage, "demo/terrain/texture/hs.hlsl", &shaders);
  LoadStageShader(kDomainStage, "demo/terrain/texture/ds.hlsl", &shaders);
  LoadStageShader(kPixelStage, "demo/terrain/texture/ps.2darray.hlsl", &shaders);
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
  RasterizerStatePtr state_;
  TexturePtr heightmap_;
  TexturePtr layertex_;
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
  scoped_ptr<azer::FileSystem> fs(new azer::NativeFileSystem(
      FilePath(UTF8ToUTF16("demo/"))));
  env->SetFileSystem(fs.Pass());

  ResourceLoader* resloader = env->resource_loader();
  InitDefaultLoader(resloader);

  Vector3 camera_pos(0.0f, 10.0f, 3.0f);
  Vector3 lookat(0.0f, 10.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateTessEffect();
  entity_ = CreateQuadTile(effect_->vertex_desc(), 5, 2.0f, Matrix4::kIdentity);
  entity_->set_primitive(kControlPoint4);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
  state_->SetCullingMode(kCullNone);
  set_draw_gridline(false);

  heightmap_ = CreateHeightmapTextureFromFile("demo/data/terrain.raw", 10.0f);
  effect_->SetHeightmap(heightmap_);


  std::vector< ::base::FilePath>  layer_path;
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/grass.dds")));
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/darkdirt.dds")));
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/stone.dds")));
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/lightdirt.dds")));
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/snow.dds")));
  layer_path.push_back(FilePath(UTF8ToUTF16("demo/terrain/tex/blend.dds")));
  ImagePtr img = Image::LoadFromFile(layer_path, kTex2DArray);
  Texture::Options texopt;
  texopt.target = kBindTargetShaderResource;
  texopt.size = gfx::Size(img->width(), img->height());
  layertex_ = rs->CreateTexture(texopt, img.get());

  dirlight_.ambient = Vector4(0.1f, 0.1f, 0.1f, 0.1f);
  dirlight_.diffuse = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  dirlight_.specular = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
  dirlight_.directional = Vector4(1.0f, -1.0f, 1.0f, 0.0f);
  dirlight_.enable = 1.0f;
  SetClearColor(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  effect_->SetDirLight(dirlight_);
  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetLayerTex(layertex_);
  effect_->SetEyePos(Vector4(camera().position(), 1.0f));
  renderer->UseEffect(effect_);
  // renderer->SetRasterizerState(state_);
  entity_->DrawIndex(renderer);
}
