#pragma pack_matrix(row_major)

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle {
  float3 initpos  : INITPOS;
  float3 velocity : VELOCITY;
  float2 size     : SIZE;
  float  age      : AGE;
  uint   type     : TYPE;
};
