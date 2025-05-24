#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include "resource.h"

#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

HWND g_hWnd;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 1000;

constexpr int PERIOD = 360;
constexpr int CIRCLE_RADIUS = 200;

int CURVE = 0;
int OFFSET = 0;
int HEIGHT = 100;

int CIRCLE_X = 0;
int CIRCLE_Y = 0;
int CIRCLE_INDEX = 0;
int DIST = 0;
double CIRCLE_ANGLE = -180.0;
bool CIRCLE = false;

bool check[4] = { false, false, false, false };

POINT stair[20];

void draw_sin(HDC hDC, const RECT& rect) {
	MoveToEx(hDC, 0, (rect.bottom / 2), NULL);

	for (int x = 0; x < rect.right; ++x) {
		double rad = (x + OFFSET) * 2 * 3.141592 / 180.0;

		int y = (int)(sin(rad) * HEIGHT);

		LineTo(hDC, x, (rect.bottom / 2) - y);

		if (CIRCLE) {
			if (x == CIRCLE_X) {
				Ellipse(hDC,
					(x - 20), (rect.bottom / 2) - (y - 20),
					(x + 20), (rect.bottom / 2) - (y + 20));
			}
		}
	}

	if (CIRCLE) {
		++CIRCLE_X;
	}
}

void draw_circle(HDC hDC, const RECT& rect) {
	for (int x = 0; x < (rect.right - OFFSET + CIRCLE_RADIUS); x += (2 * CIRCLE_RADIUS)) {
		double angle = -180.0;
		double rad = angle * 3.141592 / 180.0;

		int cx = (int)(cos(rad) * CIRCLE_RADIUS) + (x + OFFSET);
		int cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

		MoveToEx(hDC, cx, cy, NULL);

		while (angle < 0.0) {
			rad = angle * 3.141592 / 180.0;

			cx = (int)(cos(rad) * CIRCLE_RADIUS) + (x + OFFSET);
			cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

			LineTo(hDC, cx, cy);

			if (CIRCLE) {
				if ((x == CIRCLE_X) &&
					(angle == CIRCLE_ANGLE)) {
					Ellipse(hDC,
						(cx - 20), (cy - 20),
						(cx + 20), (cy + 20));
				}
			}

			angle += 1.0;
		}
	}

	if (CIRCLE) {
		CIRCLE_ANGLE += 3.0;

		if (CIRCLE_ANGLE > 0.0) {
			CIRCLE_X += (2 * CIRCLE_RADIUS);
			CIRCLE_ANGLE = -180.0;
		}
	}

	for (int x = 0; x < (rect.right - OFFSET + CIRCLE_RADIUS); x += (2 * CIRCLE_RADIUS)) {
		double angle = 0.0;
		double rad = angle * 3.141592 / 180.0;

		int cx = (int)(cos(rad) * CIRCLE_RADIUS) + (x + OFFSET);
		int cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

		MoveToEx(hDC, cx, cy, NULL);

		while (angle < 180.0) {
			angle += 1.0;
			rad = angle * 3.141592 / 180.0;

			cx = (int)(cos(rad) * CIRCLE_RADIUS) + (x + OFFSET);
			cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

			LineTo(hDC, cx, cy);
		}
	}
}

void draw_spring(HDC hDC, const RECT& rect) {
	double angle = -180.0;
	double rad = angle * 3.141592 / 180.0;

	int cx = (int)(cos(rad) * CIRCLE_RADIUS) + OFFSET;
	int cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

	MoveToEx(hDC, cx, cy, NULL);

	for (int x = 0; x < (rect.right - OFFSET + CIRCLE_RADIUS); ++x) {
		angle += 1.0;
		rad = angle * 3.141592 / 180.0;

		cx = (int)(cos(rad) * CIRCLE_RADIUS) + (x + OFFSET);
		cy = (int)(sin(rad) * HEIGHT) + (rect.bottom / 2);

		LineTo(hDC, cx, cy);

		if (CIRCLE) {
			if ((x == CIRCLE_X)) {
				Ellipse(hDC,
					(cx - 20), (cy - 20),
					(cx + 20), (cy + 20));
			}
		}
	}

	if (CIRCLE) {
		CIRCLE_X += 8;
	}
}

