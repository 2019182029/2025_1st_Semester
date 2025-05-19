#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

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
// CHARACTER
HBITMAP R_RunBit[3];
BITMAP R_RunBmp[3];

class RAT {
public:
	int m_x, m_y;
	int m_count;

	RAT(int x, int y) : m_x(x), m_y(y) {
		m_count = 0;
	}

	void print(HDC mDC) const {
		HDC cDC = CreateCompatibleDC(mDC);

		SelectObject(cDC, R_RunBit[m_count]);

		TransparentBlt(mDC, m_x, m_y, 100, 100, cDC, 0, 0, R_RunBmp[m_count].bmWidth, R_RunBmp[m_count].bmHeight, RGB(255, 255, 255));

		DeleteDC(cDC);
	}
};

RAT* rat = nullptr;

//////////////////////////////////////////////////
// CHARACTER
HBITMAP RunBit[6];
BITMAP RunBmp[6];
bool r_clicked = false;

class CHARACTER {
public:
	int m_x, m_y;
	int m_dx, m_dy;
	int m_count;

	CHARACTER() {
		m_x = (WIDTH / 2);
		m_y = (HEIGHT / 2);
		m_dx = ((rand() % 10) - 5);
		m_dy = ((rand() % 10) - 5);
		m_count = 0;
	}

	void move() {
		int new_x, new_y;

		if (rat) {
			m_dx = rat->m_x - m_x;
			m_dy = rat->m_y - m_y;

			int length = sqrt(m_dx * m_dx + m_dy * m_dy);

			m_dx = (0 == length) ? 0 : (3 * m_dx / length);
			m_dy = (0 == length) ? 0 : (3 * m_dy / length);
		}

		new_x = m_x + m_dx;
		new_y = m_y + m_dy;

		if ((0 < new_x) && (new_x < WIDTH)) {
			m_x = new_x;
		}

		if ((0 < new_y) && (new_y < HEIGHT)) {
			m_y = new_y;
		}
	}

	void print(HDC mDC) const {
		HDC cDC = CreateCompatibleDC(mDC);

		if (!r_clicked) {
			SelectObject(cDC, RunBit[m_count]);
			TransparentBlt(mDC, m_x, m_y, 100, 100, cDC, 0, 0, RunBmp[m_count].bmWidth, RunBmp[m_count].bmHeight, RGB(255, 255, 255));
		} else {
			SelectObject(cDC, RunBit[m_count + 3]);
			TransparentBlt(mDC, m_x, m_y, 100, 100, cDC, 0, 0, RunBmp[m_count + 3].bmWidth, RunBmp[m_count + 3].bmHeight, RGB(255, 255, 255));
		}

		DeleteDC(cDC);
	}
};

CHARACTER character;

//////////////////////////////////////////////////
// WINMAIN
HWND g_hWnd;
bool running = true;
auto count_t = std::chrono::system_clock::now();
auto move_t = std::chrono::system_clock::now();
auto dir_t = std::chrono::system_clock::now();

void worker() {
	while (running) {
		auto current_t = std::chrono::system_clock::now();

		if (100 < std::chrono::duration_cast<std::chrono::milliseconds>(current_t - count_t).count()) {
			character.m_count = ((character.m_count + 1) % 3);
			if (rat) {
				rat->m_count = ((rat->m_count + 1) % 3);
			}
			count_t = current_t;
		}

		if (16 < std::chrono::duration_cast<std::chrono::milliseconds>(current_t - move_t).count()) {
			character.move();
			move_t = current_t;
		}

		if (1000 < std::chrono::duration_cast<std::chrono::milliseconds>(current_t - dir_t).count()) {
			if (!rat) {
				character.m_dx = ((rand() % 10) - 5);
				character.m_dy = ((rand() % 10) - 5);
			}
			dir_t = current_t;
		}

		InvalidateRect(g_hWnd, NULL, FALSE);
	}
}

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
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	g_hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
		0, 0, WIDTH, HEIGHT,
		NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	std::thread worker_thread(worker);

	while (running) {
		while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
			if (Message.message == WM_QUIT) {
				running = false;
				break;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	worker_thread.join();

	return Message.wParam;
}

//////////////////////////////////////////////////
// WNDPROC
int picked_x = 0;
int picked_y = 0;
bool picked = false;

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;

	switch (iMessage) {
	case WM_CREATE:
		bg.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(bg.m_hBitmap, sizeof(BITMAP), &bg.m_bmp);
		GetClientRect(hWnd, &rect);

		TCHAR file[100];
		for (int i = 1; i <= 6; ++i) {
			swprintf(file, 50, TEXT("Resource\\c%d.bmp"), i);
			RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(RunBit[i - 1], sizeof(BITMAP), &RunBmp[i - 1]);
		}

		for (int i = 1; i <= 3; ++i) {
			swprintf(file, 50, TEXT("Resource\\r%d.bmp"), i);
			R_RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(R_RunBit[i - 1], sizeof(BITMAP), &R_RunBmp[i - 1]);
		}
		break;

	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		break;


	case WM_COMMAND:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CHAR:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		int x = LOWORD(lParam) - R_RunBmp[0].bmWidth;
		int y = HIWORD(lParam) - R_RunBmp[0].bmHeight;

		if (!rat) {
			rat = new RAT(x, y);
		} else {
			if ((rat->m_x - 100 < x) &&
				(x < rat->m_x + 100) &&
				(rat->m_y - 100 < y) &&
				(y < rat->m_y + 100)) {
				picked_x = x;
				picked_y = y;
				picked = true;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		if (picked) {
			picked = false;
			delete rat;
			rat = nullptr;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_MOUSEMOVE: {
		if (picked) {
			int x = LOWORD(lParam) - R_RunBmp[0].bmWidth;
			int y = HIWORD(lParam) - R_RunBmp[0].bmHeight;

			int dx = x - picked_x;
			int dy = y - picked_y;

			rat->m_x += dx;
			rat->m_y += dy;

			picked_x = x;
			picked_y = y;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONDOWN: {
		r_clicked = !r_clicked;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONUP: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HGDIOBJ old_hBitmap = SelectObject(mDC, hBitmap);

		bg.print(mDC);

		if (rat) {
			rat->print(mDC);
		}
		character.print(mDC);

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
