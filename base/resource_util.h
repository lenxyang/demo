#pragma once

#include "azer/base/file_system.h"
#include "azer/render/render.h"

azer::TexturePtr Load2DTexture(const azer::ResPath& path, azer::FileSystem* fs);
