/** math3d.hpp
 *  Contains declarations for 3D math operations.
 *  3D math operations are intended to be used with a right-handed coordinate system
 *  where +x is right, +y is up, and -z is forward.
 */

#pragma once

#include <math.h>
#ifndef PI_F
#  define PI_F     (float)(3.14159274101257324F)
#endif

#define CLAMP(value, min, max) ((value) < (min) ? (min) : (value) > (max) ? (max) : (value))
#define DEG2RAD(deg)    ((deg * PI_F) / 180.0f)
#define RAD2DEG(rad)    ((rad * 180.0f) / PI_F)

/* \brief A structure that defines a 2-dimensional vector. */
typedef struct vec2 vec2;
/* \brief A structure that defines a 3-dimensional vector. */
typedef struct vec3 vec3;
/* \brief A structure that defines a 4-dimensional vector. */
typedef struct vec4 vec4;

/* \brief A structure that defines a 4x4 matrix. */
typedef struct mat4 mat4;

/* \brief A structure that defines a quaternion. */
typedef struct quaternion quaternion;

struct vec2
{
    union
    {
        struct { float v[2]; };
        struct { float x, y; };
    };
    static vec2 zero();
    static vec2 one();
    static vec2 right();
    static vec2 left();
    static vec2 up();
    static vec2 down();
    vec2();
    vec2(float x, float y);
    float dot(const vec2 &other) const;
    float magnitude() const;
    vec2 normalized() const;
    vec2 operator+(const vec2 &other) const;
    vec2 operator-(const vec2 &other) const;
    vec2 operator*(float scalar) const;
    vec2 operator/(float scalar) const;
    bool operator==(const vec2 &other) const;
    bool operator!=(const vec2 &other) const;
    vec2 transform(const mat4 &m) const;
    vec2& operator+=(const vec2 &other);
    vec2& operator-=(const vec2 &other);
    vec2& operator*=(float scalar);
    vec2& operator/=(float scalar);
};

struct vec3
{
    union
    {
        struct { float v[3]; };
        struct { float x, y, z; };
    };
    static vec3 zero();
    static vec3 one();
    static vec3 forward();
    static vec3 backward();
    static vec3 right();
    static vec3 left();
    static vec3 up();
    static vec3 down();
    vec3();
    vec3(float x, float y, float z);
    vec3(const vec2 &v, float z);
    void normalize();
    float dot(const vec3 &other) const;
    float magnitude() const;
    vec2 xy() const;
    vec3 normalized() const;
    bool operator==(const vec3 &other) const;
    bool operator!=(const vec3 &other) const;
    vec3 operator+(const vec3 &other) const;
    vec3 operator-(const vec3 &other) const;
    vec3 operator*(float scalar) const;
    vec3 operator/(float scalar) const;
    vec3 transform(const mat4 &m) const;
    vec3 cross(const vec3 &other) const;
    vec3& operator+=(const vec3 &other);
    vec3& operator-=(const vec3 &other);
    vec3& operator*=(float scalar);
    vec3& operator/=(float scalar);
};

struct vec4
{
    union
    {
        struct { float v[4]; };
        struct {float x, y, z, w; };
    };
    static vec4 zero();
    static vec4 one();
    static vec4 up();
    static vec4 down();
    static vec4 right();
    static vec4 left();
    static vec4 forward();
    static vec4 backward();
    vec4();
    vec4(float x, float y, float z, float w);
    vec4(const vec3 &v, float w);
    void normalize();
    float dot(const vec4 &other) const;
    float magnitude() const;
    vec4 normalized() const;
    vec3 xyz() const;
    bool operator==(const vec4 &other) const;
    bool operator!=(const vec4 &other) const;
    vec4 operator+(const vec4 &other) const;
    vec4 operator-(const vec4 &other) const;
    vec4 operator*(float scalar) const;
    vec4 operator/(float scalar) const;
    vec4 transform(const mat4 &m) const;
    vec4& operator+=(const vec4 &other);
    vec4& operator-=(const vec4 &other);
    vec4& operator*=(float scalar);
    vec4& operator/=(float scalar);
};

struct quaternion
{
    union
    { 
        struct { float q[4]; };
        struct { float w, i, j, k; };
    };
    static quaternion zero();
    static quaternion identity();
    static quaternion euler_angles(const vec3 &euler);
    static quaternion euler_angles(float pitch, float yaw, float roll);
    static quaternion angle_axis(float angle, const vec3 &axis);
    quaternion();
    quaternion(float w, float i, float j, float k);
    quaternion(const vec3 &v, float w);
    float magnitude() const;
    quaternion normalized() const;
    quaternion multiply(const quaternion &other) const;
    quaternion inverse() const;
    mat4 rotation() const;
    bool operator==(const quaternion &other) const;
    bool operator!=(const quaternion &other) const;
};

struct mat4
{
    union
    {
        struct { float m[16]; };
        struct
        {
            float m11, m12, m13, m14;
            float m21, m22, m23, m24;
            float m31, m32, m33, m34;
            float m41, m42, m43, m44;
        };
    };
    static mat4 zero();
    static mat4 identity();
    static mat4 look_at(const vec3 &eye, const vec3 &target, const vec3 &up);
    static mat4 perspective(float fov, float aspect, float near, float far);
    static mat4 ortho(float left, float right, float bottom, float top, float near, float far);
    static mat4 rotation(float pitch, float yaw, float roll);
    static mat4 rotation(const vec3 euler_angles);
    static mat4 rotation(const quaternion &q);
    static mat4 rotation_x(float angle);
    static mat4 rotation_y(float angle);
    static mat4 rotation_z(float angle);
    static mat4 translation(const vec3 &t);
    static mat4 scale(const vec3 &s);
    mat4();
    mat4(float m11, float m12, float m13, float m14,
               float m21, float m22, float m23, float m24,
               float m31, float m32, float m33, float m34,
               float m41, float m42, float m43, float m44);
    bool operator==(const mat4 &other) const;
    bool operator!=(const mat4 &other) const;
    mat4 operator*(float scalar) const;
    mat4 operator*(const mat4 &other) const;
    mat4 multiply(const mat4 &other) const;
    mat4 transpose() const;
    mat4 inverse() const;
};
