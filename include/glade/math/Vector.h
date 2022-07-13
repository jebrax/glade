#pragma once

#include <cmath>
#include <functional>

// FIXME: make overloaded operators pls

namespace Glade
{

class Vector2i {
public:
  int x, y;
  
  Vector2i(void) { setIdentity(); }
  Vector2i(int x, int y) { this->x = x; this->y = y; }
  void setIdentity(void) { x = y = 0; }
  void set(const Vector2i &vector) { x  = vector.x; y  = vector.y; }
  void set(int x, int y) { this->x = x; this->y = y; }
  void add(const Vector2i &vector) { x += vector.x; y += vector.y; }
  void dot(const Vector2i &vector) { x *= vector.x; y *= vector.y; }
  void add(const Vector2i &vector, Vector2i &result) { result.x = x + vector.x; result.y = y + vector.y; }
  void dot(const Vector2i &vector, Vector2i &result) { result.x = x * vector.x; result.y = y * vector.y; }

  bool operator<(const Vector2i &rhs) const {
    int lhsDistSq = this->x * this->x + this->y * this->y;
    int rhsDistSq = rhs.x * rhs.x + rhs.y * rhs.y;
    return lhsDistSq < rhsDistSq;
  }

  bool operator==(const Vector2i &rhs) const {
    return (x == rhs.x && y == rhs.y);
  }
};

class Vector3i {
public:
  int x, y, z;

  Vector3i(void) { setIdentity(); }
  Vector3i(int x, int y, int z) { this->x = x; this->y = y; this->z = z; }
  void setIdentity(void) { x = y = z = 0; }
  void set(const Vector3i &vector) { x  = vector.x; y  = vector.y; z  = vector.z; }
  void set(int x, int y, int z) { this->x = x; this->y = y; this->z = z; }
  void add(const Vector3i &vector) { x += vector.x; y += vector.y; z += vector.z; }
  void dot(const Vector3i &vector) { x *= vector.x; y *= vector.y; z *= vector.z; }
  void add(const Vector3i &vector, Vector3i &result) { result.x = x + vector.x; result.y = y + vector.y; result.z = z + vector.z; }
  void dot(const Vector3i &vector, Vector3i &result) { result.x = x * vector.x; result.y = y * vector.y; result.z = z * vector.z; }

  bool operator==(const Vector3i &rhs) const {
    return (x == rhs.x && y == rhs.y && z == rhs.z);
  }
};

class Vector2f {
public:
  float x, y;
  
  Vector2f(void) { setIdentity(); }
  Vector2f(float x, float y) { this->x = x; this->y = y; }
  void setIdentity(void) { x = y = 0.0f; }
  void set(const Vector2f &vector) { x  = vector.x; y  = vector.y; }
  void set(float x, float y) { this->x = x; this->y = y; }
  void add(const Vector2f &vector) { x += vector.x; y += vector.y; }
  void dot(const Vector2f &vector) { x *= vector.x; y *= vector.y; }
  void add(const Vector2f &vector, Vector2f &result) { result.x = x + vector.x; result.y = y + vector.y; }
  void dot(const Vector2f &vector, Vector2f &result) { result.x = x * vector.x; result.y = y * vector.y; }
};

class Vector3f {
public:
  float x, y, z;
  
