#include "StdAfx.h"
#include "MessagesDlg.h"
#include "microsip.h"
#include "microsipDlg.h"
#include "settings.h"
#include "Transfer.h"

static CmicrosipDlg *microsipDlg;

static DWORD __stdcall MEditStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CString sThisWrite;
	sThisWrite.GetBufferSetLength(cb);

	CString *psBuffer = (CString *)dwCookie;

	for (int i=0;i<cb;i++)
	{
		sThisWrite.SetAt(i,*(pbBuff+i));
	}

	*psBuffer += sThisWrite;

	*pcb = sThisWrite.GetLength();
	sThisWrite.ReleaseBuffer();
	return 0;
}

static DWORD __stdcall MEditStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    CString *psBuffer = (CString*)dwCookie;
 
    if (cb > psBuffer->GetLength()) cb = psBuffer->GetLength();
 
    for (int i = 0; i < cb; i++)
    {
        *(pbBuff + i) = psBuffer->GetAt(i);
    }
 
    *pcb = cb;
    *psBuffer = psBuffer->Mid(cb);
 
    return 0;
}

MessagesDlg::MessagesDlg(CWnd* pParent /*=NULL*/)
: CBaseDialog(MessagesDlg::IDD, pParent)
{
	this->m_hWnd = NULL;
	microsipDlg = (CmicrosipDlg* )pParent;
	Create (IDD, pParent);
}

MessagesDlg::~MessagesDlg(void)
{
}

void MessagesDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, tabComponent);
}

BOOL MessagesDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	AutoMove(IDC_TAB,0,0,100,0);
	AutoMove(IDC_LAST_CALL,100,0,0,0);
	AutoMove(IDC_CLOSE_ALL,100,0,0,0);
	AutoMove(IDC_TRANSFER,100,0,0,0);
	AutoMove(IDC_HOLD,100,0,0,0);
	AutoMove(IDC_END,100,0,0,0);
	AutoMove(IDC_LIST,0,0,100,80);
	AutoMove(IDC_MESSAGE,0,80,100,20);

	lastCall = NULL;
	tab = &tabComponent;
	
	HICON m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon, FALSE);
	
	TranslateDialog(this->m_hWnd);

#ifndef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(SW_HIDE);
#endif

	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
	richEdit->SetEventMask(richEdit->GetEventMask() | ENM_KEYEVENTS);

	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
    richEditList->SetEventMask(richEdit->GetEventMask() | ENM_MOUSEEVENTS);

	CFont* font = this->GetFont();
	LOGFONT lf;
	font->GetLogFont(&lf);
	lf.lfHeight = 16;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	fontList.CreateFontIndirect(&lf);
	richEditList->SetFont(&fontList);
	lf.lfHeight = 18;
	fontMessage.CreateFontIndirect(&lf);
	richEdit->SetFont(&fontMessage);


	para.cbSize=sizeof(PARAFORMAT2);
	para.dwMask = PFM_STARTINDENT | PFM_LINESPACING | PFM_SPACEBEFORE | PFM_SPACEAFTER;
	para.dxStartIndent=100;
	para.dySpaceBefore=100;
	para.dySpaceAfter=0;
	para.bLineSpacingRule = 5;
	para.dyLineSpacing = 22;
	richEditList->SetParaFormat(para);
	return TRUE;
}


