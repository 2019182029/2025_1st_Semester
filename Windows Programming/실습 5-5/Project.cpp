#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <gdiplus.h>

#pragma comment (lib, "msimg32.lib")
#pragma comment(lib, "gdiplus.lib")
//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

using namespace Gdiplus;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// Background
constexpr int WIDTH = 1024;
constexpr int HEIGHT = 800;

RECT rect;

class BACKGROUND {
public:
	BITMAP m_bmp;
	HBITMAP m_hBitmap;

	void print(HDC mDC) const {
		HDC bDC = CreateCompatibleDC(mDC);
		HGDIOBJ hBitmap = SelectObject(bDC, m_hBitmap);

		StretchBlt(mDC, 0, 0, rect.right, rect.bottom,
			bDC, 0, 0, m_bmp.bmWidth, m_bmp.bmHeight, SRCCOPY);

		SelectObject(bDC, hBitmap);
		DeleteDC(bDC);
	}
};

BACKGROUND bg;

//////////////////////////////////////////////////
// CHARACTER
HBITMAP RunBit[2];

class CHARACTER {
public:
	int m_x, m_y;
	int m_count;
	int m_velocity;

	void print(HDC mDC) {
		HDC cDC = CreateCompatibleDC(mDC);

		SelectObject(cDC, RunBit[m_count]);
		m_count = (m_count + 1) % 2;

		TransparentBlt(mDC, m_x, m_y, 90, 90, cDC, 0, 0, 90, 90, RGB(0, 0, 64));

		DeleteDC(cDC);
	}
};

CHARACTER character;

//////////////////////////////////////////////////
// WINMAIN
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE	hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
		0, 0, WIDTH, HEIGHT,
		NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

//////////////////////////////////////////////////
// WNDPROC
LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;

	switch (iMessage) {
	case WM_CREATE:
		bg.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(bg.m_hBitmap, sizeof(BITMAP), &bg.m_bmp);
		GetClientRect(hWnd, &rect);
		
		TCHAR file[100];
		for (int i = 1; i < 5 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\ch%d.bmp"), i);
			RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		SetTimer(hWnd, 0, 100, NULL);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CHAR:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_MOUSEMOVE: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONUP: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HGDIOBJ old_hBitmap = SelectObject(mDC, hBitmap);

		bg.print(mDC);

		character.print(mDC);

		BitBlt(hDC, 0, 0, rect.right, rect.bottom, mDC, 0, 0, SRCCOPY);

		SelectObject(mDC, old_hBitmap);
		DeleteObject(hBitmap);
		DeleteDC(mDC);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}
