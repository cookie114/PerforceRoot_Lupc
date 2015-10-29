#include "StdAfx.h"
#include "SettingsDlg.h"
#include "microsipDlg.h"
#include "settings.h"
#include "Preview.h"

#ifndef _GLOBAL_NO_SETTINGS

static BOOL prev;

static CmicrosipDlg *microsipDlg;

SettingsDlg::SettingsDlg(CWnd* pParent /*=NULL*/)
: CDialog(SettingsDlg::IDD, pParent)
{
	microsipDlg = (CmicrosipDlg* ) AfxGetMainWnd();	
	Create (IDD, pParent);
	prev = FALSE;
}

SettingsDlg::~SettingsDlg(void)
{
	microsipDlg->settingsDlg = NULL;
}

BOOL SettingsDlg::OnInitDialog()
{
	CComboBox *combobox;
	CComboBox *combobox2;
	unsigned count;
	int i;

	CDialog::OnInitDialog();

	TranslateDialog(this->m_hWnd);
#ifndef _GLOBAL_NO_AUTO
	combobox= (CComboBox*)GetDlgItem(IDC_AUTO_ANSWER);
	combobox->AddString(Translate(_T("No")));
	combobox->AddString(Translate(_T("All calls")));
#ifndef _GLOBAL_ACCOUNT_MINI
	combobox->AddString(Translate(_T("SIP header")));
#endif
	combobox->SetCurSel(accountSettings.autoAnswer);
#endif

	combobox= (CComboBox*)GetDlgItem(IDC_DENY_INCOMING);
	combobox->AddString(Translate(_T("No")));
#ifndef _GLOBAL_ACCOUNT_MINI
	combobox->AddString(Translate(_T("Different user")));
	combobox->AddString(Translate(_T("Different domain")));
	combobox->AddString(Translate(_T("Different user or domain")));
	combobox->AddString(Translate(_T("Different remote domain")));
#endif
	combobox->AddString(Translate(_T("All calls")));
#ifndef _GLOBAL_ACCOUNT_MINI
	if (accountSettings.denyIncoming==_T("user"))
	{
		i=1;
	} else if (accountSettings.denyIncoming==_T("domain"))
	{
		i=2;
	} else if (accountSettings.denyIncoming==_T("address"))
	{
		i=3;
	} else if (accountSettings.denyIncoming==_T("rdomain"))
	{
		i=4;
	} else if (accountSettings.denyIncoming==_T("all"))
	{
		i=5;
	} else
	{
		i=0;
	}
#else
	if (accountSettings.denyIncoming==_T("all"))
	{
		i=1;
	} else
	{
		i=0;
	}
#endif
	combobox->SetCurSel(i);

#if !defined _GLOBAL_NO_CONTACTS && !defined _GLOBAL_ACCOUNT_MINI
	GetDlgItem(IDC_DIRECTORY)->SetWindowText(accountSettings.usersDirectory);
#endif
#ifndef _GLOBAL_SOUND_EVENTS
	((CButton*)GetDlgItem(IDC_LOCAL_DTMF))->SetCheck(accountSettings.localDTMF);
#endif
#ifndef _GLOBAL_SINGLE_MODE
	((CButton*)GetDlgItem(IDC_SINGLE_MODE))->SetCheck(accountSettings.singleMode);
#endif
#ifndef _GLOBAL_NO_LOG
	((CButton*)GetDlgItem(IDC_ENABLE_LOG))->SetCheck(accountSettings.enableLog);
#endif
#ifndef _GLOBAL_NO_VAD
	((CButton*)GetDlgItem(IDC_VAD))->SetCheck(accountSettings.vad);
	((CButton*)GetDlgItem(IDC_EC))->SetCheck(accountSettings.ec);
	((CButton*)GetDlgItem(IDC_FORCE_CODEC))->SetCheck(accountSettings.forceCodec);
#endif
#ifndef _GLOBAL_NO_RINGING_SOUND
	GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(accountSettings.ringingSound);
#endif

	pjmedia_aud_dev_info aud_dev_info[64];
	count = 64;
	pjsua_enum_aud_devs(aud_dev_info, &count);

	combobox= (CComboBox*)GetDlgItem(IDC_MICROPHONE);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);

	for (unsigned i=0;i<count;i++)
	{
		if (aud_dev_info[i].input_count) {
			CString audDevName(aud_dev_info[i].name);
			combobox->AddString( audDevName );
			if (!accountSettings.audioInputDevice.Compare(audDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
		}
	}
	combobox= (CComboBox*)GetDlgItem(IDC_SPEAKERS);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	combobox2= (CComboBox*)GetDlgItem(IDC_RING);
	combobox2->AddString(Translate(_T("Default")));
	combobox2->SetCurSel(0);
	for (unsigned i=0;i<count;i++)
	{
		if (aud_dev_info[i].output_count) {
			CString audDevName(aud_dev_info[i].name);
			combobox->AddString(audDevName);
			combobox2->AddString(audDevName);
			if (!accountSettings.audioOutputDevice.Compare(audDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
			if (!accountSettings.audioRingDevice.Compare(audDevName))
			{
				combobox2->SetCurSel(combobox->GetCount()-1);
			}
		}
	}

	pjsua_codec_info codec_info[64];
#ifndef _GLOBAL_CODECS_HARDCODED
	CListBox *listbox;
	CListBox *listbox2;
	listbox = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS_ALL);
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	count = 64;
	pjsua_enum_codecs(codec_info, &count);
	for (unsigned i=0;i<count;i++)
	{
#ifdef _GLOBAL_CODECS_AVAILABLE
		if (StrStr(_T(_GLOBAL_CODECS_AVAILABLE),PjToStr(&codec_info[i].codec_id)))
		{
#endif
			if (codec_info[i].priority
#ifdef _GLOBAL_CODECS_ENABLED
				&& (!accountSettings.audioCodecs.IsEmpty() || StrStr(_T(_GLOBAL_CODECS_ENABLED),PjToStr(&codec_info[i].codec_id)))
#endif
				)
			{
				listbox2->AddString(PjToStr(&codec_info[i].codec_id));
			} else
			{
				listbox->AddString(PjToStr(&codec_info[i].codec_id));
			}	
#ifdef _GLOBAL_CODECS_AVAILABLE
		}
#endif
	}
#endif

#ifdef _GLOBAL_VIDEO
	((CButton*)GetDlgItem(IDC_DISABLE_H264))->SetCheck(accountSettings.disableH264);
 	((CButton*)GetDlgItem(IDC_DISABLE_H263))->SetCheck(accountSettings.disableH263);
	if (accountSettings.bitrateH264.IsEmpty()) {
		const pj_str_t codec_id = {"H264", 4};
		pjmedia_vid_codec_param param;
		pjsua_vid_codec_get_param(&codec_id, &param);
		accountSettings.bitrateH264.Format(_T("%d"),param.enc_fmt.det.vid.max_bps/1000);
	}
	if (accountSettings.bitrateH263.IsEmpty()) {
		const pj_str_t codec_id = {"H263", 4};
		pjmedia_vid_codec_param param;
		pjsua_vid_codec_get_param(&codec_id, &param);
		accountSettings.bitrateH263.Format(_T("%d"),param.enc_fmt.det.vid.max_bps/1000);
	}
	GetDlgItem(IDC_BITRATE_264)->SetWindowText(accountSettings.bitrateH264);
	GetDlgItem(IDC_BITRATE_263)->SetWindowText(accountSettings.bitrateH263);

	combobox= (CComboBox*)GetDlgItem(IDC_VID_CAP_DEV);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	pjmedia_vid_dev_info vid_dev_info[64];
	count = 64;
	pjsua_vid_enum_devs(vid_dev_info, &count);
	for (unsigned i=0;i<count;i++)
	{
		if (vid_dev_info[i].fmt_cnt && (vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING || vid_dev_info[i].dir==PJMEDIA_DIR_ENCODING_DECODING))
		{
			CString vidDevName(vid_dev_info[i].name);
			combobox->AddString(vidDevName);
			if (!accountSettings.videoCaptureDevice.Compare(vidDevName))
			{
				combobox->SetCurSel(combobox->GetCount()-1);
			}
		}
	}

	combobox= (CComboBox*)GetDlgItem(IDC_VIDEO_CODEC);
	combobox->AddString(Translate(_T("Default")));
	combobox->SetCurSel(0);
	count = 64;
	pjsua_vid_enum_codecs(codec_info, &count);
	for (unsigned i=0;i<count;i++)
	{
		combobox->AddString(PjToStr(&codec_info[i].codec_id));
		if (!accountSettings.videoCodec.Compare(PjToStr(&codec_info[i].codec_id)))
		{
			combobox->SetCurSel(combobox->GetCount()-1);
		}
	}
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	((CButton*)GetDlgItem(IDC_DISABLE_LOCAL))->SetCheck(accountSettings.disableLocalAccount);
#endif

#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	combobox= (CComboBox*)GetDlgItem(IDC_UPDATES_INTERVAL);
	combobox->AddString(Translate(_T("Daily")));
	combobox->AddString(Translate(_T("Weekly")));
	combobox->AddString(Translate(_T("Monthly")));
	combobox->AddString(Translate(_T("Quarterly")));
	combobox->AddString(Translate(_T("Never")));
	if (accountSettings.updatesInterval==_T("daily"))
	{
		i=0;
	} else if (accountSettings.updatesInterval==_T("monthly"))
	{
		i=2;
	} else if (accountSettings.updatesInterval==_T("quarterly"))
	{
		i=3;
	} else if (accountSettings.updatesInterval==_T("never"))
	{
		i=4;
	} else
	{
		i=1;
	}
	combobox->SetCurSel(i);
#endif

	return TRUE;
}

void SettingsDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BEGIN_MESSAGE_MAP(SettingsDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &SettingsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &SettingsDlg::OnBnClickedOk)
#ifndef _GLOBAL_CODECS_HARDCODED
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MODIFY, &SettingsDlg::OnDeltaposSpinModify)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ORDER, &SettingsDlg::OnDeltaposSpinOrder)
#endif
#ifndef _GLOBAL_CUSTOM
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_RINGING_SOUND, &SettingsDlg::OnNMClickSyslinkRingingSound)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUTO_ANSWER, &SettingsDlg::OnNMClickSyslinkAutoAnswer)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DENY_INCOMING, &SettingsDlg::OnNMClickSyslinkDenyIncoming)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DIRECTORY, &SettingsDlg::OnNMClickSyslinkDirectory)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_LOCAL_DTMF, &SettingsDlg::OnNMClickSyslinkLocalDTMF)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_SINGLE_MODE, &SettingsDlg::OnNMClickSyslinkSingleMode)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_VAD, &SettingsDlg::OnNMClickSyslinkVAD)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_EC, &SettingsDlg::OnNMClickSyslinkEC)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_FORCE_CODEC, &SettingsDlg::OnNMClickSyslinkForceCodec)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_H264, &SettingsDlg::OnNMClickSyslinkDisableH264)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_H263, &SettingsDlg::OnNMClickSyslinkDisableH263)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_AUDIO_CODECS, &SettingsDlg::OnNMClickSyslinkAudioCodecs)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_ENABLE_LOG, &SettingsDlg::OnNMClickSyslinkEnableLog)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_DISABLE_LOCAL, &SettingsDlg::OnNMClickSyslinkDisableLocal)
#endif
#ifdef _GLOBAL_VIDEO
	ON_BN_CLICKED(IDC_PREVIEW, &SettingsDlg::OnBnClickedPreview)
