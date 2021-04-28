#pragma once

#include "defines.h"
#include <math.h>


namespace vectorizer
{
    template<class T>
    inline T lerp(T a, T b, float t)
    {
        return a + (b - a) * t;
    };

    struct vector2
    {
        vector2() = default;
        constexpr vector2(float x, float y = 0.f) : x(x), y(y) {}
        constexpr vector2(const vector2& other) : x(other.x), y(other.y) {}

        float x;
        float y;

        inline constexpr vector2 operator+(const vector2& other) const { return { x + other.x, y + other.y }; }
        inline constexpr vector2 operator-(const vector2& other) const { return { x - other.x, y - other.y }; }
        inline constexpr vector2 operator-() const { return { -x, -y }; }
        inline constexpr vector2 operator*(float fac) const { return { x * fac, y * fac }; }
        inline constexpr vector2 operator/(float fac) const { return { x / fac, y / fac }; }

        inline constexpr float dot(const vector2& other) const { return x * other.x + y * other.y; }
        inline float mag() const { return sqrtf(x * x + y * y); }
        inline constexpr float sqr_mag() const { return x * x + y * y; }

        inline vector2 normalized() const { if (auto m = mag()) return *this / m; return identity(); }

        inline float angle_between(const vector2& other) const
        {
            float mag_product = mag() * other.mag();

            if (mag_product == 0.f)
                return (float)M_PI * 2.f;

            float trig_coefficient = dot(other) / mag_product;

            if (trig_coefficient > 1.f || trig_coefficient < -1.f)
                return (float)M_PI * 2.f;

            return acosf(trig_coefficient);
        }

        inline vector2 rotated_by(float radians) const
        {
            return rotate(*this, radians);
        }

        inline static vector2 rotate(const vector2& vec, float radians)
        {
            return vector2{ vec.x * cosf(radians) - vec.y * sinf(radians), vec.x * sinf(radians) + vec.y * cosf(radians) };
        }

        inline static vector2 identity() { return { 0.f, 0.f }; }
    };

    typedef vector2 sizef;

    struct vector3
    {
        vector3() = default;
        constexpr vector3(const vector3& other) = default;
        constexpr vector3(float x, float y = 0.f, float z = 0.f) : x(x), y(y), z(z) {};
        constexpr vector3(const vector2& other) : x(other.x), y(other.y), z(0.f) {};

        float x;
        float y;
        float z;

        inline constexpr vector3 operator+(const vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
        inline constexpr vector3 operator-(const vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
        inline constexpr vector3 operator-() const { return { -x, -y, -z }; }
        inline constexpr vector3 operator*(const vector3& other) const { return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x }; }

        inline constexpr float dot(const vector3& other) const { return x * other.x + y * other.y + z * other.z; }
        inline float mag() const { return sqrtf(x * x + y * y + z * z); }
        inline constexpr float sqr_mag() const { return x * x + y * y + z * z; }

    };

    struct vector4
    {
        vector4() = default;
        constexpr vector4(const vector4& other) = default;
        constexpr vector4(vector4&& other) = default;
        constexpr vector4(float x, float y = 0.f, float z = 0.f, float w = 0.f) : x(x), y(y), z(z), w(w) {};
        constexpr vector4(const vector3& other) : x(other.x), y(other.y), z(other.z), w(0.f) {};
        constexpr vector4(const vector2& other) : x(other.x), y(other.y), z(0.f), w(0.f) {};

        float x;
        float y;
        float z;
        float w;

        inline vector4 operator+(const vector4& other) const { return { x + other.x, y + other.y, z + other.z, w + other.w }; }
        inline vector4 operator-(const vector4& other) const { return { x - other.x, y - other.y, z - other.z, w - other.w }; }
        inline vector4 operator-() const { return { -x, -y, -z, -w }; }
        inline vector4 operator*(float factor) const { return { x * factor, y * factor, z * factor, w * factor }; }

        inline operator vector3() const { return { x, y, z }; }
        inline operator vector2() const { return { x, y }; }
    };

    struct bounds2d
    {
        vector2 min;
        vector2 max;

        inline constexpr float area() const { return (max.x - min.x) * (max.y - min.y); }
        inline constexpr vector2 size() const { return max - min; }
    };

    struct vector2i
    {
        vector2i() = default;
        constexpr vector2i(const vector2i& other) = default;
        constexpr vector2i(int x, int y = 0) : x(x), y(y) {};

        int x;
        int y;

        inline constexpr vector2i operator+(const vector2i& other) const { return { x + other.x, y + other.y }; }
        inline constexpr vector2i operator-(const vector2i& other) const { return { x - other.x, y - other.y }; }
        inline constexpr vector2i operator-() const { return { -x, -y }; }
        inline vector2i& operator+=(const vector2i& other) { x += other.x; y += other.y; return *this; }
        inline vector2i& operator-=(const vector2i& other) { x -= other.x; y -= other.y; return *this; }

        inline constexpr bool operator==(const vector2i& other) const { return x == other.x && y == other.y; }
        inline constexpr bool operator!=(const vector2i& other) const { return x != other.x || y != other.y; }

        inline explicit constexpr operator vector2() const { return vector2{ (float)x, (float)y }; }

