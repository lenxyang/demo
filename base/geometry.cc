#include "demo/base/geometry.h"

#include "azer/math/math.h"
#include "azer/render/mesh.h"
#include "azer/render/index_pack.h"
#include "azer/render/indices_buffer.h"
#include "azer/render/render_system.h"
#include "azer/render/vertex_pack.h"
#include "azer/render/vertex_buffer.h"

namespace azer {

// =====================================================================
// Code for Generate Sphere Objects
namespace {

#define FUNC_BOOL_RET(func)  {                  \
    bool ret = (func);                          \
    if (!ret) return false;                     \
  }

bool GenerateTopTriangleStrip(int32 top, int32 begin, int32 vertex_num,
                              bool closed, IndexPack* ipack) {
  int num = closed ? vertex_num : vertex_num - 1;
  for (int i = 0; i < num; ++i) {
    int index1 = i % vertex_num + begin;
    int index2 = (i + 1) % vertex_num + begin;
    FUNC_BOOL_RET(ipack->WriteAndAdvance(top));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(index1));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(index2));
  }
  return true;
}

bool GenerateBottomTriangleStrip(int32 bottom, int32 begin, int32 vertex_num,
                                 bool closed, IndexPack* ipack) {
  int num = closed ? vertex_num : vertex_num - 1;
  for (int i = 0; i < num; ++i) {
    int index1 = i % vertex_num + begin;
    int index2 = (i + 1) % vertex_num + begin;
    FUNC_BOOL_RET(ipack->WriteAndAdvance(bottom));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(index2));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(index1));
  }
  return true;
}

bool GenerateStripIndex(int32 line1, int32 line2, int32 vertex_num, bool closed,
                        IndexPack* ipack) {
  int num = closed ? vertex_num : vertex_num - 1;
  for (int i = 0; i < num; ++i) {
    int index1 = i % vertex_num;
    int index2 = (i + 1) % vertex_num;
    FUNC_BOOL_RET(ipack->WriteAndAdvance(line1 + index2));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(line1 + index1));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(line2 + index1));

    FUNC_BOOL_RET(ipack->WriteAndAdvance(line1 + index2));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(line2 + index1));
    FUNC_BOOL_RET(ipack->WriteAndAdvance(line2 + index2));
  }
  return true;
}

inline int32 CalcSphereIndexNum(int32 stack_num, int32 slice_num) {
  return (stack_num - 2 - 1) * slice_num * 3 * 2 + slice_num * 2 * 3;
}

inline int32 CalcSphereVertexNum(int32 stack_num, int32 slice_num) {
  return (stack_num - 2) * slice_num + 2;
}

SlotVertexDataPtr InitSphereVertexData(VertexDesc* desc, const Matrix4& matrix,
                                       float radius, int32 stack, int32 slice) {
  const int32 kVertexNum = CalcSphereVertexNum(stack, slice);
  SlotVertexDataPtr vdata(new SlotVertexData(desc, kVertexNum));
  VertexPack vpack(vdata.get());

  int num = 0;
  CHECK(vpack.first());
  vpack.WriteVector4(matrix * Vector4(0.0f, radius, 0.0f, 1.0f), VertexPos(0, 0));
  num++;

  for (int i = 1; i < stack - 1; ++i) {
    float y = radius * sin(Degree(90.0f - i * 180.0f / (float)stack));
    float slice_radius =  cos(Degree(90.0f - i * 180.0f / (float)stack));
    for (int j = 0; j < slice; ++j) {
      float degree = 360.0f - j * 360.0f / slice;
      float x = radius * slice_radius * cos(Degree(degree));
      float z = radius * slice_radius * sin(Degree(degree));

      CHECK(vpack.next(1));
      Vector4 pos = std::move(matrix * Vector4(x, y, z, 1.0f));
      vpack.WriteVector4(pos, VertexPos(0, 0));
      num++;
    }
  }

  CHECK(vpack.next(1));
  vpack.WriteVector4(matrix * Vector4(0.0f, -radius, 0.0f, 1.0f), VertexPos(0, 0));
  num++;
  DCHECK_EQ(num, kVertexNum);
  return vdata;
}

