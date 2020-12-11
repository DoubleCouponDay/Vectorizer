#ifndef _OUR_MATH_H
#define _OUR_MATH_H

#include <math.h>
#include <string.h>

typedef struct 
{
    float x, y;
} floaty2;

typedef union 
{
    struct
    {
        float m11, m12, dx, m21, m22, dy;
    };
    struct
    {
        float _11, _12, _13;
        float _21, _22, _23;
    };
    float m[2][3];
} mat2x3;

static inline mat2x3 mat2x3_Identity()
{
    return { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
}

static inline mat2x3 mat2x3_Create(float _11, float _12, float _13, float _21, float _22, float _23)
{
    return { _11, _12, _13, _21, _22, _23 };
}

static inline void mat2x3_Copy(mat2x3 *dst, mat2x3 *src)
{
    memcpy(dst, src, sizeof(mat2x3));
}

static inline mat2x3 mat2x3_Clone(mat2x3 *other)
{
    return {other->_11, other->_12, other->_13, other->_21, other->_22, other->_23};
}

static inline mat2x3 mat2x3_Translation(float x, float y)
{
    return mat2x3_Create(1.f, 0.f, x, 0.f, 1.f, y);
}

static inline mat2x3 mat2x3_Scale(floaty2 scale, floaty2 center = {0.f, 0.f})
{
    mat2x3 scaled;

    scaled._11 = scale.x;
    scaled._12 = 0.0;
    scaled._21 = 0.0;
    scaled._22 = scale.y;
    scaled._13 = center.x - scale.x * center.x;
    scaled._23 = center.y - scale.y * center.y;

    return scaled;
}

static inline mat2x3 mat2x3_RotationR(float angle)
{
    mat2x3 rotation;

    rotation._11 = cosf(angle);
    rotation._12 = sinf(angle);
    rotation._21 = -sinf(angle);
    rotation._22 = cosf(angle);

    return rotation;
}

static inline mat2x3 mat2x3_RotationR(float angle, floaty2 center)
{
    mat2x3 rotation = mat2x3_Translation(-center.x, -center.y);

    // Rotating with matrix like
    //  _              _
    // | cos(θ), sin(θ) |
    // |_-sin(θ),cos(θ)_|

    mat2x3 rot_temp = mat2x3_RotationR(angle);
    rotation = mat2x3_Multiply(&rotation, &rot_temp);

    mat2x3 trans = mat2x3_Translation(center.x, center.y);

    mat2x3 out = mat2x3_Multiply(&rotation, &trans);

    return out;
}

static inline float mat2x3_Determinant(mat2x3 *mat)
{
    return (mat->_11 * mat->_22) - (mat->_12 * mat->_21);
}

static inline bool mat2x3_IsInvertible(mat2x3 *mat)
{
    return mat2x3_Determinant(mat) != 0.f;
}

static inline bool Invert(mat2x3 *mat) noexcept
{
    float det = mat2x3_Determinant(mat);
    if (det == 0.f)
        return false;

    float invdet = 1.f / det;

    mat2x3 minv; // inverse of matrix m
    minv.m[0][0] = (mat->m[1][1] * 1.f - 0.f * mat->m[1][2]) * invdet;
    minv.m[0][1] = (mat->m[0][2] * 0.f - mat->m[0][1] * 1.f) * invdet;
    minv.m[0][2] = (mat->m[0][1] * mat->m[1][2] - mat->m[0][2] * mat->m[1][1]) * invdet;
    minv.m[1][0] = (mat->m[1][2] * 0.f - mat->m[1][0] * 1.f) * invdet;
    minv.m[1][1] = (mat->m[0][0] * 1.f - mat->m[0][2] * 0.f) * invdet;
    minv.m[1][2] = (mat->m[1][0] * mat->m[0][2] - mat->m[0][0] * mat->m[1][2]) * invdet;
    mat2x3_Copy(mat, &minv);

    return true;
}

static inline bool mat2x3_IsIdentity(mat2x3 *mat)
{
    return mat->_11 == 1.f && mat->_12 == 0.f && mat->_21 == 0.f && mat->_22 == 1.f && mat->_13 == 0.f && mat->_23 == 0.f;
}

static inline mat2x3 mat2x3_Multiply(mat2x3 *a, mat2x3 *b)
{
    mat2x3 out;
    out._11 = a->_11 * b->_11 + a->_12 * b->_21;
    out._12 = a->_11 * b->_12 + a->_12 * b->_22;
    out._21 = a->_21 * b->_11 + a->_22 * b->_21;
    out._22 = a->_21 * b->_12 + a->_22 * b->_22;
    out._13 = a->_13 * b->_11 + a->_23 * b->_21 + b->_13;
    out._23 = a->_13 * b->_12 + a->_23 * b->_22 + b->_23;
    return out;
}

/*inline void SetProduct(const mat2x3 &a, const mat2x3 &b) noexcept
{
    _11 = a._11 * b._11 + a._12 * b._21;
    _12 = a._11 * b._12 + a._12 * b._22;
    _21 = a._21 * b._11 + a._22 * b._21;
    _22 = a._21 * b._12 + a._22 * b._22;
    _13 = a._13 * b._11 + a._23 * b._21 + b._13;
    _23 = a._13 * b._12 + a._23 * b._22 + b._23;
}

inline mat2x3 &operator*=(const mat2x3 &other)
{
    mat2x3 copy = *a;
    _11 = copy._11 * b->_11 + copy._12 * b->_21;
    _12 = copy._11 * b->_12 + copy._12 * b->_22;
    _21 = copy._21 * b->_11 + copy._22 * b->_21;
    _22 = copy._21 * b->_12 + copy._22 * b->_22;
    _13 = copy._13 * b->_11 + copy._23 * b->_21 + b->_13;
    _23 = copy._13 * b->_12 + copy._23 * b->_22 + b->_23;
    return *a;
}

inline floaty2 TransformPoint(floaty2 point) const
{
    floaty2 result =
        {
            point.x * m11 + point.y * m21 + dx,
            point.x * m12 + point.y * m22 + dy};

    return result;
}

inline PointRect TransformRect(PointRect rect)
{
    floaty2 tmpa, tmpb;
    tmpa = TransformPoint({rect.left, rect.top});
    tmpb = TransformPoint({rect.right, rect.bottom});
    return {tmpa.x, tmpa.y, tmpb.x, tmpb.y};
}*/




inline float shifted_data_variance(float *data, int data_len)
{
    if (data_len < 2)
        return 0.f;
    
    float K = data[0];
    int n = 0;
    float Ex = 0.f, Ex2 = 0.f;

    for (int i = 0; i < data_len; ++i)
    {
        ++n;
        Ex += data[i] - K;
        Ex2 += (data[i] - K) * (data[i] - K);
    }
    float variance = (Ex2 - (Ex * Ex) / (float)n) / (float)(n - 1);

    return variance;
}

#endif