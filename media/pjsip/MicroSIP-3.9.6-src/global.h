#pragma once

#include "const.h"
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>

enum EUserWndMessages
{
	MYWM_FIRST_USER_MSG = (WM_USER + 0x100 + 1),

	MYWM_NOTIFYICON,

	MYWM_CREATE_RINGING,
	MYWM_DESTROY_RINGING,
	MYWM_CALL_ANSWER,
	MYWM_CALL_HANGUP,
	MYWM_ON_ACCOUNT,
	MYWM_ON_CALL_MEDIA_STATE,
	MYWM_ON_PAGER,
	MYWM_ON_PAGER_STATUS,
	MYWM_ON_BUDDY_STATE,
	MYWM_PLAY_SOUND,
	MYWM_SHELL_EXECUTE,
	MYWM_SET_PANE_TEXT,
	
	IDT_TIMER_0,
	IDT_TIMER_1,
	IDT_TIMER_BALANCE,
	IDT_TIMER_INIT_RINGIN,
	IDT_TIMER_CALL,
	IDT_TIMER_CONTACTS_BLINK,
	IDT_TIMER_DIRECTORY,
	IDT_TIMER_SAVE,
	
	UM_CLOSETAB,
	UM_DBLCLICKTAB,
	UM_QUERYTAB,

	ID_ACCOUNT_CHANGE,
	ID_ACCOUNT_EDIT = (ID_ACCOUNT_CHANGE + 100),
	
};


enum {MSIP_MESSAGE_TYPE_LOCAL, MSIP_MESSAGE_TYPE_REMOTE, MSIP_MESSAGE_TYPE_SYSTEM};
enum {MSIP_TRANSPORT_AUTO, MSIP_TRANSPORT_TCP, MSIP_TRANSPORT_TLS};
enum {MSIP_SOUND_STOP, MSIP_SOUND_RING, MSIP_SOUND_RINGIN, MSIP_SOUND_RINGOUT, MSIP_SOUND_CUSTOM};
enum {MSIP_CALL_OUT, MSIP_CALL_IN, MSIP_CALL_MISS};

#ifndef _GLOBAL_CODECS_ENABLED
#define _GLOBAL_CODECS_ENABLED "speex/16000/1 SILK/16000/1 G722/16000/1 PCMA/8000/1 PCMU/8000/1 speex/8000/1 SILK/8000/1 iLBC/8000/1 GSM/8000/1 G729/8000/1"
#endif

#ifndef _GLOBAL_NAME_NICE
#define _GLOBAL_NAME_NICE _GLOBAL_NAME
#endif

#if !defined _GLOBAL_TAB_HELP && !defined _GLOBAL_CUSTOM
#define _GLOBAL_TAB_HELP "http://www.microsip.org/help"
#endif

struct SIPURI {
	CString user;
	CString domain;
	CString name;
};

struct Contact {
	CString number;
	CString name;
	BOOL presence;
	time_t presenceTime;
	BOOL ringing;
	int image;
	BOOL fromDirectory;
	BOOL candidate;
	Contact():presenceTime(0),ringing(FALSE),image(0),candidate(FALSE){}
};

struct MessagesContact {
	CString name;
	CString number;
	CString messages;
	CString message;
	CString lastSystemMessage;
	CTime lastSystemMessageTime;
	pjsua_call_id callId;
	int mediaStatus;
	MessagesContact():mediaStatus(PJSUA_CALL_MEDIA_ERROR){}
};

struct Call {
	int key;
	CString id;
	CString name;
	CString number;
	int type;
	int time;
	int duration;
	CString info;
};


struct my_call_data
{
   pj_pool_t          *pool;
   pjmedia_port       *tonegen;
   pjsua_conf_port_id  toneslot;
};

extern struct my_call_data *tone_gen;
extern int transport;
extern pjsua_acc_id account;

CString GetErrorMessage(pj_status_t status);
BOOL ShowErrorMessage(pj_status_t status);
BOOL IsIP(CString host);
CString RemovePort(CString domain);
void ParseSIPURI(CString in, SIPURI* out);
CString PjToStr(const pj_str_t* str, BOOL utf = FALSE);
pj_str_t StrToPjStr(CString str);
char* StrToPj(CString str);
CString Utf8DecodeUni(CStringA str);
CStringA UnicodeToAnsi(CString str);
void OpenURL(CString url);
CString GetDuration(int sec, bool zero = false);
void AddTransportSuffix(CString &str);
CString GetSIPURI(CString str, bool isSimple = false, bool isLocal = false);
BOOL SelectSIPAccount(CString number, pjsua_acc_id &acc_id, pj_str_t &pj_uri);
CString FormatNumber(CString number);

#ifdef _GLOBAL_API
void APIReadString(CString key, CString *dest, CString *data);
#endif

struct my_call_data *call_init_tonegen(pjsua_call_id call_id);
BOOL call_play_digit(pjsua_call_id call_id, const char *digits);
void call_deinit_tonegen(pjsua_call_id call_id);
void destroyDTMFPlayer(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime
						   ) ;
void call_hangup_fast(pjsua_call_id call_id,pjsua_call_info *p_call_info = NULL);

unsigned call_get_count_noincoming();
void call_hangup_all_noincoming();

#ifndef _GLOBAL_CUSTOM

void OpenHelp(CString code);

#endif

#if defined _GLOBAL_URLENCODE || defined _GLOBAL_ACCOUNT_REG

CStringA urlencode(CStringA str);
CStringA char2hex(char dec);

#endif