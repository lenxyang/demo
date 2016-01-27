#include "sdkmesh_loader.h"

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "demo/base/resource_util.h"
#include "azer/render/render.h"

using namespace azer;
using base::UTF8ToUTF16;

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

void InitFromSDKMaterial(const SDKMESH_MATERIAL& mh, bool enable_vertex_color,
                         bool enable_skinned, bool enable_dual_texture, 
                         FileSystem* fs, SdkMeshMaterial* mtrl) {
  mtrl->ambient_ = mh.Ambient;
  mtrl->diffuse_ = mh.Diffuse;
  mtrl->specular_ = mh.Specular;
  mtrl->emissive_ = mh.Emissive;
  if (mh.Diffuse.w != 1.f && mh.Diffuse.w != 0.f) {
    mtrl->alpha_ = mh.Diffuse.w;
  } else {
    mtrl->alpha_ = 1.0f;
  }

  if (mh.Power) {
    mtrl->specular_power_ = mh.Power;
    mtrl->specular_ = mh.Specular;
  }
  // mtrl->alpha_ = (mh.Alpha > 1.0f);

  std::string path1 = std::string("//") + mh.DiffuseTexture;
  if (path1.length() > 2) {
    mtrl->texture1_ = Load2DTexture(ResPath(UTF8ToUTF16(path1)), fs);
  }
  if (enable_dual_texture) {
    std::string path2 = std::string("//") + mh.SpecularTexture;
    mtrl->texture2_ = Load2DTexture(ResPath(UTF8ToUTF16(path2)), fs);
  }
}


//---------------------------------------------------------------------------------
// Direct3D 9 Vertex Declaration to DirectInput 11 Input Layout mapping

static void GetInputLayoutDesc( _In_reads_(32) const D3DVERTEXELEMENT9 decl[],
                                std::vector<VertexDesc::Desc>& descs,
                                bool &perVertexColor, bool& enableSkinning, 
                                bool& dualTexture) {
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
      enableSkinning = true;
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

  if (!posfound)
    throw std::exception("SV_Position is required");

  if (texcoords == 2) {
    dualTexture = true;
  }
}



//===============================================================================
// Model Loader
//===============================================================================

