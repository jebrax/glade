#version 410 core

const float ZERO = 0.0, ONE = 1.0;

uniform mat4 uProjectionMatrix;
uniform mat4 uWorldViewMatrix;

uniform float uTexCoordScaleX0;
uniform float uTexCoordScaleY0;
uniform float uTexCoordOffsetX0;
uniform float uTexCoordOffsetY0;

in vec4 aPosition;
in vec3 aNormal;
in vec2 aTexCoord0;

out vec2 vTexCoord0;

vec2 transformTexCoords(vec2 texCoord, vec2 offset, vec2 scale) {
  vec2 result;

  if (scale.x < ZERO) {
    result.x = ONE + texCoord.x * scale.x - offset.x;
  } else {
    result.x = texCoord.x * scale.x + offset.x;
  }

  if (scale.y < ZERO) {
    result.y = ONE + texCoord.y * scale.y - offset.y;
  } else {
    result.y = texCoord.y * scale.y + offset.y;
  }

  return result;
}

void main() {
  gl_Position = uProjectionMatrix * uWorldViewMatrix * aPosition;
  vTexCoord0 = transformTexCoords(aTexCoord0, vec2(uTexCoordOffsetX0, uTexCoordOffsetY0), vec2(uTexCoordScaleX0, uTexCoordScaleY0));
}

