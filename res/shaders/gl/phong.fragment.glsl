#version 410 

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord0;

out vec4 fragColor;

uniform mat4 uCameraTransform;
uniform vec3 uCameraPosition;

vec3 objectColor = vec3(0.4, 0.1, 0.2);

vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 lightPosition = vec3(-14.0, 12.0, -10.0);
float ambientStrength = 0.4;
float specularStrength = 1.0;
float specularShininess = 64;

void main(void)
{
  vec3 normal = normalize(vNormal);

  // Temporarily stick the directional light to the camera
  lightPosition = uCameraPosition;

  // Ambient
  vec3 ambient = ambientStrength * lightColor;

  vec3 lightDir = lightPosition - vFragPos;
  lightDir = normalize(lightDir);

  // Diffuse
  float angleToLight = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = angleToLight * lightColor;

  vec3 viewDir = normalize(uCameraPosition - vFragPos);
  vec3 reflectDir = reflect(-lightDir, normal);

  // Specular
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularShininess);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * objectColor;

  if (vTexCoord0.y > 0.9 || vTexCoord0.x > 0.9) {
    // debug color
    result = vec3(0.1, 0.8, 0.2);
  }

  fragColor = vec4(result, 1.0);
}

