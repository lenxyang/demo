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

// sphere
struct GeoSphereParams {
  float radius;
  int32 stack;
  int32 slice;
};
MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const GeoSphereParams& params);
MeshPartPtr CreateSphereMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                 const GeoSphereParams& params);
MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, 
                                      const GeoSphereParams& params);
MeshPartPtr CreateSphereFrameMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                      const GeoSphereParams& params);

// box
MeshPartPtr CreateBoxMeshPart(VertexDesc* desc);
MeshPartPtr CreateBoxMeshPart(VertexDesc* desc, const Matrix4& transform);
MeshPartPtr CreateBoxFrameMeshPart(VertexDesc* desc);
MeshPartPtr CreateBoxFrameMeshPart(VertexDesc* desc, const Matrix4& transform);

// plane
struct GeoPlaneParams {
  int32 row;
  int32 column;
  float row_width;
  float column_width;
};
MeshPartPtr CreatePlaneMeshPart(VertexDesc* desc, const GeoPlaneParams& params);
MeshPartPtr CreatePlaneMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                const GeoPlaneParams& params);
MeshPartPtr CreatePlaneFrameMeshPart(VertexDesc* desc, const GeoPlaneParams& params);
MeshPartPtr CreatePlaneFrameMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                     const GeoPlaneParams& params);

// round
MeshPartPtr CreateRoundMeshPart(VertexDesc* desc, float radius, int32 slice);
MeshPartPtr CreateRoundMeshPart(VertexDesc* desc, const Matrix4& transform, 
                                float radius, int32 slice);
MeshPartPtr CreateRoundFrameMeshPart(VertexDesc* desc, float radius, int32 slice);
MeshPartPtr CreateRoundFrameMeshPart(VertexDesc* desc, const Matrix4& transform,
                                     float radius, int32 slice);

//
struct GeoConeParams {
  float height;
  float radius;
  int slice;
};
MeshPartPtr CreateConeMeshPart(VertexDesc* desc, const GeoConeParams& params);
MeshPartPtr CreateConeMeshPart(VertexDesc* desc, const Matrix4& transform,
                               const GeoConeParams& params);
MeshPartPtr CreateTaperMeshPart(VertexDesc* desc, const GeoConeParams& params);
MeshPartPtr CreateTaperMeshPart(VertexDesc* desc, const Matrix4& transform,
                                const GeoConeParams& params);

// util
void CalcIndexedTriangleNormal(SlotVertexData* vbd, IndicesData* idata);
void CalcTriangleNormal(SlotVertexData* vbd, int* indices);
}  // namespace azer
