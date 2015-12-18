#include "demo/base/resource_util.h"

#include "base/logging.h"
#include "azer/base/image.h"
#include "azer/base/image_data.h"
#include "azer/render/render.h"

using namespace azer;
bool ReadFileContents(const ResPath& path, FileContents* contents, FileSystem* fs) {
  FilePtr ptr = fs->OpenFile(path);
  if (!ptr.get()) {
    LOG(ERROR) << "Failed to open file: " << path.fullpath();
    return false;
  }

  if (ptr->PRead(0, -1, contents)) {
    return true;
  } else {
    LOG(ERROR) << "Failed to read file";
    return false;
  }
}

TexturePtr Load2DTexture(const ResPath& path, FileSystem* fs) {
  FileContents contents;
  if (!ReadFileContents(path, &contents, fs)) {
    return TexturePtr();
  }

  const char* data = (char*)(&contents.front());
  ImageDataPtr imgdata(ImageData::Load2D(data, contents.size()));
  ImagePtr img(new Image(imgdata, Image::k2D));
  RenderSystem* rs = RenderSystem::Current();
  Texture::Options opt;
  opt.target = Texture::kShaderResource;
  opt.size = gfx::Size(img->width(), img->height());
  return rs->CreateTexture(opt, img.get());
}
