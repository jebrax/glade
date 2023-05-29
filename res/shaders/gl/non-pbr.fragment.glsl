#version 410 

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord0;

out vec4 fragColor;

uniform mat4 uCameraTransform;
uniform vec3 uCameraPosition;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 lightPosition = vec3(-14.0, 12.0, -10.0);
float ambientStrength = 0.2;
float specularStrength = 1.0;
float specularShininess = 96;

void main(void)
{
  // Temporarily stick the directional light to the camera
  //lightPosition = uCameraPosition;

  vec3 normal = normalize(vNormal);
  vec3 diffuseTexel = vec3(texture(uDiffuseMap, vTexCoord0));

  // Ambient
  vec3 ambient = ambientStrength * lightColor * diffuseTexel;

  vec3 lightDir = lightPosition - vFragPos;
  lightDir = normalize(lightDir);

  // Diffuse
  float angleToLight = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = angleToLight * lightColor * diffuseTexel;

  vec3 viewDir = normalize(uCameraPosition - vFragPos);
  vec3 reflectDir = reflect(-lightDir, normal);

  // Specular
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularShininess);
  vec3 specular = specularStrength * spec * lightColor * vec3(texture(uSpecularMap, vTexCoord0));

  vec3 result = (ambient + diffuse + specular);

  fragColor = vec4(result, 1.0);
}

