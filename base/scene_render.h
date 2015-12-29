#pragma once

#include "azer/render/render.h"
#include "lordaeron/scene/scene_node_traverse.h"
#include "lordaeron/scene/scene_render_tree.h"
#include "demo/base/shadow_render_tree.h"

namespace lord {
class SceneNode;
class LightController;
}  // namespace lord


class EffectedSceneRenderer;
class ObjectNodeRenderDelegate : public lord::SceneRenderNodeDelegate {
 public:
  ObjectNodeRenderDelegate(lord::SceneRenderNode* node, 
                           EffectedSceneRenderer* renderer);
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;
 private:
  bool Init();
  azer::MeshPtr mesh_;
  azer::MeshPtr bounding_mesh_;
  azer::MeshPtr normal_mesh_;
  EffectedSceneRenderer* tree_renderer_;
  DISALLOW_COPY_AND_ASSIGN(ObjectNodeRenderDelegate);
};

class LampNodeRenderDelegate : public lord::SceneRenderNodeDelegate {
 public:
  explicit LampNodeRenderDelegate(lord::SceneRenderNode* node, 
                                  EffectedSceneRenderer* tree_render);
  void Update(const azer::FrameArgs& args) override;
  void Render(azer::Renderer* renderer) override;
 private:
  bool Init();
  azer::Camera camera_;
  scoped_ptr<ShadowDepthRenderer> scene_renderer_;
  EffectedSceneRenderer* tree_render_;
  DISALLOW_COPY_AND_ASSIGN(LampNodeRenderDelegate);
};
 
class EffectedSceneRenderer {
 public:
  EffectedSceneRenderer();
  void Init(lord::SceneNode* root, const azer::Camera* camera);
  void Update(const azer::FrameArgs& args);
  void Render(azer::Renderer* renderer);
  lord::SceneRenderNode* root() { return root_;}
  const azer::Camera* camera() { return camera_;}
 private:
  void UpdateNode(lord::SceneRenderNode* node, const azer::FrameArgs& args);
  void RenderNode(lord::SceneRenderNode* node, azer::Renderer* renderer);
  const azer::Camera* camera_;
  lord::SceneRenderNodePtr root_;
  DISALLOW_COPY_AND_ASSIGN(EffectedSceneRenderer);
};

