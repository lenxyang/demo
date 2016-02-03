#pragma pack_matrix(row_major)

#define PT_EMITTER 0
#define PT_FLARE 1

struct Partice {
  float3 initpos  : POSITION;
  float3 velocity : VELOCITY;
  float2 size     : SIZE;
  float  age      : AGE;
  float  type     : TYPE;
};
