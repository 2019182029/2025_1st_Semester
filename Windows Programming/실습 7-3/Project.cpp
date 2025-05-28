#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include <vector>
#include "resource.h"

//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

#define IDC_BUTTON1 10
#define IDC_BUTTON2 11
#define IDC_BUTTON3 12
#define IDC_BUTTON4 13
#define IDC_BUTTON5 14
#define IDC_BUTTON6 15

#define IDC_LISTBOX1 20

HWND g_hWnd, g_hChild;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

HWND hList;
HWND hButton[6];
TCHAR items[10][2] = { L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 1000;

int x_gap = 0;
int current_page = 0;
bool selecting = false;
bool done = false;

//////////////////////////////////////////////////
// Background
class BACKGROUND {
public:
	BITMAP m_bmp;
	HBITMAP m_hBitmap;
	bool m_filled = false;

	void print(HDC mDC, const RECT& rect) const {
		HDC bDC = CreateCompatibleDC(mDC);
		HGDIOBJ hBitmap = SelectObject(bDC, m_hBitmap);

		StretchBlt(mDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			bDC, 0, 0, m_bmp.bmWidth, m_bmp.bmHeight, SRCCOPY);

		SelectObject(bDC, hBitmap);
		DeleteDC(bDC);
	}
};

BACKGROUND backgrounds[10];
std::vector<BACKGROUND> bgs;

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
	static HBITMAP hBitmap[2];
	GetClientRect(hWnd, &rt);

	switch (iMessage) {
	case WM_CREATE:
		g_hChild = CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER , 10, 10, 500, 500, hWnd, NULL, g_hInst, NULL);
		
		hButton[0] = CreateWindow(L"button", L"Select", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 210, 100, 25, hWnd, (HMENU)IDC_BUTTON1, g_hInst, NULL);
		hButton[1] = CreateWindow(L"button", L"Move", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 245, 100, 25, hWnd, (HMENU)IDC_BUTTON2, g_hInst, NULL);
		hButton[2] = CreateWindow(L"button", L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 280, 100, 25, hWnd, (HMENU)IDC_BUTTON3, g_hInst, NULL);
		hButton[3] = CreateWindow(L"button", L"Done", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 315, 100, 25, hWnd, (HMENU)IDC_BUTTON4, g_hInst, NULL);
		hButton[4] = CreateWindow(L"button", L"←", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 350, 100, 25, hWnd, (HMENU)IDC_BUTTON5, g_hInst, NULL);
		hBitmap[0] = (HBITMAP)LoadImage(g_hInst, L"Resource\\left.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		SendMessage(hButton[4], BM_SETIMAGE, 0, (LPARAM)hBitmap[0]);
		hButton[5] = CreateWindow(L"button", L"→", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 385, 100, 25, hWnd, (HMENU)IDC_BUTTON6, g_hInst, NULL);
		hBitmap[1] = (HBITMAP)LoadImage(g_hInst, L"Resource\\right.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		SendMessage(hButton[5], BM_SETIMAGE, 0, (LPARAM)hBitmap[1]);

		hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD, 600, 10, 100, 200, hWnd, (HMENU)IDC_LISTBOX1, g_hInst, NULL);
		for (int i = 0; i < 10; ++i)
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)items[i]);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			x_gap -= 10;
			if (abs(x_gap) == bgs.size() * 500) { x_gap = 0; }
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON1:
			selecting = true;
			break;

		case IDC_BUTTON2:
			if (!done) { break; }
			SetTimer(hWnd, 0, 16, NULL);
			break;

		case IDC_BUTTON3:
			if (!done) { break; }
			KillTimer(hWnd, 0);
			break;

		case IDC_BUTTON4:
			EnableWindow(hButton[0], FALSE);
			for (const auto& bg : backgrounds) {
				if (bg.m_filled) {
					bgs.emplace_back(bg);
				}
			}
			done = true;
			break;

		case IDC_BUTTON5:
			--current_page;
			if (-1 == current_page) {
				current_page = 9;
			}
			break;

		case IDC_BUTTON6:
			current_page = (current_page + 1) % 10;
			break;

		case IDC_LISTBOX1:
			if (!selecting) { break; }

			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE: {
				int i = SendMessage(hList, LB_GETCURSEL, 0, 0);
				TCHAR str[100];
				wsprintf(str, L"Resource\\%d.bmp", i);
				backgrounds[current_page].m_hBitmap = (HBITMAP)LoadImage(g_hInst, str, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				GetObject(backgrounds[current_page].m_hBitmap, sizeof(BITMAP), &backgrounds[current_page].m_bmp);
				backgrounds[current_page].m_filled = true;
				break;
			}
			}
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
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		hOldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

		if (!done) {
			FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
			backgrounds[current_page].print(mDC, rt);
		} else {
			int width = rt.right - rt.left;

			for (int i = 0; i < bgs.size(); ++i) {
				RECT rect;
				rect.top = rt.top;
				rect.bottom = rt.bottom;
				rect.left = i * width + x_gap;
				rect.right = rect.left + width;
				FillRect(mDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
				bgs[i].print(mDC, rect);
			}
			RECT rect;
			rect.top = rt.top;
			rect.bottom = rt.bottom;
			rect.left = bgs.size() * width + x_gap;
			rect.right = rect.left + width;
			FillRect(mDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
			bgs[0].print(mDC, rect);
		}

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
