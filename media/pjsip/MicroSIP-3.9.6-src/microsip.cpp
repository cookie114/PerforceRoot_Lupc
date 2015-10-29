// microsip.cpp : Defines the class behaviors for the application.
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include "microsip.h"
#include "microsipDlg.h"
#include "const.h"
#include "settings.h"

#include "Strsafe.h"


#pragma comment(linker, "/SECTION:.shr,RWS")
#pragma data_seg(".shr")
HWND hGlobal = NULL;
#pragma data_seg()


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmicrosipApp

BEGIN_MESSAGE_MAP(CmicrosipApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CmicrosipApp construction

CmicrosipApp::CmicrosipApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CmicrosipApp object

CmicrosipApp theApp;


// CmicrosipApp initialization

BOOL CmicrosipApp::InitInstance()
{
	 bool AlreadyRunning;
	 HANDLE hMutexOneInstance = ::CreateMutex( NULL, TRUE,
        _T("MICROSIP-088FA840-B10D-11D3-BC36-006067709674"));
    AlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
    if (hMutexOneInstance != NULL) {
        ::ReleaseMutex(hMutexOneInstance);
    }
    if ( AlreadyRunning && hGlobal != NULL) {
		if ( lstrcmp(theApp.m_lpCmdLine, _T("/exit"))==0) {
			::SendMessage(hGlobal, WM_CLOSE, NULL, NULL);
		} else if ( lstrcmp(theApp.m_lpCmdLine, _T("/minimized"))==0) {
		} else if ( lstrcmp(theApp.m_lpCmdLine, _T("/hidden"))==0) {
		} else {
			::ShowWindow(hGlobal, SW_SHOW);
			::SetForegroundWindow(hGlobal);
			if (lstrlen(theApp.m_lpCmdLine)) {
				COPYDATASTRUCT cd;
				cd.dwData = 1;
				cd.lpData = theApp.m_lpCmdLine;
				cd.cbData = sizeof(TCHAR) * (lstrlen(theApp.m_lpCmdLine) + 1);
				::SendMessage(hGlobal, WM_COPYDATA, NULL, (LPARAM)&cd);
			}
		}
        return FALSE;
	} else {
		if ( lstrcmp(theApp.m_lpCmdLine, _T("/exit"))==0) {
			return FALSE;
		}
	}

#ifdef _GLOBAL_PRIORITY
   SetPriorityClass(GetCurrentProcess(), _GLOBAL_PRIORITY);
#endif

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	// Set this to include all the common control classes you want to use
	// in your application.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES |
		ICC_LINK_CLASS | 
		ICC_BAR_CLASSES | 
		ICC_LINK_CLASS | 
		ICC_STANDARD_CLASSES | 
		ICC_TAB_CLASSES | 
		ICC_UPDOWN_CLASS;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	//AfxEnableControlContainer();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	AfxInitRichEdit2();
		
	CmicrosipDlg *microsipDlg = new CmicrosipDlg;
	m_pMainWnd = microsipDlg;
	hGlobal = m_pMainWnd->m_hWnd;

	//--
	LRESULT pResult;
	microsipDlg->OnTcnSelchangeTab(NULL, &pResult);
#ifndef _GLOBAL_NO_ACCOUNT
#ifdef _GLOBAL_ACCOUNT_MINI
	if (
#ifndef _GLOBAL_NO_AUTO_REGISTER
#ifndef _GLOBAL_API
		accountSettings.account.username.IsEmpty()
#else
		accountSettings.account.apiLogin.IsEmpty()
#endif
#else
		1
#endif
		)
	{
		microsipDlg->OnMenuAccountAdd();
	}
#endif
#endif
#ifdef _GLOBAL_CONFERENCE_DIALOG
	new ConferenceDlg(microsipDlg);
#endif

	if (microsipDlg->m_startMinimized) {
		m_pMainWnd->ShowWindow(SW_HIDE);
	}

	microsipDlg->onPowerBroadcast(PBT_APMRESUMEAUTOMATIC, NULL);

	//--

	if (m_pMainWnd)
      return TRUE;
   else
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