IndicesDataPtr InitSphereIndicesData(int32 stack, int32 slice) {
  const int kIndexNum = CalcSphereIndexNum(stack, slice);
  const int32 kVertexNum = CalcSphereVertexNum(stack, slice);
  int bottom_index = kVertexNum - 1;
  IndicesDataPtr idata(new IndicesData(kIndexNum));  
  IndexPack ipack(idata.get());
  CHECK(GenerateTopTriangleStrip(0, 1, slice, true, &ipack));
  for (int i = 1; i < stack - 2; ++i) {
    CHECK(GenerateStripIndex(1 + slice * (i - 1), 1 + slice * i, slice, true,
                             &ipack));
  }
  CHECK(GenerateBottomTriangleStrip(bottom_index, bottom_index - slice,
                                    slice, true, &ipack));
  CHECK(ipack.count() == kIndexNum);
  return idata;
}

IndicesDataPtr InitSphereWireFrameIndicesData(int32 stack, int32 slice) {
  const int kIndexNum = (stack - 1) * slice * 2 + (stack - 2) * (slice + 1) * 2;
  const int32 kVertexNum = CalcSphereVertexNum(stack, slice);
  IndicesDataPtr idata(new IndicesData(kIndexNum));  
  IndexPack ipack(idata.get());
  for (int i = 0; i < slice; ++i) {
    CHECK(ipack.WriteAndAdvance(0));
    CHECK(ipack.WriteAndAdvance(1 + i));
  }

  for (int i = 1; i < stack - 2; ++i) {
    for (int j = 0; j < slice; ++j) {
      CHECK(ipack.WriteAndAdvance(1 + (i - 1) * slice + j));
      CHECK(ipack.WriteAndAdvance(1 + i * slice + j));
    }
  }

  for (int i = 0; i < slice; ++i) {
    CHECK(ipack.WriteAndAdvance(kVertexNum - 1));
    CHECK(ipack.WriteAndAdvance(kVertexNum - 1 - (i + 1)));
  }

  for (int i = 1; i < stack - 1; ++i) {
    for (int j = 0; j < slice; ++j) {
      CHECK(ipack.WriteAndAdvance(1 + (i - 1) * slice + j));
      CHECK(ipack.WriteAndAdvance(1 + (i - 1) * slice + (j + 1) % slice));
    }
  }
  CHECK(ipack.count()  == kIndexNum);
  return idata;
}
}  // namespace

