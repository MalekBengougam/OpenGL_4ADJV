#pragma once

// M_PI est defini dans <math.h> et non plus dans <cmath>
// cependant il faut necessairement ajouter le define suivant 
// car M_PI n'est pas standard en C++
#define _USE_MATH_DEFINES 1
#include <math.h>		
#include <cstring>

struct vec2 { float x, y; };
struct vec3 
{ 
	float x, y, z;

	inline vec3 operator*(const float t) const {
		return{ x*t, y*t, z*t };
	}

	inline vec3 operator+(const vec3& rhs) const {
		return{ x + rhs.x, y + rhs.y, z + rhs.z };
	}

	inline vec3 operator-(const vec3& rhs) const {
		return{ x - rhs.x, y - rhs.y, z - rhs.z };
	}

	inline vec3 operator*(const vec3& rhs) const {
		return{ x * rhs.x, y * rhs.y, z * rhs.z };
	}

	inline float dot(const vec3& rhs) const {
		return x*rhs.x + y*rhs.y + z*rhs.z;
	}

	inline vec3& normalize() {
		float inv_length = 1.f / sqrtf(x*x + y*y + z*z);
		x *= inv_length;
		y *= inv_length;
		z *= inv_length;
		return *this;
	}

	static inline vec3 cross(const vec3& lhs, const vec3& rhs) {
		return vec3{ lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z , lhs.x*rhs.y - lhs.y*rhs.x };
	}
};

struct mat4
{
	float m[16];

	void scale(const vec3& factors)
	{
		memset(m, 0, sizeof(mat4));
		m[0] = factors.x; m[5] = factors.y; m[10] = factors.z;
		m[15] = 1.f;
	}

	void rotationUp(const float angle)
	{
		memset(m, 0, sizeof(mat4));
		m[0] = cosf(angle);
		m[2] = -sinf(angle);
		m[5] = 1.f;
		m[8] = sinf(angle);
		m[10] = cosf(angle);
		m[15] = 1.f;
	}

	void translation(const vec3& position)
	{
		memset(m, 0, sizeof(mat4));
		m[0] = 1.f; m[5] = 1.f; m[10] = 1.f;
		m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1.f;
	}

	void perspective(float fovh, float aspect, float znear, float zfar)
	{
		memset(m, 0, sizeof(mat4));
		float radFov = fovh * ((float)M_PI / 180.f);
		float cot = 1.f / tanf(radFov);
		m[0] = cot / aspect;
		m[5] = cot;
		m[10] = -(zfar + znear) / (zfar - znear);
		m[11] = -1.f;
		m[14] = -2.f*(zfar*znear) / (zfar - znear);
	}
};
