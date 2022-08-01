#pragma once

#include "../Math/Math.hpp"
#include "../Ray/Ray.h"
#include "../Random/Random.h"

class Camera {
public:
	Camera() {}
	Camera(Vec3 lookfrom, Vec3 lookat, Vec3 vup, float vfov, float aspect, float aperture, float focus_dist) {
		origin = lookfrom;
		lens_radius = aperture / 2.0f;

		float theta = degrees_to_radians(vfov);
		float half_height = tan(theta / 2.0f);
		float half_width = aspect * half_height;

		w = vector_normalize<3, float>(lookfrom - lookat);
		u = vector_normalize<3, float>(vector_cross<3, float>(vup, w));
		v = vector_cross<3, float>(w, u);
		lower_left_corner = origin
			- half_width * focus_dist * u
			- half_height * focus_dist * v
			- focus_dist * w;

		horizontal = 2.0f * half_width * focus_dist * u;
		vertical = 2.0f * half_height * focus_dist * v;
	}

	inline Ray GetRay(float s, float t) {
		Vec3 rd = lens_radius * random_in_unit_disk();
		Vec3 offset = u * rd.x + v * rd.y;

		return Ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
	}

public:
	Vec3 origin;
	Vec3 lower_left_corner;
	Vec3 horizontal;
	Vec3 vertical;
	Vec3 u, v, w;
	float lens_radius;
};