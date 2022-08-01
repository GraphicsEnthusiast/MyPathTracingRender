#include "Material.h"
#include <vector>

using namespace std;

Vec3 Reflect(const Vec3& v, const Vec3& n) {
	return v - 2.0f * vector_dot<3, float>(v, n) * n;
}

Vec3 Refract(const Vec3& uv, const Vec3& n, float etai_over_etat) {
	auto cos_theta = vector_dot<3, float>(0.0f - uv, n);
	Vec3 r_out_parallel = etai_over_etat * (uv + cos_theta * n);
	Vec3 r_out_perp = -sqrtf(1.0f - vector_length_square<3, float>(r_out_parallel)) * n;
	return r_out_parallel + r_out_perp;
}

float Schlick(float cosine, float ref_idx) {
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

bool Lambertian::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 Lambertian::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	Vec3 N = vector_normalize<3, float>(rec.normal);
	Vec3 bsdf = albedo_t->Value(rec.uv, rec.p) / PI;
	float costheta = vector_dot<3, float>(N, vector_normalize<3, float>(L.GetDirection()));
	costheta = (costheta < 0.0f) ? 0.0f : costheta;

	return bsdf * costheta;
}

Vec3 Lambertian::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.Sample(V, rec, L);
}

float Lambertian::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.PDF_Value(V, rec, L);
}

bool DiffuseLight::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	return false;
}

Vec3 DiffuseLight::Emitted(const Ray& ray, HitRecord& rec) {
	return emit->Value(rec.uv, rec.p);
}

Vec3 DiffuseLight::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.Sample(V, rec, L);
}

Vec3 DiffuseLight::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return Vec3(1.0f, 1.0f, 1.0f);
}

float DiffuseLight::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.PDF_Value(V, rec, L);
}

bool Metal::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 Metal::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	Vec3 reflected = Reflect(vector_normalize<3, float>(V.GetDirection()), rec.normal);

	return reflected + fuzz * random_in_unit_sphere();
}

Vec3 Metal::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return albedo_t->Value(rec.uv, rec.p);
}

float Metal::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return 1.0f;
}

bool Dielectric::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 Dielectric::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	float etai_over_etat = (rec.front_face) ? (1.0f / ref_idx) : (ref_idx);

	Vec3 unit_direction = vector_normalize<3, float>(V.GetDirection());
	float cos_theta = value_min<float>(vector_dot<3, float>(0.0f - unit_direction, rec.normal), 1.0f);
	float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);
	if (etai_over_etat * sin_theta > 1.0f) {
		Vec3 reflected = Reflect(unit_direction, rec.normal);

		return reflected;
	}
	float reflect_prob = Schlick(cos_theta, etai_over_etat);
	if (random_float() < reflect_prob) {
		Vec3 reflected = Reflect(unit_direction, rec.normal);

		return reflected;
	}
	Vec3 refracted = Refract(unit_direction, rec.normal, etai_over_etat);
	
	return refracted;
}

Vec3 Dielectric::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return albedo_t->Value(rec.uv, rec.p);
}

float Dielectric::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return 1.0f;
}

