// stdafx.h : include file for standard system include files,

//  or project specific include files that are used frequently, but

//      are changed infrequently



#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_



#if _MSC_VER > 1000

#pragma once

#endif // _MSC_VER > 1000




#define DEBUG_CONNECT_SERVER 0 //0 launch the comm thread  1 try to connect without thread
#define ADMIN_CLIENT   (_DEBUG)


#if ADMIN_CLIENT

  #define USE_RICH_EDIT_FOR_CHAT_MONITOR	0	//20001120MB

#endif



#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers

#if ADMIN_CLIENT

  #define _WIN32_IE 0x0300	// For commctrl.h: 0x300=IE3+, etc. (4.70)

  // For Windows 2000+ stuff

  //#define _WIN32_WINNT	0x0500

  //#define WINVER		0x0500

#else

  #define _WIN32_IE 0x0200	// For commctrl.h: 0x200=NT4, Win95 and later.  IE3, IE4, etc. not required.

#endif



// Windows Header Files:

#include <windows.h>

#include <commctrl.h>

#include <wininet.h>

#if USE_RICH_EDIT_FOR_CHAT_MONITOR

  #include <richedit.h>

#endif // USE_RICH_EDIT_FOR_CHAT_MONITOR



// C RunTime Header Files

#include <stdio.h>	// needed for sprintf()
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <tchar.h>
#include <string.h>
#include <sys\types.h>
#include <time.h>
#include <process.h>


// Local Header Files

#include <gamedata.h>
#include <llip.h>
#include "client.h"



// TODO: reference additional headers your program requires here
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

