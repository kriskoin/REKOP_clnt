// car.cpp : implementation file
//

#include "stdafx.h"
#include "client.h"
#include "car.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// car dialog


car::car(CWnd* pParent /*=NULL*/)
	: CDialog(car::IDD, pParent)
{
	//{{AFX_DATA_INIT(car)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void car::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(car)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(car, CDialog)
	//{{AFX_MSG_MAP(car)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// car message handlers
