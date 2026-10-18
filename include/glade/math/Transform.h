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
  Transform(void)
      : invertor(1), thirdPerson(false), position(new Glade::Vector3f()),
        rotation(new Glade::Vector3f()), scale(new Glade::Vector3f(1, 1, 1)) {}

  Transform(float pos_x, float pos_y, float pos_z, float rot_x, float rot_y, float rot_z,
            float scale_x, float scale_y, float scale_z)
      : invertor(1), thirdPerson(false), position(new Glade::Vector3f(pos_x, pos_y, pos_z)),
        rotation(new Glade::Vector3f(rot_x, rot_y, rot_z)),
        scale(new Glade::Vector3f(scale_x, scale_y, scale_z)) {}

  void reset() {
    invertor = 1;
    thirdPerson = false;
    position->x = 0.0;
    position->y = 0.0;
    position->z = 0.0;
    rotation->x = 0.0;
    rotation->y = 0.0;
    rotation->z = 0.0;
    scale->x = 1.0;
    scale->y = 1.0;
    scale->z = 1.0;
  }

  SharedVector getScale(void) const { return scale; }
  SharedVector getPosition(void) const { return position; }
  SharedVector getRotation(void) const { return rotation; }
  void setPosition(float x, float y, float z) { position->set(x, y, z); }
  void setRotation(float x, float y, float z) { rotation->set(x, y, z); }
  void setScale(float x, float y, float z) { scale->set(x, y, z); }
  void setPosition(const SharedVector &position_) { this->position = position_; }
  void setRotation(const SharedVector &rotation_) { this->rotation = rotation_; }
  void setScale(const SharedVector &scale_) { this->scale = scale_; }

  void set(Transform &transform) {
    SharedVector position = transform.getPosition();
    SharedVector rotation = transform.getRotation();
    SharedVector scale = transform.getScale();

    setPosition(position->x, position->y, position->z);
    setRotation(rotation->x, rotation->y, rotation->z);
    setScale(scale->x, scale->y, scale->z);
  }

  void invertTranslation(bool invert) { invertor = invert ? -1 : 1; }
  bool isInvertedTranslation(void) { return invertor < 0; }

  void combineWith(const Transform &relative_transform, Transform &result_transform) {
    return Transform::combine(*this, relative_transform, false, false, false, result_transform);
  }

  void combineWith(const Transform &relative_transform, bool preserve_position,
                   bool preserve_rotation, bool preserve_scale, Transform &result_transform) {
    return Transform::combine(*this, relative_transform, preserve_position, preserve_rotation,
                              preserve_scale, result_transform);
  }

  /**
   * Комбинирует два трансформа так, что их переносы и повороты складываются, а скейлы перемножаются
   */
  static void combine(const Transform &base_transform, const Transform &relative_transform,
                      bool preserve_position, bool preserve_rotation, bool preserve_scale,
                      Transform &result_transform) {
    result_transform.reset();

    if (preserve_position) {
      result_transform.getPosition()->set(*relative_transform.getPosition());
    } else {
      base_transform.getPosition()->add(*relative_transform.getPosition(),
                                        *result_transform.getPosition());
    }

    if (preserve_rotation) {
      result_transform.getRotation()->set(*relative_transform.getRotation());
    } else {
      base_transform.getRotation()->add(*relative_transform.getRotation(),
                                        *result_transform.getRotation());
    }

    if (preserve_scale) {
      result_transform.getScale()->set(*relative_transform.getScale());
    } else {
      result_transform.getScale()->set(*base_transform.getScale());
      result_transform.getScale()->multiply(*relative_transform.getScale());
    }
  }

  /**
   * Возвращает матрицу трансформации в пространстве
   */
  void getMatrix(float *result) {
    Matrix::setIdentityM(result, 0);
    Matrix::translateM(result, 0, invertor * position->x, invertor * position->y,
                       invertor * position->z);

    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->x), 1, 0, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->y), 0, 1, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->z), 0, 0, 1);

    Matrix::scaleM(result, 0, scale->x, scale->y, scale->z);
  }

  /**
   * Возвращает матрицу трансформации в пространстве для камеры (этой матрицей нужно трансформировать видимые объекты)
   */
  void getCameraMatrix(float *result) {
    Matrix::setIdentityM(result, 0);
    Matrix::scaleM(result, 0, scale->x, scale->y, scale->z);

    if (thirdPerson) {
      Matrix::translateM(result, 0, 0, 0, -invertor * 15);
    }

    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->y), 0, 1, 0);
    Matrix::rotateM(result, 0, ::radians_to_degrees(rotation->x), ::cos(rotation->y), 0,
                    ::sin(rotation->y));
    Matrix::translateM(result, 0, -invertor * position->x, -invertor * position->y,
                       -invertor * position->z);
  }
};
