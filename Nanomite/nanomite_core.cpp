#include "nanomite.h"

//---------------------------------------------------------------------------
HWND	hwLBCallStack,
		hDlgSettings,
		hDlgMain,
		hDlgBPManager,
		hDlgDetInfo,
		hDlgMemMap,
		hDlgHeapMap,
		hDlgWndList,
		hDlgResList,
		hDlgDbgStringInfo;

int iMemMapPID = 0,
	iHeapMapPID = 0,
	iPeViewPID = 0,
	iStringViewPID = 0,
	iWndListPID = 0,
	iHandleViewPID = 0,
	iResListPID = 0;

HMENU	hMemMapMenu,
		hHeapMapMenu,
		hSuspendMenu,
		hStopMenu,
		hPeMenu,
		hStringMenu,
		hWndListMenu,
		hHandleMenu,
		hResMenu;

clsDebugger newDebugger;

PTCHAR	tcLogging,
		tcTempState;

DWORD dwExceptionCount;
//---------------------------------------------------------------------------

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icx);

	HWND hWND = 0;

	tcLogging = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	tcTempState = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINFRAME),hWND, reinterpret_cast<DLGPROC>(MainDLGProc));

	free(tcLogging);
	free(tcTempState);

	_CrtDumpMemoryLeaks();
	return false;
}

LRESULT CALLBACK MainDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			hDlgMain = hWndDlg;
			ReadFromSettingsFile();

			HICON hIconMain = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MAIN));
			HANDLE hIconStart = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_START),IMAGE_ICON,18,18,0);
			HANDLE hIconStop = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_STOP),IMAGE_ICON,18,18,0);
			HANDLE hIconReStart = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_RESTART),IMAGE_ICON,18,18,0);
			HANDLE hIconStepOver = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_STEPOVER),IMAGE_ICON,18,18,0);
			HANDLE hIconStepIn = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_STEPIN),IMAGE_ICON,18,18,0);
			HANDLE hIconSuspend = LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_SUSPEND),IMAGE_ICON,18,18,0);

			SendMessage(hDlgMain,WM_SETICON,ICON_BIG,(LPARAM)hIconMain);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_START),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconStart);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_STOP),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconStop);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_RESTART),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconReStart);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_STEPOVER),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconStepOver);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_STEPIN),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconStepIn);
			SendMessage(GetDlgItem(hDlgMain,ID_DEBUG_SUSPEND),BM_SETIMAGE,ICON_BIG,(LPARAM)hIconSuspend);

			LoadCallBacks();

			//------------------- Init. ListC. ----------
			LVCOLUMN LvCol;
			hwLBCallStack = GetDlgItem(hDlgMain,IDC_LIST2);
			SendMessage(hwLBCallStack,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Stack Offset";                         
			LvCol.cx = 0x8A;                               
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Func. Addr";
			LvCol.cx = 0x8A;
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"<mod.func>"; 
			LvCol.cx = 0x100;
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Return To";
			LvCol.cx = 0x8A;
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"Return To - <mod.func>"; 
			LvCol.cx = 0x100; 
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);
			LvCol.pszText = L"SourceLine"; 
			LvCol.cx = 0x60; 
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol);
			LvCol.pszText = L"SourceFile"; 
			LvCol.cx = 0x80;
			SendMessage(hwLBCallStack,LVM_INSERTCOLUMN,6,(LPARAM)&LvCol);

			//-------------------------------------------

			//------------------- Init. DisAss. ----------
			HWND hwDisAssLC = GetDlgItem(hDlgMain,ID_DISASS);
			SendMessage(hwDisAssLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Offset";                         
			LvCol.cx = 0x8A;                               
			SendMessage(hwDisAssLC,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"OpCodes";
			LvCol.cx = 0xAA;
			SendMessage(hwDisAssLC,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Mnemonics"; 
			LvCol.cx = 0xDD;
			SendMessage(hwDisAssLC,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Comment";
			LvCol.cx = 0xA9;
			SendMessage(hwDisAssLC,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			//-------------------------------------------

			//------------------- Init. StackView ----------
			HWND hwStackViewLC = GetDlgItem(hDlgMain,ID_STACKVIEW);
			SendMessage(hwStackViewLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Offset";                         
			LvCol.cx = 0x8A;                               
			SendMessage(hwStackViewLC,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Data";
			LvCol.cx = 0x8A;
			SendMessage(hwStackViewLC,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Comment"; 
			LvCol.cx = 0x75;
			SendMessage(hwStackViewLC,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			//-------------------------------------------

			//------------------- Init. Log ----------
			HWND hwLogLC = GetDlgItem(hDlgMain,IDC_LOG);
			SendMessage(hwLogLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Time";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwLogLC,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Log";
			LvCol.cx = 0x200;
			SendMessage(hwLogLC,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			//-------------------------------------------

			//----------------- Loading BreakPoint Manager -----------------
			hDlgBPManager = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_BPMANAGER),hDlgMain,reinterpret_cast<DLGPROC>(BPManagerDLGProc));
			//----------------- Loading BreakPoint Manager -----------------

			//----------------- Loading Detail Info ------------------------
			hDlgDetInfo = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DETINFO),hDlgMain,reinterpret_cast<DLGPROC>(DetailInfoDLGProc));
			//----------------- Loading Detail Info ------------------------
		
			//----------------- Loading DebugString Info ------------------------
			hDlgDbgStringInfo = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DBGSTR),hDlgMain,reinterpret_cast<DLGPROC>(DebugStringDLGProc));
			//----------------- Loading DebugString Info ------------------------


			//----------------- Init. Font ------------------------------
			HFONT hFont = CreateFont(11,0,0,0,FW_MEDIUM,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,L"Lucida Console");
			SendMessage(hwDisAssLC,WM_SETFONT,(WPARAM)hFont,TRUE);
			SendMessage(hwLBCallStack,WM_SETFONT,(WPARAM)hFont,TRUE);
			SendMessage(hwLogLC,WM_SETFONT,(WPARAM)hFont,TRUE);
			SendMessage(hwStackViewLC,WM_SETFONT,(WPARAM)hFont,TRUE);
			//----------------- Init. Font ------------------------------

			return true;
		}

	//case WM_SIZE:
	//	{
	//		RECT rectCurrent;
	//		GetClientRect(hWndDlg,&rectCurrent);

	//		if(rectCurrent.bottom < 496 || rectCurrent.right < 1066)
	//		{
	//			SetWindowPos(hWndDlg,NULL,rectCurrent.left,rectCurrent.top,1080,555,SWP_NOMOVE);
	//			return true;
	//		}

	//		// Y + && -
	//		if(rectCurrent.bottom > HIWORD(lParam) && rectCurrent.right == LOWORD(lParam))
	//		{
	//			GetClientRect(GetDlgItem(hWndDlg,ID_DISASS),&rectCurrent);
	//			SetWindowPos(GetDlgItem(hWndDlg,ID_DISASS),NULL,0,0,rectCurrent.right,rectCurrent.bottom + 10,SWP_NOMOVE | SWP_NOOWNERZORDER);
	//		} 
	//		else if(rectCurrent.bottom < HIWORD(lParam) && rectCurrent.right == LOWORD(lParam))
	//		{
	//			GetClientRect(GetDlgItem(hWndDlg,ID_DISASS),&rectCurrent);
	//			SetWindowPos(GetDlgItem(hWndDlg,ID_DISASS),NULL,0,0,rectCurrent.right,rectCurrent.bottom - 10,SWP_NOMOVE | SWP_NOOWNERZORDER);
	//		}
	//		// X + && -
	//		else if(rectCurrent.bottom == HIWORD(lParam) && rectCurrent.right > LOWORD(lParam))
	//		{
	//			GetClientRect(GetDlgItem(hWndDlg,ID_DISASS),&rectCurrent);
	//			SetWindowPos(GetDlgItem(hWndDlg,ID_DISASS),NULL,0,0,rectCurrent.right + 10,rectCurrent.bottom,SWP_NOMOVE | SWP_NOOWNERZORDER);
	//		}
	//		else if(rectCurrent.bottom == HIWORD(lParam) && rectCurrent.right < LOWORD(lParam))
	//		{
	//			GetClientRect(GetDlgItem(hWndDlg,ID_DISASS),&rectCurrent);
	//			SetWindowPos(GetDlgItem(hWndDlg,ID_DISASS),NULL,0,0,rectCurrent.right - 10,rectCurrent.bottom,SWP_NOMOVE | SWP_NOOWNERZORDER);
	//		}

	//		RedrawWindow(hWndDlg,NULL,NULL,RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	//		return true;
	//	}
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_DEBUG_RESTART:
			if(newDebugger.GetDebuggingState())
				newDebugger.StopDebuggingAll();

			CleanUpGUI();
			if(newDebugger.IsTargetSet())
				StartDebugging();
			return true;

		case IDC_BPMANAGER:
			ShowWindow(hDlgBPManager,SW_SHOW);
			return true;

		case ID_FILE_OPENA:
			if(newDebugger.GetDebuggingState())
				newDebugger.StopDebuggingAll();

			newDebugger.RemoveBPs();
			ReadFromSettingsFile();
			CleanUpGUI();
			MenuLoadNewFile();
			UpdateStateLable(0x3);
			return true;

		case ID_DEBUG_SUSPEND:
			hSuspendMenu = GenerateRunningPIDMenu(true);
			break;

		case ID_DEBUG_START:
			if(!newDebugger.GetDebuggingState())
			{
				ReadFromSettingsFile();
				CleanUpGUI();

				if(!newDebugger.IsTargetSet())
					if(!MenuLoadNewFile())
						return true;

				StartDebugging();
				UpdateStateLable(0x1);
			}
			else
			{
				newDebugger.ResumeDebugging();
				UpdateStateLable(0x1);
			}
			return true;

		case ID_DEBUG_STOP:
			hStopMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_FILE_EXIT:
			if(newDebugger.GetDebuggingState())
				newDebugger.StopDebuggingAll();
			EndDialog(hDlgMain,0);
			return true;

		case ID_DEBUG_STEPOVER:
			if(newDebugger.GetDebuggingState())
#ifdef _AMD64_
				newDebugger.StepOver(CalcNewOffset(newDebugger.ProcessContext.Rip));
#else
				newDebugger.StepOver(CalcNewOffset(newDebugger.ProcessContext.Eip));
#endif
			return true;

		case ID_DEBUG_STEPIN:
			if(newDebugger.GetDebuggingState())
				newDebugger.StepIn();
			return true;

		case ID_FILE_DETACH:
			if(newDebugger.GetDebuggingState())
			{
				if(!newDebugger.DetachFromProcess())
					MessageBox(hDlgMain,L"Failed to detach from Process!",L"Nanomite",MB_OK);
				else
					UpdateStateLable(0x3);
			}
			return true;
			
		case ID_ABOUT_OPTION:
			{
				HWND hOption = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPTION),hDlgMain,reinterpret_cast<DLGPROC>(OptionDLGProc));
				ShowWindow(hOption,SW_SHOW);
				return true;
			}
		case ID_DETINFO:
			ShowWindow(hDlgDetInfo,SW_SHOW);
			return true;

		case ID_ABOUT_ABOUT:
			{
				HWND hAbout = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_ABOUT),hDlgMain,reinterpret_cast<DLGPROC>(AboutDLGProc));
				ShowWindow(hAbout,SW_SHOW);
				return true;
			}
		case ID_FILE_ATTACH:
			if(!newDebugger.GetDebuggingState())
			{
				HWND hAttachTo = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_ATTACH),hDlgMain,reinterpret_cast<DLGPROC>(AttachToDLGProc));
				ShowWindow(hAttachTo,SW_SHOW);
			}
			return true;

		case ID_DEBUGSTRINGS:
			ShowWindow(hDlgDbgStringInfo,SW_SHOW);
			return true;

		case ID_WNDLIST:
			hWndListMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_MEMMAP:
			hMemMapMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_HEAPMAP:
			hHeapMapMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_PEVIEW:
			hPeMenu = GenerateRunningPIDMenu(false);
			return true;

		case ID_HANDLELISTS:
			hHandleMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_RESSOURCES:
			hResMenu = GenerateRunningPIDMenu(true);
			return true;

		case ID_STRINGS:
			hStringMenu = GenerateRunningPIDMenu(true);
			return true;
		}
		break;

	case WM_MENUCOMMAND:
		{
			if((HMENU)lParam == hMemMapMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hMemMapMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iMemMapPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iMemMapPID = i;
						}
					}
					free(sTemp);

					HWND hwMemMap = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_MEMVIEW),hDlgMain,reinterpret_cast<DLGPROC>(MemMapDLGProc));
					ShowWindow(hwMemMap,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hHeapMapMenu)
			{		
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hHeapMapMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iHeapMapPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iHeapMapPID = i;
						}
					}	
					free(sTemp);

					HWND hwHeapMap = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_HEAPMAP),hDlgMain,reinterpret_cast<DLGPROC>(HeapMapDLGProc));
					ShowWindow(hwHeapMap,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hSuspendMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hSuspendMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
					{
						newDebugger.SuspendDebuggingAll();
						UpdateStateLable(0x2);			
					}
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
							{
								newDebugger.SuspendDebugging(newDebugger.PIDs[i].dwPID);
							}
						}	
					}
					free(sTemp);
				}
			}
			else if((HMENU)lParam == hStopMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hStopMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
					{
						newDebugger.StopDebuggingAll();
						UpdateStateLable(0x3);
					}
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								newDebugger.StopDebugging(newDebugger.PIDs[i].dwPID);
						}			
					}
					free(sTemp);
				}
			}
			else if((HMENU)lParam == hPeMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hPeMenu,wParam,sTemp,255,NULL);
					for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
					{
						wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
						if(wcsstr(sTemp,ss.str().c_str()) != 0)
							iPeViewPID = i;
					}
					free(sTemp);

					HWND hwPEView = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_PEVIEW),hDlgMain,reinterpret_cast<DLGPROC>(PeViewDLGProc));
					ShowWindow(hwPEView,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hStringMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hStringMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iStringViewPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iStringViewPID = i;
						}
					}	
					free(sTemp);

					HWND hwStringView = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_STRINGVIEW),hDlgMain,reinterpret_cast<DLGPROC>(StringViewDLGProc));
					ShowWindow(hwStringView,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hHandleMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hHandleMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iHandleViewPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iHandleViewPID = i;
						}
					}	
					free(sTemp);

					HWND hwHandleLiew = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_HANDLELIST),hDlgMain,reinterpret_cast<DLGPROC>(HandleViewDLGProc));
					ShowWindow(hwHandleLiew,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hWndListMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hWndListMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iWndListPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iWndListPID = i;
						}
					}	
					free(sTemp);

					hDlgWndList = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_WNDLIST),hDlgMain,reinterpret_cast<DLGPROC>(WndListDLGProc));
					ShowWindow(hDlgWndList,SW_SHOW);
				}
			}
			else if((HMENU)lParam == hResMenu)
			{
				if(newDebugger.GetDebuggingState())
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					GetMenuString(hResMenu,wParam,sTemp,255,NULL);
					if(wcsstr(sTemp,L"ALL") != 0 || wcscmp(sTemp,L"") == 0)
						iResListPID = newDebugger.PIDs.size();
					else
					{
						for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
						{
							wstringstream ss; ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
							if(wcsstr(sTemp,ss.str().c_str()) != 0)
								iResListPID = i;
						}
					}	
					free(sTemp);

					hDlgResList = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_RESSOURCES),hDlgMain,reinterpret_cast<DLGPROC>(RessourceDLGProc));
					ShowWindow(hDlgResList,SW_SHOW);
				}
			}
			break;
		}

	case WM_CLOSE:
		if(newDebugger.GetDebuggingState())
			newDebugger.StopDebuggingAll();
		EndDialog(hDlgMain,0);
		return true;

	case WM_VSCROLL:

	switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			{
				if((HWND)lParam == GetDlgItem(hDlgMain,ID_SCROLLER))
				{
					DWORD64 dwOffset = 0;
					PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

					LVITEM lvItem;
					memset(&lvItem,0,sizeof(lvItem));

					lvItem.mask = LVIF_TEXT;
					lvItem.cchTextMax = 255;
					lvItem.iItem = 0;
					lvItem.iSubItem = 0;
					lvItem.pszText = sTemp;
					SendMessage(GetDlgItem(hDlgMain,ID_DISASS),LVM_GETITEMTEXT,0,(LPARAM)&lvItem);
					wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwOffset;

					ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_DISASS));
					LoadDisAssView(dwOffset);
					free(sTemp);
					break;
				}
				else if((HWND)lParam == GetDlgItem(hDlgMain,ID_SCROLLER2))
				{
					DWORD dwStackSize = NULL;
					DWORD64 dwOffset = NULL;

#ifdef _AMD64_
					BOOL bIsWOW64 = false;
					IsWow64Process(newDebugger.GetCurrentProcessHandle(),&bIsWOW64);

					if(bIsWOW64) dwStackSize = 4;	
					else dwStackSize = 8;
#else
					dwStackSize = 4;
#endif

					PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

					LVITEM lvItem;
					memset(&lvItem,0,sizeof(lvItem));

					lvItem.mask = LVIF_TEXT;
					lvItem.cchTextMax = 255;
					lvItem.iItem = 0;
					lvItem.iSubItem = 0;
					lvItem.pszText = sTemp;
					SendMessage(GetDlgItem(hDlgMain,ID_STACKVIEW),LVM_GETITEMTEXT,0,(LPARAM)&lvItem);
					wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwOffset;

					ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_STACKVIEW));
					if(dwOffset == 0x0)
