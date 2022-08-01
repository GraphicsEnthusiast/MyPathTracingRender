#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <windows.h>
#include <omp.h>//openmp���̼߳���

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

const int nx = WINDOW_WIDTH;//ͼ����
const int ny = WINDOW_HEIGHT;//ͼ��߶�
const int spp = SAMPLEPERPIXEL;//sample per pixel, ÿ�����صĲ�������
const int channels = 3;//����rgb��ͨ��
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

//����̨���
HANDLE hOutput;
unsigned long lgsize;

HINSTANCE g_hInst;
HWND g_hWnd;
HDC g_hDc;
DWORD g_tPre = 0, g_tNow = 0;//����������������¼ʱ��,g_tPre��¼��һ�λ�ͼ��ʱ�䣬g_tNow��¼�˴�׼����ͼ��ʱ��

//��������
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
	while (msg.message != WM_QUIT) {//ʹ��whileѭ���������Ϣ����WM_QUIT��Ϣ���ͼ���ѭ��
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {//�鿴Ӧ�ó�����Ϣ���У�����Ϣʱ�������е���Ϣ�ɷ���ȥ��
			TranslateMessage(&msg);//���������Ϣת��Ϊ�ַ���Ϣ
			DispatchMessage(&msg);//�ַ�һ����Ϣ�����ڳ���
		}
		g_tNow = GetTickCount();//��ȡ��ǰϵͳʱ��
		if (g_tNow - g_tPre >= 5) {
			Render(g_hWnd);//���˴�ѭ���������ϴλ�ͼʱ�����0.005��ʱ�ٽ����ػ����
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
	MoveWindow(g_hWnd, 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, true); //����������ʾʱ��λ�ã�ʹ�������Ͻ�λ�ڣ�50,50����
	ShowWindow(g_hWnd, nCmdShow); //����ShowWindow��������ʾ����
	UpdateWindow(g_hWnd);

	g_hDc = GetDC(g_hWnd);

	//����������������ʾ���
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
	unsigned char* normal_data = stbi_load("ObjectTexture/RustyMetal/normal.png", &x4, &y4, &n4, 0);//Ŀǰû�õ�
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

	//���������ȣ��ݹ���ֹ
	if (depth <= 0) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	//�������е�������
	if (!bvh->Hit(ray, 0.001f, INF, rec)) {
		return Vec3(0.0f, 0.0f, 0.0f);

		//Vec3 normalize_direction = vector_normalize<3, float>(ray.GetDirection());
		//float t = 0.5f * (normalize_direction.y + 1.0f);
		//return vector_lerp<3, float>(Vec3(1.0f, 1.0f, 1.0f), Vec3(0.5f, 0.7f, 1.0f), t);
	}

	//����˹���̶ģ���0.2�ĸ��ʹ�����ʧ
	float p = 0.8f;
	float t = random_float();
	if (t > p) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	//�������
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
	
	//�ж��Ƿ��ǹ�Դ
	Vec3 emitted = rec.mat_ptr->Emitted(ray, rec);
	if (!rec.mat_ptr->Scatter(ray, rec, scatteredRay, pdf)) {
		return emitted;
	}

	return emitted + rec.mat_ptr->BSDF_Cos(ray, rec, scatteredRay, pdf) * PathTracing(scatteredRay, depth - 1) / (rec.mat_ptr->PDF_Value(ray, rec, scatteredRay, pdf) + 0.001f)/ p;
}

void Render(HWND hwnd) {
	int depth = DEPTH;//�ݹ����

	omp_set_num_threads(32);//�̸߳���
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
			//unsigned char:0-255���պ��ܱ�ʾ��ɫ
			//����λͼ�����µߵ��ģ���Ӧ��ny - j - 1���ߵ����Ϊny - (ny - 1 - j) - 1

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
	////ִ�л���
	//StretchDIBits(g_hDc, 0, 0, bmi.bmiHeader.biWidth,
	//	bmi.bmiHeader.biHeight, 0, 0, bmi.bmiHeader.biWidth,
	//	bmi.bmiHeader.biHeight, data, (BITMAPINFO*)&bmi.bmiHeader,
	//	DIB_RGB_COLORS, SRCCOPY);

	g_tPre = GetTickCount();
}