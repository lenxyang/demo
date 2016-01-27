#include "demo/base/sdkmesh.h"

#include "demo/base/sdkmesh_effect.h"
#include "demo/base/resource_util.h"

using namespace azer;

enum D3DDECLUSAGE
{
  D3DDECLUSAGE_POSITION = 0,
  D3DDECLUSAGE_BLENDWEIGHT =1,
  D3DDECLUSAGE_BLENDINDICES =2,
  D3DDECLUSAGE_NORMAL =3,
  D3DDECLUSAGE_TEXCOORD = 5,
  D3DDECLUSAGE_TANGENT = 6,
  D3DDECLUSAGE_BINORMAL = 7,
  D3DDECLUSAGE_COLOR = 10,
};

enum D3DDECLTYPE
{
  D3DDECLTYPE_FLOAT1    =  0,  // 1D float expanded to (value, 0., 0., 1.)
  D3DDECLTYPE_FLOAT2    =  1,  // 2D float expanded to (value, value, 0., 1.)
  D3DDECLTYPE_FLOAT3    =  2,  // 3D float expanded to (value, value, value, 1.)
  D3DDECLTYPE_FLOAT4    =  3,  // 4D float
  D3DDECLTYPE_D3DCOLOR  =  4,  // 4D packed unsigned bytes mapped to 0. to 1. range
  // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
  D3DDECLTYPE_UBYTE4    =  5,  // 4D unsigned byte
  D3DDECLTYPE_UBYTE4N   =  8,  // Each of 4 bytes is normalized by dividing to 255.0
  D3DDECLTYPE_SHORT4N   = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
  // Note: There is no equivalent to D3DDECLTYPE_DEC3N (14) as a DXGI_FORMAT
  D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
  D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values

  D3DDECLTYPE_UNUSED    = 17,  // When the type field in a decl is unused.
};

#pragma pack(push,4)

struct D3DVERTEXELEMENT9 {
  uint16    Stream;     // Stream index
  uint16    Offset;     // Offset in the stream in bytes
  uint8    Type;       // Data type
  uint8    Method;     // Processing method
  uint8    Usage;      // Semantics
  uint8    UsageIndex; // Semantic index
};

#pragma pack(pop)

//-------------------------------------------------------------------------------
// Hard Defines for the various structures
//-------------------------------------------------------------------------------
const uint32_t SDKMESH_FILE_VERSION = 101;
const uint32_t MAX_VERTEX_ELEMENTS = 32;
const uint32_t MAX_VERTEX_STREAMS = 16;
const uint32_t MAX_FRAME_NAME = 100;
const uint32_t MAX_MESH_NAME = 100;
const uint32_t MAX_SUBSET_NAME = 100;
const uint32_t MAX_MATERIAL_NAME = 100;
const uint32_t MAX_TEXTURE_NAME = MAX_PATH;
const uint32_t MAX_MATERIAL_PATH = MAX_PATH;
const uint32_t INVALID_FRAME = uint32_t(-1);
const uint32_t INVALID_MESH =  uint32_t(-1);
const uint32_t INVALID_MATERIAL = uint32_t(-1);
const uint32_t INVALID_SUBSET = uint32_t(-1);
const uint32_t INVALID_ANIMATION_DATA = uint32_t(-1);
const uint32_t INVALID_SAMPLER_SLOT = uint32_t(-1);
const uint32_t ERROR_RESOURCE_VALUE = 1;

template<typename TYPE> bool IsErrorResource( TYPE data)
{
  if( ( TYPE)ERROR_RESOURCE_VALUE == data)
    return true;
  return false;
}

//------------------------------------------------------------------------------
// Enumerated Types.  These will have mirrors in both D3D9 and D3D11
//------------------------------------------------------------------------------
enum SDKMESH_PRIMITIVE_TYPE
{
  PT_TRIANGLE_LIST = 0,
  PT_TRIANGLE_STRIP,
  PT_LINE_LIST,
  PT_LINE_STRIP,
  PT_POINT_LIST,
  PT_TRIANGLE_LIST_ADJ,
  PT_TRIANGLE_STRIP_ADJ,
  PT_LINE_LIST_ADJ,
  PT_LINE_STRIP_ADJ,
  PT_QUAD_PATCH_LIST,
  PT_TRIANGLE_PATCH_LIST,
};

