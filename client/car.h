#if !defined(AFX_CAR_H__4370C8AE_BB92_463F_B5F1_95FF2108A4A2__INCLUDED_)
#define AFX_CAR_H__4370C8AE_BB92_463F_B5F1_95FF2108A4A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// car.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// car dialog

class car : public CDialog
{
// Construction
public:
	car(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(car)
	enum { IDD = IDD_CARDROOM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(car)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(car)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAR_H__4370C8AE_BB92_463F_B5F1_95FF2108A4A2__INCLUDED_)
