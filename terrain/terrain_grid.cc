#include <memory>

#include "azer/render/geometry.h"
#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/material.h"
#include "demo/base/textured_effect.h"
#include "demo/terrain/tile.h"

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
  TessEffect() {
    world_ = Matrix4::kIdentity;
    color_ = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
  }
  ~TessEffect() {}

  const char* GetEffectName() const override { return kEffectName;}
  bool Init(azer::VertexDesc* desc, const ShaderPrograms& sources) override {
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
  DISALLOW_COPY_AND_ASSIGN(TessEffect);
};
const char TessEffect::kEffectName[] = "TessEffect";

typedef scoped_refptr<TessEffect> TessEffectPtr;
TessEffectPtr CreateTessEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec4},
  };
  Effect::ShaderPrograms shaders;
  shaders.resize(kRenderPipelineStageNum);
  shaders[kVertexStage].path = "effect.vs";
  shaders[kVertexStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "struct VsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "}\n;"
      "struct VSInput {\n"
      "  float4 position:POSITION;\n"
      "};\n"
      "VsOutput vs_main(VSInput input) {\n"
      "  VsOutput o;"
      "  o.position = input.position;"
      "  return o;"
      "}";
  shaders[kHullStage].path = "effect.hs";
  shaders[kHullStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "cbuffer c_buffer {"
      "  float4 eyepos;"
      "};\n"
      "struct VsOutput {"
      "  float4 position: SV_POSITION;"
      "};\n"
      "struct HSCOutput {"
      "  float edge[4]: SV_TessFactor;"
      "  float inside[2]:  SV_InsideTessFactor;"
      "};\n"
      "struct HsOutput {"
      "  float4 position: POSITION;"
      "};\n"
      "float gMinDist = 0.0f;\n"
      "float gMaxDist = 300.0f;\n"
      "float gMinTess = 0.0f;\n"
      "float gMaxTess = 3.0f;\n"
      "float CalcTesFactor(float4 p) {"
      "  float d = distance(p.xyz, eyepos.xyz);"
      "  float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));"
      "  return pow(2, lerp(gMaxTess, gMinTess, s));"
      "}\n"
      "HSCOutput PatchConstantFunc(InputPatch<VsOutput, 4> patch, "
      "  uint patchid : SV_PrimitiveID) {\n"
      "  HSCOutput output;"
      "  float4 e0 = 0.5f * (patch[0].position + patch[1].position);"
      "  float4 e1 = 0.5f * (patch[1].position + patch[2].position);"
      "  float4 e2 = 0.5f * (patch[2].position + patch[3].position);"
      "  float4 e3 = 0.5f * (patch[3].position + patch[0].position);"
      "  float4 c = 0.25f * (patch[0].position + patch[1].position + "
      "                      patch[2].position + patch[3].position);"
      "  output.edge[0] = CalcTesFactor(e0);"
      "  output.edge[1] = CalcTesFactor(e1);"
      "  output.edge[2] = CalcTesFactor(e2);"
      "  output.edge[3] = CalcTesFactor(e3);"
      "  output.inside[0] = CalcTesFactor(c);"
      "  output.inside[1] = output.inside[0];"
      "  return output;\n"
      "}\n"
      "[domain(\"quad\")]\n"
      "[partitioning(\"integer\")]\n"
      "[outputtopology(\"triangle_cw\")]\n"
      "[outputcontrolpoints(4)]\n"
      "[patchconstantfunc(\"PatchConstantFunc\")]\n"
      "[maxtessfactor(64.0f)]\n"
      "HsOutput hs_main(InputPatch<VsOutput, 4> patch, "
      "  uint pointid: SV_OutputControlPointID, "
      "  uint patchid: SV_PrimitiveID) {\n"
      "  HsOutput output\n;"
      "  output.position = patch[pointid].position;\n"
      "  return output;\n"
      "}\n";
  shaders[kDomainStage].path = "effect.ds";
  shaders[kDomainStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "cbuffer c_buffer {"
      "  float4x4 pvw;"
      "  float4x4 world;"
      "};\n"
      "struct HSCOutput {"
      "  float edge[4]: SV_TessFactor;"
      "  float inside[2]:  SV_InsideTessFactor;"
      "};\n"
      "struct HsOutput {"
      "  float4 position: POSITION;"
      "};\n"
      "struct DsOutput {"
      "  float4 position: SV_POSITION;"
      "};\n"
      "[domain(\"quad\")]\n"
      "DsOutput ds_main(HSCOutput input, "
      "                 const OutputPatch<HsOutput, 4> quad, "
      "                 float2 uv : SV_DomainLocation) {\n"
      "  DsOutput output;\n"
      "  float3 v1 = lerp(quad[0].position.xyz, quad[1].position.xyz, uv.x);\n"
      "  float3 v2 = lerp(quad[3].position.xyz, quad[2].position.xyz, uv.x);\n"
      "  output.position = mul(pvw, float4(lerp(v1, v2, uv.y), 1.0f));\n"
      "  return output;\n"
      "}\n";
  shaders[kPixelStage].path = "effect.ps";
  shaders[kPixelStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "struct DsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "};\n"
      "cbuffer c_buffer {\n"
      "  float4 color;\n"
      "};\n"
      "float4 ps_main(DsOutput o):SV_TARGET {\n"
      "  return color;"
      "}\n";
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

  Vector3 camera_pos(0.0f, 0.0f, 3.0f);
  Vector3 lookat(0.0f, 0.0f, 0.0f);
  Vector3 up(0.0f, 1.0f, 0.0f);
  mutable_camera()->reset(camera_pos, lookat, up);

  effect_ = CreateTessEffect();
  entity_ = CreateQuadTile(effect_->vertex_desc(), 4, 32.0f, Matrix4::kIdentity);
  entity_->set_topology(kControlPoint4);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
  state_->SetCullingMode(kCullNone);
  set_draw_gridline(false);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  Vector3 position[] = {
    Vector3(-1.0f, -1.0f, 0.0f),
  };

  Vector4 edge[] = {
    Vector4(4.0f, 4.0f, 4.0f, 1.0f),
  };

  Vector4 inside[] = {
    Vector4(4.0f, 4.0f, 4.0f, 1.0f),
  };

  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
  for (uint32 i = 0; i < arraysize(position); ++i) {
    Matrix4 world = Translate(position[i]);
    effect_->SetWorld(world);
    effect_->SetEyePos(Vector4(camera().position(), 1.0f));
    renderer->UseEffect(effect_);
    renderer->SetRasterizerState(state_);
    entity_->DrawIndex(renderer);
  }
}
