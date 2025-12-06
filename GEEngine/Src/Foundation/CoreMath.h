#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <algorithm>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define M_PI 3.141592654f
#define SQ(x) ((x) * (x))

template<typename T>
static T lerp(const T& a, const T& b, float t) {
    return a * (1.0f - t) + b * t;
}

class Vec3 {
public:
    union {
        float v[3];
        struct { float x, y, z; };
    };

    Vec3(float X = 0, float Y = 0, float Z = 0) : x(X), y(Y), z(Z) {}

    // Arithmetic
    Vec3 operator+(const Vec3& r) const {
        return Vec3(x + r.x, y + r.y, z + r.z);
    }

    Vec3 operator-(const Vec3& r) const {
        return Vec3(x - r.x, y - r.y, z - r.z);
    }

    Vec3 operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3& operator+=(const Vec3& r) {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }

    Vec3 operator*(const Vec3& r) const {
        return Vec3(x * r.x, y * r.y, z * r.z);
    }

    Vec3 operator*(float s) const {
        return Vec3(x * s, y * s, z * s);
    }

    Vec3 operator/(float s) const {
        float inv = 1.0f / s;
        return Vec3(x * inv, y * inv, z * inv);
    }

    Vec3& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Vec3& operator/=(float s) {
        float inv = 1.0f / s;
        x *= inv; y *= inv;
        z *= inv;
        return *this;
    }

    // Length
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float lengthSq() const {
        return x * x + y * y + z * z;
    }

    // Normalization
    Vec3 normalized() const {
        float lsq = lengthSq();
        if (lsq <= 0.0f) {
            return Vec3(0, 0, 0);
        }

        float inv = 1.0f / std::sqrt(lsq);
        return Vec3(x * inv, y * inv, z * inv);
    }

    float normalize() {
        float len = length();
        if (len <= 0.0f) {
            x = y = z = 0;
            return 0;
        }

        float inv = 1.0f / len;
        x *= inv;
        y *= inv;
        z *= inv;
        return len;
    }

    // Dot and cross
    float dot(const Vec3& r) const {
        return x * r.x + y * r.y + z * r.z;
    }

    Vec3 cross(const Vec3& r) const {
        return Vec3(y * r.z - z * r.y,
            z * r.x - x * r.z,
            x * r.y - y * r.x);
    }

    static float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    }

    float max() const {
        return std::max(x, std::max(y, z));
    }


    Vec3 max(const Vec3& v1, const Vec3& v2) {
        return Vec3(
            std::max(v1.v[0], v2.v[0]),
            std::max(v1.v[1], v2.v[1]),
            std::max(v1.v[2], v2.v[2]));
    }
};

class Vec4 {
public:
    union {
        float v[4];
        struct { float x, y, z, w; };
    };

    Vec4() : x(0), y(0), z(0), w(0) {}

    Vec4(float xx, float yy, float zz, float ww)
        : x(xx), y(yy), z(zz), w(ww) {
    }

    Vec4(const Vec3& p, float ww) : x(p.x), y(p.y), z(p.z), w(ww) {}

    float max() const {
        return std::max(x, std::max(y, z));
    }

    void perspectiveDivide() {
        if (w == 0.0f) {
            return;
        }
        float invW = 1.0f / w;
        x *= invW;
        y *= invW;
        z *= invW;
        w = invW;
    }
};

class Matrix {
public:
    union {
        float m[16];
        float a[4][4];
    };

    Matrix() { identity(); }

    void identity() {
        for (int i = 0; i < 16; ++i) m[i] = 0;
        a[0][0] = a[1][1] = a[2][2] = a[3][3] = 1.0f;
    }

    static Matrix Identity() {
        Matrix r;
        r.identity();
        return r;
    }

    // Vec4 transform
    Vec4 multiply(const Vec4& v) const {
        return Vec4(
            v.x * m[0] + v.y * m[1] + v.z * m[2] + v.w * m[3],
            v.x * m[4] + v.y * m[5] + v.z * m[6] + v.w * m[7],
            v.x * m[8] + v.y * m[9] + v.z * m[10] + v.w * m[11],
            v.x * m[12] + v.y * m[13] + v.z * m[14] + v.w * m[15]
        );
    }

