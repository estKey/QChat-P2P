#include<WindowsX.h>

#define ERRORMSG(hwnd,s) MessageBox(hwnd,s,title,MB_OK | MB_ICONERROR)
#define QUERYOK(hwnd,s) MessageBox(hwnd,s,title,MB_OKCANCEL | MB_ICONINFORMATION)
#define ARRLEN(a) (sizeof(a)/sizeof(a[0]))

#define WM_GENLINK (WM_USER+1)
#define WM_NETDATA (WM_USER+2)
#define WM_PEARCLOSE (WM_USER+3)
#define WM_APPENDMSG (WM_USER+4)

typedef struct tagNETDATA
{
	unsigned int ver;	//版本
	unsigned int type;	//消息类型
	unsigned int len;	//消息字节数
}NETDATA,*PNETDATA;

#define MSG_TYPE_CLOSE	1
#define MSG_TYPE_TEXT	2

HWND hMainWnd = NULL;
SOCKET skt = INVALID_SOCKET;
HINSTANCE hInst;

RECT ClientRect = { 0, 0, /*420 674 580*/580, 439 };

HFONT hDefFont;
WCHAR strButWndClass[] = L"mybut";

void RecvData();

LRESULT CALLBACK WndButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	HDC hdc;
	PAINTSTRUCT ps;
	//static HBRUSH hCurrBrush,hDefBrush,hOverBrush;
	static BOOL bMouseTrack = TRUE;
	static BOOL bMouseDown = FALSE;
	static HBITMAP hButton1,hButton2,hCurrButton;
	switch (uMsg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (hdc)
		{
			RECT ClientRect;
			WCHAR t[100];
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, hDefFont);
			SetTextColor(hdc, RGB(154, 15, 189));
			if (GetClientRect(hWnd, &ClientRect) && GetWindowText(hWnd, t, ARRLEN(t)))
			{
				HDC  hCompDC = CreateCompatibleDC(hdc);
				if (hCompDC)
				{
					if (SelectObject(hCompDC, (HGDIOBJ)hCurrButton))
					{
						StretchBlt(hdc, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top, hCompDC, 0, 0, 100, 25, SRCCOPY);
					}
					DeleteDC(hCompDC);
				}
#define FONT_SIZE 16
				TextOut(hdc, (ClientRect.right - ClientRect.left - FONT_SIZE * wcslen(t)) / 2 + (bMouseDown ? 1 : 0) - 2, (ClientRect.bottom - ClientRect.top - FONT_SIZE) / 2 + (bMouseDown ? 1 : 0), t, wcslen(t));
			}
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_MOUSEMOVE:
//#pragma comment(lib,"comctrl32.lib")
		if (bMouseTrack)
		{
			TRACKMOUSEEVENT csTME;
			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE | TME_HOVER;
			csTME.hwndTrack = hWnd;// 指定要 追踪 的窗口
			csTME.dwHoverTime = 10;  // 鼠标在按钮上停留超过 10ms ，才认为状态为 HOVER
			TrackMouseEvent(&csTME); // 开启 Windows 的 WM_MOUSELEAVE ， WM_MOUSEHOVER 事件支持 

			bMouseTrack = FALSE;
		}
		break;
	case WM_MOUSEHOVER:
		hCurrButton = hButton2;
		bMouseDown = FALSE;
		bMouseTrack = TRUE;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_MOUSELEAVE:
		hCurrButton = hButton1;
		bMouseDown = FALSE;
		bMouseTrack = TRUE;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
		bMouseDown = TRUE;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_LBUTTONUP:
		bMouseDown = FALSE;
		InvalidateRect(hWnd, NULL, TRUE);
		PostMessage(hMainWnd, WM_COMMAND, ((BN_CLICKED & 0xffff) << 16) | (GetWindowLong(hWnd, GWL_ID) & 0xffff), (LPARAM)hWnd);
		break;
	case WM_CREATE:
		hButton1 = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, 0);
		hButton2 = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAP2), IMAGE_BITMAP, 0, 0, 0);
		hCurrButton = hButton1;
		/*
		hDefBrush = NULL;// CreateSolidBrush(RGB(127, 134, 149));
		hOverBrush = CreateSolidBrush(RGB(255, 0, 0));
		hCurrBrush = hDefBrush;
		*/
		break;
	case WM_SETTEXT:
		InvalidateRect(hWnd, NULL, TRUE);
		ret = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	default:
		ret = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return ret;
}