enum SDKMESH_INDEX_TYPE
{
  IT_16BIT = 0,
  IT_32BIT,
};

enum FRAME_TRANSFORM_TYPE
{
  FTT_RELATIVE = 0,
  //This is not currently used but is here to support absolute 
  // transformations in the future
  FTT_ABSOLUTE,
};

#pragma pack(push,8)
struct SDKMESH_HEADER {
  //Basic Info and sizes
  uint32 Version;
  uint8  IsBigEndian;
  uint64 HeaderSize;
  uint64 NonBufferDataSize;
  uint64 BufferDataSize;

  //Stats
  uint32 NumVertexBuffers;
  uint32 NumIndexBuffers;
  uint32 NumMeshes;
  uint32 NumTotalSubsets;
  uint32 NumFrames;
  uint32 NumMaterials;

  //Offsets to Data
  uint64 VertexStreamHeadersOffset;
  uint64 IndexStreamHeadersOffset;
  uint64 MeshDataOffset;
  uint64 SubsetDataOffset;
  uint64 FrameDataOffset;
  uint64 MaterialDataOffset;
};

struct SDKMESH_VERTEX_BUFFER_HEADER
{
  uint64 NumVertices;
  uint64 SizeBytes;
  uint64 StrideBytes;
  D3DVERTEXELEMENT9 Decl[MAX_VERTEX_ELEMENTS];
  uint64 DataOffset;
  /*
  union {
    uint64 DataOffset;
    ID3D11Buffer* pVB11;
  };
  */
};

struct SDKMESH_INDEX_BUFFER_HEADER
{
  uint64 NumIndices;
  uint64 SizeBytes;
  uint32 IndexType;
  uint64 DataOffset;
  /*
  union {
    uint64 DataOffset;
    ID3D11Buffer* pIB11;
  };
  */
};

struct SDKMESH_MESH
{
  char Name[MAX_MESH_NAME];
  uint8 NumVertexBuffers;
  uint32 VertexBuffers[MAX_VERTEX_STREAMS];
  uint32 IndexBuffer;
  uint32 NumSubsets;
  uint32 NumFrameInfluences; //aka bones

  Vector3 BoundingBoxCenter;
  Vector3 BoundingBoxExtents;

  union {
    uint64 SubsetOffset;
    uint32* pSubsets;
  };
  union {
    uint64 FrameInfluenceOffset;
    uint32* pFrameInfluences;
  };
};

struct SDKMESH_SUBSET
{
  char Name[MAX_SUBSET_NAME];
  uint32 MaterialID;
  uint32 PrimitiveType;
  uint64 IndexStart;
  uint64 IndexCount;
  uint64 VertexStart;
  uint64 VertexCount;
};

struct SDKMESH_FRAME
{
  char Name[MAX_FRAME_NAME];
  uint32 Mesh;
  uint32 ParentFrame;
  uint32 ChildFrame;
  uint32 SiblingFrame;
  Matrix4 Matrix;
  uint32 AnimationDataIndex;//Used to index which set of keyframes transforms this frame
};

struct SDKMESH_MATERIAL
{
  char    Name[MAX_MATERIAL_NAME];

  // Use MaterialInstancePath
  char    MaterialInstancePath[MAX_MATERIAL_PATH];

  // Or fall back to d3d8-type materials
  char    DiffuseTexture[MAX_TEXTURE_NAME];
  char    NormalTexture[MAX_TEXTURE_NAME];
  char    SpecularTexture[MAX_TEXTURE_NAME];

  Vector4 Diffuse;
  Vector4 Ambient;
  Vector4 Specular;
  Vector4 Emissive;
  FLOAT Power;

