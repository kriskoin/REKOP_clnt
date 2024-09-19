#if !defined(AFX_LISTITEMS_H__4269F783_375B_4ADE_BB87_8A292C46CB5C__INCLUDED_)
#define AFX_LISTITEMS_H__4269F783_375B_4ADE_BB87_8A292C46CB5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CListItem;

/////////////////////////////////////////////////////////////////////////////
// CListItems wrapper class

class CListItems : public COleDispatchDriver
{
public:
	CListItems() {}		// Calls COleDispatchDriver default constructor
	CListItems(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CListItems(const CListItems& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetCount();
	void SetCount(long nNewValue);
	CListItem Add(VARIANT* Index, VARIANT* Key, VARIANT* Text, VARIANT* Icon, VARIANT* SmallIcon);
	void Clear();
	CListItem GetItem(VARIANT* Index);
	void Remove(VARIANT* Index);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTITEMS_H__4269F783_375B_4ADE_BB87_8A292C46CB5C__INCLUDED_)