HFONT hEditRead,hEditWrite;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	static HBRUSH hBrush;
	static HICON hIconLogo16;
	static HBITMAP had;

	switch (uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			if (hdc)
			{
				HDC  hCompDC;
				FillRect(hdc, &ClientRect, hBrush);
				DrawIconEx(hdc, 10, 10, (HICON)hIconLogo16, 0, 0, 0, NULL, DI_NORMAL);
				SetBkMode(hdc, TRANSPARENT);

				SelectObject(hdc, hDefFont);
				SetTextColor(hdc, RGB(240, 240, 240));
				TextOut(hdc, 30, 10, title, wcslen(title));

				hCompDC = CreateCompatibleDC(hdc);

				if (hCompDC)
				{
					if (SelectObject(hCompDC, (HGDIOBJ)had))
					{
						StretchBlt(hdc, 420, 40, 149, 350, hCompDC, 0, 0, 149, 350, SRCCOPY);
					}
					DeleteDC(hCompDC);
				}
				EndPaint(hWnd, &ps);
			}
		}
		break;
	case WM_NCHITTEST:
		{
			ret = HTCAPTION;
		}
		break;
	case WM_CREATE:
		{
			WNDCLASSEX w;
			hIconLogo16 = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_LOGO16), IMAGE_ICON,0,0, 0);
			had = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_AD), IMAGE_BITMAP, 0, 0, 0);
			hBrush = CreateSolidBrush(RGB(154, 15, 189));
