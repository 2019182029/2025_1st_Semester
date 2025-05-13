#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

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
// GLASS
float x_scale = 0.0f;
float y_scale = 0.0f;

class GLASS {
public:
	int m_top, m_bottom, m_left, m_right;
	int m_m_top, m_m_bottom, m_m_left, m_m_right;

	void print(HDC hDC, HDC mDC, bool h_pressed = false, bool v_pressed = false, bool i_pressed = false) const {
		HPEN oldPen = (HPEN)SelectObject(hDC, CreatePen(PS_SOLID, 5, RGB(255, 0, 0)));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		if (h_pressed) {
			StretchBlt(hDC, m_left, m_bottom, (m_right - m_left), (m_top - m_bottom),
				mDC, int(m_m_left * x_scale), int(m_m_top * y_scale), int((m_m_right - m_m_left) * x_scale), int((m_m_bottom - m_m_top) * y_scale), i_pressed ? NOTSRCCOPY : SRCCOPY);
		} else if (v_pressed) {
			StretchBlt(hDC, m_right, m_top, (m_left - m_right), (m_bottom - m_top),
				mDC, int(m_m_left * x_scale), int(m_m_top * y_scale), int((m_m_right - m_m_left) * x_scale), int((m_m_bottom - m_m_top) * y_scale), i_pressed ? NOTSRCCOPY : SRCCOPY);
		} else {
			StretchBlt(hDC, m_left, m_top, (m_right - m_left), (m_bottom - m_top),
				mDC, int(m_m_left * x_scale), int(m_m_top * y_scale), int((m_m_right - m_m_left) * x_scale), int((m_m_bottom - m_m_top) * y_scale), i_pressed ? NOTSRCCOPY : SRCCOPY);
		}

		Rectangle(hDC, m_left, m_top, m_right, m_bottom);

		SelectObject(hDC, oldPen);
		DeleteObject(oldPen);
	}
};

GLASS glass;

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
bool c_pressed = false;
bool f_pressed = false;
bool h_pressed = false;
bool v_pressed = false;
bool m_pressed = false;
bool n_pressed = false;
bool i_pressed = false;

bool left_clicked = false;

bool glass_scale = false;
bool glass_picked = false;
bool glass_created = false;

int dx = 10;
int dy = 10;
int cnt = 0;
bool expand = false;

int old_x = 0;
int old_y = 0;

std::vector<GLASS> glasses;