    // Vec3 as point
    Vec3 transformPoint(const Vec3& v) const {
        return Vec3(
            v.x * m[0] + v.y * m[1] + v.z * m[2] + m[3],
            v.x * m[4] + v.y * m[5] + v.z * m[6] + m[7],
            v.x * m[8] + v.y * m[9] + v.z * m[10] + m[11]
        );
    }

    // Vec3 as direction
    Vec3 transformVector(const Vec3& v) const {
        return Vec3(
            v.x * m[0] + v.y * m[1] + v.z * m[2],
            v.x * m[4] + v.y * m[5] + v.z * m[6],
            v.x * m[8] + v.y * m[9] + v.z * m[10]
        );
    }

    // Matrix multiplication
    Matrix operator*(const Matrix& r) const {
        Matrix out;
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                out.a[row][col] =
                    a[row][0] * r.a[0][col] +
                    a[row][1] * r.a[1][col] +
                    a[row][2] * r.a[2][col] +
                    a[row][3] * r.a[3][col];
            }
        }
        return out;
    }

    Matrix transpose() const {
        Matrix r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r.a[i][j] = a[j][i];
        return r;
    }

    // Transformations
    static Matrix translation(const Vec3& v) {
        Matrix r = Identity();
        r.m[3] = v.x;
        r.m[7] = v.y;
        r.m[11] = v.z;
        return r;
    }

    static Matrix scaling(const Vec3& v) {
        Matrix r = Identity();
        r.m[0] = v.x;
        r.m[5] = v.y;
        r.m[10] = v.z;
        return r;
    }

    static Matrix rotationX(float a) {
        Matrix r = Identity();
        float c = cos(a);
        float s = sin(a);
        r.m[5] = c;
        r.m[6] = -s;
        r.m[9] = s;
        r.m[10] = c;
        return r;
    }

    static Matrix rotationY(float a) {
        Matrix r = Identity();
        float c = cos(a);
        float s = sin(a);
        r.m[0] = c;
        r.m[2] = s;
        r.m[8] = -s;
        r.m[10] = c;
        return r;
    }

    static Matrix rotationZ(float angle) {
        float c = std::cos(angle), s = std::sin(angle);
        Matrix r;
        r.m[0] = c;
        r.m[1] = -s;
        r.m[4] = s;
        r.m[5] = c;
        return r;
    }

    static Matrix perspective(const float n, const float f, float aspect, const float fov) {
        Matrix m;
        float fovRad = fov * M_PI / 180.0f;
        float t = 1.0f / std::tanf(fovRad * 0.5);

        m.a[0][0] = t / aspect;
        m.a[1][1] = t;
        m.a[2][2] = f / (n - f);
        m.a[2][3] = (f * n) / (n - f);
        m.a[3][2] = -1.0f;
        return m;
    }

    static Matrix perspectiveLH(const float n, const float f, float aspect, const float fov) {
     
        Matrix m;

        float fovRad = fov * M_PI / 180.0f;
        float t = 1.0f / std::tanf(fovRad * 0.5f);

        m.a[0][0] = t / aspect;
        m.a[1][1] = t;
        m.a[2][2] = f / (f - n);
        m.a[2][3] = (-f * n) / (f - n);
        m.a[3][2] = 1.0f;
        return m;
    }

    static Matrix lookAt(const Vec3& from, const Vec3& to, const Vec3& up) {
        Vec3 zaxis = (from - to).normalized();
        Vec3 xaxis = Vec3::cross(up, zaxis).normalized();
        Vec3 yaxis = Vec3::cross(zaxis, xaxis);

        Matrix m;

        m.a[0][0] = xaxis.x;
        m.a[0][1] = xaxis.y;
        m.a[0][2] = xaxis.z;
        m.a[0][3] = -Vec3::dot(xaxis, from);
        m.a[1][0] = yaxis.x;
        m.a[1][1] = yaxis.y;
        m.a[1][2] = yaxis.z;
        m.a[1][3] = -Vec3::dot(yaxis, from);
        m.a[2][0] = zaxis.x;
        m.a[2][1] = zaxis.y;
        m.a[2][2] = zaxis.z;
        m.a[2][3] = -Vec3::dot(zaxis, from);
        m.a[3][3] = 1.0f;
        return m;
    }

    static Matrix lookAtLH(const Vec3& from, const Vec3& to, const Vec3& up)
    {
        Vec3 zaxis = (to - from).normalized(); 
        Vec3 xaxis = Vec3::cross(up, zaxis).normalized();
        Vec3 yaxis = Vec3::cross(zaxis, xaxis);

        Matrix m;

        m.a[0][0] = xaxis.x;
        m.a[0][1] = xaxis.y;
        m.a[0][2] = xaxis.z;
        m.a[0][3] = -Vec3::dot(xaxis, from);
        m.a[1][0] = yaxis.x;
        m.a[1][1] = yaxis.y;
        m.a[1][2] = yaxis.z;
        m.a[1][3] = -Vec3::dot(yaxis, from);
        m.a[2][0] = zaxis.x;
        m.a[2][1] = zaxis.y;
        m.a[2][2] = zaxis.z;
        m.a[2][3] = -Vec3::dot(zaxis, from);
        m.a[3][3] = 1.0f;

        return m;
    }
};

