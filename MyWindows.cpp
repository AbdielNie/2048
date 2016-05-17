#include "MyDirectX.h"

using namespace std;
bool gameover = false;

LRESULT WINAPI WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		gameover = true;
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	{
		HWND hwnd;
		if ((hwnd = FindWindow("MainWindow2048Class", "2048 for DirectX")) != NULL)
		{
			MessageBox(NULL, "你已经运行了一个该程序的实例。", "提示", MB_OK);
			SetForegroundWindow(hwnd);
			return FALSE;
		}
	}

	WNDCLASSEX wc;
	MSG msg;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(101));;
	wc.lpszMenuName = NULL;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = "MainWindow2048Class";
	if (!RegisterClassEx(&wc))
		return FALSE;
	HWND hwnd = CreateWindow("MainWindow2048Class", APPTITLE.c_str(), WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, SCREENW, SCREENH, NULL, NULL, hInstance, NULL);
	if (hwnd == 0)
		return 0;
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	if (!Game_Init(hwnd)) return 0;

	while (!gameover)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Game_Run(hwnd);
	}
	Game_End();
	return msg.wParam;
}