#ifdef _AMD64_
						dwOffset = newDebugger.ProcessContext.Rsp;
#else
						dwOffset = newDebugger.ProcessContext.Esp;
#endif
					LoadStackView(dwOffset,dwStackSize);
					free(sTemp);
					break;
				}
			}

		case SB_LINEDOWN:
			{
				if((HWND)lParam == GetDlgItem(hDlgMain,ID_SCROLLER))
				{
					DWORD64 dwOffset = 0;
					PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

					LVITEM lvItem;
					memset(&lvItem,0,sizeof(lvItem));

					lvItem.mask = LVIF_TEXT;
					lvItem.cchTextMax = 255;
					lvItem.iItem = 10;
					lvItem.iSubItem = 0;
					lvItem.pszText = sTemp;
					SendMessage(GetDlgItem(hDlgMain,ID_DISASS),LVM_GETITEMTEXT,10,(LPARAM)&lvItem);
					wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwOffset;

					ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_DISASS));
					LoadDisAssView(dwOffset);
					free(sTemp);
					break;
				}
				else if((HWND)lParam == GetDlgItem(hDlgMain,ID_SCROLLER2))
				{
					DWORD dwStackSize = NULL;
					DWORD64 dwOffset = NULL;

#ifdef _AMD64_
					BOOL bIsWOW64 = false;
					IsWow64Process(newDebugger.GetCurrentProcessHandle(),&bIsWOW64);

					if(bIsWOW64) dwStackSize = 4;	
					else dwStackSize = 8;
#else
					dwStackSize = 4;
#endif

					PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

					LVITEM lvItem;
					memset(&lvItem,0,sizeof(lvItem));

					lvItem.mask = LVIF_TEXT;
					lvItem.cchTextMax = 255;
					lvItem.iItem = 0;
					lvItem.iSubItem = 0;
					lvItem.pszText = sTemp;
					SendMessage(GetDlgItem(hDlgMain,ID_STACKVIEW),LVM_GETITEMTEXT,0,(LPARAM)&lvItem);
					wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwOffset;
					
					ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_STACKVIEW));
					if(dwOffset == 0x0)
#ifdef _AMD64_
						dwOffset = newDebugger.ProcessContext.Rsp;
#else
						dwOffset = newDebugger.ProcessContext.Esp;
#endif
					LoadStackView(dwOffset + (dwStackSize * 12),dwStackSize);
					free(sTemp);
					break;
				}
			}
		}
		break;
	}

	return false;
}

