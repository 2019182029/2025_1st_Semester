#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include "resource.h"

#pragma comment (lib, "msimg32.lib")
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int WIDTH = 700;
constexpr int HEIGHT = 700;
constexpr int ROW = 6;
constexpr int COL = 6;
constexpr int OFFSET = 100;

int OBJECT_INDEX = 4;
int OBSTACLE_COUNT = 2;

HWND g_hWnd;

HBITMAP Bit[6];
BITMAP Bmp[6];

bool game_start = false;
bool game_clear = false;
int slide_count = 0;

class BLOCK {
public:
	int m_x, m_y;
	int m_index;
	bool m_is_obstacle;
	bool m_is_moving;
	bool m_is_upgraded;

public:
	BLOCK() {
		m_x = 0;
		m_y = 0;
		m_index = 0;
		m_is_obstacle = false;
		m_is_moving = false;
		m_is_upgraded = false;
	}

	void move(int dx, int dy) {

	}

	void print(HDC hDC) {
		if (!m_is_obstacle) {
			HDC mDC = CreateCompatibleDC(hDC);
			HGDIOBJ hBitmap = SelectObject(mDC, Bit[m_index]);

			StretchBlt(hDC, m_x, m_y, Bmp[m_index].bmWidth, Bmp[m_index].bmHeight,
				mDC, 0, 0, Bmp[m_index].bmWidth, Bmp[m_index].bmHeight, SRCCOPY);

			SelectObject(mDC, hBitmap);
			DeleteDC(mDC);
		} 
		else {
			HBRUSH MyBrush = CreateSolidBrush(RGB(0, 0, 200));
			HBRUSH OldBrush = (HBRUSH)SelectObject(hDC, MyBrush);

			Rectangle(hDC, 
				m_x, m_y, 
				m_x + Bmp[0].bmWidth, m_y + Bmp[0].bmHeight);

			SelectObject(hDC, OldBrush);
			DeleteObject(MyBrush);
		}
	}
};