void MessagesDlg::PostNcDestroy()
{
	CBaseDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(MessagesDlg, CBaseDialog)
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_COMMAND(ID_CLOSEALLTABS,OnCloseAllTabs)
	ON_COMMAND(ID_GOTOLASTTAB,OnGoToLastTab)
	ON_COMMAND(ID_COPY,OnCopy)
	ON_COMMAND(ID_SELECT_ALL,OnSelectAll)
	ON_BN_CLICKED(IDCANCEL, &MessagesDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &MessagesDlg::OnBnClickedOk)
	ON_NOTIFY(EN_MSGFILTER, IDC_MESSAGE, &MessagesDlg::OnEnMsgfilterMessage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &MessagesDlg::OnTcnSelchangeTab)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB, &MessagesDlg::OnTcnSelchangingTab)
	ON_MESSAGE(WM_CONTEXTMENU,OnContextMenu)
	ON_MESSAGE(UM_CLOSETAB, &MessagesDlg::OnCloseTab)
	ON_BN_CLICKED(IDC_CALL_END, &MessagesDlg::OnBnClickedCallEnd)
	ON_BN_CLICKED(IDC_VIDEO_CALL, &MessagesDlg::OnBnClickedVideoCall)
	ON_BN_CLICKED(IDC_HOLD, &MessagesDlg::OnBnClickedHold)
	ON_BN_CLICKED(IDC_TRANSFER, &MessagesDlg::OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_END, &MessagesDlg::OnBnClickedEnd)
	ON_BN_CLICKED(IDC_CLOSE_ALL, &MessagesDlg::OnBnClickedCloseAll)
	ON_BN_CLICKED(IDC_LAST_CALL, &MessagesDlg::OnBnClickedLastCall)
END_MESSAGE_MAP()

LRESULT MessagesDlg::OnContextMenu(WPARAM wParam,LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam); 
	int y = GET_Y_LPARAM(lParam); 
	POINT pt = { x, y };
	RECT rc;
	if (x!=-1 || y!=-1) {
		ScreenToClient(&pt);
		GetClientRect(&rc); 
		if (!PtInRect(&rc, pt)) {
			x = y = -1;
		} 
	} else {
		::ClientToScreen((HWND)wParam, &pt);
		x = 10+pt.x;
		y = 10+pt.y;
	}
	if (x!=-1 || y!=-1) {
			CMenu menu;
			menu.LoadMenu(IDR_MENU_TABS);
			CMenu* tracker = menu.GetSubMenu(0);
			TranslateMenu(tracker->m_hMenu);
			tracker->TrackPopupMenu( 0, x, y, this );
			return TRUE;
	}
	return DefWindowProc(WM_CONTEXTMENU,wParam,lParam);
}

void MessagesDlg::OnClose() 
{
	call_hangup_all_noincoming();
	this->ShowWindow(SW_HIDE);
}

void MessagesDlg::OnMove(int x, int y)
{
	if (IsWindowVisible() && !IsZoomed() && !IsIconic()) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.messagesX = cRect.left;
		accountSettings.messagesY = cRect.top;
		microsipDlg->AccountSettingsPendingSave();
	}
}

void MessagesDlg::OnSize(UINT type, int w, int h)
{
	CBaseDialog::OnSize(type, w, h);
	if (this->IsWindowVisible() && type == SIZE_RESTORED) {
		CRect cRect;
		GetWindowRect(&cRect);
		accountSettings.messagesW = cRect.Width();
		accountSettings.messagesH = cRect.Height();
		microsipDlg->AccountSettingsPendingSave();
	}
}

void MessagesDlg::OnBnClickedCancel()
{
	OnClose();
}

void MessagesDlg::OnBnClickedOk()
{
}

