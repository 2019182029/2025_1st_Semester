#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include <vector>
#include "resource.h"

//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

#define IDC_BUTTON_PLATFORM1 10
#define IDC_BUTTON_PLATFORM2 11
#define IDC_BUTTON_PLATFORM3 12
#define IDC_BUTTON_PLATFORM4 13
#define IDC_BUTTON_PLATFORM5 14
#define IDC_BUTTON_PLATFORM6 15
#define IDC_BUTTON_CANCEL 16
#define IDC_BUTTON_TEST 17
#define IDC_BUTTON_LEFT 18
#define IDC_BUTTON_RIGHT 19

#define IDC_LISTBOX1 20

HWND g_hWnd, g_hChild;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

HWND hList;
HWND hButton[10];
HBITMAP hBitmap[6];
TCHAR items[10][2] = { L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 1000;

constexpr int ROW = 15;
constexpr int COL = 15;

bool test = false;

int current_page = 0;
int current_platform = -1;

void player_platform_collision();

//////////////////////////////////////////////////
// Background
class BACKGROUND {
public:
	BITMAP m_bmp;
	HBITMAP m_hBitmap;
	bool m_filled = false;
	int m_img_index = 0;

	int m_platform[ROW][COL];

	BACKGROUND() {
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				m_platform[r][c] = -1;
			}
		}
	}

	void print(HDC mDC, const RECT& rect) const {
		HDC bDC = CreateCompatibleDC(mDC);
		HGDIOBJ hGdiobj = SelectObject(bDC, m_hBitmap);

		StretchBlt(mDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			bDC, 0, 0, m_bmp.bmWidth, m_bmp.bmHeight, SRCCOPY);

		int x_offset = (rect.right - rect.left) / 15;
		int y_offset = (rect.bottom - rect.top) / 15;

		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				int index = m_platform[r][c];

				if (index != -1) {
					SelectObject(bDC, hBitmap[index]);
					StretchBlt(mDC, rect.left + c * x_offset, rect.top + r * y_offset, x_offset, y_offset,
						bDC, 0, 0, 100, 100, SRCCOPY);
				}
			}
		}

		if (!test) {
			for (int i = 0; i < 15; ++i) {
				MoveToEx(mDC, rect.left, i * y_offset, NULL);
				LineTo(mDC, rect.right, i * y_offset);

				MoveToEx(mDC, i * y_offset, rect.top, NULL);
				LineTo(mDC, i * y_offset, rect.bottom);
			}
		}

		SelectObject(bDC, hBitmap);
		DeleteDC(bDC);
	}
};

BACKGROUND backgrounds[10];
std::vector<BACKGROUND> bgs;

//////////////////////////////////////////////////
// PLAYER
constexpr int LEFT = 0;
constexpr int RIGHT = 1;

int camera_x = 0;

class PLAYER {
public:
	int m_x, m_y;
	int m_g = 0;
	int m_dir = -1;

	void move() {
		int old_x = m_x;
		int old_y = m_y;
		int old_camera_x = camera_x;

		switch (m_dir) {
		case LEFT:
			m_x -= 5;
			camera_x -= 5;
			if (camera_x < 0) camera_x = 0;
			break;

		case RIGHT:
			m_x += 5;
			camera_x += 5;
			if (camera_x > (bgs.size() - 1) * 500) camera_x = (bgs.size() - 1) * 500;
			break;
		}

		m_y += m_g;

		m_g = min(10, m_g + 1);

		player_platform_collision();
	}

	void print(HDC mDC, const RECT& rect) const {
		int x_offset = (rect.right - rect.left) / 15;
		int y_offset = (rect.bottom - rect.top) / 15;

		int s_x = m_x - camera_x;
		int s_y = m_y;

		Rectangle(mDC,
			s_x, s_y,
			s_x + x_offset, s_y + y_offset);
	}
};

