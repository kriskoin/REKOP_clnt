// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "splits.h"

// Dispatch interfaces referenced by this interface
#include "split.h"


/////////////////////////////////////////////////////////////////////////////
// CSplits properties

/////////////////////////////////////////////////////////////////////////////
// CSplits operations

long CSplits::GetCount()
{
	long result;
	InvokeHelper(0x1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CSplit CSplits::GetItem(const VARIANT& Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		&Index);
	return CSplit(pDispatch);
}

CSplit CSplits::Add(short Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		Index);
	return CSplit(pDispatch);
}

void CSplits::Remove(const VARIANT& Index)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Index);
}