#endif
#ifndef _GLOBAL_NO_RINGING_SOUND
	ON_BN_CLICKED(IDC_BROWSE, &SettingsDlg::OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_RINGING_SOUND, &SettingsDlg::OnEnChangeRingingSound)
	ON_BN_CLICKED(IDC_DEFAULT, &SettingsDlg::OnBnClickedDefault)
#endif
END_MESSAGE_MAP()


void SettingsDlg::OnClose() 
{
	DestroyWindow();
}

void SettingsDlg::OnBnClickedCancel()
{
	OnClose();
}

void SettingsDlg::OnBnClickedOk()
{
	this->ShowWindow(SW_HIDE);
	microsipDlg->PJDestroy();

	CComboBox *combobox;
	int i;
#ifndef _GLOBAL_NO_AUTO
	combobox= (CComboBox*)GetDlgItem(IDC_AUTO_ANSWER);
	accountSettings.autoAnswer = combobox->GetCurSel();
#endif

	combobox= (CComboBox*)GetDlgItem(IDC_DENY_INCOMING);
	i = combobox->GetCurSel();
#ifndef _GLOBAL_ACCOUNT_MINI
	switch (i) {
		case 1:
			accountSettings.denyIncoming=_T("user");
			break;
		case 2:
			accountSettings.denyIncoming=_T("domain");
			break;
		case 3:
			accountSettings.denyIncoming=_T("address");
			break;
		case 4:
			accountSettings.denyIncoming=_T("rdomain");
			break;
		case 5:
			accountSettings.denyIncoming=_T("all");
			break;
		default:
			accountSettings.denyIncoming=_T("");
	}
#else
	accountSettings.denyIncoming=i?_T("all"):_T("");
#endif
#if !defined _GLOBAL_NO_CONTACTS && !defined _GLOBAL_ACCOUNT_MINI
	GetDlgItem(IDC_DIRECTORY)->GetWindowText(accountSettings.usersDirectory);
	accountSettings.usersDirectory.Trim();
#endif
#ifndef _GLOBAL_SOUND_EVENTS
	accountSettings.localDTMF=((CButton*)GetDlgItem(IDC_LOCAL_DTMF))->GetCheck();
#endif
#ifndef _GLOBAL_SINGLE_MODE
	accountSettings.singleMode=((CButton*)GetDlgItem(IDC_SINGLE_MODE))->GetCheck();
#endif
#ifndef _GLOBAL_NO_LOG
	accountSettings.enableLog=((CButton*)GetDlgItem(IDC_ENABLE_LOG))->GetCheck();
#endif
#ifndef _GLOBAL_NO_VAD
	accountSettings.vad=((CButton*)GetDlgItem(IDC_VAD))->GetCheck();
	accountSettings.ec=((CButton*)GetDlgItem(IDC_EC))->GetCheck();
	accountSettings.forceCodec =((CButton*)GetDlgItem(IDC_FORCE_CODEC))->GetCheck();
#endif

	GetDlgItem(IDC_MICROPHONE)->GetWindowText(accountSettings.audioInputDevice);
	if (accountSettings.audioInputDevice==Translate(_T("Default")))
	{
		accountSettings.audioInputDevice = _T("");
	}

	GetDlgItem(IDC_SPEAKERS)->GetWindowText(accountSettings.audioOutputDevice);
	if (accountSettings.audioOutputDevice==Translate(_T("Default")))
	{
		accountSettings.audioOutputDevice = _T("");
	}

	GetDlgItem(IDC_RING)->GetWindowText(accountSettings.audioRingDevice);
	if (accountSettings.audioRingDevice==Translate(_T("Default")))
	{
		accountSettings.audioRingDevice = _T("");
	}

#ifndef _GLOBAL_CODECS_HARDCODED
	accountSettings.audioCodecs = _T("");
	CListBox *listbox2;
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	for (unsigned i = 0; i < listbox2->GetCount(); i++)
	{
		CString str;
		listbox2->GetText(i, str);
		accountSettings.audioCodecs += str + _T(" ");
	}
	accountSettings.audioCodecs.Trim();
#endif

#ifdef _GLOBAL_VIDEO
	accountSettings.disableH264=((CButton*)GetDlgItem(IDC_DISABLE_H264))->GetCheck();
	accountSettings.disableH263=((CButton*)GetDlgItem(IDC_DISABLE_H263))->GetCheck();
	GetDlgItem(IDC_BITRATE_264)->GetWindowText(accountSettings.bitrateH264);
	if (!atoi(CStringA(accountSettings.bitrateH264))) {
		accountSettings.bitrateH264=_T("");
	}
	GetDlgItem(IDC_BITRATE_263)->GetWindowText(accountSettings.bitrateH263);
	if (!atoi(CStringA(accountSettings.bitrateH263))) {
		accountSettings.bitrateH263=_T("");
	}
	GetDlgItem(IDC_VID_CAP_DEV)->GetWindowText(accountSettings.videoCaptureDevice);
	if (accountSettings.videoCaptureDevice==Translate(_T("Default")))
	{
		accountSettings.videoCaptureDevice = _T("");
	}

	GetDlgItem(IDC_VIDEO_CODEC)->GetWindowText(accountSettings.videoCodec);
	if (accountSettings.videoCodec==Translate(_T("Default")))
	{
		accountSettings.videoCodec = _T("");
	}
#endif

#ifndef _GLOBAL_NO_RINGING_SOUND
	GetDlgItem(IDC_RINGING_SOUND)->GetWindowText(accountSettings.ringingSound);
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	accountSettings.disableLocalAccount=((CButton*)GetDlgItem(IDC_DISABLE_LOCAL))->GetCheck();
#endif

#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	combobox= (CComboBox*)GetDlgItem(IDC_UPDATES_INTERVAL);
	i = combobox->GetCurSel();
	switch (i) {
		case 0:
			accountSettings.updatesInterval=_T("daily");
			break;
		case 2:
			accountSettings.updatesInterval=_T("monthly");
			break;
		case 3:
			accountSettings.updatesInterval=_T("quarterly");
			break;
		case 4:
			accountSettings.updatesInterval=_T("never");
			break;
		default:
			accountSettings.updatesInterval=_T("");
	}
#endif

	accountSettings.SettingsSave();
	microsipDlg->PJCreate();
	microsipDlg->PJAccountAddLocal();
	microsipDlg->PJAccountAdd();

	OnClose();
}

