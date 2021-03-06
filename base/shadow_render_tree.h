#pragma once

#include "azer/render/render.h"
#include "azer/render/effect_creator.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/resource/resource_util.h"
#include "lordaeron/scene/scene_node_traverse.h"
#include "lordaeron/scene/render_node.h"

class ShadowDepthRenderer;
class ShadowDepthRenderDelegate;

class ShadowDepthRenderDelegate : public lord::RenderNodeDelegate,
                                  public azer::EffectParamsProvider {
 public:
  static const char kEffectProviderName[];
  explicit ShadowDepthRenderDelegate(lord::RenderNode* node,
                                     ShadowDepthRenderer* tree_renderer);
  ~ShadowDepthRenderDelegate();
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;

  // override from azer::EffectParamsProvider
  const azer::Matrix4& GetWorld() const { return world_;}
  const azer::Matrix4& GetPV() const;
  const char* GetProviderName() const override { return kEffectProviderName;}
 private:
  void Init();
  ShadowDepthRenderer* tree_renderer_;
  azer::MeshPtr shadow_;
  azer::Matrix4 world_;
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthRenderDelegate);
};

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;
class ShadowEffectAdapter : public EffectParamsAdapter {
 public:
  ShadowEffectAdapter();
  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(ShadowEffectAdapter);
};

class ShadowDepthRenderer : public lord::LightObserver {
 public:
  ShadowDepthRenderer(lord::ResourceLoader* loader, lord::Light* light);
  ~ShadowDepthRenderer();
  const azer::Camera& camera() const { return camera_;}
  void Init(lord::SceneNode* root, const azer::Camera* camera);
  void Update(const azer::FrameArgs& args);
  void Render(azer::Renderer* renderer);
  azer::MeshPtr CreateShadowMesh(azer::MeshPtr mesh);
  lord::RenderNode* root() { return root_;}
 private:
  void OnUpdateNode(lord::RenderNode* node, const azer::FrameArgs& args);
  void OnRenderNode(lord::RenderNode* node, azer::Renderer* renderer);
  void SetLight(lord::LightPtr light);
  lord::RenderNodePtr root_;
  lord::LightPtr light_;
  azer::Camera camera_;
  azer::EffectPtr effect_;
  bool need_update_;
  DISALLOW_COPY_AND_ASSIGN(ShadowDepthRenderer);
};