MessagesContact* MessagesDlg::AddTab(CString number, CString name, BOOL activate, pjsua_call_info *call_info, BOOL notShowWindow, BOOL ifExists)
{
	MessagesContact* messagesContact;

	SIPURI sipuri;
	ParseSIPURI(number, &sipuri);
	if (!accountSettings.account.domain.IsEmpty() && RemovePort(accountSettings.account.domain) == RemovePort(sipuri.domain) ) {
		sipuri.domain = accountSettings.account.domain;
	}

	number = (sipuri.user.GetLength() ? sipuri.user + _T("@") : _T("")) + sipuri.domain;

	LONG exists = -1;
	for (int i=0; i < tab->GetItemCount(); i++)
	{
		messagesContact = GetMessageContact(i);

		CString compareNumber = messagesContact->number;
#ifdef _GLOBAL_NUMBER_PREFIX
		compareNumber = _T(_GLOBAL_NUMBER_PREFIX) + compareNumber;
#endif
		if (messagesContact->number == number || compareNumber == number) {
			exists=i;
			if (call_info)
			{
				if (messagesContact->callId != -1) {
					if (messagesContact->callId != call_info->id) {
						if (call_info->state != PJSIP_INV_STATE_DISCONNECTED) {
							microsipDlg->PostMessage(MYWM_CALL_ANSWER, (WPARAM)call_info->id, -486);
						}
						return NULL;
					}
				} else {
					messagesContact->callId = call_info->id;
				}
			}
			break;
		}
	}
	if (exists==-1)
	{
		if (ifExists)
		{
			return NULL;
		}
#ifndef _GLOBAL_NO_CONTACTS
		if (!name.GetLength()) {
			name = microsipDlg->pageContacts->GetNameByNumber(number);
#endif
			if (!name.GetLength()) {
				if (!sipuri.name.GetLength())
				{
					name = (sipuri.domain == accountSettings.account.domain ? sipuri.user : number);
				} else 
				{
					name = sipuri.name + _T(" (") + (sipuri.domain == accountSettings.account.domain ? sipuri.user : number) + _T(")");
				}
			}
#ifndef _GLOBAL_NO_CONTACTS
		}
#endif
		messagesContact = new MessagesContact();
		messagesContact->callId = call_info ? call_info->id : -1;
		messagesContact->number = number;
		messagesContact->name = name;
		
		TCITEM item;
		item.mask = TCIF_PARAM | TCIF_TEXT;
		name.Format(_T("   %s  "), name);
		item.pszText=name.GetBuffer();
		item.cchTextMax=0;
		item.lParam = (LPARAM)messagesContact;
		exists = tab->InsertItem(tab->GetItemCount(),&item);
		if (tab->GetCurSel() == exists)
		{
			OnChangeTab(call_info);
		}
	} else
	{
		if (tab->GetCurSel() == exists && call_info)
		{
			UpdateCallButton(messagesContact->callId != -1, call_info);
		}
	}
	//if (tab->GetCurSel() != exists && (activate || !IsWindowVisible()))
	if (tab->GetCurSel() != exists && activate)
	{
		long result;
		OnTcnSelchangingTab(NULL, &result);
		tab->SetCurSel(exists);
		OnChangeTab(call_info);
	}
	if (!IsWindowVisible()) {
		if (!notShowWindow) 
		{
			if (!accountSettings.hidden) {
				ShowWindow(SW_SHOW);
				CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
				GotoDlgCtrl(richEdit);
			}
		}
	}
	return messagesContact;
}

void MessagesDlg::OnChangeTab(pjsua_call_info *p_call_info)
{
	tab->HighlightItem(tab->GetCurSel(),FALSE);

	MessagesContact* messagesContact = GetMessageContact();
	SetWindowText(messagesContact->name);

	if (messagesContact->callId != -1) {
		pjsua_call_info call_info;
		if (!p_call_info) {
			pjsua_call_get_info(messagesContact->callId, &call_info);
			p_call_info = &call_info;
		}
		UpdateCallButton(TRUE, p_call_info);
		if (accountSettings.singleMode
			&&(p_call_info->role==PJSIP_ROLE_UAC ||
				(p_call_info->role==PJSIP_ROLE_UAS &&
				(p_call_info->state == PJSIP_INV_STATE_CONFIRMED
				|| p_call_info->state == PJSIP_INV_STATE_CONNECTING)
				))
			) {
			SIPURI sipuri;
			ParseSIPURI(messagesContact->number, &sipuri);
#ifndef _GLOBAL_ACCOUNT_MINI
			microsipDlg->pageDialer->SetNumber(!sipuri.user.IsEmpty() && sipuri.domain == accountSettings.account.domain ? sipuri.user : messagesContact->number);
#else
			microsipDlg->pageDialer->SetNumber(!sipuri.user.IsEmpty() ? sipuri.user : messagesContact->number);
#endif
		}
	} else {
		UpdateCallButton();
		if (accountSettings.singleMode) {
			microsipDlg->pageDialer->Clear();
		}
	}

	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);

	CString messages = messagesContact->messages;
	EDITSTREAM es;
	es.dwCookie = (DWORD) &messages;
	es.pfnCallback = MEditStreamInCallback; 
	richEditList->StreamIn(SF_RTF, es);

	richEditList->PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	richEdit->SetWindowText(messagesContact->message);

	int nEnd = richEdit->GetTextLengthEx(GTL_NUMCHARS);
	richEdit->SetSel(nEnd, nEnd);
}

void MessagesDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnChangeTab();
	*pResult = 0;
}


void MessagesDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
	CString str;
	int len = richEdit->GetWindowTextLength();
	LPTSTR ptr = str.GetBuffer(len);
	richEdit->GetWindowText(ptr,len+1);
	str.ReleaseBuffer();

	MessagesContact* messagesContact = GetMessageContact();
	messagesContact->message = str;
	*pResult = 0;
}

LRESULT  MessagesDlg::OnCloseTab(WPARAM wParam,LPARAM lParam)
{
	int i=wParam;
	CloseTab(i);
	return TRUE;
}

BOOL MessagesDlg::CloseTab(int i, BOOL safe)
{
	int curSel = tab->GetCurSel();

	MessagesContact* messagesContact = GetMessageContact(i);
	if (messagesContact->callId != -1)
	{
		if (safe) {
			return FALSE;
		}
		call_hangup_fast(messagesContact->callId);
	}
	delete messagesContact;
	tab->DeleteItem(i);
	int count = tab->GetItemCount();
	if (!count) {
		GetDlgItem(IDC_MESSAGE)->SetWindowText(NULL);
		GetDlgItem(IDC_LIST)->SetWindowText(NULL);
		OnClose();
	} else  {
		tab->SetCurSel( curSel < count ? curSel: count-1 );
		OnChangeTab();
	}
	return TRUE;
}

pjsua_call_id MessagesDlg::CallMake(CString number, bool hasVideo, pj_status_t *pStatus)
{
	if (accountSettings.singleMode) {
		call_hangup_all_noincoming();
	}
#ifdef _GLOBAL_VIDEO
	if (hasVideo) {
		microsipDlg->createPreviewWin();
	}
#endif

	microsipDlg->SetSoundDevice(microsipDlg->audio_output);

	pjsua_acc_id acc_id;
	pj_str_t pj_uri;
	SelectSIPAccount(number,acc_id,pj_uri);

	pjsua_call_id call_id;
	
	pjsua_call_setting call_setting;
	pjsua_call_setting_default(&call_setting);
	call_setting.flag = 0;
	call_setting.vid_cnt=hasVideo ? 1 : 0;

	pj_status_t status = pjsua_call_make_call(
		acc_id,
		&pj_uri,
		&call_setting,
		NULL,
		NULL,
		&call_id);
	if (pStatus) {
		*pStatus = status;
	}
	return status == PJ_SUCCESS ? call_id : PJSUA_INVALID_ID;
}

void MessagesDlg::CallStart(bool hasVideo)
{
	MessagesContact* messagesContact = GetMessageContact();
	pj_status_t status;
	pjsua_call_id call_id = CallMake(messagesContact->number,hasVideo, &status);
	if (call_id!=PJSUA_INVALID_ID) {
		messagesContact->callId = call_id;
		UpdateCallButton(TRUE);
	} else {
		CString message = GetErrorMessage(status);
		AddMessage(messagesContact,message);
		if (accountSettings.singleMode) {
			AfxMessageBox(message);
		}
	}
}

void MessagesDlg::OnBnClickedCallEnd()
{
	MessagesContact* messagesContact = GetMessageContact();
	if (messagesContact->callId == -1)
	{
		CallStart();
	}
}

void MessagesDlg::OnEndCall(pjsua_call_info *call_info)
{
	for (int i = 0; i < tab->GetItemCount(); i++)
	{
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId == call_info->id)
		{
			lastCall = messagesContact;
			messagesContact->callId = -1;
			if (tab->GetCurSel()==i)
			{
				UpdateCallButton(FALSE, call_info);
			}
			break;
		}
	}
}

void MessagesDlg::UpdateCallButton(BOOL active, pjsua_call_info *call_info)
{
	GetDlgItem(IDC_CALL_END)->ShowWindow(active? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_END)->ShowWindow(!active? SW_HIDE : SW_SHOW);
#ifdef _GLOBAL_VIDEO
	GetDlgItem(IDC_VIDEO_CALL)->ShowWindow(active? SW_HIDE : SW_SHOW);
#endif
	UpdateHoldButton(call_info);
}

