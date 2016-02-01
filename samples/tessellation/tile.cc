#include "demo/samples/tessellation/tile.h"

#include "azer/render/render.h"
#include "azer/render/index_pack.h"

using namespace azer;

EntityPtr CreateQuadTile(VertexDesc* vertex_desc, int32 level, float cell, 
                         const Matrix4& mat) {
  VertexPos npos;
  bool has_normal = GetSemanticIndex("normal", 0, vertex_desc, &npos);

  const int32 kGridLine = (1 << level) + 1;
  const int32 kVertexCount = kGridLine * kGridLine;
  SlotVertexDataPtr vdata(new SlotVertexData(vertex_desc, kVertexCount));
  VertexPack vpack(vdata.get());
  CHECK(vpack.first());
  float zbegin = -(kGridLine - 1) * cell * 0.5f;
  float xbegin = -(kGridLine - 1) * cell * 0.5f;
  for (int32 i = 0; i < kGridLine; ++i) {
    float z = zbegin + i * cell;
    for (int32 j = 0; j < kGridLine; ++j) {
      float x = xbegin + j * cell;
      Vector4 pos = mat * Vector4(x, 0.0f, z, 1.0f);
      vpack.WriteVector3Or4(pos, VertexPos(0, 0));
      if (has_normal) {
        vpack.WriteVector3Or4(Vector4(0.0f, 1.0f, 1.0f, 0.0f), npos);
      }
      vpack.next(1);
    }
  }
  
  const int32 kQuadCount = (kGridLine - 1) * (kGridLine - 1);
  const int32 kIndicesCount = kQuadCount * 4;
  IndicesDataPtr idata(new IndicesData(kIndicesCount));
  IndexPack ipack(idata.get());
  for (int32 i = 0; i < kGridLine - 1; ++i) {
    for (int32 j = 0; j < kGridLine - 1; ++j) {
      CHECK(ipack.WriteAndAdvance(i * kGridLine + j));
      CHECK(ipack.WriteAndAdvance((i + 1) * kGridLine +j));
      CHECK(ipack.WriteAndAdvance((i + 1) * kGridLine + j + 1));
      CHECK(ipack.WriteAndAdvance(i * kGridLine + j + 1));
    }
  }

  RenderSystem* rs = RenderSystem::Current();
  VertexBufferPtr vb = rs->CreateVertexBuffer(VertexBuffer::Options(), vdata);
  IndicesBufferPtr ib = rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata);
  EntityPtr entity(new Entity(vb, ib));
  entity->set_vmin(mat * Vector4( xbegin, 0.0f,   zbegin, 1.0f));
  entity->set_vmax(mat * Vector4(-xbegin, 0.01f, -zbegin, 1.0f));
  entity->set_primitive(kControlPoint4);
  return entity;
}