//wi：物体表面指向摄像机
//wo：物体表面指向光源
Vec3 LambertianMicrofacet_Beckmann::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	kd = albedo_t->Value(rec.uv, rec.p);
	float roughness = roughness_t->Value(rec.uv, rec.p).r;
	float metallic = metallic_t->Value(rec.uv, rec.p).r;
	float ao = ao_t->Value(rec.uv, rec.p).r;

	float x = kd.x, y = kd.y, z = kd.z;
	if (x > y && x > z) {
		ks = x;
	}
	else if (y > z) {
		ks = y;
	}
	else {
		ks = z;
	}

	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 wo = vector_normalize<3, float>(L.GetDirection());
	Vec3 n = vector_normalize<3, float>(rec.normal);

	//Vec3 tangent_normal = normal_t->Value(rec.uv, rec.p);
	//tangent_normal = vector_normalize<3, float>(tangent_normal * 0.5f + 0.5f);
	//ONB onb(n);
	//Vec3 normal = onb.LocalToGlobal(tangent_normal);
	//if (vector_length_square<3, float>(normal) < 0.1f) {
	//	normal = n;
	//}
	//rec.normal_from_map = normal;
	Vec3 normal = n;

	if (vector_dot<3, float>(normal, wo) < 0.0f || vector_dot<3, float>(normal, wi) < 0.0f) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}
	Vec3 wh = vector_normalize<3, float>(wo + wi);
	float cosThetaI = vector_dot<3, float>(normal, wi);
	float cosThetaO = vector_dot<3, float>(normal, wo);
	float cosThetaH = vector_dot<3, float>(normal, wh);

	//diffuse
	//float VoH = vector_dot<3, float>(wi, wh);
	//float a = roughness * roughness;
	//float s = a;// / ( 1.29 + 0.5 * a );
	//float s2 = s * s;
	//float VoL = 2.0f * VoH * VoH - 1.0f;		// float angle identity
	//float Cosri = VoL - cosThetaI * cosThetaO;
	//float C1 = 1.0f - 0.5f * s2 / (s2 + 0.33f);
	//float C2 = 0.45f * s2 / (s2 + 0.09f) * Cosri * (Cosri >= 0.0f ? (value_max<float>(cosThetaO, cosThetaI)) : 1.0f);

	//specular
    //Beckmann
	//NDF，角度转换避免反三角函数带来的开销
	float azimutal = 1.0f / PI;
	float longitudinal = exp(-(1 - cosThetaH * cosThetaH) / (cosThetaH * cosThetaH * roughness * roughness))
		/ (roughness * roughness * cosThetaH * cosThetaH * cosThetaH * cosThetaH);
	float D = azimutal * longitudinal;

	//Fresnel
	//float F = FresnelSchlick(vector_dot<3, float>(wh, wi), intIOR, extIOR);
	Vec3 F = FresnelSchlick_F0(wi, wh, kd, metallic);

	//Beckmann
	//G
	float G1 = 0.0f, G2 = 0.0f;
	if (vector_dot<3, float>(wh, wi) / cosThetaI > 0.0f) {
		float b = cosThetaI / (sqrtf(1 - cosThetaI * cosThetaI) * roughness);
		if (b < 1.6f) {
			G1 = (3.535f * b + 2.181f * b * b) / (1.0f + 2.276f * b + 2.577f * b * b);
		}
		else {
			G1 = 1.0f;
		}
	}
	if (vector_dot<3, float>(wh, wo) / cosThetaO > 0.0f) {
		float b = cosThetaO / (sqrtf(1 - cosThetaO * cosThetaO) * roughness);
		if (b < 1.6f) {
			G2 = (3.535f * b + 2.181f * b * b) / (1.0f + 2.276f * b + 2.577f * b * b);
		}
		else {
			G2 = 1.0f;
		}
	}
	float G = G1 * G2;

	Vec3 bsdf = kd / PI + ks * D * F * G / (4.0f * cosThetaI * cosThetaO + 0.001f);
	bsdf = ao * bsdf;

	return bsdf * cosThetaO;
}

float LambertianMicrofacet_Beckmann::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.PDF_Value(V, rec, L);
}

bool LambertianMicrofacet_Beckmann::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 LambertianMicrofacet_Beckmann::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.Sample(V, rec, L);
}

bool OrenNayar::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 OrenNayar::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.Sample(V, rec, L);
}

Vec3 OrenNayar::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 wo = vector_normalize<3, float>(L.GetDirection());
	Vec3 wh = vector_normalize<3, float>(wo + wi);
	Vec3 n = vector_normalize<3, float>(rec.normal);

	float VoH = vector_dot<3, float>(wi, wh);
	float NoV = vector_dot<3, float>(n, wi);
	float NoL = vector_dot<3, float>(n, wo);
	if (NoV < 0.0f || NoL < 0.0f) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	Vec3 color = albedo_t->Value(rec.uv, rec.p);

	float roughness = roughness_t->Value(rec.uv, rec.p).r;
	float a = roughness * roughness;
	float s = a;// / ( 1.29 + 0.5 * a );
	float s2 = s * s;
	float VoL = 2.0f * VoH * VoH - 1.0f;		// float angle identity
	float Cosri = VoL - NoV * NoL;
	float C1 = 1.0f - 0.5f * s2 / (s2 + 0.33f);
	float C2 = 0.45f * s2 / (s2 + 0.09f) * Cosri * (Cosri >= 0.0f ? (value_max<float>(NoL, NoV)) : 1.0f);
	return color / PI * (C1 + C2) * (1.0f + roughness * 0.5f);
}

float OrenNayar::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.PDF_Value(V, rec, L);
}

bool OrenNayarMicrofacet_GGX::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	L = Ray(rec.p, this->Sample(V, rec, L, pdf));

	return true;
}

Vec3 OrenNayarMicrofacet_GGX::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.Sample(V, rec, L);
}

