#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <windows.h>
#include <omp.h>//openmp多线程加速

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

#include "Camera/Camera.h"
#include "Math/Math.hpp"
#include "Random/Random.h"
#include "Sample/Sample.h"
#include "Material/Material.h"
#include "Ray/Ray.h"
#include "BVH/BVH.h"
#include "PDF/PDF.h"
#include "HitObject/HitObject.h"
#include "Texture/Texture.h"

#define WINDOW_WIDTH 1600	
#define WINDOW_HEIGHT 800
#define DEPTH 50
#define WINDOW_TITLE L"MyPathTracingRender"
#define SAMPLEPERPIXEL 50

const int nx = WINDOW_WIDTH;//图像宽度
const int ny = WINDOW_HEIGHT;//图像高度
const int spp = SAMPLEPERPIXEL;//sample per pixel, 每个像素的采样次数
const int channels = 3;//代表rgb三通道
unsigned char* data = new unsigned char[nx * ny * channels];
std::shared_ptr<ImageTexture> hdr;

const float aspect_ratio = float(nx) / float(ny);
Vec3 lookfrom(10.0f, 2.0f, 3.0f);
Vec3 lookat(0.0f, 0.0f, 0.0f);
Vec3 vup(0.0f, 1.0f, 0.0f);
float dist_to_focus = 10.0f;
float aperture = 0.1f;
Camera camera(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);
BVHNode* bvh;
HitObjectList world;
HitObjectList lights;

//控制台输出
HANDLE hOutput;
unsigned long lgsize;

HINSTANCE g_hInst;
HWND g_hWnd;
HDC g_hDc;
DWORD g_tPre = 0, g_tNow = 0;//声明两个变量来记录时间,g_tPre记录上一次绘图的时间，g_tNow记录此次准备绘图的时间

//函数声明
int MyWindowsClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
void BitMapHeader(BITMAPINFO& bmi, int nx, int ny, int channels);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Render(HWND hwnd);
Vec3 PathTracing(Ray& ray, int depth);
void CreateScene();
//Vec3 BackGroundColor(const Ray& ray);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MyWindowsClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	CreateScene();

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {//使用while循环，如果消息不是WM_QUIT消息，就继续循环
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {//查看应用程序消息队列，有消息时将队列中的消息派发出去。
			TranslateMessage(&msg);//将虚拟键消息转换为字符消息
			DispatchMessage(&msg);//分发一个消息给窗口程序。
		}
		g_tNow = GetTickCount();//获取当前系统时间
		if (g_tNow - g_tPre >= 5) {
			Render(g_hWnd);//当此次循环运行与上次绘图时间相差0.005秒时再进行重绘操作
		}

	}
	delete data;
	delete bvh;

	return msg.wParam;
}

int MyWindowsClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MyPathTracingRender";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	int i;
	g_hInst = hInstance;
	HBITMAP bmp;

	g_hWnd = CreateWindow(L"MyPathTracingRender", WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
		WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!g_hWnd) {
		return FALSE;
	}
	MoveWindow(g_hWnd, 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, true); //调整窗口显示时的位置，使窗口左上角位于（50,50）处
	ShowWindow(g_hWnd, nCmdShow); //调用ShowWindow函数来显示窗口
	UpdateWindow(g_hWnd);

	g_hDc = GetDC(g_hWnd);

	//开启命令行用于显示输出
	AllocConsole();
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	char strbuf[50] = "MyPathTracingRender:\n";
	WriteFile(hOutput, strbuf, strlen(strbuf), &lgsize, 0);

	_cprintf("spp = %d\n", SAMPLEPERPIXEL);
	_cprintf("depth = %d\n", DEPTH);
	_cprintf("width = %d\n", WINDOW_WIDTH);
	_cprintf("height = %d\n", WINDOW_HEIGHT);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	int i;

	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		FreeConsole();
		ReleaseDC(g_hWnd, g_hDc);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void BitMapHeader(BITMAPINFO& bmi, int nx, int ny, int channels) {
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = nx;
	bmi.bmiHeader.biHeight = ny;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = nx * ny * channels;
}

