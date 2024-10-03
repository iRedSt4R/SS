#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <string>
#include <tchar.h> // For _tprintf

enum class ERunMode
{
	Fullscreen,
	Settings,
	PreviewWindow,
	Unknown
};

// globals
int windowWidth = 0;
int windowHeight = 0;

RECT rectPosition;
int rectSpeedX = 5;
int rectSpeedY = 5;
int rectWidth = 50;
int rectHeight = 50;
static constexpr COLORREF rectColor = RGB(0, 0, 255);

void SaveSettings(int value) {
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\GeneralArcadeSS"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		RegSetValueEx(hKey, TEXT("RectSpeedValue"), 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
		RegCloseKey(hKey);
	}
}

int LoadSettings() {
	HKEY hKey;
	DWORD value = 5; // Default value
	DWORD dataSize = sizeof(value);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\GeneralArcadeSS"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, TEXT("RectSpeedValue"), NULL, NULL, (LPBYTE)&value, &dataSize);
		RegCloseKey(hKey);
	}
	return (int)value;
}

// settings window proc
LRESULT CALLBACK ConfigWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	static HWND hEdit;
	static HWND hButtonOK, hButtonCancel;

	switch (msg) {
	case WM_CREATE: {
		// Create a static text label
		CreateWindow(
			TEXT("STATIC"),
			TEXT("Rect speed:"),
			WS_VISIBLE | WS_CHILD,
			10, 10, 100, 20,
			hwnd, NULL, NULL, NULL);

		// Create an edit control for user input
		hEdit = CreateWindow(
			TEXT("EDIT"),
			NULL,
			WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
			120, 10, 150, 20,
			hwnd, (HMENU)1, NULL, NULL);

		// Load saved value into the edit control
		float savedValue = rectSpeedX;
		TCHAR buffer[32];
		_stprintf_s(buffer, TEXT("%.2f"), savedValue);
		SetWindowText(hEdit, buffer);

		// Create OK and Cancel buttons
		hButtonOK = CreateWindow(
			TEXT("BUTTON"),
			TEXT("OK"),
			WS_VISIBLE | WS_CHILD,
			70, 50, 70, 25,
			hwnd, (HMENU)IDOK, NULL, NULL);

		hButtonCancel = CreateWindow(
			TEXT("BUTTON"),
			TEXT("Cancel"),
			WS_VISIBLE | WS_CHILD,
			160, 50, 70, 25,
			hwnd, (HMENU)IDCANCEL, NULL, NULL);
		break;
	}
#if 1
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: {
			// Get the value from the edit control
			TCHAR buffer[32];
			GetWindowText(hEdit, buffer, 32);
			float value = _tstof(buffer);
			// Save the value
			SaveSettings(value);
			break;
		}
		case IDCANCEL:
			break;
		}
		break;
#endif
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// Function to update the position of the rectangle and bounce off screen edges
void UpdateRectanglePosition(int screenWidth, int screenHeight)
{
	// Move the rectangle
	rectPosition.left += rectSpeedX;
	rectPosition.right += rectSpeedX;
	rectPosition.top += rectSpeedY;
	rectPosition.bottom += rectSpeedY;

	// hirizontal borders reverse
	if (rectPosition.left <= 0 || rectPosition.right >= screenWidth)
	{
		rectSpeedX = -rectSpeedX;
	}

	// vertical borders reverse
	if (rectPosition.top <= 0 || rectPosition.bottom >= screenHeight)
	{
		rectSpeedY = -rectSpeedY;
	}
}

// color previous rectangle position
void ClearPreviousRectangle(HDC hdc)
{
	int red = rand() % 256;
	int green = rand() % 256;
	int blue = rand() % 256;
	HBRUSH hBrush = CreateSolidBrush(RGB(red, green, blue));
	FillRect(hdc, &rectPosition, hBrush);
	DeleteObject(hBrush);
}

// rect draw
void DrawRectangle(HDC hdc)
{
	HBRUSH hBrush = CreateSolidBrush(rectColor);
	FillRect(hdc, &rectPosition, hBrush); // Fill the rectangle with color
	DeleteObject(hBrush);
}

/// fullscreen window proc
LRESULT WINAPI wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_MOUSEMOVE: 
	{
		static bool mouseInitialized = false;
		static int startX;
		static int startY;

		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		if (!mouseInitialized)
		{
			startX = x;
			startY = y;

			mouseInitialized = true;
		}
		else if (startX != x && startY != y) 
		{ 
			::PostQuitMessage(0);
		}
	}break;
	case WM_KEYDOWN: {
		::PostQuitMessage(0); /// exit when user press any key
	}break;
	case WM_DESTROY: { /// standard exiting from winapi window
		::PostQuitMessage(0);
		return 0;
	}
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


