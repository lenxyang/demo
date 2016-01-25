#include "sdkmesh_loader.h"

#include <string>
#include <vector>

#include "base/basictypes.h"


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

struct D3DVERTEXELEMENT9
{
  WORD    Stream;     // Stream index
  WORD    Offset;     // Offset in the stream in bytes
  BYTE    Type;       // Data type
  BYTE    Method;     // Processing method
  BYTE    Usage;      // Semantics
  BYTE    UsageIndex; // Semantic index
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
  union {
    uint64 DataOffset;
    ID3D11Buffer* pVB11;
  };
};

struct SDKMESH_INDEX_BUFFER_HEADER
{
  uint64 NumIndices;
  uint64 SizeBytes;
  uint32 IndexType;
  union {
    uint64 DataOffset;
    ID3D11Buffer* pIB11;
  };
};

struct SDKMESH_MESH
{
  char Name[MAX_MESH_NAME];
  BYTE NumVertexBuffers;
  uint32 VertexBuffers[MAX_VERTEX_STREAMS];
  uint32 IndexBuffer;
  uint32 NumSubsets;
  uint32 NumFrameInfluences; //aka bones

  DirectX::XMFLOAT3 BoundingBoxCenter;
  DirectX::XMFLOAT3 BoundingBoxExtents;