void CreateScene() {
	int x0, y0, n0;
	unsigned char* hdr_data = stbi_load("HDR/Desert_Highway/Road_to_MonumentValley_8k.jpg", &x0, &y0, &n0, 0);
	hdr = make_shared<ImageTexture>(hdr_data, x0, y0, n0);

	lights.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, -0.4f, 0.4f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	world.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, -0.4f, 0.4f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//lights.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, 0.6f, 1.4f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//world.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, 0.6f, 1.4f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//lights.Add(make_shared<Sphere>(Vec3(0.2f, 1.0f, 1.5f), 0.3f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//world.Add(make_shared<Sphere>(Vec3(0.2f, 1.5f, 1.5f), 0.3f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//lights.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, -1.4f, -0.6f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));
	//world.Add(make_shared<XZ_Rect>(-0.2f, 0.6f, -1.4f, -0.6f, 1.5f, make_shared<DiffuseLight>(make_shared<ConstantTexture>(Vec3(6.0f, 6.0f, 6.0f)))));

	int x1, y1, n1;
	unsigned char* albedo_data = stbi_load("ObjectTexture/RustyMetal/albedo.png", &x1, &y1, &n1, 0);
	int x2, y2, n2;
	unsigned char* roughness_data = stbi_load("ObjectTexture/RustyMetal/roughness.png", &x2, &y2, &n2, 0);
	int x3, y3, n3;
	unsigned char* metallic_data = stbi_load("ObjectTexture/RustyMetal/metallic.png", &x3, &y3, &n3, 0);
	int x4, y4, n4;
	unsigned char* normal_data = stbi_load("ObjectTexture/RustyMetal/normal.png", &x4, &y4, &n4, 0);//目前没用到
	int x5, y5, n5;
	unsigned char* ao_data = stbi_load("ObjectTexture/RustyMetal/ao.png", &x5, &y5, &n5, 0);
	int x, y, n;
	unsigned char* texture_data = stbi_load("ObjectTexture/Earth/earthmap.jpg", &x, &y, &n, 0);
	world.Add(make_shared<Sphere>(Vec3(0.0f, 0.0f, -1.0f), 0.5f, make_shared<LambertianMicrofacet_Beckmann>(make_shared<ImageTexture>(albedo_data, x1, y1, n1),
		make_shared<ImageTexture>(roughness_data, x2, y2, n2), make_shared<ImageTexture>(metallic_data, x3, y3, n3), make_shared<ImageTexture>(normal_data, x4, y4, n4),
		make_shared<ImageTexture>(ao_data, x5, y5, n5))));
	world.Add(make_shared<Sphere>(Vec3(0.0f, 0.0f, 1.0f), 0.5f, make_shared<OrenNayarMicrofacet_GGX>(make_shared<ImageTexture>(albedo_data, x1, y1, n1),
		make_shared<ImageTexture>(roughness_data, x2, y2, n2), make_shared<ImageTexture>(metallic_data, x3, y3, n3), make_shared<ImageTexture>(normal_data, x4, y4, n4),
		make_shared<ImageTexture>(ao_data, x5, y5, n5))));
	//world.Add(make_shared<Sphere>(Vec3(0.0f, 0.0f, 2.0f), 0.5f, make_shared<OrenNayar>(make_shared<ImageTexture>(texture_data, x, y, n), make_shared<ImageTexture>(roughness_data, x2, y2, n2))));
	world.Add(make_shared<Sphere>(Vec3(0.0f, 0.0f, 0.0f), 0.5f, make_shared<Metal>(make_shared<ConstantTexture>(Vec3(0.8f, 0.88f, 0.85f)), 0.0f)));
	//world.Add(make_shared<Sphere>(Vec3(0.0f, 0.0f, -1.0f), 0.5f, make_shared<Dielectric>(make_shared<ConstantTexture>(Vec3(1.0f, 1.0f, 1.0f)), 1.5f)));
	auto checker = make_shared<CheckerTexture>(make_shared<ConstantTexture>(Vec3(0.9f, 0.9f, 0.9f)), make_shared<ConstantTexture>(Vec3(0.2f, 0.3f, 0.1f)));
	world.Add(make_shared<XZ_Rect>(-20.0f, 20.0f, -20.0f, 20.0f, -0.5f, make_shared<Lambertian>(checker)));
	//world.Add(make_shared<Sphere>(Vec3(0.0f, -100.5f, -1.0f), 100.0f, make_shared<Lambertian>(checker)));
	bvh = new BVHNode(world.objects, 0, world.objects.size() - 1, 0.001f, INF);
}

//Vec3 BackGroundColor(const Ray& ray, const HitRecord& rec) {
//	Vec3 normalised_direction = vector_normalize<3, float>(ray.GetDirection());
//	float theta = acos(-normalised_direction.y);
//	float phi = atan2(-normalised_direction.z, normalised_direction.x) + PI;
//
//	float u = phi / (2.0f * PI);
//	float v = theta / PI;
//	Vec2 uv(u, v);
//
//	return hdr->Value(uv, rec.p);
//}

