#include <windows.h>
#include <tchar.h>
#include <array>
#include <queue>
#include <vector>
#include <random>
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
// PIECE
class PIECE {
public:
	int m_hx, m_hy;
	int m_mx, m_my;
	int m_dx = 0, m_dy = 0;

	PIECE(int mx, int my) :
		m_mx(mx), m_my(my) {}

	void set_h(int hx, int hy) { m_hx = hx; m_hy = hy; }
	void set_m(int mx, int my) { m_mx = mx; m_my = my; }

	void print(HDC hDC, HDC mDC, RECT rect, BITMAP bmp, int division) const {
		StretchBlt(hDC, 
			
			(m_hx * (rect.right / 3)) + m_dx, (m_hy * (rect.bottom / 3)) + m_dy,
			(rect.right / division), (rect.bottom / division),
			
			mDC, 
			
			m_mx * (bmp.bmWidth / 3), m_my * (bmp.bmHeight / 3),
			(bmp.bmWidth / division), (bmp.bmHeight / division), 
			
			SRCCOPY);
	}
};

std::queue<PIECE*> removed_pieces;
std::vector<std::pair<int, int>> empty_spaces;
std::array<std::array<PIECE*, 3>, 3> pieces_3x3;
std::array<std::array<PIECE*, 4>, 4> pieces_4x4;
std::array<std::array<PIECE*, 5>, 5> pieces_5x5;

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
int cur_mode = 3;

bool is_moving = false;