LRESULT CALLBACK OptionDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			hDlgSettings = hWndDlg;
			ReadFromSettingsFile();

			SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK1),BM_SETCHECK,(WPARAM)newDebugger.dbgSettings.bDebugChilds,NULL);
			SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK2),BM_SETCHECK,(WPARAM)newDebugger.dbgSettings.bAutoLoadSymbols,NULL);
			SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK8),BM_SETCHECK,(WPARAM)newDebugger.dbgSettings.dwSuspendType,NULL);
			SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK9),BM_SETCHECK,(WPARAM)newDebugger.dbgSettings.dwDefaultExceptionMode,NULL);

			switch (newDebugger.dbgSettings.dwBreakOnEPMode)
			{
			case 0: // 0 = Default EP
				SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO3),BM_SETCHECK,(WPARAM)BST_CHECKED,NULL);
				break;
			case 1: // 1 = Kernel EP
				SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO1),BM_SETCHECK,(WPARAM)BST_CHECKED,NULL);
				break;
			case 2: // 2 = TLS Callback
				SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO2),BM_SETCHECK,(WPARAM)BST_CHECKED,NULL);
				break;
			case 3: // 3= Direct Run
				SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO4),BM_SETCHECK,(WPARAM)BST_CHECKED,NULL);
				break;
			}

			for(size_t i = 0;i < newDebugger.ExceptionHandler.size();i++)
			{
				/*if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_BREAKPOINT)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK3),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
				else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_SINGLE_STEP)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK4),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
				else */if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK5),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
				if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_PRIV_INSTRUCTION)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK7),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
				if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_ILLEGAL_INSTRUCTION)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK7),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
				if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_INT_DIVIDE_BY_ZERO)
					SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK6),BM_SETCHECK,(WPARAM)newDebugger.ExceptionHandler[i].dwAction,NULL);
			}

			return true;
		}
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_SAVE:
			newDebugger.dbgSettings.bDebugChilds = SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK1),BM_GETCHECK,NULL,NULL);
			newDebugger.dbgSettings.bAutoLoadSymbols = SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK2),BM_GETCHECK,NULL,NULL);
			newDebugger.dbgSettings.dwSuspendType = SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK8),BM_GETCHECK,NULL,NULL);
			newDebugger.dbgSettings.dwDefaultExceptionMode = SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK9),BM_GETCHECK,NULL,NULL);

			newDebugger.CustomExceptionRemoveAll();
			//newDebugger.CustomExceptionAdd(EXCEPTION_BREAKPOINT,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK3),BM_GETCHECK,NULL,NULL),NULL);
			//newDebugger.CustomExceptionAdd(EXCEPTION_SINGLE_STEP,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK4),BM_GETCHECK,NULL,NULL),NULL);
			newDebugger.CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK5),BM_GETCHECK,NULL,NULL),NULL);
			newDebugger.CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK7),BM_GETCHECK,NULL,NULL),NULL);
			newDebugger.CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK7),BM_GETCHECK,NULL,NULL),NULL);
			newDebugger.CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,SendMessage(GetDlgItem(hDlgSettings,IDC_CHECK6),BM_GETCHECK,NULL,NULL),NULL);

			if(SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO1),BM_GETCHECK,NULL,NULL) == BST_CHECKED)
				newDebugger.dbgSettings.dwBreakOnEPMode = 1; // system ep
			if(SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO3),BM_GETCHECK,NULL,NULL) == BST_CHECKED)
				newDebugger.dbgSettings.dwBreakOnEPMode = 0; // def ep
			if(SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO2),BM_GETCHECK,NULL,NULL) == BST_CHECKED)
				newDebugger.dbgSettings.dwBreakOnEPMode = 2; // tls
			if(SendMessage(GetDlgItem(hDlgSettings,IDC_RADIO4),BM_GETCHECK,NULL,NULL) == BST_CHECKED)
				newDebugger.dbgSettings.dwBreakOnEPMode = 3; // direct


			if(WriteToSettingsFile())
				MessageBox(hDlgSettings,L"Settings saved!",L"Nanomite v 0.1",MB_OK);

			EndDialog(hDlgSettings,0);
			return true;

		case IDC_CANCEL:
			EndDialog(hDlgSettings,0);
			return true;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlgSettings,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK BPManagerDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwBreakPointLC = GetDlgItem(hWndDlg,IDC_LIST1);
	switch(Msg)
	{
	case WM_NOTIFY:
		{
			if(GetAsyncKeyState(VK_DELETE) == 0xffff8001)
			{
				int x = SendMessage(hwBreakPointLC,LVM_GETHOTITEM,0,0);
				x = SendMessage(hwBreakPointLC,LVM_GETNEXTITEM,-1,MAKELPARAM(LVNI_SELECTED, 0));
				if(x != -1)
				{
					DWORD dwOffsets = NULL,
						dwType = NULL,
						dwPID = NULL;
					PTCHAR sTemp = (PTCHAR)malloc(255);
					LVITEM lvItem;
					memset(&lvItem,0,sizeof(lvItem));

					lvItem.mask = LVIF_TEXT;
					lvItem.cchTextMax = 255;
					lvItem.iItem = x;
					lvItem.iSubItem = 0;
					lvItem.pszText = sTemp;
					ListView_GetItemText(hwBreakPointLC,x,0,sTemp,255);
					wstringstream ss; ss << hex << sTemp; ss >> dwOffsets;

					lvItem.iSubItem = 1;
					ListView_GetItemText(hwBreakPointLC,x,1,sTemp,255);
					wstringstream ss2; ss2 << hex << sTemp; ss2 >> dwPID;

					lvItem.iSubItem = 2;
					ListView_GetItemText(hwBreakPointLC,x,2,sTemp,255);
					if(wcscmp(sTemp,L"Software BP ( 0xCC / INT 3)") == 0)
						dwType = 0;
					else if(wcscmp(sTemp,L"Memory BP ( PageGuard)") == 0)
						dwType = 1;
					else if(wcscmp(sTemp,L"Hardware BP ( INT1 / Dr. Reg)") == 0)
						dwType = 2;
					newDebugger.RemoveBPFromList(dwOffsets,dwType,dwPID);
					ListView_DeleteItem(hwBreakPointLC,x);
					free(sTemp);
				}
			}
			return true;
		}
	case WM_INITDIALOG:
		{
			LVCOLUMN lvBPCol;
			SendMessage(hwBreakPointLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			Edit_SetText(GetDlgItem(hWndDlg,IDC_BPSIZE),L"1");
			Edit_SetText(GetDlgItem(hWndDlg,IDC_BPPID),L"-1");

			ComboBox_InsertString(GetDlgItem(hWndDlg,IDC_COMBO1),0,L"Software BP ( 0xCC / INT 3)");
			ComboBox_InsertString(GetDlgItem(hWndDlg,IDC_COMBO1),1,L"Memory BP ( PageGuard)");
			ComboBox_InsertString(GetDlgItem(hWndDlg,IDC_COMBO1),2,L"Hardware BP ( INT1 / Dr. Reg)");

			memset(&lvBPCol,0,sizeof(lvBPCol));                  
			lvBPCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			lvBPCol.pszText = L"Offset";                         
			lvBPCol.cx = 0x40;                               
			SendMessage(hwBreakPointLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"PID";
			lvBPCol.cx = 0x30;
			SendMessage(hwBreakPointLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"Type"; 
			lvBPCol.cx = 0x100;
			SendMessage(hwBreakPointLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"State";
			lvBPCol.cx = 0x35;
			SendMessage(hwBreakPointLC,LVM_INSERTCOLUMN,3,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"Keep";
			lvBPCol.cx = 0x25;
			SendMessage(hwBreakPointLC,LVM_INSERTCOLUMN,4,(LPARAM)&lvBPCol);
			return true;
		}
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_BPM_ADD:
			{
				DWORD dwType = 0,dwOffset = 0,dwPID = 0;
				wstringstream ss;
				PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
				int itemIndex;
				LVITEM LvItem;
				LVCOLUMN LvCol;

				BOOL bEnable = SendMessage(GetDlgItem(hWndDlg,IDC_BPENABLE),BM_GETCHECK,NULL,NULL);
				BOOL bKeep = SendMessage(GetDlgItem(hWndDlg,IDC_BPKEEP),BM_GETCHECK,NULL,NULL);

				memset(&LvItem,0,sizeof(LvItem));

				itemIndex = SendMessage(hwBreakPointLC,LVM_GETITEMCOUNT,0,0);

				// BP Offset
				SendMessage(GetDlgItem(hWndDlg,IDC_EDIT1),WM_GETTEXT,255,(LPARAM)sTemp);

				if(wcsstr(sTemp,L"::") != 0)
				{
					vector<wstring> vOffset = split(sTemp,L"::");
					PCHAR szTemp = (PCHAR)malloc(vOffset[1].length());
					wcstombs(szTemp,vOffset[1].c_str(),vOffset[1].length());
					szTemp[vOffset[1].length()] = '\0';
					ss.str(wstring());ss << hex << (DWORD)GetProcAddress(LoadLibrary(vOffset[0].c_str()),szTemp);
					wcscpy(sTemp,ss.str().c_str());
					//free(szTemp);
				}
				
				if(_wtol(sTemp) <= 0 || wcsstr(sTemp,L"0x") != 0x0)
				{
					MessageBox(hDlgBPManager,L"Invalid Offset!",L"Nanomite",MB_OK);
					return true;
				}
				LvItem.mask = LVIF_TEXT;
				LvItem.cchTextMax = 256;
				LvItem.iItem = itemIndex;
				LvItem.iSubItem = 0;
				LvItem.pszText = sTemp;
				SendMessage(hwBreakPointLC,LVM_INSERTITEM,0,(LPARAM)&LvItem);

				// BP - PID
				SendMessage(GetDlgItem(hWndDlg,IDC_BPPID),WM_GETTEXT,255,(LPARAM)sTemp);
				LvItem.pszText = sTemp;
				LvItem.iSubItem = 1;
				SendMessage(hwBreakPointLC,LVM_SETITEM,0,(LPARAM)&LvItem);

				// BP - TYPE
				SendMessage(GetDlgItem(hWndDlg,IDC_COMBO1),WM_GETTEXT,255,(LPARAM)sTemp);
				LvItem.pszText = sTemp;
				LvItem.iSubItem = 2;
				SendMessage(hwBreakPointLC,LVM_SETITEM,0,(LPARAM)&LvItem);

				// BP - State
				wcscpy(sTemp,(bEnable ? L"ENABLED" : L"DISABLED"));
				LvItem.pszText = sTemp;
				LvItem.iSubItem = 3;
				SendMessage(hwBreakPointLC,LVM_SETITEM,0,(LPARAM)&LvItem);

				// BP - Keep
				wcscpy(sTemp,(bKeep ? L"YES" : L"NO"));
				LvItem.pszText = sTemp;
				LvItem.iSubItem = 4;
				SendMessage(hwBreakPointLC,LVM_SETITEM,0,(LPARAM)&LvItem);
			
				for(int i = 0; i <= itemIndex;i++)
				{
					memset(&LvItem,0,sizeof(LvItem));
					LvItem.mask = LVIF_TEXT;
					LvItem.cchTextMax = 256;
					LvItem.iItem = i;
					LvItem.iSubItem = 0;
					LvItem.pszText = sTemp;
					SendMessage(hwBreakPointLC,LVM_GETITEMTEXT,i,(LPARAM)&LvItem);
					wstringstream ssdwOffset;ssdwOffset.str(wstring());ssdwOffset << sTemp;ssdwOffset >> hex >> dwOffset;
					
					LvItem.iSubItem = 1;
					SendMessage(hwBreakPointLC,LVM_GETITEMTEXT,i,(LPARAM)&LvItem);
					wstringstream ssdwPID;ssdwPID.str(wstring());ssdwPID << sTemp;ssdwPID >> hex >> dwPID;

					LvItem.iSubItem = 2;
					SendMessage(hwBreakPointLC,LVM_GETITEMTEXT,i,(LPARAM)&LvItem);

					if(wcscmp(sTemp,L"Software BP ( 0xCC / INT 3)") == 0)
						dwType = 0;
					else if(wcscmp(sTemp,L"Memory BP ( PageGuard)") == 0)
						dwType = 1;
					else if(wcscmp(sTemp,L"Hardware BP ( INT1 / Dr. Reg)") == 0)
						dwType = 2;

					bool bAdd = false;
					LvItem.iSubItem = 3;
					SendMessage(hwBreakPointLC,LVM_GETITEMTEXT,i,(LPARAM)&LvItem);
					if(wcscmp(sTemp,L"ENABLED") == 0)
						bAdd = true;
					else
						bAdd = false;

					LvItem.iSubItem = 4;
					SendMessage(hwBreakPointLC,LVM_GETITEMTEXT,i,(LPARAM)&LvItem);
					if(wcscmp(sTemp,L"YES") == 0)
						bKeep = true;
					else
						bKeep = false;

					if(bAdd)
						newDebugger.AddBreakpointToList(dwType,DR_EXECUTE,dwPID,dwOffset,0,bKeep);
				}

				MessageBox(hDlgBPManager,L"Breakpoint Saved!",L"Nanomite",MB_OK);

				free(sTemp);
				return true;
			}
		case ID_BPM_HIDE:
			ShowWindow(hDlgBPManager,SW_HIDE);
			return true;
		}
		break;
	case WM_CLOSE:
		ShowWindow(hDlgBPManager,SW_HIDE);
		return true;
	}
	return false;
}

LRESULT CALLBACK DetailInfoDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVITEM lvDETItem;
			LVCOLUMN lvDETCol;

			HWND hwPIDLC = GetDlgItem(hWndDlg,ID_DETINFO_PID);
			HWND hwTIDLC = GetDlgItem(hWndDlg,ID_DETINFO_TID);
			HWND hwEXCEPTIONLC = GetDlgItem(hWndDlg,ID_DETINFO_EXCEPTIONS);
			HWND hwDLLLC = GetDlgItem(hWndDlg,ID_DETINFO_DLLs);

			SendMessage(hwPIDLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwTIDLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwEXCEPTIONLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwDLLLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);


			//int (*dwOnPID)(DWORD dwPID,string sFile,DWORD dwExitCode,DWORD dwEP,bool bFound);
			memset(&lvDETCol,0,sizeof(lvDETCol));                  
			lvDETCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
			lvDETCol.pszText = L"PID";                         
			lvDETCol.cx = 0x50;                               
			SendMessage(hwPIDLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"EntryPoint";
			lvDETCol.cx = 0x8A;
			SendMessage(hwPIDLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"ExitCode"; 
			lvDETCol.cx = 0x50;
			SendMessage(hwPIDLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"FilePath";
			lvDETCol.cx = 0x140;
			SendMessage(hwPIDLC,LVM_INSERTCOLUMN,3,(LPARAM)&lvDETCol);


			// 	int (*dwOnThread)(DWORD dwPID,DWORD dwTID,DWORD dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
			memset(&lvDETCol,0,sizeof(lvDETCol));                  
			lvDETCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			lvDETCol.pszText = L"PID";                         
			lvDETCol.cx = 0x50;                               
			SendMessage(hwTIDLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"TID";
			lvDETCol.cx = 0x50;
			SendMessage(hwTIDLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"EntryPoint"; 
			lvDETCol.cx = 0x8A;
			SendMessage(hwTIDLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"Exit Code";
			lvDETCol.cx = 0x50;
			SendMessage(hwTIDLC,LVM_INSERTCOLUMN,3,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"State";
			lvDETCol.cx = 0x50;
			SendMessage(hwTIDLC,LVM_INSERTCOLUMN,4,(LPARAM)&lvDETCol);

			// 	int (*dwOnException)(string sFuncName,string sModName,DWORD dwOffset,DWORD dwExceptionCode,DWORD dwPID,DWORD dwTID);
			memset(&lvDETCol,0,sizeof(lvDETCol));                  
			lvDETCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			lvDETCol.pszText = L"Offset";                         
			lvDETCol.cx = 0x8A;                               
			SendMessage(hwEXCEPTIONLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"Exc. Code";
			lvDETCol.cx = 0x8A;
			SendMessage(hwEXCEPTIONLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"PID / TID"; 
			lvDETCol.cx = 0x99;
			SendMessage(hwEXCEPTIONLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"Module.Function";
			lvDETCol.cx = 0x100;
			SendMessage(hwEXCEPTIONLC,LVM_INSERTCOLUMN,3,(LPARAM)&lvDETCol);

			//int (*dwOnDll)(string sDLLPath,DWORD dwPID,DWORD dwEP,bool bLoaded);
			memset(&lvDETCol,0,sizeof(lvDETCol));                  
			lvDETCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			lvDETCol.pszText = L"PID";                         
			lvDETCol.cx = 0x50;                               
			SendMessage(hwDLLLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"Entry Point";
			lvDETCol.cx = 0x8A;
			SendMessage(hwDLLLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"State"; 
			lvDETCol.cx = 0x50;
			SendMessage(hwDLLLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvDETCol);
			lvDETCol.pszText = L"Path";
			lvDETCol.cx = 0x140;
			SendMessage(hwDLLLC,LVM_INSERTCOLUMN,3,(LPARAM)&lvDETCol);
			return true;
		}

	case WM_CLOSE:
		ShowWindow(hDlgDetInfo,SW_HIDE);
		return true;
	}
	return false;
}

LRESULT CALLBACK AttachToDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwAttachLC = GetDlgItem(hWndDlg,ID_PROCLIST);

	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN lvBPCol;

			SendMessage(hwAttachLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&lvBPCol,0,sizeof(lvBPCol));                  
			lvBPCol.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			lvBPCol.pszText = L"PID";                         
			lvBPCol.cx = 0x50;                               
			SendMessage(hwAttachLC,LVM_INSERTCOLUMN,0,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"Process";
			lvBPCol.cx = 0x99;
			SendMessage(hwAttachLC,LVM_INSERTCOLUMN,1,(LPARAM)&lvBPCol);
			lvBPCol.pszText = L"Path"; 
			lvBPCol.cx = 0x99;
			SendMessage(hwAttachLC,LVM_INSERTCOLUMN,2,(LPARAM)&lvBPCol);


			HANDLE hToolSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

			if( hToolSnapShot != INVALID_HANDLE_VALUE)
			{
				PROCESSENTRY32 pProcessEntry;
				pProcessEntry.dwSize = sizeof(PROCESSENTRY32);

				if(Process32First(hToolSnapShot,&pProcessEntry))
				{
					do 
					{
						HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,false,pProcessEntry.th32ProcessID);
						if(hProc != INVALID_HANDLE_VALUE)
						{
							int itemIndex = 0;
							PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
							LVITEM lvDETITEM;
							memset(&lvDETITEM,0,sizeof(lvDETITEM));

							itemIndex = SendMessage(hwAttachLC,LVM_GETITEMCOUNT,0,0);

							// PID
							wsprintf(sTemp,L"%d",pProcessEntry.th32ProcessID);
							lvDETITEM.mask = LVIF_TEXT;
							lvDETITEM.cchTextMax = 255;
							lvDETITEM.iItem = itemIndex;
							lvDETITEM.iSubItem = 0;
							lvDETITEM.pszText = sTemp;
							SendMessage(hwAttachLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

							// ProcessName
							wsprintf(sTemp,L"%s",pProcessEntry.szExeFile);
							lvDETITEM.iSubItem = 1;
							SendMessage(hwAttachLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

							MODULEENTRY32 pModEntry;
							pModEntry.dwSize = sizeof(MODULEENTRY32);

							HANDLE hModules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pProcessEntry.th32ProcessID);

							if(hModules != INVALID_HANDLE_VALUE && Module32First(hModules,&pModEntry))
							{
								do 
								{
									if(pModEntry.th32ProcessID == pProcessEntry.th32ProcessID)
									{
										wsprintf(sTemp,L"%s",pModEntry.szExePath);
										break;
									}
								} while (Module32Next(hModules,&pModEntry));
							}

							// ProcessPath
							lvDETITEM.iSubItem = 2;
							SendMessage(hwAttachLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
						}
					} while (Process32Next(hToolSnapShot,&pProcessEntry));
				}
			}
			return true;
		}
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_ATTACH:
			{
				DWORD dwPID = 0;
				PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
				LVITEM lvItem;
				memset(&lvItem,0,sizeof(lvItem));

				int iPos = ListView_GetNextItem(hwAttachLC, -1, LVNI_SELECTED | LVNI_FOCUSED);

				lvItem.mask = LVIF_TEXT;
				lvItem.cchTextMax = 255;
				lvItem.iItem = iPos;
				lvItem.iSubItem = 0;
				lvItem.pszText = sTemp;
				SendMessage(hwAttachLC,LVM_GETITEMTEXT,iPos,(LPARAM)&lvItem);
				wstringstream ssdwOffset;ssdwOffset.str(wstring());ssdwOffset << sTemp;ssdwOffset >> dwPID;
				free(sTemp);

				if(dwPID != 0)
				{
					newDebugger.AttachToProcess(dwPID);

					ListBox_ResetContent(GetDlgItem(hDlgMain,IDC_LIST1));
					ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_PID));
					ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_TID));
					ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_DLLs));
					ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_EXCEPTIONS));
					ListView_DeleteAllItems(GetDlgItem(hDlgMain,IDC_LIST2));
					ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_DISASS));

					StartDebugging();
					UpdateStateLable(0x1);
					EndDialog(hWndDlg,0);
					return true;
				}
				else
				{
					EndDialog(hWndDlg,0);
					return true;
				}
			}
		case ID_CLOSE:
			ListView_DeleteAllItems(hwAttachLC);
			EndDialog(hWndDlg,0);
			return true;
		}
		break;

	case WM_CLOSE:
		ListView_DeleteAllItems(hwAttachLC);
		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK MemMapDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			hDlgMemMap = hWndDlg;
			HWND hwMemMap = GetDlgItem(hDlgMemMap,IDC_MEM);
			SendMessage(hwMemMap,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Base Address";                         
			LvCol.cx = 0x8A;                               
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Size";
			LvCol.cx = 0x50;
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Module"; 
			LvCol.cx = 0x100;
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"Type";
			LvCol.cx = 0x50;
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);
			LvCol.pszText = L"Access"; 
			LvCol.cx = 0x50; 
			SendMessage(hwMemMap,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol);

			int iForEntry = ((iMemMapPID == newDebugger.PIDs.size()) ? 0 : iMemMapPID); 
			int iForEnd = ((iMemMapPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iMemMapPID);

			PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
			PTCHAR sTemp2 = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

			LVITEM lvDETITEM;

			MODULEENTRY32 pModEntry;
			pModEntry.dwSize = sizeof(MODULEENTRY32);
			MEMORY_BASIC_INFORMATION mbi;

			for(int i = iForEntry; i < iForEnd;i++)
			{
				DWORD64 dwAddress = NULL;
				while(VirtualQueryEx(newDebugger.PIDs[i].hProc,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
				{
					memset(&lvDETITEM,0,sizeof(lvDETITEM));
					int iItemIndex = ListView_GetItemCount(hwMemMap);

					// PID
					wsprintf(sTemp,L"0x%08X",newDebugger.PIDs[i].dwPID);
					lvDETITEM.mask = LVIF_TEXT;
					lvDETITEM.cchTextMax = 255;
					lvDETITEM.iItem = iItemIndex;
					lvDETITEM.iSubItem = 0;
					lvDETITEM.pszText = sTemp;
					SendMessage(hwMemMap,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

					// Base Address
#ifdef _AMD64_
					wsprintf(sTemp,L"0x%016I64X",mbi.BaseAddress);
#else
					wsprintf(sTemp,L"0x%016X",mbi.BaseAddress);
#endif
					lvDETITEM.iSubItem = 1;
					SendMessage(hwMemMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					// Size
					wsprintf(sTemp,L"0x%08X",mbi.RegionSize);
					lvDETITEM.iSubItem = 2;
					SendMessage(hwMemMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					// Path
					int iModPos = NULL,
						iModLen = NULL;

					memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
					memset(sTemp2,0,MAX_PATH * sizeof(TCHAR));
					GetMappedFileName(newDebugger.PIDs[i].hProc,(LPVOID)dwAddress,sTemp2,MAX_PATH * sizeof(TCHAR));

					iModLen = wcslen(sTemp2);
					if(iModLen > 0)
					{
						for(int i = iModLen; i > 0 ; i--)
						{
							if(sTemp2[i] == '\\')
							{
								iModPos = i;
								break;
							}
						}
						
						memcpy(sTemp,(LPVOID)&sTemp2[iModPos + 1],(iModLen - iModPos) * sizeof(TCHAR));

						lvDETITEM.iSubItem = 3;
						SendMessage(hwMemMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);					
					}

					// Mem Type
					switch (mbi.State)
					{
					case MEM_FREE:			wsprintf(sTemp,L"%s",L"Free");		break;
					case MEM_RESERVE:       wsprintf(sTemp,L"%s",L"Reserve");	break;
					case MEM_COMMIT:
						switch (mbi.Type)
						{
						case MEM_FREE:		wsprintf(sTemp,L"%s",L"Free");     break;
						case MEM_RESERVE:   wsprintf(sTemp,L"%s",L"Reserve");  break;
						case MEM_IMAGE:		wsprintf(sTemp,L"%s",L"Image");    break;
						case MEM_MAPPED:    wsprintf(sTemp,L"%s",L"Mapped");   break;
						case MEM_PRIVATE:   wsprintf(sTemp,L"%s",L"Private");  break;
						}
						break;
					}
					lvDETITEM.iSubItem = 4;
					SendMessage(hwMemMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					// Access
					wsprintf(sTemp,L"%s",L"Unknown");
					if(mbi.State == MEM_FREE) mbi.Protect = PAGE_NOACCESS;
					if(mbi.State == MEM_RESERVE) mbi.Protect = mbi.AllocationProtect;
					switch (mbi.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE))
					{
					case PAGE_READONLY:          wsprintf(sTemp,L"%s",L"-R--"); break;
					case PAGE_READWRITE:         wsprintf(sTemp,L"%s",L"-RW-"); break;
					case PAGE_WRITECOPY:         wsprintf(sTemp,L"%s",L"-RWC"); break;
					case PAGE_EXECUTE:           wsprintf(sTemp,L"%s",L"E---"); break;
					case PAGE_EXECUTE_READ:      wsprintf(sTemp,L"%s",L"ER--"); break;
					case PAGE_EXECUTE_READWRITE: wsprintf(sTemp,L"%s",L"ERW-"); break;
					case PAGE_EXECUTE_WRITECOPY: wsprintf(sTemp,L"%s",L"ERWC"); break; 
					case PAGE_NOACCESS:          wsprintf(sTemp,L"%s",L"----"); break;
					}
					lvDETITEM.iSubItem = 5;
					SendMessage(hwMemMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					dwAddress += mbi.RegionSize;
				}
			}
			free(sTemp2);
			free(sTemp);
			return true;
		}
	case WM_CLOSE:
		ListView_DeleteAllItems(GetDlgItem(hDlgMemMap,IDC_MEM));
		EndDialog(hDlgMemMap,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK HeapMapDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	hDlgHeapMap = hWndDlg;
	HWND hwHeapMap = GetDlgItem(hDlgHeapMap,IDC_HEAPMAP);
	HWND hwHeapMap2 = GetDlgItem(hDlgHeapMap,IDC_HEAPMAP2);
	
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			SendMessage(hwHeapMap,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwHeapMap2,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			
			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Offset";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwHeapMap2,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Hex";                         
			LvCol.cx = 0x120;                               
			SendMessage(hwHeapMap2,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Ascii";
			LvCol.cx = 0x99;
			SendMessage(hwHeapMap2,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);

			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Heap ID";
			LvCol.cx = 0x50;
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Base Address";
			LvCol.cx = 0x60;
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Size";
			LvCol.cx = 0x50;
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"Block Count"; 
			LvCol.cx = 0x60;
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);
			LvCol.pszText = L"Flags";
			LvCol.cx = 0x50;
			SendMessage(hwHeapMap,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol);
			
			int iForEntry = ((iHeapMapPID == newDebugger.PIDs.size()) ? 0 : iHeapMapPID); 
			int iForEnd = ((iHeapMapPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iHeapMapPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
				HEAPLIST32 heapList;
				int iHeapCount = 0;
				HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST,newDebugger.PIDs[i].dwPID);

				heapList.dwSize = sizeof(HEAPLIST32);

				if (hHeapSnap != INVALID_HANDLE_VALUE)
				{
					if(Heap32ListFirst(hHeapSnap,&heapList))
					{
						do
						{
							HEAPENTRY32 he;
							ZeroMemory(&he, sizeof(HEAPENTRY32));
							he.dwSize = sizeof(HEAPENTRY32);

							if( Heap32First(&he,newDebugger.PIDs[i].dwPID,heapList.th32HeapID))
							{
								do
								{
									LVITEM lvDETITEM;
									memset(&lvDETITEM,0,sizeof(lvDETITEM));
									PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
									int iItemIndex = ListView_GetItemCount(hwHeapMap);

									// PID
									wsprintf(sTemp,L"0x%08X",newDebugger.PIDs[i].dwPID);
									lvDETITEM.mask = LVIF_TEXT;
									lvDETITEM.cchTextMax = 255;
									lvDETITEM.iItem = iItemIndex;
									lvDETITEM.iSubItem = 0;
									lvDETITEM.pszText = sTemp;
									SendMessage(hwHeapMap,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

									// Heap ID
									wsprintf(sTemp,L"0x%08X",he.th32HeapID);
									lvDETITEM.iSubItem = 1;
									SendMessage(hwHeapMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
									
									// Base Address
									wsprintf(sTemp,L"0x%08X",he.dwAddress);
									lvDETITEM.iSubItem = 2;
									SendMessage(hwHeapMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

									// Allocated Size
									wsprintf(sTemp,L"0x%08X",he.dwBlockSize);
									lvDETITEM.iSubItem = 3;
									SendMessage(hwHeapMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

									// Block Count
									wsprintf(sTemp,L"%d",++iHeapCount);
									lvDETITEM.iSubItem = 4;
									SendMessage(hwHeapMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

									// Flags
									wsprintf(sTemp,L"0x%08X",he.dwFlags);
									lvDETITEM.iSubItem = 5;
									SendMessage(hwHeapMap,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
									he.dwSize = sizeof(HEAPENTRY32);
									free(sTemp);
								} while( Heap32Next(&he) );
							}
							heapList.dwSize = sizeof(HEAPLIST32);
						} while (Heap32ListNext(hHeapSnap,&heapList));
					}
					CloseHandle(hHeapSnap);
				}
			}
			return true;
		}

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
				{
				case NM_CLICK:
					{
						if(((LPNMHDR)lParam)->hwndFrom != hwHeapMap)
							return true;

						DWORD dwOffset = NULL,
							dwSize = NULL,
							dwPID = NULL;
						LPNMITEMACTIVATE lpItem = (LPNMITEMACTIVATE) lParam;
						int iITem = lpItem->iItem;
						PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

						LVITEM lvItem;
						memset(&lvItem,0,sizeof(lvItem));

						lvItem.mask = LVIF_TEXT;
						lvItem.cchTextMax = 255;
						lvItem.iItem = iITem;
						lvItem.iSubItem = 0;
						lvItem.pszText = sTemp;
						SendMessage(hwHeapMap,LVM_GETITEMTEXT,iITem,(LPARAM)&lvItem);
						wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwPID;

						lvItem.iSubItem = 2;
						SendMessage(hwHeapMap,LVM_GETITEMTEXT,iITem,(LPARAM)&lvItem);
						wstringstream ss2;ss2.str(wstring());ss2 << sTemp;ss2 >> hex >> dwOffset;

						lvItem.iSubItem = 3;
						SendMessage(hwHeapMap,LVM_GETITEMTEXT,iITem,(LPARAM)&lvItem);
						wstringstream ss3;ss3.str(wstring());ss3 << sTemp;ss3 >> hex >> dwSize;
						free(sTemp);

						PrintMemToHexView(dwPID,dwOffset,dwSize,hwHeapMap2);
						return true;
					}
			}
			return true;
		}
	case WM_CLOSE:
		ListView_DeleteAllItems(hwHeapMap2);
		ListView_DeleteAllItems(hwHeapMap);
		EndDialog(hDlgHeapMap,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK AboutDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			PTCHAR sTemp = (PTCHAR)malloc(sizeof(TCHAR) * 255);

			wsprintf(sTemp,L"\t\t\tNanomite v 0.1\r\n\r\nCoded by: Zer0Flag\r\n\r\nCoded in: C++\r\n\r\nGreetz: C3lt1c, s0rb, aSh, #coderz.cc and all others ;)\r\n\r\nContact:\tzer0fl4g@googlemail.com\r\n");
			Edit_SetText(GetDlgItem(hWndDlg,IDC_ABOUT),sTemp);

			free(sTemp);
			return true;
		}
	case WM_CLOSE:
		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK PeViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwIDH = GetDlgItem(hWndDlg,IDC_DOS_HEADER),
		hwIFH = GetDlgItem(hWndDlg,IDC_FILE_HEADER),
		hwIOH = GetDlgItem(hWndDlg,IDC_OPTIONAL_HEADER),
		hwFI = GetDlgItem(hWndDlg,IDC_FileImports),
		hwFS = GetDlgItem(hWndDlg,IDC_FileSections),
		hwTLSDIR = GetDlgItem(hWndDlg,IDC_TLSDir),
		hwDATADIR = GetDlgItem(hWndDlg,IDC_DATADIRS),
		hwSTATE = GetDlgItem(hWndDlg,IDC_PEViewState);

	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			SendMessage(hwIDH,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwIFH,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwIOH,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwFI,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwFS,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwTLSDIR,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwDATADIR,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"Name";                         
			LvCol.cx = 0x99;                               
			SendMessage(hwIDH,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Data";                         
			LvCol.cx = 0x53;                               
			SendMessage(hwIDH,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

			LvCol.pszText = L"Name";                         
			LvCol.cx = 0x99;                               
			SendMessage(hwIFH,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Data";                         
			LvCol.cx = 0x53;                               
			SendMessage(hwIFH,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

			LvCol.pszText = L"Name";                         
			LvCol.cx = 0x99;                               
			SendMessage(hwIOH,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Data";                         
			LvCol.cx = 0x57;                               
			SendMessage(hwIOH,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

			LvCol.pszText = L"Name";                         
			LvCol.cx = 0x35;                               
			SendMessage(hwFS,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Virt. Size";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwFS,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);			
			LvCol.pszText = L"Virt. Adr.";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwFS,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"SizeOfRawData";                         
			LvCol.cx = 0x56;   
			SendMessage(hwFS,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"PointerToRawData";                         
			LvCol.cx = 0x56;  
			SendMessage(hwFS,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);
			LvCol.pszText = L"Characteristics";                         
			LvCol.cx = 0x56;                               
			SendMessage(hwFS,LVM_INSERTCOLUMN,5,(LPARAM)&LvCol);

			LvCol.pszText = L"Module";                         
			LvCol.cx = 0x99;                               
			SendMessage(hwFI,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"API";                         
			LvCol.cx = 0x99;                               
			SendMessage(hwFI,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Offset";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwFI,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);

			LvCol.pszText = L"Adr. Callbacks";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwTLSDIR,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Start Adr.";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwTLSDIR,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"End Adr.";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwTLSDIR,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);


			LvCol.pszText = L"Nr";                         
			LvCol.cx = 0x30;                               
			SendMessage(hwDATADIR,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Virtual Adr.";                         
			LvCol.cx = 0x45;                               
			SendMessage(hwDATADIR,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Size";                         
			LvCol.cx = 0x42;                               
			SendMessage(hwDATADIR,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);

			int iForEntry = ((iPeViewPID == newDebugger.PIDs.size()) ? 0 : iPeViewPID); 
			int iForEnd = ((iPeViewPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iPeViewPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
				printPEView(hWndDlg,i);
			}
			return true;
		}
	
	case WM_CLOSE:
		ListView_DeleteAllItems(hwIDH);
		ListView_DeleteAllItems(hwIFH);
		ListView_DeleteAllItems(hwIOH);
		ListView_DeleteAllItems(hwFI);
		ListView_DeleteAllItems(hwFS);
		ListView_DeleteAllItems(hwTLSDIR);
		ListView_DeleteAllItems(hwDATADIR);
		Static_SetText(hwSTATE,L"");

		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK StringViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			HWND hwStringList = GetDlgItem(hWndDlg,IDC_STRINGS);

			SendMessage(hwStringList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwStringList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Offset";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwStringList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"String - Ascii";                         
			LvCol.cx = 0x200;                               
			SendMessage(hwStringList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);


			int iForEntry = ((iStringViewPID == newDebugger.PIDs.size()) ? 0 : iStringViewPID); 
			int iForEnd = ((iStringViewPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iStringViewPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
				bool bNotEndOfFile = true;
				PTCHAR sTemp = (PTCHAR)malloc(255 * sizeof(WCHAR));
				wifstream inputFile;
				
				inputFile.open(newDebugger.PIDs[i].sFileName,ifstream::binary);

				if(!inputFile.is_open())
				{
					MessageBox(hWndDlg,newDebugger.PIDs[i].sFileName,L"Error opening File!",MB_OKCANCEL);
					bNotEndOfFile = false;
				}

				while(bNotEndOfFile && inputFile.good())
				{
					wstringstream sTempString;
					TCHAR sT;
					inputFile.get(sT);

					while(inputFile.good())
					{
						if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
							((int)sT >= 0x61 && (int)sT <= 0x7a)	||
							((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
							((int)sT == 0x20))
							sTempString << sT;
						else
							break;
						inputFile.get(sT);
					}

					if(sTempString.str().length() >= 2)
					{
						LVITEM lvDETITEM;
						memset(&lvDETITEM,0,sizeof(lvDETITEM));

						int iItemIndex = ListView_GetItemCount(hwStringList);
						
						// PID
						wsprintf(sTemp,L"0x%08X",newDebugger.PIDs[i].dwPID);
						lvDETITEM.mask = LVIF_TEXT;
						lvDETITEM.cchTextMax = 255;
						lvDETITEM.iItem = iItemIndex;
						lvDETITEM.iSubItem = 0;
						lvDETITEM.pszText = sTemp;
						SendMessage(hwStringList,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

						// Offset
						wsprintf(sTemp,L"%08X",inputFile.tellg());
						lvDETITEM.iSubItem = 1;
						SendMessage(hwStringList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

						// String
						wsprintf(sTemp,L"%s",sTempString.str().c_str());
						lvDETITEM.iSubItem = 2;
						SendMessage(hwStringList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					}
				}
				free(sTemp);
			}
			return true;
		}

	case WM_CLOSE:
		ListView_DeleteAllItems(GetDlgItem(hWndDlg,IDC_STRINGS));
		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK WndListDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			HWND hwWndList = GetDlgItem(hDlgWndList = hWndDlg,IDC_WNDLIST);
			LVCOLUMN LvCol;

			SendMessage(hwWndList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwWndList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Window Name";                         
			LvCol.cx = 0x100;                               
			SendMessage(hwWndList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Is Visible";
			LvCol.cx = 0x50;
			SendMessage(hwWndList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"hWnd"; 
			LvCol.cx = 0x50;
			SendMessage(hwWndList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"File Path"; 
			LvCol.cx = 0x160; 
			SendMessage(hwWndList,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);

			int iForEntry = ((iWndListPID == newDebugger.PIDs.size()) ? 0 : iWndListPID); 
			int iForEnd = ((iWndListPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iWndListPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
				EnumWindows((WNDENUMPROC)EnumWindowCallBack,(LPARAM)newDebugger.PIDs[i].dwPID);
			}
			return true;
		}

	case WM_CLOSE:
		ListView_DeleteAllItems(GetDlgItem(hDlgWndList = hWndDlg,IDC_WNDLIST));
		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK HandleViewDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwHandleList = GetDlgItem(hWndDlg,IDC_HANDLELC);

	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQuerySystemInformation");
			_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtDuplicateObject");
			_NtQueryObject NtQueryObject = (_NtQueryObject)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQueryObject");

			LVCOLUMN LvCol;

			SendMessage(hwHandleList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwHandleList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Handle";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwHandleList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Type";
			LvCol.cx = 0x50;
			SendMessage(hwHandleList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Name"; 
			LvCol.cx = 0x195;
			SendMessage(hwHandleList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);

			int iForEntry = ((iHandleViewPID == newDebugger.PIDs.size()) ? 0 : iHandleViewPID); 
			int iForEnd = ((iHandleViewPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iHandleViewPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
				NTSTATUS status;
				ULONG handleInfoSize = 0x10000;
				DWORD dwPID = newDebugger.PIDs[i].dwPID;
				HANDLE hProc = newDebugger.PIDs[i].hProc;

				PSYSTEM_HANDLE_INFORMATION handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(handleInfoSize);

				while ((status = NtQuerySystemInformation(SystemHandleInformation,handleInfo,handleInfoSize,NULL)) == STATUS_INFO_LENGTH_MISMATCH)
					handleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(handleInfo, handleInfoSize *= 2);

				if (!NT_SUCCESS(status))
				{
					MessageBox(hWndDlg,L"NtQuerySystemInformation failed!",L"Nanomite",MB_OK);
					return false;
				}

				for (int iCount = 0; iCount < handleInfo->HandleCount; iCount++)
				{
					SYSTEM_HANDLE handle = handleInfo->Handles[iCount];
					HANDLE dupHandle = NULL;
					POBJECT_TYPE_INFORMATION objectTypeInfo;
					PVOID objectNameInfo;
					UNICODE_STRING objectName,
						objectTypeName;
					ULONG returnLength;
					TCHAR *sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR)),
						*sTemp2 = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
					BOOL bWideBool = false;

					if (handle.ProcessId != dwPID)
						continue;

					if (!NT_SUCCESS(NtDuplicateObject(hProc,(HANDLE)handle.Handle,GetCurrentProcess(),&dupHandle,0,0,0)))
						continue;

					objectTypeInfo = (POBJECT_TYPE_INFORMATION)malloc(0x1000);
					if (!NT_SUCCESS(NtQueryObject(dupHandle,ObjectTypeInformation,objectTypeInfo,0x1000,NULL)))
					{
						CloseHandle(dupHandle);
						continue;
					}
					//WideCharToMultiByte(CP_ACP,NULL,objectTypeInfo->Name.Buffer,-1,sTemp2,objectTypeInfo->Name.MaximumLength,NULL,&bWideBool);
					
					if (handle.GrantedAccess == 0x0012019f)
					{
						InsertHandleIntoLC(hwHandleList,dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"Didn´t get name");
						free(objectTypeInfo);
						CloseHandle(dupHandle);
						continue;
					}

					objectNameInfo = malloc(0x1000);
					if (!NT_SUCCESS(NtQueryObject(dupHandle,ObjectNameInformation,objectNameInfo,0x1000,&returnLength)))
					{
						objectNameInfo = realloc(objectNameInfo, returnLength);
						if (!NT_SUCCESS(NtQueryObject(dupHandle,ObjectNameInformation,objectNameInfo,returnLength,NULL)))
						{
							InsertHandleIntoLC(hwHandleList,dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"Couldn´t get name");
							free(objectTypeInfo);
							free(objectNameInfo);
							CloseHandle(dupHandle);
							continue;
						}
					}

					objectName = *(PUNICODE_STRING)objectNameInfo;

					if (objectName.Length)
					{
						//WideCharToMultiByte(CP_ACP,NULL,objectName.Buffer,-1,sTemp,objectName.MaximumLength,NULL,&bWideBool);
						InsertHandleIntoLC(hwHandleList,dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,objectName.Buffer);
					}
					else
						InsertHandleIntoLC(hwHandleList,dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"unnamed");

					free(objectTypeInfo);
					free(objectNameInfo);
					free(sTemp);
					free(sTemp2);
					CloseHandle(dupHandle);
				}

				free(handleInfo);
			}
			return true;
		}

	case WM_CLOSE:
		ListView_DeleteAllItems(hwHandleList);
		EndDialog(hWndDlg,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK RessourceDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			hDlgResList = hWndDlg;
			HWND hwRestList = GetDlgItem(hDlgResList,IDC_RES);
			LVCOLUMN LvCol;
			SendMessage(hwRestList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(hwRestList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwRestList,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"File Path";                         
			LvCol.cx = 0x120;                               
			SendMessage(hwRestList,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
			LvCol.pszText = L"Res Type";
			LvCol.cx = 0x50;
			SendMessage(hwRestList,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
			LvCol.pszText = L"Res Name";
			LvCol.cx = 0x100;
			SendMessage(hwRestList,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);
			LvCol.pszText = L"Res Size";
			LvCol.cx = 0x50;
			SendMessage(hwRestList,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol);


			int iForEntry = ((iResListPID == newDebugger.PIDs.size()) ? 0 : iResListPID); 
			int iForEnd = ((iResListPID != newDebugger.PIDs.size()) ? (iForEntry + 1) : iResListPID);

			for(int i = iForEntry; i < iForEnd;i++)
			{
#ifndef _AMD64_
				HMODULE hFile = LoadLibrary(newDebugger.PIDs[i].sFileName);
				EnumResourceTypes(hFile,EnumResTypes,NULL);
				FreeLibrary(hFile);
#endif
			}
			return true;
		}

	case WM_CLOSE:
		ListView_DeleteAllItems(GetDlgItem(hDlgResList,IDC_RES));
		EndDialog(hDlgResList,0);
		return true;
	}
	return false;
}

LRESULT CALLBACK DebugStringDLGProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			LVCOLUMN LvCol;
			HWND hwDebugStringLC = GetDlgItem(hWndDlg,IDC_DBGSTR);
			SendMessage(hwDebugStringLC,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

			memset(&LvCol,0,sizeof(LvCol));                  
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;                                     
			LvCol.pszText = L"PID";                         
			LvCol.cx = 0x50;                               
			SendMessage(hwDebugStringLC,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);
			LvCol.pszText = L"Debug String";                         
			LvCol.cx = 0x200;        
			SendMessage(hwDebugStringLC,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);

			break;
		}
	case WM_CLOSE:
		ShowWindow(hWndDlg,SW_HIDE);
		return true;
	}
	return false;
}

void DebuggingLoop(clsDebugger *tempDebugger)
{
	BOOL bDebugIt = true;
	tempDebugger->StartDebugging();

	while(bDebugIt)
	{
		WaitForSingleObject(tempDebugger->hDebuggingHandle,INFINITE);

		if(!tempDebugger->GetDebuggingState())
		{
			bDebugIt = false;
		}
		else
		{
			SendMessage(hwLBCallStack,LVM_DELETEALLITEMS,0,0);
			SendMessage(GetDlgItem(hDlgMain,ID_DISASS),LVM_DELETEALLITEMS,0,0);

			tempDebugger->ShowCallStack();
			LoadRegView();
#ifdef _AMD64_
			BOOL bIsWOW64 = false;
			HANDLE hProcess = NULL;

			for(size_t i = 0; i < tempDebugger->PIDs.size(); i++)
				if(tempDebugger->PIDs[i].dwPID == tempDebugger->GetCurrentPID())
					hProcess = tempDebugger->PIDs[i].hProc;

			IsWow64Process(hProcess,&bIsWOW64);
			if(bIsWOW64)
			{
				LoadStackView(newDebugger.wowProcessContext.Esp,4);
				LoadDisAssView(newDebugger.wowProcessContext.Eip);
			}
			else
			{
				LoadStackView(newDebugger.ProcessContext.Rsp,8);
				LoadDisAssView(newDebugger.ProcessContext.Rip);
			}
#else
			LoadStackView(newDebugger.ProcessContext.Esp,4);
			LoadDisAssView(newDebugger.ProcessContext.Eip);
#endif			
			UpdateStateLable(0x2);
		}
	}
	UpdateStateLable(0x3);
}

bool MenuLoadNewFile()
{
	TCHAR cFile[MAX_PATH];

	OPENFILENAME ofn; 
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	ofn.hwndOwner = NULL;
	ofn.lpstrFile = cFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(cFile);
	ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn) == true)
		newDebugger.SetTarget(ofn.lpstrFile);
	else
	{
		MessageBox(NULL,L"No valid file!",L"Nanomite",MB_OK);
		return false;
	}
	return true;
}

void StartDebugging()
{
	HANDLE hThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)&DebuggingLoop,&newDebugger,NULL,NULL);
}

void LoadCallBacks()
{
	newDebugger.dwOnCallStack = &OnCallStack;
	newDebugger.dwOnDbgString = &OnDbgString;
	newDebugger.dwOnDll = &OnDll;
	newDebugger.dwOnLog = &OnLog;
	newDebugger.dwOnThread = &OnThread;
	newDebugger.dwOnException = &OnException;
	newDebugger.dwOnPID = &OnPID;
}

int OnThread(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound)
{
	HWND hwTIDLC = GetDlgItem(hDlgDetInfo,ID_DETINFO_TID);
	wstringstream sLog;
	int itemIndex = 0;
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));
	memset(tcLogging,0,MAX_PATH * sizeof(TCHAR));

	itemIndex = SendMessage(hwTIDLC,LVM_GETITEMCOUNT,0,0);

	if(!bFound)
	{
		// PID
		wsprintf(tcLogging,L"0x%08X",dwPID);
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 256;
		lvDETITEM.iItem = itemIndex;
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = tcLogging;
		SendMessage(hwTIDLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// TID
		wsprintf(tcLogging,L"0x%08X",dwTID);
		lvDETITEM.iSubItem = 1;
		SendMessage(hwTIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// EP
		wsprintf(tcLogging,L"0x%016I64X",dwEP);
		lvDETITEM.iSubItem = 2;
		SendMessage(hwTIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// ExitCode
		//wsprintf(sTemp,"0x%08X",dwExitCode);
		//lvDETITEM.iSubItem = 3;
		//SendMessage(hwTIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// State
		wsprintf(tcLogging,L"%s",L"Running");
		lvDETITEM.iSubItem = 4;
		SendMessage(hwTIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	}
	else
	{
		for(int i = 0; i <= itemIndex;i++)
		{
			DWORD dwThread_PID = 0,dwThread_TID = 0;
			LVITEM lvItem;
			memset(&lvItem,0,sizeof(lvItem));

			lvItem.mask = LVIF_TEXT;
			lvItem.cchTextMax = 256;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = tcLogging;
			SendMessage(hwTIDLC,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
			wstringstream ss;ss.str(wstring());ss << tcLogging;ss >> hex >> dwThread_PID;

			lvItem.iSubItem = 1;
			SendMessage(hwTIDLC,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
			wstringstream sd;sd.str(wstring());sd << tcLogging;sd >> hex >> dwThread_TID;
			
			if(dwPID == dwThread_PID && dwTID == dwThread_TID)
			{
				// Change ExitCode
				wsprintf(tcLogging,L"0x%08X",dwExitCode);
				lvItem.iSubItem = 3;
				SendMessage(hwTIDLC,LVM_SETITEMTEXT,i,(LPARAM)&lvItem);

				// Change State
				wsprintf(tcLogging,L"%s",L"Terminated");
				lvItem.iSubItem = 4;
				SendMessage(hwTIDLC,LVM_SETITEM,i,(LPARAM)&lvItem);
			}
		}
	}

	UpdateStateLable(0x1);
	return 0;
}

int OnPID(DWORD dwPID,wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound)
{
	HWND hwPIDLC = GetDlgItem(hDlgDetInfo,ID_DETINFO_PID);
	
	memset(tcLogging,0,MAX_PATH * sizeof(TCHAR));
	int itemIndex = SendMessage(hwPIDLC,LVM_GETITEMCOUNT,0,0);
	if(!bFound)
	{
		wstringstream sLog;
		LVITEM lvDETITEM;
		memset(&lvDETITEM,0,sizeof(lvDETITEM));

		// PID
		wsprintf(tcLogging,L"0x%08X",dwPID);
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 256;
		lvDETITEM.iItem = itemIndex;
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = tcLogging;
		SendMessage(hwPIDLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// EntryPoint
		wsprintf(tcLogging,L"0x%016I64X",dwEP);
		lvDETITEM.iSubItem = 1;
		SendMessage(hwPIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		//// ExitCode
		//wsprintf(tcLogging,"0x%08X",dwExitCode);
		//lvDETITEM.iSubItem = 2;
		//SendMessage(hwPIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// File
		wsprintf(tcLogging,L"%s",sFile.c_str());
		lvDETITEM.iSubItem = 3;
		SendMessage(hwPIDLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	}
	else
	{
		for(int i = 0; i <= itemIndex;i++)
		{
			DWORD dwThread_PID = 0;
			LVITEM lvItem;
			memset(&lvItem,0,sizeof(lvItem));

			lvItem.mask = LVIF_TEXT;
			lvItem.cchTextMax = 256;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = tcLogging;
			SendMessage(hwPIDLC,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
			wstringstream ss;ss.str(wstring());ss << tcLogging;ss >> hex >> dwThread_PID;

			if(dwPID == dwThread_PID)
			{
				// Change ExitCode
				wsprintf(tcLogging,L"0x%08X",dwExitCode);
				lvItem.iSubItem = 2;
				SendMessage(hwPIDLC,LVM_SETITEMTEXT,i,(LPARAM)&lvItem);
			}
		}
	}

	UpdateStateLable(0x1);
	return 0;
}

int OnException(wstring sFuncName,wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	HWND hwExceptionLC = GetDlgItem(hDlgDetInfo,ID_DETINFO_EXCEPTIONS);
	dwExceptionCount++;

	wstringstream sLog;
	int itemIndex = 0;
	PTCHAR sTemp = (PTCHAR)malloc(256);
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));

	itemIndex = SendMessage(hwExceptionLC,LVM_GETITEMCOUNT,0,0);

	// Offset
	wsprintf(sTemp,L"0x%016I64X",dwOffset);
	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = 256;
	lvDETITEM.iItem = itemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	SendMessage(hwExceptionLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

	// Exc. Code
	wsprintf(sTemp,L"0x%016I64X",dwExceptionCode);
	lvDETITEM.iSubItem = 1;
	SendMessage(hwExceptionLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	// PID / TID
	wsprintf(sTemp,L"0x%08X / 0x%08X",dwPID,dwTID);
	lvDETITEM.iSubItem = 2;
	SendMessage(hwExceptionLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	// Module.Function
	wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
	lvDETITEM.iSubItem = 3;
	SendMessage(hwExceptionLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	UpdateStateLable(0x1);
	return 0;
}

int OnDbgString(wstring sMessage,DWORD dwPID)
{
	HWND hwDebugStringLC = GetDlgItem(hDlgDbgStringInfo,IDC_DBGSTR);
	int itemIndex = 0;
	PTCHAR sTemp = (PTCHAR)malloc((wcslen(sMessage.c_str()) + 20) * sizeof(TCHAR));
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));

	itemIndex = SendMessage(hwDebugStringLC,LVM_GETITEMCOUNT,0,0);

	// PID
	wsprintf(sTemp,L"0x%08X",dwPID);
	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = wcslen(sMessage.c_str()) + 20;
	lvDETITEM.iItem = itemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	SendMessage(hwDebugStringLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

	// DebugString
	memcpy(sTemp,sMessage.c_str(),(wcslen(sMessage.c_str()) + 20) * sizeof(TCHAR));
	lvDETITEM.iSubItem = 1;
	SendMessage(hwDebugStringLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	free(sTemp);
	return 0;
}

int OnLog(tm timeInfo,wstring sLog)
{
	HWND hwLogLC = GetDlgItem(hDlgMain,IDC_LOG);
	int itemIndex = SendMessage(hwLogLC,LVM_GETITEMCOUNT,0,0);
	PTCHAR sTemp = (PTCHAR)malloc((wcslen(sLog.c_str()) + 20) * sizeof(TCHAR));
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));
	
	// time stamp
	wsprintf(sTemp,L"[%i:%i:%i]",timeInfo.tm_hour,timeInfo.tm_min,timeInfo.tm_sec);
	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = wcslen(sLog.c_str()) + 20;
	lvDETITEM.iItem = itemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	SendMessage(hwLogLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

	// DebugString
	memcpy(sTemp,sLog.c_str(),(wcslen(sLog.c_str()) + 20) * sizeof(TCHAR));
	lvDETITEM.iSubItem = 1;
	SendMessage(hwLogLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	SendMessage(hwLogLC,WM_VSCROLL,SB_BOTTOM,NULL);

	free(sTemp);
	return 0;
}

int OnDll(wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded)
{
	HWND hwDLLLC = GetDlgItem(hDlgDetInfo,ID_DETINFO_DLLs);
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));
	memset(tcLogging,0,MAX_PATH * sizeof(TCHAR));

	int itemIndex = SendMessage(hwDLLLC,LVM_GETITEMCOUNT,0,0);

	if(bLoaded)
	{
		// PID
		wsprintf(tcLogging,L"0x%08X",dwPID);
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 256;
		lvDETITEM.iItem = itemIndex;
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = tcLogging;
		SendMessage(hwDLLLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// EntryPoint
		wsprintf(tcLogging,L"0x%016I64X",dwEP);
		lvDETITEM.iSubItem = 1;
		SendMessage(hwDLLLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// State
		wsprintf(tcLogging,L"%s",(bLoaded ? L"LOADED" : L"UNLOADED"));
		lvDETITEM.iSubItem = 2;
		SendMessage(hwDLLLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// Path
		wsprintf(tcLogging,L"%s",sDLLPath.c_str());
		lvDETITEM.iSubItem = 3;
		SendMessage(hwDLLLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	}
	else
	{
		for(int i = 0; i <= itemIndex;i++)
		{
			DWORD dwThread_PID = 0;
			DWORD64 dwEntryPoint = 0;
			LVITEM lvItem;
			memset(&lvItem,0,sizeof(lvItem));

			lvItem.mask = LVIF_TEXT;
			lvItem.cchTextMax = 256;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			lvItem.pszText = tcLogging;
			SendMessage(hwDLLLC,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
			wstringstream ss;ss.str(wstring());ss << tcLogging;ss >> hex >> dwThread_PID;

			lvItem.iSubItem = 1;
			SendMessage(hwDLLLC,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
			wstringstream sss;sss.str(wstring());sss << tcLogging;sss >> hex >> dwEntryPoint;

			if(dwPID == dwThread_PID && dwEntryPoint == dwEP)
			{
				// State
				wsprintf(tcLogging,L"%s",L"UNLOADED");
				lvItem.iSubItem = 2;
				SendMessage(hwDLLLC,LVM_SETITEM,i,(LPARAM)&lvItem);
			}
		}
	}

	UpdateStateLable(0x1);
	return 0;
}

int OnCallStack(DWORD64 dwStackAddr,
				DWORD64 dwReturnTo,wstring sReturnToFunc,wstring sReturnToModuleName,
				DWORD64 dwEIP,wstring sFuncName,wstring sFuncModule,
				wstring sSourceFilePath,int iSourceLineNum)
{
	int itemIndex;
	LVITEM LvItem;
	memset(&LvItem,0,sizeof(LvItem));
	memset(tcLogging,0,MAX_PATH * sizeof(TCHAR));

	itemIndex = SendMessage(hwLBCallStack,LVM_GETITEMCOUNT,0,0);
	
	// Current Stack Addr
	wsprintf(tcLogging,L"0x%016I64X",dwStackAddr);
	LvItem.mask=LVIF_TEXT;
	LvItem.cchTextMax = 256;
	LvItem.iItem = itemIndex;
	LvItem.iSubItem = 0;
	LvItem.pszText = tcLogging;
	SendMessage(hwLBCallStack,LVM_INSERTITEM,0,(LPARAM)&LvItem);

	// Current Addr
	wsprintf(tcLogging,L"0x%016I64X",dwEIP);
	LvItem.iSubItem = 1;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);

	// current module.function
	if(sFuncName.length() > 0)
		wsprintf(tcLogging,L"%s.%s",sFuncModule.c_str(),sFuncName.c_str());
	else
		wsprintf(tcLogging,L"%s.0x%016I64X",sFuncModule.c_str(),dwEIP);
	LvItem.iSubItem = 2;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);
	
	// Return Addr
	wsprintf(tcLogging,L"0x%016I64X",dwReturnTo);
	LvItem.iSubItem = 3;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);

	// return to module.function
	if(sReturnToFunc.length() > 0)
		wsprintf(tcLogging,L"%s.%s",sReturnToModuleName.c_str(),sReturnToFunc.c_str());
	else
		wsprintf(tcLogging,L"%s.0x%016I64X",sReturnToModuleName.c_str(),dwReturnTo);
	LvItem.iSubItem = 4;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);

	// SourceLine
	wsprintf(tcLogging,L"%d",iSourceLineNum);
	LvItem.iSubItem = 5;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);

	// SourcePath
	wsprintf(tcLogging,L"%s",sSourceFilePath.c_str());
	LvItem.iSubItem = 6;
	SendMessage(hwLBCallStack,LVM_SETITEM,0,(LPARAM)&LvItem);

	return 0;
}

bool WriteToSettingsFile()
{
	wofstream outfile;
	outfile.open("NanomiteConfig.ini");

	if(!outfile.is_open())
		return false;

	TCHAR cTemp[128];
	wsprintf(cTemp,L"%s=%s\n",L"DebugChilds",newDebugger.dbgSettings.bDebugChilds ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));
	wsprintf(cTemp,L"%s=%s\n",L"AutoLoadSym",newDebugger.dbgSettings.bAutoLoadSymbols ? L"true" : L"false");
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%d\n",L"SUSPENDTYPE",newDebugger.dbgSettings.dwSuspendType);
	outfile.write(cTemp,wcslen(cTemp));

	for(size_t i = 0;i < newDebugger.ExceptionHandler.size();i++)
	{
		if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_BREAKPOINT)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_BREAKPOINT",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_SINGLE_STEP)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_SINGLE_STEP",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_ACCESS_VIOLATION",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_PRIV_INSTRUCTION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_PRIV_INSTRUCTION",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_ILLEGAL_INSTRUCTION)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_ILLEGAL_INSTRUCTION",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
		else if(newDebugger.ExceptionHandler[i].dwExceptionType == EXCEPTION_INT_DIVIDE_BY_ZERO)
		{
			wsprintf(cTemp,L"%s=%d\n",L"EXCEPTION_INT_DIVIDE_BY_ZERO",newDebugger.ExceptionHandler[i].dwAction);
			outfile.write(cTemp,wcslen(cTemp));
		}
	}

	wsprintf(cTemp,L"%s=%d\n",L"dwDefaultExceptionMode",newDebugger.dbgSettings.dwDefaultExceptionMode);
	outfile.write(cTemp,wcslen(cTemp));

	wsprintf(cTemp,L"%s=%d\n",L"BreakOnEPMode",newDebugger.dbgSettings.dwBreakOnEPMode);
	outfile.write(cTemp,wcslen(cTemp));

	outfile.close();
	return true;
}

bool ReadFromSettingsFile()
{
	wstring sLine;
	wifstream infile;
	infile.open(L"NanomiteConfig.ini");

	if(!infile.is_open())
		return false;

	newDebugger.CustomExceptionRemoveAll();
	while(!infile.eof())
	{
		getline(infile,sLine); // Saves the line in STRING.

		vector<wstring> sSettingLine = split(sLine,L"=");

		if(sSettingLine[0] == L"DebugChilds")
		{
			if(sSettingLine[1] == L"true")
				newDebugger.dbgSettings.bDebugChilds = true;
			else
				newDebugger.dbgSettings.bDebugChilds = false;
		}
		else if(sSettingLine[0] == L"AutoLoadSym")
		{
			if(sSettingLine[1] == L"true")
				newDebugger.dbgSettings.bAutoLoadSymbols = true;
			else
				newDebugger.dbgSettings.bAutoLoadSymbols = false;
		}
/*		else if(sSettingLine[0] == L"EXCEPTION_BREAKPOINT")
			newDebugger.CustomExceptionAdd(EXCEPTION_BREAKPOINT,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_SINGLE_STEP")
			newDebugger.CustomExceptionAdd(EXCEPTION_SINGLE_STEP,_wtoi(sSettingLine[1].c_str()),NULL);
		else */if(sSettingLine[0] == L"EXCEPTION_ACCESS_VIOLATION")
			newDebugger.CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_PRIV_INSTRUCTION")
			newDebugger.CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_ILLEGAL_INSTRUCTION")
			newDebugger.CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"EXCEPTION_INT_DIVIDE_BY_ZERO")
			newDebugger.CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,_wtoi(sSettingLine[1].c_str()),NULL);
		else if(sSettingLine[0] == L"BreakOnEPMode")
			newDebugger.dbgSettings.dwBreakOnEPMode = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"dwDefaultExceptionMode")
			newDebugger.dbgSettings.dwDefaultExceptionMode = _wtoi(sSettingLine[1].c_str());
		else if(sSettingLine[0] == L"SUSPENDTYPE")
			newDebugger.dbgSettings.dwSuspendType = _wtoi(sSettingLine[1].c_str());
	}
	infile.close();
	return true;
}

vector<wstring> split(const wstring& s,const wstring& f ){
	vector<wstring> temp;
	if ( f.empty() ) {
		temp.push_back( s );
		return temp;
	}
	typedef wstring::const_iterator iter;
	const iter::difference_type f_size( distance( f.begin(), f.end() ) );
	iter i( s.begin() );
	for ( iter pos; ( pos = search( i , s.end(), f.begin(), f.end() ) ) != s.end(); ) {
		temp.push_back( wstring( i, pos ) );
		advance( pos, f_size );
		i = pos;
	}
	temp.push_back( wstring( i, s.end() ) );
	return temp;
}

void LoadDisAssView(DWORD64 dwEIP)
{
	if(newDebugger.GetCurrentPID() == 0)
		return;

	HWND hwDisAs = GetDlgItem(hDlgMain,ID_DISASS);
	DISASM newDisAss;
	bool bContinueDisAs = true;
	DWORD64 dwEndOffset = dwEIP + 100,
		dwStartOffset = dwEIP - 50;
	DWORD dwOldProtection = 0,
		dwNewProtection = PAGE_EXECUTE_READWRITE;
	int iLen = 0,iVor = 0, iNach = 0;

	HANDLE hProc = NULL;
	LPVOID pBuffer = malloc(150);
	
	memset(&newDisAss, 0, sizeof(DISASM));

	for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
	{
		if(newDebugger.PIDs[i].dwPID == newDebugger.GetCurrentPID())
			hProc = newDebugger.PIDs[i].hProc;
	}

	bool bUnProtect = VirtualProtectEx(hProc,(LPVOID)dwStartOffset,150,dwNewProtection,&dwOldProtection);
	if(ReadProcessMemory(hProc,(LPVOID)dwStartOffset,pBuffer,150,NULL))
	{
		newDisAss.EIP = (DWORD64)pBuffer;
		newDisAss.VirtualAddr = dwStartOffset;
#ifdef _AMD64_
		newDisAss.Archi = 64;
#else
		newDisAss.Archi = 0;
#endif

		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		while(bContinueDisAs)
		{
			newDisAss.SecurityBlock = (int)dwEndOffset - newDisAss.VirtualAddr;

			iLen = Disasm(&newDisAss);

			if (iLen == OUT_OF_BLOCK)
				bContinueDisAs = false;
			else
			{			
				int itemIndex = 0;
				memset(sTemp,0,MAX_PATH *  sizeof(TCHAR));
				LVITEM lvDETITEM;
				memset(&lvDETITEM,0,sizeof(lvDETITEM));

				itemIndex = SendMessage(hwDisAs,LVM_GETITEMCOUNT,0,0);

				// Current Offset
				swprintf(sTemp,L"0x%016I64X",newDisAss.VirtualAddr);
				lvDETITEM.mask = LVIF_TEXT;
				lvDETITEM.cchTextMax = 256;
				lvDETITEM.iItem = itemIndex;
				lvDETITEM.iSubItem = 0;
				lvDETITEM.pszText = sTemp;
				SendMessage(hwDisAs,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

				// OpCodez
				BYTE bBuffer;
				bool bReaded = false;
				SIZE_T dwBytesRead = NULL;
					
				memset(sTemp,0,MAX_PATH *  sizeof(TCHAR));
				int iTempLen = ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? 1 : ((iLen == UNKNOWN_OPCODE) ? 0 : iLen));
				for(size_t i = 0;i < iTempLen;i++)
				{
					DWORD64 dwOffset = newDisAss.VirtualAddr + i;
					bReaded = ReadProcessMemory(hProc,(LPVOID)dwOffset,&bBuffer,1,&dwBytesRead);
					wsprintf(sTemp,L"%s %02X",sTemp,bBuffer);
				}
				lvDETITEM.iSubItem = 1;
				SendMessage(hwDisAs,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

				// Instruction
				if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
					wsprintf(sTemp,L"%s",L"db 00");
				else
					wsprintf(sTemp,L"%S",newDisAss.CompleteInstr);						
				lvDETITEM.iSubItem = 2;
				SendMessage(hwDisAs,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

				// Comment
				memset(sTemp,NULL,MAX_PATH * sizeof(TCHAR));
				if(strstr(newDisAss.Instruction.Mnemonic,"call") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jmp") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"push") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jnz") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"je") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jl") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jng") != 0 ||
					strstr(newDisAss.Instruction.Mnemonic,"jne") != 0)
				{
					wstring sFuncName,sModName;
					newDebugger.LoadSymbolForAddr(sFuncName,sModName,newDisAss.Instruction.AddrValue);
					if(sFuncName.length() > 0 && sModName.length() > 0)
						wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
					else if(sModName.length() > 0 && sFuncName.length() == 0)
						wsprintf(sTemp,L"%s.0x%08X",sModName.c_str(),newDisAss.VirtualAddr);
				}
				else
				{
					if(newDisAss.Instruction.Opcode != 0xCC &&
						newDisAss.Instruction.Opcode != 0x90)
					{
						wstring sFuncName,sModName;
						newDebugger.LoadSymbolForAddr(sFuncName,sModName,newDisAss.VirtualAddr);
						if(sFuncName.length() > 0)
							wsprintf(sTemp,L"%s",sFuncName.c_str());
						else 
							wsprintf(sTemp,L"%s",L"");
					}
					else
						wsprintf(sTemp,L"%s",L"");
				}
					
				lvDETITEM.iSubItem = 3;
				SendMessage(hwDisAs,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
			}

			newDisAss.EIP = newDisAss.EIP + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			newDisAss.VirtualAddr = newDisAss.VirtualAddr + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			if (newDisAss.VirtualAddr >= dwEndOffset)
				bContinueDisAs = false;
		}
		free(sTemp);
	}

	bool bProtect = VirtualProtectEx(hProc,(LPVOID)dwStartOffset,150,dwOldProtection,NULL);

	int iItemIndex = ListView_GetItemCount(hwDisAs),
		iSearchedIndex = 0;
	PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	for(size_t i = 0;i < iItemIndex;i++)
	{
		DWORD64 dwLCEIP = 0;
		LVITEM lvItem;
		memset(&lvItem,0,sizeof(lvItem));

		lvItem.mask = LVIF_TEXT;
		lvItem.cchTextMax = 256;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = sTemp;
		SendMessage(hwDisAs,LVM_GETITEMTEXT,i,(LPARAM)&lvItem);
		wstringstream ss;ss.str(wstring());ss << sTemp;ss >> hex >> dwLCEIP;

		if(dwEIP == dwLCEIP)
		{
			iSearchedIndex = i;

			if(iSearchedIndex >= 5)
			{
				ListView_DeleteItem(hwDisAs,i = 0);
			}
		}

		if(iSearchedIndex == 5)
		{
			i = 25;
			iItemIndex = ListView_GetItemCount(hwDisAs);
			ListView_DeleteItem(hwDisAs,i);
		}
	}

	free(sTemp);
	free(pBuffer);
}

void LoadRegView()
{
	DWORD dwEFlags = newDebugger.ProcessContext.EFlags;
	BOOL bCF = false, // Carry Flag
		bPF = false, // Parity Flag
		bAF = false, // Auxiliarty carry flag
		bZF = false, // Zero Flag
		bSF = false, // Sign Flag
		bTF = false, // Trap Flag
		bIF = false, // Interrupt Flag
		bDF = false, // Direction Flag
		bOF = false; // Overflow Flag

	bOF = (dwEFlags & 0x800) ? true : false;
	bDF = (dwEFlags & 0x400) ? true : false;
	bTF = (dwEFlags & 0x100) ? true : false;
	bSF = (dwEFlags & 0x80) ? true : false;
	bZF = (dwEFlags & 0x40) ? true : false;
	bAF = (dwEFlags & 0x10) ? true : false;
	bPF = (dwEFlags & 0x4) ? true : false;
	bCF = (dwEFlags & 0x1) ? true : false;

	memset(tcLogging,0,MAX_PATH * sizeof(TCHAR));
#ifdef _AMD64_
	BOOL bIsWow64Process = false;
	HANDLE hProcess = NULL;

	for(size_t i = 0; i < newDebugger.PIDs.size(); i++)
		if(newDebugger.PIDs[i].dwPID == newDebugger.GetCurrentPID())
			hProcess = newDebugger.PIDs[i].hProc;

	IsWow64Process(hProcess,&bIsWow64Process);
	if(bIsWow64Process)
	{
		wsprintf(tcLogging,L"EAX\t- 0x%08X\tCF:%X\r\nECX\t- 0x%08X\tPF:%X\r\nEDX\t- 0x%08X\tAF:%X\r\nEBX\t- 0x%08X\tZF:%X\r\nESP\t- 0x%08X\tSF:%X\r\nEBP\t- 0x%08X\tTF:%X\r\nESI\t- 0x%08X\tDF:%X\r\nEDI\t- 0x%08X\tOF:%X\r\nEIP\t- 0x%08X\r\n",
			newDebugger.wowProcessContext.Eax,
			bCF,
			newDebugger.wowProcessContext.Ecx,
			bPF,
			newDebugger.wowProcessContext.Edx,
			bAF,
			newDebugger.wowProcessContext.Ebx,
			bZF,
			newDebugger.wowProcessContext.Esp,
			bSF,
			newDebugger.wowProcessContext.Ebp,
			bTF,
			newDebugger.wowProcessContext.Esi,
			bDF,
			newDebugger.wowProcessContext.Edi,
			bOF,
			newDebugger.wowProcessContext.Eip);
		// r8 - r15
	}
	else
	{
		wsprintf(tcLogging,L"RAX\t- 0x%016I64X\tCF:%X\r\nRCX\t- 0x%016I64X\tPF:%X\r\nRDX\t- 0x%016I64X\tAF:%X\r\nRBX\t- 0x%016I64X\tZF:%X\r\nRSP\t- 0x%016I64X\tSF:%X\r\nRBP\t- 0x%016I64X\tTF:%X\r\nRSI\t- 0x%016I64X\tDF:%X\r\nRDI\t- 0x%016I64X\tOF:%X\r\nRIP\t- 0x%016I64X\r\n",
			newDebugger.ProcessContext.Rax,
			bCF,
			newDebugger.ProcessContext.Rcx,
			bPF,
			newDebugger.ProcessContext.Rdx,
			bAF,
			newDebugger.ProcessContext.Rbx,
			bZF,
			newDebugger.ProcessContext.Rsp,
			bSF,
			newDebugger.ProcessContext.Rbp,
			bTF,
			newDebugger.ProcessContext.Rsi,
			bDF,
			newDebugger.ProcessContext.Rdi,
			bOF,
			newDebugger.ProcessContext.Rip);
		// r8 - r15
	}
#else
	wsprintf(tcLogging,L"EAX\t- 0x%08X\tCF:%X\r\nECX\t- 0x%08X\tPF:%X\r\nEDX\t- 0x%08X\tAF:%X\r\nEBX\t- 0x%08X\tZF:%X\r\nESP\t- 0x%08X\tSF:%X\r\nEBP\t- 0x%08X\tTF:%X\r\nESI\t- 0x%08X\tDF:%X\r\nEDI\t- 0x%08X\tOF:%X\r\nEIP\t- 0x%08X\r\n",
		newDebugger.ProcessContext.Eax,
		bCF,
		newDebugger.ProcessContext.Ecx,
		bPF,
		newDebugger.ProcessContext.Edx,
		bAF,
		newDebugger.ProcessContext.Ebx,
		bZF,
		newDebugger.ProcessContext.Esp,
		bSF,
		newDebugger.ProcessContext.Ebp,
		bTF,
		newDebugger.ProcessContext.Esi,
		bDF,
		newDebugger.ProcessContext.Edi,
		bOF,
		newDebugger.ProcessContext.Eip);
#endif
	Edit_SetText(GetDlgItem(hDlgMain,ID_REGVIEW),tcLogging);
}

bool PrintMemToHexView(DWORD dwPID,DWORD dwOffset,DWORD dwSize,HWND hwHexView)
{
	SIZE_T dwBytesRead = NULL;
	DWORD dwCounter = NULL,
		dwProtection = NULL,
		dwStepSize = 0x10,
		dwBaseOffset = dwOffset;
	HANDLE hProcess = NULL;
	LPVOID pBuffer = malloc(dwSize);
	TCHAR *tcTempBuffer = (PTCHAR)malloc(255 * sizeof(255)),
		*tcAsciiHexTemp = (PTCHAR)malloc(255 * sizeof(255));

	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));

	ListView_DeleteAllItems(hwHexView);
	for(size_t i = 0;i < newDebugger.PIDs.size();i++)
	{
		if(dwPID == newDebugger.PIDs[i].dwPID)
			hProcess = newDebugger.PIDs[i].hProc;
	}

	if(hProcess == INVALID_HANDLE_VALUE)
		return false;

	if(!VirtualProtectEx(hProcess,(LPVOID)dwOffset,dwSize,PAGE_EXECUTE_READWRITE,&dwProtection))
	{
		free(pBuffer);
		free(tcAsciiHexTemp);
		free(tcTempBuffer);
		return false;
	}

	if(!ReadProcessMemory(hProcess,(LPVOID)dwOffset,(LPVOID)pBuffer,dwSize,&dwBytesRead))
	{
		free(pBuffer);
		free(tcAsciiHexTemp);
		free(tcTempBuffer);
		return false;
	}

	for(int iLineStep = 0; iLineStep < (dwSize / dwStepSize); iLineStep++)
	{
		if(dwOffset > (dwBaseOffset + dwSize))
			break;

		// Offset
		wsprintf(tcTempBuffer,L"0x%08X",dwOffset);
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 255;
		lvDETITEM.iItem = ListView_GetItemCount(hwHexView);
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = tcTempBuffer;
		SendMessage(hwHexView,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// Hexiss
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%02X ",*(PCHAR)((DWORD)pBuffer + dwCounter + i));
			if(wcsstr(tcAsciiHexTemp,L"FFF") != NULL)
				wcscat(tcTempBuffer,L"FF ");
			else
				wcscat(tcTempBuffer,tcAsciiHexTemp);
		}
		lvDETITEM.iSubItem = 1;
		SendMessage(hwHexView,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		//Acsii
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%c ",*(PCHAR)((DWORD)pBuffer + dwCounter + i));
			if(wcsstr(tcAsciiHexTemp,L"  ") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\r") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\t") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\n") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(*(PCHAR)((DWORD)pBuffer + dwCounter + i) ==  0x00)
				wcscat(tcTempBuffer,L". ");
			else
				wcscat(tcTempBuffer,tcAsciiHexTemp);
		}
		lvDETITEM.iSubItem = 2;
		SendMessage(hwHexView,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		dwOffset += dwStepSize;
		dwCounter += dwStepSize;
	}

	VirtualProtectEx(hProcess,(LPVOID)dwOffset,dwSize,dwProtection,NULL);
	free(pBuffer);
	free(tcAsciiHexTemp);
	free(tcTempBuffer);
	return true;
}

DWORD CalcNewOffset(DWORD dwCurrentOffset)
{
	if(newDebugger.GetCurrentPID() == 0)
		return 0;

	DISASM newDisAss;
	bool bContinueDisAs = true;
	DWORD dwEndOffset = dwCurrentOffset + 70,
		dwStartOffset = dwCurrentOffset,
		dwOldProtection = NULL,
		dwNewProtection = PAGE_EXECUTE_READWRITE,
		dwNewOffset = NULL;
	int iLen = 0;
	HANDLE hProc = NULL;
	LPVOID pBuffer = malloc(70);

	memset(&newDisAss, 0, sizeof(DISASM));

	for(size_t i = 0;i < newDebugger.PIDs.size(); i++)
	{
		if(newDebugger.PIDs[i].dwPID == newDebugger.GetCurrentPID())
			hProc = newDebugger.PIDs[i].hProc;
	}

	bool bUnProtect = VirtualProtectEx(hProc,(LPVOID)dwStartOffset,70,dwNewProtection,&dwOldProtection);

	if(ReadProcessMemory(hProc,(LPVOID)dwStartOffset,pBuffer,70,NULL))
	{
		newDisAss.EIP = (int)pBuffer;
		newDisAss.VirtualAddr = dwStartOffset;
		newDisAss.Archi = 0;

		bool bOneMore = false;

		while(bContinueDisAs)
		{
			newDisAss.SecurityBlock = (int)dwEndOffset - newDisAss.VirtualAddr;

			iLen = Disasm(&newDisAss);

			if (iLen == OUT_OF_BLOCK)
				bContinueDisAs = false;
			else
			{
				if(bOneMore)
				{
					dwNewOffset = newDisAss.VirtualAddr;
					bContinueDisAs = false;
				}

				if(newDisAss.VirtualAddr == dwCurrentOffset)
					bOneMore = true;
			}

			newDisAss.EIP = newDisAss.EIP + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			newDisAss.VirtualAddr = newDisAss.VirtualAddr + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			if (newDisAss.VirtualAddr >= (int)dwEndOffset)
				bContinueDisAs = false;
		}
	}

	bool bProtect = VirtualProtectEx(hProc,(LPVOID)dwCurrentOffset,70,dwOldProtection,NULL);
	free(pBuffer);

	return dwNewOffset;
}

void LoadStackView(DWORD64 dwESP, DWORD dwStackSize)
{
	bool bCheckVar = false;
	SIZE_T dwBytesRead = NULL;
	LPBYTE bBuffer;
	PTCHAR sTemp;
	HANDLE hProcess = newDebugger.GetCurrentProcessHandle();
	HWND hwStackViewLC = GetDlgItem(hDlgMain,ID_STACKVIEW);
	DWORD dwOldProtect = NULL,
		dwNewProtect = PAGE_EXECUTE_READWRITE,
		dwSize = 12 * dwStackSize;
	DWORD64	dwStartOffset = dwESP - dwStackSize * 6,
		dwEndOffset = dwESP + dwStackSize * 6;

	ListView_DeleteAllItems(hwStackViewLC);

	if(hProcess == INVALID_HANDLE_VALUE)
		return;

	if(!VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwNewProtect,&dwOldProtect))
		return;

	bBuffer = (LPBYTE)malloc(dwSize);
	if(bBuffer == NULL)
		return;

	if(!ReadProcessMemory(hProcess,(LPVOID)dwStartOffset,(LPVOID)bBuffer,dwSize,&dwBytesRead))
		return;

	sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

	for(size_t i = 0;i < (dwSize / dwStackSize); i++)
	{
		LVITEM lvDETITEM;
		memset(&lvDETITEM,0,sizeof(lvDETITEM));

		int itemIndex = SendMessage(hwStackViewLC,LVM_GETITEMCOUNT,0,0);

		// Current Offset
		wsprintf(sTemp,L"0x%016I64X",(dwStartOffset + i * dwStackSize));
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 255;
		lvDETITEM.iItem = itemIndex;
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = sTemp;
		SendMessage(hwStackViewLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// Value
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		IsWow64Process(newDebugger.GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			for(int id = 3;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
		else
			for(int id = 7;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));

#else
		for(int id = 3;id != -1;id--)
			wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * dwStackSize + id)));
#endif
		lvDETITEM.iSubItem = 1;
		SendMessage(hwStackViewLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// Comment
		DWORD dwOffset = NULL;
		wstringstream ss; ss << sTemp; ss >> hex >> dwOffset;
		wstring sFuncName,sModName;
		newDebugger.LoadSymbolForAddr(sFuncName,sModName,dwOffset);
		if(sFuncName != L"")
			wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
		else 
			wsprintf(sTemp,L"%s",L"");
		lvDETITEM.iSubItem = 2;
		SendMessage(hwStackViewLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	}

	bCheckVar = VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwOldProtect,NULL);
	free(bBuffer);
	free(sTemp);
}

HMENU GenerateRunningPIDMenu(bool bWithAll)
{
	HMENU hMapMenu = CreatePopupMenu();
	MENUINFO menuInfo;
	memset(&menuInfo, 0, sizeof(menuInfo));
	menuInfo.cbSize = sizeof(menuInfo);
	menuInfo.fMask = MIM_STYLE;
	menuInfo.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(hMapMenu, &menuInfo);

	if(bWithAll)
		InsertMenu(hMapMenu,newDebugger.PIDs.size(),MF_BYPOSITION | MF_STRING,1,L"ALL");

	for(size_t i = 0;i < newDebugger.PIDs.size();i++)
	{
		if(newDebugger.PIDs[i].bRunning)
		{
			wstringstream ss;ss << "PID: 0x" << hex << newDebugger.PIDs[i].dwPID;
			InsertMenu(hMapMenu,i,MF_BYPOSITION | MF_STRING,i,ss.str().c_str());
		}
	}
	POINT pMouse;
	GetCursorPos(&pMouse);
	TrackPopupMenu(hMapMenu,TPM_TOPALIGN | TPM_LEFTALIGN,pMouse.x,pMouse.y,NULL,hDlgMain,NULL);
	return hMapMenu;
}

bool printPEView(HWND hWndDlg,int i)
{
	HWND hwIDH = GetDlgItem(hWndDlg,IDC_DOS_HEADER),
		hwIFH = GetDlgItem(hWndDlg,IDC_FILE_HEADER),
		hwIOH = GetDlgItem(hWndDlg,IDC_OPTIONAL_HEADER),
		hwFI = GetDlgItem(hWndDlg,IDC_FileImports),
		hwFS = GetDlgItem(hWndDlg,IDC_FileSections),
		hwTLSDIR = GetDlgItem(hWndDlg,IDC_TLSDir),
		hwDATADIR = GetDlgItem(hWndDlg,IDC_DATADIRS),
		hwSTATE = GetDlgItem(hWndDlg,IDC_PEViewState);

	HMODULE hFile = LoadLibrary(newDebugger.PIDs[i].sFileName);
	if(hFile != 0)
	{
		IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)hFile;
		if(pDOSHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			IMAGE_NT_HEADERS* pNTHeader = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);
			if(pNTHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				PTCHAR sTemp = (PTCHAR)malloc(255);
				wsprintf(sTemp,L"[%08X] - File: %s",newDebugger.PIDs[i].dwPID,newDebugger.PIDs[i].sFileName);
				Static_SetText(hwSTATE,sTemp);
				free(sTemp);

				InsertPEPartIntoLC(L"e_magic",pDOSHeader->e_magic,hwIDH);
				InsertPEPartIntoLC(L"e_cblp",pDOSHeader->e_cblp,hwIDH);
				InsertPEPartIntoLC(L"e_cp",pDOSHeader->e_cp,hwIDH);
				InsertPEPartIntoLC(L"e_crlc",pDOSHeader->e_crlc,hwIDH);
				InsertPEPartIntoLC(L"e_cparhdr",pDOSHeader->e_cparhdr,hwIDH);
				InsertPEPartIntoLC(L"e_minalloc",pDOSHeader->e_minalloc,hwIDH);
				InsertPEPartIntoLC(L"e_maxalloc",pDOSHeader->e_maxalloc,hwIDH);
				InsertPEPartIntoLC(L"e_ss",pDOSHeader->e_ss,hwIDH);
				InsertPEPartIntoLC(L"e_sp",pDOSHeader->e_sp,hwIDH);
				InsertPEPartIntoLC(L"e_csum",pDOSHeader->e_csum,hwIDH);
				InsertPEPartIntoLC(L"e_ip",pDOSHeader->e_ip,hwIDH);
				InsertPEPartIntoLC(L"e_cs",pDOSHeader->e_cs,hwIDH);
				InsertPEPartIntoLC(L"e_lfarlc",pDOSHeader->e_lfarlc,hwIDH);
				InsertPEPartIntoLC(L"e_ovno",pDOSHeader->e_ovno,hwIDH);
				//InsertPEPartIntoLC("e_res",(DWORD)pDOSHeader->e_res,hwIDH);
				InsertPEPartIntoLC(L"e_oemid",pDOSHeader->e_oemid,hwIDH);
				InsertPEPartIntoLC(L"e_oeminfo",pDOSHeader->e_oeminfo,hwIDH);
				//InsertPEPartIntoLC("e_res2",(DWORD)pDOSHeader->e_res2,hwIDH);


				InsertPEPartIntoLC(L"Machine",pNTHeader->FileHeader.Machine,hwIFH);
				InsertPEPartIntoLC(L"NumberOfSections",pNTHeader->FileHeader.NumberOfSections,hwIFH);
				InsertPEPartIntoLC(L"TimeDateStamp",pNTHeader->FileHeader.TimeDateStamp,hwIFH);
				InsertPEPartIntoLC(L"PointerToSymbolTable",pNTHeader->FileHeader.PointerToSymbolTable,hwIFH);
				InsertPEPartIntoLC(L"NumberOfSymbols",pNTHeader->FileHeader.NumberOfSymbols,hwIFH);
				InsertPEPartIntoLC(L"SizeOfOptionalHeader",pNTHeader->FileHeader.SizeOfOptionalHeader,hwIFH);
				InsertPEPartIntoLC(L"Characteristics",pNTHeader->FileHeader.Characteristics,hwIFH);


				InsertPEPartIntoLC(L"Magic",pNTHeader->OptionalHeader.Magic,hwIOH);
				InsertPEPartIntoLC(L"MajorLinkerVersion",pNTHeader->OptionalHeader.MajorLinkerVersion,hwIOH);
				InsertPEPartIntoLC(L"MinorLinkerVersion",pNTHeader->OptionalHeader.MinorLinkerVersion,hwIOH);
				InsertPEPartIntoLC(L"SizeOfCode",pNTHeader->OptionalHeader.SizeOfCode,hwIOH);
				InsertPEPartIntoLC(L"SizeOfInitializedData",pNTHeader->OptionalHeader.SizeOfInitializedData,hwIOH);
				InsertPEPartIntoLC(L"SizeOfUninitializedData",pNTHeader->OptionalHeader.SizeOfUninitializedData,hwIOH);
				InsertPEPartIntoLC(L"AddressOfEntryPoint",pNTHeader->OptionalHeader.AddressOfEntryPoint,hwIOH);
				InsertPEPartIntoLC(L"BaseOfCode",pNTHeader->OptionalHeader.BaseOfCode,hwIOH);
#ifndef _AMD64_
				InsertPEPartIntoLC(L"BaseOfData",pNTHeader->OptionalHeader.BaseOfData,hwIOH);
#endif
				InsertPEPartIntoLC(L"ImageBase",pNTHeader->OptionalHeader.ImageBase,hwIOH);
				InsertPEPartIntoLC(L"SectionAlignment",pNTHeader->OptionalHeader.SectionAlignment,hwIOH);
				InsertPEPartIntoLC(L"FileAlignment",pNTHeader->OptionalHeader.FileAlignment,hwIOH);
				InsertPEPartIntoLC(L"MajorOperatingSystemVersion",pNTHeader->OptionalHeader.MajorOperatingSystemVersion,hwIOH);
				InsertPEPartIntoLC(L"MinorOperatingSystemVersion",pNTHeader->OptionalHeader.MinorOperatingSystemVersion,hwIOH);
				InsertPEPartIntoLC(L"MajorImageVersion",pNTHeader->OptionalHeader.MajorImageVersion,hwIOH);
				InsertPEPartIntoLC(L"MinorImageVersion",pNTHeader->OptionalHeader.MinorImageVersion,hwIOH);
				InsertPEPartIntoLC(L"MajorSubsystemVersion",pNTHeader->OptionalHeader.MajorSubsystemVersion,hwIOH);
				InsertPEPartIntoLC(L"MinorSubsystemVersion",pNTHeader->OptionalHeader.MinorSubsystemVersion,hwIOH);
				InsertPEPartIntoLC(L"Win32VersionValue",pNTHeader->OptionalHeader.Win32VersionValue,hwIOH);
				InsertPEPartIntoLC(L"SizeOfImage",pNTHeader->OptionalHeader.SizeOfImage,hwIOH);
				InsertPEPartIntoLC(L"SizeOfHeaders",pNTHeader->OptionalHeader.SizeOfHeaders,hwIOH);
				InsertPEPartIntoLC(L"CheckSum",pNTHeader->OptionalHeader.CheckSum,hwIOH);
				InsertPEPartIntoLC(L"Subsystem",pNTHeader->OptionalHeader.Subsystem,hwIOH);
				InsertPEPartIntoLC(L"DllCharacteristics",pNTHeader->OptionalHeader.DllCharacteristics,hwIOH);
				InsertPEPartIntoLC(L"SizeOfStackReserve",pNTHeader->OptionalHeader.SizeOfStackReserve,hwIOH);
				InsertPEPartIntoLC(L"SizeOfStackCommit",pNTHeader->OptionalHeader.SizeOfStackCommit,hwIOH);
				InsertPEPartIntoLC(L"SizeOfHeapReserve",pNTHeader->OptionalHeader.SizeOfHeapReserve,hwIOH);
				InsertPEPartIntoLC(L"SizeOfHeapCommit",pNTHeader->OptionalHeader.SizeOfHeapCommit,hwIOH);
				InsertPEPartIntoLC(L"LoaderFlags",pNTHeader->OptionalHeader.LoaderFlags,hwIOH);
				InsertPEPartIntoLC(L"NumberOfRvaAndSizes",pNTHeader->OptionalHeader.NumberOfRvaAndSizes,hwIOH);


				for(size_t i = 0;i < pNTHeader->OptionalHeader.NumberOfRvaAndSizes;i++)
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					int itemIndex = ListView_GetItemCount(hwDATADIR);

					LVITEM lvDETITEM;
					memset(&lvDETITEM,0,sizeof(lvDETITEM));

					lvDETITEM.mask = LVIF_TEXT;
					lvDETITEM.cchTextMax = 255;
					lvDETITEM.iItem = itemIndex;
					lvDETITEM.iSubItem = 0;
					lvDETITEM.pszText = sTemp;
					wsprintf(sTemp,L"%02d",i);
					SendMessage(hwDATADIR,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pNTHeader->OptionalHeader.DataDirectory[i].VirtualAddress);
					lvDETITEM.iSubItem = 1;
					SendMessage(hwDATADIR,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pNTHeader->OptionalHeader.DataDirectory[i].Size);
					lvDETITEM.iSubItem = 2;
					SendMessage(hwDATADIR,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					free(sTemp);
				}


				PIMAGE_SECTION_HEADER pSH = (PIMAGE_SECTION_HEADER)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS32));
				for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections;i++)
				{
					PTCHAR sTemp = (PTCHAR)malloc(255);
					int itemIndex = ListView_GetItemCount(hwFS);

					LVITEM lvDETITEM;
					memset(&lvDETITEM,0,sizeof(lvDETITEM));

					lvDETITEM.mask = LVIF_TEXT;
					lvDETITEM.cchTextMax = 255;
					lvDETITEM.iItem = itemIndex;
					lvDETITEM.iSubItem = 0;
					lvDETITEM.pszText = sTemp;
					wsprintf(sTemp,L"%S",pSH->Name);
					SendMessage(hwFS,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pSH->Misc.VirtualSize);
					lvDETITEM.iSubItem = 1;
					SendMessage(hwFS,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pSH->VirtualAddress);
					lvDETITEM.iSubItem = 2;
					SendMessage(hwFS,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pSH->SizeOfRawData);
					lvDETITEM.iSubItem = 3;
					SendMessage(hwFS,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pSH->PointerToRawData);
					lvDETITEM.iSubItem = 4;
					SendMessage(hwFS,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pSH->Characteristics);
					lvDETITEM.iSubItem = 5;
					SendMessage(hwFS,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

					pSH++;
					free(sTemp);
				}

				DWORD dwVAOfImportSection = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
				if(dwVAOfImportSection != 0)
				{
					PIMAGE_IMPORT_DESCRIPTOR pImportHeader = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hFile + dwVAOfImportSection);
					do
					{
						PIMAGE_THUNK_DATA32 pIAT = (PIMAGE_THUNK_DATA32)((BYTE*)hFile + pImportHeader->FirstThunk);
						if (pIAT->u1.Ordinal)
						{
							do
							{
								if (IMAGE_SNAP_BY_ORDINAL32(pIAT->u1.Ordinal))
								{
									//wsprintf(sTemp,"\tOrdinal:\t\t\t0x%08X",IMAGE_ORDINAL32(pIAT->u1.Ordinal));
									//ListBox_AddString(hwPEViewData,sTemp);
								} else {
									PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hFile + pIAT->u1.AddressOfData);
									
									PTCHAR sTemp = (PTCHAR)malloc(255);
									int itemIndex = ListView_GetItemCount(hwFI);

									LVITEM lvDETITEM;
									memset(&lvDETITEM,0,sizeof(lvDETITEM));

									lvDETITEM.mask = LVIF_TEXT;
									lvDETITEM.cchTextMax = 255;
									lvDETITEM.iItem = itemIndex;
									lvDETITEM.iSubItem = 0;
									lvDETITEM.pszText = sTemp;
									wsprintf(sTemp,L"%S",(PTCHAR)((BYTE*)hFile + pImportHeader->Name));
									SendMessage(hwFI,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
									wsprintf(sTemp,L"%S",pImportName->Name);
									lvDETITEM.iSubItem = 1;
									SendMessage(hwFI,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
									wsprintf(sTemp,L"%08X",pIAT->u1.AddressOfData);
									lvDETITEM.iSubItem = 2;
									SendMessage(hwFI,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
									free(sTemp);
								}
								pIAT++;
							} while (pIAT->u1.AddressOfData != 0);
						}
						pImportHeader++;
					} while (pImportHeader->Name);
				}

				DWORD TLSTableVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
				if(TLSTableVA != 0)
				{
					PIMAGE_TLS_DIRECTORY32 pTLS = (PIMAGE_TLS_DIRECTORY32)((BYTE*)hFile + TLSTableVA);

					PTCHAR sTemp = (PTCHAR)malloc(255);
					int itemIndex = ListView_GetItemCount(hwFI);

					LVITEM lvDETITEM;
					memset(&lvDETITEM,0,sizeof(lvDETITEM));

					lvDETITEM.mask = LVIF_TEXT;
					lvDETITEM.cchTextMax = 255;
					lvDETITEM.iItem = itemIndex;
					lvDETITEM.iSubItem = 0;
					lvDETITEM.pszText = sTemp;
					wsprintf(sTemp,L"%08X",pTLS->AddressOfCallBacks);
					SendMessage(hwFI,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pTLS->StartAddressOfRawData);
					lvDETITEM.iSubItem = 1;
					SendMessage(hwFI,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					wsprintf(sTemp,L"%08X",pTLS->StartAddressOfRawData);
					lvDETITEM.iSubItem = 2;
					SendMessage(hwFI,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
					free(sTemp);
				}
			}
			else
				MessageBox(hWndDlg,L"Invalid NT Header!",L"Nanomite",MB_OKCANCEL);
		}	
		else
			MessageBox(hWndDlg,L"Invalid DOS Header!",L"Nanomite",MB_OKCANCEL);
	}
	else
		MessageBox(hWndDlg,L"Could not open the File!",L"Nanomite",MB_OKCANCEL);

	return true;
}

bool InsertPEPartIntoLC(wstring sPEStructName,DWORD dwData,HWND hwLC)
{
	PTCHAR sTemp = (PTCHAR)malloc(255);
	int itemIndex = ListView_GetItemCount(hwLC);
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));

	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = 255;
	lvDETITEM.iItem = itemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	wsprintf(sTemp,L"%s",sPEStructName.c_str());
	SendMessage(hwLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
	wsprintf(sTemp,L"%08X",dwData);
	lvDETITEM.iSubItem = 1;
	SendMessage(hwLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	free(sTemp);
	return true;
}

bool CALLBACK EnumWindowCallBack(HWND hWnd,LPARAM lParam)
{
	DWORD dwHwPID = NULL;
	GetWindowThreadProcessId(hWnd,&dwHwPID);
	
	HWND hwWndList = GetDlgItem(hDlgWndList,IDC_WNDLIST);
	int iItemIndex = ListView_GetItemCount(hwWndList),
		iPid = (int)lParam;

	if(dwHwPID == iPid)
	{
		LVITEM lvDETITEM;
		memset(&lvDETITEM,0,sizeof(lvDETITEM));
		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));

		// PID
		wsprintf(sTemp,L"0x%08X",dwHwPID);
		lvDETITEM.mask = LVIF_TEXT;
		lvDETITEM.cchTextMax = 255;
		lvDETITEM.iItem = iItemIndex;
		lvDETITEM.iSubItem = 0;
		lvDETITEM.pszText = sTemp;
		SendMessage(hwWndList,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

		// GetWindowName
		GetWindowText(hWnd,sTemp,MAX_PATH);
		lvDETITEM.iSubItem = 1;
		SendMessage(hwWndList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// IsVisible
		wsprintf(sTemp,L"%s",(IsWindowVisible(hWnd) ? L"TRUE" : L"FALSE"));
		lvDETITEM.iSubItem = 2;
		SendMessage(hwWndList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// hWnd
		wsprintf(sTemp,L"0x%08X",hWnd);
		lvDETITEM.iSubItem = 3;
		SendMessage(hwWndList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		// GetModuleName
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		GetWindowModuleFileName(hWnd,sTemp,MAX_PATH);
		lvDETITEM.iSubItem = 4;
		SendMessage(hwWndList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

		//// String
		//wsprintf(sTemp,"%s",sTempString.str().c_str());
		//lvDETITEM.iSubItem = 2;
		//SendMessage(hwWndList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
		free(sTemp);
	}
	return true;
}

bool InsertHandleIntoLC(HWND hwLC,DWORD dwPID,DWORD dwHandleID,PTCHAR sType,PTCHAR sName)
{
	PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	int itemIndex = ListView_GetItemCount(hwLC);
	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));

	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = 255;
	lvDETITEM.iItem = itemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	wsprintf(sTemp,L"%08X",dwPID);
	SendMessage(hwLC,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);
	wsprintf(sTemp,L"%s",sType);
	lvDETITEM.iSubItem = 2;
	SendMessage(hwLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	wsprintf(sTemp,L"%08X",dwHandleID);
	lvDETITEM.iSubItem = 1;
	SendMessage(hwLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);
	wsprintf(sTemp,L"%s",sName);
	lvDETITEM.iSubItem = 3;
	SendMessage(hwLC,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	free(sTemp);
	return true;
}

BOOL CALLBACK EnumResTypes(HMODULE hModule,LPTSTR lpszType,LONG lParam)
{
#ifndef _AMD64_
	if(EnumResourceNames(hModule,lpszType,EnumResNames,lParam) == 0)
	{
		DWORD dwError = GetLastError();
		return false;
	}
#endif
	return true;
}

BOOL CALLBACK EnumResNames(HMODULE hModule,LPCTSTR lpszType,LPTSTR lpszName,LONG lParam)
{
	HWND hwResList = GetDlgItem(hDlgResList,IDC_RES);
	int iItemIndex = ListView_GetItemCount(hwResList);
	EnumInfoStruct* PIDInfo = (EnumInfoStruct*)lParam;

	LVITEM lvDETITEM;
	memset(&lvDETITEM,0,sizeof(lvDETITEM));
	PTCHAR sTemp = (PTCHAR)malloc(255);


	// PID
	wsprintf(sTemp,L"0x%08X",PIDInfo->dwPID);
	lvDETITEM.mask = LVIF_TEXT;
	lvDETITEM.cchTextMax = 255;
	lvDETITEM.iItem = iItemIndex;
	lvDETITEM.iSubItem = 0;
	lvDETITEM.pszText = sTemp;
	SendMessage(hwResList,LVM_INSERTITEM,0,(LPARAM)&lvDETITEM);

	// File Path
	wsprintf(sTemp,L"%s",PIDInfo->sFileName.c_str());
	lvDETITEM.iSubItem = 1;
	SendMessage(hwResList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	// Res Type
	if(IS_INTRESOURCE(lpszType))
		wsprintf(sTemp,L"0x%08X",lpszType);
	else
		wsprintf(sTemp,L"%s",lpszType);
	lvDETITEM.iSubItem = 2;
	SendMessage(hwResList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	// Res Name
	wsprintf(sTemp,L"%s",lpszName);
	lvDETITEM.iSubItem = 3;
	SendMessage(hwResList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	// Res Size
	wsprintf(sTemp,L"0x%08X",SizeofResource(hModule,FindResource(hModule,lpszName,lpszType)));
	lvDETITEM.iSubItem = 4;
	SendMessage(hwResList,LVM_SETITEM,0,(LPARAM)&lvDETITEM);

	free(sTemp);
	return true;
}

void UpdateStateLable(DWORD dwState)
{
	PTCHAR tcStateString = (PTCHAR)malloc(128);
	memset(tcTempState,0,sizeof(MAX_PATH * sizeof(TCHAR)));

	switch(dwState)
	{
	case 0x1: // Running
		swprintf_s(tcStateString,64,L"Running");
		break;
	case 0x2: // Suspended
		swprintf_s(tcStateString,64,L"Suspended");
		break;
	case 0x3: // Terminated
		swprintf_s(tcStateString,64,L"Terminated");
		break;
	}
	swprintf_s(tcTempState,255,L"\t\tPIDs: %d  TIDs: %d  DLLs: %d  Exceptions: %d State: %s",
		newDebugger.PIDs.size(),
		newDebugger.TIDs.size(),
		newDebugger.DLLs.size(),
		dwExceptionCount,
		tcStateString);

	Static_SetText(GetDlgItem(hDlgMain,IDC_STATE),tcTempState);

	free(tcStateString);
}

void CleanUpGUI()
{
	ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_PID));
	ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_TID));
	ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_DLLs));
	ListView_DeleteAllItems(GetDlgItem(hDlgDetInfo,ID_DETINFO_EXCEPTIONS));
	ListView_DeleteAllItems(GetDlgItem(hDlgMain,IDC_LIST2));
	ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_DISASS));
	ListView_DeleteAllItems(GetDlgItem(hDlgMain,ID_STACKVIEW));
	ListView_DeleteAllItems(GetDlgItem(hDlgMain,IDC_LOG));
	ListView_DeleteAllItems(GetDlgItem(hDlgDbgStringInfo,IDC_DBGSTR));
	dwExceptionCount = 0;
}