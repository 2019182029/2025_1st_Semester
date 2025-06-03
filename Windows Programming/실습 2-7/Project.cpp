#include <windows.h>
#include <tchar.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

const int WIDTH = 800;
const int HEIGHT = 600;

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

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hDC;
	PAINTSTRUCT ps;
	static SIZE size;
	static TCHAR str[10][100];
	static int row, col;

	switch (iMessage) {
	case WM_CREATE:
		CreateCaret(hWnd, NULL, 5, 15);
		ShowCaret(hWnd);
		ZeroMemory(&str, sizeof(str));
		row = 0; col = 0;
		break;

	case WM_CHAR:
		// Input
		if (VK_SPACE == wParam
		    || ('A' <= wParam && wParam <= 'Z')
			|| ('a' <= wParam && wParam <= 'z')) {
			str[row][col++] = wParam;

			// 문자열의 끝이라면 NULL 문자 추가
			if (!str[row][col])
				str[row][col] = '\0';

			// 문자가 30자 이상 되면 다음 줄로 내려간다.
			if (col == 30) {
				row = (row + 1) % 10;
				col = 0;
			}
		}
		// Return
		else if (VK_RETURN == wParam) {
			row = (row + 1) % 10;
			col = 0;
		}
		// Back
		else if (VK_BACK == wParam) {
			// 캐럿 위치의 문자를 삭제하고 뒤의 문자들을 앞으로 이동시킨다.
			if (0 < col) {
				for (int i = col - 1; i < lstrlen(str[row]); ++i) {
					str[row][i] = str[row][i + 1];
				}
				--col;
			}
			// 맨 앞의 위치에서는 전 줄로 캐럿이 이동한다.
			else {
				row = (0 != row) ? row - 1 : 9;
				col = lstrlen(str[row]);
			}
		}
		// Escape
		else if (VK_ESCAPE == wParam) {
			HideCaret(hWnd);
			DestroyCaret();
			PostQuitMessage(0);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		GetTextExtentPoint32(hDC, str[row], col, &size);
		for (int i = 0; i < 10; ++i)
			TextOut(hDC, 0, i * 15, str[i], lstrlen(str[i]));
		SetCaretPos(size.cx, row * 15);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		HideCaret(hWnd);
		DestroyCaret();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}