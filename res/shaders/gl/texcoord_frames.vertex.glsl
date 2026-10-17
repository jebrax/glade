#version 410 core

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

// XY scales the shared [-1,1] rectangle; ZW offsets it around the pivot.
// Ordinary textured drawables use (1,1,0,0).
uniform vec4 uSpriteGeometry;

void main() {
  vec4 position = aPosition;
  position.xy = position.xy * uSpriteGeometry.xy + uSpriteGeometry.zw;
  gl_Position = uProjectionMatrix * uWorldViewMatrix * position;
  vTexCoord0 = aTexCoord0 * vec2(uTexCoordScaleX0, uTexCoordScaleY0) + vec2(uTexCoordOffsetX0, uTexCoordOffsetY0);
}