void draw_stair(HDC hDC, const RECT& rect) {
	for (int i = 0; i < 19; ++i) {
		MoveToEx(hDC, (stair[i].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[i].y / 2), NULL);
		LineTo(hDC, (stair[i + 1].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[i + 1].y / 2));
	}
	MoveToEx(hDC, (stair[19].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[19].y / 2), NULL);
	LineTo(hDC, (stair[0].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[0].y / 2));

	for (int i = 0; i < 19; ++i) {
		MoveToEx(hDC, (rect.right + stair[i].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[i].y / 2), NULL);
		LineTo(hDC, (rect.right + stair[i + 1].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[i + 1].y / 2));
	}
	MoveToEx(hDC, (rect.right + stair[19].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[19].y / 2), NULL);
	LineTo(hDC, (rect.right + stair[0].x + OFFSET), (rect.bottom / 2) - (HEIGHT * stair[0].y / 2));

	if (CIRCLE) {
		switch (CIRCLE_INDEX) {
		case 0:
		case 2:
		case 4:
		case 6:
		case 8:
			++CIRCLE_X;
			if (DIST >= (rect.right / 6)) {
				DIST = 0;
				++CIRCLE_INDEX;
				CIRCLE_INDEX = CIRCLE_INDEX % 20;
			}
			break;

		case 15:
		case 17:
		case 19:
		case 1:
		case 3:
			--CIRCLE_Y;
			if (DIST >= (HEIGHT)) {
				DIST = 0;
				++CIRCLE_INDEX;
				CIRCLE_INDEX = CIRCLE_INDEX % 20;
			}
			break;

		case 5:
		case 7:
		case 9:
		case 11:
		case 13:
			++CIRCLE_Y;
			if (DIST >= (HEIGHT)) {
				DIST = 0;
				++CIRCLE_INDEX;
				CIRCLE_INDEX = CIRCLE_INDEX % 20;
			}
			break;

		case 10:
		case 12:
		case 14:
		case 16:
		case 18:
			--CIRCLE_X;
			if (DIST >= (rect.right / 6)) {
				DIST = 0;
				++CIRCLE_INDEX;
				CIRCLE_INDEX = CIRCLE_INDEX % 20;
			}
			break;
		}

		++DIST;

		Ellipse(hDC,
			(rect.right / 12 + CIRCLE_X - 20) + OFFSET, (rect.bottom / 2 - HEIGHT / 2) + (CIRCLE_Y - 20),
			(rect.right / 12 + CIRCLE_X + 20) + OFFSET, (rect.bottom / 2 - HEIGHT / 2) + (CIRCLE_Y + 20));

		Ellipse(hDC,
			(rect.right / 12 + CIRCLE_X - 20) + OFFSET + rect.right, (rect.bottom / 2 - HEIGHT / 2) + (CIRCLE_Y - 20),
			(rect.right / 12 + CIRCLE_X + 20) + OFFSET + rect.right, (rect.bottom / 2 - HEIGHT / 2) + (CIRCLE_Y + 20));
	}

	OFFSET = OFFSET % rect.right;
}

