#include <memory>

#include "azer/render/geometry.h"
#include "lordaeron/sandbox/sandbox.h"
#include "lordaeron/resource/variant_resource.h"
#include "demo/base/material.h"
#include "demo/base/textured_effect.h"

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
  bool Init(azer::VertexDesc* desc, const azer::Shaders& sources) override {
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
  struct vs_cbuffer {
    azer::Matrix4 pvw;
    azer::Matrix4 world;
  };

  struct hs_cbuffer {
    Vector4 edge;
  };

  struct ps_cbuffer {
    azer::Vector4 color;
  };
#pragma pack(pop)

  void SetPV(const azer::Matrix4& value) { pv_ = value;}
  void SetWorld(const azer::Matrix4& value) { world_ = value;}
  void SetColor(const azer::Vector4& value) {color_ = value;}
  void SetEdge(const azer::Vector4& value) {edge_ = value;}
 protected:
  void ApplyGpuConstantTable(azer::Renderer* renderer) override {
    Matrix4 pvw = std::move(pv_ * world_);
    {
      GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &pvw, sizeof(Matrix4));
      tb->SetValue(1, &world_, sizeof(Matrix4));
    }
    {
      GpuConstantsTable* tb = gpu_table_[(int)kHullStage].get();
      DCHECK(tb != NULL);
      tb->SetValue(0, &edge_, sizeof(Vector4));
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
    GpuConstantsTable::Desc vs_table_desc[] = {
      GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                              offsetof(vs_cbuffer, pvw), 1),
      GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                              offsetof(vs_cbuffer, world), 1),
    };
    gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
        arraysize(vs_table_desc), vs_table_desc);

    // generate GpuTable init for stage kPixelStage
    GpuConstantsTable::Desc hs_table_desc[] = {
      GpuConstantsTable::Desc("edge", GpuConstantsType::kVector4,
                              offsetof(hs_cbuffer, edge), 1),
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
  Vector4 edge_;
  DISALLOW_COPY_AND_ASSIGN(TessEffect);
};
const char TessEffect::kEffectName[] = "TessEffect";

typedef scoped_refptr<TessEffect> TessEffectPtr;
TessEffectPtr CreateTessEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec4},
  };
  Shaders shaders;
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
      "cbuffer c_buffer {\n"
      "  float4x4 pvw;"
      "  float4x4 world;"
      "};"
      "VsOutput vs_main(VSInput input) {\n"
      "  VsOutput o;"
      "  o.position = mul(pvw, input.position);"
      "  return o;"
      "}";
  shaders[kHullStage].path = "effect.hs";
  shaders[kHullStage].code = ""
      "cbuffer c_buffer { float4 edge;};\n"
      "struct VsOutput {"
      "  float4 position: SV_POSITION;"
      "};\n"
      "struct HSCOutput {"
      "  float edge[3]: SV_TessFactor;"
      "  float inside:  SV_InsideTessFactor;"
      "};\n"
      "struct HsOutput {"
      "  float4 position: POSITION;"
      "};\n"
      "HSCOutput PatchConstantFunc(InputPatch<VsOutput, 3> input, "
      "  uint patchid : SV_PrimitiveID) {"
      "  HSCOutput output;"
      "  output.edge[0] = edge.x;"
      "  output.edge[1] = edge.y;"
      "  output.edge[2] = edge.z;"
      "  output.inside = edge.w;"
      "  return output;"
      "}"
      "[domain(\"tri\")]\n"
      "[partitioning(\"integer\")]\n"
      "[outputtopology(\"triangle_cw\")]\n"
      "[outputcontrolpoints(3)]\n"
      "[patchconstantfunc(\"PatchConstantFunc\")]\n"
      "HsOutput hs_main(InputPatch<VsOutput, 3> patch, "
      "  uint pointid: SV_OutputControlPointID, "
      "  uint patchid: SV_PrimitiveID) {"
      "  HsOutput output;"
      "  output.position = patch[pointid].position;"
      "  return output;"
      "}";
  shaders[kDomainStage].path = "effect.ds";
  shaders[kDomainStage].code = "#pragma pack_matrix(row_major)\n"
      "struct HSCOutput {"
      "  float edge[3]: SV_TessFactor;"
      "  float inside:  SV_InsideTessFactor;"
      "};\n"
      "struct HsOutput {"
      "  float4 position: POSITION;"
      "};\n"
      "struct DomainOutput {"
      "  float4 position: SV_POSITION;"
      "};\n"
      "[domain(\"tri\")]\n"
      "DomainOutput ds_main(HSCOutput input, float3 uvw : SV_DomainLocation,"
      "  const OutputPatch<HsOutput, 3> patch) {"
      "  DomainOutput output;"
      "  output.position = uvw.x * patch[0].position"
      "                  + uvw.y * patch[1].position"
      "                  + uvw.z * patch[2].position;"
      "  return output;"
      "}";
  shaders[kPixelStage].path = "effect.ps";
  shaders[kPixelStage].code = "#pragma pack_matrix(row_major)\n"
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

class MyRenderWindow : public lord::RenderWindow {
 public:
  MyRenderWindow(const gfx::Rect& rect) : lord::RenderWindow(rect) {}
  void OnInit() override;
  void OnUpdateFrame(const azer::FrameArgs& args) override;
  void OnRenderFrame(const azer::FrameArgs& args, Renderer* renderer) override;
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
  Vector3 points[] = {Vector3( 0.0f,  0.51f, 0.0f),
                      Vector3(-0.5f, -0.21f, 0.0f),
                      Vector3( 0.5f, -0.21f, 0.0f)};
  entity_ = CreateGeoPointsList(points, (int)arraysize(points), 
                                effect_->vertex_desc(), Matrix4::kIdentity);
  entity_->set_primitive(kControlPoint3);

  state_ = RenderSystem::Current()->CreateRasterizerState();
  state_->SetFillMode(kWireFrame);
}

void MyRenderWindow::OnUpdateFrame(const FrameArgs& args) {
}

void MyRenderWindow::OnRenderFrame(const FrameArgs& args, Renderer* renderer) {
  Vector3 position[] = {
    Vector3(-1.0f, -1.0f, 0.0f),
    Vector3(-1.0f,  1.0f, 0.0f),
    Vector3( 1.0f, -1.0f, 0.0f),
    Vector3( 1.0f,  1.0f, 0.0f),
  };

  Vector4 edge[] = {
    Vector4(3.0f, 3.0f, 3.0f, 1.0f),
    Vector4(3.0f, 3.0f, 3.0f, 3.0f),
    Vector4(3.0f, 3.0f, 3.0f, 4.0f),
    Vector4(3.0f, 5.0f, 6.0f, 6.0f),
  };

  effect_->SetPV(camera().GetProjViewMatrix());
  effect_->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
  for (uint32 i = 0; i < arraysize(position); ++i) {
    
    effect_->SetWorld(Translate(position[i]));
    effect_->SetEdge(edge[i]);
    renderer->UseEffect(effect_);
    renderer->SetRasterizerState(state_);
    entity_->Draw(renderer);
  }
}