  /*
  union {
    uint64 Force64_1;//Force the union to 64bits
    ID3D11Texture2D* pDiffuseTexture11;
  };
  union {
    uint64 Force64_2;//Force the union to 64bits
    ID3D11Texture2D* pNormalTexture11;
  };
  union {
    uint64 Force64_3;//Force the union to 64bits
    ID3D11Texture2D* pSpecularTexture11;
  };

  union {
    uint64 Force64_4;//Force the union to 64bits
    ID3D11ShaderResourceView* pDiffuseRV11;
  };
  union {
    uint64 Force64_5;    //Force the union to 64bits
    ID3D11ShaderResourceView* pNormalRV11;
  };
  union {
    uint64 Force64_6;//Force the union to 64bits
    ID3D11ShaderResourceView* pSpecularRV11;
  };
  */
};

struct SDKANIMATION_FILE_HEADER {
  uint32 Version;
  uint8 IsBigEndian;
  uint32 FrameTransformType;
  uint32 NumFrames;
  uint32 NumAnimationKeys;
  uint32 AnimationFPS;
  uint64 AnimationDataSize;
  uint64 AnimationDataOffset;
};

struct SDKANIMATION_DATA {
  Vector3 Translation;
  Vector4 Orientation;
  Vector3 Scaling;
};

struct SDKANIMATION_FRAME_DATA {
  char FrameName[MAX_FRAME_NAME];
  union
  {
    uint64 DataOffset;
    SDKANIMATION_DATA* pAnimationData;
  };
};

#pragma pack(pop)

PrimitiveTopology TranslatePrimitiveType(uint32 type) {
  PrimitiveTopology primitive;
  switch(type) {
    case PT_TRIANGLE_LIST:
      primitive = kTriangleList;
      break;
    case PT_TRIANGLE_STRIP:
      primitive = kTriangleStrip;
      break;
    case PT_LINE_LIST:
      primitive = kLineList;
      break;
    case PT_LINE_STRIP:
      primitive = kLineStrip;
      break;
    case PT_POINT_LIST:
      primitive = kPointList;
      break;
    case PT_TRIANGLE_LIST_ADJ:
    case PT_TRIANGLE_STRIP_ADJ:
    case PT_LINE_LIST_ADJ:
    case PT_LINE_STRIP_ADJ:
    case PT_QUAD_PATCH_LIST:
    case PT_TRIANGLE_PATCH_LIST:
      throw std::exception("Direct3D9 era tessellation not supported");

    default:
      throw std::exception("Unknown primitive type");
  }

  return primitive;
}

