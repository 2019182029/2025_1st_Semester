#include <windows.h>
#include <tchar.h>
#include <math.h>
#include "resource.h"

HWND g_hWnd;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int WIDTH = 1000;
constexpr int HEIGHT = 1000;
constexpr int RADIUS = 200;

int curve = 0;
int x_gap = 0;
int y_gap = 200;

bool circle = false;
int circle_x = 0;
int circle_y = 0;

POINT stair[20];

void draw_sin(HDC hDC, const RECT& rect) {
	MoveToEx(hDC, 0, rect.bottom / 2, NULL);
	for (int x = 1; x < rect.right; ++x) {
		double rad = (x + x_gap) * 2 * 3.141592 / 180.0;
		int y = (int)(sin(rad) * y_gap);
		LineTo(hDC, x, rect.bottom / 2 - y);

		if (circle) {
			if (x == circle_x) {
				Ellipse(hDC, x - 10, rect.bottom / 2 - y - 10,
					x + 10, rect.bottom / 2 - y + 10);

				circle_x += 1;
				circle %= rt.right;
			}
		}
	}
}

void draw_circle(HDC hDC, const RECT& rect) {
	for (int x = 0; x < rect.right * 2; x += (2 * RADIUS)) {
		double angle = -180.0;
		double rad = angle * 3.141592 / 180.0;

		int cx = (int)(cos(rad) * RADIUS) + (x + x_gap);
		int cy = (int)(sin(rad) * y_gap) + (rect.bottom / 2);

		MoveToEx(hDC, cx, cy, NULL);
		while (angle < 0.0) {
			angle += 1.0;
			rad = angle * 3.141592 / 180.0;

			cx = (int)(cos(rad) * RADIUS) + (x + x_gap);
			cy = (int)(sin(rad) * y_gap) + (rect.bottom / 2);
			LineTo(hDC, cx, cy);
		}
	}
}

void draw_spring(HDC hDC, const RECT& rect) {
	double angle = 180.0;
	double rad = angle * 3.141592 / 180.0;

	int cx = (int)(cos(rad) * RADIUS) + x_gap;
	int cy = (int)(sin(rad) * y_gap) + (rect.bottom / 2);

	MoveToEx(hDC, cx, cy, NULL);
	for (int x = 0; x < rect.right * 2; ++x) {
		angle += 1.0;
		rad = angle * 3.141592 / 180.0;
		cx = (int)(cos(rad) * RADIUS) + (x + x_gap);
		cy = (int)(sin(rad) * y_gap) + (rect.bottom / 2);
		LineTo(hDC, cx, cy);
	}
}

void draw_stair(HDC hDC, const RECT& rect) {
	for (int i = 0; i < 19; ++i) {
		MoveToEx(hDC, stair[i].x + x_gap, rect.bottom / 2 - stair[i].y * y_gap / 2, NULL);
		LineTo(hDC, stair[i + 1].x + x_gap, rect.bottom / 2 - stair[i + 1].y * y_gap / 2);
	}
	MoveToEx(hDC, stair[19].x + x_gap, rect.bottom / 2 - stair[19].y * y_gap / 2, NULL);
	LineTo(hDC, stair[0].x + x_gap, rect.bottom / 2 - stair[0].y * y_gap / 2);

	for (int i = 0; i < 19; ++i) {
		MoveToEx(hDC, rect.right + stair[i].x + x_gap, rect.bottom / 2 - stair[i].y * y_gap / 2, NULL);
		LineTo(hDC, rect.right + stair[i + 1].x + x_gap, rect.bottom / 2 - stair[i + 1].y * y_gap / 2);
	}
	MoveToEx(hDC, rect.right + stair[19].x + x_gap, rect.bottom / 2 - stair[19].y * y_gap / 2, NULL);
	LineTo(hDC, rect.right + stair[0].x + x_gap, rect.bottom / 2 - stair[0].y * y_gap / 2);
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
		0, 0, WIDTH, HEIGHT,
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

	static int dy = -2;

	switch (iMessage) {
	case WM_CREATE:
		stair[0].x = 100;
		stair[0].y = -1;

		stair[1].x = rt.right / 4;
		stair[1].y = -1;

		stair[2].x = rt.right / 4;
		stair[2].y = -2;

		stair[3].x = rt.right / 2 - 100;
		stair[3].y = -2;

		stair[4].x = rt.right / 2 - 100;
		stair[4].y = -3;

		stair[5].x = rt.right / 2 + 100;
		stair[5].y = -3;

		stair[6].x = rt.right / 2 + 100;
		stair[6].y = -2;

		stair[7].x = 3 * rt.right / 4;
		stair[7].y = -2;

		stair[8].x = 3 * rt.right / 4;
		stair[8].y = -1;

		stair[9].x = rt.right - 100;
		stair[9].y = -1;

		for (int i = 0; i < 10; ++i) {
			stair[10 + i].x = rt.right - stair[i].x;
			stair[10 + i].y = -stair[i].y;
		}
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			x_gap -= 10;
			switch (curve) {
			case 0:
			case 2:
				x_gap = x_gap % 360;
				break;

			case 1:
				x_gap = x_gap % (2 * RADIUS);
				break;

			case 3:
				x_gap = x_gap % rt.right;
				break;
			}
			break;

		case 1: {
			int new_y_gap = y_gap + dy;
			if (new_y_gap > 200 || new_y_gap <= 0) {
				dy *= -1;
			}
			y_gap += dy;
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

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		SelectObject(mDC, (HBITMAP)hBitmap);

		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		switch (curve) {
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

		MoveToEx(mDC, 0, rt.bottom / 2, NULL);
		LineTo(mDC, rt.right, rt.bottom / 2);

		MoveToEx(mDC, rt.right / 2, 0, NULL);
		LineTo(mDC, rt.right / 2, rt.bottom);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
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
			curve = 0;
			break;

		case IDC_RADIO2:
			curve = 1;
			break;

		case IDC_RADIO3:
			curve = 2;
			break;

		case IDC_RADIO4:
			curve = 3;
			break;

		case IDC_BUTTON1:
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
			break;

		case IDC_BUTTON5:
			circle = true;
			break;

		case IDC_BUTTON6:
			InvalidateRect(g_hWnd, NULL, FALSE);
			break;

		case IDC_BUTTON7:
			EndDialog(hDlg, 0);
			break;
		}
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
