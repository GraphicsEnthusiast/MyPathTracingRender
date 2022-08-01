#pragma once

#include "../Math/Math.hpp"
#include "../ONB/ONB.h"
#include "../PDF/PDF.h"
#include "../Ray/Ray.h"
#include "../HitObject/HitObject.h"
#include "../Texture/Texture.h"

enum MATERIALTYPE {
	DIFFUSE,
	SPECULAR_GGX,
	SPECULAR_Beckmann,
	OTHER
};

class Material {
public:
	virtual Vec3 Emitted(const Ray& ray, HitRecord& rec) { return Vec3(0.0f, 0.0f, 0.0f); }
	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) = 0;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) = 0;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) = 0;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) = 0;

	virtual float GetRoughness(HitRecord& rec) {
		return 0.0f;
	}
	virtual float GetKs() {
		return 0.0f;
	}
	virtual Vec3 GetNormalFromTexture(HitRecord& rec) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

public:
	MATERIALTYPE m_type = OTHER;
};

class Metal :public Material {
public:
	Metal(const shared_ptr<Texture>& a, float f) : albedo_t(a), fuzz(f < 1.0f ? f : 1.0f) {}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<Texture> albedo_t;
	float fuzz;
};

class Dielectric :public Material {
public:
	Dielectric(shared_ptr<Texture> a, float ri) : ref_idx(ri), albedo_t(a) {}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<Texture> albedo_t;
	float ref_idx;
};

class DiffuseLight :public Material {
public:
	DiffuseLight(shared_ptr<Texture> a) : emit(a) {}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Emitted(const Ray& ray, HitRecord& rec) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<Texture> emit;
};

class Lambertian :public Material {
public:
	Lambertian(shared_ptr<Texture> a)
		: albedo_t(a) {
		m_type = DIFFUSE;
	}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<Texture> albedo_t;
};

class OrenNayar :public Material {
public:
	OrenNayar(shared_ptr<Texture> a, shared_ptr<Texture> r)
		: albedo_t(a), roughness_t(r) {
		m_type = DIFFUSE;
	}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<Texture> albedo_t;
	shared_ptr<Texture> roughness_t;
};

class LambertianMicrofacet_Beckmann :public Material {
public:
	LambertianMicrofacet_Beckmann(shared_ptr<Texture> a, shared_ptr<Texture> r, shared_ptr<Texture> m, shared_ptr<Texture> n, shared_ptr<Texture> ao, float intior = 1.5f, float extior = 1.0f):
		roughness_t(r), metallic_t(m), normal_t(n), ao_t(ao), intIOR(intior), extIOR(extior), albedo_t(a) {
		m_type = SPECULAR_Beckmann;
	}

	static inline float FresnelSchlick(float cosine, float intIOR, float extIOR) {
		float etaI = intIOR, etaT = extIOR;
		float cosThetaI = cosine;

		if (extIOR == intIOR) {
			return 0.0f;
		}

		/* Swap the indices of refraction if the interaction starts
		   at the inside of the object */
		if (cosThetaI < 0.0f) {
			std::swap(etaI, etaT);
			cosThetaI = -cosThetaI;
		}

		/* Using Snell's law, calculate the squared sine of the
		   angle between the normal and the transmitted ray */
		float eta = etaI / etaT,
			sinThetaTSqr = eta * eta * (1 - cosThetaI * cosThetaI);

		if (sinThetaTSqr > 1.0f) {
			return 1.0f;/* Total internal reflection! */
		}

		float cosThetaT = std::sqrtf(1.0f - sinThetaTSqr);

		float Rs = (etaI * cosThetaI - etaT * cosThetaT)
			/ (etaI * cosThetaI + etaT * cosThetaT);
		float Rp = (etaT * cosThetaI - etaI * cosThetaT)
			/ (etaT * cosThetaI + etaI * cosThetaT);

		return (Rs * Rs + Rp * Rp) / 2.0f;
	}
	static inline Vec3 FresnelSchlick_F0(const Vec3& wi, const Vec3& h, const Vec3& albedo, float metallic) {
		//Schlick近似
        //使用球面高斯近似代替幂。
        //计算效率略高，差异不明显
		Vec3 F0 = vector_lerp<3, float>(Vec3(0.04f, 0.04f, 0.04f), albedo, metallic);
		float HoWi = vector_dot<3, float>(h, wi);
		return F0 + (Vec3(1.0f, 1.0f, 1.0f) - F0) * pow(2.0f, (-5.55473f * HoWi - 6.98316f) * HoWi);
	}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