class BOARD {
public:
	std::array<std::array<BLOCK*, 6>, 6> m_blocks;

public:
	BOARD() {
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				m_blocks[r][c] = nullptr;
			}
		}

		for (int i = 0; i < OBSTACLE_COUNT;) {
			int r = (rand() % 6);
			int c = (rand() % 6);

			if (!m_blocks[r][c]) {
				BLOCK* block = new BLOCK();
				block->m_x = (c * OFFSET);
				block->m_y = (r * OFFSET);
				block->m_index = -1;
				block->m_is_obstacle = true;
				m_blocks[r][c] = block;
				++i;
			}
		}

		for (int i = 0; i < 2;) {
			int r = (rand() % 6);
			int c = (rand() % 6);

			if (!m_blocks[r][c]) {
				BLOCK* block = new BLOCK();
				block->m_x = (c * OFFSET);
				block->m_y = (r * OFFSET);
				m_blocks[r][c] = block;
				++i;
			}
		}
	}
	~BOARD() {}

	void slide(WPARAM wParam) {
		slide_count = 0;

		switch (wParam) {
		case VK_UP: {
			int count = 0;

			for (int c = 0; c < COL; ++c) {
				for (int r = 1; r < ROW; ++r) {
					if (!m_blocks[r][c]) { continue; }
					if (m_blocks[r][c]->m_is_obstacle) { continue; }

					if (!m_blocks[r - 1][c] ||
						m_blocks[r - 1][c]->m_is_moving ||
						((!m_blocks[r - 1][c]->m_is_upgraded && !m_blocks[r][c]->m_is_upgraded) && (m_blocks[r - 1][c]->m_index == m_blocks[r][c]->m_index))) {
						m_blocks[r][c]->m_is_moving = true;
						++count;
					}
				}
			}

			if (count) {
				SetTimer(g_hWnd, 0, 16, NULL);
			}
			else {
				int empty = std::count_if(m_blocks.begin(), m_blocks.end(), [](const auto& row) {
					return std::any_of(row.begin(), row.end(), [](const auto& block) {
						return block != nullptr;
						});
					});

				if (0 == empty) {
					game_start = false;
					InvalidateRect(g_hWnd, NULL, FALSE);
					MessageBox(g_hWnd, L"Game Over!", L"Game Over!", MB_ICONERROR);
					return;
				}

				while (true) {
					int r = (rand() % 6);
					int c = (rand() % 6);

					if (!m_blocks[r][c]) {
						BLOCK* block = new BLOCK();
						block->m_x = (c * OFFSET);
						block->m_y = (r * OFFSET);
						m_blocks[r][c] = block;
						break;
					}
				}
			}
			break;
		}

		case VK_DOWN: {
			int count = 0;

			for (int c = 0; c < COL; ++c) {
				for (int r = ROW - 2; r >= 0; --r) {
					if (!m_blocks[r][c]) { continue; }
					if (m_blocks[r][c]->m_is_obstacle) { continue; }

					if (!m_blocks[r + 1][c] ||
						m_blocks[r + 1][c]->m_is_moving ||
						((!m_blocks[r + 1][c]->m_is_upgraded && !m_blocks[r][c]->m_is_upgraded) && (m_blocks[r + 1][c]->m_index == m_blocks[r][c]->m_index))) {
						m_blocks[r][c]->m_is_moving = true;
						++count;
					}
				}
			}

			if (count) {
				SetTimer(g_hWnd, 1, 16, NULL);
			}
			else {
				int empty = std::count_if(m_blocks.begin(), m_blocks.end(), [](const auto& row) {
					return std::any_of(row.begin(), row.end(), [](const auto& block) {
						return block != nullptr;
						});
					});

				if (0 == empty) {
					game_start = false;
					InvalidateRect(g_hWnd, NULL, FALSE);
					MessageBox(g_hWnd, L"Game Over!", L"Game Over!", MB_ICONERROR);
					return;
				}

				while (true) {
					int r = (rand() % 6);
					int c = (rand() % 6);

					if (!m_blocks[r][c]) {
						BLOCK* block = new BLOCK();
						block->m_x = (c * OFFSET);
						block->m_y = (r * OFFSET);
						m_blocks[r][c] = block;
						break;
					}
				}
			}
			break;
		}

		case VK_LEFT: {
			int count = 0;

			for (int r = 0; r < ROW; ++r) {
				for (int c = 1; c < COL; ++c) {
					if (!m_blocks[r][c]) { continue; }
					if (m_blocks[r][c]->m_is_obstacle) { continue; }

					if (!m_blocks[r][c - 1] ||
						m_blocks[r][c - 1]->m_is_moving ||
						((!m_blocks[r][c - 1]->m_is_upgraded && !m_blocks[r][c]->m_is_upgraded) && (m_blocks[r][c - 1]->m_index == m_blocks[r][c]->m_index))) {
						m_blocks[r][c]->m_is_moving = true;
						++count;
					}
				}
			}

			if (count) {
				SetTimer(g_hWnd, 2, 16, NULL);
			}
			else {
				int empty = std::count_if(m_blocks.begin(), m_blocks.end(), [](const auto& row) {
					return std::any_of(row.begin(), row.end(), [](const auto& block) {
						return block != nullptr;
						});
					});

				if (0 == empty) {
					game_start = false;
					InvalidateRect(g_hWnd, NULL, FALSE);
					MessageBox(g_hWnd, L"Game Over!", L"Game Over!", MB_ICONERROR);
					return;
				}

				while (true) {
					int r = (rand() % 6);
					int c = (rand() % 6);

					if (!m_blocks[r][c]) {
						BLOCK* block = new BLOCK();
						block->m_x = (c * OFFSET);
						block->m_y = (r * OFFSET);
						m_blocks[r][c] = block;
						break;
					}
				}
			}
			break;
		}

		case VK_RIGHT: {
			int count = 0;

			for (int r = 0; r < ROW; ++r) {
				for (int c = COL - 2; c >= 0; --c) {
					if (!m_blocks[r][c]) { continue; }
					if (m_blocks[r][c]->m_is_obstacle) { continue; }

					if (!m_blocks[r][c + 1] ||
						m_blocks[r][c + 1]->m_is_moving ||
						((!m_blocks[r][c + 1]->m_is_upgraded && !m_blocks[r][c]->m_is_upgraded) && (m_blocks[r][c + 1]->m_index == m_blocks[r][c]->m_index))) {
						m_blocks[r][c]->m_is_moving = true;
						++count;
					}
				}
			}

			if (count) {
				SetTimer(g_hWnd, 3, 16, NULL);
			}
			else {
				int empty = std::count_if(m_blocks.begin(), m_blocks.end(), [](const auto& row) {
					return std::any_of(row.begin(), row.end(), [](const auto& block) {
						return block != nullptr;
						});
					});

				if (0 == empty) {
					game_start = false;
					InvalidateRect(g_hWnd, NULL, FALSE);
					MessageBox(g_hWnd, L"Game Over!", L"Game Over!", MB_ICONERROR);
					return;
				}

				while (true) {
					int r = (rand() % 6);
					int c = (rand() % 6);

					if (!m_blocks[r][c]) {
						BLOCK* block = new BLOCK();
						block->m_x = (c * OFFSET);
						block->m_y = (r * OFFSET);
						m_blocks[r][c] = block;
						break;
					}
				}
			}
			break;
		}
		}
	}

	void print(HDC hDC) {
		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				Rectangle(hDC,
					c * OFFSET, r * OFFSET,
					(c + 1) * OFFSET, (r + 1) * OFFSET);
			}
		}


		for (int r = 0; r < ROW; ++r) {
			for (int c = 0; c < COL; ++c) {
				if (m_blocks[r][c]) {
					m_blocks[r][c]->print(hDC);
				}
			}
		}
	}
};