void reset(HWND hWnd) {
	c_pressed = false;
	f_pressed = false;
	h_pressed = false;
	v_pressed = false;
	m_pressed = false;
	n_pressed = false;
	i_pressed = false;

	left_clicked = false;

	glass_scale = false;
	glass_picked = false;
	glass_created = false;

	dx = 10;
	dy = 10;
	cnt = 0;
	expand = false;

	glass.m_top = 0;
	glass.m_bottom = 0;
	glass.m_left = 0;
	glass.m_right = 0;
	glass.m_m_top = 0;
	glass.m_m_bottom = 0;
	glass.m_m_left = 0;
	glass.m_m_right = 0;

	old_x = 0;
	old_y = 0;

	glasses.clear();

	KillTimer(hWnd, 0);
	KillTimer(hWnd, 1);
}

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC, rDC;
	static BITMAP bmp;
	static HBITMAP hBitmap;
	static RECT rect;

	switch (iMessage) {
	case WM_CREATE:
		hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		GetClientRect(hWnd, &rect);

		x_scale = float(bmp.bmWidth) / rect.right;
		y_scale = float(bmp.bmHeight) / rect.bottom;
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0: {
			if (0 > (glass.m_top + dy) ||
				rect.bottom < (glass.m_bottom + dy)) { 
				dy *= -1; 
			}

			if (0 > (glass.m_left + dx) ||
				rect.right < (glass.m_right + dx)) {
				dx *= -1;
			}

			glass.m_top += dy;
			glass.m_m_top += dy;
			glass.m_bottom += dy;
			glass.m_m_bottom += dy;
			glass.m_left += dx;
			glass.m_m_left += dx;
			glass.m_right += dx;
			glass.m_m_right += dx;

			for (auto& g : glasses) {
				g.m_m_top += dy;
				g.m_m_bottom += dy;
				g.m_m_left += dx;
				g.m_m_right += dx;
			}
			break;

		case 1:
			if (60 < ++cnt) {
				expand = !expand;
				cnt = 0;
			}

			if (expand) {
				glass.m_top += -1;
				glass.m_m_top += -1;
				glass.m_bottom += 1;
				glass.m_m_bottom += 1;
				glass.m_left += -1;
				glass.m_m_left += -1;
				glass.m_right += 1;
				glass.m_m_right += 1;

				for (auto& g : glasses) {
					g.m_m_top += -1;
					g.m_m_bottom += 1;
					g.m_m_left += -1;
					g.m_m_right += 1;
				}
			} else {
				glass.m_top += 1;
				glass.m_m_top += 1;
				glass.m_bottom += -1;
				glass.m_m_bottom += -1;
				glass.m_left += 1;
				glass.m_m_left += 1;
				glass.m_right += -1;
				glass.m_m_right += -1;

				for (auto& g : glasses) {
					g.m_m_top += 1;
					g.m_m_bottom += -1;
					g.m_m_left += 1;
					g.m_m_right += -1;
				}
			}
			break;
		}
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
		case '1':
			hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(hBitmap, sizeof(BITMAP), &bmp);

			x_scale = float(bmp.bmWidth) / rect.right;
			y_scale = float(bmp.bmHeight) / rect.bottom;

			reset(hWnd);
			break;

		case '2':
			hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("tukorea.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(hBitmap, sizeof(BITMAP), &bmp);

			x_scale = float(bmp.bmWidth) / rect.right;
			y_scale = float(bmp.bmHeight) / rect.bottom;

			reset(hWnd);
			break;
		
		case 'e':
			glass.m_m_top += int(5 * y_scale);
			glass.m_m_bottom -= int(5 * y_scale);
			glass.m_m_left += int(5 * x_scale);
			glass.m_m_right -= int(5 * x_scale);
			break;

		case 's':
			glass.m_m_top -= int(5 * y_scale);
			glass.m_m_bottom += int(5 * y_scale);
			glass.m_m_left -= int(5 * x_scale);
			glass.m_m_right += int(5 * x_scale);
			break;

		case 'b':
			glass.m_m_top = glass.m_top;
			glass.m_m_bottom = glass.m_bottom;
			glass.m_m_left = glass.m_left;
			glass.m_m_right = glass.m_right;
			break;

		case 'p': {
			glasses.emplace_back();
			auto iter = glasses.end() - 1;
			iter->m_top = rand() % rect.bottom;
			iter->m_bottom = iter->m_top + ((glass.m_m_bottom - glass.m_m_top) / 2);
			iter->m_left = rand() % rect.right;
			iter->m_right = iter->m_left + ((glass.m_m_right - glass.m_m_left) / 2);
			iter->m_m_top = glass.m_m_top;
			iter->m_m_bottom = glass.m_m_bottom;
			iter->m_m_left = glass.m_m_left;
			iter->m_m_right = glass.m_m_right;
			break;
		}

		case 'f': 
			f_pressed = !f_pressed;
			break;

		case 'h':
			h_pressed = !h_pressed;
			v_pressed = false;
			break;

		case 'v':
			h_pressed = false;
			v_pressed = !v_pressed;
			break;

		case 'm':
			m_pressed = !m_pressed;

			if (m_pressed) {
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 0, 16, NULL);
			} else {
				KillTimer(hWnd, 0);
			}
			break;

		case 'n':
			n_pressed = !n_pressed;

			if (n_pressed) {
				KillTimer(hWnd, 0);
				SetTimer(hWnd, 1, 16, NULL);
			} else {
				KillTimer(hWnd, 1);
			}
			break;

		case 'i':
			i_pressed = !i_pressed;
			break;

		case 'r':
			reset(hWnd);
			break;

		case 'q':
			PostQuitMessage(0);
			return 0;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			glass.m_top -= 5;
			glass.m_m_top -= 5;
			glass.m_bottom -= 5;
			glass.m_m_bottom -= 5;

			for (auto& g : glasses) {
				g.m_m_top -= 5;
				g.m_m_bottom -= 5;
			}
			break;

		case VK_DOWN:
			glass.m_top += 5;
			glass.m_m_top += 5;
			glass.m_bottom += 5;
			glass.m_m_bottom += 5;

			for (auto& g : glasses) {
				g.m_m_top += 5;
				g.m_m_bottom += 5;
			}
			break;

		case VK_LEFT:
			glass.m_left -= 5;
			glass.m_m_left -= 5;
			glass.m_right -= 5;
			glass.m_m_right -= 5;

			for (auto& g : glasses) {
				g.m_m_left -= 5;
				g.m_m_right -= 5;
			}
			break;

		case VK_RIGHT:
			glass.m_left += 5;
			glass.m_m_left += 5;
			glass.m_right += 5;
			glass.m_m_right += 5;

			for (auto& g : glasses) {
				g.m_m_left += 5;
				g.m_m_right += 5;
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		left_clicked = true;

		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (!glass_created) {
			glass.m_top = glass.m_m_top = y;
			glass.m_bottom = glass.m_m_bottom = y;
			glass.m_left = glass.m_m_left = x;
			glass.m_right = glass.m_m_right = x;
		} else {
			if (((glass.m_right - 5) < x) &&
				(x < (glass.m_right + 5)) &&
				((glass.m_bottom - 5) < y) &&
				(y < (glass.m_bottom + 5))) {
				old_x = x;
				old_y = y;
				glass_scale = true;
			} else if ((glass.m_left < x) &&
				(x < glass.m_right) &&
				(glass.m_top < y) &&
				(y < glass.m_bottom)) {
				old_x = x;
				old_y = y;
				glass_picked = true;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_MOUSEMOVE: {
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (left_clicked) {
			if (!glass_created) {
				glass.m_bottom = glass.m_m_bottom = y;
				glass.m_right = glass.m_m_right = x;
			} else {
				int dx = x - old_x;
				int dy = y - old_y;
				old_x = x;
				old_y = y;

				if (glass_scale) {
					glass.m_bottom += dy;
					glass.m_m_bottom += dy;
					glass.m_right += dx;
					glass.m_m_right += dx;

					for (auto& g : glasses) {
						g.m_m_bottom += dy;
						g.m_m_right += dx;
					}
				} else if (glass_picked) {
					glass.m_top += dy;
					glass.m_m_top += dy;
					glass.m_bottom += dy;
					glass.m_m_bottom += dy;
					glass.m_left += dx;
					glass.m_m_left += dx;
					glass.m_right += dx;
					glass.m_m_right += dx;

					for (auto& g : glasses) {
						g.m_m_top += dy;
						g.m_m_bottom += dy;
						g.m_m_left += dx;
						g.m_m_right += dx;
					}
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	}

	case WM_LBUTTONUP:
		left_clicked = false;

		if (!glass_created) {
			glass_created = true;
		} else {
			if (glass_scale) {
				glass_scale = false;
			} else if (glass_picked) {
				glass_picked = false;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_RBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		HBITMAP rBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);

		rDC = CreateCompatibleDC(hDC);
		SelectObject(rDC, (HBITMAP)rBitmap);

		mDC = CreateCompatibleDC(hDC);
		SelectObject(mDC, (HBITMAP)hBitmap);

		if (f_pressed) {
			StretchBlt(rDC, 0, 0, rect.right, rect.bottom,
				mDC, int(glass.m_m_left* x_scale), int(glass.m_m_top* y_scale), int((glass.m_m_right - glass.m_m_left)* x_scale), int((glass.m_m_bottom - glass.m_m_top)* y_scale), SRCCOPY);
		} else {
			StretchBlt(rDC, 0, 0, rect.right, rect.bottom,
				mDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		}

		glass.print(rDC, mDC, false, false, i_pressed);

		for (const auto& g : glasses) {
			g.print(rDC, mDC, h_pressed, v_pressed, i_pressed);
		}

		BitBlt(hDC, 0, 0, rect.right, rect.bottom, rDC, 0, 0, SRCCOPY);

		DeleteDC(mDC);
		DeleteDC(rDC);
		DeleteObject(rBitmap);

		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}