static void GetInputLayoutDesc(const D3DVERTEXELEMENT9 decl[],
                               std::vector<VertexDesc::Desc>& descs) {
  uint32_t offset = 0;
  uint32_t texcoords = 0;

  bool posfound = false;
  VertexDesc::Desc desc;
  desc.input_slot = 0;
  desc.instance_data_step = 0;
  desc.aligned = false;
  for(uint32_t index = 0; index < MAX_VERTEX_ELEMENTS; ++index) {
    if (decl[index].Usage == 0xFF)
      break;

    if (decl[index].Type == D3DDECLTYPE_UNUSED)
      break;

    if (decl[index].Usage == D3DDECLUSAGE_POSITION
        && decl[index].Type == D3DDECLTYPE_FLOAT3) {
      strcpy(desc.name, "POSITION");
      desc.semantic_index = 0;
      desc.type = kVec3;
      descs.push_back(desc);
      posfound = true;
    } else if (decl[index].Usage == D3DDECLUSAGE_NORMAL) {
      strcpy(desc.name, "NORMAL");
      desc.semantic_index = 0;
      desc.type = kVec3;
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
        desc.type = kVec3;
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        CHECK(false);
      } else {
        break;
      }
      descs.push_back(desc);
    } else if (decl[index].Usage == D3DDECLUSAGE_COLOR
               && decl[index].Type == D3DDECLTYPE_D3DCOLOR) {
      strcpy(desc.name, "COLOR");
      desc.semantic_index = 0;
      desc.type = kUint;
      descs.push_back(desc);
    } else if (decl[index].Usage == D3DDECLUSAGE_TANGENT) {
      strcpy(desc.name, "TANGENT");
      desc.semantic_index = 0;
      desc.type = kVec3;
      descs.push_back(desc);
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        CHECK(false);
      } else {
        break;
      }
    } else if (decl[index].Usage == D3DDECLUSAGE_BINORMAL) {
      strcpy(desc.name, "BINORMAL");
      desc.semantic_index = 0;
      desc.type = kVec3;
      descs.push_back(desc);
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        CHECK(false);
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        CHECK(false);
      } else {
        break;
      }
    } else if (decl[index].Usage == D3DDECLUSAGE_TEXCOORD) {
      strcpy(desc.name, "TEXCOORD");
      desc.semantic_index = decl[index].UsageIndex;;
      desc.type = kVec2;
      bool unk = false;
      switch(decl[index].Type) {
        case D3DDECLTYPE_FLOAT2:    
          desc.type = kVec2; break;
        case D3DDECLTYPE_FLOAT1:
          desc.type = kFloat; break;
        case D3DDECLTYPE_FLOAT3:
          desc.type = kVec3; break;
        case D3DDECLTYPE_FLOAT4:
          desc.type = kVec4; break;
        case D3DDECLTYPE_FLOAT16_2:
          CHECK(false); break;
        case D3DDECLTYPE_FLOAT16_4:
          CHECK(false); break;
        default:
          unk = true;
          break;
      }
      if (unk)
        break;

      ++texcoords;
      descs.push_back(desc);
    } else if (decl[index].Usage == D3DDECLUSAGE_BLENDINDICES
               && decl[index].Type == D3DDECLTYPE_UBYTE4) {
      strcpy(desc.name, "BLENDINDICES");
      desc.semantic_index = decl[index].UsageIndex;;
      desc.type = kFloat;
      descs.push_back(desc);
    } else if (decl[index].Usage == D3DDECLUSAGE_BLENDWEIGHT
               && decl[index].Type == D3DDECLTYPE_UBYTE4N) {
      strcpy(desc.name, "BLENDWEIGHT");
      desc.semantic_index = decl[index].UsageIndex;;
      desc.type = kFloat;
      descs.push_back(desc);
    } else {
      break;
    }
  }
}

SdkMeshData::SdkMeshData() {}

bool SdkMeshData::CreateMesh(std::vector<azer::MeshPtr>* meshes, 
                             azer::EffectAdapterContext* ctx,
                             FileSystem* fs) {
  RenderSystem* rs = RenderSystem::Current();
  std::vector<SdkMeshMaterialPtr> materials;
  scoped_refptr<SdkMeshEffect> effect = CreateSdkMeshEffect();
  std::vector<VertexBufferPtr> vbs;
  std::vector<IndicesBufferPtr> ibs;

  for (int32 i = 0; i < mtrls_.size(); ++i) {
    SdkMeshMaterialPtr m(new SdkMeshMaterial);
    m->set_ambient(Vector4(mtrls_[i].ambient_color, 1.0f));
    m->set_diffuse(Vector4(mtrls_[i].diffuse_color, 1.0f));
    m->set_specular(Vector4(mtrls_[i].specular_color, 1.0f));
    m->set_emissive(Vector4(mtrls_[i].emissive_color, 1.0f));

    if (!mtrls_[i].diffuse_texture.empty()) {
      m->set_diffusemap(Load2DTexture(
          ResPath(UTF8ToUTF16(mtrls_[i].diffuse_texture)), fs));
    }

    if (!mtrls_[i].normal_texture.empty()) {
      m->set_normalmap(Load2DTexture(
        ResPath(UTF8ToUTF16(mtrls_[i].normal_texture)), fs));
    }

    if (!mtrls_[i].specular_texture.empty()) {
      m->set_specularmap(Load2DTexture(
          ResPath(UTF8ToUTF16(mtrls_[i].specular_texture)), fs));
    }
    materials.push_back(m);
  }

  for (int32 i = 0; i < vdata_vec_.size(); ++i) {
    vbs.push_back(rs->CreateVertexBuffer(VertexBuffer::Options(), vdata_vec_[i]));
  }
  for (int32 i = 0; i < idata_vec_.size(); ++i) {
    ibs.push_back(rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata_vec_[i]));
  }

  for (uint32 i = 0; i < meshes_.size(); ++i) {
    azer::MeshPtr mesh(new azer::Mesh(ctx));
    meshes->push_back(mesh);
    for (uint32 j = 0; j < meshes_[i].subsets.size(); ++j) {
      MeshPartPtr part(new MeshPart(effect));
      part->SetEffectAdapterContext(ctx);
      const Subset& subset = meshes_[i].subsets[j];
      VertexBuffer* vb = vbs[subset.vertex_data_index].get();
      IndicesBuffer* ib = subset.indices_data_index >= 0 
        ? ibs[subset.indices_data_index].get() : NULL;
      EntityPtr entity = new Entity(vb, ib);
      entity->set_vertex_base(subset.vertex_base);
      entity->set_start_index(subset.start_index);
      part->AddEntity(entity);
      part->AddProvider(materials[subset.material_index]);
      mesh->AddMeshPart(part);
    }
  }

  return true;
}

