#pragma once

#include <memory>
#include <vector>
#include <cmath>
#include <iostream>

#include "../Math/Math.hpp"
#include "../Ray/Ray.h"
#include "../ONB/ONB.h"
#include "../Sample/Sample.h"
#include "../Random//Random.h"

using std::shared_ptr;
using std::make_shared;

//前向声明
class Material;

//包围盒
class AABB {
public:
	Vec3 AA;//小
	Vec3 BB;//大

public:
	AABB() { AA = BB = Vec3(0.0f, 0.0f, 0.0f); }
	AABB(const Vec3& a, const Vec3& b) { AA = a; BB = b; }

	inline Vec3 GetMin() const { return AA; }
	inline Vec3 GetMax() const { return BB; }
	inline float GetArea() { return 2.0f * (fabs(BB.x - AA.x) * fabs(BB.y - AA.y) + fabs(BB.x - AA.x) * fabs(BB.z - AA.z) + fabs(BB.y - AA.y) * fabs(BB.z - AA.z)); }

	//光线和包围盒求交
	bool Hit(const Ray& ray, float t_min, float t_max);
};

struct HitRecord {
	Vec3 p;
	Vec3 normal;
	//Vec3 normal_from_map;
	float t;
	Vec2 uv = Vec2(0.0f, 0.0f);
	bool front_face;
	shared_ptr<Material> mat_ptr;

	inline void SetFaceNormal(const Ray& ray, const Vec3& outward_normal) {
		front_face = vector_dot<3, float>(ray.GetDirection(), outward_normal) < 0;
		normal = front_face ? outward_normal : 0.0f - outward_normal;
	}
};

//与光线求交物体的虚基类
class HitObject {
public:
	virtual bool Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) = 0;
	virtual bool BoundingBox(float t0, float t1, AABB& output_box) = 0;
	virtual float PDF_Value(const Vec3& o, const Vec3& v) {
		return 1.0f;
	}
	virtual Vec3 Sample(const Vec3& o) {
		return Vec3(1.0f, 1.0f, 1.0f);
	}
};


//球
class Sphere :public HitObject {
public:
	Vec3 center;
	float radius;
	shared_ptr<Material> mat_ptr;

public:
	Sphere(Vec3 c, float r, shared_ptr<Material> m) {
		center = c;
		radius = r;
		mat_ptr = m;
	}

	void GetSphereUV(const Vec3& p, Vec2& uv);
	virtual bool Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) override;
	virtual bool BoundingBox(float t0, float t1, AABB& output_box);
	virtual float PDF_Value(const Vec3& o, const Vec3& v) override;
	virtual Vec3 Sample(const Vec3& o) override;
};

class XZ_Rect :public HitObject {
public:
	XZ_Rect() {}

	XZ_Rect(float _x0, float _x1, float _z0, float _z1, float _k, shared_ptr<Material> mat)
		: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

	void GetXZ_RectUV(const Vec3& p, Vec2& uv);
	virtual bool Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) override;
	virtual bool BoundingBox(float time0, float time1, AABB& output_box) override;
	virtual float PDF_Value(const Vec3& o, const Vec3& v) override;
	virtual Vec3 Sample(const Vec3& o) override;

public:
	shared_ptr<Material> mp;
	float x0, x1, z0, z1, k;
};


class HitObjectList :public HitObject {
public:
	HitObjectList() {}
	HitObjectList(shared_ptr<HitObject> object) { Add(object); }

	inline void Clear() { objects.clear(); }
	inline void Add(shared_ptr<HitObject> object) { objects.push_back(object); }

	virtual bool Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) override;
	virtual bool BoundingBox(float t0, float t1, AABB& output_box);
	virtual float PDF_Value(const Vec3& o, const Vec3& v) override;
	virtual Vec3 Sample(const Vec3& o) override;

public:
	std::vector<shared_ptr<HitObject>> objects;
};

//返回包围box0和box1的包围盒
inline AABB SurroundingBox(AABB box0, AABB box1) {
	float xmin = value_min<float>(box0.GetMin().x, box1.GetMin().x);
	float ymin = value_min<float>(box0.GetMin().y, box1.GetMin().y);
	float zmin = value_min<float>(box0.GetMin().z, box1.GetMin().z);

	float xmax = value_max<float>(box0.GetMax().x, box1.GetMax().x);
	float ymax = value_max<float>(box0.GetMax().y, box1.GetMax().y);
	float zmax = value_max<float>(box0.GetMax().z, box1.GetMax().z);

	Vec3 box_small(xmin, ymin, zmin);
	Vec3 box_big(xmax, ymax, zmax);

	return AABB(box_small, box_big);
}

inline AABB AABBUnion(AABB const& box1, AABB const& box2) {
	AABB tbox;
	tbox.AA = vector_min<3, float>(box1.GetMin(), box2.GetMin());
	tbox.BB = vector_max<3, float>(box1.GetMax(), box2.GetMax());
	return tbox;
}