  union {
    uint64 SubsetOffset;
    INT* pSubsets;
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
};

struct SDKANIMATION_FILE_HEADER {
  uint32 Version;
  BYTE IsBigEndian;
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


//---------------------------------------------------------------------------------
// Direct3D 9 Vertex Declaration to DirectInput 11 Input Layout mapping

static void GetInputLayoutDesc( _In_reads_(32) const DXUT::D3DVERTEXELEMENT9 decl[],
                                std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc,
                                bool &perVertexColor, bool& enableSkinning, 
                                bool& dualTexture) {
  static const D3D11_INPUT_ELEMENT_DESC elements[] = {
    { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,     0, 
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BLENDINDICES",0, DXGI_FORMAT_R8G8B8A8_UINT,      0,
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BLENDWEIGHT", 0, DXGI_FORMAT_R8G8B8A8_UNORM,     0,
      D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  uint32_t offset = 0;
  uint32_t texcoords = 0;

  bool posfound = false;

  for(uint32_t index = 0; index < DXUT::MAX_VERTEX_ELEMENTS; ++index) {
    if (decl[index].Usage == 0xFF)
      break;

    if (decl[index].Type == D3DDECLTYPE_UNUSED)
      break;

    if (decl[index].Offset != offset)
      break;

    if (decl[index].Usage == D3DDECLUSAGE_POSITION
         && decl[index].Type == D3DDECLTYPE_FLOAT3) {
      inputDesc.push_back(elements[0]);
      offset += 12;
      posfound = true;
    } else if (decl[index].Usage == D3DDECLUSAGE_NORMAL) {
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
        inputDesc.push_back(elements[1]);
        offset += 12;
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[1];
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[1];
        desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[1];
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        inputDesc.push_back(desc);
        offset += 4;
      } else {
        break;
      }
    } else if (decl[index].Usage == D3DDECLUSAGE_COLOR
               && decl[index].Type == D3DDECLTYPE_D3DCOLOR) {
      inputDesc.push_back(elements[2]);
      offset += 4;
      perVertexColor = true;
    } else if (decl[index].Usage == D3DDECLUSAGE_TANGENT) {
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
        inputDesc.push_back(elements[3]);
        offset += 12;
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[3];
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[3];
        desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[3];
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        inputDesc.push_back(desc);
        offset += 4;
      } else {
        break;
      }
    } else if (decl[index].Usage == D3DDECLUSAGE_BINORMAL) {
      if (decl[index].Type == D3DDECLTYPE_FLOAT3) {
        inputDesc.push_back(elements[4]);
        offset += 12;
      } else if (decl[index].Type == D3DDECLTYPE_FLOAT16_4) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[4];
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_SHORT4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[4];
        desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
        inputDesc.push_back(desc);
        offset += 8;
      } else if (decl[index].Type == D3DDECLTYPE_UBYTE4N) {
        D3D11_INPUT_ELEMENT_DESC desc = elements[4];
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        inputDesc.push_back(desc);
        offset += 4;
      } else {
        break;
      }
    } else if (decl[index].Usage == D3DDECLUSAGE_TEXCOORD) {
      D3D11_INPUT_ELEMENT_DESC desc = elements[5];
      desc.SemanticIndex = decl[index].UsageIndex;

      bool unk = false;
      switch(decl[index].Type) {
        case D3DDECLTYPE_FLOAT2:    
          offset += 8; break;
        case D3DDECLTYPE_FLOAT1:    
          desc.Format = DXGI_FORMAT_R32_FLOAT; offset += 4; break;
        case D3DDECLTYPE_FLOAT3:
          desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; offset += 12; break;
        case D3DDECLTYPE_FLOAT4:
          desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; offset += 16; break;
        case D3DDECLTYPE_FLOAT16_2:
          desc.Format = DXGI_FORMAT_R16G16_FLOAT; offset += 4; break;
        case D3DDECLTYPE_FLOAT16_4:
          desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; offset += 8; break;
        default:
          unk = true;
          break;
      }

      if (unk)
        break;

      ++texcoords;

      inputDesc.push_back(desc);
    } else if (decl[index].Usage == D3DDECLUSAGE_BLENDINDICES
               && decl[index].Type == D3DDECLTYPE_UBYTE4) {
      enableSkinning = true;
      inputDesc.push_back(elements[6]);
      offset += 4;
    } else if (decl[index].Usage == D3DDECLUSAGE_BLENDWEIGHT
               && decl[index].Type == D3DDECLTYPE_UBYTE4N) {
      enableSkinning = true;
      inputDesc.push_back(elements[7]);
      offset += 4;
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



//======================================================================================
// Model Loader
//======================================================================================

std::unique_ptr<Model> DirectX::Model::CreateFromSDKMESH(
    ID3D11Device* d3dDevice, const uint8_t* meshData, size_t dataSize, 
    IEffectFactory& fxFactory, bool ccw, bool pmalpha) {
  if (!d3dDevice || !meshData)
    throw std::exception("Device and meshData cannot be null");

  // File Headers
  if (dataSize < sizeof(DXUT::SDKMESH_HEADER))
    throw std::exception("End of file");
  auto header = reinterpret_cast<const DXUT::SDKMESH_HEADER*>(meshData);

  size_t headerSize = sizeof(DXUT::SDKMESH_HEADER)
      + header->NumVertexBuffers * sizeof(DXUT::SDKMESH_VERTEX_BUFFER_HEADER)
      + header->NumIndexBuffers * sizeof(DXUT::SDKMESH_INDEX_BUFFER_HEADER);
  if (header->HeaderSize != headerSize)
    throw std::exception("Not a valid SDKMESH file");

  if (dataSize < header->HeaderSize)
    throw std::exception("End of file");

  if(header->Version != DXUT::SDKMESH_FILE_VERSION)
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
       || (dataSize < (header->VertexStreamHeadersOffset + header->NumVertexBuffers * sizeof(DXUT::SDKMESH_VERTEX_BUFFER_HEADER))))
    throw std::exception("End of file");
  auto vbArray = reinterpret_cast<const DXUT::SDKMESH_VERTEX_BUFFER_HEADER*>(
      meshData + header->VertexStreamHeadersOffset);

  if (dataSize < header->IndexStreamHeadersOffset
       || (dataSize < (header->IndexStreamHeadersOffset + header->NumIndexBuffers * sizeof(DXUT::SDKMESH_INDEX_BUFFER_HEADER))))
    throw std::exception("End of file");
  auto ibArray = reinterpret_cast<const DXUT::SDKMESH_INDEX_BUFFER_HEADER*>(
      meshData + header->IndexStreamHeadersOffset);

  if (dataSize < header->MeshDataOffset
       || (dataSize < (header->MeshDataOffset + header->NumMeshes * sizeof(DXUT::SDKMESH_MESH))))
    throw std::exception("End of file");
  auto meshArray = reinterpret_cast<const DXUT::SDKMESH_MESH*>(
      meshData + header->MeshDataOffset);

  if (dataSize < header->SubsetDataOffset
      || (dataSize < (header->SubsetDataOffset + header->NumTotalSubsets * sizeof(DXUT::SDKMESH_SUBSET))))
    throw std::exception("End of file");
  auto subsetArray = reinterpret_cast<const DXUT::SDKMESH_SUBSET*>(
      meshData + header->SubsetDataOffset);

  if (dataSize < header->FrameDataOffset
       || (dataSize < (header->FrameDataOffset + header->NumFrames * sizeof(DXUT::SDKMESH_FRAME))))
    throw std::exception("End of file");
  // TODO - auto frameArray = reinterpret_cast<const DXUT::SDKMESH_FRAME*>(meshData + header->FrameDataOffset);

  if (dataSize < header->MaterialDataOffset
       || (dataSize < (header->MaterialDataOffset + header->NumMaterials * sizeof(DXUT::SDKMESH_MATERIAL))))
    throw std::exception("End of file");
  auto materialArray = reinterpret_cast<const DXUT::SDKMESH_MATERIAL*>(
      meshData + header->MaterialDataOffset);

  // Buffer data
  uint64_t bufferDataOffset = header->HeaderSize + header->NonBufferDataSize;
  if ((dataSize < bufferDataOffset)
       || (dataSize < bufferDataOffset + header->BufferDataSize))
    throw std::exception("End of file");
  const uint8_t* bufferData = meshData + bufferDataOffset;

  // Create vertex buffers
  std::vector<ComPtr<ID3D11Buffer>> vbs;
  vbs.resize(header->NumVertexBuffers);

  std::vector<std::shared_ptr<std::vector<D3D11_INPUT_ELEMENT_DESC>>> vbDecls;
  vbDecls.resize(header->NumVertexBuffers);

  std::vector<bool> perVertexColor;
  perVertexColor.resize(header->NumVertexBuffers);

  std::vector<bool> enableSkinning;
  enableSkinning.resize(header->NumVertexBuffers);

  std::vector<bool> enableDualTexture;
  enableDualTexture.resize(header->NumVertexBuffers);

  for(UINT j=0; j < header->NumVertexBuffers; ++j) {
    auto& vh = vbArray[j];

    if (dataSize < vh.DataOffset
         || (dataSize < vh.DataOffset + vh.SizeBytes))
      throw std::exception("End of file");

    vbDecls[j] = std::make_shared<std::vector<D3D11_INPUT_ELEMENT_DESC>>();
    bool vertColor = false;
    bool skinning = false;
    bool dualTexture = false;
    GetInputLayoutDesc(vh.Decl, *vbDecls[j].get(), vertColor, skinning, dualTexture);
    perVertexColor[j] = vertColor;
    enableSkinning[j] = skinning;
    enableDualTexture[j] = dualTexture;

    auto verts = reinterpret_cast<const uint8_t*>(
        bufferData + (vh.DataOffset - bufferDataOffset));

    D3D11_BUFFER_DESC desc = {0};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(vh.SizeBytes);
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = verts;

    ThrowIfFailed(d3dDevice->CreateBuffer(&desc, &initData, &vbs[j]));

    SetDebugObjectName(vbs[j].Get(), "ModelSDKMESH");
  }

  // Create index buffers
  std::vector<ComPtr<ID3D11Buffer>> ibs;
  ibs.resize(header->NumIndexBuffers);

  for(UINT j=0; j < header->NumIndexBuffers; ++j) {
    auto& ih = ibArray[j];

    if (dataSize < ih.DataOffset
         || (dataSize < ih.DataOffset + ih.SizeBytes))
      throw std::exception("End of file");

    if (ih.IndexType != DXUT::IT_16BIT && ih.IndexType != DXUT::IT_32BIT)
      throw std::exception("Invalid index buffer type found");

    auto indices = reinterpret_cast<const uint8_t*>(bufferData + (ih.DataOffset - bufferDataOffset));

    D3D11_BUFFER_DESC desc = {0};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(ih.SizeBytes);
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = indices;
    ThrowIfFailed(d3dDevice->CreateBuffer(&desc, &initData, &ibs[j]));
    SetDebugObjectName(ibs[j].Get(), "ModelSDKMESH");
  }

  // Create meshes
  std::vector<MaterialRecordSDKMESH> materials;
  materials.resize(header->NumMaterials);

  std::unique_ptr<Model> model(new Model());
  model->meshes.reserve(header->NumMeshes);

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

      // TODO - auto influences = reinterpret_cast<const UINT*>(meshData + mh.FrameInfluenceOffset);
    }

    auto mesh = std::make_shared<ModelMesh>();
    WCHAR meshName[DXUT::MAX_MESH_NAME];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mh.Name, -1, meshName, DXUT::MAX_MESH_NAME);
    mesh->name = meshName;
    mesh->ccw = ccw;
    mesh->pmalpha = pmalpha;

