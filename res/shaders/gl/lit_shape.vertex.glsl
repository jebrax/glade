#version 410

const float ZERO = 0.0, ONE = 1.0;

uniform mat4    uProjectionMatrix;
uniform mat4    uWorldViewMatrix;

uniform vec3    uLightDirection;
uniform vec3    uCameraPosition;
uniform vec4    uLightAmbient;
uniform vec4    uLightDiffuse;
uniform vec4    uLightSpecular;
uniform vec4    uMaterialAmbient;
uniform vec4    uMaterialDiffuse;
uniform vec4    uMaterialSpecular;
uniform float   uMaterialShininess;

in vec3  aPosition;
in vec3  aNormal;
in vec2  aTexCoord0;

out vec4 vColor;

vec4 calculateLight(vec3 normal, vec3 vertex_pos) {
  vec4 computedColor = vec4(ZERO, ZERO, ZERO, ZERO);
  float ndotl;
  float ndoth;

  // normalized vertex pos is the look vector from camera to the vertex
  vertex_pos = normalize(vertex_pos);

  vec3 halfplane = vec3(
    uLightDirection.x - vertex_pos.x,
    uLightDirection.y - vertex_pos.y,
    uLightDirection.z - vertex_pos.z
  );

  halfplane = normalize(halfplane);

  ndotl = max(ZERO, dot(normal, uLightDirection));
  ndoth = max(ZERO, dot(normal, halfplane));
  computedColor += (uLightAmbient * uMaterialAmbient);
  computedColor += (ndotl * uLightDiffuse * uMaterialDiffuse);

  if (ndoth > ZERO) {
    computedColor += (pow(ndoth, uMaterialShininess) *
    uMaterialSpecular *
    uLightSpecular);
  }

  return computedColor;
}

void main(void) {
  vec4 finalVertexPosition = uWorldViewMatrix * vec4(aPosition, 1.0);
  gl_Position = uProjectionMatrix * finalVertexPosition;

  vec4 rotatedNormal = normalize(uWorldViewMatrix * vec4(aNormal.xyz, ZERO));
  vColor = calculateLight(rotatedNormal.xyz, finalVertexPosition.xyz);
}

