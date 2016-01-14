#include "demo/base/geometry.h"

#include "azer/render/mesh.h"
#include "azer/render/indices_buffer.h"
#include "azer/render/vertex_buffer.h"

namespace azer {

// =====================================================================
// Code for Generate Sphere Objects
namespace {
inline int32 CalcSphereIndexNum(int32 stack_num, int32 slice_num) {
  return (stack_num - 2 - 1) * slice_num * 3 * 2 + slice_num * 2 * 3;
}

inline int32 CalcSphereVertexNum(int32 stack_num, int32 slice_num) {
  return (stack_num - 2) * slice_num + 2;
}

SlotVertexDataPtr InitSphereVertexData(VertexDesc* desc, const Matrixr4& matrix,
                                       float radius, int32 stack, int32 slice) {
  const int32 kVertexNum = CalcSphereVertexNum(stack, slice);
  SlotVertexDataPtr vdata(new SlotVertexData(desc, kVertexNum));
  VertexPack vpack(vdata.get());

  int num = 0;
  CHECK(vpack.first());
  vpack.WriteVector4(Vector4(0.0f, radius, 0.0f, 1.0f), VertexPos(0, 0));
  num++;

  for (int i = 1; i < stack - 1; ++i) {
    float y = radius * sin(Degree(90.0f - i * 180.0f / (float)stack));
    float slice_radius =  cos(Degree(90.0f - i * 180.0f / (float)stack));
    for (int j = 0; j < slice; ++j) {
      float degree = 360.0f - j * 360.0f / slice;
      float x = radius * slice_radius * cos(Degree(degree));
      float z = radius * slice_radius * sin(Degree(degree));

      CHECK(vpack.next(1));
      vpack.WriteVector4(Vector4(x, y, z, 1.0f), VertexPos(0, 0));
      num++;
    }
  }

  CHECK(vpack.next(1));
  vpack.WriteVector4(Vector4(0.0f, -radius, 0.0f, 1.0f), VertexPos(0, 0));
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


EntityPtr CreateSphere(VertexDesc* desc, float radius, int32 stack, int32 slice) {
  return CreateSphereEntity(desc, Matrix4::kIdentity, radius, stack, slice);
}

EntityPtr CreateSphereEntity(VertexDesc* desc, const Matrix4& vertex_transform, 
                             float radius, int32 stack, int32 slice) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, radius, stack, slice));
  IndicesDataPtr idata = InitSphereIndicesData(stack, slice);

  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBuffer vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBuffer ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = vertex_transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = vertex_transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
}

EntityPtr CreateSphereFrameEntity(VertexDesc* desc, float radius, 
                                  int32 stack, int32 slice)
EntityPtr CreateSphereFrameEntity(VertexDesc* desc, 
                                  const Matrix4& vertex_transform, 
                                  float radius, int32 stack, int32 slice) {
  SlotVertexDataPtr vdata(InitSphereVertexData(desc, radius, stack, slice));
  IndicesDataPtr edge_idata = InitSphereWireFrameIndicesData(stack, slice);
  VertexPos npos;
  if (GetSemanticIndex("normal", 0, desc, &npos)) {
    CalcNormal(vdata.get(), idata.get());
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBuffer vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBuffer ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), edge_data);
  EntityPtr entity(new Entity(desc, vb, ib));
  Vector4 vmin = vertex_transform * Vector4(-0.5f, -0.5f, -0.5f, 1.0f);
  Vector4 vmax = vertex_transform * Vector4( 0.5f,  0.5f,  0.5f, 1.0f);
  entity->set_vmin(Vector3(vmin.x, vmin.y, vmin.z));
  entity->set_vmax(Vector3(vmax.x, vmax.y, vmax.z));
}
}  // namespace azer