#ifndef _GLOBAL_NO_RINGING_SOUND
void SettingsDlg::OnBnClickedBrowse()
{
	CFileDialog dlgFile( TRUE, _T("wav"), 0, OFN_NOCHANGEDIR, _T("WAV Files (*.wav)|*.wav|") );
	if (dlgFile.DoModal()==IDOK) {
		CString cwd;
		LPTSTR ptr = cwd.GetBuffer(MAX_PATH);
		::GetCurrentDirectory(MAX_PATH, ptr);
		cwd.ReleaseBuffer();
		if ( cwd.MakeLower() + _T("\\") + dlgFile.GetFileName().MakeLower() == dlgFile.GetPathName().MakeLower() ) {
			GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(dlgFile.GetFileName());
		} else {
			GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(dlgFile.GetPathName());
		}
	}
}

void SettingsDlg::OnEnChangeRingingSound()
{
	CString str;
	GetDlgItem(IDC_RINGING_SOUND)->GetWindowText(str);
	GetDlgItem(IDC_DEFAULT)->EnableWindow(str.GetLength()>0);
}

void SettingsDlg::OnBnClickedDefault()
{
	GetDlgItem(IDC_RINGING_SOUND)->SetWindowText(NULL);
}
#endif

#ifndef _GLOBAL_CODECS_HARDCODED
void SettingsDlg::OnDeltaposSpinModify(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CListBox *listbox;
	CListBox *listbox2;
	listbox = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS_ALL);
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	if (pNMUpDown->iDelta == -1) {
		//add
		int selected = listbox->GetCurSel();
		if (selected != LB_ERR) 
		{
			CString str;
			listbox->GetText(selected, str);
			listbox2->AddString(str);
			listbox->DeleteString(selected);
			listbox->SetCurSel( selected < listbox->GetCount() ? selected : selected-1 );
		}
	} else {
		//remove
		int selected = listbox2->GetCurSel();
		if (selected != LB_ERR) 
		{
			CString str;
			listbox2->GetText(selected, str);
			listbox->AddString(str);
			listbox2->DeleteString(selected);
			listbox2->SetCurSel( selected < listbox2->GetCount() ? selected : selected-1 );
		}
	}
	*pResult = 0;
}

