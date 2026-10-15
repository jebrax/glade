#version 410

uniform mat4    uProjectionMatrix;
uniform mat4    uWorldViewMatrix;
uniform mat4    uWorldMatrix;

in vec3  aPosition;
out vec3 vTexCoord0;

void main(void) {
  vec4 finalVertexPosition = uWorldViewMatrix * vec4(aPosition, 1.0);
  gl_Position = uProjectionMatrix * finalVertexPosition;
  // Cubemap lookup uses the 3D direction from the cube centre to its vertex.
  vTexCoord0 = aPosition;
}