        inline constexpr vector2i rotated_by(int turns) const
        {
            switch (turns % 4)
            {
            case +1:
            case -3:
                return vector2i{ y, -x };
            case +2:
            case -2:
                return vector2i{ -x, -y };
            case +3:
            case -1:
                return vector2i{ -y, x };
            default:
            case 0:
                return *this;
            }
        }
    };

    struct sizei
    {
        sizei() = default;
        constexpr sizei(const sizei & other) = default;
        constexpr sizei(size_t x, size_t y = 0) : x(x), y(y) {};

        size_t x;
        size_t y;

        inline constexpr sizei operator+(const sizei & other) const { return { x + other.x, y + other.y }; }

        inline constexpr bool operator==(const sizei & other) const { return x == other.x && y == other.y; }
        inline constexpr bool operator!=(const sizei & other) const { return x != other.x || y != other.y; }

        inline explicit constexpr operator vector2() const { return vector2{ (float)x, (float)y }; }

        inline constexpr sizei rotated_by(int turns) const
        {
            switch (turns % 4)
            {
            case +1:
            case -3:
                return sizei{ y, -x };
            case +2:
            case -2:
                return sizei{ -x, -y };
            case +3:
            case -1:
                return sizei{ -y, x };
            default:
            case 0:
                return *this;
            }
        }
    };

    typedef sizei vector2u;

    struct bounds2di
    {
        bounds2di() = default;
        constexpr bounds2di(const bounds2di& other) = default;
        constexpr bounds2di(bounds2di&& other) = default;
        constexpr bounds2di(int min_x, int min_y, int max_x, int max_y) : min(min_x, min_y), max(max_x, max_y) {};
        constexpr bounds2di(vector2i min, vector2i max) : min(min), max(max) {};

        bounds2di& operator=(const bounds2di& other) = default;

        vector2i min;
        vector2i max;

        inline constexpr int area() const { return (max.x - min.x) * (max.y - min.y); }
        inline constexpr vector2i size() const { return max - min; }
        inline constexpr int width() const { return max.x - min.x; }
        inline constexpr int height() const { return max.y - min.y; }
    };

    inline vector2 vec_cross_trunc(vector3 a, vector3 b)
    {
        return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z };
    }

    struct dimensional_indexer
    {
        explicit constexpr dimensional_indexer(size_t width, size_t height = 0) : width(width), height(height) {}

        size_t width;
        size_t height;

        inline constexpr int index(int x, int y) const { return x + y * width; }
        inline constexpr int index(int x, int y, int z) const { return x + y * width + z * width * height; }
        inline constexpr int index(vector2i spot) const { return index(spot.x, spot.y); }

        inline constexpr vector2i reverse(int index) const { return { index % (int)width, index / (int)width }; }

        inline constexpr int operator()(int x, int y) const { return index(x, y); }
        inline constexpr int operator()(int x, int y, int z) const { return index(x, y, z); }
        inline constexpr int operator()(vector2i spot) const { return index(spot); }

        inline constexpr static int index_of(int x, int y, int width) { return x + y * width; }
        inline constexpr static int index_of(int x, int y, int z, int width, int height) { return x + y * width + z * width * height; }
    };

    // Maybe another header:

    class CompassDirection
    {
    public:
        enum Direction
        {
            North,
            East,
            South,
            West
        };

        CompassDirection() = default;
        constexpr CompassDirection(Direction dir) : _dir(dir) {}
        explicit CompassDirection(vector2 direction) : _dir(from_vector(direction)) {}

        operator Direction() const { return _dir; }

        constexpr vector2 to_dir() const
        {
            switch (_dir)
            {
            default:
            case North:
                return vector2{ 0.f, 1.f };
            case East:
                return vector2{ 1.f, 0.f };
            case South:
                return vector2{ 0, -1.f };
            case West:
                return vector2{ -1.f, 0.f };
            }
        }

        constexpr vector2i to_diri() const
        {
            switch (_dir)
            {
            default:
            case North:
                return vector2i{ 0, 1 };
            case East:
                return vector2i{ 1, 0 };
            case South:
                return vector2i{ 0, -1 };
            case West:
                return vector2i{ -1, 0 };
            }
        }

        static Direction from_vector(const vector2& dir)
        {
            vector2 norm = dir.normalized();

            float angle = norm.angle_between(vector2{ 0.f, 1.f });

            if (angle < M_PI_4)
                return North;

            if (angle > M_PI - M_PI_4)
                return South;

            if (norm.x > 0.f)
                return East;

            return West;
        }

        inline constexpr CompassDirection turned_by(int turns) const
        {
            switch (turns % 4)
            {
            case +1:
            case -3:
                switch (_dir)
                {
                default:
                case North:
                    return East;
                case East:
                    return South;
                case South:
                    return West;
                case West:
                    return North;
                }
            case +2:
            case -2:
                switch (_dir)
                {
                default:
                case CompassDirection::North:
                    return South;
                case CompassDirection::East:
                    return West;
                case CompassDirection::South:
                    return North;
                case CompassDirection::West:
                    return East;
                }
            case +3:
            case -1:
                switch (_dir)
                {
                default:
                case CompassDirection::North:
                    return West;
                case CompassDirection::East:
                    return North;
                case CompassDirection::South:
                    return East;
                case CompassDirection::West:
                    return South;
                }
            default:
            case 0:
                return *this;
            }
        }

    private:
        Direction _dir;
    };

    // End another header
}