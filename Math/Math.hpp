#pragma once

#include <assert.h>
#include <math.h>
#include <limits>

const float INF = std::numeric_limits<float>::infinity();
const float PI = 3.1415926535f;
const float MAX = FLT_MAX;
const float MIN = FLT_MIN;

inline float degrees_to_radians(float degrees) {
	return degrees * PI / 180.0f;
}

//求最大值
template<typename T>
inline T value_max(T a, T b) {
	return a >= b ? a : b;
}

//求最小值
template<typename T>
inline T value_min(T a, T b) {
	return a <= b ? a : b;
}

template<typename T>
inline T value_between(T min, T max, T t) {
	if (t < min) {
		return min;
	}
	if (t > max) {
		return max;
	}
	return t;
}

//泛化矢量：N是矢量维度，T为数据类型
template<size_t N, typename T> 
struct Vec {
	T m[N];//元素数组

	inline Vec() {
		for (size_t i = 0; i < N; i++) {
			m[i] = T();
		}
	}
	inline Vec(const Vec<N, T>& v) {
		for (size_t i = 0; i < N; i++) {
			m[i] = v.m[i];
		}
	}
	inline Vec(const T* ptr) { 
		for (size_t i = 0; i < N; i++) {
			m[i] = ptr[i];
		}
	}
	inline Vec operator=(const Vec<N, T>& v) {
		for (size_t i = 0; i < N; i++) {
			this->m[i] = v.m[i];
		}
		return *this;
	}
	inline const T& operator[](size_t i) const {
		assert(i >= 0 && i < N);
		return m[i];
	}
};

//特化二维矢量
template<typename T>
struct Vec<2, T> {
	union {
		T m[2];
		struct {
			T x, y;
		};
		struct {
			T u, v;
		};
	};

	inline Vec() {
		x = T();
		y = T();
	}
	inline Vec(T _x, T _y) {
		x = _x;
		y = _y;
	}
	inline Vec(const Vec<2, T>& v) {
		x = v.x;
		y = v.y;
	}
	inline Vec operator=(const Vec<2, T>& v) {
		this->x = v.x;
		this->y = v.y;
		return *this;
	}
	inline Vec(const T* ptr) {
		for (size_t i = 0; i < 2; i++)
			m[i] = ptr[i];
	}
	inline const T& operator[](size_t i) const {
		assert(i >= 0 && i < 2);
		return m[i];
	}
};

//特化三维矢量
template<typename T>
struct Vec<3, T> {
	union {
		T m[3];
		struct {
			T x, y, z;
		};
		struct {
			T r, g, b;
		};
	};

	inline Vec() {
		x = T();
		y = T();
		z = T();
	}
	inline Vec(T _x, T _y, T _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	inline Vec(const Vec<3, T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	inline Vec(const T* ptr) {
		for (size_t i = 0; i < 3; i++)
			m[i] = ptr[i];
	}
	inline Vec operator=(const Vec<3, T>& v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}
	inline const T& operator[](size_t i) const {
		assert(i >= 0 && i < 3);
		return m[i];
	}
};

//特化四维矢量
template<typename T>
struct Vec<4, T> {
	union {
		T m[4];
		struct {
			T x, y, z, w;
		};
		struct {
			T r, g, b, a;
		};
	};

	inline Vec() {
		x = T();
		y = T();
		z = T();
		x = T();
	}
	inline Vec(T _x, T _y, T _z, T _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	inline Vec(const Vec<4, T>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}
	inline Vec operator=(const Vec<4, T>& v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
		return *this;
	}
	inline Vec(const T* ptr) {
		for (size_t i = 0; i < 4; i++)
			m[i] = ptr[i];
	}
	inline const T& operator[](size_t i) const {
		assert(i >= 0 && i < 4);
		return m[i];
	}
};
//别名
using Vec2 = Vec<2, float>;
using Vec3 = Vec<3, float>;
using Vec4 = Vec<4, float>;

//矢量运算
//a+b
template<size_t N, typename T>
inline Vec<N, T> operator+(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] + b.m[i];
	}
	return c;
}

//a-b
template<size_t N, typename T>
inline Vec<N, T> operator-(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] - b.m[i];
	}
	return c;
}

//a*b
template<size_t N, typename T>
inline Vec<N, T> operator*(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] * b.m[i];
	}
	return c;
}

//a/b
template<size_t N, typename T>
inline Vec<N, T> operator/(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] / b.m[i];
	}
	return c;
}

//a!=b
template<size_t N, typename T>
inline bool operator!=(const Vec<N, T>& a, const Vec<N, T>& b) {
	for (size_t i = 0; i < N; i++) {
		if (a.m[i] != b.m[i]) {
			return true;
		}
	}
	return false;
}

//a+x
template<size_t N, typename T>
inline Vec<N, T> operator+(const Vec<N, T>& a, T x) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] + x;
	}
	return c;
}