// -----------------------------------------------------------------------------
// Shading Frame (Tangent, Bitangent, Normal)
// -----------------------------------------------------------------------------
class ShadingFrame {
public:
    Matrix M;

    ShadingFrame() {
        M.identity();
    }

    static ShadingFrame fromNormal(const Vec3& normal) {
        ShadingFrame f;

        Vec3 z = normal.normalized();
        Vec3 ref = (std::fabs(z.z) < 0.999f) ? Vec3(0, 0, 1) : Vec3(0, 1, 0);

        Vec3 x = ref.cross(z).normalized();
        Vec3 y = z.cross(x).normalized();

        f.M.a[0][0] = x.x; f.M.a[0][1] = x.y; f.M.a[0][2] = x.z;
        f.M.a[1][0] = y.x; f.M.a[1][1] = y.y; f.M.a[1][2] = y.z;
        f.M.a[2][0] = z.x; f.M.a[2][1] = z.y; f.M.a[2][2] = z.z;

        return f;
    }

    Vec3 toLocal(const Vec3& v) const {
        return M.transpose().transformVector(v);
    }

    Vec3 toWorld(const Vec3& v) const {
        return M.transformVector(v);
    }
};

// -----------------------------------------------------------------------------
// Quaternion
// -----------------------------------------------------------------------------
class Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float ww, float xx, float yy, float zz)
        : w(ww), x(xx), y(yy), z(zz) {
    }

    static Quaternion Identity() {
        return Quaternion(1, 0, 0, 0);
    }

    float magnitude() const {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    Quaternion normalized() const {
        float mag = magnitude();
        if (mag <= 0.0f) return Identity();
        float inv = 1.0f / mag;
        return Quaternion(w * inv, x * inv, y * inv, z * inv);
    }

    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    Quaternion inverse() const {
        float lsq = w * w + x * x + y * y + z * z;
        if (lsq <= 0.0f) return Identity();
        float inv = 1.0f / lsq;
        return Quaternion(w * inv, -x * inv, -y * inv, -z * inv);
    }

    Quaternion operator*(const Quaternion& r) const {
        return Quaternion(
            w * r.w - x * r.x - y * r.y - z * r.z,
            w * r.x + x * r.w + y * r.z - z * r.y,
            w * r.y - x * r.z + y * r.w + z * r.x,
            w * r.z + x * r.y - y * r.x + z * r.w
        );
    }

    static Quaternion fromAxisAngle(const Vec3& axis, float angle) {
        Vec3 n = axis.normalized();
        float half = angle * 0.5f;
        float s = std::sin(half);
        return Quaternion(std::cos(half), n.x * s, n.y * s, n.z * s);
    }

    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t) {
        Quaternion b = q2;
        float dot = q1.w * b.w + q1.x * b.x + q1.y * b.y + q1.z * b.z;

        if (dot < 0.0f)
        {
            dot = -dot;
            b.w = -b.w; b.x = -b.x; b.y = -b.y; b.z = -b.z;
        }

        const float EPS = 0.9995f;
        if (dot > EPS)
        {
            Quaternion r(
                q1.w + t * (b.w - q1.w),
                q1.x + t * (b.x - q1.x),
                q1.y + t * (b.y - q1.y),
                q1.z + t * (b.z - q1.z)
            );
            return r.normalized();
        }

        float theta0 = std::acos(dot);
        float theta = theta0 * t;

        float sin0 = std::sin(theta0);
        float s0 = std::cos(theta) - dot * std::sin(theta) / sin0;
        float s1 = std::sin(theta) / sin0;

        return Quaternion(
            s0 * q1.w + s1 * b.w,
            s0 * q1.x + s1 * b.x,
            s0 * q1.y + s1 * b.y,
            s0 * q1.z + s1 * b.z
        );
    }

    Vec3 rotate(const Vec3& v) const {
        Quaternion p(0, v.x, v.y, v.z);
        Quaternion r = (*this) * p * conjugate();
        return Vec3(r.x, r.y, r.z);
    }

    Matrix toMatrix() const {
        float aa = w * w, bb = x * x, cc = y * y;
        float ab = w * x, ac = w * y, ad = w * z;
        float bc = x * y, bd = x * z, cd = y * z;

        Matrix M;

        M.m[0] = 1 - 2 * (bb + cc);
        M.m[1] = 2 * (ab - cd);
        M.m[2] = 2 * (ac + bd);
        M.m[3] = 0;

        M.m[4] = 2 * (ab + cd);
        M.m[5] = 1 - 2 * (aa + cc);
        M.m[6] = 2 * (bc - ad);
        M.m[7] = 0;

        M.m[8] = 2 * (ac - bd);
        M.m[9] = 2 * (bc + ad);
        M.m[10] = 1 - 2 * (aa + bb);
        M.m[11] = 0;

        M.m[12] = 0;
        M.m[13] = 0;
        M.m[14] = 0;
        M.m[15] = 1;

        return M;
    }
};

