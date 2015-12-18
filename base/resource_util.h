#pragma once

#include "azer/base/file_system.h"
#include "azer/render/render.h"

bool ReadFileContents(const azer::ResPath& path, azer::FileContents* contents, 
                      azer::FileSystem* fs);
azer::TexturePtr Load2DTexture(const azer::ResPath& path, azer::FileSystem* fs);
