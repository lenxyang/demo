#pragma once

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "azer/base/export.h"

namespace azer {
class Matrix4;
class MeshPart;
class IndicesData;
class SlotVertexData;
class VertexDesc;

typedef scoped_refptr<MeshPart> MeshPartPtr;

MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, float radius, 
                                 int32 stack, int32 slice);
MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const Matrix4& vertex_transform, 
                                 float radius, int32 stack, int32 slice);

MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, float radius, 
                                      int32 stack, int32 slice);
MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, 
                                      const Matrix4& vertex_transform, 
                                      float radius, int32 stack, int32 slice);

void CalcIndexedTriangleNormal(SlotVertexData* vbd, IndicesData* idata);
void CalcTriangleNormal(SlotVertexData* vbd, int* indices);
}  // namespace azer
