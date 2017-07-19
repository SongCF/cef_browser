// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "client_app.h"

#include <string>

#include "client_handler.h"
#include "include/cef_browser.h"
#include "include/wrapper/cef_helpers.h"
#include <fstream>

ClientApp::ClientApp() {
}

void ClientApp::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();

	// Information used when creating the native window.
	CefWindowInfo window_info;

#if defined(OS_WIN)
	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	window_info.SetAsPopup(NULL, L"瀑布电竞");
#endif

	// SimpleHandler implements browser-level callbacks.
	CefRefPtr<ClientHandler> handler(new ClientHandler());

	// Specify CEF browser settings here.
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

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
		browser_settings, NULL);
}


void ClientApp::OnContextCreated(
	CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefV8Context> context)
{
	CEF_REQUIRE_RENDERER_THREAD();
	CefRefPtr<CefV8Value> window = context->GetGlobal();
	CefRefPtr<CefV8Handler> cbHandler = this;
	CefRefPtr<CefV8Value> func1 = CefV8Value::CreateFunction(L"MessageBox", cbHandler);
	window->SetValue(L"MessageBox", func1, V8_PROPERTY_ATTRIBUTE_NONE);
	CefRefPtr<CefV8Value> func2 = CefV8Value::CreateFunction(L"GetValue", cbHandler);
	window->SetValue(L"GetValue", func2, V8_PROPERTY_ATTRIBUTE_NONE);
	CefRefPtr<CefV8Value> func3 = CefV8Value::CreateFunction(L"Minimize", cbHandler);
	window->SetValue(L"Minimize", func3, V8_PROPERTY_ATTRIBUTE_NONE);
}


void ClientApp::OnWebKitInitialized()
{
	CEF_REQUIRE_RENDERER_THREAD();
// 	std::string extensionCode =
// 		"var g_value=\"global value here\";"
// 		"var test;"
// 		"if (!test)"
// 		"  test = {};"
// 		"(function() {"
// 		"  test.myfunc = function() {"
// 		"    native function hehe(int,int);"
// 		"    return hehe(10, 50);"
// 		"  };"
// 		"})();";
// 
// 	// 声明本地函数 native function hehe();" 如果有参数列表需要写具体的类型,而不能写var类型！与本地声明一直
// 	// 调用本地函数    return hehe();"
// 
// 	// Create an instance of my CefV8Handler object.
// 	CefRefPtr<CefV8Handler> handler = this;
// 
// 	// Register the extension.
// 	CefRegisterExtension("v8/mycode", extensionCode, handler);
}

//render process handle msg
bool ClientApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message) 
{
	CEF_REQUIRE_RENDERER_THREAD();
	return false;
}



// js call cpp methods   in render thread.
bool ClientApp::Execute(const CefString& name,
	CefRefPtr<CefV8Value> object,
	const CefV8ValueList& arguments,
	CefRefPtr<CefV8Value>& retval,
	CefString& exception)
{
	CEF_REQUIRE_RENDERER_THREAD();
	if (name.compare(L"MessageBox") == 0)
	{
		if (arguments.size() == 1 && arguments[0]->IsString()) {
			//js引擎运行在render进程，弹窗需要放到browser进程
			CefRefPtr<CefProcessMessage> objMsg = CefProcessMessage::Create(L"MessageBox");
			CefRefPtr<CefListValue> arglist = objMsg->GetArgumentList();
			arglist->SetString(0, arguments[0]->GetStringValue());
			CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, objMsg);
			return true;
		}
	}
	else if (name.compare(L"GetValue") == 0)
	{
		if (arguments.size() == 1 && arguments[0]->IsString())
		{
			retval = CefV8Value::CreateString(L"return_string");
			return true;
		}
	}
	else if (name.compare(L"Minimize") == 0)
	{
		MessageBox(NULL, L"js call Minimize", L"", NULL);
		return true;
	}
 	return false;
}
