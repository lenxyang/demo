#include "demo/base/geometry_loader.h"

#include "lordaeron/resource/mesh_loader.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "azer/render/render.h"
#include "lordaeron/resource/mesh_load_util.h"
#include "lordaeron/resource/resource_util.h"
#include "demo/base/geometry.h"

namespace lord {
using namespace azer;

const char GeometryLoader::kSpecialLoaderName[] = "lord::GeometryLoader";
GeometryLoader::GeometryLoader() {
}

GeometryLoader::~GeometryLoader() {
}

const char* GeometryLoader::GetLoaderName() const {
  return kSpecialLoaderName;
}
VariantResource GeometryLoader::Load(const ConfigNode* node,
                                     ResourceLoadContext* ctx) {
  ConfigNode* vertex_desc_node = GetTypedReferNode("vertex_desc", node);
  VertexDescPtr vertex_desc = LoadReferVertexDesc(vertex_desc_node, ctx);
  ConfigNode* effect_node = GetTypedReferNode("effect", node);
  EffectPtr effect = LoadReferEffect(effect_node, ctx);
  ConfigNode* material_node = GetTypedReferNode("material", node);
  MaterialPtr material;
  if (material_node) {
    material = LoadReferMaterial(material_node, ctx);
  }

  if (!vertex_desc.get() || !effect.get()) {
    return VariantResource();
  }

  EntityPtr entity;
  std::string geometry_type  = node->GetAttr("geometry_type");
  if (geometry_type == "sphere") {
    eneity = CreateSphere(node, vertex_desc.get(), ctx);
  }


  MeshPartPtr part(new MeshPart(effect));
  part->AddEntity(entity);
  VariantResource resource;
  resource.type = kResTypeMesh;
  resource.mesh = new Mesh;
  resource.mesh->AddMeshPart(part);
  if (material.get())
    mesh->AddProvider(material);
  return resource;
}

EntityPtr GeometryLoader::CreateSphere(const ConfigNode* node, VertexDesc* desc,
                                       ResourceLoadContext* ctx) {
  float radius = 1.0f;
  int32 stack = 24, slice = 24;
  if (node->HasAttr("radius")) {
    CHECK(node->GetAttrAsFloat("radius", &radius));
  }
  if (node->HasAttr("stack")) {
    CHECK(node->GetAttrAsInt("stack", &stack));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &slice));
  }
  EntityPtr entity = CreateSphereEntity(desc, radius, stack, slice);
  return entity;
}

bool GeometryLoader::CouldLoad(ConfigNode* node) const {
  return node->tagname() == "mesh";
}

}  // namespace lord

