#pragma once

#include <iostream>
#include <algorithm>
#include <omp.h>//openmp���̼߳���

#include "../HitObject//HitObject.h"

class BVHNode :public HitObject {
public:
	BVHNode(){}

	BVHNode(std::vector<shared_ptr<HitObject>>& objects, size_t start, size_t end, float t_min, float t_max);

	virtual bool Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) override;
	virtual bool BoundingBox(float t0, float t1, AABB& output_box) override;

public:
	//BVH�ڵ�Ҳ��HitObject
	shared_ptr<HitObject> left;
	shared_ptr<HitObject> right;
	AABB box;
};