//x+a
template<size_t N, typename T>
inline Vec<N, T> operator+(T x, const Vec<N, T>& a) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] + x;
	}
	return c;
}

//a-x
template<size_t N, typename T>
inline Vec<N, T> operator-(const Vec<N, T>& a, T x) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] - x;
	}
	return c;
}

//x-a
template<size_t N, typename T>
inline Vec<N, T> operator-(T x, const Vec<N, T>& a) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = x - a.m[i];
	}
	return c;
}

//a*x
template<size_t N, typename T>
inline Vec<N, T> operator*(const Vec<N, T>& a, T x) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] * x;
	}
	return c;
}

//x*a
template<size_t N, typename T>
inline Vec<N, T> operator*(T x, const Vec<N, T>& a) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] * x;
	}
	return c;
}

//a/x
template<size_t N, typename T>
inline Vec<N, T> operator/(const Vec<N, T>& a, T x) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = a.m[i] / x;
	}
	return c;
}

//x/a
template<size_t N, typename T>
inline Vec<N, T> operator/(T x, const Vec<N, T>& a) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = x / a.m[i];
	}
	return c;
}

//矢量函数
//|a| ^ 2
template<size_t N, typename T>
inline T vector_length_square(const Vec<N, T>& a) {
	T sum = 0;
	for (size_t i = 0; i < N; i++) {
		sum += a[i] * a[i];
	}
	return sum;
}

//|a|
template<size_t N, typename T>
inline T vector_length(const Vec<N, T>& a) {
	return sqrt(vector_length_square(a));
}

//|a|, 特化float类型，使用sqrtf
template<size_t N>
inline float vector_length(const Vec<N, float>& a) {
	return sqrtf(vector_length_square(a));
}

//a/|a|, 归一化
template<size_t N, typename T>
inline Vec<N, T> vector_normalize(const Vec<N, T>& a) {
	return a / vector_length(a);
}

//矢量点乘
template<size_t N, typename T>
inline T vector_dot(const Vec<N, T>& a, const Vec<N, T>& b) {
	T sum = 0;
	for (size_t i = 0; i < N; i++) {
		sum += a[i] * b[i];
	}
	return sum;
}

//二维矢量叉乘，得到标量
template<size_t, typename T>
inline T vector_cross(const Vec<2, T>& a, const Vec<2, T>& b) {
	return a.x * b.y - a.y * b.x;
}

//三维矢量叉乘，得到新矢量
template<size_t, typename T>
inline Vec<3, T> vector_cross(const Vec<3, T>& a, const Vec<3, T>& b) {
	return Vec<3, T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

//四维矢量叉乘：前三维叉乘，后一位保留
template<size_t, typename T>
inline Vec<4, T> vector_cross(const Vec<4, T>& a, const Vec<4, T>& b) {
	return Vec<4, T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, a.w);
}

//a+(b-a)*t, 插值
template<size_t N, typename T>
inline Vec<N, T> vector_lerp(const Vec<N, T>& a, const Vec<N, T>& b, float t) {
	return a + (b - a) * t;
}

//各个元素取最大值
template<size_t N, typename T>
inline Vec<N, T> vector_max(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = (a.m[i] > b.m[i]) ? a.m[i] : b.m[i];
	}
	return c;
}

//各个元素取最小值
template<size_t N, typename T>
inline Vec<N, T> vector_min(const Vec<N, T>& a, const Vec<N, T>& b) {
	Vec<N, T> c;
	for (size_t i = 0; i < N; i++) {
		c.m[i] = (a.m[i] < b.m[i]) ? a.m[i] : b.m[i];
	}
	return c;
}

//将矢量的值控制在minx-maxx范围内
template<size_t N, typename T>
inline Vec<N, T> vector_between(const Vec<N, T>& minx, const Vec<N, T>& maxx, const Vec<N, T>& x) {
	return vector_min(vector_max(minx, x), maxx);
}

//矩阵
template<size_t ROW, size_t COL, typename T> 
struct Mat {
	T m[ROW][COL];

	inline Mat() {}

	inline Mat(const Mat<ROW, COL, T>& src) {
		for (size_t r = 0; r < ROW; r++) {
			for (size_t c = 0; c < COL; c++)
				m[r][c] = src.m[r][c];
		}
	}

	inline const T* operator[](size_t row) const { assert(row < ROW); return m[row]; }
	inline T* operator[](size_t row) { assert(row < ROW); return m[row]; }

	//取一行
	inline Vec<COL, T> Row(size_t row) const {
		assert(row < ROW);
		Vec<COL, T> a;
		for (size_t i = 0; i < COL; i++) {
			a.m[i] = m[row][i];
		}
		return a;
	}

	//取一列
	inline Vec<ROW, T> Col(size_t col) const {
		assert(col < COL);
		Vec<ROW, T> a;
		for (size_t i = 0; i < ROW; i++) {
			a.m[i] = m[i][col];
		}
		return a;
	}