Vec3 PathTracing(Ray& ray, int depth) {
	HitRecord rec;

	//超过最大深度，递归终止
	if (depth <= 0) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	//与世界中的物体求交
	if (!bvh->Hit(ray, 0.001f, INF, rec)) {
		return Vec3(0.0f, 0.0f, 0.0f);

		//Vec3 normalize_direction = vector_normalize<3, float>(ray.GetDirection());
		//float t = 0.5f * (normalize_direction.y + 1.0f);
		//return vector_lerp<3, float>(Vec3(1.0f, 1.0f, 1.0f), Vec3(0.5f, 0.7f, 1.0f), t);
	}

	//俄罗斯轮盘赌，有0.2的概率光线消失
	float p = 0.8f;
	float t = random_float();
	if (t > p) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	//计算光线
	Ray scatteredRay;
	MixturePDF pdf;
	auto light_pdf = make_shared<HitObjectPDF>(make_shared<HitObjectList>(lights), rec.p);
	if (rec.mat_ptr->m_type == SPECULAR_Beckmann) {
		auto beck_pdf = make_shared<BeckmannPDF>(rec.mat_ptr->GetRoughness(rec), rec.mat_ptr->GetKs());
		pdf = MixturePDF(beck_pdf, light_pdf);
	}
	else if (rec.mat_ptr->m_type == SPECULAR_GGX) {
		auto ggx_pdf = make_shared<GGXPDF>(rec.mat_ptr->GetRoughness(rec), rec.mat_ptr->GetKs());
		pdf = MixturePDF(ggx_pdf, light_pdf);
	}
	else if (rec.mat_ptr->m_type == DIFFUSE) {
		auto cos_pdf = make_shared<COSPDF>(rec.normal);
		pdf = MixturePDF(cos_pdf, light_pdf);
	}
	//pdf.SetMix(1.0f);
	
	//判断是否是光源
	Vec3 emitted = rec.mat_ptr->Emitted(ray, rec);
	if (!rec.mat_ptr->Scatter(ray, rec, scatteredRay, pdf)) {
		return emitted;
	}

	return emitted + rec.mat_ptr->BSDF_Cos(ray, rec, scatteredRay, pdf) * PathTracing(scatteredRay, depth - 1) / (rec.mat_ptr->PDF_Value(ray, rec, scatteredRay, pdf) + 0.001f)/ p;
}

void Render(HWND hwnd) {
	int depth = DEPTH;//递归深度

	omp_set_num_threads(32);//线程个数
#pragma omp parallel for
	for (int j = 0; j < ny; j++) {
		for (int i = 0; i < nx; i++) {
			Vec3 color(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < spp; s++) {
				float u = (i + random_float()) / nx;
				float v = (j + random_float()) / ny;
				Ray ray = camera.GetRay(u, v);
				color = color + PathTracing(ray, depth);
			}
			color = color / (float)spp;
			color = color / (color + 1.0f);
			//unsigned char:0-255，刚好能表示颜色
			//由于位图是上下颠倒的，本应是ny - j - 1，颠倒后改为ny - (ny - 1 - j) - 1

			data[(ny - (ny - 1 - j) - 1) * nx * 3 + 3 * i] = static_cast<unsigned char>(255.999f * sqrtf(color.b));
			data[(ny - (ny - 1 - j) - 1) * nx * 3 + 3 * i + 1] = static_cast<unsigned char>(255.999f * sqrtf(color.g));
			data[(ny - (ny - 1 - j) - 1) * nx * 3 + 3 * i + 2] = static_cast<unsigned char>(255.999f * sqrtf(color.r));
		}
	}

	BITMAPINFO bmi;
	BitMapHeader(bmi, nx, ny, channels);
	HDC hCompatibleDC = CreateCompatibleDC(g_hDc);
	HBITMAP hCompatibleBitmap = CreateCompatibleBitmap(g_hDc, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);
	SetDIBits(g_hDc, hCompatibleBitmap, 0, bmi.bmiHeader.biHeight, data, (BITMAPINFO*)&bmi.bmiHeader, DIB_RGB_COLORS);
	BitBlt(g_hDc, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hCompatibleDC, 0, 0, SRCCOPY);
	SelectObject(hCompatibleDC, hOldBitmap);
	DeleteObject(hCompatibleDC);

	//BITMAPINFO bmi;
	//::ZeroMemory(&bmi, sizeof(BITMAPINFO));
	//BitMapHeader(bmi, nx, ny, channels);
	//
	////执行绘制
	//StretchDIBits(g_hDc, 0, 0, bmi.bmiHeader.biWidth,
	//	bmi.bmiHeader.biHeight, 0, 0, bmi.bmiHeader.biWidth,
	//	bmi.bmiHeader.biHeight, data, (BITMAPINFO*)&bmi.bmiHeader,
	//	DIB_RGB_COLORS, SRCCOPY);

	g_tPre = GetTickCount();
}