#include <windows.h>
#include <tchar.h>
#include <math.h>
#include "resource.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int WIDTH = 1000;
constexpr int HEIGHT = 1000;

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
	//WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
	HBITMAP hBitmap;
	RECT rt;

	switch (iMessage) {
	case WM_CREATE:
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
		GetClientRect(hWnd, &rt);
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		SelectObject(mDC, (HBITMAP)hBitmap);



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

	static int shape = 0;
	static int size = 1;
	static int check[2] = { 0, 0 };
	static bool draw = false;

	static bool left_clicked = false;
	static int from_x = 50;
	static int from_y = 50;
	static int to_x = from_x;
	static int to_y = from_y;
	static int moved_x = 0;

	switch (iMessage) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO2, IDC_RADIO3, IDC_RADIO2);
		CheckRadioButton(hDlg, IDC_RADIO4, IDC_RADIO6, IDC_RADIO4);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO2:
			shape = 0;
			break;

		case IDC_RADIO3:
			shape = 1;
			break;

		case IDC_RADIO4:
			size = 1;
			break;

		case IDC_RADIO5:
			size = 2;
			break;

		case IDC_RADIO6:
			size = 3;
			break;

		case IDC_CHECK1:
			check[0] = 1 - check[0];
			break;

		case IDC_CHECK2:
			check[1] = 1 - check[1];
			break;

		case IDC_BUTTON1:
			draw = true;
			break;

		case IDC_BUTTON2:
			draw = false;
			SetTimer(hDlg, 0, 16, NULL);
			break;

		case IDC_BUTTON3:
			EndDialog(hDlg, 0);
			break;
		}
		InvalidateRect(hDlg, NULL, TRUE);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			int dx = to_x - from_x;
			int dy = to_y - from_y;

			int length = sqrt(dx * dx + dy * dy);

			if (5 > length) {
				from_x = to_x;
				from_y = to_y;
				KillTimer(hDlg, 0);
				break;
			}

			from_x += (5 * dx / length);
			from_y += (5 * dy / length);
			break;
		}
		InvalidateRect(hDlg, NULL, TRUE);
		break;

	case WM_LBUTTONDOWN:
		if (!draw) { break; }
		left_clicked = true;
		break;

	case WM_MOUSEMOVE:
		if (!draw) { break; }
		if (!left_clicked) { break; }
		to_x = LOWORD(lParam);
		to_y = HIWORD(lParam);
		InvalidateRect(hDlg, NULL, TRUE);
		break;

	case WM_LBUTTONUP:
		if (!draw) { break; }
		left_clicked = false;
		break;

	case WM_PAINT: {
		hDC = BeginPaint(hDlg, &ps);

		// COLOR
		COLORREF color;
		if (check[0] && check[1]) color = RGB(255, 0, 255);
		else if (check[0]) color = RGB(255, 0, 0);
		else if (check[1]) color = RGB(0, 0, 255);
		else color = RGB(255, 255, 255);

		HBRUSH hBrush = CreateSolidBrush(color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

		// SHAPE
		switch (shape) {
		case 0:
			Rectangle(hDC, from_x - 25, from_y - 25, from_x + 25, from_y + 25);
			break;

		case 1:
			Ellipse(hDC, from_x - 25, from_y - 25, from_x + 25, from_y + 25);
			break;
		}

		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		// PEN
		HPEN hPen = CreatePen(PS_SOLID, size, RGB(0, 0, 0));
		HPEN oldPen = (HPEN)SelectObject(hDC, hPen);
		
		// Line
		MoveToEx(hDC, from_x, from_y, NULL);
		LineTo(hDC, to_x, to_y);

		SelectObject(hDC, oldPen);
		DeleteObject(hPen);

		EndPaint(hDlg, &ps);
		break;
	}

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}
