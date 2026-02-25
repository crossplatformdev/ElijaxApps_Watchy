#pragma once

#include <math.h>

struct Vec2f {
  float x;
  float y;

  constexpr Vec2f() : x(0.0f), y(0.0f) {}
  constexpr Vec2f(float x_, float y_) : x(x_), y(y_) {}

  inline float lengthSquared() const { return x * x + y * y; }
  inline float length() const { return sqrtf(lengthSquared()); }

  inline Vec2f getNormalized() const {
    const float len = length();
    if (len <= 0.0f) {
      return Vec2f(0.0f, 0.0f);
    }
    const float inv = 1.0f / len;
    return Vec2f(x * inv, y * inv);
  }

  inline float dot(const Vec2f& other) const { return x * other.x + y * other.y; }

  constexpr Vec2f operator+() const { return *this; }
  constexpr Vec2f operator-() const { return Vec2f(-x, -y); }

  constexpr Vec2f operator+(const Vec2f& other) const { return Vec2f(x + other.x, y + other.y); }
  constexpr Vec2f operator-(const Vec2f& other) const { return Vec2f(x - other.x, y - other.y); }

  constexpr Vec2f operator*(float scalar) const { return Vec2f(x * scalar, y * scalar); }

  inline Vec2f& operator+=(const Vec2f& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  inline Vec2f& operator-=(const Vec2f& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  inline Vec2f& operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }
};

struct Vec3f {
  float x;
  float y;
  float z;

  constexpr Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
  constexpr Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

  inline float lengthSquared() const { return x * x + y * y + z * z; }
  inline float length() const { return sqrtf(lengthSquared()); }

  constexpr Vec3f operator-(const Vec3f& other) const { return Vec3f(x - other.x, y - other.y, z - other.z); }
};