//////////////////////////////////////////////////
// WINMAIN
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE	hPrevInstance, _In_ LPSTR lpszCmdParam, _In_ int nCmdShow) {
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
	//WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

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
	HBITMAP hBitmap;
	RECT rt;
	GetClientRect(hWnd, &rt);

	static int count = 0;
	static int dy = -1;

	switch (iMessage) {
	case WM_CREATE:
		stair[0].x = rt.right / 12;			stair[0].y = -1;
		stair[1].x = rt.right / 4;			stair[1].y = -1;
		stair[2].x = rt.right / 4;			stair[2].y = -3;
		stair[3].x = 5 * rt.right / 12;		stair[3].y = -3;
		stair[4].x = 5 * rt.right / 12;		stair[4].y = -5;
		stair[5].x = 7 * rt.right / 12;		stair[5].y = -5;
		stair[6].x = 7 * rt.right / 12;		stair[6].y = -3;
		stair[7].x = 3 * rt.right / 4;		stair[7].y = -3;
		stair[8].x = 3 * rt.right / 4;		stair[8].y = -1;
		stair[9].x = 11 * rt.right / 12;	stair[9].y = -1;

		for (int i = 0; i < 10; ++i) {
			stair[10 + i].x = rt.right - stair[i].x;
			stair[10 + i].y = -stair[i].y;
		}
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			OFFSET -= 10;
			break;

		case 1:
			++count;
			HEIGHT += dy;

			if (60 == count) {
				count = 0;
				dy *= -1;
			}
			break;

		case 2:
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_COMMAND:
		break;

	case WM_CHAR:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dialog_Proc);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		SelectObject(mDC, (HBITMAP)hBitmap);

		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		int r = 0;
		int g = 0;
		int b = 0;

		if (check[0]) { g = min(g + 255, 255); b = min(b + 255, 255); }
		if (check[1]) { r = min(r + 255, 255); b = min(b + 255, 255); }
		if (check[2]) { r = min(r + 255, 255); g = min(g + 255, 255); }
		if (check[3]) { r = 255 - r; g = 255 - g; b = 255 - b; }

		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(r, g, b));
		HPEN oldPen = (HPEN)SelectObject(mDC, hPen);

		switch (CURVE) {
		case 0:
			draw_sin(mDC, rt);
			break;

		case 1:
			draw_circle(mDC, rt);
			break;

		case 2:
			draw_spring(mDC, rt);
			break;

		case 3:
			draw_stair(mDC, rt);
			break;
		}

		SelectObject(mDC, oldPen);
		DeleteObject(hPen);

		MoveToEx(mDC, 0, rt.bottom / 2, NULL);
		LineTo(mDC, rt.right, rt.bottom / 2);

		MoveToEx(mDC, rt.right / 2, 0, NULL);
		LineTo(mDC, rt.right / 2, rt.bottom);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

void Reset(HWND hDlg) {
	CheckRadioButton(hDlg, IDC_RADIO2, IDC_RADIO1, IDC_RADIO1);
	CURVE = 0;

	KillTimer(g_hWnd, 0);
	KillTimer(g_hWnd, 1);
	CIRCLE = false;

	OFFSET = 0;
	HEIGHT = 100;

	CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK2, BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK3, BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_CHECK4, BST_UNCHECKED);
	for (auto& b : check) {
		b = false;
	}
}

BOOL Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC;

	switch (iMessage) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO2, IDC_RADIO1, IDC_RADIO1);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO1:
			CURVE = 0;
			break;

		case IDC_RADIO2:
			CURVE = 1;
			break;

		case IDC_RADIO3:
			CURVE = 2;
			break;

		case IDC_RADIO4:
			CURVE = 3;
			break;

		case IDC_BUTTON1:
			OFFSET = 0;
			SetTimer(g_hWnd, 0, 16, NULL);
			break;

		case IDC_BUTTON2:
			SetTimer(g_hWnd, 1, 16, NULL);
			break;

		case IDC_BUTTON3:
			KillTimer(g_hWnd, 0);
			KillTimer(g_hWnd, 1);
			break;

		case IDC_BUTTON4:
			Reset(hDlg);
			break;

		case IDC_BUTTON5:
			OFFSET = 0;
			CIRCLE_X = 0;
			CIRCLE_Y = 0;
			CIRCLE_INDEX = 0;
			DIST = 0;
			CIRCLE_ANGLE = -180.0;
			CIRCLE = true;
			SetTimer(g_hWnd, 2, 16, NULL);
			break;

		case IDC_BUTTON6:
			break;

		case IDC_BUTTON7:
			EndDialog(hDlg, 0);
			break;

		case IDC_CHECK1:
			check[0] = 1 - check[0];
			break;

		case IDC_CHECK2:
			check[1] = 1 - check[1];
			break;

		case IDC_CHECK3:
			check[2] = 1 - check[2];
			break;

		case IDC_CHECK4:
			check[3] = 1 - check[3];
			break;
		}
		InvalidateRect(g_hWnd, NULL, FALSE);
		break;

	case WM_PAINT: {
		hDC = BeginPaint(hDlg, &ps);
		EndPaint(hDlg, &ps);
		break;
	}

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}
