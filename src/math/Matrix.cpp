#include "glade/math/Matrix.h"

void Matrix::multiplyMm(float *r, int r_offset, const float *lhs, int lhs_offset, const float *rhs,
                        int rhs_offset) {
  for (int i = 0; i < 4; i++) {
    const float rhs_i0 = rhs[GLADE_I(i, 0) + rhs_offset];
    float ri0 = lhs[GLADE_I(0, 0) + lhs_offset] * rhs_i0;
    float ri1 = lhs[GLADE_I(0, 1) + lhs_offset] * rhs_i0;
    float ri2 = lhs[GLADE_I(0, 2) + lhs_offset] * rhs_i0;
    float ri3 = lhs[GLADE_I(0, 3) + lhs_offset] * rhs_i0;

    for (int j = 1; j < 4; j++) {
      const float rhs_ij = rhs[GLADE_I(i, j) + rhs_offset];
      ri0 += lhs[GLADE_I(j, 0) + lhs_offset] * rhs_ij;
      ri1 += lhs[GLADE_I(j, 1) + lhs_offset] * rhs_ij;
      ri2 += lhs[GLADE_I(j, 2) + lhs_offset] * rhs_ij;
      ri3 += lhs[GLADE_I(j, 3) + lhs_offset] * rhs_ij;
    }

    r[GLADE_I(i, 0) + r_offset] = ri0;
    r[GLADE_I(i, 1) + r_offset] = ri1;
    r[GLADE_I(i, 2) + r_offset] = ri2;
    r[GLADE_I(i, 3) + r_offset] = ri3;
  }
}

void Matrix::transposeM(float m_trans[16], int m_trans_offset, const float m[16], int m_offset) {
  int m_base;

  for (int i = 0; i < 4; i++) {
    m_base = i * 4 + m_offset;
    m_trans[i + m_trans_offset] = m[m_base];
    m_trans[i + 4 + m_trans_offset] = m[m_base + 1];
    m_trans[i + 8 + m_trans_offset] = m[m_base + 2];
    m_trans[i + 12 + m_trans_offset] = m[m_base + 3];
  }
}

// Invert a 4 x 4 matrix using Cramer's Rule
bool Matrix::invertM(float m_inv[16], int m_inv_offset, const float m[16], int m_offset) {
  // array of transpose source matrix
  float src[16];

  // transpose matrix
  transposeM(src, 0, m, m_offset);

  // temp array for pairs
  float tmp[12];

  // calculate pairs for first 8 elements (cofactors)
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];

  // Holds the destination matrix while we're building it up.
  float dst[16];

  // calculate first 8 elements (cofactors)
  dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
  dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
  dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
  dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
  dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
  dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
  dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
  dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
  dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
  dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
  dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
  dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
  dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
  dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
  dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
  dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

  // calculate pairs for second 8 elements (cofactors)
  tmp[0] = src[2] * src[7];
  tmp[1] = src[3] * src[6];
  tmp[2] = src[1] * src[7];
  tmp[3] = src[3] * src[5];
  tmp[4] = src[1] * src[6];
  tmp[5] = src[2] * src[5];
  tmp[6] = src[0] * src[7];
  tmp[7] = src[3] * src[4];
  tmp[8] = src[0] * src[6];
  tmp[9] = src[2] * src[4];
  tmp[10] = src[0] * src[5];
  tmp[11] = src[1] * src[4];

  // calculate second 8 elements (cofactors)
  dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
  dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
  dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
  dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
  dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
  dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
  dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
  dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
  dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
  dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
  dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
  dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
  dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
  dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
  dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
  dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

  // calculate determinant
  float det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

  if (det == 0.0f) {
    // матрица вырожденная, и для нее не существует обратной
    return false;
  }

  // calculate matrix inverse
  det = 1.0f / det;

  for (int j = 0; j < 16; j++) {
    m_inv[j + m_inv_offset] = dst[j] * det;
  }

  return true;
}

