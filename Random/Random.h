#pragma once

#include <cstdlib>

#include "../Math/Math.hpp"

//0-1, 随机数
inline float random_float() {
	return static_cast<float>(rand() / (RAND_MAX + 1.0f));
}

//min-max, 随机数
inline float random_float(float min, float max) {
	return min + (max - min) * random_float();
}

inline Vec3 random() {
	return Vec3(random_float(), random_float(), random_float());
}

inline Vec3 random(float min, float max) {
	return Vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

inline Vec3 random_unit_vector() {
	float a = random_float(0.0f, 2.0f * PI);
	float z = random_float(-1.0f, 1.0f);
	float r = sqrtf(1.0f - z * z);
	return Vec3(r * cos(a), r * sin(a), z);
}

inline Vec3 random_in_unit_sphere() {
	while (true) {
		Vec3 p = random(-1.0f, 1.0f);
		if (vector_length_square<3, float>(p) >= 1.0f) {
			continue;
		}
		return p;
	}
}

inline Vec3 random_in_unit_disk() {
	while (true) {
		Vec3 p = Vec3(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), 0.0f);
		if (vector_length_square<3, float>(p) >= 1.0f) {
			continue;
		}
		return p;
	}
}