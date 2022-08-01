#pragma once

#include <memory>
#include <vector>
#include <iostream>

#include "../Math/Math.hpp"

using std::shared_ptr;
using std::make_shared;

class Texture {
public:
	virtual Vec3 Value(const Vec2& uv, const Vec3& p) = 0;
};

class ConstantTexture : public Texture {
public:
	ConstantTexture() {}
	ConstantTexture(Vec3 c) : color(c) {}

	virtual Vec3 Value(const Vec2& uv, const Vec3& p) override;

public:
	Vec3 color;
};

class CheckerTexture :public Texture {
public:
    CheckerTexture() {}
    CheckerTexture(shared_ptr<Texture> t0, shared_ptr<Texture> t1): even(t0), odd(t1) {}

    virtual Vec3 Value(const Vec2& uv, const Vec3& p) override;

public:
	shared_ptr<Texture> odd;
	shared_ptr<Texture> even;
};

class ImageTexture :public Texture {
public:
	ImageTexture() {}
	ImageTexture(unsigned char* pixels, int A, int B, int N)
		: data(pixels), nx(A), ny(B), nn(N) {}

	~ImageTexture() {
		delete data;
	}

	virtual Vec3 Value(const Vec2& uv, const Vec3& p = Vec3(0.0f, 0.0f, 0.0f)) override;

public:
	unsigned char* data;
	int nx, ny, nn;
};