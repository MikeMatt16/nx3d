#include "math3d.hpp"

vec2 vec2::zero() { return vec2(0.0f, 0.0f); }

vec2 vec2::one() { return vec2(1.0f, 1.0f); }

vec2 vec2::left() { return vec2(-1.0f, 0.0f); }

vec2 vec2::right() { return vec2(1.0f, 0.0f); }

vec2 vec2::up() { return vec2(0.0f, 1.0f); }

vec2 vec2::down() { return vec2(0.0f, -1.0f); }

vec2::vec2() : x(0.0f), y(0.0f) {}

vec2::vec2(float x, float y) : x(x), y(y) { }

float vec2::dot(const vec2 &other) const
{
    return (x * other.x) + (y * other.y);
}

float vec2::magnitude() const
{
    return sqrtf((x * x) + (y * y));
}

vec2 vec2::normalized() const
{
    float m = magnitude();
    if (m == 0.0f) return vec2(0.0f, 0.0f);
    return vec2(x / m, y / m);
}

vec2 vec2::operator+(const vec2 &other) const
{
    return vec2(x + other.x, y + other.y);
}

vec2 vec2::operator-(const vec2 &other) const
{
    return vec2(x - other.x, y - other.y);
}

vec2 vec2::operator*(float scalar) const
{
    return vec2(x * scalar, y * scalar);
}

vec2 vec2::operator/(float scalar) const
{
    return vec2(x / scalar, y / scalar);
}

bool vec2::operator==(const vec2 &other) const
{
    return x == other.x && y == other.y;
}

bool vec2::operator!=(const vec2 &other) const
{
    return x != other.x || y != other.y;
}

vec2 vec2::transform(const mat4 &m) const
{
    return vec2
    (
        (m.m11 * x) + (m.m12 * y) + m.m14,
        (m.m21 * x) + (m.m22 * y) + m.m24
    );
}

vec2 &vec2::operator+=(const vec2 &other)
{
    x += other.x;
    y += other.y;
    return *this;
}

