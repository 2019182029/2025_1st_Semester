#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include "resource.h"

//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

#define IDC_BUTTON1 10
#define IDC_BUTTON2 11
#define IDC_BUTTON3 12

#define IDC_RADIO1 100
#define IDC_RADIO2 101
#define IDC_RADIO3 102
#define IDC_RADIO4 103
#define IDC_RADIO5 104
#define IDC_RADIO6 105

HWND g_hWnd, g_hChild;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

HWND hButton[3];
HWND hRadio[6];

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 1000;

constexpr int ROW = 50;
constexpr int COL = 50;
constexpr int OFFSET = 20;

bool draw = false;
bool clicked = false;
COLORREF select_color = RGB(255, 255, 255);

class BOARD {
public:
	COLORREF m_color[ROW][COL];

public:
	BOARD() {
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				m_color[r][c] = RGB(255, 255, 255);
			}
		}
	}

	void print(HDC hDC, const RECT& rt) const {
		FillRect(hDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				HBRUSH hBrush = CreateSolidBrush(m_color[r][c]);
				HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

				Rectangle(hDC,
					c * OFFSET, r * OFFSET,
					(c + 1) * OFFSET, (r + 1) * OFFSET);

				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
		}
	}
};

BOARD board;

//////////////////////////////////////////////////
// WINMAIN
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE	hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow) {
	MSG Message;
	WNDCLASSEX WndClass;
	WNDCLASSEX ChildClass;
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
	//WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	ChildClass.cbSize = sizeof(ChildClass);
	ChildClass.style = CS_HREDRAW | CS_VREDRAW;
	ChildClass.lpfnWndProc = (WNDPROC)ChildProc; // 차일드 윈도우 프로시저 지정
	ChildClass.cbClsExtra = 0;
	ChildClass.hInstance = hInstance;
	ChildClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	ChildClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	ChildClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	ChildClass.lpszMenuName = NULL;
	ChildClass.lpszClassName = L"ChildClass"; // 차일드 윈도우 클래스 이름
	RegisterClassEx(&ChildClass);

	g_hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
		0, 0, W_WIDTH, W_HEIGHT,
		NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

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
	RECT rt;
	GetClientRect(hWnd, &rt);

	switch (iMessage) {
	case WM_CREATE:
		g_hChild = CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER , 10, 10, 500, 500, hWnd, NULL, g_hInst, NULL);
		
		hButton[0] = CreateWindow(L"button", L"Draw", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 10, 100, 25, hWnd, (HMENU)IDC_BUTTON1, g_hInst, NULL);
		hButton[1] = CreateWindow(L"button", L"Erase", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 45, 100, 25, hWnd, (HMENU)IDC_BUTTON2, g_hInst, NULL);
		hButton[2] = CreateWindow(L"button", L"Quit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 80, 100, 25, hWnd, (HMENU)IDC_BUTTON3, g_hInst, NULL);

		hRadio[0] = CreateWindow(L"button", L"Red", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 600, 115, 100, 30, hWnd, (HMENU)IDC_RADIO1, g_hInst, NULL);
		hRadio[1] = CreateWindow(L"button", L"Green", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 600, 150, 100, 30, hWnd, (HMENU)IDC_RADIO2, g_hInst, NULL);
		hRadio[2] = CreateWindow(L"button", L"Blue", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 600, 185, 100, 30, hWnd, (HMENU)IDC_RADIO3, g_hInst, NULL);
		hRadio[3] = CreateWindow(L"button", L"Yellow", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 600, 220, 100, 30, hWnd, (HMENU)IDC_RADIO4, g_hInst, NULL);
		hRadio[4] = CreateWindow(L"button", L"Cyan", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 600, 255, 100, 30, hWnd, (HMENU)IDC_RADIO5, g_hInst, NULL);
		hRadio[5] = CreateWindow(L"button", L"Magenta", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 600, 290, 100, 30, hWnd, (HMENU)IDC_RADIO6, g_hInst, NULL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON1:
			draw = true;
			break;

		case IDC_BUTTON2:
			for (auto& row : board.m_color) {
				for (auto& cell : row) {
					cell = RGB(255, 255, 255);
				}
			}
			break;

		case IDC_BUTTON3:
			PostQuitMessage(0);
			return 0;

		case IDC_RADIO1:
			select_color = RGB(255, 0, 0);
			break;

		case IDC_RADIO2:
			select_color = RGB(0, 255, 0);
			break;

		case IDC_RADIO3:
			select_color = RGB(0, 0, 255);
			break;

		case IDC_RADIO4:
			select_color = RGB(255, 255, 0);
			break;

		case IDC_RADIO5:
			select_color = RGB(0, 255, 255);
			break;

		case IDC_RADIO6:
			select_color = RGB(255, 0, 255);
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CHAR:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_PAINT: {
		InvalidateRect(g_hChild, NULL, FALSE);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

LRESULT ChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	RECT rt;
	HBITMAP hBitmap, hOldBitmap;
	GetClientRect(hWnd, &rt);

	switch (iMessage) {
	case WM_LBUTTONDOWN: {
		if (!draw) { break; }

		clicked = true;

		int x = LOWORD(lParam) / OFFSET;
		int y = HIWORD(lParam) / OFFSET;

		board.m_color[y][x] = select_color;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		clicked = false;
		break;
	}

	case WM_MOUSEMOVE: {
		if (!draw || !clicked) { break; }

		int x = LOWORD(lParam) / OFFSET;
		int y = HIWORD(lParam) / OFFSET;

		board.m_color[y][x] = select_color;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		hOldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

		board.print(mDC, rt);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		SelectObject(mDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(mDC);
		EndPaint(hWnd, &ps);
		break;
	}
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}