void CreateFromSDKMESH(const uint8* meshData, uint32 dataSize, 
                       bool ccw, bool pmalpha, SdkModel* model, FileSystem* fs) {
  // File Headers
  if (dataSize < sizeof(SDKMESH_HEADER))
    throw std::exception("End of file");
  auto header = reinterpret_cast<const SDKMESH_HEADER*>(meshData);

  size_t headerSize = sizeof(SDKMESH_HEADER)
      + header->NumVertexBuffers * sizeof(SDKMESH_VERTEX_BUFFER_HEADER)
      + header->NumIndexBuffers * sizeof(SDKMESH_INDEX_BUFFER_HEADER);
  if (header->HeaderSize != headerSize)
    throw std::exception("Not a valid SDKMESH file");

  if (dataSize < header->HeaderSize)
    throw std::exception("End of file");

  if(header->Version != SDKMESH_FILE_VERSION)
    throw std::exception("Not a supported SDKMESH version");

  if (header->IsBigEndian)
    throw std::exception("Loading BigEndian SDKMESH files not supported");

  if (!header->NumMeshes)
    throw std::exception("No meshes found");

  if (!header->NumVertexBuffers)
    throw std::exception("No vertex buffers found");

  if (!header->NumIndexBuffers)
    throw std::exception("No index buffers found");

  if (!header->NumTotalSubsets)
    throw std::exception("No subsets found");

  if (!header->NumMaterials)
    throw std::exception("No materials found");

  // Sub-headers
  if (dataSize < header->VertexStreamHeadersOffset
       || (dataSize < (header->VertexStreamHeadersOffset + header->NumVertexBuffers * sizeof(SDKMESH_VERTEX_BUFFER_HEADER))))
    throw std::exception("End of file");
  auto vbArray = reinterpret_cast<const SDKMESH_VERTEX_BUFFER_HEADER*>(
      meshData + header->VertexStreamHeadersOffset);

  if (dataSize < header->IndexStreamHeadersOffset
       || (dataSize < (header->IndexStreamHeadersOffset + header->NumIndexBuffers * sizeof(SDKMESH_INDEX_BUFFER_HEADER))))
    throw std::exception("End of file");
  auto ibArray = reinterpret_cast<const SDKMESH_INDEX_BUFFER_HEADER*>(
      meshData + header->IndexStreamHeadersOffset);

  if (dataSize < header->MeshDataOffset
       || (dataSize < (header->MeshDataOffset + header->NumMeshes * sizeof(SDKMESH_MESH))))
    throw std::exception("End of file");
  auto meshArray = reinterpret_cast<const SDKMESH_MESH*>(
      meshData + header->MeshDataOffset);

  if (dataSize < header->SubsetDataOffset
      || (dataSize < (header->SubsetDataOffset + header->NumTotalSubsets * sizeof(SDKMESH_SUBSET))))
    throw std::exception("End of file");
  auto subsetArray = reinterpret_cast<const SDKMESH_SUBSET*>(
      meshData + header->SubsetDataOffset);

  if (dataSize < header->FrameDataOffset
       || (dataSize < (header->FrameDataOffset + header->NumFrames * sizeof(SDKMESH_FRAME))))
    throw std::exception("End of file");
  // TODO - auto frameArray = reinterpret_cast<const SDKMESH_FRAME*>(meshData + header->FrameDataOffset);

  if (dataSize < header->MaterialDataOffset
       || (dataSize < (header->MaterialDataOffset + header->NumMaterials * sizeof(SDKMESH_MATERIAL))))
    throw std::exception("End of file");
  auto materialArray = reinterpret_cast<const SDKMESH_MATERIAL*>(
      meshData + header->MaterialDataOffset);

  // Buffer data
  uint64_t bufferDataOffset = header->HeaderSize + header->NonBufferDataSize;
  if ((dataSize < bufferDataOffset)
       || (dataSize < bufferDataOffset + header->BufferDataSize))
    throw std::exception("End of file");
  const uint8_t* bufferData = meshData + bufferDataOffset;

  // Create vertex buffers
  std::vector<bool> perVertexColor;
  perVertexColor.resize(header->NumVertexBuffers);

  std::vector<bool> enableSkinning;
  enableSkinning.resize(header->NumVertexBuffers);

  std::vector<bool> enableDualTexture;
  enableDualTexture.resize(header->NumVertexBuffers);

  RenderSystem* rs = RenderSystem::Current();
  std::vector<VertexBufferPtr> vbs;;
  for(UINT j=0; j < header->NumVertexBuffers; ++j) {
    auto& vh = vbArray[j];
    if (dataSize < vh.DataOffset
         || (dataSize < vh.DataOffset + vh.SizeBytes))
      throw std::exception("End of file");

    bool vertColor = false;
    bool skinning = false;
    bool dualTexture = false;

    std::vector<VertexDesc::Desc> descs;
    GetInputLayoutDesc(vh.Decl, descs, vertColor, skinning, dualTexture);
    perVertexColor[j] = vertColor;
    enableSkinning[j] = skinning;
    enableDualTexture[j] = dualTexture;

    VertexDescPtr desc(new VertexDesc(&descs.front(), descs.size()));
    int32 vertex_count = vh.SizeBytes / desc->vertex_size();
    SlotVertexDataPtr vdata(new SlotVertexData(desc, vertex_count));
    auto verts = reinterpret_cast<const uint8_t*>(
        bufferData + (vh.DataOffset - bufferDataOffset));
    memcpy(vdata->pointer(), verts, vh.SizeBytes);
    vbs.push_back(rs->CreateVertexBuffer(VertexBuffer::Options(), vdata));
  }

  // Create index buffers
  std::vector<IndicesBufferPtr> ibs; 
  for(UINT j=0; j < header->NumIndexBuffers; ++j) {
    auto& ih = ibArray[j];

    if (dataSize < ih.DataOffset
         || (dataSize < ih.DataOffset + ih.SizeBytes))
      throw std::exception("End of file");

    if (ih.IndexType != IT_16BIT && ih.IndexType != IT_32BIT)
      throw std::exception("Invalid index buffer type found");

    auto indices = reinterpret_cast<const uint8_t*>(
        bufferData + (ih.DataOffset - bufferDataOffset));

    IndexType type = ih.IndexType == IT_16BIT ? kIndexUint16 : kIndexUint32;
    int32 unit_size = ih.IndexType == IT_16BIT ? 2 : 4;
    int32 index_count = ih.SizeBytes / unit_size;
    IndicesDataPtr idata(new IndicesData(index_count, type));
    memcpy(idata->pointer(), indices, ih.SizeBytes);
    ibs.push_back(rs->CreateIndicesBuffer(IndicesBuffer::Options(), idata));
  }

  
  model->materials.resize(header->NumMaterials);
  // Create meshes
  for(UINT meshIndex = 0; meshIndex < header->NumMeshes; ++meshIndex) {
    auto& mh = meshArray[meshIndex];

    if (!mh.NumSubsets
         || !mh.NumVertexBuffers
         || mh.IndexBuffer >= header->NumIndexBuffers
         || mh.VertexBuffers[0] >= header->NumVertexBuffers)
      throw std::exception("Invalid mesh found");

    // mh.NumVertexBuffers is sometimes not what you'd expect, so we skip validating it

    if (dataSize < mh.SubsetOffset
         || (dataSize < mh.SubsetOffset + mh.NumSubsets*sizeof(UINT)))
      throw std::exception("End of file");

    auto subsets = reinterpret_cast<const UINT*>(meshData + mh.SubsetOffset);

    if (mh.NumFrameInfluences > 0) {
      if (dataSize < mh.FrameInfluenceOffset
           || (dataSize < mh.FrameInfluenceOffset + mh.NumFrameInfluences*sizeof(UINT)))
        throw std::exception("End of file");

      // TODO - auto influences = reinterpret_cast<const UINT*>(
      // meshData + mh.FrameInfluenceOffset);
    }

    
    SdkMesh mesh;
    mesh.name = mh.Name;
    mesh.ccw = ccw;
    mesh.pmalpha = pmalpha;
    mesh.center = mh.BoundingBoxCenter;
    mesh.extents = mh.BoundingBoxExtents;

    // Create subsets
    for(UINT j = 0; j < mh.NumSubsets; ++j) {
      auto sIndex = subsets[j];
      if (sIndex >= header->NumTotalSubsets)
        throw std::exception("Invalid mesh found");

      PrimitiveTopology primitive;
      auto& subset = subsetArray[sIndex];
      switch(subset.PrimitiveType) {
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

      if (subset.MaterialID >= header->NumMaterials)
        throw std::exception("Invalid mesh found");
      
      if (!model->materials[subset.MaterialID].get()) {
        size_t vi = mh.VertexBuffers[0];
        scoped_refptr<SdkMeshMaterial> mtrl(new SdkMeshMaterial);
        InitFromSDKMaterial(materialArray[subset.MaterialID], 
                            perVertexColor[vi],
                            enableSkinning[vi],
                            enableDualTexture[vi],
                            fs,
                            mtrl.get());
        model->materials[subset.MaterialID] = mtrl;
      }

      EntityPtr entity;
      // part.isalpha = mat.alpha;
      entity = new Entity(vbs[mh.VertexBuffers[0]], ibs[mh.IndexBuffer]);
      entity->set_primitive(primitive);
      entity->set_vertex_base(static_cast<uint32_t>(subset.VertexStart));
      entity->set_start_index(static_cast<uint32_t>(subset.IndexStart));
      MeshPartPtr part(new MeshPart(NULL));
      part->AddEntity(entity);
      part->AddProvider(model->materials[subset.MaterialID]);
      mesh.part.push_back(part);
    }

    model->meshes.push_back(mesh);
  }
}


bool LoadSDKModel(const ::base::FilePath& path, SdkModel* model) {
  std::string contents;
  if (!ReadFileToString(path, &contents)) {
    LOG(ERROR) << "Failed to load vertex data from: " << path.value();
    return false;
  }

  try {
    CreateFromSDKMESH((const uint8*)contents.c_str(), contents.length(),
                      true, false, model, NULL);
    return true;
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to Load SDKMesh: " << e.what();
    return false;
  }
}

bool LoadSDKModel(const azer::ResPath& path, azer::FileSystem* fs, SdkModel* model) {
  FileContents contents;
  if (!LoadFileContents(path, &contents , fs)) {
    LOG(ERROR) << "Failed to load data: " << path.fullpath();
    return false;
  }

  try {
    CreateFromSDKMESH(&contents.front(), contents.size(), true, false, model, fs);
    return true;
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to Load SDKMesh: " << e.what();
    return false;
  }
}

// class SdkMeshMaterial
const char SdkMeshMaterial::kEffectProviderName[] = "SdkMeshMaterial";
SdkMeshMaterial::SdkMeshMaterial() {}
const char* SdkMeshMaterial::GetProviderName() const {
  return kEffectProviderName;
}

// class SdkMeshEffect
const char SdkMeshEffect::kEffectName[] = "SdkMeshEffect";
SdkMeshEffect::SdkMeshEffect() {
  world_ = Matrix4::kIdentity;
}
bool SdkMeshEffect::Init(VertexDesc* desc, const ShaderPrograms& sources) {
  DCHECK(sources.size() == kRenderPipelineStageNum);
  DCHECK(!sources[kVertexStage].code.empty());
  DCHECK(!sources[kPixelStage].code.empty());
  DCHECK(desc);
  vertex_desc_ = desc;
  InitShaders(sources);
  InitGpuConstantTable();
  return true;
}
void SdkMeshEffect::InitGpuConstantTable() {
  RenderSystem* rs = RenderSystem::Current();
  // generate GpuTable init for stage kVertexStage
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("pvw", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, pvw), 1),
    GpuConstantsTable::Desc("world", GpuConstantsType::kMatrix4,
                            offsetof(vs_cbuffer, world), 1),
  };
  gpu_table_[kVertexStage] = rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc);
}