void CalcIndexedTriangleNormal(SlotVertexData* vbd, IndicesData* idata) {
  VertexPos npos;
  if (!GetSemanticIndex("normal", 0, vbd->vertex_desc(), &npos)) {
    return;
  }

  std::vector<float> used;
  std::vector<Vector4> normals;
  used.resize(vbd->vertex_count());
  normals.resize(vbd->vertex_count());
  
  VertexPack vpack(vbd);
  vpack.first();
  for (int i = 0; i < vbd->vertex_count(); ++i) {
    DCHECK(!vpack.end());
    vpack.WriteVector4(Vector4(0.0f, 0.0f, 0.0f, 0.0f), npos);
    vpack.next(1);
    used[i] = 0.0f;
    normals[i] = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  
  vpack.first();
  IndexPack ipack(idata);
  for (int i = 0; i < idata->num(); i+=3) {
    uint32 idx1 = ipack.ReadAndAdvanceOrDie();
    uint32 idx2 = ipack.ReadAndAdvanceOrDie();
    uint32 idx3 = ipack.ReadAndAdvanceOrDie();
    Vector4 p1, p2, p3;
    CHECK(vpack.move(idx1));
    vpack.ReadVector4(&p1, VertexPos(0, 0));
    CHECK(vpack.move(idx2));
    vpack.ReadVector4(&p2, VertexPos(0, 0));
    CHECK(vpack.move(idx3));
    vpack.ReadVector4(&p3, VertexPos(0, 0));
    used[idx1] += 1.0f;
    used[idx2] += 1.0f;
    used[idx3] += 1.0f;

    Vector4 normal = Vector4(CalcPlaneNormal(p1, p2, p3), 0.0f);
    normals[idx1] += normal;
    normals[idx2] += normal;
    normals[idx3] += normal;
  }

  vpack.first();
  for (int i = 0; i < normals.size(); ++i) {
    Vector4 normal = normals[i] / used[i];
    normal.Normalize();
    vpack.WriteVector4(normal, npos);
    vpack.next(1);
  }
}

void CalcTriangleListNormal(SlotVertexData* vbd, int* indices) {
  VertexPos npos;
  if (!GetSemanticIndex("normal", 0, vbd->vertex_desc(), &npos)) {
    return;
  }

  std::vector<float> used;
  std::vector<Vector4> normals;
  used.resize(vbd->vertex_count());
  normals.resize(vbd->vertex_count());
  
  VertexPack vpack(vbd);
  vpack.first();
  for (int i = 0; i < vbd->vertex_count(); ++i) {
    vpack.WriteVector4(Vector4(0.0f, 0.0f, 0.0f, 0.0f), npos);
    vpack.next(1);
    used[i] = 0.0f;
    normals[i] = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  
  vpack.first();
  for (int i = 0; i < vbd->vertex_count(); i+=3) {
    int idx1 = *(indices + i);
    int idx2 = *(indices + i + 1);
    int idx3 = *(indices + i + 2);
    Vector4 p1, p2, p3;
    CHECK(vpack.move(idx1));
    vpack.ReadVector4(&p1, VertexPos(0, 0));
    CHECK(vpack.move(idx2));
    vpack.ReadVector4(&p2, VertexPos(0, 0));
    CHECK(vpack.move(idx3));
    vpack.ReadVector4(&p3, VertexPos(0, 0));
    used[idx1] += 1.0f;
    used[idx2] += 1.0f;
    used[idx3] += 1.0f;

    Vector4 normal = Vector4(CalcPlaneNormal(p1, p2, p3), 0.0f);
    normals[idx1] += normal;
    normals[idx2] += normal;
    normals[idx3] += normal;
  }

  for (int i = 0; i < normals.size(); ++i) {
    normals[i] /= used[i];
  }

  vpack.first();
  for (int i = 0; i < normals.size(); ++i) {
    Vector4 normal = normals[i] / used[i];
    normal.Normalize();
    vpack.WriteVector4(normal, npos);
    vpack.next(1);
  }
}


// class Sphere objects
MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const GeoSphereParams& params) {
  return CreateSphereMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                 const GeoSphereParams& params) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, transform, params.radius,
                                               params.stack, params.slice));
  IndicesDataPtr idata = InitSphereIndicesData(params.stack, params.slice);

  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcIndexedTriangleNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;
}

MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, 
                                      const GeoSphereParams& params) {
  return CreateSphereFrameMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                      const GeoSphereParams& params) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, transform, params.radius,
                                               params.stack, params.slice));
  IndicesDataPtr idata = InitSphereIndicesData(params.stack, params.slice);
  IndicesDataPtr edge_idata = InitSphereWireFrameIndicesData(
      params.stack, params.slice);
  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcIndexedTriangleNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), edge_idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_topology(kLineList);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;
}

// Generate Box Geometry 
namespace {
SlotVertexDataPtr CreateBoxVertexData(VertexDesc* desc) {
  const Vector4 position[] = {
    Vector4(-0.5f,  0.5f,  0.5f, 1.0f),
    Vector4( 0.5f,  0.5f,  0.5f, 1.0f),
    Vector4( 0.5f, -0.5f,  0.5f, 1.0f),
    Vector4(-0.5f, -0.5f,  0.5f, 1.0f),
    Vector4(-0.5f,  0.5f, -0.5f, 1.0f),
    Vector4( 0.5f,  0.5f, -0.5f, 1.0f),
    Vector4( 0.5f, -0.5f, -0.5f, 1.0f),
    Vector4(-0.5f, -0.5f, -0.5f, 1.0f),
  };

  const Vector2 texcoord0[] = {
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(1.0f, 0.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),

    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(1.0f, 0.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),

    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(1.0f, 0.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),

    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(1.0f, 0.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),

    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(1.0f, 0.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),

    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 1.0f),
    Vector2(0.0f, 1.0f),
    Vector2(0.0f, 0.0f),
    Vector2(1.0f, 0.0f),
    Vector2(1.0f, 1.0f),
  };

  Vector4 normal[] = {
    Vector4(0.0f, 0.0f, 1.0f, 0.0f),
    Vector4(1.0f, 0.0f, 0.0f, 0.0f),
    Vector4(0.0f, 0.0f, -1.0f, 0.0f),
    Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
    Vector4(0.0f,  1.0f, 0.0f, 0.0f),
    Vector4(0.0f,  -1.0f, 0.0f, 0.0f),
  };