PLAYER player;

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
		
		hButton[0] = CreateWindow(L"button", L"PLATFORM1", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 210, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM1, g_hInst, NULL);
		hButton[1] = CreateWindow(L"button", L"PLATFORM2", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 245, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM2, g_hInst, NULL);
		hButton[2] = CreateWindow(L"button", L"PLATFORM3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 280, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM3, g_hInst, NULL);
		hButton[3] = CreateWindow(L"button", L"PLATFORM4", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 315, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM4, g_hInst, NULL);
		hButton[4] = CreateWindow(L"button", L"PLATFORM5", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 350, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM5, g_hInst, NULL);
		hButton[5] = CreateWindow(L"button", L"PLATFORM6", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP, 600, 385, 100, 25, hWnd, (HMENU)IDC_BUTTON_PLATFORM6, g_hInst, NULL);
		hButton[6] = CreateWindow(L"button", L"CANCEL", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 420, 100, 25, hWnd, (HMENU)IDC_BUTTON_CANCEL, g_hInst, NULL);
		hButton[7] = CreateWindow(L"button", L"TEST", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 455, 100, 25, hWnd, (HMENU)IDC_BUTTON_TEST, g_hInst, NULL);
		hButton[8] = CreateWindow(L"button", L"←", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 600, 490, 45, 25, hWnd, (HMENU)IDC_BUTTON_LEFT, g_hInst, NULL);
		hButton[9] = CreateWindow(L"button", L"→", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 655, 490, 45, 25, hWnd, (HMENU)IDC_BUTTON_RIGHT, g_hInst, NULL);

		for (int i = 0; i < 6; ++i) {
			TCHAR str[30];
			wsprintf(str, L"Resource\\platform%d.bmp", i);
			hBitmap[i] = (HBITMAP)LoadImage(g_hInst, str, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			SendMessage(hButton[i], BM_SETIMAGE, 0, (LPARAM)hBitmap[i]);
		}

		hList = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD, 600, 10, 100, 200, hWnd, (HMENU)IDC_LISTBOX1, g_hInst, NULL);
		for (int i = 0; i < 10; ++i) {
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)items[i]);
		}
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			player.move();
			break;
		}
		InvalidateRect(g_hChild, NULL, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_PLATFORM1:
			current_platform = 0;
			break;

		case IDC_BUTTON_PLATFORM2:
			current_platform = 1;
			break;

		case IDC_BUTTON_PLATFORM3:
			current_platform = 2;
			break;

		case IDC_BUTTON_PLATFORM4:
			current_platform = 3;
			break;

		case IDC_BUTTON_PLATFORM5:
			current_platform = 4;
			break;

		case IDC_BUTTON_PLATFORM6:
			current_platform = 5;
			break;

		case IDC_BUTTON_CANCEL:
			current_platform = -1;
			break;

		case IDC_BUTTON_TEST:
			test = true;
			for (const auto& bg : backgrounds) {
				if (bg.m_filled) {
					bgs.emplace_back(bg);
				}
			}
			SetTimer(hWnd, 0, 16, NULL);
			break;

		case IDC_BUTTON_LEFT:
			current_page -= 1;
			if (current_page == -1) {
				current_page = 9;
			}
			break;

		case IDC_BUTTON_RIGHT:
			current_page = (current_page + 1) % 10;
			break;

		case IDC_LISTBOX1:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE: {
				int i = SendMessage(hList, LB_GETCURSEL, 0, 0);
				TCHAR str[20];
				wsprintf(str, L"Resource\\%d.bmp", i);
				backgrounds[current_page].m_hBitmap = (HBITMAP)LoadImage(g_hInst, str, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
				GetObject(backgrounds[current_page].m_hBitmap, sizeof(BITMAP), &backgrounds[current_page].m_bmp);
				backgrounds[current_page].m_filled = true;
				backgrounds[current_page].m_img_index = i;
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
		if (!test) { break; }
		switch (wParam) {
		case VK_LEFT:
			player.m_dir = LEFT;
			break;

		case VK_RIGHT:
			player.m_dir = RIGHT;
			break;

		case VK_UP:
			player.m_g = -10;
			break;
		}
		InvalidateRect(g_hChild, NULL, FALSE);
		break;

	case WM_KEYUP:
		if (!test) { break; }
		switch (wParam) {
		case VK_LEFT:
			player.m_dir = -1;
			break;

		case VK_RIGHT:
			player.m_dir = -1;
			break;
		}
		InvalidateRect(g_hChild, NULL, FALSE);
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
	static bool lbuffon_clicked = false;

	switch (iMessage) {
	case WM_LBUTTONDOWN: {
		lbuffon_clicked = true;

		int x_offset = (rt.right - rt.left) / 15;
		int y_offset = (rt.bottom - rt.top) / 15;

		int x = LOWORD(lParam) / x_offset;
		int y = HIWORD(lParam) / y_offset;

		if (0 <= x && 
			x <= 14 &&
			0 <= y 
			&& y <= 14) {
			backgrounds[current_page].m_platform[y][x] = current_platform;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP:
		lbuffon_clicked = false;
		break;

	case WM_MOUSEMOVE: {
		if (!lbuffon_clicked) {
			break;
		}

		int x_offset = (rt.right - rt.left) / 15;
		int y_offset = (rt.bottom - rt.top) / 15;

		int x = LOWORD(lParam) / x_offset;
		int y = HIWORD(lParam) / y_offset;

		if (0 <= x &&
			x <= 14 &&
			0 <= y
			&& y <= 14) {
			backgrounds[current_page].m_platform[y][x] = current_platform;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		hOldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

		if (!test) {
			FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
			backgrounds[current_page].print(mDC, rt);
		} else {
			int index = 0;

			for (const auto& bg : bgs) {
				RECT rect = rt;
				rect.left = rect.left + (rect.right * index) - camera_x;
				rect.right = rect.right + (rect.right * index) - camera_x;

				bg.print(mDC, rect);

				++index;
			}

			player.print(mDC, rt);
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

void player_platform_collision() {
	if (!test) return;

	if (0 > player.m_g) return;

	int x_offset = 500 / 15;
	int y_offset = 500 / 15;

	for (int i = 0; i < bgs.size(); ++i) {
		for (int r = 0; r < 15; ++r) {
			for (int c = 0; c < 15; ++c) {
				if (bgs[i].m_platform[r][c] == -1) continue;

				int x = i * 500 + c * y_offset - camera_x;
				int y = r * x_offset;

				if (player.m_x <= x + x_offset &&
					x <= player.m_x + x_offset &&
					player.m_y <= y + y_offset &&
					y <= player.m_y + y_offset) {
					player.m_y = y - y_offset;
					player.m_g = 0;
					return;
				}
			}
		}
	}

	return;
}
