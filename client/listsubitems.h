#if !defined(AFX_LISTSUBITEMS_H__91E18ABD_7C0C_4CBD_BBEC_F7E20D9AD4FA__INCLUDED_)
#define AFX_LISTSUBITEMS_H__91E18ABD_7C0C_4CBD_BBEC_F7E20D9AD4FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CListSubItem;

/////////////////////////////////////////////////////////////////////////////
// CListSubItems wrapper class

class CListSubItems : public COleDispatchDriver
{
public:
	CListSubItems() {}		// Calls COleDispatchDriver default constructor
	CListSubItems(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CListSubItems(const CListSubItems& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetCount();
	void SetCount(long nNewValue);
	CListSubItem Add(VARIANT* Index, VARIANT* Key, VARIANT* Text, VARIANT* ReportIcon, VARIANT* ToolTipText);
	void Clear();
	CListSubItem GetItem(VARIANT* Index);
	void Remove(VARIANT* Index);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTSUBITEMS_H__91E18ABD_7C0C_4CBD_BBEC_F7E20D9AD4FA__INCLUDED_)