void SettingsDlg::OnDeltaposSpinOrder(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CListBox *listbox2;
	listbox2 = (CListBox*)GetDlgItem(IDC_AUDIO_CODECS);
	int selected = listbox2->GetCurSel();
	if (selected != LB_ERR) 
	{
		CString str;
		listbox2->GetText(selected, str);
		if (pNMUpDown->iDelta == -1) {
			//up
			if (selected > 0)
			{
				listbox2->DeleteString(selected);
				listbox2->InsertString(selected-1,str);
				listbox2->SetCurSel(selected-1);
			}
		} else {
			//down
			if (selected < listbox2->GetCount()-1)
			{
				listbox2->DeleteString(selected);
				listbox2->InsertString(selected+1,str);
				listbox2->SetCurSel(selected+1);
			}
		}
	}
	*pResult = 0;
}
#endif

#ifndef _GLOBAL_CUSTOM

void SettingsDlg::OnNMClickSyslinkRingingSound(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ringingSound"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkAutoAnswer(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("autoAnswer"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDenyIncoming(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("denyIncoming"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDirectory(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("directory"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkLocalDTMF(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("soundEvents"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkSingleMode(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("singleMode"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkVAD(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("vad"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkEC(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("ec"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkForceCodec(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("forceCodec"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDisableH264(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("disableH264"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDisableH263(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("disableH263"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkAudioCodecs(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("audioCodecs"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkEnableLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("log"));
	*pResult = 0;
}

void SettingsDlg::OnNMClickSyslinkDisableLocal(NMHDR *pNMHDR, LRESULT *pResult)
{
	OpenHelp(_T("disableLocal"));
	*pResult = 0;
}

#endif

#ifdef _GLOBAL_VIDEO
void SettingsDlg::OnBnClickedPreview()
{
	CComboBox *combobox;
	combobox = (CComboBox*)GetDlgItem(IDC_VID_CAP_DEV);
	CString name;
	combobox->GetWindowText(name);
	if (!microsipDlg->previewWin) {
		microsipDlg->previewWin = new Preview(microsipDlg);
	}
	microsipDlg->previewWin->Start(microsipDlg->VideoCaptureDeviceId(name));
}
#endif


#endif