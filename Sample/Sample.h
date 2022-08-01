#pragma once

#include <vector>
#include <assert.h>

#include "../Random/Random.h"

inline Vec3 random_cosine_direction() {
	float r1 = random_float();
	float r2 = random_float();
	float z = sqrtf(1.0f - r2);

	float phi = 2 * PI * r1;
	float x = cos(phi) * sqrt(r2);
	float y = sin(phi) * sqrt(r2);

	return Vec3(x, y, z);
}

inline Vec3 random_to_sphere(float radius, float distance_squared) {
	float r1 = random_float();
	float r2 = random_float();
	float z = 1.0f + r2 * (sqrtf(1.0f - radius * radius / distance_squared) - 1.0f);

	float phi = 2.0f * PI * r1;
	float x = cos(phi) * sqrtf(1.0f - z * z);
	float y = sin(phi) * sqrtf(1.0f - z * z);

	return Vec3(x, y, z);
}

inline Vec3 square_to_Beckmann(float alpha) {
	Vec2 sample(random_float(), random_float());

	float phi = PI * 2.0f * sample.x;
	float theta = atan(sqrt(-alpha * alpha * log(1.0f - sample.y)));
	float cosPhi = cos(phi);
	float sinPhi = sin(phi);
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	float x = sinTheta * cosPhi;
	float y = sinTheta * sinPhi;
	float z = cosTheta;

	return Vec3(x, y, z);
}

inline Vec3 square_to_GGX(float alpha) {
	float Xi1 = random_float();
	float Xi2 = random_float();
	float cosTheta2 = (1 - Xi1) / (Xi1 * (alpha * alpha - 1) + 1);
	float cosTheta = sqrtf(cosTheta2);
	float sinTheta = sqrtf(1 - cosTheta2);
	float phi = 2.0f * PI * Xi2;
	Vec3 h(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	return h;
}

//别名采样
class AliasMethod {
public:
	AliasMethod(const std::vector<float>& distribution = std::vector<float>()) {
		Init(distribution);
	}

public:
	void Init(const std::vector<float>& distribution);

	void Clear() { table.clear(); }

	// 0, 1, ..., n - 1
	int Sample() const;
	int Sample(float& p) const;

	float P(int i) const;

private:
	struct Item {
		Item() :u(-1), k(-1) {}

		float p; // orig probability
		float u; // choose probability
		int k; // alias
	};

	std::vector<Item> table;
};