  int indices[] = {0, 2, 1, 0, 3, 2,  // front
                   1, 6, 5, 1, 2, 6,  // right
                   5, 7, 4, 5, 6, 7,  // back
                   4, 3, 0, 4, 7, 3,  // left
                   4, 1, 5, 4, 0, 1,  // top
                   3, 6, 2, 3, 7, 6}; // bottom
  VertexPos normal_pos, tex0_pos;
  bool kHasNormal0Idx = GetSemanticIndex("normal", 0, desc, &normal_pos);
  bool kHasTexcoord0Idx = GetSemanticIndex("texcoord", 0, desc, &tex0_pos);
  SlotVertexDataPtr vdata(new SlotVertexData(desc, arraysize(indices)));
  VertexPack vpack(vdata.get());
  vpack.first();
  for (int i = 0; i < static_cast<int>(arraysize(indices)); ++i) {
    int index = indices[i];
    DCHECK(!vpack.end());
    vpack.WriteVector4(position[index], VertexPos(0, 0));
    vpack.WriteVector2(texcoord0[index], tex0_pos);
    vpack.next(1);
  }
  DCHECK(vpack.end());

  if (kHasNormal0Idx) {
    vpack.first(); 
    for (int i = 0; i < static_cast<int>(arraysize(indices)); i += 6) {
      int index = i / arraysize(normal);
      for (int j = 0; j < 6; ++j) { 
        vpack.WriteVector4(normal[index], normal_pos);
        vpack.next(1);
      }
    }
  }

  return vdata;
}

IndicesDataPtr CreateBoxFrameIndicesData() {
  int32 edge_indices[] = {0, 2, 2, 1, 1, 4, 4, 0,
                          0, 14, 2, 8, 1, 7, 4, 13,
                          14, 8, 8, 7, 7, 13, 13, 14};
  IndicesDataPtr idata(new IndicesData(arraysize(edge_indices)));
  IndexPack ipack(idata.get());
  for (uint32 i = 0; i < arraysize(edge_indices); ++i) {
    CHECK(ipack.WriteAndAdvance(edge_indices[i]));
  }
  return idata;
}
}

MeshPartPtr CreateBoxMeshPart(VertexDesc* desc) {
  return CreateBoxMeshPart(desc, Matrix4::kIdentity);
}

