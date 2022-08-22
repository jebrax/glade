#version 410

const float ZERO = 0.0, ONE = 1.0;

uniform mat4    uProjectionMatrix;
uniform mat4    uWorldViewMatrix;
uniform mat4    uWorldMatrix;

in vec3  aPosition;
in vec3  aNormal;
in vec2  aTexCoord0;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoord0;

void main(void) {
  vec4 finalVertexPosition = uWorldViewMatrix * vec4(aPosition, 1.0);
  gl_Position = uProjectionMatrix * finalVertexPosition;
  // Position of the fragment in the world space
  vFragPos = (uWorldMatrix * vec4(aPosition, 1.0)).xyz;

  // TODO normal matrix should be precalculated on CPU
  mat3 normalMatrix = mat3(transpose(inverse(uWorldMatrix)));
  // Do we need normalize here?

  // The rotated normal of the fragment in the world (does not depend on the viewer position (view matrix))
  vNormal = normalMatrix * aNormal;  
  vTexCoord0 = aTexCoord0;
}

