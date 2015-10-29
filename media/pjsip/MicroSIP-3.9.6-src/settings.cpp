#include "stdafx.h"
#include "settings.h"
#include "Crypto.h"

using namespace MFC;

AccountSettings accountSettings;
bool firstRun;
bool pj_ready;

AccountSettings::AccountSettings()
{
	CString str;
	LPTSTR ptr;

	accountId = 0;
	//--
	logFile.Format(_T("%s.log"), _T(_GLOBAL_NAME));
	iniFile.Format(_T("%s.ini"), _T(_GLOBAL_NAME));

	CFileStatus rStatus;
	CString appData;
	ptr = appData.GetBuffer(MAX_PATH);
	::GetCurrentDirectory(MAX_PATH, ptr);
	appData.ReleaseBuffer();
	appData += _T("\\");

	CString appDataLocal;
	ptr = appDataLocal.GetBuffer(MAX_PATH);
	SHGetSpecialFolderPath(
		0,
		ptr, 
		CSIDL_LOCAL_APPDATA, 
		FALSE ); 
	appDataLocal.ReleaseBuffer();
	appDataLocal.AppendFormat(_T("\\%s\\"),_T(_GLOBAL_NAME));

	CString appDataRoaming;
	ptr = appDataRoaming.GetBuffer(MAX_PATH);
	SHGetSpecialFolderPath(
		0,
		ptr, 
		CSIDL_APPDATA,
		FALSE ); 
	appDataRoaming.ReleaseBuffer();
	appDataRoaming.AppendFormat(_T("\\%s\\"),_T(_GLOBAL_NAME));

	firstRun = true;
	if (CFile::GetStatus(appData + iniFile, rStatus)) {
		firstRun = false;
	}

	HANDLE h = CreateFile( appData + iniFile, GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ |
		FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (h != INVALID_HANDLE_VALUE )
	{
		LARGE_INTEGER filesize;
		// copy ini from appdata to workdir
		if (::GetFileSizeEx(h, &filesize) && filesize.LowPart==0 ) {
			CloseHandle(h);
			CopyFile(appDataRoaming + iniFile, appData + iniFile,  FALSE);
		} else {
			CloseHandle(h);
		}
		iniFile = appData + iniFile;
		logFile = appData + logFile;
	} else {
		CreateDirectory(appDataRoaming, NULL);
		CreateDirectory(appDataLocal, NULL);
		if (CFile::GetStatus(appDataRoaming + iniFile, rStatus)) {
			firstRun = false;
		} else {
			// move ini file from old location
			MoveFile(appDataLocal + iniFile, appDataRoaming + iniFile);
			// copy ini from workdir to appdata
			CopyFile(appData + iniFile, appDataRoaming + iniFile,  TRUE);
		}
		iniFile = appDataRoaming + iniFile;
		logFile = appDataLocal + logFile;
	}
	//--

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("accountId"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	if (str.IsEmpty()) {
		if (AccountLoad(-1,account)) {
			accountId = 1;
			WritePrivateProfileString(_T("Settings"),_T("accountId"), _T("1"), iniFile);
		}
	} else {
		accountId=atoi(CStringA(str));
		if (accountId>0) {
			if (!AccountLoad(accountId,account)) {
				accountId = 0;
			}
		}
	}

#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	ptr = updatesInterval.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("UpdatesInterval"), NULL, ptr, 256, iniFile);
	updatesInterval.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("CheckUpdatesTime"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	checkUpdatesTime = atoi(CStringA(str));
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("disableLocalAccount"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	disableLocalAccount=str==_T("1")?1:0;
#else
	disableLocalAccount=1;
#endif

#ifndef _GLOBAL_NO_AUTO
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AutoAnswer"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
#ifdef _GLOBAL_AUTO_ANSWER_VALUE
	autoAnswer=str==_T("1")?1:(str==_T("2")?2:(str==_T("0")?0:_GLOBAL_AUTO_ANSWER_VALUE));
#else
	autoAnswer=str==_T("1")?1:(str==_T("2")?2:0);
#endif
#endif

	ptr = userAgent.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("userAgent"), NULL, ptr, 256, iniFile);
	userAgent.ReleaseBuffer();

	ptr = denyIncoming.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("denyIncoming"), NULL, ptr, 256, iniFile);
	denyIncoming.ReleaseBuffer();

#if !defined _GLOBAL_NO_CONTACTS && !defined _GLOBAL_ACCOUNT_MINI
	ptr = usersDirectory.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("usersDirectory"), NULL, ptr, 256, iniFile);
	usersDirectory.ReleaseBuffer();
#endif

#ifndef _GLOBAL_SOUND_EVENTS
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("LocalDTMF"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
#ifdef _GLOBAL_SOUND_EVENTS_VALUE
	localDTMF=str==_T("0")?0:(str==_T("1")?1:_GLOBAL_SOUND_EVENTS_VALUE);
#else
	localDTMF=str==_T("0")?0:1;
#endif
#else
	localDTMF = _GLOBAL_SOUND_EVENTS;
#endif

	ptr = ringingSound.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("RingingSound"), NULL, ptr, 256, iniFile);
	ringingSound.ReleaseBuffer();
	ptr = audioInputDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AudioInputDevice"), NULL, ptr, 256, iniFile);
	audioInputDevice.ReleaseBuffer();
	ptr = audioOutputDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AudioOutputDevice"), NULL, ptr, 256, iniFile);
	audioOutputDevice.ReleaseBuffer();
	ptr = audioRingDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AudioRingDevice"), NULL, ptr, 256, iniFile);
	audioRingDevice.ReleaseBuffer();
	
#ifndef _GLOBAL_CODECS_HARDCODED
	ptr = audioCodecs.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AudioCodecs"), NULL, ptr, 256, iniFile);
	audioCodecs.ReleaseBuffer();
#else
	audioCodecs = _T(_GLOBAL_CODECS_HARDCODED);
#endif

#ifndef _GLOBAL_NO_VAD
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("vad"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	vad = str == "1" ? 1 : 0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("ec"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	ec = str == "1" ? 1 : 0;

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("forceCodec"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
#else
	vad = 0;
	ec = 1;
#endif

#ifdef _GLOBAL_FORCE_CODEC_CHECKED
	forceCodec = str == "0" ? 0 : 1;
#else
	forceCodec = str == "1" ? 1 : 0;
#endif

#ifdef _GLOBAL_VIDEO
	ptr = videoCaptureDevice.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("VideoCaptureDevice"), NULL, ptr, 256, iniFile);
	videoCaptureDevice.ReleaseBuffer();
	ptr = videoCodec.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("VideoCodec"), NULL, ptr, 256, iniFile);
	videoCodec.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("DisableH264"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	disableH264 = str == "1" ? 1 : 0;
	ptr = bitrateH264.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("BitrateH264"), NULL, ptr, 256, iniFile);
	bitrateH264.ReleaseBuffer();
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("DisableH263"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	disableH263 = str == "1" ? 1 : 0;
	ptr = bitrateH263.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("BitrateH263"), NULL, ptr, 256, iniFile);
	bitrateH263.ReleaseBuffer();
#endif

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainX"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainX = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainY"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainY = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainW"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainW = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("mainH"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	mainH = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesX"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesX = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesY"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesY = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesW"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesW = atoi(CStringA(str));
	
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("messagesH"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	messagesH = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth0"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth0 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth1"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth1 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth2"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth2 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth3"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth3 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("callsWidth4"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	callsWidth4 = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("volumeOutput"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	volumeOutput = str.IsEmpty()?100:atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("volumeInput"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
#ifndef _GLOBAL_VOLUME_MIC
	volumeInput = str.IsEmpty()?100:atoi(CStringA(str));
#else
	volumeInput = str.IsEmpty()?_GLOBAL_VOLUME_MIC:atoi(CStringA(str));
#endif

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("ActiveTab"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	activeTab = atoi(CStringA(str));

	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("AlwaysOnTop"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	alwaysOnTop = atoi(CStringA(str));

	ptr = cmdIncomingCall.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdIncomingCall"), NULL, ptr, 256, iniFile);
	cmdIncomingCall.ReleaseBuffer();

	ptr = cmdCallAnswer.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("cmdCallAnswer"), NULL, ptr, 256, iniFile);
	cmdCallAnswer.ReleaseBuffer();

#ifndef _GLOBAL_NO_LOG
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("EnableLog"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	enableLog=str==_T("1")?1:0;
#else
	enableLog=0;
#endif

#ifndef _GLOBAL_SINGLE_MODE
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(_T("Settings"),_T("SingleMode"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	singleMode=str==_T("0")?0:1;
#else
	singleMode = 1;
#endif

	hidden = 0;
}

void AccountSettings::AccountDelete(int id)
{
	CString section;
	section.Format(_T("Account%d"),id);
	WritePrivateProfileStruct(section, NULL, NULL, 0, iniFile);
}

bool AccountSettings::AccountLoad(int id, Account &account)
{
	CString str;
	LPTSTR ptr;

	CString section;
	if (id==-1) {
		section = _T("Settings");
	} else {
		section.Format(_T("Account%d"),id);
	}

#ifdef _GLOBAL_ACCOUNT_SIP_SERVER
	account.server = _T(_GLOBAL_ACCOUNT_SIP_SERVER);
#else
	ptr = account.server.GetBuffer(255);
	GetPrivateProfileString(section,_T("Server"), NULL, ptr, 256, iniFile);
	account.server.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_SIP_PROXY
	account.proxy = _T(_GLOBAL_ACCOUNT_SIP_PROXY);
#else
	ptr = account.proxy.GetBuffer(255);
	GetPrivateProfileString(section,_T("Proxy"), NULL, ptr, 256, iniFile);
	account.proxy.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_SRTP
	account.srtp = _T(_GLOBAL_ACCOUNT_SRTP);
#else
	ptr = account.srtp.GetBuffer(255);
	GetPrivateProfileString(section,_T("SRTP"), NULL, ptr, 256, iniFile);
	account.srtp.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_TRANSPORT
	account.transport = _T(_GLOBAL_ACCOUNT_TRANSPORT);
#else
#ifndef _GLOBAL_ACCOUNT_MINI
	ptr = account.transport.GetBuffer(255);
	GetPrivateProfileString(section,_T("Transport"), NULL, ptr, 256, iniFile);
	account.transport.ReleaseBuffer();
#else
	account.transport=_T("udp");
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	ptr = account.publicAddr.GetBuffer(255);
	GetPrivateProfileString(section,_T("PublicAddr"), NULL, ptr, 256, iniFile);
	account.publicAddr.ReleaseBuffer();
	ptr = account.listenPort.GetBuffer(255);
	GetPrivateProfileString(section,_T("ListenPort"), NULL, ptr, 256, iniFile);
	account.listenPort.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_PUBLISH
	account.publish=_GLOBAL_ACCOUNT_PUBLISH;
#else
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("Publish"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account.publish=str==_T("1")?1:0;
#endif

#ifdef _GLOBAL_ACCOUNT_STUN
	account.stun = _T(_GLOBAL_ACCOUNT_STUN);
#else
	ptr = account.stun.GetBuffer(255);
	GetPrivateProfileString(section,_T("STUN"), NULL, ptr, 256, iniFile);
	account.stun.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_ICE
	account.ice=_GLOBAL_ACCOUNT_ICE;
#else
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("ICE"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account.ice=str==_T("1")?1:0;
#endif

#ifdef _GLOBAL_ACCOUNT_ALLOW_REWRITE
	account.allowRewrite = _GLOBAL_ACCOUNT_ALLOW_REWRITE;
#else
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("allowRewrite"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	account.allowRewrite=str==_T("1")?1:0;
#endif


#if !defined _GLOBAL_ACCOUNT_DOMAIN || defined _GLOBAL_PROFILE
	ptr = account.domain.GetBuffer(255);
	GetPrivateProfileString(section,_T("Domain"), NULL, ptr, 256, iniFile);
	account.domain.ReleaseBuffer();
#ifdef _GLOBAL_PROFILE
	if (account.domain==_T("B")) {
		account.domain = _T(_GLOBAL_PROFILE_B);
#ifdef _GLOBAL_PROFILE_C
	} else if (account.domain==_T("C")) {
		account.domain = _T(_GLOBAL_PROFILE_C);
#endif
	} else {
		account.domain = _T(_GLOBAL_PROFILE_A);
	}
	account.server=account.domain;
	account.proxy=account.domain;
#endif
#else
	account.domain= _T(_GLOBAL_ACCOUNT_DOMAIN);
#endif

#ifdef _GLOBAL_ACCOUNT_LOGIN
	account.authID= _T(_GLOBAL_ACCOUNT_LOGIN);
#else
	ptr = account.authID.GetBuffer(255);
	GetPrivateProfileString(section,_T("AuthID"), NULL, ptr, 256, iniFile);
	account.authID.ReleaseBuffer();
#endif

	CString usernameLocal;
	CString passwordLocal;
#ifdef _GLOBAL_ACCOUNT_USERNAME
	usernameLocal = _T(_GLOBAL_ACCOUNT_USERNAME);
#else
	ptr = usernameLocal.GetBuffer(255);
	GetPrivateProfileString(section,_T("Username"), NULL, ptr, 256, iniFile);
	usernameLocal.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_PASSWORD
	passwordLocal = _T(_GLOBAL_ACCOUNT_PASSWORD);
#else
	ptr = str.GetBuffer(255);
	GetPrivateProfileString(section,_T("PasswordSize"), NULL, ptr, 256, iniFile);
	str.ReleaseBuffer();
	CByteArray arPassword;
	CCrypto crypto;
	if (!str.IsEmpty()) {
		int size = atoi(CStringA(str));
		arPassword.SetSize(size>0?size:16);
		GetPrivateProfileStruct(section,_T("Password"), arPassword.GetData(),arPassword.GetSize(), iniFile);
		if (crypto.DeriveKey((LPCTSTR)_GLOBAL_KEY)) {
			if (!crypto.Decrypt(arPassword,passwordLocal)) {
				str = _T("");
			}
		}
	}
	if (str.IsEmpty()){
		ptr = passwordLocal.GetBuffer(255);
		GetPrivateProfileString(section,_T("Password"), NULL, ptr, 256, iniFile);
		passwordLocal.ReleaseBuffer();
		if (!passwordLocal.IsEmpty() && crypto.DeriveKey((LPCTSTR)_GLOBAL_KEY)) {
			crypto.Encrypt(passwordLocal,arPassword);
			if (arPassword.GetSize()) {
				WritePrivateProfileStruct(section,_T("Password"),arPassword.GetData(),arPassword.GetSize(),iniFile);
				CString str;
				str.Format(_T("%d"),arPassword.GetSize());
				WritePrivateProfileString(section,_T("PasswordSize"),str,iniFile);
			}
		}
	}
#endif

#ifndef _GLOBAL_ACCOUNT_PASSWORD
#ifndef _GLOBAL_ACCOUNT_REMEMBER_PASSWORD_CHECKED
	account.rememberPassword = passwordLocal.GetLength()?1:0;
#else
	account.rememberPassword = !usernameLocal.GetLength() || passwordLocal.GetLength()?1:0;
#endif
#else
	account.rememberPassword = usernameLocal.GetLength()?1:0;
#endif


#ifndef _GLOBAL_API
	account.username = usernameLocal;
	account.password = passwordLocal;
#else
	account.username = _T("");
	account.password = _T("");
	account.apiLogin = usernameLocal;
	account.apiPassword = passwordLocal;
#endif


#ifdef _GLOBAL_HOST_BASED
	DWORD size;
	size = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(account.username.GetBuffer(size),&size);
	account.username.ReleaseBuffer();
	account.password = "12345";
#endif

#ifdef _GLOBAL_API_ID
	ptr = account.apiId.GetBuffer(255);
	GetPrivateProfileString(section,_T("id"), NULL, ptr, 256, iniFile);
	account.apiId.ReleaseBuffer();
#endif

#ifdef _GLOBAL_ACCOUNT_NAME
	account.displayName = _T(_GLOBAL_ACCOUNT_NAME);
#else
	ptr = account.displayName.GetBuffer(255);
	GetPrivateProfileString(section,_T("DisplayName"), NULL, ptr, 256, iniFile);
	account.displayName.ReleaseBuffer();
#endif

	if (id==-1) {
		// delete old
		WritePrivateProfileString(section,_T("Server"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Proxy"), NULL, iniFile);
		WritePrivateProfileString(section,_T("SRTP"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Transport"), NULL, iniFile);
		WritePrivateProfileString(section,_T("PublicAddr"), NULL, iniFile);
		WritePrivateProfileString(section,_T("ListenPort"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Publish"), NULL, iniFile);
		WritePrivateProfileString(section,_T("STUN"), NULL, iniFile);
		WritePrivateProfileString(section,_T("ICE"), NULL, iniFile);
		WritePrivateProfileString(section,_T("allowRewrite"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Domain"), NULL, iniFile);
		WritePrivateProfileString(section,_T("AuthID"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Username"), NULL, iniFile);
		WritePrivateProfileString(section,_T("PasswordSize"), NULL, iniFile);
		WritePrivateProfileString(section,_T("Password"), NULL, iniFile);
		WritePrivateProfileString(section,_T("id"), NULL, iniFile);
		WritePrivateProfileString(section,_T("DisplayName"), NULL, iniFile);
		// save new
		AccountSave(1, &account);
	}

	return !account.domain.IsEmpty() && !account.username.IsEmpty();
}

void AccountSettings::AccountSave(int id, Account *account)
{
	CString section;
	section.Format(_T("Account%d"),id);

#ifndef _GLOBAL_ACCOUNT_SIP_SERVER
	WritePrivateProfileString(section,_T("Server"),account->server,iniFile);
#endif

#ifndef _GLOBAL_ACCOUNT_SIP_PROXY
	WritePrivateProfileString(section,_T("Proxy"),account->proxy,iniFile);
#endif

#ifdef _GLOBAL_PROFILE
	if (account->domain == _T(_GLOBAL_PROFILE_B)) {
		WritePrivateProfileString(section,_T("Domain"),_T("B"),iniFile);
#ifdef _GLOBAL_PROFILE_C
	} else if (account->domain == _T(_GLOBAL_PROFILE_C)) {
		WritePrivateProfileString(section,_T("Domain"),_T("C"),iniFile);
#endif
	} else {
		WritePrivateProfileString(section,_T("Domain"),_T("A"),iniFile);
	}
#else
#ifndef _GLOBAL_ACCOUNT_DOMAIN
	WritePrivateProfileString(section,_T("Domain"),account->domain,iniFile);
#endif
#endif

#ifndef _GLOBAL_ACCOUNT_LOGIN
	WritePrivateProfileString(section,_T("AuthID"),account->authID,iniFile);
#endif

	CString usernameLocal;
	CString passwordLocal;

#ifndef _GLOBAL_API
	usernameLocal = account->username;
	passwordLocal = account->password;
#else
	usernameLocal = account->apiLogin;
	passwordLocal = account->apiPassword;
#endif

	CCrypto crypto;
	CByteArray arPassword;
	if (crypto.DeriveKey((LPCTSTR)_GLOBAL_KEY)) {
		crypto.Encrypt(passwordLocal,arPassword);
	}

	if (!account->rememberPassword) {
#ifndef _GLOBAL_ACCOUNT_USERNAME
		WritePrivateProfileString(section,_T("Username"),_T(""),iniFile);
#endif
#ifndef _GLOBAL_ACCOUNT_PASSWORD
		WritePrivateProfileString(section,_T("Password"),_T(""),iniFile);
		WritePrivateProfileString(section,_T("PasswordSize"),NULL,iniFile);
#endif
	}

	if (account->rememberPassword) {
#ifndef _GLOBAL_ACCOUNT_USERNAME
		WritePrivateProfileString(section,_T("Username"),usernameLocal,iniFile);
#endif
#ifndef _GLOBAL_ACCOUNT_PASSWORD
		if (arPassword.GetSize()) {
			WritePrivateProfileStruct(section,_T("Password"),arPassword.GetData(),arPassword.GetSize(),iniFile);
			CString str;
			str.Format(_T("%d"),arPassword.GetSize());
			WritePrivateProfileString(section,_T("PasswordSize"),str,iniFile);
		} else {
			WritePrivateProfileString(section,_T("Password"),passwordLocal,iniFile);
			WritePrivateProfileString(section,_T("PasswordSize"),NULL,iniFile);
		}
#endif
	}

#ifdef _GLOBAL_API_ID
	WritePrivateProfileString(section,_T("id"),account->apiId,iniFile);
#endif

#ifndef _GLOBAL_ACCOUNT_NAME
	WritePrivateProfileString(section,_T("DisplayName"),account->displayName,iniFile);
#endif

#ifndef _GLOBAL_ACCOUNT_MINI
	WritePrivateProfileString(section,_T("PublicAddr"),account->publicAddr,iniFile);
	WritePrivateProfileString(section,_T("ListenPort"),account->listenPort,iniFile);
	WritePrivateProfileString(section,_T("SRTP"),account->srtp,iniFile);
	WritePrivateProfileString(section,_T("Transport"),account->transport,iniFile);
	WritePrivateProfileString(section,_T("Publish"),account->publish?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(section,_T("STUN"),account->stun,iniFile);
	WritePrivateProfileString(section,_T("ICE"),account->ice?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(section,_T("allowRewrite"),account->allowRewrite?_T("1"):_T("0"),iniFile);
#endif
}

void AccountSettings::SettingsSave()
{
	CString str;

	str.Format(_T("%d"),accountId);
	WritePrivateProfileString(_T("Settings"),_T("accountId"),str,iniFile);

#ifndef _GLOBAL_ACCOUNT_MINI
	WritePrivateProfileString(_T("Settings"),_T("disableLocalAccount"),disableLocalAccount?_T("1"):_T("0"),iniFile);
#endif

#ifndef _GLOBAL_NO_LOG
	WritePrivateProfileString(_T("Settings"),_T("EnableLog"),enableLog?_T("1"):_T("0"),iniFile);
#endif

#if !defined _GLOBAL_CUSTOM || defined _GLOBAL_UPDATES
	WritePrivateProfileString(_T("Settings"),_T("UpdatesInterval"),updatesInterval,iniFile);
	str.Format(_T("%d"),checkUpdatesTime);
	WritePrivateProfileString(_T("Settings"),_T("CheckUpdatesTime"),str,iniFile);
#endif

#ifndef _GLOBAL_NO_AUTO
	WritePrivateProfileString(_T("Settings"),_T("AutoAnswer"),autoAnswer==1?_T("1"):(autoAnswer==2?_T("2"):_T("0")),iniFile);
#endif
	WritePrivateProfileString(_T("Settings"),_T("denyIncoming"),denyIncoming,iniFile);
#if !defined _GLOBAL_NO_CONTACTS && !defined _GLOBAL_ACCOUNT_MINI
	WritePrivateProfileString(_T("Settings"),_T("usersDirectory"),usersDirectory,iniFile);
#endif
#ifndef _GLOBAL_SINGLE_MODE
	WritePrivateProfileString(_T("Settings"),_T("SingleMode"),singleMode?_T("1"):_T("0"),iniFile);
#endif
#ifndef _GLOBAL_SOUND_EVENTS
	WritePrivateProfileString(_T("Settings"),_T("LocalDTMF"),localDTMF?_T("1"):_T("0"),iniFile);
#endif
	WritePrivateProfileString(_T("Settings"),_T("RingingSound"),ringingSound,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("AudioInputDevice"),_T("\"")+audioInputDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("AudioOutputDevice"),_T("\"")+audioOutputDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("AudioRingDevice"),_T("\"")+audioRingDevice+_T("\""),iniFile);
#ifndef _GLOBAL_CODECS_HARDCODED
	WritePrivateProfileString(_T("Settings"),_T("AudioCodecs"),audioCodecs,iniFile);
#endif
#ifndef _GLOBAL_NO_VAD
	WritePrivateProfileString(_T("Settings"),_T("vad"),vad?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("ec"),ec?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("forceCodec"),forceCodec?_T("1"):_T("0"),iniFile);
#endif
#ifdef _GLOBAL_VIDEO
	WritePrivateProfileString(_T("Settings"),_T("VideoCaptureDevice"),_T("\"")+videoCaptureDevice+_T("\""),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("VideoCodec"),videoCodec,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("DisableH264"),disableH264?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("BitrateH264"),bitrateH264,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("DisableH263"),disableH263?_T("1"):_T("0"),iniFile);
	WritePrivateProfileString(_T("Settings"),_T("BitrateH263"),bitrateH263,iniFile);
#endif

	str.Format(_T("%d"),mainX);
	WritePrivateProfileString(_T("Settings"),_T("mainX"),str,iniFile);

	str.Format(_T("%d"),mainY);
	WritePrivateProfileString(_T("Settings"),_T("mainY"),str,iniFile);

	str.Format(_T("%d"),mainW);
	WritePrivateProfileString(_T("Settings"),_T("mainW"),str,iniFile);

	str.Format(_T("%d"),mainH);
	WritePrivateProfileString(_T("Settings"),_T("mainH"),str,iniFile);

	str.Format(_T("%d"),messagesX);
	WritePrivateProfileString(_T("Settings"),_T("messagesX"),str,iniFile);

	str.Format(_T("%d"),messagesY);
	WritePrivateProfileString(_T("Settings"),_T("messagesY"),str,iniFile);

	str.Format(_T("%d"),messagesW);
	WritePrivateProfileString(_T("Settings"),_T("messagesW"),str,iniFile);

	str.Format(_T("%d"),messagesH);
	WritePrivateProfileString(_T("Settings"),_T("messagesH"),str,iniFile);

	str.Format(_T("%d"),callsWidth0);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth0"),str,iniFile);
		
	str.Format(_T("%d"),callsWidth1);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth1"),str,iniFile);

	str.Format(_T("%d"),callsWidth2);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth2"),str,iniFile);

	str.Format(_T("%d"),callsWidth3);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth3"),str,iniFile);

	str.Format(_T("%d"),callsWidth4);
	WritePrivateProfileString(_T("Settings"),_T("callsWidth4"),str,iniFile);

	str.Format(_T("%d"),volumeOutput);
	WritePrivateProfileString(_T("Settings"),_T("volumeOutput"),str,iniFile);

	str.Format(_T("%d"),volumeInput);
	WritePrivateProfileString(_T("Settings"),_T("volumeInput"),str,iniFile);

	str.Format(_T("%d"),activeTab);
	WritePrivateProfileString(_T("Settings"),_T("ActiveTab"),str,iniFile);

	str.Format(_T("%d"),alwaysOnTop);
	WritePrivateProfileString(_T("Settings"),_T("AlwaysOnTop"),str,iniFile);
	
	WritePrivateProfileString(_T("Settings"),_T("cmdIncomingCall"),cmdIncomingCall,iniFile);
	WritePrivateProfileString(_T("Settings"),_T("cmdCallAnswer"),cmdCallAnswer,iniFile);
}
