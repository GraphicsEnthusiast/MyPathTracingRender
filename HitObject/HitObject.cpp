#include "HitObject.h"

void Sphere::GetSphereUV(const Vec3& p, Vec2& uv) {
	float phi = atan2(p.z, p.x);
	float theta = asin(p.y);
	uv.u = 1.0f - (phi + PI) / (2.0f * PI);
	uv.v = (theta + PI / 2.0f) / PI;
}

bool Sphere::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) {
	Vec3 oc = ray.GetOrigin() - center;
	float a = vector_dot<3, float>(ray.GetDirection(), ray.GetDirection());
	float b = 2.0f * vector_dot<3, float>(oc, ray.GetDirection());
	float c = vector_dot<3, float>(oc, oc) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant > 0.0f) {
		float root = sqrt(discriminant);
		float temp = (-b - root) / (2.0f * a);
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = ray.At(rec.t);
			Vec3 outward_normal = (rec.p - center) / radius;
			rec.SetFaceNormal(ray, outward_normal);
			rec.mat_ptr = mat_ptr;
			GetSphereUV(outward_normal, rec.uv);

			return true;
		}
		temp = (-b + root) / (2.0f * a);
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = ray.At(rec.t);
			Vec3 outward_normal = (rec.p - center) / radius;
			rec.SetFaceNormal(ray, outward_normal);
			rec.mat_ptr = mat_ptr;
			GetSphereUV(outward_normal, rec.uv);

			return true;
		}
	}
	return false;
}

float HitObjectList::PDF_Value(const Vec3& o, const Vec3& v) {
	float weight = 1.0f / objects.size();
	float sum = 0.0f;

	for (const auto& object : objects) {
		sum += weight * object->PDF_Value(o, v);
	}

	return sum;
}

Vec3 HitObjectList::Sample(const Vec3& o) {
	int int_size = static_cast<int>(objects.size());

	return objects[static_cast<int>(random_float(0, int_size - 1))]->Sample(o);
}

bool HitObjectList::Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) {
	HitRecord temp_rec;
	bool hit_anything = false;
	float closest_so_far = t_max;

	for (const auto& object : objects) {
		if (object->Hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}

bool AABB::Hit(const Ray& ray, float t_min, float t_max) {
	for (int a = 0; a < 3; a++) {
		float t0 = value_min<float>((AA[a] - ray.GetOrigin()[a]) / ray.GetDirection()[a], (BB[a] - ray.GetOrigin()[a]) / ray.GetDirection()[a]);
		float t1 = value_max<float>((AA[a] - ray.GetOrigin()[a]) / ray.GetDirection()[a], (BB[a] - ray.GetOrigin()[a]) / ray.GetDirection()[a]);
		t_min = value_max<float>(t0, t_min);
		t_max = value_min<float>(t1, t_max);
		if (t_max <= t_min) {
			return false;
		}
	}
	return true;
}

bool Sphere::BoundingBox(float t0, float t1, AABB& output_box) {
	output_box = AABB(center - Vec3(radius, radius, radius), center + Vec3(radius, radius, radius));

	return true;
}

float Sphere::PDF_Value(const Vec3& o, const Vec3& v) {
	HitRecord rec;
	if (!this->Hit(Ray(o, v), 0.0001f, INF, rec)) {
		return 0;
	}

	float cos_theta_max = sqrtf(1 - radius * radius / vector_length_square<3, float>(center - o));
	float solid_angle = 2.0f * PI * (1 - cos_theta_max);

	return  1.0f / solid_angle;
}

Vec3 Sphere::Sample(const Vec3& o) {
	Vec3 direction = center - o;
	float distance_squared = vector_length_square<3, float>(direction);
	ONB uvw(direction);
	return uvw.LocalToGlobal(random_to_sphere(radius, distance_squared));
}

bool HitObjectList::BoundingBox(float t0, float t1, AABB& output_box) {
	if (objects.empty()) {
		return false;
	}

	AABB temp_box;
	bool first_box = true;

	for (const auto& object : objects) {
		if (!object->BoundingBox(t0, t1, temp_box)) {
			return false;
		}
		output_box = first_box ? temp_box : SurroundingBox(output_box, temp_box);
		first_box = false;
	}

	return true;
}

void XZ_Rect::GetXZ_RectUV(const Vec3& p, Vec2& uv) {
	float x_len = fabs(x1 - x0);
	float z_len = fabs(z1 - z0);
	float px_len = fabs(p.x - x0);
	float pz_len = fabs(p.z - z0);

	uv.u = px_len / x_len;
	uv.v = pz_len / z_len;
}

bool XZ_Rect::Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) {
	float t = (k - r.GetOrigin().y) / r.GetDirection().y;
	if (t < t_min || t > t_max) {
		return false;
	}

	float x = r.GetOrigin().x + t * r.GetDirection().x;
	float z = r.GetOrigin().z + t * r.GetDirection().z;
	if (x < x0 || x > x1 || z < z0 || z > z1) {
		return false;
	}

	rec.uv.u = (x - x0) / (x1 - x0);
	rec.uv.v = (z - z0) / (z1 - z0);
	rec.t = t;
	Vec3 outward_normal = Vec3(0.0f, 1.0f, 0.0f);
	rec.SetFaceNormal(r, outward_normal);
	rec.mat_ptr = mp;
	rec.p = r.At(t);
	GetXZ_RectUV(rec.p, rec.uv);

	return true;
}

bool XZ_Rect::BoundingBox(float time0, float time1, AABB& output_box) {
	//y轴方向无宽度，给一个极小的y值
	output_box = AABB(Vec3(x0, k - 0.0001f, z0), Vec3(x1, k + 0.0001f, z1));
	return true;
}

float XZ_Rect::PDF_Value(const Vec3& o, const Vec3& v) {
	HitRecord rec;
	if (!this->Hit(Ray(o, v), 0.0001f, INF, rec)) {
		return 1.0f;
	}

	float area = (x1 - x0) * (z1 - z0);
	float distance_squared = rec.t * rec.t * vector_length_square<3, float>(v);
	float cosine = fabs(vector_dot<3, float>(v, rec.normal) / vector_length<3, float>(v));

	return distance_squared / (cosine * area);
}

Vec3 XZ_Rect::Sample(const Vec3& origin) {
	Vec3 random_point = Vec3(random_float(x0, x1), k, random_float(z0, z1));

	return random_point - origin;
}
