#pragma once

#include <windows.h>
#include <corecrt_math_defines.h>

typedef DWORD Void;
typedef DWORD Any;
typedef DWORD uint;
typedef DWORD Hash;
typedef int Entity;
typedef int Player;
typedef int FireId;
typedef int Ped;
typedef int Vehicle;
typedef int Cam;
typedef int CarGenerator;
typedef int Group;
typedef int Train;
typedef int Pickup;
typedef int Object;
typedef int Weapon;
typedef int Interior;
typedef int Blip;
typedef int Texture;
typedef int TextureDict;
typedef int CoverPoint;
typedef int Camera;
typedef int TaskSequence;
typedef int ColourIndex;
typedef int Sphere;
typedef int ScrHandle;

#define PI 3.141592653589793238462643383279502884L

struct Vector2
{
	float x, y;

	Vector2()
		: x(0.f)
		, y(0.f)
	{ }

	Vector2(float x, float y)
		: x(x)
		, y(y)
	{ }
};

struct Vector3
{
public:
	alignas(8) float x;
	alignas(8) float y;
	alignas(8) float z;

public:
	Vector3()
		: x(0.f)
		, y(0.f)
		, z(0.f)
	{ }

	Vector3(float x, float y, float z)
		: x(x)
		, y(y)
		, z(z)
	{ }

	Vector3(float arr[3])
		: x(arr[0])
		, y(arr[1])
		, z(arr[2])
	{ }

	Vector3 operator*(const float& scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	Vector3 operator+(const float& scale)
	{
		x += scale;
		y += scale;
		z += scale;
		return *this;
	}

	Vector3 operator+(const Vector3& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	Vector3 operator / (float divide) const { return { this->x / divide, this->y / divide, this->z / divide }; }

	Vector3 operator-(const Vector3& vec)
	{
		return Vector3(x - vec.x, y - vec.y, z - vec.z);
	}

	Vector3 operator-(const float& v)
	{
		return Vector3(x - v, y - v, z - v);
	}

	Vector3& operator += (const Vector3& other) { this->x += other.x; this->y += other.y; this->z += other.z; return *this; }
	Vector3& operator -= (const Vector3& other) { this->x -= other.x; this->y -= other.y; this->z -= other.z; return *this; }

	Vector3 operator* (float scalar) const { return { this->x * scalar, this->y * scalar, this->z * scalar }; }

	bool IsValid()
	{
		return x != 0 && y != 0 && z != 0;
	}

	inline Vector3 forward()
	{
		float sp{}, sy{}, cp{}, cy{};
		float angle{};

		angle = y * (M_PI / 180.0f);
		sy = sinf(angle);
		cy = cosf(angle);

		angle = -x * (M_PI / 180.0f);
		sp = sinf(angle);
		cp = cosf(angle);

		return { cp * cy, cp * sy, -sp };
	}
};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;

	Vector4()
		: x(0.f)
		, y(0.f)
		, z(0.f)
		, w(0.f)
	{ }

	Vector4(float x, float y, float z, float w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{ }


	inline Vector4 operator*(const Vector4& vector)
	{
		return Vector4(x * vector.x, y * vector.y, z * vector.z, w * vector.w);
	}

	inline Vector4 operator*(float value)
	{
		return Vector4(x * value, y * value, z * value, w * value);
	}
};