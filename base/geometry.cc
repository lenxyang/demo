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

void GenerateConeHat(bool up, float top, float bottom, float radius, int32 slice, 
                     VertexPack* vpack, IndexPack* ipack) {
  const int begin = vpack->index();
  float slice_degree = 360.0f / slice;
  vpack->WriteVector4(Vector4(0.0f, top, 0.0f, 1.0f), VertexPos(0, 0));
  vpack->next(1);
  for (int32 i = 0; i < slice; ++i) {
    float degree = 360.0f - slice_degree *  i;
    float x = azer::cos(Degree(degree)) * radius;
    float z = azer::sin(Degree(degree)) * radius;

    vpack->WriteVector4(Vector4(x, bottom, z, 1.0f), VertexPos(0, 0));
    vpack->next(1);
  }
  
  const int end = vpack->index();
  for (int i = 0; i < slice; ++i) {
    int index1 = i + begin + 1;
    int index2 = (i + 1) % slice + begin + 1;
    CHECK(ipack->WriteAndAdvance(begin));
    if (up) {
      CHECK(ipack->WriteAndAdvance(index1));
      CHECK(ipack->WriteAndAdvance(index2));
    } else {
      CHECK(ipack->WriteAndAdvance(index2));
      CHECK(ipack->WriteAndAdvance(index1));
    }
  }
}

void GenerateBarrel(float top_radius, float bottom_radius, float height, 
                    int32 stack, int32 slice, VertexPack* vpack, IndexPack* ipack) {
  VertexPos tpos;
  GetSemanticIndex("texcoord", 0, vpack->desc(), &tpos);
  float height_unit = height / ((float)stack - 1.0f);
  float radius_unit = (bottom_radius - top_radius) / (float)stack;
  float slice_radius = top_radius;
  float y = height;
  float tex_u_unit = 1.0f / slice;
  float tex_v_unit = 1.0f / (stack + 2.0f);
  const int32 begin = vpack->index();
  for (int i = 0; i < stack; ++i) {
    slice_radius += radius_unit;
    for (int j = 0; j < slice; ++j) {
      float degree = 360.0f - j * 360.0f / slice;
      float x = slice_radius * cos(Degree(degree));
      float z = slice_radius * sin(Degree(degree));

      vpack->WriteVector4(Vector4(x, y, z, 1.0f), VertexPos(0, 0));
      float u = j * tex_u_unit;
      float v = (i + 1) * tex_v_unit;
      vpack->WriteVector2(Vector2(0.0f, 0.0f), tpos); 
      vpack->next(1);
    }
    y -= height_unit;
  }
  
  for (int i = 0; i < stack - 1; ++i) {
    int32 line1 = begin + i * slice; 
    int32 line2 = begin + (i + 1) * slice; 
    for (int j = 0; j < slice; ++j) {
      int index1 = j % slice;
      int index2 = (j + 1) % slice;
      CHECK(ipack->WriteAndAdvance(line1 + index2));
      CHECK(ipack->WriteAndAdvance(line1 + index1));
      CHECK(ipack->WriteAndAdvance(line2 + index1));
      
      CHECK(ipack->WriteAndAdvance(line1 + index2));
      CHECK(ipack->WriteAndAdvance(line2 + index1));
      CHECK(ipack->WriteAndAdvance(line2 + index2));
    }
  }
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
MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, float radius, int32 stack,
                                 int32 slice) {
  return CreateSphereMeshPart(desc, Matrix4::kIdentity, radius, stack, slice);
}

MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const Matrix4& vertex_transform, 
                             float radius, int32 stack, int32 slice) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, vertex_transform, radius,
                                               stack, slice));
  IndicesDataPtr idata = InitSphereIndicesData(stack, slice);

  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcIndexedTriangleNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = vertex_transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = vertex_transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;
}

MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, float radius, 
                                  int32 stack, int32 slice) {
  return CreateSphereFrameMeshPart(desc, Matrix4::kIdentity, radius, stack, slice);
}

MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, 
                                  const Matrix4& vertex_transform, 
                                  float radius, int32 stack, int32 slice) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, vertex_transform, radius,
                                               stack, slice));
  IndicesDataPtr idata = InitSphereIndicesData(stack, slice);
  IndicesDataPtr edge_idata = InitSphereWireFrameIndicesData(stack, slice);
  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcIndexedTriangleNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), edge_idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = vertex_transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = vertex_transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
  MeshPartPtr part(new MeshPart(NULL));
  part->AddEntity(entity);
  return part;
}
}  // namespace azer
