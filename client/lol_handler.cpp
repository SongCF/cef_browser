#include <string>
#include <iostream>
#include <windows.h>

//回调函数,用于接收采集器抓到的数据
void _stdcall ReceiveMessage(char *args)
{
	std::string message = args;
	std::cout << message << std::endl;
}

typedef void(*pMain)(DWORD procAddress);

int start_lol_handler()
{
	HMODULE hModule = LoadLibrary(L"LolDataHelper.dll");
	if (hModule == NULL)
	{
		MessageBox(NULL, L"load LolDataHelper.dll failed", L"Error", MB_OK);
		return -1;
	}

	pMain Main = (pMain)GetProcAddress(hModule, "Main");
	Main((DWORD)ReceiveMessage);

	//MessageBox(NULL, L"LOL Handler start success!", L"OK", NULL);
	//std::cout << "LOL Handler start success!" << std::endl;
	return 0;
}
