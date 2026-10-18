#include <glade/math/Vector.h>
#include <glade/render/Perception.h>

Perception::Perception()
    : baseLightDirection(0.4f, 0.7f, -0.2f), lightDirection(baseLightDirection) {
  /*
  baseLightDirection.normalize();
  lightDirection.set(baseLightDirection);

  setUniform("uLightAmbient",   Glade::Vector4f(0.5f, 0.5f, 0.5f, 1.0f));
  setUniform("uLightDiffuse",   Glade::Vector4f(0.5f, 0.5f, 0.5f, 1.0f));
  setUniform("uLightSpecular",  Glade::Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

  setUniform("uLightDirection", lightDirection);
  */
}

void Perception::adjust() {
  /*
  // rotating the light direction vector
  Glade::Vector4f rotatedVector;
  Glade::Vector4f vector(baseLightDirection.x, baseLightDirection.y, baseLightDirection.z, 0.0f); // maybe w should be 1?

  float matrix[16];
  getTransform()->getCameraMatrix(matrix);
  Matrix::multiplyMV(rotatedVector, matrix, vector);

  lightDirection.set(rotatedVector.x, rotatedVector.y, rotatedVector.z);
  lightDirection.normalize();

  setUniform("uLightDirection", lightDirection);
  */
}
