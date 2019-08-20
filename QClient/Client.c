#include"resource.h"
#include<WinSock2.h>
#include<Windows.h>


LPWSTR title = L"QChat-P2P";

#include"..\\common.h"

#pragma comment(lib,"Ws2_32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

LRESULT CALLBACK LoginDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI NetWork(LPVOID p);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	hInst = hInstance;
	if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONNECT), NULL, LoginDlgProc))
	{
		DWORD tid;
		CreateThread(NULL, 0, NetWork, NULL, 0, &tid);
		CreateWnd();
	}

	return 0;
}

LRESULT CALLBACK LoginDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 1;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_PORT, L"5555");
		SetDlgItemText(hDlg, IDC_IP, L"127.0.0.1");
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, 1);
			break;
		case IDOK:
		{
			WCHAR str[100];
			char ip[100], port[10];
			int result;
			WSADATA wsaData;

			GetDlgItemTextA(hDlg, IDC_IP, ip, 100);
			GetDlgItemTextA(hDlg, IDC_PORT, port, 10);

			result = WSAStartup(MAKEWORD(2, 2), &wsaData);//加载套接字模块

			if (result)
			{
				wsprintf(str, L"Socket initialization failed，ErrorCode:%u", result);
				ERRORMSG(hDlg, str);
			}
			else
			{
				SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建一个套接字
				if (INVALID_SOCKET == s)
				{
					ERRORMSG(hDlg, L"Socket creation failed");
				}
				else
				{
					struct sockaddr_in sock;//IP地址、端口号等信息

					memset(&sock, 0, sizeof(sock));

					sock.sin_addr.S_un.S_addr = inet_addr(ip);
					sock.sin_family = AF_INET;
					sock.sin_port = htons(atoi(port));//0 to 65535
					SetDlgItemText(hDlg, IDOK, L"Connecting...");
					EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
					if (0 == connect(s, (struct sockaddr *)&sock, sizeof(sock)))	//连接主机
					{
						skt = s;
						EndDialog(hDlg, 0);
					}
					else
					{
						wsprintf(str, L"Connect failed, ErrorCode:%d", WSAGetLastError());
						ERRORMSG(hDlg, str);
					}
					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
					SetDlgItemText(hDlg, IDOK, L"Connect");
					//closesocket(s);
				}

				//WSACleanup();
			}
		}
		break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 1);
		break;
	default:
		ret = 0;
	}

	return ret;
}

DWORD WINAPI NetWork(LPVOID p)
{
	while (!hMainWnd)
		Sleep(100);
	RecvData();
	return 0;
}