#include "Sample.h"

using namespace std;

void AliasMethod::Init(const vector<float>& distribution) {
	table.clear();
	const auto num = static_cast<int>(distribution.size());
	table.resize(num);

	vector<int> overfulls;
	vector<int> underfulls;
	overfulls.reserve(num);
	underfulls.reserve(num);

	for (int i = 0; i < num; i++) {
		//默认设置为i
		table[i].k = static_cast<int>(i);

		table[i].p = distribution[i];
		table[i].u = num * distribution[i];

		if (table[i].u < 1) {
			underfulls.push_back(i);
		}
		if (table[i].u > 1) {
			overfulls.push_back(i);
		}
	}

	while (!overfulls.empty() && !underfulls.empty()) {
		int i = overfulls.back();
		overfulls.pop_back();

		int j = underfulls.back();
		underfulls.pop_back();

		table[i].u -= 1 - table[j].u;
		table[j].k = i;

		if (table[i].u > 1) {
			overfulls.push_back(i);
		}
		if (table[i].u < 1) {
			underfulls.push_back(i);
		}
	}

	for (auto i : overfulls) {
		table[i].u = 1.0f;
	}

	for (auto j : underfulls) {
		table[j].u = 1.f;
	}
}

int AliasMethod::Sample() const {
	float x = random_float();
	auto n = static_cast<int>(table.size());
	auto nx = n * x;

	// 0, 1, ..., n - 1
	auto i = static_cast<int>(nx);

	// 因为 0 <= x <= 1，所以 i 可能为 n
	if (i == n) {
		i = n - 1;
	}

	// [0, 1)
	auto y = nx - i;

	if (y < table[i].u) {
		return i;
	}
	else {
		return table[i].k;
	}
}

int AliasMethod::Sample(float& p) const {
	int i = Sample();
	p = P(i);
	return i;
}

float AliasMethod::P(int i) const {
	assert(i >= 0 && i < static_cast<int>(table.size()));
	return table[i].p;
}