void SdkMeshEffect::ApplyGpuConstantTable(Renderer* renderer) {
  {
    Matrix4 pvw = std::move(pv_ * world_);
    GpuConstantsTable* tb = gpu_table_[(int)kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, &pvw, sizeof(Matrix4));
    tb->SetValue(1, &world_, sizeof(Matrix4));
  }
}

scoped_refptr<SdkMeshEffect> CreateSimpleEffect() {
  // class PositionVertex
  const VertexDesc::Desc kVertexDesc[] = {
    {"POSITION", 0, kVec3},
    {"NORMAL", 0, kVec3},
    {"TEXCOORD", 0, kVec2},
    {"TANGENT", 0, kVec3},
  };
  Effect::ShaderPrograms shaders;
  shaders.resize(kRenderPipelineStageNum);
  shaders[kVertexStage].path = "effect.vs";
  shaders[kVertexStage].stage = kVertexStage;
  shaders[kVertexStage].code = ""
      "#pragma pack_matrix(row_major)\n"
      "struct VsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "}\n;"
      "struct VSInput {\n"
      "  float3 position:POSITION;\n"
      "  float3 normal:NORMAL;\n"
      "  float2 texcoord:TEXCOORD;\n"
      "  float3 tangent:TANGENT;\n"
      "};\n"
      "cbuffer c_buffer {\n"
      "  float4x4 pvw;"
      "  float4x4 world;"
      "};"
      "VsOutput vs_main(VSInput input) {\n"
      "VsOutput o;"
      "o.position = mul(pvw, float4(input.position, 1.0));"
      "return o;"
      "}";
  shaders[kPixelStage].path = "effect.ps";
  shaders[kPixelStage].stage = kPixelStage;
  shaders[kPixelStage].code = "#pragma pack_matrix(row_major)\n"
      "struct VsOutput {\n"
      "  float4 position:SV_POSITION;\n"
      "};\n"
      "float4 ps_main(VsOutput o):SV_TARGET {\n"
      "  return color;"
      "}\n";
  VertexDescPtr desc(new VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  scoped_refptr<SdkMeshEffect> ptr(new SdkMeshEffect);
  ptr->Init(desc, shaders);
  return ptr;
}