void MessagesDlg::UpdateHoldButton(pjsua_call_info *call_info)
{
#ifndef _GLOBAL_NO_HOLD
	MessagesContact* messagesContact = GetMessageContact();
	if (messagesContact) {
		CButton* button = (CButton*)GetDlgItem(IDC_HOLD);
		CButton* buttonTransfer = (CButton*)GetDlgItem(IDC_TRANSFER);
		CButton* buttonDialer = (CButton*)microsipDlg->pageDialer->GetDlgItem(IDC_HOLD);
		CButton* buttonTransferDialer = (CButton*)microsipDlg->pageDialer->GetDlgItem(IDC_TRANSFER);
		if (messagesContact->callId != -1) {
			if (call_info && messagesContact->callId == call_info->id && call_info->media_cnt>0) {
				buttonTransfer->ShowWindow(SW_SHOW);
				buttonTransferDialer->EnableWindow(TRUE);
				if (call_info->media_status == PJSUA_CALL_MEDIA_ACTIVE
					|| call_info->media_status == PJSUA_CALL_MEDIA_REMOTE_HOLD
					) {
						button->ShowWindow(SW_SHOW);
						button->SetCheck(BST_UNCHECKED);
						buttonDialer->EnableWindow(TRUE);
						buttonDialer->SetCheck(BST_UNCHECKED);
						return;
				} else if (call_info->media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD
					|| call_info->media_status == PJSUA_CALL_MEDIA_NONE) {
						button->ShowWindow(SW_SHOW);
						button->SetCheck(BST_CHECKED);
						buttonDialer->EnableWindow(TRUE);
						buttonDialer->SetCheck(BST_CHECKED);
						return;
				}
			}
		} else {
			button->ShowWindow(SW_HIDE);
			button->SetCheck(BST_UNCHECKED);
			buttonDialer->EnableWindow(FALSE);
			buttonDialer->SetCheck(BST_UNCHECKED);
			buttonTransfer->ShowWindow(SW_HIDE);
			buttonTransferDialer->EnableWindow(FALSE);
		}
	}
#endif
}

void MessagesDlg::Call(BOOL hasVideo)
{
	if (!accountSettings.singleMode || !call_get_count_noincoming())
	{
		MessagesContact* messagesContact = GetMessageContact();
		if (messagesContact->callId == -1)
		{
			CallStart(hasVideo);
		}
	} else {
		microsipDlg->GotoTab(0);
	}
}

