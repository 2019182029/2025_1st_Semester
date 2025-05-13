#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "resource1.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int WIDTH = 1000;
constexpr int HEIGHT = 1000;
constexpr int ROW = 40;
constexpr int COL = 40;
constexpr int OFFSET = 24;

class BOARD {
private:

public:
	BOARD() {}
	~BOARD() {}

	void print(HDC hDC);
};

void BOARD::print(HDC hDC) {
	for (int r = 0; r < ROW; ++r) {
		for (int c = 0; c < COL; ++c) {
			Rectangle(hDC, 
				r * OFFSET, c * OFFSET,
				(r + 1) * OFFSET, (c + 1) * OFFSET);
		}
	}
}

BOARD board;

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
bool a_pressed = false;
bool r_pressed = false;

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	static BITMAP bmp;
	static HBITMAP hBitmap;
	static RECT rect;
	static int div;
	static int sel;

	switch (iMessage) {
	case WM_CREATE:
		hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("tukorea.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		GetClientRect(hWnd, &rect);
		div = 1;
		sel = -1;
		break;

	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CHAR:
		switch (wParam) {
		case 'a':
			if (!a_pressed) {
				a_pressed = !a_pressed;
				r_pressed = false;
				div = 1;
				rect.right = bmp.bmWidth;
				rect.bottom = bmp.bmHeight;
			}
			else {
				a_pressed = !a_pressed;
				r_pressed = false;
				div = 1;
				GetClientRect(hWnd, &rect);
			}
			break;

		case '+':
			rect.right += 50;
			rect.bottom += 50;
			break;

		case '-':
			rect.right -= 50;
			rect.bottom -= 50;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			a_pressed = false;
			div = wParam - '0';
			GetClientRect(hWnd, &rect);
			break;

		case 'r':
			r_pressed = !r_pressed;
			break;

		case 'q':
			PostQuitMessage(0);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		if (2 > div) { break; }

		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		for (int i = 0; i < div; ++i) {
			if (((rect.right / div) * i < x) &&
				(x < (rect.right) / div * (i + 1))) {
				sel = i;
				break;
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONDOWN: {
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		SelectObject(mDC, (HBITMAP)hBitmap);

		for (int i = 0; i < div; ++i) {
			if (i == sel) {
				if (r_pressed) {
					StretchBlt(hDC,
						(rect.right / div) * i, 0,
						(rect.right / div), rect.bottom,
						mDC,
						0, 0,
						bmp.bmWidth, bmp.bmHeight,
						NOTSRCCOPY);
				}
				else {
					StretchBlt(hDC,
						(rect.right / div)* i, 0,
						(rect.right / div), rect.bottom,
						mDC,
						0, 0,
						bmp.bmWidth, bmp.bmHeight,
						SRCCOPY);
				}

				HPEN oldPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 5, RGB(255, 0, 0))); // 예시: 빨간색 테두리
				HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

				Rectangle(hDC,
					(rect.right / div) * i, 0,
					(rect.right / div) * (i + 1), rect.bottom);

				SelectObject(hDC, oldBrush);
				SelectObject(hDC, oldPen);
				DeleteObject(oldPen); 
			}
			else {
				StretchBlt(hDC,
					(rect.right / div)* i, 0,
					(rect.right / div), rect.bottom,
					mDC,
					0, 0,
					bmp.bmWidth, bmp.bmHeight,
					SRCCOPY);
			}
		}

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
