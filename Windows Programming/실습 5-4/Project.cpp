#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>

#pragma comment (lib, "msimg32.lib")
//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

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
// Background
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
// SQUARE
class SQUARE {
public:
	int m_top, m_bottom, m_left, m_right;

	void print(HDC hDC) const {
		HPEN oldPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 5, RGB(255, 0, 0)));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, m_left, m_top, m_right, m_bottom);

		SelectObject(hDC, oldPen);
		DeleteObject(oldPen);
	}
};

SQUARE square;

//////////////////////////////////////////////////
// CHARACTER
bool p_pressed = false;
HBITMAP RunBit[5], PriBit[5];

class CHARACTER {
public:
	int m_x, m_y;
	int m_velocity;
	int m_count;
	bool m_prisoned = false;

	CHARACTER(int x, int y) : m_x(x), m_y(y) { 
		m_velocity = rand() % 10;
		m_count = 0; 
	}

	void move() {
		if (p_pressed) {
			if ((square.m_left < (m_x + 45)) &&
				((m_x + 45) < square.m_right) &&
				(square.m_top < (m_y + 45)) &&
				((m_y + 45) < square.m_bottom)) {
				m_prisoned = true;

				if ((m_y + 90 + m_velocity) < square.m_bottom) {
					m_y += m_velocity;
				}
			}
			else {
				m_prisoned = false;

				m_y += m_velocity;
			}
		}

		if (rect.bottom < m_y) {
			m_y = -90;
		}

		m_count = (m_count + 1) % 5;
	}

	void print(HDC mDC) const {
		HDC cDC = CreateCompatibleDC(mDC);

		if (!m_prisoned) {
			SelectObject(cDC, RunBit[m_count]);
		} else {
			SelectObject(cDC, PriBit[m_count]);
		}

		TransparentBlt(mDC, m_x, m_y, 90, 90, cDC, 0, 0, 90, 90, RGB(0, 0, 64));

		DeleteDC(cDC);
	}
};

std::vector<CHARACTER> characters;

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
bool left_clicked = false;
bool right_clicked = false;
int right_clicked_x = 0;
int right_clicked_y = 0;

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

			swprintf(file, 50, TEXT("Resource\\d%d.bmp"), i);
			PriBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 0; i < 30; ++i) {
			characters.emplace_back((rand() % rect.right), 0);
		}

		SetTimer(hWnd, 0, 100, NULL);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			for (auto& character : characters) {
				character.move();
			}
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
		switch (wParam) {
		case 'p':
			p_pressed = true;
			break;

		case 'd':
			square.m_top = 0;
			square.m_bottom = 0;
			square.m_left = 0;
			square.m_right = 0;
			break;

		case 'r':
			p_pressed = false;

			square.m_top = 0;
			square.m_bottom = 0;
			square.m_left = 0;
			square.m_right = 0;

			for (auto& character : characters) {
				character.m_y = 0;
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		left_clicked = true;

		square.m_top = HIWORD(lParam);
		square.m_bottom = HIWORD(lParam);
		square.m_left = LOWORD(lParam);
		square.m_right = LOWORD(lParam);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		left_clicked = false;
		break;
	}

	case WM_MOUSEMOVE:
		if (left_clicked) {
			square.m_bottom = HIWORD(lParam);
			square.m_right = LOWORD(lParam);
		} else if (right_clicked) {
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			int x_gap = x - right_clicked_x;
			int y_gap = y - right_clicked_y;

			square.m_top += y_gap;
			square.m_bottom += y_gap;
			square.m_left += x_gap;
			square.m_right += x_gap;

			for (auto& character : characters) {
				if (character.m_prisoned) {
					character.m_x += x_gap;
					character.m_y += y_gap;
				}
			}

			right_clicked_x = x;
			right_clicked_y = y;
		}
		break;

	case WM_RBUTTONDOWN: {
		right_clicked = true;

		right_clicked_x = LOWORD(lParam);
		right_clicked_y = HIWORD(lParam);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONUP: {
		right_clicked = false;
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HGDIOBJ old_hBitmap = SelectObject(mDC, hBitmap);

		bg.print(mDC);

		for (const auto& character : characters) {
			character.print(mDC);
		}

		square.print(mDC);

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
