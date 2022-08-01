#include "ONB.h"

void ONB::BuildFrom(const Vec3& n) {
	axis[2] = vector_normalize<3, float>(n);
	Vec3 a = (fabs(w().x) > 0.9f) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 0.0f, 0.0f);
	axis[1] = vector_normalize<3, float>(vector_cross<3, float>(w(), a));
	axis[0] = vector_cross<3, float>(w(), v());
}

void ONB::BuildFrom(const Vec3& n, const Vec3& t, const Vec3& b) {
	axis[2] = n;//z
	axis[0] = t;
	axis[1] = b;
}
