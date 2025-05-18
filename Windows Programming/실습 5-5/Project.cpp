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
// Object
class OBJECT {
public:
	int m_x, m_y;
	COLORREF m_color;

	OBJECT() {
		m_x = rand() % WIDTH;
		m_y = rand() % HEIGHT;
		m_color = RGB(0, 255, 255);
	}

	void print(HDC hDC) const {
		HBRUSH newBrush = CreateSolidBrush(m_color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, newBrush);

		Rectangle(hDC,
			m_x, m_y,
			m_x + 100, m_y + 100);

		DeleteObject(SelectObject(hDC, oldBrush));
		SelectObject(hDC, oldBrush);
	}
};

std::vector<OBJECT> objects;

//////////////////////////////////////////////////
// Prey
class PREY {
public:
	int m_x, m_y;
	COLORREF m_color;

	PREY() {
		m_x = rand() % WIDTH;
		m_y = rand() % HEIGHT;
		m_color = RGB(rand() % 255, rand() % 255, rand() % 255);
	}

	void print(HDC hDC) const {
		HBRUSH newBrush = CreateSolidBrush(m_color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, newBrush);

		Ellipse(hDC,
			m_x, m_y,
			m_x + 50, m_y + 50);

		DeleteObject(SelectObject(hDC, oldBrush));
		SelectObject(hDC, oldBrush);
	}
};

std::vector<PREY> preys;

//////////////////////////////////////////////////
// BULLET
class BULLET {
public:
	int m_x, m_y;
	int m_dx, m_dy;
	bool m_disabled = false;
	COLORREF m_color;

	BULLET(int x, int y, int dx, int dy) :
	m_x(x), m_y(y), m_dx(dx), m_dy(dy) {
		m_color = RGB(0, 0, 0);
	}

	void move() {
		m_x += m_dx;
		m_y += m_dy;
	}

	void print(HDC hDC) const {
		HBRUSH newBrush = CreateSolidBrush(m_color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, newBrush);

		Ellipse(hDC,
			m_x, m_y,
			m_x + 10, m_y + 10);

		DeleteObject(SelectObject(hDC, oldBrush));
		SelectObject(hDC, oldBrush);
	}
};

std::vector<BULLET> bullets;

//////////////////////////////////////////////////
// CHARACTER
constexpr int UP = 0;
constexpr int DOWN = 1;
constexpr int LEFT = 2;
constexpr int RIGHT = 3;

int t_pressed = 1;

HBITMAP UpBit[2];
HBITMAP DownBit[2];
HBITMAP LeftBit[2];
HBITMAP RightBit[2];

class CHARACTER {
public:
	int m_x, m_y;
	int m_size;
	int m_count;
	int m_dir;
	COLORREF m_color;

	CHARACTER() {
		m_x = WIDTH / 2;
		m_y = HEIGHT / 2;
		m_size = 100;
		m_count = 0;
		m_dir = RIGHT;
		m_color = RGB(255, 255, 0);
	}

	void move() {
		switch (m_dir) {
		case UP:
			m_y -= 5;
			break;

		case DOWN:
			m_y += 5;
			break;

		case LEFT:
			m_x -= 5;
			break;

		case RIGHT:
			m_x += 5;
			break;
		}
	}

	void print(HDC mDC) {
		HDC cDC = CreateCompatibleDC(mDC);

		switch (m_dir) {
		case UP:
			SelectObject(cDC, UpBit[m_count]);
			break;

		case DOWN:
			SelectObject(cDC, DownBit[m_count]);
			break;

		case LEFT:
			SelectObject(cDC, LeftBit[m_count]);
			break;

		case RIGHT:
			SelectObject(cDC, RightBit[m_count]);
			break;
		}

		for (int i = 0; i < 100; ++i) {
			for (int j = 0; j < 100; ++j) {
				if (255 != GetBValue(GetPixel(cDC, i, j))) {
					SetPixel(cDC, i, j, m_color);
				}
			}
		}

		TransparentBlt(mDC, m_x, m_y, m_size, m_size, cDC, 0, 0, 100, 100, RGB(255, 255, 255));

		for (int i = 0; i < t_pressed; ++i) {
			switch (m_dir) {
			case UP:
				TransparentBlt(mDC, m_x, m_y + i * m_size, m_size, m_size, cDC, 0, 0, 100, 100, RGB(255, 255, 255));
				break;

			case DOWN:
				TransparentBlt(mDC, m_x, m_y + i * -m_size, m_size, m_size, cDC, 0, 0, 100, 100, RGB(255, 255, 255));
				break;

			case LEFT:
				TransparentBlt(mDC, m_x + i * m_size, m_y, m_size, m_size, cDC, 0, 0, 100, 100, RGB(255, 255, 255));
				break;

			case RIGHT:
				TransparentBlt(mDC, m_x + i * -m_size, m_y, m_size, m_size, cDC, 0, 0, 100, 100, RGB(255, 255, 255));
				break;
			}
		}
		
		DeleteDC(cDC);
	}
};

