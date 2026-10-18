#pragma once

#include <math.h>
#include <string.h>
#include "Vector.h"

// Index of the matrix element
#define GLADE_I(_i, _j) ((_j) + 4 * (_i))
#define GLADE_PI 3.141592653589793

class Matrix {
public:
  static void mx4transform(float x, float y, float z, float w, const float *p_m, float *p_dest) {
    p_dest[0] = p_m[0 + 4 * 0] * x + p_m[0 + 4 * 1] * y + p_m[0 + 4 * 2] * z + p_m[0 + 4 * 3] * w;
    p_dest[1] = p_m[1 + 4 * 0] * x + p_m[1 + 4 * 1] * y + p_m[1 + 4 * 2] * z + p_m[1 + 4 * 3] * w;
    p_dest[2] = p_m[2 + 4 * 0] * x + p_m[2 + 4 * 1] * y + p_m[2 + 4 * 2] * z + p_m[2 + 4 * 3] * w;
    p_dest[3] = p_m[3 + 4 * 0] * x + p_m[3 + 4 * 1] * y + p_m[3 + 4 * 2] * z + p_m[3 + 4 * 3] * w;
  }

  // FIXME: add offsets (just like in the multiplyMM)
  static void multiplyMv(float *r, const float *lhs, const float *rhs) {
    mx4transform(rhs[0], rhs[1], rhs[2], rhs[3], lhs, r);
  }

  static void multiplyMv(Glade::Vector4f &result, const float *lhs, const Glade::Vector4f &rhs) {
    float dest[4] = {0};
    mx4transform(rhs.x, rhs.y, rhs.z, rhs.w, lhs, dest);
    result.x = dest[0];
    result.y = dest[1];
    result.z = dest[2];
    result.w = dest[3];
  }

  static float length(float x, float y, float z) { return (float)::sqrt(x * x + y * y + z * z); }

  static void setIdentityM(float sm[16], int sm_offset) {
    for (int i = 0; i < 16; i++) {
      sm[sm_offset + i] = 0.0f;
    }

    for (int i = 0; i < 16; i += 5) {
      sm[sm_offset + i] = 1.0f;
    }
  }

  static void scaleM(float sm[16], int sm_offset, const float m[16], int m_offset, float x, float y,
                     float z) {
    for (int i = 0; i < 4; i++) {
      int smi = sm_offset + i;
      int mi = m_offset + i;
      sm[smi] = m[mi] * x;
      sm[4 + smi] = m[4 + mi] * y;
      sm[8 + smi] = m[8 + mi] * z;
      sm[12 + smi] = m[12 + mi];
    }
  }

  static void scaleM(float m[16], int m_offset, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
      int mi = m_offset + i;
      m[mi] *= x;
      m[4 + mi] *= y;
      m[8 + mi] *= z;
    }
  }

  static void translateM(float tm[16], int tm_offset, const float m[16], int m_offset, float x,
                         float y, float z) {
    for (int i = 0; i < 12; i++) {
      tm[tm_offset + i] = m[m_offset + i];
    }

    for (int i = 0; i < 4; i++) {
      int tmi = tm_offset + i;
      int mi = m_offset + i;
      tm[12 + tmi] = m[mi] * x + m[4 + mi] * y + m[8 + mi] * z + m[12 + mi];
    }
  }

  static void translateM(float m[16], int m_offset, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
      int mi = m_offset + i;
      m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
  }

  static void rotateM(float rm[16], int rm_offset, const float m[16], int m_offset, float a,
                      float x, float y, float z) {
    float r[16];
    setRotateM(r, 0, a, x, y, z);
    multiplyMm(rm, rm_offset, m, m_offset, r, 0);
  }

  static void rotateM(float m[16], int m_offset, float a, float x, float y, float z) {
    float temp[32];
    setRotateM(temp, 0, a, x, y, z);
    multiplyMm(temp, 16, m, m_offset, temp, 0);
    ::memcpy(m + m_offset, temp + 16,
             16 * sizeof(float)); // было System.arraycopy(temp, 16, m, mOffset, 16);
  }

  static void multiplyMm(float *r, int r_offset, const float *lhs, int lhs_offset, const float *rhs,
                         int rhs_offset);

  static void transposeM(float m_trans[16], int m_trans_offset, const float m[16], int m_offset);
  static bool invertM(float m_inv[16], int m_inv_offset, const float m[16], int m_offset);

  static void orthoM(float m[16], int m_offset, float left, float right, float bottom, float top,
                     float near, float far);

  static void frustumM(float m[16], int offset, float left, float right, float bottom, float top,
                       float near, float far);

  static void perspectiveM(float m[16], int offset, float fovy, float aspect, float z_near,
                           float z_far);

  static void setRotateM(float rm[16], int rm_offset, float a, float x, float y, float z);
  static void setRotateEulerM(float rm[16], int rm_offset, float x, float y, float z);
};
