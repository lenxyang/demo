#include "demo/terrain/util.h"

#include <cmath>
#include "base/files/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "azer/base/image.h"
#include "azer/render/render.h"

using namespace azer;
using base::UTF8ToUTF16;

bool LoadRawHeightmap(const std::string& path, int32* width, int32* height, 
                      std::vector<uint8>* data) {
  std::string contents;
  if (!::base::ReadFileToString(::base::FilePath(UTF8ToUTF16(path)), &contents)) {
    LOG(ERROR) << "Failed to read file: " << path;
    return false;
  }

  data->resize(contents.length());
  memcpy(&data->front(), contents.c_str(), contents.length());
  *width = std::sqrt(contents.length());
  *height = *width;
  return true;
}


TexturePtr CreateHeightmapTexture(const std::vector<uint8>& data, float scalar, 
                                  int32 width, int32 height) {
  RenderSystem* rs = RenderSystem::Current();
  ImageDataPtr imgdata(new ImageData(width, height, kRGBAf));
  const uint8* cur = &data.front();
  Vector4* dest = (Vector4*)imgdata->data();
  for (int32 i = 0; i < width; ++i) {
    for (int j = 0;j < height; ++j, ++cur, ++dest) {
      float h = (float)*cur * scalar / 255.0f;
      *dest = Vector4(h, h, h, 1.0f);
    }
  }

  ImagePtr img(new Image(imgdata, Image::k2D));
  Texture::Options opt;
  opt.size = gfx::Size(width, height);
  opt.target = kBindTargetShaderResource;
  return rs->CreateTexture(opt, img);
}

TexturePtr CreateHeightmapTextureFromFile(const std::string& path, float s) {
  int32 width, height;
  std::vector<uint8> data;
  if (!LoadRawHeightmap(path, &width, &height, &data)) {
    return TexturePtr();
  }

  return CreateHeightmapTexture(data, s, width, height);
}
