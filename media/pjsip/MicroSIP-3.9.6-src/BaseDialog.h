#pragma once

#include <vector>

class CBaseDialog : public CDialog
{
	// Construction
public:
	CBaseDialog(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

	void AutoMove(int iID, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct);
	void AutoMove(HWND hWnd, double dXMovePct, double dYMovePct, double dXSizePct, double dYSizePct);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseDialog)
protected:
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CBaseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	struct SMovingChild
	{
		HWND        m_hWnd;
		double      m_dXMoveFrac;
		double      m_dYMoveFrac;
		double      m_dXSizeFrac;
		double      m_dYSizeFrac;
		CRect       m_rcInitial;
	};
	typedef std::vector<SMovingChild>   MovingChildren;

	MovingChildren  m_MovingChildren;
	CSize           m_szInitial;
	CSize           m_szMinimum;
};
