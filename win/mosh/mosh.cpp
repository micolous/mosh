// mosh.cpp : Defines the entry point for the application.
//

#include "mosh.h"
#include "stmclient.h"
#include "xterm_colorcodes.h"
#include <sstream>
#include <string>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HFONT mainFont;
HFONT underlineFont;
SIZE fontSize;

HBITMAP bmpDoubleBuffer;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#include <iostream>

STMClient* stm;

class TerminalImpl : public ITerminal
{
public:
	Terminal::Framebuffer* fb;
	TerminalImpl(STMClient* stm)
	{
		fb = NULL;
		window_size.ws_row = 25;
		window_size.ws_col = 80;
	}

	void SetHWND(HWND hWnd)
	{
		this->hWnd = hWnd;
	}

	virtual winsize GetSize()
	{
		return window_size;
	}
	virtual void SetSize(const winsize& newsize)
	{
		window_size = newsize;
	}

	virtual void Clear()
	{
	}

	virtual void Update(Terminal::Framebuffer* fb)
	{
		SetWindowText(hWnd, std::wstring(fb->get_window_title().begin(), fb->get_window_title().end()).c_str());
		if (fb->ds.get_width() != window_size.ws_col || fb->ds.get_height() != window_size.ws_row)
		{
			SetWindowPos(hWnd, NULL, 0, 0, fontSize.cx * fb->ds.get_width(), fontSize.cy * fb->ds.get_height(), SWP_NOMOVE);
			window_size.ws_col = fb->ds.get_width();
			window_size.ws_row = fb->ds.get_height();
		}
		this->fb = fb;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	virtual void Shutdown()
	{
		PostQuitMessage(0);
	}

private:
	HWND hWnd;
	winsize window_size;
}*terminalImpl;

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
		{
			return 1;
		}
	}

	mainFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, (L"Lucida Console"));
	underlineFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, (L"Lucida Console"));

	char env_key[1024];
	DWORD ret = GetEnvironmentVariableA("MOSH_KEY", env_key, 1024);
	if (ret == 0)
		return -1;
	std::string key = env_key;
	_putenv("MOSH_KEY=");
	std::wistringstream is(lpCmdLine);
	std::wstring host;
	int port;
	is >> host >> port;
	if (!is)
		return -1;
	char ahost[1024];
	int ahostlen = WideCharToMultiByte(CP_ACP, 0, host.c_str(), host.size(), ahost, 1024, 0, NULL);
	ahost[ahostlen] = 0;

	stm = new STMClient(ahost, port, key.c_str(), NULL);

	terminalImpl = new TerminalImpl(stm);
	stm->init(terminalImpl);
	stm->main();

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOSH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOSH));
	bool done = false;
	// Main message loop:
	while (!done)
	{
		//GetMessage(&msg, NULL, 0, 0)
		while (::PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			BOOL ret = GetMessage(&msg, NULL, 0, 0);
			if (!ret)
			{
				done = true;
				break;
			}
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (!done)
		{
			//std::wostringstream os;
			//os << GetTickCount() << " oneloop "  << std::endl;
			//OutputDebugString(os.str().c_str());
			stm->oneloop();
		}
	}
	
	DeleteObject(mainFont);
	DeleteObject(underlineFont);
	WSACleanup();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOSH));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	HBRUSH brush;
	brush = CreateSolidBrush(0);
	wcex.hbrBackground	= brush;//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MOSH);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   HDC dc = GetDC(GetDesktopWindow());
   SelectObject(dc, mainFont);
   SetMapMode(dc, MM_TEXT);
   SIZE sz;
   //for(int i = 0; i < 26;i++)
   {
	   std::wstring x = L"A";
	   //x[0] += i;
	   GetTextExtentPoint32(dc, x.c_str(), 1, &sz);
	   std::wostringstream os;
	   os << x << ' ' << sz.cx << ' ' <<sz.cy <<std::endl;
	   OutputDebugStringW(os.str().c_str());
	   fontSize = sz;
   }
   ReleaseDC(GetDesktopWindow(), dc);
   
   RECT rect;
   rect.top = 0;
   rect.bottom = 25*sz.cy;
   rect.left = 0;
   rect.right = 80*sz.cx;

   AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   {
	   HDC dc = GetDC(hWnd);
	   HDC dcBuf = CreateCompatibleDC(dc);
	   bmpDoubleBuffer = CreateCompatibleBitmap(dc, sz.cx*80, sz.cy*25);
	   HGDIOBJ old = SelectObject(dcBuf, bmpDoubleBuffer);
	   BitBlt(dcBuf, 0, 0, sz.cx*80, sz.cy*25, NULL, 0, 0, BLACKNESS );
	   SelectObject(dcBuf, old);
	   DeleteDC(dcBuf);
	   ReleaseDC(hWnd, dc);
   }

   HMENU systemMenu = GetSystemMenu(hWnd, FALSE);
   AppendMenu(systemMenu, MF_MENUBARBREAK, 0, 0);
   AppendMenu(systemMenu, 0, IDM_SET_FONT, L"Set Font...");

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   SetTimer(hWnd, 1, 100, NULL);

   terminalImpl->SetHWND(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
//		switch (wmId)
//		{
//		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
//		}
		break;
	case WM_TIMER:
		{
			//std::wostringstream os;
			//os << GetTickCount() << " oneloop "  << std::endl;
			//OutputDebugString(os.str().c_str());
			stm->oneloop();
		}
		break;
	case WM_SYSCHAR:
		{
			int charCode = wParam;
			std::wostringstream os;
			os << L"WM_SYSCHAR" << charCode << ' ' <<lParam << std::endl;
			OutputDebugString(os.str().c_str());
			if (lParam & (0x20000000))
			{
				// alt key is pressed
				stm->process_user_input(27);
				stm->process_user_input(charCode);
			}
		}
		break;
	case WM_CHAR:
		{
			int charCode = wParam;
			std::wostringstream os;
			os << GetTickCount() << " WM_CHAR " << wParam << std::endl;
			OutputDebugString(os.str().c_str());
			stm->process_user_input(charCode);
			stm->oneloop();
		}
		break;
	case WM_PAINT:
		{
			DWORD startTime=GetTickCount();
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rect;
			SetMapMode(hdc, MM_TEXT);
			SetTextColor(hdc, 0x808080);
			SetBkMode(hdc, TRANSPARENT);
			GetWindowRect(hWnd, &rect);
			HGDIOBJ oldFont = SelectObject(hdc, mainFont);
			SIZE sz;
			GetTextExtentPoint32(hdc, L"W", 1, &sz);
			fontSize = sz;
			if (terminalImpl->fb)
			{
				HDC dcBuf = CreateCompatibleDC(hdc);
				HGDIOBJ old = SelectObject(dcBuf, bmpDoubleBuffer);
				HGDIOBJ oldFont2 = SelectObject(dcBuf, mainFont);
				BitBlt(dcBuf, 0, 0, sz.cx*80, sz.cy*25, NULL, 0, 0, BLACKNESS );
				int cursorX = terminalImpl->fb->ds.get_cursor_col();
				int cursorY = terminalImpl->fb->ds.get_cursor_row();
				for(int y = 0; y < terminalImpl->GetSize().ws_row; y++)
					for(int x = 0; x < terminalImpl->GetSize().ws_col; )
					{
						const Terminal::Cell* cell = terminalImpl->fb->get_cell(y, x);

						std::wstring data(cell->contents.begin(), cell->contents.end());
						std::wostringstream os;
						//cell->renditions.foreground_color;

						if (cell->renditions.underlined)
						{
							SelectObject(dcBuf, underlineFont);
						}

						int textColor = cell->renditions.foreground_color;
						if (textColor == 0)
							textColor = 37;
						textColor = (textColor-30+256)%256;
						if (cell->renditions.bold)
						{
							if (textColor < 8)
								textColor += 8;
						}

						SetTextColor(dcBuf, get_xterm_colorref(textColor));
						if (cell->renditions.background_color == 0)
							SetBkColor(dcBuf, 0);
						else
							SetBkColor(dcBuf, get_xterm_colorref((cell->renditions.background_color-40+256)%256));

						if (cursorX == x && cursorY == y)
						{
							SetTextColor(dcBuf, 0);
							SetBkColor(dcBuf, 0x80FF80);
						}
						TextOutW(dcBuf, x*sz.cx, y*sz.cy, data.c_str(), data.size());

						x += cell->width;

						if (cell->renditions.underlined)
						{
							SelectObject(dcBuf, mainFont);
						}
					}
				BitBlt(hdc, 0, 0, sz.cx*terminalImpl->GetSize().ws_col, sz.cy * terminalImpl->GetSize().ws_row, dcBuf, 0, 0, SRCCOPY);

				SelectObject(dcBuf, oldFont2);
				SelectObject(dcBuf, old);
				DeleteDC(dcBuf);
			}
			SelectObject(hdc, oldFont);
			EndPaint(hWnd, &ps);
			std::wostringstream os;
			os << "WM_PAINT " << (GetTickCount()-startTime) << std::endl;
			OutputDebugString(os.str().c_str());
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
