#version 410

uniform mat4    uProjectionMatrix;
uniform mat4    uWorldViewMatrix;
uniform mat4    uWorldMatrix;

in vec3  aTexCoord0;
out vec3 vTexCoord0;

void main(void) {
  vec4 finalVertexPosition = uWorldViewMatrix * vec4(aPosition, 1.0);
  gl_Position = uProjectionMatrix * finalVertexPosition;
  vTexCoord0 = aTexCoord0;
}
