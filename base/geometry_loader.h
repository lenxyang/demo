#pragma once

#include "azer/render/render.h"
#include "lordaeron/effect/light.h"
#include "lordaeron/resource/scene_loader.h"

namespace lord {
class GeometryLoader : public ResourceSpecialLoader {
 public:
  static const char kSpecialLoaderName[];
  GeometryLoader();
  ~GeometryLoader();

  const char* GetLoaderName() const override;
  VariantResource Load(const azer::ConfigNode* node,
                       ResourceLoadContext* ctx) override;
  bool CouldLoad(azer::ConfigNode* node) const override;
 private:
  azer::MeshPartPtr CreateSphere(const azer::ConfigNode* node, 
                                 azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateBox(const azer::ConfigNode* node,
                              azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreatePlane(const azer::ConfigNode* node,
                               azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateCone(const azer::ConfigNode* node,
                               azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateRound(const azer::ConfigNode* node,
                                azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateCylinder(const azer::ConfigNode* node,
                                   azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateBarrel(const azer::ConfigNode* node,
                                 azer::VertexDesc* desc, ResourceLoadContext* ctx);
  azer::MeshPartPtr CreateTour(const azer::ConfigNode* node,
                               azer::VertexDesc* desc, ResourceLoadContext* ctx);
  DISALLOW_COPY_AND_ASSIGN(GeometryLoader);
};
}  // namespace lord