void MessagesDlg::AddMessage(MessagesContact* messagesContact, CString message, int type, BOOL blockForeground)
{
	CTime tm = CTime::GetCurrentTime();

	if (type == MSIP_MESSAGE_TYPE_SYSTEM) {
		if ( messagesContact->lastSystemMessage == message && messagesContact->lastSystemMessageTime > tm.GetTime()-2) {
			messagesContact->lastSystemMessageTime = tm;
			return;
		}
		messagesContact->lastSystemMessage = message;
		messagesContact->lastSystemMessageTime = tm;
	} else if (!messagesContact->lastSystemMessage.IsEmpty()) {
		messagesContact->lastSystemMessage = _T("");
	}

	if (IsWindowVisible() && !blockForeground) {
		SetForegroundWindow();
	}
	CRichEditCtrl richEdit;
	MessagesContact* messagesContactSelected = GetMessageContact();

	CRichEditCtrl *richEditList = (CRichEditCtrl *)GetDlgItem(IDC_LIST);
	if (messagesContactSelected != messagesContact) {
		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = 300;
		rect.bottom = 300;
		richEdit.Create(ES_MULTILINE | ES_READONLY | ES_NUMBER | WS_VSCROLL, rect, this, NULL);
		richEdit.SetFont(&fontList);
		richEdit.SetParaFormat(para);	

		CString messages = messagesContact->messages;
		EDITSTREAM es;
		es.dwCookie = (DWORD) &messages;
		es.pfnCallback = MEditStreamInCallback; 
		richEdit.StreamIn(SF_RTF, es);

		richEditList = &richEdit;
	}

	COLORREF color;
	CString name;
	if (type==MSIP_MESSAGE_TYPE_LOCAL) {
		color = RGB (0,0,0);
		if (!accountSettings.account.displayName.IsEmpty()) {
			name = accountSettings.account.displayName;
		}
	} else if (type==MSIP_MESSAGE_TYPE_REMOTE) {
		color = RGB (21,101,206);
		name = messagesContact->name;
		int pos = name.Find(_T(" ("));
		if (pos==-1) {
			pos = name.Find(_T("@"));
		}
		if (pos!=-1) {
			name = name.Mid(0,pos);
		}
	}

	int nBegin;
	CHARFORMAT cf;
	CString str;

	CString time = tm.Format(_T("%X"));

	nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
	richEditList->SetSel(nBegin, nBegin);
	str.Format(_T("[%s]  "),time);
	richEditList->ReplaceSel( str );
	cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;
	cf.crTextColor = RGB (131,131,131);
	cf.dwEffects = 0;
	cf.yHeight = 160;
	richEditList->SetSel(nBegin,-1);
	richEditList->SetSelectionCharFormat(cf);

	if (type != MSIP_MESSAGE_TYPE_SYSTEM) {
		cf.yHeight = 200;
	}
	if (name.GetLength()) {
		nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
		richEditList->SetSel(nBegin, nBegin);
		richEditList->ReplaceSel( name + _T(": "));
		cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;
		cf.crTextColor = color;
		cf.dwEffects = CFE_BOLD;
		richEditList->SetSel(nBegin,-1);
		richEditList->SetSelectionCharFormat(cf);
	}

	nBegin = richEditList->GetTextLengthEx(GTL_NUMCHARS);
	richEditList->SetSel(nBegin, nBegin);
	richEditList->ReplaceSel(message+_T("\r\n"));
	cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_SIZE;

	cf.crTextColor = type == MSIP_MESSAGE_TYPE_SYSTEM ? RGB (131, 131, 131) : color;
	cf.dwEffects = 0;

	richEditList->SetSel(nBegin,-1);
	richEditList->SetSelectionCharFormat(cf);

	if (messagesContactSelected == messagesContact)	{
		richEditList->PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	} else {
		for (int i = 0; i < tab->GetItemCount(); i++) {
			if (messagesContact == GetMessageContact(i))
			{
				tab->HighlightItem(i, TRUE);
				break;
			}
		}
	}

	str=_T("");
	EDITSTREAM es;
	es.dwCookie = (DWORD) &str;
	es.pfnCallback = MEditStreamOutCallback; 
	richEditList->StreamOut(SF_RTF, es);
	messagesContact->messages=str;
}

void MessagesDlg::OnEnMsgfilterMessage(NMHDR *pNMHDR, LRESULT *pResult)
{
	MSGFILTER *pMsgFilter = reinterpret_cast<MSGFILTER *>(pNMHDR);

	if (pMsgFilter->msg == WM_CHAR) {
		if ( pMsgFilter->wParam == VK_RETURN ) {
			CRichEditCtrl* richEdit = (CRichEditCtrl*)GetDlgItem(IDC_MESSAGE);
			CString message;
			int len = richEdit->GetWindowTextLength();
			LPTSTR ptr = message.GetBuffer(len);
			richEdit->GetWindowText(ptr,len+1);
			message.ReleaseBuffer();
			message.Trim();
			if (message.GetLength()) {
				MessagesContact* messagesContact = GetMessageContact();
				if (SendMessage (messagesContact,message) ) {
					richEdit->SetWindowText(NULL);
					GotoDlgCtrl(richEdit);
					AddMessage(messagesContact, message, MSIP_MESSAGE_TYPE_LOCAL);
				}
			}
			*pResult= 1;
			return;
		}
	}
	*pResult = 0;
}

