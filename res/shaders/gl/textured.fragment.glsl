#version 410 core

uniform sampler2D uTextureSampler0;

in vec2 vTexCoord0;
out vec4 fragColor;

void main(void)
{
  fragColor = texture(uTextureSampler0, vTexCoord0);
}