Vec3 OrenNayarMicrofacet_GGX::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	kd = albedo_t->Value(rec.uv, rec.p);
	float roughness = roughness_t->Value(rec.uv, rec.p).r;
	float metallic = metallic_t->Value(rec.uv, rec.p).r;
	float ao = ao_t->Value(rec.uv, rec.p).r;

	float x = kd.x, y = kd.y, z = kd.z;
	if (x > y && x > z) {
		ks = x;
	}
	else if (y > z) {
		ks = y;
	}
	else {
		ks = z;
	}

	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 wo = vector_normalize<3, float>(L.GetDirection());
	Vec3 n = vector_normalize<3, float>(rec.normal);

	//Vec3 tangent_normal = normal_t->Value(rec.uv, rec.p);
	//tangent_normal = vector_normalize<3, float>(tangent_normal * 0.5f + 0.5f);
	//ONB onb(n);
	//Vec3 normal = onb.LocalToGlobal(tangent_normal);
	//if (vector_length_square<3, float>(normal) < 0.1f) {
	//	normal = n;
	//}
	//rec.normal_from_map = normal;
	Vec3 normal = n;

	if (vector_dot<3, float>(normal, wo) < 0.0f || vector_dot<3, float>(normal, wi) < 0.0f) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}
	Vec3 wh = vector_normalize<3, float>(wo + wi);
	float cosThetaI = vector_dot<3, float>(normal, wi);
	float cosThetaO = vector_dot<3, float>(normal, wo);
	float cosThetaH = vector_dot<3, float>(normal, wh);

	//diffuse
	float VoH = vector_dot<3, float>(wi, wh);
	float a = roughness * roughness;
	float s = a;// / ( 1.29 + 0.5 * a );
	float s2 = s * s;
	float VoL = 2.0f * VoH * VoH - 1.0f;		// float angle identity
	float Cosri = VoL - cosThetaI * cosThetaO;
	float C1 = 1.0f - 0.5f * s2 / (s2 + 0.33f);
	float C2 = 0.45f * s2 / (s2 + 0.09f) * Cosri * (Cosri >= 0.0f ? (value_max<float>(cosThetaO, cosThetaI)) : 1.0f);

	//specular
	//GGX
	//NDF，角度转换避免反三角函数带来的开销
	float D = NDF(wh, n, roughness);

	//Fresnel
	//float F = FresnelSchlick(vector_dot<3, float>(wh, wi), intIOR, extIOR);
	Vec3 F = FresnelSchlick_F0(wi, wh, kd, metallic);

	//GGX
	//G
	float G = Ge(wo, wi, n, roughness);

	Vec3 bsdf = kd / PI * (C1 + C2) * (1.0f + roughness * 0.5f) + ks * D * F * G / (4.0f * cosThetaI * cosThetaO + 0.001f);
	bsdf = ao * bsdf;

	return bsdf * cosThetaO;
}

float OrenNayarMicrofacet_GGX::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return pdf.PDF_Value(V, rec, L);
}

InfiniteAreaLight::InfiniteAreaLight(shared_ptr<ImageTexture> a) {
	img = a;

	aliasMethod.Clear();

	if (!img) {
		return;
	}

	int w = img->nx;
	int h = img->ny;
	vector<float> distribution(w * h);
	float sum = 0.0f;
	for (int y = 0; y < h; y++) {
		float sinTheta = sin(PI * (y + 0.5f) / static_cast<float>(h));
		for (int x = 0; x < w; x++) {
			int idx = x + y * w;
			Vec2 uv(x, y);
			Vec3 rgb = img->Value(uv);
			distribution[idx] = sinTheta * (rgb.r * 0.2126f + rgb.g * 0.7152f + rgb.b * 0.0722f);
			sum += distribution[idx];
		}
	}

	for (auto& p : distribution) {
		p /= sum;
	}

	aliasMethod.Init(distribution);
}

bool InfiniteAreaLight::Scatter(const Ray& V, HitRecord& rec, Ray& L, PDF& pdf) {
	return false;
}

Vec3 InfiniteAreaLight::Emitted(const Ray& ray, HitRecord& rec) {
	return Vec3();
}

Vec3 InfiniteAreaLight::Sample(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return Vec3();
}

Vec3 InfiniteAreaLight::BSDF_Cos(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return Vec3();
}

float InfiniteAreaLight::PDF_Value(const Ray& V, HitRecord& rec, const Ray& L, PDF& pdf) {
	return 0.0f;
}