// -----------------------------------------------------------------------------
// Colour
// -----------------------------------------------------------------------------
class Colour {
public:
    float r, g, b, a;

    Colour() : r(0), g(0), b(0), a(1) {}
    Colour(float R, float G, float B, float A = 1.0f)
        : r(R), g(G), b(B), a(A) {
    }

    Colour operator+(const Colour& c) const {
        return Colour(r + c.r, g + c.g, b + c.b, a + c.a);
    }

    Colour operator*(const Colour& c) const {
        return Colour(r * c.r, g * c.g, b * c.b, a * c.a);
    }

    Colour operator*(float s) const {
        return Colour(r * s, g * s, b * s, a * s);
    }

    Colour operator/(float s) const {
        float inv = 1.0f / s;
        return Colour(r * inv, g * inv, b * inv, a * inv);
    }
};

// -----------------------------------------------------------------------------
// Tools
// -----------------------------------------------------------------------------

static float edgeFunction(const Vec4& v0, const Vec4& v1, const Vec4& p) {
    return (((p.x - v0.x) * (v1.y - v0.y)) - ((v1.x - v0.x) * (p.y - v0.y)));
}

static Vec4 NDCToScreen(const Vec4& ndc, int width, int height)
{
    Vec4 out;
    out.x = (ndc.x + 1.0f) * 0.5f * width;
    out.y = (1.0f - ndc.y) * 0.5f * height;  // y flipped
    out.z = ndc.z;
    out.w = 1.0f;
    return out;
};