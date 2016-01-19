#pragma once

#include "azer/render/render.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/render_env_node.h"
#include "lordaeron/scene/scene_node_traverse.h"
#include "lordaeron/scene/scene_render.h"
#include "demo/base/shadow_render_tree.h"

namespace lord {
class SceneNode;
class LightController;
}  // namespace lord

class ShadowDepthRenderer;
class EffectedSceneRender;
class ObjectNodeRenderDelegate : public lord::RenderNodeDelegate {
 public:
  ObjectNodeRenderDelegate(lord::RenderNode* node, EffectedSceneRender* renderer);
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;
 protected:
  bool Init();
  azer::MeshPtr mesh_;
  azer::MeshPtr bounding_mesh_;
  azer::MeshPtr normal_mesh_;
  EffectedSceneRender* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(ObjectNodeRenderDelegate);
};

class MirrorObjectNodeRenderDelegate : public ObjectNodeRenderDelegate {
 public:
  MirrorObjectNodeRenderDelegate(lord::RenderNode* node, 
                                 EffectedSceneRender* renderer);
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;
 private:
  DISALLOW_COPY_AND_ASSIGN(MirrorObjectNodeRenderDelegate);
};

class LampNodeRenderDelegate : public lord::RenderNodeDelegate {
 public:
  explicit LampNodeRenderDelegate(lord::RenderNode* node, 
                                  EffectedSceneRender* tree_render);
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;
 private:
  bool Init();
  azer::Camera camera_;
  scoped_ptr<ShadowDepthRenderer> scene_renderer_;
  EffectedSceneRender* tree_render_;
  DISALLOW_COPY_AND_ASSIGN(LampNodeRenderDelegate);
};

class EffectedEnvNodeDelegate : public lord::RenderEnvNodeDelegate,
                                public lord::SceneRenderObserver {
 public:
  struct LightData {
    lord::SceneNode* scene_node;
    lord::LightPtr light;
    azer::RendererPtr renderer;
    ShadowDepthRenderer* scene_renderer;
    azer::Camera camera;
  };

  EffectedEnvNodeDelegate(lord::RenderEnvNode* envnode, EffectedSceneRender* render);
  ~EffectedEnvNodeDelegate();
  
  int32 light_count() const;
  const lord::Light* light_at(int32 index) const;
  const LightData* light_data_at(int32 index) const;
  azer::Renderer* GetLightShadowmapRenderer(int32 index) const;
  azer::Texture* GetLightShadowmap(int32 index) const;
  const azer::Matrix4& GetLightShadowmapPV(int32 index) const;

  void Init(lord::SceneNode* render_node, lord::RenderNode* node) override;
  void OnUpdateNode(const azer::FrameArgs& args) override;
  void OnFrameRenderBegin(lord::SceneRender* sr, azer::Renderer* renderer) override;
 private:
  void RenderDepthMap(LightData* data, azer::Renderer* renderer);
  void InitLightData(LightData* data);
  void RebuildLightData(lord::SceneNode* node);

  EffectedSceneRender* scene_render_;
  lord::SceneNodes light_nodes_;
  std::vector<LightData> light_data_;
  std::vector<LightData> parent_light_data_;
  lord::SceneNode* scene_node_;
  azer::DepthStencilStatePtr depth_state_;
  const azer::FrameArgs* args_;
  azer::OverlayPtr overlay_;
  DISALLOW_COPY_AND_ASSIGN(EffectedEnvNodeDelegate);
};
 
class EffectedSceneRender : public lord::SceneRender {
 public:
  EffectedSceneRender();
 private:
  DISALLOW_COPY_AND_ASSIGN(EffectedSceneRender);
};

