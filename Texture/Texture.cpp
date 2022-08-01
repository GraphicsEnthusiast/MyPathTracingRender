#include "Texture.h"

Vec3 ConstantTexture::Value(const Vec2& uv, const Vec3& p) {
	return color;
}

Vec3 CheckerTexture::Value(const Vec2& uv, const Vec3& p) {
	float sines = sin(10.0f * p.x) * sin(10.0f * p.y) * sin(10.0f * p.z);
	if (sines < 0.0f) {
		return odd->Value(uv, p);
	}
	else {
		return even->Value(uv, p);
	}
}

Vec3 ImageTexture::Value(const Vec2& uv, const Vec3& p) {
	if (data == NULL) {
		std::cerr << "texture is null.\n";
		return Vec3(0.0f, 1.0f, 1.0f);
	}

	int i = static_cast<int>((uv.u) * nx);
	int j = static_cast<int>((1 - uv.v) * ny - 0.001f);

	if (i < 0) {
		i = 0;
	}
	if (j < 0) {
		j = 0;
	}
	if (i > nx - 1) {
		i = nx - 1;
	}
	if (j > ny - 1) {
		j = ny - 1;
	}

	float r = (data[nn * i + nn * nx * j + 0]) / 255.0f;
	float g = (data[nn * i + nn * nx * j + 1]) / 255.0f;
	float b = (data[nn * i + nn * nx * j + 2]) / 255.0f;

	return Vec3(r, g, b);
}
