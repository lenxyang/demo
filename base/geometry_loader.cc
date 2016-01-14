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

  MeshPartPtr part;
  std::string geometry_type  = node->GetAttr("geotype");
  if (geometry_type == "sphere") {
    part = CreateSphere(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "box") {
    part = CreateBox(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "plane") {
    part = CreatePlane(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "round") {
    part = CreateRound(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "cone") {
    part = CreateCone(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "barrel") {
    part = CreateBarrel(node, vertex_desc.get(), ctx);
  } else if (geometry_type == "cylinder") {
    part = CreateCylinder(node, vertex_desc.get(), ctx);
  } else {
    CHECK(false);
  }
  CHECK(part.get());
  part->SetEffect(effect);
  VariantResource resource;
  resource.type = kResTypeMesh;
  resource.mesh = new Mesh;
  resource.mesh->AddMeshPart(part); 
  resource.retcode = 0;
  if (material.get())
    resource.mesh->AddProvider(material);
  return resource;
}

MeshPartPtr GeometryLoader::CreateSphere(const ConfigNode* node, VertexDesc* desc,
                                         ResourceLoadContext* ctx) {
  GeoSphereParams params;
  params.radius = 1.0f;
  params.stack = 24;
  params.slice = 24;
  if (node->HasAttr("radius")) {
    CHECK(node->GetAttrAsFloat("radius", &params.radius));
  }
  if (node->HasAttr("stack")) {
    CHECK(node->GetAttrAsInt("stack", &params.stack));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &params.slice));
  }
  MeshPartPtr ptr = CreateSphereMeshPart(desc, params);
  return ptr;
}

MeshPartPtr GeometryLoader::CreateBox(const ConfigNode* node,
                                      VertexDesc* desc, ResourceLoadContext* ctx) {
  MeshPartPtr ptr = CreateBoxMeshPart(desc);
  return ptr;
}

MeshPartPtr GeometryLoader::CreatePlane(const ConfigNode* node,
                                        VertexDesc* desc, ResourceLoadContext* ctx) {
  GeoPlaneParams params;
  params.row = 10;
  params.row_width = 1.0f;
  params.column = 10;
  params.column_width = 1.0f;
  if (node->HasAttr("row_width")) {
    CHECK(node->GetAttrAsFloat("row_width", &params.row_width));
  }
  if (node->HasAttr("row")) {
    CHECK(node->GetAttrAsInt("row", &params.row));
  }
  if (node->HasAttr("column_width")) {
    CHECK(node->GetAttrAsFloat("column_width", &params.column_width));
  }
  if (node->HasAttr("column")) {
    CHECK(node->GetAttrAsInt("column", &params.column));
  }
  return CreatePlaneMeshPart(desc, params);
}

MeshPartPtr GeometryLoader::CreateCone(const ConfigNode* node,
                                       VertexDesc* desc, ResourceLoadContext* ctx) {
  GeoConeParams params;
  params.slice = 24;
  params.height = 1.0f;
  params.radius = 1.0f;
  if (node->HasAttr("radius")) {
    CHECK(node->GetAttrAsFloat("radius", &params.radius));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &params.slice));
  }
  if (node->HasAttr("height")) {
    CHECK(node->GetAttrAsFloat("height", &params.height));
  }
  return CreateConeMeshPart(desc, params);
}

MeshPartPtr GeometryLoader::CreateRound(const ConfigNode* node,
                                        VertexDesc* desc, 
                                        ResourceLoadContext* ctx) {
  int32 slice = 24;;
  float radius = 1.0f;
  if (node->HasAttr("radius")) {
    CHECK(node->GetAttrAsFloat("radius", &radius));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &slice));
  }
  if (node->GetAttr("wireframe") == "true") {
    return CreateRoundMeshPart(desc, radius, slice);
  } else {
    return CreateRoundFrameMeshPart(desc, radius, slice);
  }
}

MeshPartPtr GeometryLoader::CreateCylinder(const ConfigNode* node,
                                           VertexDesc* desc, 
                                           ResourceLoadContext* ctx) {
  GeoBarrelParams params;
  params.slice = 24;
  params.stack = 24;
  params.height = 1.0f;
  params.top_radius = 1.0f;
  params.bottom_radius = 1.0f;
  if (node->HasAttr("bottom_radius")) {
    CHECK(node->GetAttrAsFloat("bottom_radius", &params.bottom_radius));
  }
  if (node->HasAttr("top_radius")) {
    CHECK(node->GetAttrAsFloat("top_radius", &params.top_radius));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &params.slice));
  }
  if (node->HasAttr("stack")) {
    CHECK(node->GetAttrAsInt("stack", &params.stack));
  }
  if (node->HasAttr("height")) {
    CHECK(node->GetAttrAsFloat("height", &params.height));
  }
  return CreateCylinderMeshPart(desc, params);
}

MeshPartPtr GeometryLoader::CreateBarrel(const ConfigNode* node,
                                         VertexDesc* desc, 
                                         ResourceLoadContext* ctx) {
  GeoBarrelParams params;
  params.slice = 24;
  params.stack = 24;
  params.height = 1.0f;
  params.top_radius = 1.0f;
  params.bottom_radius = 1.0f;
  if (node->HasAttr("bottom_radius")) {
    CHECK(node->GetAttrAsFloat("bottom_radius", &params.bottom_radius));
  }
  if (node->HasAttr("top_radius")) {
    CHECK(node->GetAttrAsFloat("top_radius", &params.top_radius));
  }
  if (node->HasAttr("slice")) {
    CHECK(node->GetAttrAsInt("slice", &params.slice));
  }
  if (node->HasAttr("stack")) {
    CHECK(node->GetAttrAsInt("stack", &params.stack));
  }
  if (node->HasAttr("height")) {
    CHECK(node->GetAttrAsFloat("height", &params.height));
  }
  return CreateBarrelMeshPart(desc, params);
}

MeshPartPtr GeometryLoader::CreateTour(const ConfigNode* node,
                                       VertexDesc* desc, 
                                       ResourceLoadContext* ctx) {
  return MeshPartPtr();
}

bool GeometryLoader::CouldLoad(ConfigNode* node) const {
  return node->tagname() == "geometry";
}


}  // namespace lord