vec2 &vec2::operator-=(const vec2 &other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

vec2 &vec2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

vec2 &vec2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

vec3 vec3::zero() { return vec3(0.0f, 0.0f, 0.0f); }

vec3 vec3::one() { return vec3(1.0f, 1.0f, 1.0f); }

vec3 vec3::forward() { return vec3(0.0f, 0.0f, -1.0f); }

vec3 vec3::backward() { return vec3(0.0f, 0.0f, 1.0f); }

vec3 vec3::right() { return vec3(1.0f, 0.0f, 0.0f); }

vec3 vec3::left() { return vec3(-1.0f, 0.0f, 0.0f); }

vec3 vec3::up() { return vec3(0.0f, 1.0f, 0.0f); }

vec3 vec3::down() { return vec3(0.0f, -1.0f, 0.0f); }

vec3::vec3() : x(0.0f), y(0.0f), z(0.0f) { }

vec3::vec3(float x, float y, float z) : x(x), y(y), z(z) { }

vec3::vec3(const vec2 &v, float z) : x(v.x), y(v.y), z(z) { }

void vec3::normalize()
{
    float m = magnitude();
    if (m == 0)
    {
        x = y = z = 0.0f;
        return;
    }
    x /= m;
    y /= m;
    z /= m;
}

float vec3::dot(const vec3 &other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

float vec3::magnitude() const
{
    return sqrtf((x * x) + (y * y) + (z * z));
}

vec2 vec3::xy() const
{
    return vec2(x, y);
}

vec3 vec3::normalized() const
{
    float m = magnitude();
    if (m == 0.0f) return vec3(0.0f, 0.0f, 0.0f);
    return vec3(x / m, y / m, z / m);
}

vec3 vec3::operator+(const vec3 &other) const
{
    return vec3(x + other.x, y + other.y, z + other.z);
}

vec3 vec3::operator-(const vec3 &other) const
{
    return vec3(x - other.x, y - other.y, z - other.z);
}

vec3 vec3::operator*(float scalar) const
{
    return vec3(x * scalar, y * scalar, z * scalar);
}

vec3 vec3::operator/(float scalar) const
{
    return vec3(x / scalar, y / scalar, z / scalar);
}

bool vec3::operator==(const vec3 &other) const
{
    return
        x == other.x &&
        y == other.y &&
        z == other.z;
}

bool vec3::operator!=(const vec3 &other) const
{
    return
        x != other.x ||
        y != other.y ||
        z != other.z;
}

vec3 vec3::transform(const mat4 &m) const
{
    return vec3
    (
        (m.m11 * x) + (m.m12 * y) + (m.m13 * z) + m.m14,
        (m.m21 * x) + (m.m22 * y) + (m.m23 * z) + m.m24,
        (m.m31 * x) + (m.m32 * y) + (m.m33 * z) + m.m34
    );
}

vec3 vec3::cross(const vec3 &other) const
{
    return vec3
    (
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

vec3 &vec3::operator+=(const vec3 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

vec3 &vec3::operator-=(const vec3 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

vec3 &vec3::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

vec3 &vec3::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

vec4 vec4::zero() { return vec4(0.0f, 0.0f, 0.0f, 0.0f); }

vec4 vec4::one() { return vec4(1.0f, 1.0f, 1.0f, 1.0f); }

vec4 vec4::up() { return vec4(0.0f, 1.0f, 0.0f, 0.0f); }

vec4 vec4::down() { return vec4(0.0f, -1.0f, 0.0f, 0.0f); }

vec4 vec4::right() { return vec4(1.0f, 0.0f, 0.0f, 0.0f); }

vec4 vec4::left() { return vec4(-1.0f, 0.0f, 0.0f, 0.0f); }

vec4 vec4::forward() { return vec4(0.0f, 0.0f, -1.0f, 0.0f); }

vec4 vec4::backward() { return vec4(0.0f, 0.0f, 1.0f, 0.0f); }

vec4::vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }

vec4::vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }

vec4::vec4(const vec3 &v, float w) : x(v.x), y(v.y), z(v.z), w(w) { }

void vec4::normalize()
{
    float m = magnitude();
    if (m == 0.0f)
    {
        x = y = z = w = 0.0f;
        return;
    }
    x /= m;
    y /= m;
    z /= m;
    w /= m;
}

float vec4::dot(const vec4 &other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

float vec4::magnitude() const
{
    return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
}

vec4 vec4::normalized() const
{
    float m = magnitude();
    if (m == 0) return vec4::zero();
    return vec4(x / m, y / m, z / m, w / m);
}

vec3 vec4::xyz() const
{
    return vec3(x, y, z);
}

bool vec4::operator==(const vec4 &other) const
{
    return
        x == other.x &&
        y == other.y &&
        z == other.z &&
        w == other.w;
}

bool vec4::operator!=(const vec4 &other) const
{
    return
        x != other.x ||
        y != other.y ||
        z != other.z ||
        w != other.w;
}

vec4 vec4::operator+(const vec4 &other) const
{
    return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

vec4 vec4::operator-(const vec4 &other) const
{
    return vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

vec4 vec4::operator*(float scalar) const
{
    return vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

vec4 vec4::operator/(float scalar) const
{
    return vec4(x / scalar, y / scalar, z / scalar, w / scalar);
}

vec4 vec4::transform(const mat4 &m) const
{
    return vec4
    (
        (m.m11 * x) + (m.m12 * y) + (m.m13 * z) + (m.m14 * w),
        (m.m21 * x) + (m.m22 * y) + (m.m23 * z) + (m.m24 * w),
        (m.m31 * x) + (m.m32 * y) + (m.m33 * z) + (m.m34 * w),
        (m.m41 * x) + (m.m42 * y) + (m.m43 * z) + (m.m44 * w)
    );
}

vec4 &vec4::operator+=(const vec4 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

vec4 &vec4::operator-=(const vec4 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

vec4 &vec4::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

vec4 &vec4::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

quaternion quaternion::zero() { return quaternion(0.0f, 0.0f, 0.0f, 0.0f); }

quaternion quaternion::identity() { return quaternion(1.0f, 0.0f, 0.0f, 0.0f); }

quaternion quaternion::euler_angles(const vec3 &euler)
{
    return euler_angles(euler.x, euler.y, euler.z);
}

quaternion quaternion::euler_angles(float pitch, float yaw, float roll)
{
    float half_yaw = yaw * 0.5f;
    float half_pitch = pitch * 0.5f;
    float half_roll = roll * 0.5f;
    quaternion qz = quaternion(cosf(half_roll), 0.0f, 0.0f, sinf(half_roll));
    quaternion qy = quaternion(cosf(half_yaw), 0.0f, sinf(half_yaw), 0.0f);
    quaternion qx = quaternion(cosf(half_pitch), sinf(half_pitch), 0.0f, 0.0f);
    return qz.multiply(qy).multiply(qx).normalized();
}

quaternion quaternion::angle_axis(float angle, const vec3 &axis)
{
    // todo: implement angle axis conversion
    return quaternion::identity();
}

quaternion::quaternion() : w(0.0f), i(0.0f), j(0.0f), k(0.0f) { }

quaternion::quaternion(float w, float i, float j, float k) : w(w), i(i), j(j), k(k) { }

quaternion::quaternion(const vec3 &v, float w) : w(w), i(v.x), j(v.y), k(v.z) { }

float quaternion::magnitude() const
{
    return sqrtf(w * w + i * i + j * j + k * k);
}

quaternion quaternion::normalized() const
{
    float m = magnitude();
    if (m == 0.0f) return quaternion::zero();
    return quaternion(w / m, i / m, j / m, k / m);
}

quaternion quaternion::multiply(const quaternion &other) const
{
    return quaternion
    (
        w * other.w - (i * other.i + j * other.j + k * other.k),
        w * other.i + i * other.w + j * other.k - k * other.j,
        w * other.j - i * other.k + j * other.w + k * other.i,
        w * other.k + i * other.j - j * other.i + k * other.w
    );
}

quaternion quaternion::inverse() const
{
    // todo: optimize this by calculating the inverse without normalizing first
    return quaternion::identity();
}

mat4 quaternion::rotation() const
{
    return mat4::rotation(*this);
}

bool quaternion::operator==(const quaternion &other) const
{
    return
        w == other.w &&
        i == other.i &&
        j == other.j &&
        k == other.k;
}

bool quaternion::operator!=(const quaternion &other) const
{
    return
        w != other.w ||
        i != other.i ||
        j != other.j ||
        k != other.k;
}

mat4 mat4::zero() { return mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f); }

mat4 mat4::identity() { return mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f); }

mat4 mat4::look_at(const vec3 &eye, const vec3 &target, const vec3 &up)
{
    vec3 z = (target - eye).normalized();
    vec3 x = up.cross(z).normalized();
    vec3 y = z.cross(x).normalized();
    float dx = x.dot(eye);
    float dy = y.dot(eye);
    float dz = z.dot(eye);
    return mat4
    (
        x.x,  x.y,  x.z,  -dx,
        y.x,  y.y,  y.z,  -dy,
        z.x,  z.y,  z.z,  -dz,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mat4 mat4::perspective(float fov, float aspect, float near, float far)
{
    const float f = 1.0f / tanf(fov * 0.5f);
    const float z_adj = (far + near) / (near - far);
    const float z_adj2 = 2 * far * near / (near - far);
    return mat4
    (
        f / aspect, 0.0f, 0.0f,    0.0f,
        0.0f,       f,    0.0f,    0.0f,
        0.0f,       0.0f, z_adj,  -1.0f,
        0.0f,       0.0f, z_adj2,  0.0f
    );
}

mat4 mat4::ortho(float left, float right, float bottom, float top, float, float)
{
    // todo: implement orthographic projection matrix (assume right handed coordinate system with -z forward)
    return mat4::identity();
}

mat4 mat4::rotation(float pitch, float yaw, float roll)
{
    // todo: optimize this by calculating the combined rotation matrix directly instead of multiplying three separate matrices
    mat4 rx = rotation_x(pitch);
    mat4 ry = rotation_y(yaw);
    mat4 rz = rotation_z(roll);
    return rz * ry * rx;
}

mat4 mat4::rotation(const vec3 euler_angles)
{
    return mat4::rotation(euler_angles.x, euler_angles.y, euler_angles.z);
}

mat4 mat4::rotation(const quaternion &q)
{
    float ii = q.i * q.i;
    float jj = q.j * q.j;
    float kk = q.k * q.k;
    float ij = q.i * q.j;
    float ik = q.i * q.k;
    float iw = q.i * q.w;
    float jk = q.j * q.k;
    float jw = q.j * q.w;
    float kw = q.k * q.w;
    return mat4
    (
        1.0f - (2.0f * jj) - (2.0f * kk),
        (2.0f * ij) - (2.0f * kw),
        (2.0f * ik) + (2.0f * jw),
        0.0f,

        (2.0f * ij) + (2.0f * kw),
        1.0f - (2.0f * ii) - (2.0f * kk),
        (2.0f * jk) - (2.0f * iw),
        0.0f,
    
        (2.0f * ik) - (2.0f * jw),
        (2.0f * jk) + (2.0f * iw),
        1.0f - (2.0f * ii) - (2.0f * jj),
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mat4 mat4::rotation_x(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return mat4
    (
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 1
    );
}

mat4 mat4::rotation_y(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return mat4
    (
        c, 0, -s, 0,
        0, 1,  0, 0,
        s, 0,  c, 0,
        0, 0,  0, 1
    );
}

mat4 mat4::rotation_z(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return mat4
    (
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    );
}

mat4 mat4::translation(const vec3 &t)
{
    return mat4
    (
        1.0f, 0.0f, 0.0f, 0.0,
        0.0f, 1.0f, 0.0f, 0.0,
        0.0f, 0.0f, 1.0f, 0.0,
        t.x,  t.y,  t.z,  1.0f
    );
}

mat4 mat4::scale(const vec3 &s)
{
    return mat4
    {
        s.x,  0.0f, 0.0f, 0.0f,
        0.0f, s.y,  0.0f, 0.0f,
        0.0f, 0.0f, s.z,  0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

mat4::mat4() :
    m11(0.0f), m12(0.0f), m13(0.0f), m14(0.0f),
    m21(0.0f), m22(0.0f), m23(0.0f), m24(0.0f),
    m31(0.0f), m32(0.0f), m33(0.0f), m34(0.0f),
    m41(0.0f), m42(0.0f), m43(0.0f), m44(0.0f)
{

}

mat4::mat4(
    float m11, float m12, float m13, float m14,
    float m21, float m22, float m23, float m24,
    float m31, float m32, float m33, float m34,
    float m41, float m42, float m43, float m44) :
    m11(m11), m12(m12), m13(m13), m14(m14),
    m21(m21), m22(m22), m23(m23), m24(m24),
    m31(m31), m32(m32), m33(m33), m34(m34),
    m41(m41), m42(m42), m43(m43), m44(m44)
{
}

bool mat4::operator==(const mat4 &other) const
{
    return
        m11 == other.m11 && m12 == other.m12 && m13 == other.m13 && m14 == other.m14 &&
        m21 == other.m21 && m22 == other.m22 && m23 == other.m23 && m24 == other.m24 &&
        m31 == other.m31 && m32 == other.m32 && m33 == other.m33 && m34 == other.m34 &&
        m41 == other.m41 && m42 == other.m42 && m43 == other.m43 && m44 == other.m44;
}

bool mat4::operator!=(const mat4 &other) const
{
    return !(*this == other);
}

mat4 mat4::operator*(float scalar) const
{
    return mat4
    (
        m11 * scalar, m12 * scalar, m13 * scalar, m14 * scalar,
        m21 * scalar, m22 * scalar, m23 * scalar, m24 * scalar,
        m31 * scalar, m32 * scalar, m33 * scalar, m34 * scalar,
        m41 * scalar, m42 * scalar, m43 * scalar, m44 * scalar
    );
}

mat4 mat4::operator*(const mat4 &other) const
{
    return this->multiply(other);
}

mat4 mat4::multiply(const mat4 &other) const
{
    return mat4
    (
        m11 * other.m11 + m12 * other.m21 + m13 * other.m31 + m14 * other.m41,
        m11 * other.m12 + m12 * other.m22 + m13 * other.m32 + m14 * other.m42,
        m11 * other.m13 + m12 * other.m23 + m13 * other.m33 + m14 * other.m43,
        m11 * other.m14 + m12 * other.m24 + m13 * other.m34 + m14 * other.m44,
        m21 * other.m11 + m22 * other.m21 + m23 * other.m31 + m24 * other.m41,
        m21 * other.m12 + m22 * other.m22 + m23 * other.m32 + m24 * other.m42,
        m21 * other.m13 + m22 * other.m23 + m23 * other.m33 + m24 * other.m43,
        m21 * other.m14 + m22 * other.m24 + m23 * other.m34 + m24 * other.m44,
        m31 * other.m11 + m32 * other.m21 + m33 * other.m31 + m34 * other.m41,
        m31 * other.m12 + m32 * other.m22 + m33 * other.m32 + m34 * other.m42,
        m31 * other.m13 + m32 * other.m23 + m33 * other.m33 + m34 * other.m43,
        m31 * other.m14 + m32 * other.m24 + m33 * other.m34 + m34 * other.m44,
        m41 * other.m11 + m42 * other.m41 + m43 * other.m31 + m44 * other.m41,
        m41 * other.m12 + m42 * other.m42 + m43 * other.m32 + m44 * other.m42,
        m41 * other.m13 + m42 * other.m43 + m43 * other.m33 + m44 * other.m43,
        m41 * other.m14 + m42 * other.m44 + m43 * other.m34 + m44 * other.m44
    );
}

mat4 mat4::transpose() const
{
    return mat4
    (
        m11, m21, m31, m41,
        m12, m22, m32, m42,
        m13, m23, m33, m43,
        m14, m24, m34, m44
     );
}

mat4 mat4::inverse() const
{
    // todo: implement matrix inverse (assume right handed coordinate system with -z forward)
    return mat4::identity();
}
