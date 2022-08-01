#pragma once

#include "../Math/Math.hpp"

class Ray {
public:
	Ray() {}
	Ray(const Vec3& o, const Vec3& d)
		: origin(o), direction(d) {}

	inline Vec3 GetOrigin() const { return origin; }
	inline Vec3 GetDirection() const { return direction; }
	inline Vec3 At(float t) const { return origin + t * direction; }

public:
	Vec3 origin;
	Vec3 direction;
};