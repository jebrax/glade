#pragma once

extern double DOUBLE_CUBE_DIAGONAL;
extern float RADIANS_IN_DEGREE;
extern float PI;
extern float TAU;

// parameter is the probability of returning true
bool throw_coin(float probability);
float degrees_to_radians(float degrees);
float radians_to_degrees(float radians);
float simplify_angle_radians(float angle);
