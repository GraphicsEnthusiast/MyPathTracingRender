#include "PDF.h"

Vec3 COSPDF::Sample(const Ray& V, const HitRecord& rec, const Ray& L) {
	return uvw.LocalToGlobal(random_cosine_direction());
}

float COSPDF::PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) {
	float cosine = vector_dot<3, float>(vector_normalize<3, float>(L.GetDirection()), uvw.w());

	return (cosine <= 0.0f) ? 0.0f : cosine / PI;
}

Vec3 HitObjectPDF::Sample(const Ray& V, const HitRecord& rec, const Ray& L) {
	return ptr->Sample(origin);
}

float HitObjectPDF::PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) {
	return ptr->PDF_Value(origin, L.GetDirection());
}

float MixturePDF::PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) {
	return mix * p[0]->PDF_Value(V, rec, L) + (1.0f - mix) * p[1]->PDF_Value(V, rec, L);
}

Vec3 MixturePDF::Sample(const Ray& V, const HitRecord& rec, const Ray& L) {
	if (random_float() < mix) {
		return p[0]->Sample(V, rec, L);
	}
	else {
		return p[1]->Sample(V, rec, L);
	}
}

Vec3 reflect(const Vec3& v, const Vec3& n) {
	return v - 2.0f * vector_dot<3, float>(v, n) * n;
}

Vec3 BeckmannPDF::Sample(const Ray& V, const HitRecord& rec, const Ray& L) {
	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 normal = vector_normalize<3, float>(rec.normal);

	if (vector_dot<3, float>(normal, wi) < 0.0f) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	float t = random_float();

	if (t > ks) {
		//diffuse
		ONB uvw(normal);
		Vec3 wo = uvw.LocalToGlobal(random_cosine_direction());
		return wo;
	}
	else {
		//specular
		//根据Beckmann模型采样微表面法线

		Vec3 n = vector_normalize<3, float>(square_to_Beckmann(roughness));

		//根据微表面法线算反射方向，用镜面反射原理
		float nor = vector_dot<3, float>(n, wi);//wi在n的投影长度
		Vec3 wo = reflect(0.0f - wi, n);

		if (vector_dot<3, float>(n, wo) <= 0.0f) {
			return Vec3(0.0f, 0.0f, 0.0f);
		}
		return wo;
	}
}

float BeckmannPDF::PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) {
	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 wo = vector_normalize<3, float>(L.GetDirection());
	Vec3 normal = vector_normalize<3, float>(rec.normal);

	if (vector_dot<3, float>(normal, wo) < 0.0f || vector_dot<3, float>(normal, wi) < 0.0f) {
		return 0.0f;
	}
	Vec3 wh = vector_normalize<3, float>(wo + wi);
	float cosThetaI = vector_dot<3, float>(normal, wi);
	float cosThetaO = vector_dot<3, float>(normal, wo);
	float cosThetaH = vector_dot<3, float>(normal, wh);

	float azimutal = 1.0f / PI;
	float longitudinal = exp(-(1 - cosThetaH * cosThetaH) / (cosThetaH * cosThetaH * roughness * roughness))
		/ (roughness * roughness * cosThetaH * cosThetaH * cosThetaH);
	float D = azimutal * longitudinal;

	float J = 4.0f * vector_dot<3, float>(wh, wo);

	float pdfvalue = ks * D / J + (1.0f - ks) * cosThetaO * azimutal;

	return pdfvalue;
}

Vec3 GGXPDF::Sample(const Ray& V, const HitRecord& rec, const Ray& L) {
	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 normal = vector_normalize<3, float>(rec.normal);

	if (vector_dot<3, float>(normal, wi) < 0.0f) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	float t = random_float();

	if (t > ks) {
		//diffuse
		ONB uvw(normal);
		Vec3 wo = uvw.LocalToGlobal(random_cosine_direction());
		return wo;
	}
	else {
		//specular
		float alpha = roughness * roughness;
		Vec3 h = square_to_GGX(alpha);
		Vec3 wo = reflect(0.0f - wi, h);

		if (vector_dot<3, float>(normal, wo) <= 0.0f) {
			return Vec3(0.0f, 0.0f, 0.0f);
		}
		return wo;
	}
}

float GGXPDF::PDF_Value(const Ray& V, const HitRecord& rec, const Ray& L) {
	Vec3 wi = vector_normalize<3, float>(0.0f - V.GetDirection());
	Vec3 wo = vector_normalize<3, float>(L.GetDirection());
	Vec3 n = vector_normalize<3, float>(rec.normal);
	if (vector_dot<3, float>(n, wo) < 0.0f || vector_dot<3, float>(n, wi) < 0.0f) {
		return 0.0f;
	}

	ONB uvw(n);
	float cosine = vector_dot<3, float>(vector_normalize<3, float>(L.GetDirection()), uvw.w());

	float diffuse_pdf = (cosine <= 0.0f) ? 0.0f : cosine / PI;

	Vec3 h = vector_normalize<3, float>(wo + wi);
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float NoH = vector_dot<3, float>(h, n);
	
	float specular_pdf = (alpha2 / (PI * pow(NoH * NoH * (alpha2 - 1.0f) + 1.0f, 2.0f))) / (4.0f * vector_dot<3, float>(h, wo));

	return (1.0f - ks) * diffuse_pdf + ks * specular_pdf;
}

