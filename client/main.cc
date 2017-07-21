
// win32 windows + cef browser
//http://blog.csdn.net/mfcing/article/details/43953433

#include <windows.h>

#include "client_app.h"
#include "include/cef_sandbox_win.h"
#include "resource.h"
#include "client_handler.h"
#include <fstream>
#include <sstream>
#include <string>


// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif



// 全局变量:                             
#define szTitle L""                  
#define szWindowClass L"CLIENT"
#define MSG_TRAY (WM_USER + 1)

HWND g_wnd = NULL;
HINSTANCE g_instance = NULL;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void TrayIconVisible(bool b);




// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  g_instance = hInstance;

  void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // SimpleApp implements application-level callbacks. It will create the first
  // browser instance in OnContextInitialized() after CEF has initialized.
  CefRefPtr<ClientApp> app(new ClientApp);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Specify CEF global settings here.
  CefSettings settings;
  settings.single_process = true;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get(), sandbox_info);

  MyRegisterClass(hInstance);
  // 执行应用程序初始化:  
  if (!InitInstance(hInstance, nCmdShow))
  {
	  return FALSE;
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}


//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFSIMPLE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = szTitle;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	float r = 0.75;
	int x = rect.left + width * (1 - r) / 2;
	int y = rect.top + height * (1 - r) / 2;
	int w = width * r;
	int h = height * r;
	//x、y、nWidth、nHeight参数，设置为CW_USEDEFAULT这个默认值，
	//只对于层叠窗口有效，也就是说只有当dwStyle设置为WS_OVERLAPPEDWINDOW这个参数时，
	//才可以使用CW_USEDEFAULT这个宏，否则的话，全部默认是0，这样子窗口自然显示不出来了。
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP, x, y, w, h, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		g_wnd = hWnd;
		RECT rect;
		::GetClientRect(hWnd, &rect);
		CefWindowInfo window_info;
		window_info.SetAsChild(hWnd, rect);
		CefRefPtr<ClientHandler> handler(new ClientHandler());
		CefBrowserSettings browser_settings;
		std::string url;
		std::ifstream in("url");
		if (!in.is_open()) {
			url = "chrome://version";
		}
		else {
			std::stringstream buffer;
			buffer << in.rdbuf();
			url = buffer.str();
		}
		CefBrowserHost::CreateBrowser(window_info, handler.get(), url, browser_settings, NULL);
		TrayIconVisible(true);
		break;
	}
	case WM_DESTROY:
		TrayIconVisible(false);
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
	{
		ShowWindow(hWnd, SW_HIDE);
		break;
	}
	case MSG_TRAY:
		switch (lParam)
		{
		case WM_RBUTTONUP://右键起来时
		case WM_LBUTTONUP://左键的处理 
		case WM_LBUTTONDBLCLK://双击左键的处理 
			ShowWindow(hWnd, SW_SHOW);
			break;
		}
		break;
	case WM_SIZE:
	{
		//MessageBox(hWnd, L"WM_SIZE", L"", MB_OK);
// 		if (wParam == SIZE_MINIMIZED
// 			|| NULL == g_handler
// 			|| NULL == g_handler->GetBrowserHostWnd())
// 			break;
// 		HWND hBrowserWnd = g_handler->GetBrowserHostWnd();
// 		RECT rect;
// 		::GetClientRect(hWnd, &rect);
// 		::MoveWindow(hBrowserWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//托盘
void TrayIconVisible(bool b)
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = g_wnd;
	nid.uID = IDI_SMALL;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = MSG_TRAY;//自定义的消息名称 
	nid.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_SMALL));
	wcscpy_s(nid.szTip, TITLE);
	Shell_NotifyIcon(b ? NIM_ADD : NIM_DELETE, &nid);//在托盘区添加图标 
	//ShowWindow(g_wnd, SW_HIDE);//隐藏主窗口 
}

void HideWindow() 
{
	ShowWindow(g_wnd, SW_HIDE);
}