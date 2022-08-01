#pragma once

#include "../Math/Math.hpp"

//标准正交基
class ONB {
public:
	ONB() {}
	ONB(const Vec3& n) { BuildFrom(n); }
	ONB(const Vec3& n, const Vec3& t, const Vec3& b) { BuildFrom(n, t, b); }

	inline Vec3 operator[](int i) const { return axis[i]; }

	inline Vec3 u() const { return axis[0]; }
	inline Vec3 v() const { return axis[1]; }
	inline Vec3 w() const { return axis[2]; }

	inline Vec3 LocalToGlobal(float X, float Y, float Z) const { return X * axis[0] + Y * axis[1] + Z * axis[2]; }
	inline Vec3 LocalToGlobal(const Vec3& vect) const { return vect.x * axis[0] + vect.y * axis[1] + vect.z * axis[2]; }
	inline Vec3 GlobalToLocal(const Vec3& vect) const {
		return Vec3(
			axis[0].x * vect.x + axis[1].x * vect.y + axis[2].x * vect.z,
			axis[0].y * vect.x + axis[1].y * vect.y + axis[2].y * vect.z,
			axis[0].z * vect.x + axis[1].z * vect.y + axis[2].z * vect.z);
	}

	void BuildFrom(const Vec3& n);
	void BuildFrom(const Vec3& n, const Vec3& t, const Vec3& b);//TBN

public:
	Vec3 axis[3];
};