	virtual float GetRoughness(HitRecord& rec) override {
		return roughness_t->Value(rec.uv, rec.p).r;
	}
	virtual float GetKs() override {
		return ks;
	}
	virtual Vec3 GetNormalFromTexture(HitRecord& rec) override {
		return normal_t->Value(rec.uv, rec.p);
	}

public:
	float intIOR, extIOR;
	float ks;
	Vec3 kd;
	shared_ptr<Texture> albedo_t;
	shared_ptr<Texture> roughness_t;
	shared_ptr<Texture> metallic_t;
	shared_ptr<Texture> normal_t;
	shared_ptr<Texture> ao_t;
};

class OrenNayarMicrofacet_GGX :public Material {
public:
	OrenNayarMicrofacet_GGX(shared_ptr<Texture> a, shared_ptr<Texture> r, shared_ptr<Texture> m, shared_ptr<Texture> n, shared_ptr<Texture> ao) :
		roughness_t(r), metallic_t(m), normal_t(n), ao_t(ao), albedo_t(a) {
		m_type = SPECULAR_GGX;
	}

	static inline Vec3 FresnelSchlick_F0(const Vec3& wi, const Vec3& h, const Vec3& albedo, float metallic) {
		//Schlick近似
		//使用球面高斯近似代替幂。
		//计算效率略高，差异不明显
		Vec3 F0 = vector_lerp<3, float>(Vec3(0.04f, 0.04f, 0.04f), albedo, metallic);
		float HoWi = vector_dot<3, float>(h, wi);
		return F0 + (Vec3(1.0f, 1.0f, 1.0f) - F0) * pow(2.0f, (-5.55473f * HoWi - 6.98316f) * HoWi);
	}

	static inline float NDF(const Vec3& h, const Vec3& n, float roughness) {
		//  GGX / Trowbridge-Reitz
		float alpha = roughness * roughness;
		float alpha2 = alpha * alpha;
		float NoH = vector_dot<3, float>(h, n);
		return alpha2 / (PI * pow(NoH * NoH * (alpha2 - 1.0f) + 1.0f, 2.0f));
	}

	static inline float Ge(const Vec3& wo, const Vec3& wi, const Vec3& n, float roughness) {
		// Schlick, remap roughness and k

		// k = alpha / 2
		// direct light: alpha = pow( (roughness + 1) / 2, 2)
		// IBL(image base lighting) : alpha = pow( roughness, 2)

		float cosThetaI = vector_dot<3, float>(n, wi);
		float cosThetaO = vector_dot<3, float>(n, wo);


		if (cosThetaI <= 0.0f || cosThetaO <= 0.0f) {
			return 0;
		}

		float k = pow(roughness + 1.0f, 2.0f) / 8.0f;
		float G1_wo = cosThetaO / (cosThetaO * (1 - k) + k);
		float G1_wi = cosThetaI / (cosThetaI * (1 - k) + k);
		return G1_wo * G1_wi;
	}

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

	virtual float GetRoughness(HitRecord& rec) override {
		return roughness_t->Value(rec.uv, rec.p).r;
	}
	virtual float GetKs() override {
		return ks;
	}
	virtual Vec3 GetNormalFromTexture(HitRecord& rec) override {
		return normal_t->Value(rec.uv, rec.p);
	}

public:
	float ks;
	Vec3 kd;
	shared_ptr<Texture> albedo_t;
	shared_ptr<Texture> roughness_t;
	shared_ptr<Texture> metallic_t;
	shared_ptr<Texture> normal_t;
	shared_ptr<Texture> ao_t;
};

class InfiniteAreaLight :public Material {
public:
	InfiniteAreaLight(shared_ptr<ImageTexture> a);

	virtual bool Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) override;
	virtual Vec3 Emitted(const Ray& ray, HitRecord& rec) override;
	virtual Vec3 Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual Vec3 BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;
	virtual float PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) override;

public:
	shared_ptr<ImageTexture> img;
	AliasMethod aliasMethod;
};