#define	FONT_SIZE	16
			hDefFont = CreateFontW(16, FONT_SIZE/2, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"楷体");
			hEditRead = CreateFont(14, 7, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"宋体");
			hEditWrite = CreateFont(14, 7, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"楷体");
			
			w.cbClsExtra = 0;
			w.cbWndExtra = 0;
			w.cbSize = sizeof(w);
			w.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			w.hCursor = LoadCursor(NULL, IDC_ARROW);
			w.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LOGO16));
			w.hIconSm = w.hIcon;
			w.hInstance = hInst;
			w.lpfnWndProc = WndButtonProc;// WndButtonProc;DefWindowProc
			w.lpszClassName = strButWndClass;
			w.lpszMenuName = NULL;
			w.style = CS_DBLCLKS | CS_DROPSHADOW;

			if (RegisterClassEx(&w))
			{
#define IDB_EXIT_PROC 3
//#define IDB_SEND_FILE 4
#define IDB_SEND_MSG 5
#define IDE_READ	6
#define IDE_WRITE	7
				HWND h = CreateWindowEx(0, L"edit", L"", WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 10, 40, 400, 240, hWnd, (HMENU)IDE_READ, NULL, NULL);
				SendMessage(h, WM_SETFONT, (WPARAM)hEditRead, TRUE);
				h = CreateWindowEx(0, L"edit", L"", WS_VISIBLE | WS_CHILD | ES_MULTILINE |  ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN, 10, 290, 400, 100, hWnd, (HMENU)IDE_WRITE, NULL, NULL);
				SendMessage(h, WM_SETFONT, (WPARAM)hEditWrite, FALSE);
				SendMessage(CreateWindowExW(0, strButWndClass, L"  Exit", WS_VISIBLE | WS_CHILD, 60, 400, 100, 25, hWnd, (HMENU)IDB_EXIT_PROC, NULL, NULL), WM_SETFONT, (WPARAM)hDefFont, TRUE);
				//SendMessage(CreateWindowEx(0, strButWndClass, L"Send File", WS_VISIBLE | WS_CHILD, 160, 400, 100, 25, hWnd, (HMENU)IDB_SEND_FILE, NULL, NULL), WM_SETFONT, (WPARAM)hDefFont, TRUE);
				SendMessage(CreateWindowEx(0, strButWndClass, L"  Send", WS_VISIBLE | WS_CHILD, 160, 400, 100, 25, hWnd, (HMENU)IDB_SEND_MSG, NULL, NULL), WM_SETFONT, (WPARAM)hDefFont, TRUE);
			}
		}
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
			case BN_CLICKED:
				switch(LOWORD(wParam))
				{
					case IDB_EXIT_PROC:
						PostMessage(hWnd,WM_CLOSE,0,0);
						break;
					case IDB_SEND_MSG:
						{
							NETDATA d;
							LPWSTR p;
							d.type = MSG_TYPE_TEXT;
							d.ver = 1;
							d.len = (Edit_GetTextLength(GetDlgItem(hWnd,IDE_WRITE)) + 1)*2;

							if(2 ==d.len)
								ERRORMSG(hWnd,L"The message cannot be empty!");
							else
							{
								p=malloc(d.len);
								if(p)
								{
									GetDlgItemText(hWnd,IDE_WRITE,p,d.len);
									send(skt,&d,sizeof(d),0);
									send(skt,p,d.len,0);
									SendMessage(hWnd,WM_APPENDMSG,p,0);
									free(p);
									SetDlgItemText(hWnd,IDE_WRITE,L"");
								}
								else
									ERRORMSG(hWnd,L"Apply memory failed");
							}
						}
						break;
				}
				break;
			case EN_CHANGE:
				switch(LOWORD(wParam))
				{
					case IDE_WRITE:
					{
						NETDATA d;
						LPWSTR p;
						d.type = MSG_TYPE_TEXT;
						d.ver = 1;
						d.len = (Edit_GetTextLength(GetDlgItem(hWnd,IDE_WRITE)) + 1)*2;

						if(2 ==d.len)
							ERRORMSG(hWnd,L"The message cannot be empty!");
						else
						{
							p=malloc(d.len);
							if(p)
							{
								GetDlgItemText(hWnd,IDE_WRITE,p,d.len);
								if(wcsstr(p,L"\r\n"))
								{
									size_t x = wcslen(p);
									if(p[x-1]==L'\n' && p[x-2]==L'\r')
										p[x-2]=L'\0';
									SetDlgItemText(hWnd,IDE_WRITE,p);
									PostMessage(hMainWnd, WM_COMMAND, ((BN_CLICKED & 0xffff) << 16) | (IDB_SEND_MSG & 0xffff), (LPARAM)hWnd);
								}
								free(p);
								//ERRORMSG(hWnd,p);
							}
							else
								ERRORMSG(hWnd,L"Apply memory failed");
						}
					}
					break;
				}
				break;
		}
		
		break;
	case WM_NETDATA:
		SendMessage(hWnd,WM_APPENDMSG,wParam,1);
		free(wParam);
		break;
	case WM_APPENDMSG:
		{
#define PEAR_DATA_SPLIT L"\r\nQChater：\r\n  "
#define MY_DATA_SPLIT L"\r\nMe：\r\n  "
			LPWSTR p;
			int len = (Edit_GetTextLength(GetDlgItem(hWnd,IDE_READ)) + 1)*sizeof(WCHAR);
			len += (wcslen(wParam))*2;
			len += 2*(lParam ? sizeof(PEAR_DATA_SPLIT) : sizeof(MY_DATA_SPLIT));

			p = malloc(len);

			if(p)
			{
				LPWSTR pt=p;
				int i;
				HWND h=GetDlgItem(hWnd,IDE_READ);
				i = GetWindowText(h,p,len);
				pt += wcslen(pt);
				wcscat(pt,lParam ? PEAR_DATA_SPLIT : MY_DATA_SPLIT);
				pt += wcslen(pt);
				wcscpy(pt,wParam);
				SetWindowText(h,p);
				SendMessage(h,WM_VSCROLL,SB_BOTTOM,0); 
			}
			else
				ERRORMSG(hWnd,L"The chat history can not be updated due to memory reasons");
		}
		break;
	case WM_PEARCLOSE:
		{
			HANDLE h=GetDlgItem(hWnd,IDB_SEND_MSG);
			ERRORMSG(hWnd,L"Connection ends, cannot send messages");
			SetWindowText(h,L"Offline...");
			EnableWindow(h,FALSE);
			closesocket(skt);
			WSACleanup();
		}
		break;
	case WM_CLOSE:
		if (IDOK == QUERYOK(hWnd,L"Are you sure you want to exit?"))
		{
			NETDATA d;
			LPWSTR p;
			d.type = MSG_TYPE_CLOSE;
			d.ver = 1;
			d.len = 0;
			send(skt,&d,sizeof(d),0);
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		ret = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return ret;
}

