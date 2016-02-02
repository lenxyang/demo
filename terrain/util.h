#pragma once

#include <vector>
#include <string>
#include "base/basictypes.h"
#include "azer/render/render.h"

bool LoadRawHeightmap(const std::string& path, int32* width, int64* height, 
                      std::vector<uint8>* data);

azer::TexturePtr CreateHeightmapTexture(const std::vector<uint8>& data, 
                                        float scalar, int32 width, int32 height);

azer::TexturePtr CreateHeightmapTextureFromFile(const std::string& path, float s);
