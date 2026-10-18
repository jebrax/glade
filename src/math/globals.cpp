#include <math.h>
#include <stdlib.h>

double DOUBLE_CUBE_DIAGONAL = 1.4422495703074083823216383107801;
float RADIANS_IN_DEGREE = 0.0174532925199444f;
float PI = 3.14159265359f;
float TAU = 6.28318530718f;

bool throw_coin(float probability) { return (::rand() % 100) < ::round(probability * 100); }

float degrees_to_radians(float degrees) { return degrees * ::RADIANS_IN_DEGREE; }

float radians_to_degrees(float radians) { return radians / ::RADIANS_IN_DEGREE; }

float simplify_angle_radians(float angle) {
  return ::fabs(angle) < ::TAU ? angle : ::fmod(angle, ::TAU);
}
