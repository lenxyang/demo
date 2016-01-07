#pragma once

#include "azer/render/render.h"
#include "lordaeron/scene/render_node.h"
#include "lordaeron/scene/scene_node_traverse.h"
#include "lordaeron/scene/scene_renderer.h"
#include "demo/base/shadow_render_tree.h"

namespace lord {
class SceneNode;
class LightController;
}  // namespace lord


class EffectedSceneRenderer;
class ObjectNodeRenderDelegate : public lord::RenderNodeDelegate {
 public:
  ObjectNodeRenderDelegate(lord::RenderNode* node, 
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

class LampNodeRenderDelegate : public lord::RenderNodeDelegate {
 public:
  explicit LampNodeRenderDelegate(lord::RenderNode* node, 
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
 
class EffectedSceneRenderer : public lord::SceneRenderer {
 public:
  EffectedSceneRenderer();
 private:
  DISALLOW_COPY_AND_ASSIGN(EffectedSceneRenderer);
};

