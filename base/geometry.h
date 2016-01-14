#pragma once

#include "base/memory/ref_counted.h"
#include "azer/base/export.h"
#include "azer/math/math.h"

namespace azer {
class Entity;
typedef scoped_refptr<Entity> EntityPtr;

EntityPtr CreateSphereEntity(VertexDesc* desc, float radius, 
                             int32 stack, int32 slice);
EntityPtr CreateSphereEntity(VertexDesc* desc, const Matrix4& vertex_transform, 
                             float radius, int32 stack, int32 slice);

EntityPtr CreateSphereFrameEntity(VertexDesc* desc, float radius, 
                             int32 stack, int32 slice);
EntityPtr CreateSphereFrameEntity(VertexDesc* desc, 
                                  const Matrix4& vertex_transform, 
                                  float radius, int32 stack, int32 slice);
}  // namespace azer
