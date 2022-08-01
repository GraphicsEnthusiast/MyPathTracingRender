#pragma once

#include "../Math/Math.hpp"
#include "../Random/Random.h"
#include "../ONB/ONB.h"
#include "../Ray/Ray.h"
#include "../HitObject/HitObject.h"

class PDF {
public:
	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) = 0;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) = 0;
};

class COSPDF :public PDF {
public:
	COSPDF(Vec3 n) { uvw.BuildFrom(n); }

	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) override;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) override;

public:
	ONB uvw;
};

class BeckmannPDF :public PDF {
public:
	//BeckmannPDF(float r, float _ks, Vec3 n): roughness(r), ks(_ks), normal_form_texture(n){}
	BeckmannPDF(float r, float _ks) : roughness(r), ks(_ks){}

	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) override;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) override;

public:
	float roughness;
	float ks;
	//Vec3 normal_form_texture;
};

class GGXPDF :public PDF {
public:
	//BeckmannPDF(float r, float _ks, Vec3 n): roughness(r), ks(_ks), normal_form_texture(n){}
	GGXPDF(float r, float _ks) : roughness(r), ks(_ks) {}

	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) override;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) override;

public:
	float roughness;
	float ks;
	//Vec3 normal_form_texture;
};

class HitObjectPDF :public PDF {
public:
	HitObjectPDF(shared_ptr<HitObject> p, const Vec3& o) : ptr(p), origin(o) {}

	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) override;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) override;

public:
	Vec3 origin;
	shared_ptr<HitObject> ptr;
};

class MixturePDF :public PDF {
public:
	MixturePDF() {}
	MixturePDF(shared_ptr<PDF> p0, shared_ptr<PDF> p1) {
		p[0] = p0;
		p[1] = p1;
	}

	virtual Vec3 Sample(const Ray& V, const HitRecord& rec, const Ray& L) override;
	virtual float PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) override;

	inline void SetMix(float m = 0.5f) {
		mix = value_between<float>(0.0f, 1.0f, m);
	}

public:
	float mix;
	shared_ptr<PDF> p[2];
};