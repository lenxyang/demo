#pragma once

#include <memory>

#include "base/files/file_path.h"
#include "demo/base/common_provider.h"
#include "demo/base/basic_effect.h"
#include "demo/base/depth_effect.h"
#include "demo/base/effected_scene_render.h"
#include "demo/base/material.h"
#include "demo/base/resource_util.h"
#include "demo/base/sdkmesh.h"
#include "demo/base/sdkmesh_effect.h"
#include "demo/base/shadow_render_tree.h"
#include "demo/base/shadowmap_effect.h"
#include "demo/base/textured_effect.h"
#include "lordaeron/resource/geometry_loader.h"
#include "lordaeron/resource/variant_resource.h"
#include "lordaeron/sandbox/sandbox.h"

using base::FilePath;
using base::UTF8ToUTF16;
using base::UTF16ToUTF8;