void Matrix::orthoM(float m[16], int m_offset, float left, float right, float bottom, float top,
                    float near, float far) {
  if (left == right) {
    // throw new IllegalArgumentException("left == right"); FIXME
  }

  if (bottom == top) {
    // throw new IllegalArgumentException("bottom == top"); FIXME
  }

  if (near == far) {
    // throw new IllegalArgumentException("near == far"); FIXME
  }

  const float r_width = 1.0f / (right - left);
  const float r_height = 1.0f / (top - bottom);
  const float r_depth = 1.0f / (far - near);
  const float x = 2.0f * (r_width);
  const float y = 2.0f * (r_height);
  const float z = -2.0f * (r_depth);
  const float tx = -(right + left) * r_width;
  const float ty = -(top + bottom) * r_height;
  const float tz = -(far + near) * r_depth;

  m[m_offset + 0] = x;
  m[m_offset + 5] = y;
  m[m_offset + 10] = z;
  m[m_offset + 12] = tx;
  m[m_offset + 13] = ty;
  m[m_offset + 14] = tz;
  m[m_offset + 15] = 1.0f;
  m[m_offset + 1] = 0.0f;
  m[m_offset + 2] = 0.0f;
  m[m_offset + 3] = 0.0f;
  m[m_offset + 4] = 0.0f;
  m[m_offset + 6] = 0.0f;
  m[m_offset + 7] = 0.0f;
  m[m_offset + 8] = 0.0f;
  m[m_offset + 9] = 0.0f;
  m[m_offset + 11] = 0.0f;
}

void frustum_m(float m[16], int offset, float left, float right, float bottom, float top,
               float near, float far) {
  if (left == right) {
    // throw new IllegalArgumentException("left == right"); FIXME
  }

  if (top == bottom) {
    // throw new IllegalArgumentException("top == bottom"); FIXME
  }

  if (near == far) {
    // throw new IllegalArgumentException("near == far"); FIXME
  }

  if (near <= 0.0f) {
    // throw new IllegalArgumentException("near <= 0.0f"); FIXME
  }

  if (far <= 0.0f) {
    // throw new IllegalArgumentException("far <= 0.0f"); FIXME
  }

  const float r_width = 1.0f / (right - left);
  const float r_height = 1.0f / (top - bottom);
  const float r_depth = 1.0f / (near - far);
  const float x = 2.0f * (near * r_width);
  const float y = 2.0f * (near * r_height);
  const float A = 2.0f * ((right + left) * r_width);
  const float B = (top + bottom) * r_height;
  const float C = (far + near) * r_depth;
  const float D = 2.0f * (far * near * r_depth);

  m[offset + 0] = x;
  m[offset + 5] = y;
  m[offset + 8] = A;
  m[offset + 9] = B;
  m[offset + 10] = C;
  m[offset + 14] = D;
  m[offset + 11] = -1.0f;
  m[offset + 1] = 0.0f;
  m[offset + 2] = 0.0f;
  m[offset + 3] = 0.0f;
  m[offset + 4] = 0.0f;
  m[offset + 6] = 0.0f;
  m[offset + 7] = 0.0f;
  m[offset + 12] = 0.0f;
  m[offset + 13] = 0.0f;
  m[offset + 15] = 0.0f;
}

void Matrix::perspectiveM(float m[16], int offset, float fovy, float aspect, float z_near,
                          float z_far) {
  float f = 1.0f / (float)::tan(fovy * (GLADE_PI / 360.0));
  float range_reciprocal = 1.0f / (z_near - z_far);

  m[offset + 0] = f / aspect;
  m[offset + 1] = 0.0f;
  m[offset + 2] = 0.0f;
  m[offset + 3] = 0.0f;

  m[offset + 4] = 0.0f;
  m[offset + 5] = f;
  m[offset + 6] = 0.0f;
  m[offset + 7] = 0.0f;

  m[offset + 8] = 0.0f;
  m[offset + 9] = 0.0f;
  m[offset + 10] = (z_far + z_near) * range_reciprocal;
  m[offset + 11] = -1.0f;

  m[offset + 12] = 0.0f;
  m[offset + 13] = 0.0f;
  m[offset + 14] = 2.0f * z_far * z_near * range_reciprocal;
  m[offset + 15] = 0.0f;
}

