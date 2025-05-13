#include <windows.h>
#include <tchar.h>
#include <array>
#include <queue>
#include <vector>
#include <random>
#include <thread>
#include <iostream>
#include "resource.h"

#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

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

	void print(HDC hDC, HDC mDC, RECT rect, BITMAP bmp, int r, int c, bool reverse = false) const {
		if (!reverse) {
			StretchBlt(hDC, (m_hx * (rect.right / c)) + m_dx, (m_hy * (rect.bottom / r)) + m_dy,
				(rect.right / c), (rect.bottom / r),
				mDC, m_mx * (bmp.bmWidth / c), m_my * (bmp.bmHeight / r),
				(bmp.bmWidth / c), (bmp.bmHeight / r), SRCCOPY);
		} else {
			StretchBlt(hDC, (m_hx * (rect.right / c)) + m_dx, (m_hy * (rect.bottom / r)) + m_dy,
				(rect.right / c), (rect.bottom / r),
				mDC, m_mx * (bmp.bmWidth / c), m_my * (bmp.bmHeight / r),
				(bmp.bmWidth / c), (bmp.bmHeight / r), NOTSRCCOPY);
		}
	}
};

std::random_device rd;
std::mt19937 g(rd()); 

std::queue<PIECE*> removed_pieces;
std::vector<std::pair<int, int>> empty_spaces;
std::vector<std::pair<int, int>> moving_spaces;
std::vector<std::vector<PIECE*>> pieces;

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
int row = 0;
int col = 0;

bool is_moving = false;

int clicked_x = 0;
int clicked_y = 0;
bool clicked = false;

void set_pieces(int r, int c) {
	row = r;
	col = c;

	// Clear Containers
	while (!removed_pieces.empty()) {
		removed_pieces.pop();
	}

	empty_spaces.clear();
	moving_spaces.clear();

	for (auto& r : pieces) {
		r.clear();
	}
	pieces.clear();

	// Set Pieces
	for (int i = 0; i < row; ++i) {
		pieces.emplace_back();
	}

	for (int i = 0; i < col; ++i) {
		for (int j = 0; j < row; ++j) {
			pieces[j].emplace_back(new PIECE(i, j));
		}
	}

	std::shuffle(pieces.begin(), pieces.end(), g);
	for (int i = 0; i < row; ++i) {
		std::shuffle(pieces[i].begin(), pieces[i].end(), g);
	}

	for (int i = 0; i < col; ++i) {
		for (int j = 0; j < row; ++j) {
			pieces[j][i]->set_h(i, j);
		}
	}
}

void start_game() {
	if ((0 != row) && (0 != col)) {
		PIECE* piece = pieces[0][0];
		empty_spaces.emplace_back(std::make_pair(piece->m_hx, piece->m_hy));
		pieces[0][0] = nullptr;
		delete piece;
	}
}

bool hint = false;
bool reverse = false;
bool vh = false;
PIECE* picked = nullptr;

bool check_win() {
	int cnt = 0;
	for (int r = 0; r < row; ++r) {
		for (int c = 0; c < col; ++c) {
			if (!pieces[r][c]) {
				++cnt;
				continue;
			}

			if ((pieces[r][c]->m_hx != pieces[r][c]->m_mx) || (pieces[r][c]->m_hy != pieces[r][c]->m_my)) {
				++cnt;
			}
		}
	}

	if (!vh) {
		if (1 == cnt) {
			return true;
		}
	} else {
		if (0 == cnt) {
			return true;
		}
	}
	return false;
}