/// starting point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	rectSpeedX = LoadSettings();
	rectSpeedY = LoadSettings();

	HWND hwnd = NULL;
	MSG msg = {};
	WNDCLASSEX wc = {};
	const char* windowClassName = "Tomasz Kalina SS";
	ERunMode ss_run_mode = ERunMode::Unknown;
	{
		/// read command line
		std::string s = std::string(lpCmdLine).substr(0, 2);
		if (s == "\\c" || s == "\\C" || s == "/c" || s == "/C" || s == "") // settings
		{
			ss_run_mode = ERunMode::Settings;
		}
		else if (s == "\\s" || s == "\\S" || s == "/s" || s == "/S") // fullscreen
		{
			ss_run_mode = ERunMode::Fullscreen;
		}
		else if (s == "\\p" || s == "\\P" || s == "/p" || s == "/P") // preview window /p then handle to preview window
		{
			ss_run_mode = ERunMode::PreviewWindow;
			hwnd = (HWND)atoi(lpCmdLine + 3);
		}

	}

	// prevent multiple windows
	if (ss_run_mode != ERunMode::PreviewWindow  && FindWindow(windowClassName, NULL)) 
	{
		return -1;
	}


	// windows class
	if (ss_run_mode == ERunMode::Fullscreen) 
	{
		wc = { sizeof(WNDCLASSEX), CS_CLASSDC, wndProc, 0, 0, hInstance, NULL, NULL, NULL, NULL, windowClassName, NULL };
		::RegisterClassEx(&wc);
	}
	else if (ss_run_mode == ERunMode::Settings)
	{
		wc = { sizeof(WNDCLASSEX), CS_CLASSDC, ConfigWndProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_CROSS), (HBRUSH)(COLOR_WINDOW + 1), NULL, windowClassName, NULL };
		::RegisterClassEx(&wc);
	}

	/// create window or read size
	if (ss_run_mode == ERunMode::Fullscreen) 
	{
		windowWidth = GetSystemMetrics(SM_CXSCREEN);
		windowHeight = GetSystemMetrics(SM_CYSCREEN);
		hwnd = ::CreateWindow(wc.lpszClassName, "Tomasz Kalina SS", WS_POPUP | WS_VISIBLE, 0, 0, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);
	}
	else if (ss_run_mode == ERunMode::Settings)
	{
		hwnd = ::CreateWindow(wc.lpszClassName, "Tomasz Kalina SS", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 350, 150, NULL, NULL, wc.hInstance, NULL);
	}
	else 
	{
		RECT rc; GetWindowRect(hwnd, &rc);
		windowWidth = rc.right - rc.left;
		windowHeight = rc.bottom - rc.top;
	}

	/// show window and hide cursor
	if (ss_run_mode != ERunMode::PreviewWindow) 
	{
		::ShowWindow(hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd);
		if (ss_run_mode == ERunMode::Settings)
		{
			::ShowCursor(true);
		}
	}

	// prepare rect object
	rectWidth = windowWidth / 10;
	rectHeight = rectWidth;
	rectPosition.left = 50;
	rectPosition.top = 50;
	rectPosition.right = rectPosition.left + rectWidth;
	rectPosition.bottom = rectPosition.top + rectHeight;
	
	/// main loop
	while (msg.message != WM_QUIT) 
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) 
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		if (ss_run_mode != ERunMode::Settings)
		{
			HDC hdc = GetDC(hwnd);
			if (hdc == NULL)
			{
				return (INT)msg.wParam;
			}

			// Clear the previous rectangle
			ClearPreviousRectangle(hdc);

			// Update the rectangle's position
			UpdateRectanglePosition(windowWidth, windowHeight);

			// Draw the updated rectangle
			DrawRectangle(hdc);

			// Sleep for 16 milliseconds (~60 frames per second)
			Sleep(16);

			ReleaseDC(hwnd, hdc);
		}
		
	}

	// do some cleanup
	if (ss_run_mode != ERunMode::PreviewWindow) {
		::ShowCursor(true);
		::DestroyWindow(hwnd);
		::UnregisterClass(windowClassName, hInstance);
	}
	return (INT)msg.wParam;
}