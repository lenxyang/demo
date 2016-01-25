#include "demo/base/resource_util.h"

#include "base/logging.h"
#include "azer/base/image.h"
#include "azer/base/image_data.h"
#include "azer/render/render.h"

using namespace azer;
TexturePtr Load2DTexture(const ResPath& path, FileSystem* fs) {
  FileContents contents;
  if (!azer::LoadFileContents(path, &contents, fs)) {
    return TexturePtr();
  }

  const char* data = (char*)(&contents.front());
  ImageDataPtr imgdata(ImageData::Load2D(data, contents.size()));
  ImagePtr img(new Image(imgdata, Image::k2D));
  RenderSystem* rs = RenderSystem::Current();
  Texture::Options opt;
  opt.target = kBindTargetShaderResource;
  opt.size = gfx::Size(img->width(), img->height());
  TexturePtr tex = rs->CreateTexture(opt, img.get());
  return tex;
}