void RecvData()
{
	while(1)
	{
		NETDATA d;
		int len = 0;
		char *p=&d;
		WCHAR err_text[100];
		while(len < sizeof(d))
		{
			int r = recv(skt,p,sizeof(d)-len,0);
			if(r>=0)
			{
				len += r;
				p+=r;
			}
			else
			{
				wsprintf(err_text,L"Thread cannot receive data, there may be a connection problem, will not receive any information, ErrorCode:%u",WSAGetLastError());
				break;
			}
		}

		if(len >= sizeof(d))
		{
			p=malloc(d.len);
			len = 0;
			if(p)
			{
				char *pt=p;
				while(len < d.len)
				{
					int r = recv(skt,pt,d.len-len,0);
					if(r>=0)
					{
						len += r;
						pt += r;
					}
					else
					{
						ERRORMSG(hMainWnd,L"Thread cannot receive all data, there may be a connection problem, will not receive any information");//接收数据线程只接收部分数据，可能连接出现问题，将收不到任何信息
						return;
					}
				}

				switch(d.type)
				{
				case MSG_TYPE_TEXT:
					SendMessage(hMainWnd,WM_NETDATA,p,0);
					break;
				case MSG_TYPE_CLOSE:
					SendMessage(hMainWnd,WM_PEARCLOSE,0,0);
					break;
				}
			}
			else
			{
				ERRORMSG(hMainWnd,L"接收数据线程申请内存空间失败，将收不到任何信息");
				break;
			}
		}
		else
		{
			//ERRORMSG(hMainWnd,err_text);
			break;
		}
	}
}

void CreateWnd()
{
	WCHAR strWndClass[] = L"testchri";

		HWND hWnd;

		MSG Msg;

		WNDCLASSEX w;

		DWORD dwWndStyle = WS_POPUP;// WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;

		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		int nWndHeight = ClientRect.bottom - ClientRect.top, nWndWidth = ClientRect.right - ClientRect.left;

		int nPosX = ((nScreenWidth - nWndWidth) > 0) ? (nScreenWidth - nWndWidth) / 2 : 0;
		int nPosY = ((nScreenHeight - nWndHeight) > 0) ? ((nScreenHeight - nWndHeight)) / 2 : 0;

		w.cbClsExtra = 0;
		w.cbWndExtra = 0;
		w.cbSize = sizeof(w);
		w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		//CreateSolidBrush(83, 211, 118);
		//w.hbrBackground = (HBRUSH)CreateSolidBrush(83, 211, 118);
		//w.hbrBackground = (HBRUSH)CreateSolidBrush(255, 255, 255);
		w.hCursor = LoadCursor(NULL, IDC_ARROW);
		w.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		w.hIconSm = w.hIcon;
		w.hInstance = hInst;
		w.lpfnWndProc = WndProc;
		w.lpszClassName = strWndClass;
		w.lpszMenuName = NULL;
		w.style = CS_DBLCLKS | CS_DROPSHADOW;

		if (!RegisterClassEx(&w))
		{
			ERRORMSG(NULL,L"Register Windows failed");
			return;
		}

		AdjustWindowRect(&ClientRect, dwWndStyle, FALSE);

		hWnd = CreateWindowEx(0, strWndClass, title, dwWndStyle,nPosX, nPosY, nWndWidth, nWndHeight, NULL, NULL, hInst, NULL);
		hMainWnd = hWnd;

		if (!hWnd)
		{
			ERRORMSG(hWnd,L"Create Window failed");
			return;
		}

		ShowWindow(hWnd, SW_NORMAL);

		UpdateWindow(hWnd);

		while (GetMessage(&Msg, NULL, 0, 0))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
}