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

	// produit une matrice de vue (view matrix) simulant le placement d'une camera virtuelle dans la scene
	// en pratique cela revient a deplacer/orienter chaque objet de la scene de maniere inverse a la transform de la camera
	void lookat(const vec3& eye, const vec3& target, const vec3& up)
	{
		// transform de la camera (forme un repere/base)
		vec3 forward = (eye - target).normalize();
		vec3 right = vec3::cross(up, forward).normalize();
		vec3 newUp = vec3::cross(forward, right);
		// comme la transform est orthogonale (pas de deformation) on a une forme plus simple et rapide
		// que le calcul d'une matrice inverse. 
		// Ici Rt est une matrice de rotation 3x3, transposition de la transform de la camera 
		// et E = -(Rt * eye), ce qui produit la translation inverse '-eye' projetee dans le repère Rt 
		// | Rt E |
		// | 0  1 |

		// calcul de E par projection de -eye sur chacun des axes de la base
		float ex = -right.dot(eye);
		float ey = -newUp.dot(eye);
		float ez = -forward.dot(eye);

		m[0] = right.x; m[1] = newUp.x; m[2] = forward.x; m[3] = 0.f;	// 1ere colonne
		m[4] = right.y; m[5] = newUp.y; m[6] = forward.y; m[7] = 0.f;	// 2eme colonne
		m[8] = right.z; m[9] = newUp.z; m[10] = forward.z; m[11] = 0.f; // 3eme colonne
		m[12] = ex; m[13] = ey; m[14] = ez; m[15] = 1.f;				// 4eme colonne
	}
};
