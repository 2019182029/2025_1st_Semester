#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>

#pragma comment (lib, "msimg32.lib")
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// Background
constexpr int WIDTH = 1024;
constexpr int HEIGHT = 800;

RECT rect;

class BACKGROUND {
public:
	int m_x;
	int m_speed;
	BITMAP m_bmp;
	HBITMAP m_hBitmap;

	void print(HDC mDC) {
		HDC bDC = CreateCompatibleDC(mDC);
		HGDIOBJ hBitmap = SelectObject(bDC, m_hBitmap);

		TransparentBlt(mDC, m_x, 0, WIDTH + 5, HEIGHT, bDC, 0, 0, WIDTH, HEIGHT, RGB(255, 255, 255));
		m_x += m_speed;
		if (m_x >= WIDTH) { m_x = -WIDTH + (m_x % WIDTH); }

		SelectObject(bDC, hBitmap);
		DeleteDC(bDC);
	}
};

BACKGROUND sky1, sky2, ground1, ground2;

//////////////////////////////////////////////////
// CHARACTER
HBITMAP RunBit[6];
HBITMAP A_RunBit[6];

class CHARACTER {
public:
	int m_x, m_y;
	int m_count;
	int m_x_speed, m_y_speed;
	bool m_pressed = false;
	bool m_hitted = false;

	void move() {
		m_x += m_x_speed;
		m_y += m_y_speed;

		++m_y_speed;
		if (m_y > HEIGHT - 300) {
			m_y = HEIGHT - 300;
			m_y_speed = 0;
		}
	}

	void print(HDC mDC) {
		HDC cDC = CreateCompatibleDC(mDC);

		if (!m_pressed) {
			if (!m_hitted) {
				SelectObject(cDC, RunBit[m_count]);
				m_count = (m_count + 1) % 5;
			} else {
				SelectObject(cDC, A_RunBit[m_count]);
				m_count = (m_count + 1) % 5;
			}
		} else {
			if (!m_hitted) {
				SelectObject(cDC, RunBit[5]);
			} else {
				SelectObject(cDC, A_RunBit[5]);
			}
		}

		move();

		TransparentBlt(mDC, m_x, m_y, 90, 90, cDC, 0, 0, 90, 90, RGB(0, 0, 255));

		DeleteDC(cDC);
	}
};

CHARACTER character;

//////////////////////////////////////////////////
// MONSTER
HBITMAP M_RunBit[4];

class MONSTER {
public:
	int m_x, m_y;
	int m_count = 0;
	int m_speed;

	void move() {
		m_x += m_speed;

		if (WIDTH < m_x) { 
			m_x = -200; 

			switch (rand() % 2) {
			case 0:
				m_y = HEIGHT - 250;
				break;

			case 1:
				m_y = HEIGHT - 350;
				break;
			}
		}
	}

	void print(HDC mDC) {
		HDC cDC = CreateCompatibleDC(mDC);

		SelectObject(cDC, M_RunBit[m_count]);
		m_count = (m_count + 1) % 4;

		move();

		TransparentBlt(mDC, m_x, m_y, 80, 66, cDC, 0, 0, 80, 66, RGB(0, 0, 255));

		DeleteDC(cDC);
	}
};

MONSTER monster;

//////////////////////////////////////////////////
// Collision
bool player_monster_collision() {
	int c_l = character.m_x;
	int c_r = character.m_x + 90;
	int c_t = character.m_pressed ? character.m_y + 45 : character.m_y;
	int c_b = character.m_y + 90;

	int m_l = monster.m_x;
	int m_r = monster.m_x + 80;
	int m_t = monster.m_y;
	int m_b = monster.m_y + 66;

	if ((c_l < m_r) &&
		(c_r > m_l) &&
		(c_t < m_b) &&
		(c_b > m_t)) {
		return true;
	}
	return false;
}

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
bool l_pressed = false;
bool r_pressed = false;
bool j_pressed = false;

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;

	switch (iMessage) {
	case WM_CREATE:
		sky1.m_speed = 10;
		sky1.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\sky.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(sky1.m_hBitmap, sizeof(BITMAP), &sky1.m_bmp);

		sky2.m_x = WIDTH;
		sky2.m_speed = 10;
		sky2.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\sky.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(sky2.m_hBitmap, sizeof(BITMAP), &sky2.m_bmp);
		
		ground1.m_speed = 5;
		ground1.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\ground.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(ground1.m_hBitmap, sizeof(BITMAP), &ground1.m_bmp);

		ground2.m_x = WIDTH;
		ground2.m_speed = 5;
		ground2.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\ground.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(ground2.m_hBitmap, sizeof(BITMAP), &ground2.m_bmp);

		character.m_x = WIDTH - 200; character.m_y = HEIGHT - 300;

		monster.m_speed = 10;
		monster.m_x = -200; monster.m_y = HEIGHT - 250;

		GetClientRect(hWnd, &rect);
		
		TCHAR file[100];
		for (int i = 1; i < 5 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\monster%d.bmp"), i);
			RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 1; i < 4 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\dog%d.bmp"), i);
			M_RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		swprintf(file, 50, TEXT("Resource\\monster6.bmp"));
		RunBit[5] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		for (int i = 1; i < 5 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\monster%d.bmp"), i + 6);
			A_RunBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		swprintf(file, 50, TEXT("Resource\\monster12.bmp"));
		A_RunBit[5] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		SetTimer(hWnd, 0, 16, NULL);
		SetTimer(hWnd, 1, 16, NULL);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
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
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT:
			if (!l_pressed) { 
				l_pressed = true;
				character.m_x_speed += -10; 
			}
			break;

		case VK_RIGHT:
			if (!r_pressed) {
				r_pressed = true;
				character.m_x_speed += 10;
			}
			break;

		case VK_UP:
			character.m_y_speed -= 10;
			break;

		case VK_DOWN:
			character.m_pressed = true;
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYUP:
		switch (wParam) {
		case VK_LEFT:
			if (l_pressed) {
				l_pressed = false;
				character.m_x_speed += 10;
			}
			break;

		case VK_RIGHT:
			if (r_pressed) {
				r_pressed = false;
				character.m_x_speed += -10;
			}
			break;

		case VK_DOWN:
			character.m_pressed = false;
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_MOUSEMOVE: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONDOWN: {
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

		sky1.print(mDC);
		sky2.print(mDC);
		ground1.print(mDC);
		ground2.print(mDC);

		character.print(mDC);
		monster.print(mDC);

		if (player_monster_collision()) {
			character.m_hitted = true;
		} else {
			character.m_hitted = false;
		}

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
