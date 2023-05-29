#version 410 

in vec3 vTexCoord0;
out vec4 fragColor;

uniform samplerCube skybox;

void main(void)
{
  fragColor = texture(skybox, vTexCoord0);
}