void Matrix::setRotateM(float rm[16], int rm_offset, float a, float x, float y, float z) {
  rm[rm_offset + 3] = 0;
  rm[rm_offset + 7] = 0;
  rm[rm_offset + 11] = 0;
  rm[rm_offset + 12] = 0;
  rm[rm_offset + 13] = 0;
  rm[rm_offset + 14] = 0;
  rm[rm_offset + 15] = 1;
  a *= (float)(GLADE_PI / 180.0f);
  float s = (float)::sin(a);
  float c = (float)::cos(a);

  if (1.0f == x && 0.0f == y && 0.0f == z) {
    rm[rm_offset + 5] = c;
    rm[rm_offset + 10] = c;
    rm[rm_offset + 6] = s;
    rm[rm_offset + 9] = -s;
    rm[rm_offset + 1] = 0;
    rm[rm_offset + 2] = 0;
    rm[rm_offset + 4] = 0;
    rm[rm_offset + 8] = 0;
    rm[rm_offset + 0] = 1;
  } else if (0.0f == x && 1.0f == y && 0.0f == z) {
    rm[rm_offset + 0] = c;
    rm[rm_offset + 10] = c;
    rm[rm_offset + 8] = s;
    rm[rm_offset + 2] = -s;
    rm[rm_offset + 1] = 0;
    rm[rm_offset + 4] = 0;
    rm[rm_offset + 6] = 0;
    rm[rm_offset + 9] = 0;
    rm[rm_offset + 5] = 1;
  } else if (0.0f == x && 0.0f == y && 1.0f == z) {
    rm[rm_offset + 0] = c;
    rm[rm_offset + 5] = c;
    rm[rm_offset + 1] = s;
    rm[rm_offset + 4] = -s;
    rm[rm_offset + 2] = 0;
    rm[rm_offset + 6] = 0;
    rm[rm_offset + 8] = 0;
    rm[rm_offset + 9] = 0;
    rm[rm_offset + 10] = 1;
  } else {
    float len = length(x, y, z);

    if (1.0f != len) {
      float recip_len = 1.0f / len;
      x *= recip_len;
      y *= recip_len;
      z *= recip_len;
    }

    float nc = 1.0f - c;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;

    rm[rm_offset + 0] = x * x * nc + c;
    rm[rm_offset + 4] = xy * nc - zs;
    rm[rm_offset + 8] = zx * nc + ys;
    rm[rm_offset + 1] = xy * nc + zs;
    rm[rm_offset + 5] = y * y * nc + c;
    rm[rm_offset + 9] = yz * nc - xs;
    rm[rm_offset + 2] = zx * nc - ys;
    rm[rm_offset + 6] = yz * nc + xs;
    rm[rm_offset + 10] = z * z * nc + c;
  }
}

void Matrix::setRotateEulerM(float rm[16], int rm_offset, float x, float y, float z) {
  x *= (float)(GLADE_PI / 180.0f);
  y *= (float)(GLADE_PI / 180.0f);
  z *= (float)(GLADE_PI / 180.0f);
  float cx = (float)::cos(x);
  float sx = (float)::sin(x);
  float cy = (float)::cos(y);
  float sy = (float)::sin(y);
  float cz = (float)::cos(z);
  float sz = (float)::sin(z);
  float cxsy = cx * sy;
  float sxsy = sx * sy;

  rm[rm_offset + 0] = cy * cz;
  rm[rm_offset + 1] = -cy * sz;
  rm[rm_offset + 2] = sy;
  rm[rm_offset + 3] = 0.0f;

  rm[rm_offset + 4] = cxsy * cz + cx * sz;
  rm[rm_offset + 5] = -cxsy * sz + cx * cz;
  rm[rm_offset + 6] = -sx * cy;
  rm[rm_offset + 7] = 0.0f;

  rm[rm_offset + 8] = -sxsy * cz + sx * sz;
  rm[rm_offset + 9] = sxsy * sz + sx * cz;
  rm[rm_offset + 10] = cx * cy;
  rm[rm_offset + 11] = 0.0f;

  rm[rm_offset + 12] = 0.0f;
  rm[rm_offset + 13] = 0.0f;
  rm[rm_offset + 14] = 0.0f;
  rm[rm_offset + 15] = 1.0f;
}