LRESULT WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	static BITMAP bmp;
	static HBITMAP hBitmap;
	static RECT rect;

	switch (iMessage) {
	case WM_CREATE: {
		hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetClientRect(hWnd, &rect);
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		break;
	}

	case WM_TIMER:
		switch (wParam) {
		case 0:
			for (auto iter = moving_spaces.begin(); iter != moving_spaces.end();) {
				if ((row - 1) == iter->second) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}
				
				if (nullptr == pieces[iter->second + 1][iter->first]) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}
				
				is_moving = true;

				pieces[iter->second + 1][iter->first]->m_dy -= ((rect.bottom / row) / 16);

				if (abs(pieces[iter->second + 1][iter->first]->m_dy) >= (rect.bottom / row)) {
					pieces[iter->second + 1][iter->first]->m_hy -= 1;
					pieces[iter->second + 1][iter->first]->m_dy = 0;
					pieces[iter->second][iter->first] = pieces[iter->second + 1][iter->first];
					pieces[iter->second + 1][iter->first] = nullptr;

					iter->second += 1;
					empty_spaces.emplace_back(*iter);

					iter = moving_spaces.erase(iter);
					continue;
				}
				++iter;
			}

			if (!moving_spaces.size()) {
				KillTimer(hWnd, 0);
				is_moving = false;

				if (check_win()) {
					InvalidateRect(hWnd, NULL, TRUE);
					MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
					PostQuitMessage(0);
					return 0;
				}
			}
			break;

		case 1:
			for (auto iter = moving_spaces.begin(); iter != moving_spaces.end();) {
				if (0 == iter->second) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}
				
				if (nullptr == pieces[iter->second - 1][iter->first]) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}

				is_moving = true;

				pieces[iter->second - 1][iter->first]->m_dy += ((rect.bottom / row) / 16);

				if (abs(pieces[iter->second - 1][iter->first]->m_dy) >= (rect.bottom / row)) {
					pieces[iter->second - 1][iter->first]->m_hy += 1;
					pieces[iter->second - 1][iter->first]->m_dy = 0;
					pieces[iter->second][iter->first] = pieces[iter->second - 1][iter->first];
					pieces[iter->second - 1][iter->first] = nullptr;
					
					iter->second -= 1;
					empty_spaces.emplace_back(*iter);

					iter = moving_spaces.erase(iter);
					continue;
				}
				++iter;
			}

			if (!moving_spaces.size()) {
				KillTimer(hWnd, 1);
				is_moving = false;

				if (check_win()) {
					InvalidateRect(hWnd, NULL, TRUE);
					MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
					PostQuitMessage(0);
					return 0;
				}
			}
			break;

		case 2:
			for (auto iter = moving_spaces.begin(); iter != moving_spaces.end();) {
				if ((col - 1) == iter->first) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}
				
				if (nullptr == pieces[iter->second][iter->first + 1]) { 
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}

				is_moving = true;

				pieces[iter->second][iter->first + 1]->m_dx -= ((rect.right / col) / 16);

				if (abs(pieces[iter->second][iter->first + 1]->m_dx) >= (rect.right / col)) {
					pieces[iter->second][iter->first + 1]->m_hx -= 1;
					pieces[iter->second][iter->first + 1]->m_dx = 0;
					pieces[iter->second][iter->first] = pieces[iter->second][iter->first + 1];
					pieces[iter->second][iter->first + 1] = nullptr;
					
					iter->first += 1;
					empty_spaces.emplace_back(*iter);

					iter = moving_spaces.erase(iter);
					continue;
				}
				++iter;
			}

			if (!moving_spaces.size()) {
				KillTimer(hWnd, 2);
				is_moving = false;

				if (check_win()) {
					InvalidateRect(hWnd, NULL, TRUE);
					MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
					PostQuitMessage(0);
					return 0;
				}
			}
			break;

		case 3:
			for (auto iter = moving_spaces.begin(); iter != moving_spaces.end();) {
				if (0 == iter->first) {
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}

				if (nullptr == pieces[iter->second][iter->first - 1]) {
					empty_spaces.emplace_back(*iter);
					iter = moving_spaces.erase(iter);
					continue; 
				}

				is_moving = true;

				pieces[iter->second][iter->first - 1]->m_dx += ((rect.right / col) / 16);

				if (abs(pieces[iter->second][iter->first - 1]->m_dx) >= (rect.right / col)) {
					pieces[iter->second][iter->first - 1]->m_hx += 1;
					pieces[iter->second][iter->first - 1]->m_dx = 0;
					pieces[iter->second][iter->first] = pieces[iter->second][iter->first - 1];
					pieces[iter->second][iter->first - 1] = nullptr;
					
					iter->first -= 1;
					empty_spaces.emplace_back(*iter);

					iter = moving_spaces.erase(iter);
					continue;
				}
				++iter;
			}

			if (!moving_spaces.size()) {
				KillTimer(hWnd, 3);
				is_moving = false;

				if (check_win()) {
					InvalidateRect(hWnd, NULL, TRUE);
					MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
					PostQuitMessage(0);
					return 0;
				}
			}
			break;

		case 99:
			hint = false;
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_PICTURE_1:
			hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("kpu.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(hBitmap, sizeof(BITMAP), &bmp);
			set_pieces(row, col);
			break;

		case ID_PICTURE_2:
			hBitmap = (HBITMAP)LoadImage(g_hInst, TEXT("tukorea.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(hBitmap, sizeof(BITMAP), &bmp);
			set_pieces(row, col);
			break;

		case ID_DIVIDE_3:
			vh = false;
			set_pieces(3, 3);
			break;

		case ID_DIVIDE_4:
			vh = false;
			set_pieces(4, 4);
			break;

		case ID_DIVIDE_5:
			vh = false;
			set_pieces(5, 5);
			break;

		case ID_GAME_START: {
			start_game();
			break;
		}

		case ID_GAME_HINT:
			hint = true;
			SetTimer(hWnd, 99, 1000, NULL);
			break;

		case ID_GAME_REVERSE:
			reverse = !reverse;
			break;

		case ID_GAME_QUIT:
			PostQuitMessage(0);
			return 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_CHAR:
		switch (wParam) {
		case 's':
			start_game();
			break;

		case 'f':
			hint = true;
			SetTimer(hWnd, 99, 1000, NULL);
			break;

		case 'v':
			vh = true;
			set_pieces(1, max(row, col));
			break;

		case 'h':
			vh = true;
			set_pieces(max(row, col), 1);
			break;

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
		if (!vh) {
			clicked = true;
			clicked_x = LOWORD(lParam);
			clicked_y = HIWORD(lParam);
		} else {
			clicked = true;
			clicked_x = LOWORD(lParam);
			clicked_y = HIWORD(lParam);

			int x = clicked_x / (rect.right / col);
			int y = clicked_y / (rect.bottom / row);
			
			if (pieces[y][x]) {
				empty_spaces.emplace_back(std::make_pair(x, y));
				picked = pieces[y][x];
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}

	case WM_LBUTTONUP:
		if (!vh) {
			clicked = false;
			clicked_x = LOWORD(lParam);
			clicked_y = HIWORD(lParam);
		} else {
			if (picked) {
				int new_x = clicked_x / (rect.right / col);
				int new_y = clicked_y / (rect.bottom / row);
				
				int old_x = empty_spaces.begin()->first;
				int old_y = empty_spaces.begin()->second;

				if ((new_x != old_x) || (new_y != old_y)) {
					if (pieces[new_y][new_x]) {
						PIECE piece(pieces[old_y][old_x]->m_mx, pieces[old_y][old_x]->m_my);

						pieces[old_y][old_x]->m_mx = pieces[new_y][new_x]->m_mx;
						pieces[old_y][old_x]->m_my = pieces[new_y][new_x]->m_my;

						picked->m_dx = 0; picked->m_dy = 0;
						pieces[new_y][new_x]->m_mx = piece.m_mx;
						pieces[new_y][new_x]->m_my = piece.m_my;

						empty_spaces.clear();
						picked = nullptr;
					}
				} else {
					picked->m_dx = 0; picked->m_dy = 0;

					empty_spaces.clear();
					picked = nullptr;
				}
			}

			if (check_win()) {
				InvalidateRect(hWnd, NULL, TRUE);
				MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
				PostQuitMessage(0);
				return 0;
			}

			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

	case WM_MOUSEMOVE: {
		if (!clicked || is_moving) { break; }

		if (!vh) {
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			if (-(HEIGHT / 2) > (y - clicked_y)) {
				for (auto& p : empty_spaces) {
					moving_spaces.emplace_back(p);
				}
				empty_spaces.clear();
				SetTimer(hWnd, 0, 16, NULL);
				break;
			}

			if ((HEIGHT / 2) < (y - clicked_y)) {
				for (auto& p : empty_spaces) {
					moving_spaces.emplace_back(p);
				}
				empty_spaces.clear();
				SetTimer(hWnd, 1, 16, NULL);
				break;
			}

			if (-(WIDTH / 2) > (x - clicked_x)) {
				for (auto& p : empty_spaces) {
					moving_spaces.emplace_back(p);
				}
				empty_spaces.clear();
				SetTimer(hWnd, 2, 16, NULL);
				break;
			}

			if ((WIDTH / 2) < (x - clicked_x)) {
				for (auto& p : empty_spaces) {
					moving_spaces.emplace_back(p);
				}
				empty_spaces.clear();
				SetTimer(hWnd, 3, 16, NULL);
				break;
			}
		} else {
			if (picked) {
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);

				picked->m_dx += x - clicked_x;
				picked->m_dy += y - clicked_y;

				clicked_x = x;
				clicked_y = y;

				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	}

	case WM_RBUTTONDOWN: {
		if (is_moving) { break; }
		
		int cx = LOWORD(lParam) / (rect.right / col);
		int cy = HIWORD(lParam) / (rect.bottom / row);

		if (pieces[cy][cx]) {
			removed_pieces.push(pieces[cy][cx]);
			empty_spaces.emplace_back(std::make_pair(cx, cy));
			pieces[cy][cx] = nullptr;
		} else {
			if (!removed_pieces.empty()) {
				PIECE* p = removed_pieces.front();
				p->m_hx = cx;
				p->m_hy = cy;
				pieces[cy][cx] = p;
				auto iter = std::find_if(empty_spaces.begin(), empty_spaces.end(), [&cx, &cy](const auto& p) {
					return (p.first == cx) && (p.second == cy);
					});
				empty_spaces.erase(iter);
				removed_pieces.pop();
			} else {
				MessageBox(hWnd, L"Can't Place!", L"Error", MB_ICONERROR);
			}
		}

		if (check_win()) {
			InvalidateRect(hWnd, NULL, TRUE);
			MessageBox(hWnd, L"Game Clear!", L"You Win!", MB_OK | MB_ICONINFORMATION);
			PostQuitMessage(0);
			return 0;
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		SelectObject(mDC, (HBITMAP)hBitmap);

		if (!hint) {
			if (!reverse) {
				for (const auto& r : pieces) {
					for (const auto& piece : r) {
						if (nullptr == piece) { continue; }
						piece->print(hDC, mDC, rect, bmp, row, col);
					}
				}
			} else {
				for (const auto& r : pieces) {
					for (const auto& piece : r) {
						if (nullptr == piece) { continue; }
						piece->print(hDC, mDC, rect, bmp, row, col, true);
					}
				}
			}
		} else {
			if (!reverse) {
				StretchBlt(hDC, 0, 0, rect.right, rect.bottom,
					mDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
			} else {
				StretchBlt(hDC, 0, 0, rect.right, rect.bottom,
					mDC, 0, 0, bmp.bmWidth, bmp.bmHeight, NOTSRCCOPY);
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