	//设置一行
	inline void SetRow(size_t row, const Vec<COL, T>& a) {
		assert(row < ROW);
		for (size_t i = 0; i < COL; i++) {
			m[row][i] = a[i];
		}
	}

	//设置一列
	inline void SetCol(size_t col, const Vec<ROW, T>& a) {
		assert(col < COL);
		for (size_t i = 0; i < ROW; i++) {
			m[i][col] = a[i];
		}
	}

	//取得删除某行和某列的子矩阵：子式
	inline Mat<ROW - 1, COL - 1, T> GetMinor(size_t row, size_t col) const {
		Mat<ROW - 1, COL - 1, T> ret;
		for (size_t r = 0; r < ROW - 1; r++) {
			for (size_t c = 0; c < COL - 1; c++) {
				ret.m[r][c] = m[r < row ? r : r + 1][c < col ? c : c + 1];
			}
		}
		return ret;
	}

	//取得转置矩阵
	inline Mat<COL, ROW, T> Transpose() const {
		Mat<COL, ROW, T> ret;
		for (size_t r = 0; r < ROW; r++) {
			for (size_t c = 0; c < COL; c++)
				ret.m[c][r] = m[r][c];
		}
		return ret;
	}

	//取得0矩阵
	static inline Mat<ROW, COL, T> GetZero() {
		Mat<ROW, COL, T> ret;
		for (size_t r = 0; r < ROW; r++) {
			for (size_t c = 0; c < COL; c++)
				ret.m[r][c] = 0;
		}
		return ret;
	}

	//取得单位矩阵
	static inline Mat<ROW, COL, T> GetIdentity() {
		Mat<ROW, COL, T> ret;
		for (size_t r = 0; r < ROW; r++) {
			for (size_t c = 0; c < COL; c++)
				ret.m[r][c] = (r == c) ? 1 : 0;
		}
		return ret;
	}
};
//别名
using Mat3 = Mat<3, 3, float>;

//矩阵运算
template<size_t ROW, size_t COL, typename T>
inline bool operator==(const Mat<ROW, COL, T>& a, const Mat<ROW, COL, T>& b) {
	for (size_t r = 0; r < ROW; r++) {
		for (size_t c = 0; c < COL; c++) {
			if (a.m[r][c] != b.m[r][c]) {
				return false;
			}
		}
	}
	return true;
}

template<size_t ROW, size_t COL, typename T>
inline bool operator!=(const Mat<ROW, COL, T>& a, const Mat<ROW, COL, T>& b) {
	return !(a == b);
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator+(const Mat<ROW, COL, T>& src) {
	return src;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator-(const Mat<ROW, COL, T>& src) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++)
			out.m[j][i] = -src.m[j][i];
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator+(const Mat<ROW, COL, T>& a, const Mat<ROW, COL, T>& b) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++)
			out.m[j][i] = a.m[j][i] + b.m[j][i];
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator-(const Mat<ROW, COL, T>& a, const Mat<ROW, COL, T>& b) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++)
			out.m[j][i] = a.m[j][i] - b.m[j][i];
	}
	return out;
}

template<size_t ROW, size_t COL, size_t NEWCOL, typename T>
inline Mat<ROW, NEWCOL, T>operator*(const Mat<ROW, COL, T>& a, const Mat<COL, NEWCOL, T>& b) {
	Mat<ROW, NEWCOL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < NEWCOL; i++) {
			out.m[j][i] = vector_dot(a.Row(j), b.Col(i));
		}
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T> operator*(const Mat<ROW, COL, T>& a, T x) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++) {
			out.m[j][i] = a.m[j][i] * x;
		}
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator/(const Mat<ROW, COL, T>& a, T x) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++) {
			out.m[j][i] = a.m[j][i] / x;
		}
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T> operator*(T x, const Mat<ROW, COL, T>& a) {
	return (a * x);
}

template<size_t ROW, size_t COL, typename T>
inline Mat<ROW, COL, T>operator/(T x, const Mat<ROW, COL, T>& a) {
	Mat<ROW, COL, T> out;
	for (size_t j = 0; j < ROW; j++) {
		for (size_t i = 0; i < COL; i++) {
			out.m[j][i] = x / a.m[j][i];
		}
	}
	return out;
}

template<size_t ROW, size_t COL, typename T>
inline Vec<COL, T>operator*(const Vec<ROW, T>& a, const Mat<ROW, COL, T>& m) {
	Vec<COL, T> b;
	for (size_t i = 0; i < COL; i++) {
		b.m[i] = vector_dot<COL, T>(a, m.Col(i));
	}
	return b;
}

template<size_t ROW, size_t COL, typename T>
inline Vec<ROW, T>operator*(const Mat<ROW, COL, T>& m, const Vec<COL, T>& a) {
	Vec<ROW, T> b;
	for (size_t i = 0; i < ROW; i++) {
		b.m[i] = vector_dot<ROW, T>(a, m.Row(i));
	}
	return b;
}

