

#define MAX_NUmTOTAL_LIGHTS 10
struct Light {
  vec4 A;
  vec4 B;
  vec4 C;
  vec4 D;
  mat4 lightSpaceMat;
};