#include "BVH.h"

bool box_compare(const shared_ptr<HitObject> a, const shared_ptr<HitObject> b, int axis) {
	AABB box_a;
	AABB box_b;

	//判断有没有包围盒
	if (!a->BoundingBox(0.0f, 0.0f, box_a) || !b->BoundingBox(0.0f, 0.0f, box_b)) {
		std::cerr << "No bounding box in bvh_node constructor.\n";
	}

	return box_a.GetMin().m[axis] < box_b.GetMin().m[axis];
}

bool box_x_compare(const shared_ptr<HitObject> a, const shared_ptr<HitObject> b) {
	return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<HitObject> a, const shared_ptr<HitObject> b) {
	return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<HitObject> a, const shared_ptr<HitObject> b) {
	return box_compare(a, b, 2);
}

BVHNode::BVHNode(std::vector<shared_ptr<HitObject>>& objects, size_t start, size_t end, float t_min, float t_max) {
	float xlen = fabs(box.BB.x - box.AA.x);
	float ylen = fabs(box.BB.y - box.AA.y);
	float zlen = fabs(box.BB.z - box.AA.z);
	int axis;

	if (xlen > ylen && xlen > zlen) {
		axis = 0;
	}
	else if (ylen > zlen) {
		axis = 1;
	}
	else {
		axis = 2;
	}

	auto comparator = (axis == 0) ? box_x_compare
		: (axis == 1) ? box_y_compare
		: box_z_compare;

	//span: 跨度，范围
	size_t object_span = end - start + 1;

	if (object_span == 1) {
		left = right = objects[start];
	}
	else if (object_span == 2) {
		if (comparator(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else {
		//按某条轴来排序
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		//SAH，表面积启发算法
		int mincostIndex = 0;
		float minCost = INF;//最小花费

		omp_set_num_threads(32);//线程个数
#pragma omp parallel for
		for (int i = start; i < end; i++) {
			auto beginning = objects.begin() + start;
			auto middling = objects.begin() + i;
			auto ending = objects.begin() + end;
			auto leftshapes = std::vector<shared_ptr<HitObject>>(beginning, middling);
			auto rightshapes = std::vector<shared_ptr<HitObject>>(middling + 1, ending);

			AABB leftbox, rightbox;
			for (int k = 0; k < leftshapes.size(); k++) {
				AABB tbox;
				leftshapes[k]->BoundingBox(t_min, t_max, tbox);
				if (k == 0) {
					leftbox = tbox;
				}
				leftbox = AABBUnion(leftbox, tbox);
			}
			for (int k = 0; k < rightshapes.size(); k++) {
				AABB tbox;
				rightshapes[k]->BoundingBox(t_min, t_max, tbox);
				if (k == 0) {
					rightbox = tbox;
				}
				rightbox = AABBUnion(rightbox, tbox);
			}

			float sleft = leftbox.GetArea();
			float sright = rightbox.GetArea();

			float cost = leftshapes.size() * sleft + rightshapes.size() * sright;
			if (cost < minCost) {
				minCost = cost;
				mincostIndex = i;
			}
		}
		left = make_shared<BVHNode>(objects, start, mincostIndex, t_min, t_max);
		right = make_shared<BVHNode>(objects, mincostIndex + 1, end, t_min, t_max);

		//int mid = (start + end) / 2;
		
		//left = make_shared<BVHNode>(objects, start, mid, t_min, t_max);
		//right = make_shared<BVHNode>(objects, mid + 1, end, t_min, t_max);
	}

	AABB box_left, box_right;

	if (!left->BoundingBox(t_min, t_max, box_left) || !right->BoundingBox(t_min, t_max, box_right)) {
		std::cerr << "No bounding box in bvh_node constructor.\n";
	}

	box = SurroundingBox(box_left, box_right);
}

bool BVHNode::Hit(const Ray& r, float t_min, float t_max, HitRecord& rec) {
	if (!box.Hit(r, t_min, t_max)) {
		return false;
	}

	bool hit_left = left->Hit(r, t_min, t_max, rec);
	bool hit_right = right->Hit(r, t_min, hit_left ? rec.t : t_max, rec);

	return hit_left || hit_right;
}

bool BVHNode::BoundingBox(float t0, float t1, AABB& output_box) {
	output_box = box;

	return true;
}