bool SdkMeshData::LoadFromData(const uint8* data, int32 size) {
  if (!LoadVertexData(data, size)) {
    LOG(ERROR) << "Failed to load VertexData;";
    return false;
  }

  if (!LoadIndicesData(data, size)) {
    LOG(ERROR) << "Failed to load IndicesData;";
    return false;
  }

  if (!LoadMaterial(data, size)) {
    LOG(ERROR) << "Failed to load Material.";
    return false;
  }

  if (!LoadMesh(data, size)) {
    LOG(ERROR) << "Failed to load Mesh.";
    return false;
  }

  return true;
}

bool SdkMeshData::LoadMaterial(const uint8* data, int32 size) {
  auto header = reinterpret_cast<const SDKMESH_HEADER*>(data);
  auto materialArray = reinterpret_cast<const SDKMESH_MATERIAL*>(
      data + header->MaterialDataOffset);

  for (int32 i = 0; i < header->NumMaterials; ++i) {
    auto& m = materialArray[i];
    Material mtrl;
    mtrl.name = m.Name;
    mtrl.diffuse_color = m.Diffuse;
    mtrl.ambient_color = m.Ambient;
    mtrl.specular_color = m.Specular;
    mtrl.emissive_color = m.Emissive;
    mtrl.specular_power = m.Power;
    mtrl.diffuse_texture = m.DiffuseTexture;
    mtrl.normal_texture = m.NormalTexture;
    mtrl.specular_texture = m.SpecularTexture;
    if (!mtrl.diffuse_texture.empty()) {
      mtrl.diffuse_texture.insert(0, "//");
    }
    if (!mtrl.normal_texture.empty()) {
      mtrl.normal_texture.insert(0, "//");
    }
    if (!mtrl.specular_texture.empty()) {
      mtrl.specular_texture.insert(0, "//");
    }
    mtrls_.push_back(mtrl);
  }
  return true;
}

bool SdkMeshData::LoadIndicesData(const uint8* data, int32 size) {
  auto header = reinterpret_cast<const SDKMESH_HEADER*>(data);
  auto ibArray = reinterpret_cast<const SDKMESH_INDEX_BUFFER_HEADER*>(
      data + header->IndexStreamHeadersOffset);
  uint64 bufferDataOffset = header->HeaderSize + header->NonBufferDataSize;
  const uint8* bufferData = data + bufferDataOffset;
  for(uint32 i = 0; i < header->NumIndexBuffers; ++i) {
    auto& ih = ibArray[i];
    if (size < ih.DataOffset || (size < ih.DataOffset + ih.SizeBytes)) {
      LOG(ERROR) << "Indices data overflow.";
      return false;
    }

    if (ih.IndexType != IT_16BIT && ih.IndexType != IT_32BIT) {
      LOG(ERROR) << "Invalid index buffer type found";
      return false;
    }

    auto indices = reinterpret_cast<const uint8_t*>(
        bufferData + (ih.DataOffset - bufferDataOffset));
    
    IndexType type = ih.IndexType == IT_16BIT ? kIndexUint16 : kIndexUint32;
    int32 unit_size = ih.IndexType == IT_16BIT ? 2 : 4;
    int32 index_count = ih.NumIndices;
    IndicesDataPtr idata(new IndicesData(index_count, type));
    memcpy(idata->pointer(), indices, ih.SizeBytes);
    idata_vec_.push_back(idata);
  }

  return true;
}