CHARACTER character;

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
void player_prey_collision() {
	for (auto iter = preys.begin(); iter != preys.end();) {
		if ((character.m_x < iter->m_x + 25) &&
			(character.m_x + character.m_size > iter->m_x + 25) &&
			(character.m_y < iter->m_y + 25) &&
			(character.m_y + character.m_size > iter->m_y + 25)) {
			character.m_size += 5;
			character.m_color = iter->m_color;
			iter = preys.erase(iter);
			continue;
		}
		++iter;
	}
}

void bullet_object_collision() {
	for (auto& bullet : bullets) {
		for (auto j = objects.begin(); j != objects.end();) {
			if ((bullet.m_x > j->m_x) &&
				(bullet.m_x < j->m_x + 100) &&
				(bullet.m_y > j->m_y) &&
				(bullet.m_y < j->m_y + 100)) {
				bullet.m_disabled = true;
				j = objects.erase(j);
				preys.emplace_back();
				preys.emplace_back();
				continue;
			}
			++j;
		}
	}

	for (auto iter = bullets.begin(); iter != bullets.end();) {
		if (iter->m_disabled) {
			iter = bullets.erase(iter);
			continue;
		}
		++iter;
	}
}

void bullet_prey_collision() {
	int cnt = 0;

	for (auto& bullet : bullets) {
		for (auto j = preys.begin(); j != preys.end();) {
			if ((bullet.m_x > j->m_x) &&
				(bullet.m_x < j->m_x + 50) &&
				(bullet.m_y > j->m_y) &&
				(bullet.m_y < j->m_y + 50)) {
				bullet.m_disabled = true;
				j = preys.erase(j);
				++cnt;
				continue;
			}
			++j;
		}
	}

	for (int i = 0; i < cnt; ++i) {
		preys.emplace_back();
		preys.emplace_back();
	}

	for (auto iter = bullets.begin(); iter != bullets.end();) {
		if (iter->m_disabled) {
			iter = bullets.erase(iter);
			continue;
		}
		++iter;
	}
}

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;

	switch (iMessage) {
	case WM_CREATE:
		bg.m_hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("Resource\\kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(bg.m_hBitmap, sizeof(BITMAP), &bg.m_bmp);
		GetClientRect(hWnd, &rect);

		TCHAR file[100];
		for (int i = 1; i < 2 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\u%d.bmp"), i);
			UpBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 1; i < 2 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\d%d.bmp"), i);
			DownBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 1; i < 2 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\l%d.bmp"), i);
			LeftBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 1; i < 2 + 1; ++i) {
			swprintf(file, 50, TEXT("Resource\\r%d.bmp"), i);
			RightBit[i - 1] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		}

		for (int i = 0; i < 8; ++i) {
			OBJECT object;
			objects.push_back(object);
		}

		SetTimer(hWnd, 0, 250, NULL);
		SetTimer(hWnd, 1, 16, NULL);
		SetTimer(hWnd, 2, 2500, NULL);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 0:
			character.m_count = (character.m_count + 1) % 2;
			break;

		case 1:
			character.move();
			for (auto& bullet : bullets) {
				bullet.move();
			}
			player_prey_collision();
			bullet_object_collision();
			bullet_prey_collision();
			break;

		case 2:
			if (20 > preys.size()) {
				preys.emplace_back();
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
		case 'a':
			preys.clear();
			break;

		case 't':
			if (t_pressed < 3) {
				++t_pressed;
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			character.m_dir = UP;
			break;

		case VK_DOWN:
			character.m_dir = DOWN;
			break;

		case VK_LEFT:
			character.m_dir = LEFT;
			break;

		case VK_RIGHT:
			character.m_dir = RIGHT;
			break;

		case VK_RETURN:
			switch (character.m_dir) {
			case UP:
				bullets.emplace_back(
					character.m_x + (character.m_size / 2),
					character.m_y + (character.m_size / 2),
					0, -25);
				break;

			case DOWN:
				bullets.emplace_back(
					character.m_x + (character.m_size / 2),
					character.m_y + (character.m_size / 2),
					0, 25);
				break;

			case LEFT:
				bullets.emplace_back(
					character.m_x + (character.m_size / 2),
					character.m_y + (character.m_size / 2),
					-25, 0);
				break;

			case RIGHT:
				bullets.emplace_back(
					character.m_x + (character.m_size / 2),
					character.m_y + (character.m_size / 2),
					25, 0);
				break;
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HGDIOBJ old_hBitmap = SelectObject(mDC, hBitmap);

		bg.print(mDC);
		for (const auto& obj : objects) { obj.print(mDC); }
		for (const auto& prey : preys) { prey.print(mDC); }
		for (const auto& bullet : bullets) { bullet.print(mDC); }
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
