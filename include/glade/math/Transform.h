#pragma once

#include <cmath>
#include <memory>

#include <glade/math/util.h>
#include "Vector.h"
#include "Matrix.h"

class Transform {
public:
  typedef std::shared_ptr<Glade::Vector3f> SharedVector;
  
  SharedVector position;
  SharedVector rotation;
  SharedVector scale;
  bool thirdPerson;

private:
  char invertor;
  
public:
  Transform(void):
    invertor(1),
    thirdPerson(false),
    position(new Glade::Vector3f()),
    rotation(new Glade::Vector3f()),
    scale(new Glade::Vector3f(1, 1, 1))
  {
  }

  Transform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float scaleX, float scaleY, float scaleZ) :
    invertor(1),
    thirdPerson(false),
    position(new Glade::Vector3f(posX, posY, posZ)),
    rotation(new Glade::Vector3f(rotX, rotY, rotZ)),
    scale(new Glade::Vector3f(scaleX, scaleY, scaleZ))
  {
  }

  SharedVector getScale   (void) const { return scale;    }
  SharedVector getPosition(void) const { return position; }
  SharedVector getRotation(void) const { return rotation; }
  
  void setPosition(float x, float y, float z) { position->set(x, y, z); }
  void setRotation(float x, float y, float z) { rotation->set(x, y, z); }
  void setScale   (float x, float y, float z) { scale   ->set(x, y, z); }
  
  void setPosition(const SharedVector &position) { this->position = position; }
  void setRotation(const SharedVector &rotation) { this->rotation = rotation; }
  void setScale   (const SharedVector &scale   ) { this->scale    = scale;    }
  
  void set(Transform &transform)
  {
    SharedVector position = transform.getPosition();
    SharedVector rotation = transform.getRotation();
    SharedVector scale    = transform.getScale();
    
    setPosition(position->x, position->y, position->z);
    setRotation(rotation->x, rotation->y, rotation->z);
    setScale   (scale->x   , scale->y   , scale->z   );
  }
  
  void invertTranslation(bool invert) { invertor = invert ? -1 : 1; }
  bool isInvertedTranslation(void) { return invertor < 0; }
    
  void combineWith(const Transform &relativeTransform, Transform &resultTransform) {
    return Transform::combine(*this, relativeTransform, false, false, false, resultTransform);
  }
  
  void combineWith(const Transform &relativeTransform, bool preservePosition, bool preserveRotation, bool preserveScale, Transform &resultTransform) {
    return Transform::combine(*this, relativeTransform, preservePosition, preserveRotation, preserveScale, resultTransform);
  }
  
  /**
   * Комбинирует два трансформа так, что их переносы и повороты складываются, а скейлы перемножаются
   */
  static void combine(const Transform &baseTransform, const Transform &relativeTransform, bool preservePosition, bool preserveRotation, bool preserveScale, Transform &resultTransform) {
    if (preservePosition) {
      resultTransform.getPosition()->set(*relativeTransform.getPosition());
    } else {
      baseTransform.getPosition()->add(*relativeTransform.getPosition(), *resultTransform.getPosition());
    }
    
    if (preserveRotation) {
      resultTransform.getRotation()->set(*relativeTransform.getRotation());
    } else {
      baseTransform.getRotation()->add(*relativeTransform.getRotation(), *resultTransform.getRotation());
    }
    
    if (preserveScale) {
      resultTransform.getScale()->set(*relativeTransform.getScale());
    } else {
      // ..
    }
  }
  
  /**
   * Возвращает матрицу трансформации в пространстве
   */
  void getMatrix(float* result) {
    Matrix::setIdentityM(result, 0);
    Matrix::translateM(result, 0, invertor * position->x, invertor * position->y, invertor * position->z);

    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->x), 1, 0, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->y), 0, 1, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->z), 0, 0, 1);

    Matrix::scaleM(result, 0, scale->x, scale->y, scale->z);
  }

  /**
   * Возвращает матрицу трансформации в пространстве для камеры (этой матрицей нужно трансформировать видимые объекты)
   */
  void getCameraMatrix(float* result) {
    Matrix::setIdentityM(result, 0);
    Matrix::scaleM(result, 0, scale->x, scale->y, scale->z);

    if (thirdPerson) {
      Matrix::translateM(result, 0, 0, 0, -invertor * 15);
    }

    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->y), 0, 1, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->x), ::cos(rotation->y), 0, ::sin(rotation->y));
    Matrix::translateM(result, 0, -invertor * position->x, -invertor * position->y, -invertor * position->z);
  }
};
