#pragma once

#include "azer/render/render.h"

azer::EntityPtr CreateQuadTile(azer::VertexDesc* desc, int32 level, float cell, 
                               const azer::Matrix4& mat);
