#include"resource.h"
#include<WinSock2.h>
#include<Windows.h>

LPWSTR title = L"QChat-P2P";

#include"..\\common.h"

#pragma comment(lib,"Ws2_32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 


DWORD WINAPI NetWork(LPVOID p);
LRESULT CALLBACK WaitDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	hInst = hInstance;
	if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_WAIT), NULL, WaitDlgProc))
	{
		CreateWnd();
	}

	return 0;
}

LRESULT CALLBACK WaitDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 1;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		DWORD tid;
		CreateThread(NULL, 0, NetWork, hDlg, 0, &tid);
	}
	break;
	case WM_COMMAND:
		if (IDOK == LOWORD(wParam))
			EndDialog(hDlg, 1);
		break;
	case WM_GENLINK:
		EndDialog(hDlg, 0);
		break;
	default:
		ret = 0;
	}

	return ret;
}

DWORD WINAPI NetWork(LPVOID p)
{
	WSADATA wsaData;
	HWND hDlg = (HWND)p;
	WCHAR s[100];
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);//加载套接字模块

	if (result)
	{
		wsprintf(s, L"Socket initialization failed，ErrorCode:%u", result);
		ERRORMSG(hDlg, s);
	}
	else
	{
		SOCKET l;//listen
		l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	//创建套接字

		if (l == INVALID_SOCKET)
		{
			ERRORMSG(hDlg, L"Socket creation failed");
		}
		else
		{
			struct sockaddr_in service;
			memset(&service, 0, sizeof(service));
			service.sin_family = AF_INET;
			service.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
			service.sin_port = htons(5555);

			if (bind(l, (struct sockaddr *)&service, sizeof(service)))	//绑定端口号等
			{
				wsprintf(s, L"Binding failed, ErrorCode:%d", WSAGetLastError());
				ERRORMSG(hDlg, s);
			}
			else
			{
				if (listen(l, 1))
				{
					wsprintf(s, L"Listening failed，ErrorCode:%d", WSAGetLastError());
					ERRORMSG(hDlg, s);
				}
				else
				{
					struct sockaddr_in c;
					int len = sizeof(c);
					SOCKET acc = accept(l, (struct sockaddr *)&c, &len);

					if (INVALID_SOCKET == acc)
					{
						wsprintf(s, L"Accept connection failed，ErrorCode:%d", WSAGetLastError());
						ERRORMSG(hDlg, s);
					}
					else
					{
						skt = acc;
						SendMessage(hDlg, WM_GENLINK, 0, 0);
						while (!hMainWnd)
							Sleep(100);
						RecvData();
					}
				}
			}

			//closesocket(l);//关闭套接字
		}

		//WSACleanup();//释放套接字模块
	}

	return 0;
}