#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <iostream>
#include "resource.h"

//#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")

HWND g_hWnd;
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Windows Programming";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////
// BOARD
constexpr int W_WIDTH = 1000;
constexpr int W_HEIGHT = 1000;

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
	WndClass.lpszMenuName = NULL;
	//WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	g_hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
		0, 0, W_WIDTH, W_HEIGHT,
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
	HBITMAP hBitmap;
	RECT rt;
	GetClientRect(hWnd, &rt);

	static int count = 0;
	static int dy = -1;

	switch (iMessage) {
	case WM_CREATE:
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
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dialog_Proc);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}

	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		SelectObject(mDC, (HBITMAP)hBitmap);

		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

BOOL Dialog_Proc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	static int index = 0;
	static wchar_t formula[100];

	switch (iMessage) {
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON1:
			formula[index++] = L'1';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON2:
			formula[index++] = L'2';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON3:
			formula[index++] = L'3';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON4:
			formula[index++] = L'4';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON5:
			formula[index++] = L'5';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON6:
			formula[index++] = L'6';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON7:
			formula[index++] = L'7';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON8:
			formula[index++] = L'8';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON9:
			formula[index++] = L'9';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON10:
			formula[index++] = L'0';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON11:
			formula[index++] = L'+';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON12:
			formula[index++] = L'-';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON13:
			formula[index++] = L'*';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON14:
			formula[index++] = L'/';
			formula[index] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON15: {
			int num1 = 0, num2 = 0;
			char op = 0;
			int i = 0;

			while (formula[i] >= '0' && formula[i] <= '9') {
				num1 = num1 * 10 + (formula[i] - '0');
				i++;
			}
			if (formula[i] == '.') {
				i++;
				while (formula[i] >= '0' && formula[i] <= '9') {
					i++;
				}
			}
			op = formula[i++];
			while (formula[i] >= '0' && formula[i] <= '9') {
				num2 = num2 * 10 + (formula[i] - '0');
				i++;
			}

			float result = 0;
			bool valid = true;

			switch (op) {
			case '+': result = num1 + num2; break;
			case '-': result = num1 - num2; break;
			case '*': result = num1 * num2; break;
			case '/':
				if (num2 != 0) result = num1 / (float)num2;
				else valid = false;
				break;
			default:
				valid = false;
				break;
			}

			if (valid) {
				wchar_t resultStr[100];
				if (op != '/') {
					swprintf_s(resultStr, 100, L"%.0f", result);
				} else {
					swprintf_s(resultStr, 100, L"%.2f", result);
				}

				SetDlgItemText(hDlg, IDC_EDIT1, resultStr);

				swprintf_s(formula, 100, L"%.0f", result);
				index = lstrlen(formula);
			} else {
				SetDlgItemTextA(hDlg, IDC_EDIT1, "Error");

				formula[0] = '\0';
				index = 0;
			}
			break;
		}

		case IDC_BUTTON16: {
			int num1 = 0, num2 = 0;
			wchar_t op = 0;
			int i = 0;

			while (iswdigit(formula[i])) {
				num1 = num1 * 10 + (formula[i++] - L'0');
			}
			op = formula[i++];
			while (iswdigit(formula[i])) {
				num2 = num2 * 10 + (formula[i++] - L'0');
			}

			auto reverse = [](int n) {
				int r = 0;
				while (n > 0) {
					r = r * 10 + (n % 10);
					n /= 10;
				}
				return r;
			};

			num1 = reverse(num1);
			num2 = reverse(num2);

			swprintf_s(formula, 100, L"%d%c%d", num1, op, num2);
			index = (int)wcslen(formula);
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;
		}

		case IDC_BUTTON17: {
			formula[index] = L'\0';

			int op_pos = -1;
			for (int i = 0; i < index; ++i) {
				if (formula[i] == L'+' || formula[i] == L'-' ||
					formula[i] == L'*' || formula[i] == L'/') {
					op_pos = i;
					break;
				}
			}

			if (op_pos != -1) {
				formula[op_pos] = L'\0';
				index = op_pos;
			} else {
				formula[0] = L'\0';
				index = 0;
			}
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;
		}

		case IDC_BUTTON18:
			index = 0;
			formula[0] = L'\0';
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;

		case IDC_BUTTON19: {
			formula[index] = L'\0';
			int num = _wtoi(formula);

			wchar_t binary[100] = L"";
			int i = 0;
			if (num == 0) {
				wcscpy_s(binary, L"0");
			}
			else {
				while (num > 0) {
					binary[i++] = (num % 2) + L'0';
					num /= 2;
				}
				binary[i] = L'\0';
				_wcsrev(binary);
			}

			wchar_t resultStr[100];
			wcscpy_s(resultStr, binary);

			SetDlgItemText(hDlg, IDC_EDIT1, resultStr);
			break;
		}

		case IDC_BUTTON20: {
			formula[index] = L'\0';
			double num = _wtof(formula);
			wchar_t resultStr[100];

			if (num != 0.0) {
				double result = 1.0 / num;
				swprintf_s(resultStr, 100, L"%.2f", result);
				swprintf_s(formula, 100, L"%.0f", result);
			} else {
				wcscpy_s(formula, L"Error");
			}
			
			index = (int)wcslen(formula);
			SetDlgItemText(hDlg, IDC_EDIT1, resultStr);
			break;
		}

		case IDC_BUTTON21: {
			formula[index] = L'\0';
			int num = _wtoi(formula);
			num *= 10;
			
			swprintf_s(formula, 100, L"%d", num);
			
			index = (int)wcslen(formula);
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;
		}

		case IDC_BUTTON22:
			if (index > 0) {
				index--;
				formula[index] = L'\0';
				SetDlgItemText(hDlg, IDC_EDIT1, formula);
			}
			break;

		case IDC_BUTTON23: {
			formula[index] = L'\0';
			int exp = _wtoi(formula);
			double result = pow(10, exp);

			swprintf_s(formula, 100, L"%.0f", result);

			index = (int)wcslen(formula);
			SetDlgItemText(hDlg, IDC_EDIT1, formula);
			break;
		}

		case IDC_BUTTON24:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}