BOOL MessagesDlg::SendMessage(MessagesContact* messagesContact, CString message, CString number)
{
	message.Trim();
	if (message.GetLength())
	{
		pjsua_acc_id acc_id;
		pj_str_t pj_uri;
		SelectSIPAccount(messagesContact?messagesContact->number:number,acc_id,pj_uri);
		pj_str_t pj_message = StrToPjStr ( message );
		pj_status_t status = pjsua_im_send( acc_id, &pj_uri, NULL, &pj_message, NULL, NULL );
		if ( status != PJ_SUCCESS ) {
			if (messagesContact) {
				CString message = GetErrorMessage(status);
				AddMessage(messagesContact,message);
			}
		} else {
			return TRUE;
		}
	}
	return FALSE;
}

MessagesContact* MessagesDlg::GetMessageContact(int i)
{
	if (i ==-1) {
		i = tab->GetCurSel();
	}
	if (i != -1) {
		TCITEM item;
		item.mask = TCIF_PARAM;
		tab->GetItem(i, &item);
		return (MessagesContact*) item.lParam;
	}
	return NULL;
}
void MessagesDlg::OnBnClickedVideoCall()
{
	CallStart(true);
}

void MessagesDlg::OnBnClickedHold()
{
	MessagesContact* messagesContactSelected = GetMessageContact();
	if (messagesContactSelected->callId!=-1) {
		pjsua_call_info info;
		pjsua_call_get_info(messagesContactSelected->callId,&info);
		if (info.media_cnt>0) {
			if (info.media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD || info.media_status == PJSUA_CALL_MEDIA_NONE) {
				pjsua_call_reinvite(messagesContactSelected->callId, PJSUA_CALL_UNHOLD, NULL);
			} else {
				pjsua_call_set_hold(messagesContactSelected->callId, NULL);
			}
		}
	}
}

void MessagesDlg::OnBnClickedTransfer()
{
	if (!microsipDlg->transferDlg)
	{
		microsipDlg->transferDlg = new Transfer(this);
	}
	microsipDlg->transferDlg->SetForegroundWindow();
}

void MessagesDlg::OnBnClickedEnd()
{
	MessagesContact* messagesContact = GetMessageContact();
	call_hangup_fast(messagesContact->callId);
}

void MessagesDlg::OnCloseAllTabs()
{
	int i = 0;
	while (i < tab->GetItemCount()) {
		if (CloseTab(i,TRUE)) {
			i = 0;
		} else {
			i++;
		}
	}
}

void MessagesDlg::OnGoToLastTab()
{
	int i = 0;
	BOOL found = FALSE;
	int lastCallIndex = -1;
	while (i < tab->GetItemCount())	{
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId != -1) {
			found = TRUE;
			if (tab->GetCurSel() != i) {
				long result;
				OnTcnSelchangingTab(NULL, &result);
				tab->SetCurSel(i);
				OnChangeTab();
				break;
			}
		}
		if (messagesContact == lastCall) {
			lastCallIndex = i;
		}
		i++;
	}
	if (!found && lastCallIndex!=-1) {
		if (tab->GetCurSel() != lastCallIndex) {
			long result;
			OnTcnSelchangingTab(NULL, &result);
			tab->SetCurSel(lastCallIndex);
			OnChangeTab();
		}
	}
}

int MessagesDlg::GetCallDuration()
{
	int duration = -1;
	pjsua_call_info call_info;
	int i = 0;
	while (i < tab->GetItemCount()) {
		MessagesContact* messagesContact = GetMessageContact(i);
		if (messagesContact->callId != -1) {
			if (pjsua_call_get_info(messagesContact->callId, &call_info)==PJ_SUCCESS) {
				if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
					duration = call_info.connect_duration.sec;
				}
			}
		}
		i++;
	}
	return duration;
}

void MessagesDlg::OnCopy()
{
	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	richEditList->Copy();
}

void MessagesDlg::OnSelectAll()
{
	CRichEditCtrl* richEditList = (CRichEditCtrl*)GetDlgItem(IDC_LIST);
	richEditList->SetSel(0,-1);
}

void MessagesDlg::OnBnClickedCloseAll()
{
	OnCloseAllTabs();
}

void MessagesDlg::OnBnClickedLastCall()
{
	OnGoToLastTab();
}