    // Extents
    mesh->boundingBox.Center = mh.BoundingBoxCenter;
    mesh->boundingBox.Extents = mh.BoundingBoxExtents;
    BoundingSphere::CreateFromBoundingBox(mesh->boundingSphere, mesh->boundingBox);

    // Create subsets
    mesh->meshParts.reserve(mh.NumSubsets);
    for(UINT j = 0; j < mh.NumSubsets; ++j) {
      auto sIndex = subsets[j];
      if (sIndex >= header->NumTotalSubsets)
        throw std::exception("Invalid mesh found");

      auto& subset = subsetArray[sIndex];

      D3D11_PRIMITIVE_TOPOLOGY primType;
      switch(subset.PrimitiveType) {
        case DXUT::PT_TRIANGLE_LIST:
          primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
          break;
        case DXUT::PT_TRIANGLE_STRIP:
          primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
          break;
        case DXUT::PT_LINE_LIST:
          primType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
          break;
        case DXUT::PT_LINE_STRIP:
          primType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
          break;
        case DXUT::PT_POINT_LIST:
          primType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
          break;
        case DXUT::PT_TRIANGLE_LIST_ADJ:
          primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
          break;
        case DXUT::PT_TRIANGLE_STRIP_ADJ:
          primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
          break;
        case DXUT::PT_LINE_LIST_ADJ:
          primType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
          break;
        case DXUT::PT_LINE_STRIP_ADJ:
          primType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
          break;
        case DXUT::PT_QUAD_PATCH_LIST:
        case DXUT::PT_TRIANGLE_PATCH_LIST:
          throw std::exception("Direct3D9 era tessellation not supported");

        default:
          throw std::exception("Unknown primitive type");
      }

      if (subset.MaterialID >= header->NumMaterials)
        throw std::exception("Invalid mesh found");

      auto& mat = materials[subset.MaterialID];

      if (!mat.effect) {
        size_t vi = mh.VertexBuffers[0];
        LoadMaterial(materialArray[subset.MaterialID],
                      perVertexColor[vi], enableSkinning[vi], enableDualTexture[vi],
                      fxFactory, mat);
      }

      ComPtr<ID3D11InputLayout> il;
      CreateInputLayout(d3dDevice, mat.effect.get(), *vbDecls[mh.VertexBuffers[0]].get(), &il);

      auto part = new ModelMeshPart();
      part->isAlpha = mat.alpha;

      part->indexCount = static_cast<uint32_t>(subset.IndexCount);
      part->startIndex = static_cast<uint32_t>(subset.IndexStart);
      part->vertexOffset = static_cast<uint32_t>(subset.VertexStart);
      part->vertexStride = static_cast<uint32_t>(vbArray[mh.VertexBuffers[0]].StrideBytes);
      part->indexFormat = (ibArray[mh.IndexBuffer].IndexType == DXUT::IT_32BIT) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
      part->primitiveType = primType;
      part->inputLayout = il;
      part->indexBuffer = ibs[mh.IndexBuffer];
      part->vertexBuffer = vbs[mh.VertexBuffers[0]];
      part->effect = mat.effect;
      part->vbDecl = vbDecls[mh.VertexBuffers[0]];

      mesh->meshParts.emplace_back(part);
    }

    model->meshes.emplace_back(mesh);
  }

  return model;
}
