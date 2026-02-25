#pragma once

template<typename T> struct Vector3
{
   T x;
   T y;
   T z;

   public:
    static double dotProduct(const Vector3& v1, const Vector3& v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    static Vector3 crossProduct(const Vector3& v1, const Vector3& v2)
    {
        return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
    }

    static Vector3 rotateVectorByRightAngle(const Vector3& vector, const int& angle)
    {
        int tempAngle = angle % 4;

        switch (tempAngle)
        {
            case 1:
            return {-vector.y, vector.x, vector.z};
            case 2:
            return {-vector.x, -vector.y, vector.z};
            case 3:
            return {vector.y, -vector.x, vector.z};
            default:
            return vector;
        }
    }

    static Vector3 rotateVector(const Vector3& vector, const double& angle)
    {
        double radians = angle * DEG_TO_RAD;
        double sinAngle = sin(radians);
        double cosAngle = cos(radians);

        Vector3 newVector = {0,0,vector.z};
        newVector.x = cosAngle * vector.x - sinAngle * vector.y;
        newVector.y = sinAngle * vector.x + cosAngle * vector.y;
        return newVector;
    }

    T sqrMagnitude()
    {
        return x * x + y * y + z * z;
    }

    double Magnitude()
    {
        return sqrt(sqrMagnitude());
    }

    static float lerp(const T& a, const T& b, const T& f)
    {
        return a + f * (b - a);
    }

    static Vector3 lerp(const Vector3& a, const Vector3& b, const float& v)
    {
        return {lerp(a.x, b.x, v), lerp(a.y, b.y, v), lerp(a.z, b.z, v)};
    }

    void normalize()
    {
        double magnitude = Magnitude();

        if (magnitude <= 0.0)
            return;

        x /= magnitude;
        y /= magnitude;
        z /= magnitude;
    }

    void scale(const float& scale)
    {
        x *= scale;
        y *= scale;
        z *= scale;
    }

    Vector3 operator+(const Vector3& a) const
    {
        return {x + a.x, y + a.y, z + a.z};
    }

    Vector3 operator-(const Vector3& a) const
    {
        return {x - a.x, y - a.y, z - a.z};
    }
    
    Vector3 operator+(const T& a) const
    {
        return {x + a, y + a, z + a};
    }

    Vector3 operator-(const T& a) const
    {
        return {x - a, y - a, z - a};
    }

    Vector3 operator*(const T scale) const
    {
        return {x * scale, y * scale, z * scale};
    }

    Vector3& operator+=(const Vector3& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }

    Vector3& operator+=(const T& a)
    {
        x += a;
        y += a;
        z += a;
        return *this;
    }

    Vector3& operator-=(const T& a)
    {
        x -= a;
        y -= a;
        z -= a;
        return *this;
    }
    
    Vector3& operator*=(const T& a)
    {
        scale(a);
        return *this;
    }
};