  Vector3f(void) { setIdentity(); }
  Vector3f(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
  void setIdentity(void) { x = y = z = 0.0f; }
  void set(const Vector3f &vector) { x  = vector.x; y  = vector.y; z  = vector.z; }
  void set(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
  void add(const Vector3f &vector) { x += vector.x; y += vector.y; z += vector.z; }
  void subtract(const Vector3f &vector) { x -= vector.x; y -= vector.y; z -= vector.z; }
  void subtract(const Vector3f &vector, Vector3f &result) { result.x = x - vector.x; result.y = y - vector.y; result.z = z - vector.z; }
  void dot(const Vector3f &vector) { x *= vector.x; y *= vector.y; z *= vector.z; }
  void add(const Vector3f &vector, Vector3f &result) { result.x = x + vector.x; result.y = y + vector.y; result.z = z + vector.z; }
  void dot(const Vector3f &vector, Vector3f &result) { result.x = x * vector.x; result.y = y * vector.y; result.z = z * vector.z; }
  float magnitude() { return ::sqrt(x*x + y*y + z*z); }
  
  void cross(const Vector3f &vector, Vector3f &result)
  {
    result.x = y * vector.z - z * vector.y;
    result.y = z * vector.x - x * vector.z;
    result.z = x * vector.y - y * vector.x;
  }

  void normalize()
  {
    float mag = magnitude();

    if (mag != 0) {
      x /= mag;
      y /= mag;
      z /= mag;
    } else {
      x = y = z = 0.0f;
    }
  }
};

class Vector4f {
public:
  float x, y, z, w;
  
  Vector4f(void) { setIdentity(); }
  Vector4f(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w;}
  void setIdentity(void) { x = y = z = w = 0.0f; }
  void set(const Vector4f &vector) { x  = vector.x; y  = vector.y; z  = vector.z; w  = vector.w; }
  void set(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; }
  void add(const Vector4f &vector) { x += vector.x; y += vector.y; z += vector.z; w += vector.w; }
  void dot(const Vector4f &vector) { x *= vector.x; y *= vector.y; z *= vector.z; w *= vector.w; }
  void add(const Vector4f &vector, Vector4f &result) { result.x = x + vector.x; result.y = y + vector.y; result.z = z + vector.z; result.w = w + vector.w; }
  void dot(const Vector4f &vector, Vector4f &result) { result.x = x * vector.x; result.y = y * vector.y; result.z = z * vector.z; result.w = w * vector.w; }
};

class Vector4i {
public:
  int x, y, z, w;
  
  Vector4i(void) { setIdentity(); }
  Vector4i(int x, int y, int z, int w) { this->x = x; this->y = y; this->z = z; this->w = w;}
  void setIdentity(void) { x = y = z = w = 0.0f; }
  void set(const Vector4i &vector) { x  = vector.x; y  = vector.y; z  = vector.z; w  = vector.w; }
  void set(int x, int y, int z, int w) { this->x = x; this->y = y; this->z = z; this->w = w; }
  void add(const Vector4i &vector) { x += vector.x; y += vector.y; z += vector.z; w += vector.w; }
  void dot(const Vector4i &vector) { x *= vector.x; y *= vector.y; z *= vector.z; w *= vector.w; }
  void add(const Vector4i &vector, Vector4i &result) { result.x = x + vector.x; result.y = y + vector.y; result.z = z + vector.z; result.w = w + vector.w; }
  void dot(const Vector4i &vector, Vector4i &result) { result.x = x * vector.x; result.y = y * vector.y; result.z = z * vector.z; result.w = w * vector.w; }
};

} // namespace Glade

namespace std {
  template <>
  struct hash<Glade::Vector3i>
  {
    size_t operator()(const Glade::Vector3i& key) const
    {
      //using std::hash;
      size_t m = 283945;
      size_t x0 = 0x123456;
      size_t x1 = (std::hash<int>()(key.x) ^ x0) * m;
      size_t x2 = (std::hash<int>()(key.y) ^ x1) * m;
      size_t x3 = (std::hash<int>()(key.z) ^ x2) * m;
      return x3;
    }
  };

  template <>
  struct hash<Glade::Vector2i>
  {
    size_t operator()(const Glade::Vector2i& key) const
    {
      //using std::hash;
      size_t m = 283945;
      size_t x0 = 0x123456;
      size_t x1 = (std::hash<int>()(key.x) ^ x0) * m;
      size_t x2 = (std::hash<int>()(key.y) ^ x1) * m;
      return x2;
    }
  };
}

