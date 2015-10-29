#pragma once

#include "resource.h"
#include "const.h"
#include "settings.h"

#ifndef _GLOBAL_NO_ACCOUNT

class AccountDlg :
	public CDialog
{
public:
	//CFont m_font;
	AccountDlg(CWnd* pParent = NULL);	// standard constructor
	~AccountDlg();
#ifndef _GLOBAL_ACCOUNT_MINI
	enum { IDD = IDD_ACCOUNT };
#else
#ifndef _GLOBAL_ACCOUNT_PIN
#ifndef _GLOBAL_ACCOUNT_REG
	enum { IDD = IDD_ACCOUNT_CUSTOM };
#else
	enum { IDD = IDD_ACCOUNT_REG };
#endif
#else
	enum { IDD = IDD_ACCOUNT_PIN };
#endif
#endif

	int accountId;
	Account account;

	void Load(int id);

protected:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedDelete();
#ifndef _GLOBAL_CUSTOM
	afx_msg void OnNMClickSyslinkSipServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkSipProxy(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkUsername(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkDomain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkAuthID(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkPassword(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkName(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkEncryption(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkTransport(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkPublicAddress(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkLocalPort(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkPublishPresence(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkStunServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkIce(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSyslinkRewrite(NMHDR *pNMHDR, LRESULT *pResult);
#endif
#if !defined _GLOBAL_ACCOUNT_PASSWORD && !defined _GLOBAL_NO_DISPLAY_PASSWORD
	afx_msg void OnNMClickSyslinkDisplayPasswod(NMHDR *pNMHDR, LRESULT *pResult);
#endif
	afx_msg void OnNMClickSyslinkDelete(NMHDR *pNMHDR, LRESULT *pResult);
#ifdef _GLOBAL_ACCOUNT_REG
	afx_msg void OnNMClickSyslinkReg(NMHDR *pNMHDR, LRESULT *pResult);
#endif
};
#endif