int clicked_x = 0;
int clicked_y = 0;
bool clicked = false;

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	static BITMAP bmp;
	static HBITMAP hBitmap;
	static RECT rect;

	switch (iMessage) {
	case WM_CREATE: {
		hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("tukorea.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetClientRect(hWnd, &rect);
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		std::random_device rd;
		std::mt19937 g(rd());

		// Set 3x3 pieces
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				pieces_3x3[j][i] = new PIECE(i, j);
			}
		}

		std::shuffle(pieces_3x3.begin(), pieces_3x3.end(), g);
		for (int i = 0; i < 3; ++i) {
			std::shuffle(pieces_3x3[i].begin(), pieces_3x3[i].end(), g);
		}

		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				pieces_3x3[j][i]->set_h(i, j);
			}
		}

		PIECE* piece = pieces_3x3[0][0];
		empty_spaces.emplace_back(std::make_pair(piece->m_hx, piece->m_hy));
		pieces_3x3[0][0] = nullptr;
		delete piece;
		break;
	}

	case WM_TIMER:
		switch (wParam) {
		case 0:
			for (auto& p : empty_spaces) {
				if ((cur_mode - 1) == p.second) { continue; }
				if (nullptr == pieces_3x3[p.second + 1][p.first]) { continue; }

				pieces_3x3[p.second + 1][p.first]->m_dy -= ((rect.bottom / 3) / 16);

				if (abs(pieces_3x3[p.second + 1][p.first]->m_dy) >= (rect.bottom / 3)) {
					KillTimer(hWnd, 0);
					is_moving = false;
					pieces_3x3[p.second + 1][p.first]->m_hy -= 1;
					pieces_3x3[p.second + 1][p.first]->m_dy = 0;
					pieces_3x3[p.second][p.first] = pieces_3x3[p.second + 1][p.first];
					pieces_3x3[p.second + 1][p.first] = nullptr;
					++p.second;
				}
			}
			break;

		case 1:
			for (auto& p : empty_spaces) {
				if (0 == p.second) { continue; }
				if (nullptr == pieces_3x3[p.second - 1][p.first]) { continue; }

				pieces_3x3[p.second - 1][p.first]->m_dy += ((rect.bottom / 3) / 16);

				if (abs(pieces_3x3[p.second - 1][p.first]->m_dy) >= (rect.bottom / 3)) {
					KillTimer(hWnd, 1);
					is_moving = false;
					pieces_3x3[p.second - 1][p.first]->m_hy += 1;
					pieces_3x3[p.second - 1][p.first]->m_dy = 0;
					pieces_3x3[p.second][p.first] = pieces_3x3[p.second - 1][p.first];
					pieces_3x3[p.second - 1][p.first] = nullptr;
					--p.second;
				}
			}
			break;

		case 2:
			for (auto& p : empty_spaces) {
				if ((cur_mode - 1) == p.first) { continue; }
				if (nullptr == pieces_3x3[p.second][p.first + 1]) { continue; }

				pieces_3x3[p.second][p.first + 1]->m_dx -= ((rect.right / 3) / 16);

				if (abs(pieces_3x3[p.second][p.first + 1]->m_dx) >= (rect.right / 3)) {
					KillTimer(hWnd, 2);
					is_moving = false;
					pieces_3x3[p.second][p.first + 1]->m_hx -= 1;
					pieces_3x3[p.second][p.first + 1]->m_dx = 0;
					pieces_3x3[p.second][p.first] = pieces_3x3[p.second][p.first + 1];
					pieces_3x3[p.second][p.first + 1] = nullptr;
					++p.first;
				}
			}
			break;

		case 3:
			for (auto& p : empty_spaces) {
				if (0 == p.first) { continue; }
				if (nullptr == pieces_3x3[p.second][p.first - 1]) { continue; }

				pieces_3x3[p.second][p.first - 1]->m_dx += ((rect.right / 3) / 16);

				if (abs(pieces_3x3[p.second][p.first - 1]->m_dx) >= (rect.right / 3)) {
					KillTimer(hWnd, 3);
					is_moving = false;
					pieces_3x3[p.second][p.first - 1]->m_hx += 1;
					pieces_3x3[p.second][p.first - 1]->m_dx = 0;
					pieces_3x3[p.second][p.first] = pieces_3x3[p.second][p.first - 1];
					pieces_3x3[p.second][p.first - 1] = nullptr;
					--p.first;
				}
			}
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_CHAR:
		switch (wParam) {
		case 'q':
			PostQuitMessage(0);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_LBUTTONDOWN: {
		clicked = true;
		clicked_x = LOWORD(lParam);
		clicked_y = HIWORD(lParam);

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}

	case WM_LBUTTONUP:
		clicked = false;
		break;

	case WM_MOUSEMOVE: {
		if (!clicked || is_moving) { break; }

		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (-50 > (y - clicked_y)) {
			is_moving = true;
			SetTimer(hWnd, 0, 16, NULL);
			break;
		}

		if (50 < (y - clicked_y)) {
			is_moving = true;
			SetTimer(hWnd, 1, 16, NULL);
			break;
		}

		if (-50 > (x - clicked_x)) {
			is_moving = true;
			SetTimer(hWnd, 2, 16, NULL);
			break;
		}

		if (50 < (x - clicked_x)) { 
			is_moving = true;
			SetTimer(hWnd, 3, 16, NULL);
			break;
		}

		break;
	}

	case WM_RBUTTONDOWN: {
		if (is_moving) { break; }
		
		int cx = LOWORD(lParam) / (rect.right / 3);
		int cy = HIWORD(lParam) / (rect.bottom / 3);

		if (pieces_3x3[cy][cx]) {
			removed_pieces.push(pieces_3x3[cy][cx]);
			empty_spaces.emplace_back(std::make_pair(cx, cy));
			pieces_3x3[cy][cx] = nullptr;
		}
		else {
			if (!removed_pieces.empty()) {
				PIECE* p = removed_pieces.front();
				p->m_hx = cx;
				p->m_hy = cy;
				pieces_3x3[cy][cx] = p;
				auto iter = std::find_if(empty_spaces.begin(), empty_spaces.end(), [&cx, &cy](const auto& p) {
					return (p.first == cx) && (p.second == cy);
					});
				empty_spaces.erase(iter);
				removed_pieces.pop();
			}
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		SelectObject(mDC, (HBITMAP)hBitmap);

		for (const auto& row : pieces_3x3) {
			for (const auto& piece : row) {
				if (nullptr == piece) { continue; }
				piece->print(hDC, mDC, rect, bmp, 3);
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