BOARD* board = new BOARD();

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
	static RECT rect;

	switch (iMessage) {
	case WM_CREATE: {
		int count = 0;
		TCHAR file[100];
		for (int i = 2; i <= 64; i = i * 2) {
			swprintf(file, 50, TEXT("Resource\\%d.bmp"), i);
			Bit[count] = (HBITMAP)LoadImage(g_hInst, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			GetObject(Bit[count], sizeof(BITMAP), &Bmp[count]);
			++count;
		}
		GetClientRect(hWnd, &rect);
		break;
	}

	case WM_TIMER: {
		switch (wParam) {
		case 0:
			for (auto& row : board->m_blocks) {
				for (auto& block : row) {
					if (block && block->m_is_moving) {
						block->m_y -= 10;
					}
				}
			}
			++slide_count;

			if (10 == slide_count) {
				KillTimer(g_hWnd, 0);

				for (int c = 0; c < COL; ++c) {
					for (int r = 1; r < ROW; ++r) {
						if (board->m_blocks[r][c] && board->m_blocks[r][c]->m_is_moving) {
							board->m_blocks[r][c]->m_is_moving = false;
							if (board->m_blocks[r - 1][c] && (false == board->m_blocks[r - 1][c]->m_is_moving)) {
								delete board->m_blocks[r - 1][c];
								if (OBJECT_INDEX == ++board->m_blocks[r][c]->m_index) {
									game_start = false;
									game_clear = true;
								}
								board->m_blocks[r][c]->m_is_upgraded = true;
							}
							board->m_blocks[r - 1][c] = board->m_blocks[r][c];
							board->m_blocks[r][c] = nullptr;
						}
					}
				}

				board->slide(VK_UP);
			}
			break;

		case 1:
			for (auto& row : board->m_blocks) {
				for (auto& block : row) {
					if (block && block->m_is_moving) {
						block->m_y += 10;
					}
				}
			}
			++slide_count;

			if (10 == slide_count) {
				KillTimer(g_hWnd, 1);

				for (int c = 0; c < COL; ++c) {
					for (int r = ROW - 2; r >= 0; --r) {
						if (board->m_blocks[r][c] && board->m_blocks[r][c]->m_is_moving) {
							board->m_blocks[r][c]->m_is_moving = false;
							if (board->m_blocks[r + 1][c] && (false == board->m_blocks[r + 1][c]->m_is_moving)) {
								delete board->m_blocks[r + 1][c];
								if (OBJECT_INDEX == ++board->m_blocks[r][c]->m_index) {
									game_start = false;
									game_clear = true;
								}
								board->m_blocks[r][c]->m_is_upgraded = true;
							}
							board->m_blocks[r + 1][c] = board->m_blocks[r][c];
							board->m_blocks[r][c] = nullptr;
						}
					}
				}

				board->slide(VK_DOWN);
			}
			break;

		case 2:
			for (auto& row : board->m_blocks) {
				for (auto& block : row) {
					if (block && block->m_is_moving) {
						block->m_x -= 10;
					}
				}
			}
			++slide_count;

			if (10 == slide_count) {
				KillTimer(g_hWnd, 2);

				for (int r = 0; r < ROW; ++r) {
					for (int c = 1; c < COL; ++c) {
						if (board->m_blocks[r][c] && board->m_blocks[r][c]->m_is_moving) {
							board->m_blocks[r][c]->m_is_moving = false;
							if (board->m_blocks[r][c - 1] && (false == board->m_blocks[r][c - 1]->m_is_moving)) {
								delete board->m_blocks[r][c - 1];
								if (OBJECT_INDEX == ++board->m_blocks[r][c]->m_index) {
									game_start = false;
									game_clear = true;
								}
								board->m_blocks[r][c]->m_is_upgraded = true;
							}
							board->m_blocks[r][c - 1] = board->m_blocks[r][c];
							board->m_blocks[r][c] = nullptr;
						}
					}
				}

				board->slide(VK_LEFT);
			}
			break;

		case 3:
			for (auto& row : board->m_blocks) {
				for (auto& block : row) {
					if (block && block->m_is_moving) {
						block->m_x += 10;
					}
				}
			}
			++slide_count;

			if (10 == slide_count) {
				KillTimer(g_hWnd, 3);

				for (int r = 0; r < ROW; ++r) {
					for (int c = COL - 2; c >= 0; --c) {
						if (board->m_blocks[r][c] && board->m_blocks[r][c]->m_is_moving) {
							board->m_blocks[r][c]->m_is_moving = false;
							if (board->m_blocks[r][c + 1] && (false == board->m_blocks[r][c + 1]->m_is_moving)) {
								delete board->m_blocks[r][c + 1];
								if (OBJECT_INDEX == ++board->m_blocks[r][c]->m_index) {
									game_start = false;
									game_clear = true;
								}
								board->m_blocks[r][c]->m_is_upgraded = true;
							}
							board->m_blocks[r][c + 1] = board->m_blocks[r][c];
							board->m_blocks[r][c] = nullptr;
						}
					}
				}

				board->slide(VK_RIGHT);
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		if (game_clear) {
			game_clear = false;
			MessageBox(g_hWnd, L"You Win!", L"Game Clear!", MB_ICONINFORMATION);
		}
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_GAME_START: {
			game_start = true;
			game_clear = false;
			BOARD* old_board = board;
			board = new BOARD();
			delete old_board;
			break;
		}

		case ID_GAME_QUIT:
			PostQuitMessage(0);
			return 0;

		case ID_SCORE_32:
			OBJECT_INDEX = 4;
			break;

		case ID_SCORE_64:
			OBJECT_INDEX = 5;
			break;

		case ID_OBSTACLE_2:
			OBSTACLE_COUNT = 2;
			break;

		case ID_OBSTACLE_3:
			OBSTACLE_COUNT = 3;
			break;

		case ID_OBSTACLE_4:
			OBSTACLE_COUNT = 4;
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CHAR:
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		if (!game_start) { break; }

		switch (wParam) {
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			for (auto& row : board->m_blocks) {
				for (auto& block : row) {
					if (block) {
						block->m_is_upgraded = false;
					}
				}
			}

			board->slide(wParam);
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_LBUTTONUP: {
		break;
	}

	case WM_MOUSEMOVE:
		break;

	case WM_RBUTTONDOWN: {
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_RBUTTONUP: {
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);

		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HGDIOBJ old_hBitmap = SelectObject(mDC, hBitmap);

		board->print(mDC);

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