bool SdkMeshData::LoadVertexData(const uint8* data, int32 size) {
  auto header = reinterpret_cast<const SDKMESH_HEADER*>(data);
  auto vbArray = reinterpret_cast<const SDKMESH_VERTEX_BUFFER_HEADER*>(
      data + header->VertexStreamHeadersOffset);
  uint64 bufferDataOffset = header->HeaderSize + header->NonBufferDataSize;
  const uint8* bufferData = data + bufferDataOffset;
  for(uint32 i = 0; i < header->NumVertexBuffers; ++i) {
    auto& vh = vbArray[i];
    if (size < vh.DataOffset || (size < vh.DataOffset + vh.SizeBytes)) {
      LOG(ERROR) << "vertex data overflow.";
      return false;
    }

    std::vector<VertexDesc::Desc> descs;
    GetInputLayoutDesc(vh.Decl, descs);
    VertexDescPtr desc(new VertexDesc(&descs.front(), descs.size()));
    SlotVertexDataPtr vdata(new SlotVertexData(desc, vh.NumVertices));
    auto verts = reinterpret_cast<const uint8_t*>(
        bufferData + (vh.DataOffset - bufferDataOffset));
    memcpy(vdata->pointer(), verts, vh.SizeBytes);
    vdata_vec_.push_back(vdata);
  }
  return true;
}

bool SdkMeshData::LoadMesh(const uint8* data, int32 size) {
  auto header = reinterpret_cast<const SDKMESH_HEADER*>(data);
  auto meshArray = reinterpret_cast<const SDKMESH_MESH*>(
      data + header->MeshDataOffset);
  auto subsetArray = reinterpret_cast<const SDKMESH_SUBSET*>(
      data + header->SubsetDataOffset);
  for(uint32 midx = 0; midx < header->NumMeshes; ++midx) {
    auto& mh = meshArray[midx];
    if (!mh.NumSubsets
         || !mh.NumVertexBuffers
         || mh.IndexBuffer >= header->NumIndexBuffers
        || mh.VertexBuffers[0] >= header->NumVertexBuffers) {
      LOG(ERROR) << "Invalid mesh found";
      return false;
    }

    if (size < mh.SubsetOffset || (size < mh.SubsetOffset + 
                                   mh.NumSubsets * sizeof(uint32))) {
      LOG(ERROR) << "Meshdata overflow";
      return false;
    }

    if (mh.NumFrameInfluences > 0) {
      if (size < mh.FrameInfluenceOffset ||
          (size < mh.FrameInfluenceOffset + mh.NumFrameInfluences*sizeof(uint32))) {
        LOG(ERROR) << "End of file";
        return false;
      }
    }
    

    meshes_.push_back(Mesh());
    Mesh& mesh = meshes_.back();
    mesh.name = mh.Name;
    mesh.center = mh.BoundingBoxCenter;
    mesh.extends = mh.BoundingBoxExtents;

    auto subsets = reinterpret_cast<const uint32*>(data + mh.SubsetOffset);
    for(uint32 j = 0; j < mh.NumSubsets; ++j) {
      auto sIndex = subsets[j];
      if (sIndex >= header->NumTotalSubsets) {
        LOG(ERROR) << "Invalid mesh found";
        return false;
      }

      auto& subset = subsetArray[sIndex];
      if (subset.MaterialID >= header->NumMaterials) {
        LOG(ERROR) << "Invalid mesh found";
        return false;
      }

      PrimitiveTopology primitive = TranslatePrimitiveType(
          subsetArray[sIndex].PrimitiveType);
      Subset s;
      s.vertex_base = static_cast<uint32>(subset.VertexStart);
      s.start_index = static_cast<uint32>(subset.IndexStart);
      s.vertex_data_index = mh.VertexBuffers[0];
      s.vertex_data_index = mh.IndexBuffer;
      s.material_index = subset.MaterialID;
      s.primitive = TranslatePrimitiveType(subset.PrimitiveType);
      mesh.subsets.push_back(s);
    }
  }
  return true;
}