MeshPartPtr CreateBoxMeshPart(VertexDesc* desc, const Matrix4& transform) {
  RenderSystem* rs = RenderSystem::Current();
  SlotVertexDataPtr vdata = CreateBoxVertexData(desc);;
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  EntityPtr entity(new Entity(desc, vb));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

MeshPartPtr CreateBoxFrameMeshPart(VertexDesc* desc) {
  return CreateBoxFrameMeshPart(desc, Matrix4::kIdentity);
}

MeshPartPtr CreateBoxFrameMeshPart(VertexDesc* desc, const Matrix4& transform) {
  RenderSystem* rs = RenderSystem::Current();
  SlotVertexDataPtr vdata = CreateBoxVertexData(desc);;
  IndicesDataPtr idata = CreateBoxFrameIndicesData();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kLineList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

// Plane 
namespace {
SlotVertexDataPtr CreatePlaneVertexData(VertexDesc* desc, const Matrix4& matrix,
                                        const GeoPlaneParams& params) {
  VertexPos npos, tpos;
  const bool kHasNormalIndex = GetSemanticIndex("normal", 0, desc, &npos);
  const bool kHasTexcoordIndex = GetSemanticIndex("texcoord", 0, desc, &tpos);

  float row_width = 2.0f / (params.row - 1);
  float column_width = 2.0f / (params.column - 1);
  SlotVertexDataPtr vdata(new SlotVertexData(desc, params.row * params.column));
  VertexPack vpack(vdata.get());
  vpack.first();
  for (int i = 0; i < params.row; ++i) {
    for (int j = 0; j < params.column; ++j) {
      float x = -1.0 + j * params.column_width;
      float z = -1.0 + i * params.row_width;
      vpack.WriteVector4(matrix * Vector4(x,    0.0f, z, 1.0f), VertexPos(0, 0));
      vpack.WriteVector4(matrix * Vector4(0.0f, 1.0f, 0.0f, 0.0f), npos);

      float tu = (x + 1.0) * 0.5;
      float tv = (z + 1.0) * 0.5;
      vpack.WriteVector2(Vector2(tu, tv), tpos);
      vpack.next(1);
    }
  }
  return vdata;
}

IndicesDataPtr CreatePlaneIndicesData(const GeoPlaneParams& params) {
  const int32 kIndexNum = (params.row - 1) * (params.column - 1) * 2 * 3;
  IndicesDataPtr idata(new IndicesData(kIndexNum));
  IndexPack ipack(idata.get());
  for (int i = 0; i < params.row - 1; ++i) {
    for (int j = 0; j < params.column - 1; ++j) {
      int cur_line = i * params.column;
      int next_line = (i + 1) * params.column;
      CHECK(ipack.WriteAndAdvance(cur_line  + j));
      CHECK(ipack.WriteAndAdvance(next_line + j));
      CHECK(ipack.WriteAndAdvance(next_line + j + 1));
      CHECK(ipack.WriteAndAdvance(cur_line  + j));
      CHECK(ipack.WriteAndAdvance(next_line + j + 1));
      CHECK(ipack.WriteAndAdvance(cur_line  + j + 1));
    }
  }
  return idata;
}

IndicesDataPtr CreatePlaneFrameIndicesData(const GeoPlaneParams& params) {
  int32 count = params.row * 2 + params.column * 2;
  IndicesDataPtr idata(new IndicesData(count));
  IndexPack ipack(idata.get());
  for (uint32 i = 0; i < params.row; ++i) {
    int32 index1 = i * params.column;
    int32 index2 = (i  + 1) * params.column - 1;
    CHECK(ipack.WriteAndAdvance(index1));
    CHECK(ipack.WriteAndAdvance(index2));
  }

  for (uint32 i = 0; i < params.column; ++i) {
    int32 index1 = i;
    int32 index2 = (params.row  - 1) * params.column + i;
    CHECK(ipack.WriteAndAdvance(index1));
    CHECK(ipack.WriteAndAdvance(index2));
  }
  return idata;
}
}
MeshPartPtr CreatePlaneMeshPart(VertexDesc* desc, const GeoPlaneParams& params) {
  return CreatePlaneMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreatePlaneMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                const GeoPlaneParams& params) {
  RenderSystem* rs = RenderSystem::Current();
  SlotVertexDataPtr vdata = CreatePlaneVertexData(desc, transform, params);;
  IndicesDataPtr idata = CreatePlaneIndicesData(params);
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

MeshPartPtr CreatePlaneFrameMeshPart(VertexDesc* desc, const GeoPlaneParams& params) {
  return CreatePlaneFrameMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreatePlaneFrameMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                     const GeoPlaneParams& params) {
  RenderSystem* rs = RenderSystem::Current();
  SlotVertexDataPtr vdata = CreatePlaneVertexData(desc, transform, params);;
  IndicesDataPtr idata = CreatePlaneFrameIndicesData(params);
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kLineList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

// class round
namespace {
SlotVertexDataPtr CreateRoundVertexData(VertexDesc* desc, const Matrix4& transform,
                                        float radius, float slice) {
  VertexPos npos;
  GetSemanticIndex("normal", 0, desc, &npos);
  const int32 kVertexNum = 1 + slice + 1;
  float degree = 360.0f / (float)slice;
  SlotVertexDataPtr vdata(new SlotVertexData(desc, kVertexNum));
  VertexPack vpack(vdata);
  vpack.first();
  vpack.WriteVector4(transform * Vector4(0, 0, 0, 1.0f), VertexPos(0, 0));
  vpack.WriteVector4(transform * Vector4(0.0f, 1.0f, 0.0f, 0.0f), npos);
  vpack.next(1);
  for (int i = 1; i < kVertexNum; ++i) {
    float x = cos(Degree(i * degree)) * radius;
    float z = sin(Degree(i * degree)) * radius;
    vpack.WriteVector4(transform * Vector4(x, 0, z, 1.0f), VertexPos(0, 0));
    vpack.WriteVector4(transform * Vector4(0.0f, 1.0f, 0.0f, 0.0f), npos);
    vpack.next(1);
  }
  CHECK(vpack.end());
  return vdata;
}

IndicesDataPtr CreateRoundInidcesData(int32 slice) {
  IndicesDataPtr idata(new IndicesData(slice * 3));  
  IndexPack ipack(idata.get());
  for (int i = 0; i < slice; ++i) {
    int index1 = 1 + (i + 1) % slice;
    int index2 = 1 + i;
    CHECK(ipack.WriteAndAdvance(0));
    CHECK(ipack.WriteAndAdvance(index1));
    CHECK(ipack.WriteAndAdvance(index2));
  }
  return idata;
}

IndicesDataPtr CreateRoundFrameInidcesData(int32 slice) {
  const int kIndexNum = slice * 2;
  IndicesDataPtr idata(new IndicesData(kIndexNum));  
  IndexPack ipack(idata.get());
  for (int i = 0; i < slice; ++i) {
    CHECK(ipack.WriteAndAdvance(i + 1));
    CHECK(ipack.WriteAndAdvance((i + 1) % slice + 1));
  }
  return idata;
}
} // namespace

MeshPartPtr CreateRoundMeshPart(VertexDesc* desc, float radius, int slice) {
  return CreateRoundMeshPart(desc, Matrix4::kIdentity, radius, slice);
}

MeshPartPtr CreateRoundMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                float radius, int slice) {
  SlotVertexDataPtr vdata = CreateRoundVertexData(desc, transform, radius, slice);
  IndicesDataPtr idata = CreateRoundInidcesData(slice);
  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-radius, -0.01f, -radius, 1.0f);
  Vector4 vmax = transform * Vector4( radius,  0.01f,  radius, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

MeshPartPtr CreateRoundFrameMeshPart(VertexDesc* desc, float radius, int slice) {
  return CreateRoundFrameMeshPart(desc, Matrix4::kIdentity, radius, slice);
}

MeshPartPtr CreateRoundFrameMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                     float radius, int slice) {
  SlotVertexDataPtr vdata = CreateRoundVertexData(desc, transform, radius, slice);
  IndicesDataPtr idata = CreateRoundFrameInidcesData(slice);
  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-radius, -0.01f, -radius, 1.0f);
  Vector4 vmax = transform * Vector4( radius,  0.01f,  radius, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kLineList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

// cone
MeshPartPtr CreateTaperMeshPart(VertexDesc* desc, const GeoConeParams& params) {
  return CreateTaperMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateTaperMeshPart(VertexDesc* desc, const Matrix4& transform,
                                const GeoConeParams& params) {
  const int32 kVertexNum = 1 + params.slice + 1;
  float degree = 360.0f / (float)params.slice;
  SlotVertexDataPtr vdata(new SlotVertexData(desc, kVertexNum));
  VertexPack vpack(vdata);
  vpack.first();
  vpack.WriteVector4(transform * Vector4(0, params.height, 0, 1.0f), 
                     VertexPos(0, 0));
  vpack.next(1);
  for (int i = 1; i < kVertexNum; ++i) {
    float x = cos(Degree(i * degree)) * params.radius;
    float z = sin(Degree(i * degree)) * params.radius;
    vpack.WriteVector4(transform * Vector4(x, 0, z, 1.0f), VertexPos(0, 0));
    vpack.next(1);
  }
  CHECK(vpack.end());

  IndicesDataPtr idata = CreateRoundInidcesData(params.slice);
  CalcIndexedTriangleNormal(vdata, idata);
  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = transform * Vector4(-params.radius, -0.01f, -params.radius, 1.0f);
  Vector4 vmax = transform * Vector4( params.radius,  0.01f,  params.radius, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

MeshPartPtr CreateConeMeshPart(VertexDesc* desc, const GeoConeParams& params) {
  return CreateConeMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateConeMeshPart(VertexDesc* desc, const Matrix4& transform,
                               const GeoConeParams& params) {
  MeshPartPtr part = CreateTaperMeshPart(desc, transform, params);

  Matrix4 round_transform = std::move(transform * RotateX(Degree(180.0)));
  MeshPartPtr part2 = CreateRoundMeshPart(desc, round_transform, 
                                          params.radius, params.slice);
  CHECK(part2->entity_count() == 1);
  part->AddEntity(part2->entity_at(0));
  return part;
}

namespace {
int32 CalcCylinderIndexNum(int32 stack_num, int32 slice_num) {
  return (stack_num - 1) * slice_num * 3 * 2;
}

int32 CalcCylinderVertexNum(int32 stack_num, int32 slice_num) {
  return stack_num * slice_num;
}
}

MeshPartPtr CreateBarrelMeshPart(VertexDesc* desc, const GeoBarrelParams& params) {
  return CreateBarrelMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateBarrelMeshPart(VertexDesc* desc, const Matrix4& matrix, 
                                 const GeoBarrelParams& params) {
  const int kVertexNum = CalcCylinderVertexNum(params.stack, params.slice);
  const int kIndexNum = CalcCylinderIndexNum(params.stack, params.slice);
  SlotVertexDataPtr vdata(new SlotVertexData(desc, kVertexNum));
  IndicesDataPtr idata(new IndicesData(kIndexNum));  
  VertexPack vpack(vdata.get());
  VertexPos tpos;
  GetSemanticIndex("texcoord", 0, desc, &tpos);
  float height_unit = params.height / ((float)params.stack - 1.0f);
  float radius_unit = (params.bottom_radius - params.top_radius)
      / (float)params.stack;
  float slice_radius = params.top_radius;
  float y = params.height;
  float tex_u_unit = 1.0f / params.slice;
  float tex_v_unit = 1.0f / (params.stack + 2.0f);
  vpack.first();
  for (int i = 0; i < params.stack; ++i) {
    for (int j = 0; j < params.slice; ++j) {
      float degree = 360.0f - j * 360.0f / params.slice;
      float x = slice_radius * cos(Degree(degree));
      float z = slice_radius * sin(Degree(degree));

      vpack.WriteVector4(matrix * Vector4(x, y, z, 1.0f), VertexPos(0, 0));
      float u = j * tex_u_unit;
      float v = (i + 1) * tex_v_unit;
      vpack.WriteVector2(Vector2(0.0f, 0.0f), tpos); 
      vpack.next(1);
    }
    slice_radius += radius_unit;
    y -= height_unit;
  }

  IndexPack ipack(idata.get());
  for (int i = 0; i < params.stack - 1; ++i) {
    int32 line1 = i * params.slice; 
    int32 line2 = (i + 1) * params.slice; 
    for (int j = 0; j < params.slice; ++j) {
      int index1 = j % params.slice;
      int index2 = (j + 1) % params.slice;
      CHECK(ipack.WriteAndAdvance(line1 + index2));
      CHECK(ipack.WriteAndAdvance(line1 + index1));
      CHECK(ipack.WriteAndAdvance(line2 + index1));
      
      CHECK(ipack.WriteAndAdvance(line1 + index2));
      CHECK(ipack.WriteAndAdvance(line2 + index1));
      CHECK(ipack.WriteAndAdvance(line2 + index2));
    }
  }

  CalcIndexedTriangleNormal(vdata, idata);

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  float rad = std::max(params.top_radius, params.bottom_radius);
  Vector4 vmin = matrix * Vector4(-rad,  0.5f,          -rad, 1.0f);
  Vector4 vmax = matrix * Vector4( rad,  params.height,  rad, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  entity->set_topology(kTriangleList);
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;  
}

MeshPartPtr CreateCylinderMeshPart(VertexDesc* desc, const GeoBarrelParams& params) {
  return CreateCylinderMeshPart(desc, Matrix4::kIdentity, params);
}

MeshPartPtr CreateCylinderMeshPart(VertexDesc* desc, const Matrix4& matrix,
                                   const GeoBarrelParams& params) {
  MeshPartPtr part = CreateBarrelMeshPart(desc, matrix, params);
  {
    Matrix4 round_matrix = std::move(matrix * RotateX(Degree(180.0)));
    MeshPartPtr bot = CreateRoundMeshPart(desc, round_matrix, 
                                          params.bottom_radius, params.slice);
    CHECK(bot->entity_count() == 1);
    part->AddEntity(bot->entity_at(0));
  }

  {
    Matrix4 round_matrix = matrix * Translate(Vector3(0.0f, params.height, 0.0f));
    MeshPartPtr top = CreateRoundMeshPart(desc, round_matrix, 
                                          params.top_radius, params.slice);
    CHECK(top->entity_count() == 1);
    part->AddEntity(top->entity_at(0));
  }
  return part;
}
}  // namespace azer
