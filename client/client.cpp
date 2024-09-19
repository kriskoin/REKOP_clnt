// client.cpp : Defines the entry point for the application.



#define DISP 0



#include "stdafx.h"

#include "resource.h"

#include "client.h"

#include <direct.h>

#include <sys/stat.h>

#include <sys/utime.h>

#include <string.h>





#define RANDOMLY_DISCONNECT	0	// debugging only: randomly disconnect to make sure our reconnection code is working.



#define MAX_LOADSTRING 100





extern int dprintf_Disabled;

extern int DebugPipeHandle;









int iAdminClientFlag;		// Set if this client is running in adminstrator mode





// Global Variables:

HINSTANCE hInst;						// current instance

HWND hInitialWindow;					// window handle for our root window



#if USE_RICH_EDIT_FOR_CHAT_MONITOR

HINSTANCE hInstRich;

#endif





char *szTitle = "e-Media Poker Client - Main Message Window";	// main parent window title

char *szWindowClass = "e-Media Poker Client";	// main parent window class

char *ProgramRegistryPrefix = "e-Media Poker Client";

int ExitNowFlag;			// Set when all threads should exit.

int iProgramInstanceCount;	// # of other application instances running when our instance started.

struct Defaults Defaults;

int AutoLoginFlag;			// set to automatically login (cmd line 'autologin')

int MinimizeWindowsFlag;	// set to minimize our cardroom and table windows on creation



#if ADMIN_CLIENT

  

  int RunningManyFlag;		// set if we're running a lot of clients and they should minimize their resource usage

  extern int iInvalidateEmailAddresses;

  int AutoShowServerStats;	// set if we should automatically show server stats

  int iFlashTitleBars;		// count-down while flashing menu bars.

#endif

int iRDTSCAvailFlag;			// set if the rdtsc() instruction is available on this system

int iUserLoggingOffSystemFlag;	// set if the user is logging off (including to shut down) his system

int iOurAppIsActive;			// set if we're the active application.

int iDisableServerSwitching;	// set to disable switching from server to server.

int iCCProcessingEstimate;		// estimate (in minutes) of how long cc processing will take (from server)

int iMaxTournamentTables;		// admin: current max # of tournament tables allowed to be opened (of each type)



WORD32 dwLastUserActionMessageTime;	// SecondCounter when mouse and or keyboard message last received.

WORD32 dwRandomEntropy;			// 32 bits that we add non-predictable stuff to.

WORD32 dwMoneyInPlay;			// copy from most recent structure from server

WORD32 dwMoneyLoggedIn;			// copy from most recent structure from server



char LoginUserID[MAX_PLAYER_USERID_LEN];

char LoginPassword[MAX_PLAYER_PASSWORD_LEN];

int iRequestPriorityLogin;

int AskedForMorePlayChips = FALSE;	// has player requested more play chips?



volatile WORD32 RealChipsInBank = 0;

volatile WORD32 RealChipsInPlay = 0;

volatile WORD32 FakeChipsInBank = 0;

volatile WORD32 FakeChipsInPlay = 0;

volatile WORD32 PendingCredit = 0;

volatile WORD32 PendingCheck = 0;

volatile WORD32 PendingPaypal = 0;

volatile WORD32 CreditFeePoints = 0;



volatile INT32 GoodRakedGames = 0;

char VendorCode[6];				


int LoggedIn = LOGIN_NO_LOGIN;	// logged-in status (defined in pplib.h as LOGIN_*)

int LoggedInPrivLevel; 			// current priv level if logged in (ACCPRIV_* in gamedata.h)

int LastLoginStatus = -1;		// set to our last known login status

int ExtraSocketsToOpen = 0;		// might be set to something if we're testing

HWND hBuyInDLG = NULL;			// handle to buy-in dialog, if it exists

HWND hEnterChatTextDLG = NULL;	// handle to dlg for entering chat text

HWND hLoginProgress = NULL;		// handle to login progress indicator

HWND hLogInDLG = NULL;			// handle to login dialog, if it exists

HWND hReqHandHistory = NULL;	// handle to request hand history dlg

HWND hCashierDlg = NULL;		// handle to cashier dlg

HWND hTourSummaryDlg = NULL;	// handle to email summary requester



HWND hKeyboardTranslateHwndTable[MAX_TRANSLATOR_HWNDS];



char *szAppName = "e-Media Poker.com client";



PPCRITICAL_SECTION ClientSockCritSec;	// lock around changes to the ClientSock variable

PPCRITICAL_SECTION CardRoomVarsCritSec;

PPCRITICAL_SECTION MessageThreadCritSec;



// Foward declarations of functions included in this code module:

ATOM MyRegisterClass(HINSTANCE hInstance);

BOOL InitInstance(HINSTANCE, int);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK dlgLoginFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncLoggingInStatus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int StartupInit(LPSTR lpCmdLine);	// return T/F if it's to continue

void Shutdown(void);

void RemoveLoginProgressIndicator(void);

static void PPlaySound2(int sound_index);

static void UpdateConnectionStatus(int force_update);

char ConnectionStatusString[100];



struct CardRoom_JoinTable PendingJoinTableResults[MAX_TABLES];	// structs received by comm thread but not yet processed by message thread



// .INI file entries

ParmStruc INIParms[] = {

	// type				size	token name					address

	PARMTYPE_INT,		0,		"PortNumber",				&PortNumber,

#if INCL_SSL_SUPPORT

	PARMTYPE_INT,		0,		"PortNumber_SSL",			&PortNumber_SSL,

#endif

	PARMTYPE_INT,		0,		"InitialConnectionTimeout",	&InitialConnectionTimeout,

	PARMTYPE_STRING,	MAX_SERVERNAME_LEN, "ServerName",	ServerName,



	PARMTYPE_INT,0,0,0		// mark end.

};

int PortNumber = 28500;

//int PortNumber = 26000;

#if INCL_SSL_SUPPORT



//int PortNumber_SSL=26005;

int PortNumber_SSL = 28502;



#endif

int InitialConnectionTimeout=30;		// # of seconds after connect() before we give up on a socket.

char ServerName[MAX_SERVERNAME_LEN];	// name of server.

// return codes from Startup

enum StartupRC { STARTUP_NORMAL, STARTUP_AUTO_LOGIN, STARTUP_BAD_SHUTDOWN } ;



//****************************************************************

//

//	Determine how many instances of our program are running.

//

//	See the Win32 docs under "Identifying a Previous Instance of an

//	Application" for more info.

//

//  Sets iProgramInstanceCount to # of other application instances

//	running when this function was called (0 if we're the only one).

//

//	This function may only be called ONCE when the program starts.

//	Never call it twice.

//

void CountProgramInstances(void)

{

	iProgramInstanceCount = 0;



	// Create a named Mutex.  We don't care about the handle because

	// we'll never close it (let the OS handle that when we close).

	forever {

		char fname[MAX_FNAME_LEN];

		sprintf(fname, "%s MutEx %d", szAppName, iProgramInstanceCount);

		CreateMutex(NULL, TRUE, fname);

		int last_error = GetLastError();

		if (last_error!=ERROR_ALREADY_EXISTS) {

			break;	// we found a free instance count.

		}

		iProgramInstanceCount++;

	}

	pr(("%s(%d) %s instance count = %d\n", _FL, szAppName, iProgramInstanceCount));

}



//****************************************************************

//

//	Activate an existing instance of our program.

//

void ActivatePreviousInstance(void)

{

	// Find the other window and top it.

	HWND other_window = FindWindow(szAppName, NULL);

	if (other_window != NULL) {

		ShowWindow(other_window, SW_SHOWNORMAL);

		ReallySetForegroundWindow(other_window);

	}

}



//*********************************************************


//

// Retrieve our program's version number from the .EXE

// file's VS_VERSION_INFO resource.

//

void GetClientVersionNumber(void)

{

	#define MAX_EXE_PATH_LEN	200

	char exe_path[MAX_EXE_PATH_LEN];



	int chars = GetModuleFileName(NULL, exe_path, MAX_EXE_PATH_LEN-1);

	exe_path[chars] = 0;



	zstruct(ClientVersionNumber);

	DWORD zero = 0;

	DWORD len = GetFileVersionInfoSize(exe_path, &zero);

	if (!len) {

		Error(ERR_ERROR, "%s(%d) GetFileVersionInfoLen failed. err=%d",_FL,GetLastError());

		return;

	}

	void *vptr = malloc(len);

	if (!vptr) {

		Error(ERR_ERROR, "%s(%d) Could not malloc(%d)", _FL, len);

		return;

	}

	if (!GetFileVersionInfo(exe_path, 0, len, vptr)) {

		Error(ERR_ERROR, "%s(%d) GetFileVersionInfo failed. err=%d",_FL,GetLastError());

		free(vptr);

		return;

	}



	// We've got it all loaded in... extract some fields from it.

	VS_FIXEDFILEINFO *vsffi;

	unsigned int vsffi_len;

	if (!VerQueryValue(vptr, "\\", (void **)&vsffi, &vsffi_len)) {

		Error(ERR_ERROR, "%s(%d) VerQueryValue failed. err=%d",_FL,GetLastError());

		free(vptr);

		return;

	}

	ClientVersionNumber.major = BYTE8(vsffi->dwFileVersionMS >> 16);

	ClientVersionNumber.minor = BYTE8(vsffi->dwFileVersionMS);

	ClientVersionNumber.build = (ClientVersionNumber.major<<24) |

								(ClientVersionNumber.minor<<16) |

								WORD16(vsffi->dwFileVersionLS);


/************J Fonseca   03/06/2004****************/
//
// Guarda la version del cliente en el Registro de Windows

    char str[256];
	HKEY keyhandle;
	DWORD disposition;
	LONG result;
	sprintf(str, "Software\\e-Media Poker\\%s", ProgramRegistryPrefix);
	result = RegCreateKeyEx(HKEY_CURRENT_USER,

				str, 0, "Data", 0, KEY_ALL_ACCESS,

				NULL, &keyhandle, &disposition);

	if (result==ERROR_SUCCESS) {
		sprintf(str,"%d.%02d",ClientVersionNumber.major,ClientVersionNumber.minor);			
		RegSetValueEx(keyhandle,"Version", 0, REG_SZ, (CONST UCHAR *)str, 5);
		RegCloseKey(keyhandle);
	}
/************J Fonseca   03/06/2004****************/
    

	ClientVersionNumber.flags = 0;

	pr(("%s(%d) Client version is %d.%02d, build $%08lx (%d)\n",

				_FL, ClientVersionNumber.major,ClientVersionNumber.minor,

				ClientVersionNumber.build, ClientVersionNumber.build & 0x0000FFFF));

	free(vptr);

}



//*********************************************************

//

// Pick a good window for a random message.  It might be for

// misc client message, a seat avail message, or who knows what.

// If there is a table active, make the table the owner of the message

// rather than the lobby.  If the lobby pops up too much while playing, it's

// really annoying.

//

HWND PickWindowForRandomMessage(HWND default_hwnd)

{

	HWND foreground_hwnd = GetForegroundWindow();

	if (foreground_hwnd == default_hwnd) {

		return foreground_hwnd;

	}

	if (foreground_hwnd == hCashierDlg) {

		return foreground_hwnd;

	}

	if (foreground_hwnd == hCardRoomDlg) {

		return foreground_hwnd;

	}

  #if ADMIN_CLIENT

	if (foreground_hwnd == hAdminEditAccountDlg) {

		return foreground_hwnd;

	}

	if (foreground_hwnd == hAdminStats) {

		return foreground_hwnd;

	}

  #endif



	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd && Table[i].active_table_window) {

			return Table[i].hwnd;

		}

	}

	return default_hwnd;

}



//*********************************************************


//

// Initialize the Defaults structure to contain our default detaults.

//

void InitDefaults(void)

{

	zstruct(Defaults);

	Defaults.changed_flag = TRUE;



	// --- Any non-zero default detaults should be changed here ---

	Defaults.iQuietDealerType = 1;	// default to quiet dealer for new players

}



//*********************************************************


//

// 

void ReadDefaults(void)

{

	InitDefaults();

	long bytes_read = 0;

#if ADMIN_CLIENT

	int retry_count = 0;

tryagain:

#endif

	// ErrorType err = ReadFile("Defaults.bin", &Defaults, sizeof(Defaults), &bytes_read);

	ErrorType err = ReadFile("data\\settings.dat", &Defaults, sizeof(Defaults), &bytes_read);

	if (err || Defaults.dwVersion != DEFAULT_FILE_VERSION) {

		if (bytes_read) {

			// kp(("%s(%d) Defaults.bin is incompatible with current build (or unreadable).  Starting fresh.\n", _FL));

			kp(("%s(%d) settings.dat is incompatible with current build (or unreadable).  Starting fresh.\n", _FL));

		}

		InitDefaults();

	  #if ADMIN_CLIENT

		if (RunningManyFlag && err && retry_count < 20) {

			Sleep(500);

			retry_count++;

			goto tryagain;

		}

		if (RunningManyFlag) {

			// kp(("%s(%d) Error: failed to read defaults.bin in runningmany mode.  Aborting.\n", _FL));

			kp(("%s(%d) Error: failed to read settings.dat in runningmany mode.  Aborting.\n", _FL));

			exit(10);

		}

	  #endif

	}

	iNVidiaBugWorkaround = Defaults.iDisableGraphicOptimizations;

}





void WriteDefaults(int forced_write)

{

	if ((Defaults.changed_flag || forced_write)

	  #if ADMIN_CLIENT

		&& !RunningManyFlag

	  #endif

	) {

		Defaults.dwLength = sizeof(Defaults);

		Defaults.dwVersion = DEFAULT_FILE_VERSION;

		Defaults.changed_flag = FALSE;

		WriteFile("data\\settings.dat", &Defaults, sizeof(Defaults));

	}

}



//*********************************************************


//

// Close the connection to the server.  Make sure all data

// got sent out.  This function may take a little while.

//

void CloseConnectionToServerCleanly(void)

{

	struct ConnectionClosing cc;

	zstruct(cc);

	SendDataStructure(DATATYPE_CLOSING_CONNECTION, &cc, sizeof(cc));



	// Pause to give any remaining packets time to be sent.

	for (int i=0 ; i<20 ; i++) {

		Sleep(100);

		EnterCriticalSection(&ClientSockCritSec);

		if (!ClientSock || ClientSock->SendQueueEmpty()) {

			// The send queue is empty.  No point waiting much longer.

			LeaveCriticalSection(&ClientSockCritSec);

			break;

		}

		LeaveCriticalSection(&ClientSockCritSec);

	}

	Sleep(800);	// we need to give the TCP/IP stack a little time to get things out.

	EnterCriticalSection(&ClientSockCritSec);

	if (ClientSock) {

		ClientSock->CloseSocket();

	}

	LeaveCriticalSection(&ClientSockCritSec);

}



//*********************************************************

//

// Add/remove an hwnd from the table of windows we watch for keyboard translation

//

void AddKeyboardTranslateHwnd(HWND hwnd)

{

	// Look for an empty spot to put it in

	for (int i=0 ; i<MAX_TRANSLATOR_HWNDS ; i++) {

		if (!hKeyboardTranslateHwndTable[i]) {

			hKeyboardTranslateHwndTable[i] = hwnd;

			return;

		}

	}

	kp(("%s(%d) WARNING: MAX_TRANSLATOR_HWNDS is not big enough\n",_FL));

}



void RemoveKeyboardTranslateHwnd(HWND hwnd)

{

	// Look for this hwnd and remove it

	for (int i=0 ; i<MAX_TRANSLATOR_HWNDS ; i++) {

		if (hKeyboardTranslateHwndTable[i] == hwnd) {

			hKeyboardTranslateHwndTable[i] = NULL;

			break;

		}

	}	

}



//////////////////////////////////////////////////////////

//				Client entry point						//

//////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance,

                     HINSTANCE hPrevInstance,

                     LPSTR     lpCmdLine,

                     int       nCmdShow)

{




  int i;

  #if INCL_STACK_CRAWL

	volatile int top_of_stack_signature = TOP_OF_STACK_SIGNATURE;	// for stack crawl

  #endif

	HACCEL hAccelTable;

	MSG msg;

    dprintf_Disabled = TRUE;

    DebugPipeHandle = 1;

	// Initialize global strings

	MyRegisterClass(hInstance);



//	PPInitializeCriticalSection(&MessageThreadCritSec,CRITSECPRI_MESSAGE_THREAD,"MsgThread");

	PPInitializeCriticalSection(&CardRoomVarsCritSec, CRITSECPRI_CARDROOM,   	"CardRoom");

	PPInitializeCriticalSection(&ClientSockCritSec,   CRITSECPRI_CLIENTSOCK, 	"ClientSock");



	// Perform application initialization:

	if (!InitInstance (hInstance, nCmdShow)) {

		return FALSE;

	}





  i=stricmp(lpCmdLine, "debug");

  if ( i==0 ) {

		dprintf_Disabled = FALSE;

  }







  #if 1	//: rc_startup no longer used.

	StartupInit(lpCmdLine);

  #else

	int rc_startup = StartupInit(lpCmdLine);

  #endif

    

	



   	OpenSplashDialog();	// open the splash/connecting dialog box



	StartCommThread();	// start up the communications thread


//
   // CheckForUpgrade(TRUE);     
//


  #if USE_RICH_EDIT_FOR_CHAT_MONITOR

	// Load the RichEdit DLL to activate the RichEdit classes

    // for the chat monitor window to use.

    if (!(hInstRich=LoadLibrary("riched20.dll")))

        return 0;

  #endif



  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_CLIENT);
 

  #if ADMIN_CLIENT

	if (!RunningManyFlag)

  #endif

	{
		PreloadSoundFiles();
	//	DisplayNewsFromWebSite( );	// display any new news from our web site.
	}



		

	

	//OpenCardRoom();	// open the cardroom dialog box

  

  #if 0	//: disabled because it seems to be causing problems for

		// some people (unknown reason why).  It seems to cause more problems

		// than it solves, so we've disabling it for now.

	if (rc_startup == STARTUP_BAD_SHUTDOWN) {

		// if we had a bad shutdown last time (computer crash?), prompt right

		// away for a login attempt...

		//PostMessage(hCardRoomDlg, WM_COMMAND, (WPARAM)IDC_LOG_IN,(LPARAM)0);

		iRequestPriorityLogin = TRUE;

		LogInToServer("Bad shutdown last time");

	}

  #endif



	// Main message loop:

	while (GetMessage(&msg, NULL, 0, 0)) {

		//kp(("%s(%d) Got message $%04x for window $%08lx.  Dispatching it...\n", _FL, msg.message, msg.hwnd));

		// If this is a mouse or keyboard message (or anything else that indicates

		// a user is at his computer), keep track of the time.

		if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) ||

			(msg.message >= WM_KEYFIRST   && msg.message <= WM_KEYLAST))

		{

			//kp(("%s(%d) Got message %d\n", _FL, msg.message));

			dwLastUserActionMessageTime = SecondCounter;



			// Update our entropy whenever we get user messages

			if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

				dwRandomEntropy += (WORD32)rdtsc();

			}

			dwRandomEntropy += msg.lParam;	// add mouse location

		}



	  #if 0	//

		if (hChatTextWindow && msg.hwnd==hChatTextWindow) {

			kp(("%s(%d) got message %d for chat text window $%08lx\n",_FL,msg.message,msg.hwnd));

		}

	  #endif



		// Try dispatching the message to our various dialog boxes.

		// If it was a keyboard message, IsDialogMessage will handle it

		// by translating and dispatching for us, therefore we should not

		// pass it on to anyone else if it got handled by IsDialogMessage().

		int processed = FALSE;

	  #if 0

		if (hCardRoomDlg && IsDialogMessage(hCardRoomDlg, &msg)) {

			continue;	// it was already processed.

		}

		for (int i=0 ; i<MAX_TABLES ; i++) {

			if (Table[i].hwnd && IsDialogMessage(Table[i].hwnd, &msg)) {

				processed = TRUE;

				break;

			}

		}

	  #else	// generic for all registered dialog boxes

		for (int i=0 ; i<MAX_TRANSLATOR_HWNDS ; i++) {

			if (hKeyboardTranslateHwndTable[i] && IsDialogMessage(hKeyboardTranslateHwndTable[i], &msg)) {

				processed = TRUE;

				break;

			}

		}

	  #endif

		if (processed) {

			continue;	// it was already processed.

		}



		processed = TranslateAccelerator(msg.hwnd, hAccelTable, &msg);

		if (!processed) {

			// It wasn't an accelerator key message...

			// pass it on using the regular routines

		  #if 0	//: no longer needed?

			// was it for the login-progress indicator?

			//!!! should this hLoginProgress stuff still be here or can it be removed?

			if (!hLoginProgress || !IsDialogMessage(hLoginProgress, &msg))

		  #endif

			{

				TranslateMessage(&msg);



			  #if 0	//

				//!!!!! temp !!!!!!

				kp1(("%s(%d) Trapping all kinds of extra messages here\n",_FL));

				if (hChatTextWindow && msg.hwnd==hChatTextWindow) {

					kp(("%s(%d) got message %d for chat text window $%08lx\n",_FL,msg.message,msg.hwnd));

				}



				if (msg.message==EN_VSCROLL) {

					kp(("%s(%d) Got EN_VSCROLL for window $%08lx\n", _FL, msg.hwnd));

				}

				if (msg.message==WM_VSCROLL) {

					kp(("%s(%d) Got WM_VSCROLL for window $%08lx\n", _FL, msg.hwnd));

				}

			  #endif



				DispatchMessage(&msg);

			}

		}

		//kp(("%s(%d) Finished dispatching message $%04x for window $%08lx.\n", _FL, msg.message, msg.hwnd));

	}

	// now we're exiting...

	SendDataStructure(DATATYPE_PLAYER_LOGOFF, NULL, 0);	// send logoff packet

	CloseConnectionToServerCleanly();

	Shutdown();


  #if USE_RICH_EDIT_FOR_CHAT_MONITOR

	if (hInstRich) {

		FreeLibrary(hInstRich);

		hInstRich = NULL;

	}

  #endif



	ExitNowFlag = TRUE;	// All threads should exit.

	NOTUSED(lpCmdLine);

	NOTUSED(hPrevInstance);

  #if INCL_STACK_CRAWL

	NOTUSED(top_of_stack_signature);

  #endif

	return msg.wParam;

}



//*********************************************************

//

//  FUNCTION: MyRegisterClass()

//

//  PURPOSE: Registers the window class.

//

//  COMMENTS:

//

//    This function and its usage is only necessary if you want this code

//    to be compatible with Win32 systems prior to the 'RegisterClassEx'

//    function that was added to Windows 95. It is important to call this function

//    so that the application will get 'well formed' small icons associated

//    with it.

//

ATOM MyRegisterClass(HINSTANCE hInstance)

{

	WNDCLASSEX wcex;



	wcex.cbSize = sizeof(WNDCLASSEX);



	wcex.style			= CS_HREDRAW | CS_VREDRAW;

	wcex.lpfnWndProc	= (WNDPROC)WndProc;

	wcex.cbClsExtra		= 0;

	wcex.cbWndExtra		= 0;

	wcex.hInstance		= hInstance;

	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_CLIENT);

	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);

	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	wcex.lpszMenuName	= (LPCSTR)IDC_CLIENT;

	wcex.lpszClassName	= szWindowClass;

	wcex.hIconSm		= wcex.hIcon;



	return RegisterClassEx(&wcex);

}



//*********************************************************

//

//   FUNCTION: InitInstance(HANDLE, int)

//

//   PURPOSE: Saves instance handle and creates main window

//

//   COMMENTS:

//

//        In this function, we save the instance handle in a global variable and

//        create and display the main program window.

//

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)

{

	HWND hWnd;



	hInst = hInstance; // Store instance handle in our global variable



	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,

	      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);



	//kp(("%s(%d) hWnd = $%08lx\n", _FL, hWnd));

	if (!hWnd) {

		return FALSE;

	}

    

	hInitialWindow = hWnd;	// copy to global

	



	WinRestoreWindowPos(ProgramRegistryPrefix, "Main Window", hInitialWindow, NULL, NULL, FALSE, FALSE);



	//ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);



	NOTUSED(nCmdShow);

	return TRUE;

}



//*********************************************************

//

//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)

//

//  PURPOSE:  Processes messages for the main window.

//

//  WM_COMMAND	- process the application menu

//  WM_PAINT	- Paint the main window

//  WM_DESTROY	- post a quit message and return

//

//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) message = 0x%04x (%d)\n",_FL, message, message));

	switch (message) {

	case WM_ACTIVATEAPP:

		// save a flag to indicate if we're the active application or not.

		iOurAppIsActive = (BOOL)wParam;

		//kp(("%s(%d) WM_ACTIVATEAPP: active flag = %d\n", _FL, iOurAppIsActive));

		break;

	case WM_CREATE:

		//kp(("%s(%d) Got WM_CREATE\n",_FL));

		SetTimer(hWnd, WM_TIMER, 250, NULL);	// 'x' ms timer

		break;

	case WM_DESTROY:

		{

			// Close any open modeless dialog boxes

			// note: we must destroy from the same thread that called CreateDialog()

			for (int i=0 ; i<MAX_TABLES ; i++) {

				if (Table[i].hwnd) {

					// Close the dialog box

					pr(("%s(%d) Destroying window for table %d\n", _FL, i));

					DestroyWindow(Table[i].hwnd);

					zstruct(Table[i]);

				}

			}

		}

		WinStoreWindowPos(ProgramRegistryPrefix, "Main Window", hInitialWindow, NULL);

		PostQuitMessage(0);

		KillTimer(hWnd, WM_TIMER);	// remove our timer.

		hInitialWindow = NULL;

		break;

	case WM_ENDSESSION:

		kp(("%s(%d) Got WM_ENDSESSION: ending session = %d, just logging off = %d\n",

				_FL, (BOOL)wParam, lParam==ENDSESSION_LOGOFF));

		iUserLoggingOffSystemFlag = (BOOL)wParam;

		break;



	case WM_TIMER:

		UpdateSecondCounter();

		UpdateConnectionStatus(FALSE);

	  #if ADMIN_CLIENT

		if (iFlashTitleBars) {

			static dword last_flash;

			dword now = GetTickCount();

			if (now - last_flash >= 750) {

				last_flash = now;

				iFlashTitleBars--;

				// FLASHW_ALL is 0x03

				int flags = (iFlashTitleBars) ? 0x03 : 0;

				//kp(("%s(%d) counter = %d, flash flags = %d\n", _FL, iFlashTitleBars, flags));

				if (hCardRoomDlg) {

					FlashWindow(hCardRoomDlg, flags);

				}

				if (Table[0].hwnd) {

					FlashWindow(Table[0].hwnd, flags);

				}

				if (hAdminStats) {

					FlashWindow(hAdminStats, flags);

				}

				if (hAdminEditAccountDlg) {

					FlashWindow(hAdminEditAccountDlg, flags);

				}

				if (hChatMonitorDlg) {

					FlashWindow(hChatMonitorDlg, flags);

				}

			}

		}

	  #endif

	  #if RANDOMLY_DISCONNECT	//

		{	// Randomly disconnect.

			if (!random(120*2)) {

				kp1(("%s(%d) Randomly disconnecting to force testing of our reconnect feature.\n",_FL));

				// This is a complete kludge of a debug feature...

				// we want it simulate a network problem as closely

				// as possible, so let's just close our socket.

				EnterCriticalSection(&ClientSockCritSec);

				if (ClientSock) {

					ClientSock->CloseSocket();

				}

				LeaveCriticalSection(&ClientSockCritSec);

			}

		}

	  #endif

		return TRUE;

	case WM_SIZING:

		WinSnapWhileSizing(hWnd, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hWnd, message, wParam, lParam);

		break;

	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hWnd);

		return TRUE;

	case WMP_DISPLAY_BAD_EMAIL_NOTICE:

		MessageBox(hWnd,"The server says the email address it has on\n"

						"file for you has not been validated.\n"

						"\n"

						"Please go to the Options menu and select\n"

						"'Change/Validate Email Address' to correct it.\n",

						"Email address needs validating", MB_OK|MB_ICONINFORMATION);

		return TRUE;

	case WMP_MISC_CLIENT_MSG:

		ShowMiscClientMsg(PickWindowForRandomMessage(hWnd), &CardRoom_misc_client_msg, NULL);

		return TRUE;	// TRUE = we did process this message.

  #if ADMIN_CLIENT

	case WMP_OPEN_CHECK_RUN:

		//// !!! Don't allow check run in the first 10 minutes of the hour

		

		if (!hCheckRun) {

			hCheckRun = CreateDialog(hInst,

				MAKEINTRESOURCE(IDD_ADMIN_CHECK_RUN), NULL, dlgFuncAdminCheckRun);

		} else {

			ReallySetForegroundWindow(hCheckRun);

		}

		return TRUE;	// TRUE = we did process this message.

  #endif	// ADMIN_CLIENT

	case WMP_OPEN_TABLE_WINDOW:
	  

	  #if 0	//
	 // #if 1	//
		HWND hDlg;
		MessageBox(hDlg, "Probando", "File Not Found", MB_OK);
	
		//validate email
		if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), t->hwnd, dlgFuncChangeEmail);
		}else{
		// Comm thread asked us to open a new table window.
			OpenNewTableWindow(

				((struct CardRoom_JoinTable *)lParam)->table_serial_number,

				(PokerGame)((struct CardRoom_JoinTable *)lParam)->game_type,

				((struct CardRoom_JoinTable *)lParam)->status==2);
		}
	  #else

	  	// Process all pending join table requests...

	  	{
			HWND hDlg;
			//MessageBox(hDlg, "Probando", "File Not Found", MB_OK);
			if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
			}

			struct CardRoom_JoinTable *jt = PendingJoinTableResults;

			for (int i=0 ; i<MAX_TABLES ; i++, jt++) {

				if (jt->table_serial_number) {

					if (jt->status) {	// join or watch... open window

						if (!Defaults.keep_cardroom_window_open) {	// moved here 990907HK

							ShowWindow(hCardRoomDlg, SW_MINIMIZE);

						}

						ChipType ct = CT_PLAY;

						if (jt->flags & JOINTABLE_FLAG_REALMONEY) {

							ct = CT_REAL;

						}

						if (jt->flags & JOINTABLE_FLAG_TOURNAMENT) {

							ct = CT_TOURNAMENT;

						}
						
						//validate email
						if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
							HWND hDlg;
							DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL),hDlg, dlgFuncChangeEmail);
						}else{

							OpenNewTableWindow(

								jt->table_serial_number,

								(ClientDisplayTabIndex)jt->client_display_tab_index,

								(GameRules)jt->game_rules,

								jt->status==2,

								ct,

								jt->flags);
						}
					} else {

						// Unjoin... close window.  No warning, nothing.

						struct TableInfo *t = TablePtrFromSerialNumber(jt->table_serial_number);

						if (t) {

							kp(("%s(%d) Server has unjoined us from table %d.  Closing table window.\n", _FL, jt->table_serial_number));

							PostMessage(t->hwnd, WMP_CLOSE_YOURSELF, 0, 0);

						}

					}

					zstruct(*jt);	// we've processed this entry - free it

				}

			}

	  	}

	  #endif

		return TRUE;

	case WMP_PLAY_SOUND:

		PPlaySound2(wParam);

		return TRUE;	// TRUE = we did process this message.

	case WMP_PROCESS_VERSION_INFO:

	  	// If there is a new version of the client, proceed with auto-upgrade

		if (ClientVersionNumber.build < ServerVersionInfo.min_client_version.build ||

			ClientVersionNumber.build < ServerVersionInfo.new_client_version.build) {

		  #if ADMIN_CLIENT

			if (!RunningManyFlag)

		  #endif

			{

				pr(("%s(%d) A new client version is available.\n",_FL));

				if (!AutoUpgradeDisable) {

				        ///CheckForUpgrade(TRUE);    // -- marked my Robert, 01/07/2002, 

													// in order to disable the complicated

													// auto-upgrade function, and use the IE direct

													// download function instead



				}

			}

		}

		ChangeUpgradeMenuToRevertIfNecessary();

		return TRUE;

	case WMP_SHOW_CARDROOM:

		

		// Show the cardroom

		//kp(("%s(%d) Instance %d Got WMP_SHOW_CARDROOM\n", _FL, iProgramInstanceCount));

		if (!hCardRoomDlg) {

			OpenCardRoom();

		}

		ShowWindow(hCardRoomDlg, SW_SHOWNORMAL);

		ReallySetForegroundWindow(hCardRoomDlg);

		return TRUE;

	case WMP_SHOW_LOGIN_PROGRESS:

		if (!hLoginProgress) {

			hLoginProgress = CreateDialog(hInst,

				MAKEINTRESOURCE(IDD_LOGGING_IN_STATUS), hWnd, dlgFuncLoggingInStatus);

		} else {

			ReallySetForegroundWindow(hLoginProgress);

		}

		return TRUE;

	case WMP_SHOW_TABLE_WINDOW:

		ShowTableWindow(lParam, wParam);

		return TRUE;

	case WMP_UPDATE_LOGIN_PROGRESS:

		if (hLoginProgress) {

			SendMessage(GetDlgItem(hLoginProgress, IDC_PROGRESS), PBM_SETPOS, (WPARAM)wParam, 0);

		}

		return TRUE;

	case WMP_UPDATE_CONNECT_STATUS:

		UpdateConnectionStatus(TRUE);	// force an update

		return TRUE;

	}

	return DefWindowProc(hWnd, message, wParam, lParam);

}



//****************************************************************


//

// Program startup initialization

// return T/F if we're supposed to auto-login HK19990628

//

int StartupInit(LPSTR lpCmdLine)

{

  #if DEBUG

	kDebWinPrefixString = "Client: ";

	kp(("Client is now up and running.\n"));

  #endif

	char *login_override = NULL;

	char *password_override = NULL;



#if _WIN32_IE >= 0x300	// IE3, IE4, etc.

	INITCOMMONCONTROLSEX icc;

	zstruct(icc);

	icc.dwSize = sizeof(icc);

	icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES;

	//kp(("%s(%d) Warning: using _WIN32_IE=0x%04x\n", _FL, _WIN32_IE));

	int success = InitCommonControlsEx(&icc);

	if (!success) {

		Error(ERR_ERROR, "%s(%d) InitCommonControlsEx() failed.", _FL);

	}

#else	// Win95, NT4, no IE updates required.

	InitCommonControls();

#endif

	GetClientVersionNumber();



  #if 0	//: test critical section entry/exit order testing

  	{

		PPCRITICAL_SECTION pptest4;

		PPInitializeCriticalSection(&pptest4, CRITSECPRI_CARDROOM+1, "test4");



		kp(("%s(%d) ----- beginning of crit sec testing ------\n",_FL));

		kp(("%s(%d) test 1... out of order entering...\n",_FL));

		EnterCriticalSection(&ClientSockCritSec);

		EnterCriticalSection(&CardRoomVarsCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		LeaveCriticalSection(&ClientSockCritSec);



		kp(("%s(%d) test 2... nested entering...\n",_FL));

		kp(("%s(%d) entering CardRoomVarsCritSec\n",_FL));

		EnterCriticalSection(&CardRoomVarsCritSec);

		kp(("%s(%d) entering ClientSockCritSec\n",_FL));

		EnterCriticalSection(&ClientSockCritSec);

		kp(("%s(%d) entering CardRoomVarsCritSec\n",_FL));

		EnterCriticalSection(&CardRoomVarsCritSec);

		kp(("%s(%d) entering ClientSockCritSec\n",_FL));

		EnterCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);



		kp(("%s(%d) test 3... out of order leaving...\n",_FL));

		EnterCriticalSection(&CardRoomVarsCritSec);

		EnterCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		LeaveCriticalSection(&ClientSockCritSec);



		kp(("%s(%d) test 4... nested entering (again)...\n",_FL));

		kp(("%s(%d) entering CardRoomVarsCritSec\n",_FL));

		EnterCriticalSection(&CardRoomVarsCritSec);

		kp(("%s(%d) entering ClientSockCritSec\n",_FL));

		EnterCriticalSection(&ClientSockCritSec);

		kp(("%s(%d) entering CardRoomVarsCritSec\n",_FL));

		EnterCriticalSection(&CardRoomVarsCritSec);

		kp(("%s(%d) entering ClientSockCritSec\n",_FL));

		EnterCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);



		kp(("%s(%d) test 5... nested entering (same pri)...\n",_FL));

	  #if 0	//

		EnterCriticalSection(&CardRoomVarsCritSec);

	  #else

		PPEnterCriticalSection0(&CardRoomVarsCritSec, _FL, TRUE);

	  #endif

		EnterCriticalSection(&ClientSockCritSec);

		EnterCriticalSection(&pptest4);

		LeaveCriticalSection(&pptest4);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);



		PPDeleteCriticalSection(&pptest4);

		kp(("%s(%d) ----- end of crit sec testing ------\n",_FL));

	}

  #endif



	// Read the .Client Startup file for our default settings...

	// Modified by cris
//    BlockProcessServerVersionInfo=0;
	ReadParmFile("data\\startup.dat", getenv("COMPUTERNAME"), INIParms, TRUE);

	ReadDefaults();
	if(Defaults.upgrade==1){
		//lanza el upgrader.bat
		 WriteDefaults(TRUE);
		 PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		 //WinExec("upgrade.bat", SW_SHOWNA);
		 system("upgrade.bat");
		 return 0;
	}else{
	

	// we want to know if this client shut down cleanly

	int previous_dirty_shutdown = Defaults.dirty_shutdown;



	// we'll write it FALSE now -- it'll be set TRUE on a clean shutdown

	Defaults.dirty_shutdown = TRUE;

	WriteDefaults(TRUE);



	// Parse the command line...

	//kp(("%s(%d) Raw command line from Windows: '%s'\n", _FL, lpCmdLine));

	#define MAX_ARGV 20

	char *argv[MAX_ARGV];

	int argc = CommandLineToArgcArgv(lpCmdLine, argv, MAX_ARGV);

    



 

   FILE *stream;

   char buffer[17];

   int i,  ch;



   /* Open file to read line from: */

   if( (stream = fopen( "data\\server.dat", "r" )) == NULL )

      exit( 0 );



   /* Read in first 80 characters and place them in "buffer": */

   ch = fgetc( stream );

   for( i=0; (i < 16 ) && ( feof( stream ) == 0 ); i++ )

   {

      buffer[i] = (char)ch;

	  if (  ((char)ch != '.')  &&  (((char)ch <'0')||((char)ch >'9'))  )

         break;

      ch = fgetc( stream );

   }



   /* Add null to end string */

   buffer[i] = '\0';

   fclose( stream );




   //Read VendorCode for marketing purpose

   if( (stream = fopen( "data\\vendor.dat", "r" )) == NULL ) {

		strcpy(VendorCode, "PC");

   } else {

		fscanf(stream, "%s", VendorCode);

		fclose(stream);

   }



   // DebugPipeHandle = 1;

   if ((argc == 1 )||(argc ==2 )){
	        iAdminClientFlag = TRUE;  //  allen
			AutoUpgradeDisable = TRUE;	// don't accept new upgrades this time.
			AutoUpgradeDisable = FALSE;	// Modified by Robert, 
										//in order to turn on the auto-upgrade function.
			Defaults.keep_cardroom_window_open = TRUE;    
            iDisableServerSwitching = TRUE;	// never try another server
			strnncpy(ServerName, buffer, MAX_SERVERNAME_LEN);
		//	MessageBox(NULL, ServerName, "Server Name!!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		//	Defaults.last_server_ip_address = 0;	// always clear it
			Defaults.last_server_ip_address =IP_ConvertHostNameToIP(ServerName);
	}




//disable parse args
#if 1   //Para poder correr varios clientes desde la linea de comandos    J Fonseca   25/02/2004

	// Parse command line parameters:

	for ( i=1 ; i<argc ; i++) {

		//kp(("%s(%d) argv[%d] = '%s'\n",_FL, i, argv[i]));

		if (!stricmp(argv[i], "server")) {		// server hostname
		 
           // MessageBox(NULL, "Server activado.", "Que madre !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
			if (++i<argc) {

				iDisableServerSwitching = TRUE;	// never try another server

				strnncpy(ServerName, argv[i], MAX_SERVERNAME_LEN);

				Defaults.last_server_ip_address = 0;	// always clear it
				
				Defaults.last_server_ip_address =IP_ConvertHostNameToIP(ServerName); //J Fonseca 25/02/2004

			} else {

				kp(("%s(%d) Command line 'server' option must be followed by a hostname\n",_FL));

			}

	  #if ADMIN_CLIENT 

		} else if (!stricmp(argv[i], "admin")) {
			 //MessageBox(NULL, "Admin activado.", "Que madre !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);

		    iAdminClientFlag = TRUE;  

			AutoUpgradeDisable = TRUE;	// don't accept new upgrades this time.

			Defaults.keep_cardroom_window_open = TRUE;

		} else if (!stricmp(argv[i], "runningmany")) {	// running a lot of clients?

			MessageBox(NULL, "Varios activado.", "Que madre !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
			RunningManyFlag = TRUE;

			SfxGlobalDisable = TRUE;

			AutoLoginFlag = TRUE;

			AutoJoinDefaultTable = TRUE;

			MinimizeWindowsFlag = TRUE;

		} else if (!stricmp(argv[i], "autojoin")) {		// automatically join a table?

			AutoLoginFlag = TRUE;

			AutoJoinDefaultTable = TRUE;

		} else if (!stricmp(argv[i], "sockets")) {		// open extra sockets (for testing)

			ExtraSocketsToOpen = atoi(argv[++i]);

		} else if (!stricmp(argv[i], "serverstats")) {	// automatically show the server stats?

			AutoShowServerStats = 1;

		} else if (!stricmp(argv[i], "serverstats2")) {	// automatically show the server stats?

			AutoShowServerStats = 2;	// show in position 2

	  #endif

	  #if INCL_EMAIL_INVALIDATING

		} else if (!stricmp(argv[i], "invalidateemail")) {	// automatically invalidate email address in prefs.cpp?

			iInvalidateEmailAddresses = TRUE;

	  #endif

		} else if (!stricmp(argv[i], "autologin")) {	// automatically log in?

			AutoLoginFlag = TRUE;

		} else if (!stricmp(argv[i], "nosound")) {	// disable all sound

			SfxGlobalDisable = TRUE;

		} else if (!stricmp(argv[i], "minimize")) {	// automatically log in?

			MinimizeWindowsFlag = TRUE;

		} else if (!stricmp(argv[i], "noupdate")) {	// resume after auto-upgrade?

			AutoUpgradeDisable = TRUE;	// don't accept new upgrades this time.

		} else if (!stricmp(argv[i], "resume")) {	// resume after auto-upgrade?

			AutoUpgradeDisable = TRUE;	// don't accept new upgrades this time.

			if (Defaults.last_server_ip_address) {

				IP_ConvertIPtoString(Defaults.last_server_ip_address, ServerName, MAX_SERVERNAME_LEN);

			}

		} else if (!stricmp(argv[i], "login")) {	// login override?

			iRequestPriorityLogin = TRUE;

			login_override = argv[++i];

			while (*login_override=='"') {	//20000922MB: skip leading quotes

				login_override++;

			}

			//kp(("%s(%d) login_override = %s\n", _FL, login_override));

		} else if (!stricmp(argv[i], "password")) {	// password override?

			password_override = argv[++i];

			//kp(("%s(%d) password_override = %s\n", _FL, login_override));

		} else {

			kp(("%s(%d) Unrecognized command line parameter: '%s'. Skipping it.\n", _FL, argv[i]));

		}

	} //for parse arguments

#endif //enable/disable parse arg

	for (i=0 ; i<MAX_TABLES ; i++) {

		zstruct(Table[i]);

		Table[i].table_index = i;

	}



	CountProgramInstances();

	if (iProgramInstanceCount >= 1) {

		// We're not the first instance running.

		// Restore window for other instance and exit.

	  #if ADMIN_CLIENT

		if (!iAdminClientFlag)

	  #endif

		{

			kp(("%s(%d) We're not the first instance running.\n",_FL));

			kp(("%s(%d) Setting first instance to foreground and terminating.\n",_FL));

			DestroyWindow(hInitialWindow);	// destroy ours before searching for other instance



			// Find the other window and top it.

			HWND other_window = FindWindow(szWindowClass, szTitle);

			//kp(("%s(%d) FindWindow('%s', '%s') returned $%08lx\n", _FL, szWindowClass, szTitle, other_window));

			if (other_window != NULL) {

				//kp(("%s(%d) Sending WMP_SHOW_CARDROOM to window $%08lx\n", _FL, other_window));

				PostMessage(other_window, WMP_SHOW_CARDROOM, 0, 0);

			} else {

				kp(("%s(%d) Could not find other window.\n", _FL));

			}

			exit(0);	// leave right now.

		}

//Para correr varios clientes a la vez    J Fonseca  25/02/2004		
#if 0
	  /*--- if ( LoggedInPrivLevel <= 20  ) {   */	

	        kp(("%s(%d) We're not the first instance running.\n",_FL));

			kp(("%s(%d) Setting first instance to foreground and terminating.\n",_FL));

			DestroyWindow(hInitialWindow);	// destroy ours before searching for other instance



			// Find the other window and top it.

			HWND other_window = FindWindow(szWindowClass, szTitle);

			//kp(("%s(%d) FindWindow('%s', '%s') returned $%08lx\n", _FL, szWindowClass, szTitle, other_window));

			if (other_window != NULL) {

			//	//kp(("%s(%d) Sending WMP_SHOW_CARDROOM to window $%08lx\n", _FL, other_window));

				PostMessage(other_window, WMP_SHOW_CARDROOM, 0, 0);

			} else {

				kp(("%s(%d) Could not find other window.\n", _FL));

			}

			exit(0);	// leave right now.



	  /* -------------------------- }---------------------------------------------  */
#endif
	}





	srand(GetTickCount()+iProgramInstanceCount);	// seed our random number generator

  #if DEBUG

	// Change the program prefix for debwin to indicate the instance

	static char debwinprefix[30];

	sprintf(debwinprefix, "Client %d: ", iProgramInstanceCount+1);

	kDebWinPrefixString = debwinprefix;

	{

		// Tell DebWin where to find our source files.

		char path[MAX_FNAME_LEN];

		GetDirFromPath(__FILE__, path);

		kAddSourcePath(path);

		kAddLibSourcePath();

	}

  #endif

	OpenToolTipHooks(hInst);



  #if ADMIN_CLIENT

	// Pop up the login dialog (here temporarily)

	if (iProgramInstanceCount) {

		// We're not the first instance... try logging in as a different name

		char str[100];

		if (LoginUserID[0]) {

			strcpy(str, LoginUserID);

		} else {

			strcpy(str, Defaults.user_id);

		}

		// trim all trailing spaces and digits from the user id

		while (strlen(str) && (str[strlen(str)-1]==' ' || isdigit(str[strlen(str)-1]))) {

			str[strlen(str)-1] = 0;

		}

		sprintf(str+strlen(str), " %d", iProgramInstanceCount+1);

		strnncpy(LoginUserID, str, MAX_PLAYER_USERID_LEN);

		//kp(("GOT NAME OF %s\n",LoginUserID));

	} else

  #endif

	{

		strnncpy(LoginUserID, Defaults.user_id, MAX_PLAYER_USERID_LEN);

	}

	strnncpy(LoginPassword, DecryptPassword(Defaults.password), MAX_PLAYER_PASSWORD_LEN);



	// Override the defaults with anything passed on the command line.

	if (login_override) {

		//kp(("%s(%d) login_override = %s\n", _FL, login_override));

		strnncpy(LoginUserID, login_override, MAX_PLAYER_USERID_LEN);

	}

	if (password_override) {

		//kp(("%s(%d) password_override = %s\n", _FL, login_override));

		strnncpy(LoginPassword, password_override, MAX_PLAYER_PASSWORD_LEN);

	}



	if (AutoLoginFlag) {

		return STARTUP_AUTO_LOGIN;

	}

	if (previous_dirty_shutdown) {

		return STARTUP_BAD_SHUTDOWN;

	}

	return STARTUP_NORMAL;
	};//if read th edefaults to upgrader

}



//****************************************************************


//

// Shut down

//

void Shutdown(void)

{

	RemoveLoginProgressIndicator();

	CloseToolTipHooks();

	Defaults.dirty_shutdown = FALSE;

	WriteDefaults(TRUE);

	Defaults.dirty_shutdown = TRUE;

}



//*********************************************************


//

// Display a struct MiscClientMessage from the server.

//

void ShowMiscClientMsg(HWND parent, struct MiscClientMessage *mcm, char *title_in)

{

	#define MAX_TITLE_LEN	100

	char title[MAX_TITLE_LEN];

	if (title_in) {	// title was supplied

		strnncpy(title, title_in, MAX_TITLE_LEN);

	} else if (mcm->table_serial_number) {

		sprintf(title, "Message from table %s:",

			TableNameFromSerialNumber(mcm->table_serial_number));

	} else {

		sprintf(title, " A Message from the e-Media Poker server:");

	}

	MessageBox(parent, mcm->msg, title, MB_OK|MB_APPLMODAL|MB_TOPMOST);



	// after posting a pop-up message, set focus to other windows in order of

	// importance (if they exist)

	//  if we have a buy-in dlg, give it the focus

	if (hBuyInDLG) { // there already is one

		SetFocus(hBuyInDLG);

	}

}



/**********************************************************************************

 Function EncryptPassword(char *plain_text)


 Purpose: returns an encrypted version of our password

***********************************************************************************/

#define ENCRYPTION_MASK	0x96	// 10010110

char *EncryptPassword(char *plain_text)

{

	static char out[MAX_PLAYER_PASSWORD_LEN];

	char *p = plain_text;

	int index = 0;

	while (*p) {

		out[index] = (char)(*p ^ ENCRYPTION_MASK);

		p++;

		index++;

		if (index == MAX_PLAYER_PASSWORD_LEN) {

			index--;

			break;

		}

	}

	out[index] = 0;

	return out;

}	



/**********************************************************************************

 Function *DecryptPassword(char *encrypted_text)



 Purpose: returns the decrypted version of our password

 NOTE: same as function above for now (they just inverse each other)

***********************************************************************************/

char *DecryptPassword(char *encrypted_text)

{

	return EncryptPassword(encrypted_text);

}



/**********************************************************************************

 Function DealWithLoginStatus(int login_status)



 Purpose: when we receive a client login status, it may have changed... deal with it

 NOTE:	This is called from the comm thread.  It cannot set window text

 		or do anything else which might wait for the message thread.

***********************************************************************************/

void DealWithLoginStatus(int login_status)

{

	if (login_status == LastLoginStatus) {	// hasn't changed

		return;	// so do nothing

	}

	// it changed -- we may want to do something like post a message

	LastLoginStatus = login_status;

	RemoveLoginProgressIndicator();	// remove this if we're displaying it

	switch (login_status) {

	case LOGIN_NO_LOGIN:

		break;

	case LOGIN_INVALID:

		MessageBox(hInitialWindow,

			"Unable to process login",

			"Invalid login...",

			MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

			RemoveBuyInDLG();	// if we're buying in somewhere, abort it

		break;

	case LOGIN_BAD_USERID:

	  #if ADMIN_CLIENT

		if (RunningManyFlag) {

			Error(ERR_FATAL_ERROR, "%s(%d) Invalid login ('%s') during 'runningmany' mode.", _FL, LoginUserID);

			exit(10);

		}

	  #endif

		MessageBox(hInitialWindow,

		  #if 0	//  changed back to the  old way

			"We were unable to validate that Player ID.\n"

			"\n"

			"If you have forgotten your Player ID, please contact us\n"

			"at support@e-mediasoftware.com\n"

			"\n"

			"Be sure to include your full name and address information.\n"

			"\n"

			"We will contact you at the email address we have on file.\n",

		  #else

			"Unable to validate Player ID\n",

		  #endif

			"Invalid Player ID...",

			MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

			RemoveBuyInDLG();	// if we're buying in somewhere, abort it

		break;

	case LOGIN_BAD_PASSWORD:

		MessageBox(hInitialWindow,

		  #if 0	// : changed back to the  old way

			"We were unable to validate your password.\n"

			"\n"

			"If you have forgotten your password, please contact us\n"

			"at support@e-mediasoftware.com\n"

			"\n"

			"Be sure to include your Player ID and full registered name.\n"

			"\n"

			"We will contact you at the email address we have on file.\n",

		  #else

			"Unable to validate Password\n",

		  #endif

			"Invalid Password...",

			MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

			RemoveBuyInDLG();	// if we're buying in somewhere, abort it

		break;

	case LOGIN_VALID:

		break;

	case LOGIN_ANONYMOUS:

		break;

	default:

		Error(ERR_ERROR, "%s(%d) unknown login status=%d",_FL,login_status);

	}

	if (AutoLoginFlag && login_status != LOGIN_VALID && login_status != LOGIN_ANONYMOUS) {

		AutoLoginFlag = FALSE;

	  #if ADMIN_CLIENT

		if (AutoJoinDefaultTable) {

			// this isn't good... just quit.

			PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, TRUE, 0);

		}

	  #endif

	}

}



/**********************************************************************************

 Function RemoveBuyInDLG()


 Purpose: if the BuyInDLG is up, remove it

***********************************************************************************/

void RemoveBuyInDLG(void)

{

	if (hBuyInDLG) {	// we were buying in somewhere -- get rid of it

		PostMessage(hBuyInDLG, WMP_CLOSE_YOURSELF, 0, 0);

	}

}



/**********************************************************************************

 Function RemoveLoginProgressIndicator(void)

 

 Purpose: remove the login progress indicator dialog box

***********************************************************************************/

void RemoveLoginProgressIndicator(void)

{

	if (hLoginProgress) {

		PostMessage(hLoginProgress, WMP_CLOSE_YOURSELF, 0, 0);

	}

}



/**********************************************************************************

 Function PPlaySound(int sound_index)

 

 Purpose: plays a sound

***********************************************************************************/

int SfxGlobalDisable;	// t/f if sfx disabled globally (e.g. cmd line or something)

int SfxUserDisable;		// t/f if sfx disabled by user (e.g. preferences dialog)

int SfxTempDisable;		// t/f if sfx disabled termporarily for some programming reason



// the enum definition for these sound files is in pplib.h (SOUND_*)





static char *szSoundFiles[SOUND_MAX_SOUNDS] = {

	NULL,

	"media\\fx_mov_btn.wav",

	"media\\fx_crd_dealing.wav",

	"media\\fx_flop_dealing.wav",

	"media\\fx_crd_folding.wav",

	"media\\fx_your_turn.wav",

	"media\\fx_wake_up.wav",

	"media\\fx_chp_sliding.wav",

	"media\\fx_chp_dragging.wav",

	"media\\fx_chp_betting.wav",

	"media\\fx_check.wav",

  #if ADMIN_CLIENT

  #endif

};







static void *Sounds[SOUND_MAX_SOUNDS];	// ptrs to pre-loaded sounds



//*********************************************************



//

// Pre-load all sound files into memory

//

void PreloadSoundFiles(void)

{

	for (int i=0 ; i<SOUND_MAX_SOUNDS ; i++) {

		if (!Sounds[i] && szSoundFiles[i]) {

			long bytes_read;



			//  Sounds[i] = LoadFile(FindFile(szSoundFiles[i]), &bytes_read);

			Sounds[i] = LoadFile(FindMediaFile(szSoundFiles[i]), &bytes_read);

			if (!Sounds[i]) {

				kp(("%s(%d) Could not load sound file '%s'\n", _FL, szSoundFiles[i]));

			} else {

				pr(("%s(%d) Sound %d loaded at address $%08lx (%d bytes).  fname=%s\n",

							_FL, i, Sounds[i], bytes_read, szSoundFiles[i]));

			}

		}

	}

}



//*********************************************************



//

// Play a sound file by posting a message to the main thread's

// message queue and letting it do the work.

//

void PPlaySound(int sound_index)

{

	PostMessage(hInitialWindow, WMP_PLAY_SOUND, (WPARAM)sound_index, 0);

}



//*********************************************************



//

// Play a sound file. This function should be called only be the

// main window's message handler.  Use PPlaySound() to post the

// message which causes this function to get called.

//

static void PPlaySound2(int sound_index)

{

	static int initialized = FALSE;

	static PPCRITICAL_SECTION SoundCritSec;

	if (!initialized) {

		PPInitializeCriticalSection(&SoundCritSec, CRITSECPRI_SOUND, "Sound");

		initialized = TRUE;

	}

	EnterCriticalSection(&SoundCritSec);

	static last_sound_played = SOUND_NO_SOUND;

	if (!SfxGlobalDisable && !SfxUserDisable && !SfxTempDisable) {

		if (sound_index == SOUND_NO_SOUND) { // stop playing

			PlaySound(NULL, NULL, SND_PURGE);

		} else {

			// we want to ignore certain sounds if we just played it

			// filter out sounds we don't want repeating

			if (sound_index == SOUND_CHIPS_SLIDING && last_sound_played == SOUND_CHIPS_SLIDING) {

				LeaveCriticalSection(&SoundCritSec);

				return;

			}

			//kp(("%s(%d) Playing sound %d (fname=%s, wav file ptr = $%08lx)\n", _FL, sound_index, szSoundFiles[sound_index], Sounds[sound_index]));

			if (Sounds[sound_index]) {

				// note: PlaySound sometimes returns TRUE (success), even

				// if nothing got played !!!!

				// 19990812MB: PlaySound sometimes leaks a handle every

				// time it plays a sound.  This was happening on Mike's machine

				// with the AWE32 installed.  It worked fine on HK's machine and it

				// works now on Mike's machine with the SBLive.

			  #if 0	//19991007MB

				{

					static DWORD old_ticks;

					DWORD ticks = GetTickCount();

					kp(("%s(%d) PlaySound() spacing = %4dms\n", _FL, ticks-old_ticks));

					old_ticks = ticks;

					

				}

			  #endif

				int played_sound = PlaySound((char *)Sounds[sound_index], NULL,

						SND_MEMORY | SND_ASYNC | SND_NODEFAULT);

				if (!played_sound) {

					static int sound_error_count = 0;

					if (++sound_error_count < 4) {

						kp(("%s(%d) couldn't play sound[%d] (%s)\n", _FL,

							sound_index, szSoundFiles[sound_index]));

					} else {

						kp1(("%s(%d) Got too many sound errors... not printing them anymore.\n",_FL));

					}

				} else {

					last_sound_played = sound_index;

				}

			}

		}

	}

	LeaveCriticalSection(&SoundCritSec);

}

/*

Returns: TRUE if logged in successfully, FALSE if not.  Check LoggedIn (global) for:
	LOGIN_NO_LOGIN, LOGIN_INVALID, LOGIN_BAD_USERID, 
	LOGIN_BAD_PASSWORD, LOGIN_VALID, LOGIN_ANONYMOUS
	and check the version
*/
int preLogInToServer(char *detail_msg){
	//COMPILE_ADMIN 
    


	 //////////////////////
     //kp(("%s(%d) LogInToServer('%s') has been called.\n",_FL,detail_msg));



	// if we're already validly logged in, just return true

	if (LoggedIn == LOGIN_VALID) {

		return TRUE;

	}

	// if we've already got a login dlg up, top it...

	if (hLogInDLG) {	// already exists

		ReallySetForegroundWindow(hLogInDLG);

		return FALSE;

	}

	// if we're in the process of logging in, show that

	if (hLoginProgress) {

		ReallySetForegroundWindow(hLoginProgress);

		return FALSE;

	}

	// we're ok to try to log in

retry:

	LoggedIn = LOGIN_NO_LOGIN;

	int rc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LOGIN), HWND_DESKTOP,

			(DLGPROC)dlgLoginFunc, (LPARAM)detail_msg);



	if (rc==IDRETRY) {

		// Bring dialog up again with new defaults (probably from

		// creating a new account)

		detail_msg = "Account created successfully.\nLog in to your new account...";

		goto retry;

	}	

	if (rc != IDCANCEL) {

		SendMessage(hInitialWindow, WMP_SHOW_LOGIN_PROGRESS, 0, 0);

		if (hLoginProgress) {

			SetWindowPos(hLoginProgress, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

			ShowWindow(hLoginProgress, SW_SHOW);

		} else {

			Error(ERR_ERROR, "%s(%d) Login status dialog failed to open.  GetLastError() = %d", _FL, GetLastError());

		}

		#define MAX_MILLISECONDS_TO_WAIT_FOR_LOGIN	15000

		#define MILLISECOND_INCREMENT	50

		int milliseconds_waited = 0;

		int progress_bar = 3;

		MSG msg;

		while (LoggedIn == LOGIN_NO_LOGIN) {

			// process message queue so we get WM_PAINTs, etc...

			while (PeekMessage(&msg, hInitialWindow, 0, 0, PM_REMOVE)) {

				if (!IsDialogMessage(hLoginProgress, &msg)) {

					GetMessage(&msg, NULL, 0, 0);

					TranslateMessage(&msg);

					DispatchMessage(&msg);

				}

			}

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			Sleep(MILLISECOND_INCREMENT);

			milliseconds_waited += MILLISECOND_INCREMENT;

			// set where we should be on the progress bar

			int new_bar;

			// in the first half second, somewhere within the first 25%

			if (milliseconds_waited <= 500) {

				new_bar = (int)(25 * milliseconds_waited/500);

			} else if (milliseconds_waited <= 3000) {

				// in the first three seconds, up to 50%

				new_bar = (int)(50 * milliseconds_waited/3000);

			} else {

				// creep up slowly, never past 75%

				new_bar = (int)(75 * milliseconds_waited/MAX_MILLISECONDS_TO_WAIT_FOR_LOGIN);

			}

			progress_bar = max(new_bar, progress_bar);

			if (milliseconds_waited > MAX_MILLISECONDS_TO_WAIT_FOR_LOGIN) {	// give up

				RemoveLoginProgressIndicator();

				if (LoggedIn != LOGIN_VALID) {

					MessageBox(NULL, "Connection to server timed out", "Couldn't log in", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);

				}

				break;

			}

		}

		// cosmetics

		if (hLoginProgress) {

			Sleep(250);

			progress_bar = max(progress_bar, 50);

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			Sleep(150);

			progress_bar = max(progress_bar, 70);

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			Sleep(100);

			progress_bar = max(progress_bar, 80);

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			Sleep(100);

			progress_bar = max(progress_bar, 90);

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			Sleep(100);

			progress_bar = max(progress_bar, 100);

			SendMessage(hInitialWindow, WMP_UPDATE_LOGIN_PROGRESS, (WPARAM)progress_bar, (LPARAM)0);

			RemoveLoginProgressIndicator();

		}

	}

	return (LoggedIn == LOGIN_VALID);

};//preLogInToServer

/**********************************************************************************

 Function LogInToServer(char *detail_msg)



 Purpose: modal function to attempt login to server

 Returns: TRUE if logged in successfully, FALSE if not.  Check LoggedIn (global) for:

	LOGIN_NO_LOGIN, LOGIN_INVALID, LOGIN_BAD_USERID, 

	LOGIN_BAD_PASSWORD, LOGIN_VALID, LOGIN_ANONYMOUS

***********************************************************************************/

int LogInToServer(char *detail_msg)

{

	
	if((((ClientVersionNumber.major  & 0x0000FFFF )< (ServerVersionInfo.min_client_version.major  & 0x0000FFFF)) ||
		 ((ClientVersionNumber.minor  & 0x0000FFFF )< (ServerVersionInfo.min_client_version.minor  & 0x0000FFFF)) )||  

		 (((ClientVersionNumber.major  & 0x0000FFFF )==(ServerVersionInfo.min_client_version.major  & 0x0000FFFF))&&
		 ((ClientVersionNumber.minor  & 0x0000FFFF )==(ServerVersionInfo.min_client_version.minor  & 0x0000FFFF))&&
		 ((ClientVersionNumber.build & 0x0000FFFF ) < (ServerVersionInfo.min_client_version.build & 0x0000FFFF)))&&

		 ((!COMPILE_ADMIN) && (!COMPILE_ADMIN_SHOT))){
	        
			 ShowWindow(hCardRoomDlg, SW_HIDE);

			 iDisableServerCommunications = FALSE;

		     EnterCriticalSection(&ClientSockCritSec);

		     if (ClientSock) {
			     ClientSock->CloseSocket();
			 };//if
//cris 23-1-2004
			 //To force te download
#if 0
//end cris 23-1-2004

		     LeaveCriticalSection(&ClientSockCritSec);
			 int res=MessageBox(hCardRoomDlg, "New e-Media Poker Version Found \n Do you want to process the upgrader ?", "e-Media Poker Upgrader",MB_YESNO);
			 if(res==IDYES){
		          CheckForUpgrade(FALSE);
				  ShowWindow(hCardRoomDlg, SW_SHOW);
			 }else{
                 //return preLogInToServer(detail_msg);
				  PostMessage(hCardRoomDlg, WMP_CLOSE_YOURSELF, 0, 0);
			 };//if to process the upgrader
//cris 23-1-2004
#endif
//endcris 23-1-2004 
		
//cris 23-1-2004
			 //to force the download process 
			 //nerver ask
			 //hide cardroom
			 ShowWindow(hCardRoomDlg, SW_HIDE);
		     CheckForUpgrade(FALSE);
//cris 23-1-2004
			 

	 }else{
	      return preLogInToServer(detail_msg);
	 };//if
	 return LOGIN_NO_LOGIN;
	
}



/**********************************************************************************

 Function CALLBACK dlgFuncLoggingInStatus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 

 Purpose: dlg procedure for little logging-in status bar

***********************************************************************************/

BOOL CALLBACK dlgFuncLoggingInStatus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {



	case WM_INITDIALOG:

		PostMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM)1, 0);

		return TRUE;



	case WM_DESTROY:

		hLoginProgress = NULL;

		return TRUE;

	

	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hDlg);

		return TRUE;

	}

	

	NOTUSED(hDlg);

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;

}



/**********************************************************************************

 Function *TrimString(char *str)

 

 Purpose: trim leading and trailing spaces from a string

 Returns: ptr to the string (and modifies original)

***********************************************************************************/

char *TrimString(char *str, int max_str_len)

{

	// trim leading spaces and control characters...

	str[max_str_len-1] = 0;

	while (*str > 0 && *str <= ' ') {

		memmove(str, str+1, max_str_len-1);	// shift left.

	}

	// trim trailing spaces...

	while (strlen(str) && str[strlen(str)-1] == ' ') {

		str[strlen(str)-1] = 0;

	}

	return str;

}



/**********************************************************************************

 Function CALLBACK dlgLoginFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)



 Purpose: dialog procedure for logging in (this is a modal dlg)

***********************************************************************************/

BOOL CALLBACK dlgLoginFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) dlgLoginFunc(): message = %d\n",_FL, message));

	switch (message) {

	case WM_INITDIALOG:
         
		hLogInDLG = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_LOGIN_NAME, LoginUserID);

		SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_LOGIN_PASSWORD, LoginPassword);

		SetDlgItemTextIfNecessary(hDlg, IDC_LOGIN_DETAIL_MSG, (char *)lParam);

		// Defaults.forget_login_preferences = 1;

		// WriteDefaults(1);

		CheckDlgButton(hDlg, IDC_CHECK_REMEMBER_SETTINGS,

			Defaults.forget_login_preferences ? BST_UNCHECKED : BST_CHECKED);



		// If it has been less than 10 minutes since they created an account,

		// disable the 'create new account' button.

		{

		  #if ADMIN_CLIENT

			if (iAdminClientFlag) {

				Defaults.last_create_account_time = 0;	// always reset in admin mode

			}

		  #endif

			DWORD now = time(NULL);

			if (now >= Defaults.last_create_account_time &&

				now <= Defaults.last_create_account_time+10*60)

			{

				EnableWindow(GetDlgItem(hDlg, ID_CREATE_ACCOUNT), FALSE);

			}



			// Figure out the real time according to the server and make the

			// time zone adjustment so what we end up displaying is in CST.

			time_t server_time = time(NULL) + TimeDiffWithServer - SERVER_TIMEZONE;

			struct tm *tm = gmtime(&server_time);

			char str[200];

			zstruct(str);

			if (tm) {

				sprintf(str, "Login at %d:%02d CST\r\n", tm->tm_hour, tm->tm_min);

			} else {

				sprintf(str, "Logging in ");

			}

			if (szOurIPString[0]) {

				sprintf(str+strlen(str), "from %s", szOurIPString);

			}

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_LOGIN_SOURCE), str);

		}



		

		// If both user id and password are filled in, select the user id.  If just

		// the user id is filled in, select the password as the default control.

		if (!LoginUserID[0] || LoginPassword[0]) {

			HWND h = GetDlgItem(hDlg, IDC_EDIT_LOGIN_NAME);

			SetFocus(h);

			SendMessage(h, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

		} else {

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_LOGIN_PASSWORD));

		}

		return FALSE;	// FALSE because we're setting the focus

	case WMP_UPDATE_YOURSELF:

		SetDlgItemTextIfNecessary(hDlg, IDC_LOGIN_DETAIL_MSG, (char *)lParam);

		return TRUE;

		

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:


			GetDlgItemText(hDlg, IDC_EDIT_LOGIN_NAME, LoginUserID, MAX_PLAYER_USERID_LEN);

			GetDlgItemText(hDlg, IDC_EDIT_LOGIN_PASSWORD, LoginPassword, MAX_PLAYER_PASSWORD_LEN);

			// if either is blank, tell them.

			if (!strlen(LoginUserID)) {

				MessageBox(hDlg,

						"You must enter your UserID to log in.\n\n"

						"If you do not yet have a UserID, click\n"

						"'Create New Account' to create one.",

						"Logging in...", MB_OK|MB_APPLMODAL|MB_TOPMOST);

				HWND h = GetDlgItem(hDlg, IDC_EDIT_LOGIN_NAME);

				SetFocus(h);

				SendMessage(h, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

				return TRUE;

			}

			if (!strlen(LoginPassword)) {

				MessageBox(hDlg,

						"Your password cannot be blank.\n\n"

						"Please enter your password to log in.",

						"Logging in...", MB_OK|MB_APPLMODAL|MB_TOPMOST);

				SetFocus(GetDlgItem(hDlg, IDC_EDIT_LOGIN_PASSWORD));

				return TRUE;

			}

			// trim leading and trailing spaces from both

			TrimString(LoginUserID, MAX_PLAYER_USERID_LEN);

			TrimString(LoginPassword, MAX_PLAYER_PASSWORD_LEN);

			Defaults.forget_login_preferences =

				(BOOL8)(IsDlgButtonChecked(hDlg, IDC_CHECK_REMEMBER_SETTINGS) == BST_CHECKED ? FALSE : TRUE);

			if (!Defaults.forget_login_preferences) {

				strnncpy(Defaults.user_id, LoginUserID, MAX_PLAYER_USERID_LEN);

				strnncpy(Defaults.password, EncryptPassword(LoginPassword), MAX_PLAYER_PASSWORD_LEN);

			} else {

				// security blankout

				memset(Defaults.user_id,  0, MAX_PLAYER_USERID_LEN);

				memset(Defaults.password, 0, MAX_PLAYER_PASSWORD_LEN);

			}
		
			 
			 
			Defaults.changed_flag = TRUE;
			// send the actual login packet
			LastLoginStatus = -1;	// clear this so we get new status messages
			AutoLoginFlag = TRUE;	// flag to send login automatically whenever we (re)connect.
			SetDlgItemTextIfNecessary(hDlg, IDC_LOGIN_DETAIL_MSG, "Logging in...");
			EnableWindow(GetDlgItem(hLogInDLG,IDOK),FALSE);
			SendLoginInfo(LoginUserID, LoginPassword, iRequestPriorityLogin);
			RequestAllTableLists();
			EndDialog(hDlg, IDOK);
	
           
			return TRUE;

		case IDCANCEL:

			RemoveBuyInDLG();	// if we're buying in somewhere, abort it

			EndDialog(hDlg, IDCANCEL);

			return TRUE;

		case ID_CREATE_ACCOUNT:

			EndDialog(hDlg, IDCANCEL);

			if (CreateNewAccount()==IDRETRY) {

				// successfull... bring ourselves back up again.

				EndDialog(hDlg, IDRETRY);	// override return code.

				Defaults.last_create_account_time = time(NULL);

				Defaults.changed_flag = TRUE;

			}

			return TRUE;

		}

		break;



	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		hLogInDLG = NULL;

		return TRUE;

	}

    return FALSE;

}



//*********************************************************



//

// dlgFunc for the Progress Window

//

static HWND hProgressWindowDlg;	// if set, handle to open progress window

static int iProgressPosition;	// current position of progress indicator

static int iProgressDesiredPos;	// desired position of progress indicator

static int iProgressRate;		// rate (per WM_TIMER) it should move by

static char *szProgressMessage;	// message to put into window



static BOOL CALLBACK dlgFuncProgressWindow(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) dlgFuncProgressWindow() got message %d\n", _FL, message));

	switch (message) {

	case WM_INITDIALOG:

		SetTimer(hDlg, WM_TIMER, 1000/20, NULL);	// 20Hz timer

		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM)iProgressPosition, 0);

		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));

		SetWindowTextIfNecessary(hDlg, szProgressMessage);

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_TIMER:

		{

			int pos = min(iProgressDesiredPos, iProgressPosition + iProgressRate);

			if (pos != iProgressPosition) {

				iProgressPosition = pos;

				PostMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM)iProgressPosition, 0);

			}

		}

		return TRUE;

	case WM_DESTROY:

		//kp(("%s(%d) WM_DESTROY received.\n",_FL));

		KillTimer(hDlg, WM_TIMER);	// remove our timer.

		hProgressWindowDlg = NULL;

		return TRUE;

	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hDlg);

		return TRUE;

	}

	

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;

}



//*********************************************************



//

// Open a new progress window.

// Returns handle to the window.

// Close the window using FinishProgressWindow()

//

HWND CreateProgressWindow(HWND owner_hwnd, char *msg, int percent_to_stop_at, int duration_in_ms)

{

	if (hProgressWindowDlg) {

		// already open... don't open another.

		kp(("%s(%d) Progress window already open.  Not opening another one.\n",_FL));

		return NULL;

	}



	// Save the desired animation conditions...

	iProgressPosition = 0;

	iProgressDesiredPos = percent_to_stop_at;

	iProgressRate = max(1, (percent_to_stop_at * (1000/20)) / max(1,duration_in_ms));

	//kp(("%s(%d) Progress rate = %d\n", _FL, iProgressRate));

	szProgressMessage = msg;



	// Open the window...

	hProgressWindowDlg = CreateDialog(hInst,

			MAKEINTRESOURCE(IDD_LOGGING_IN_STATUS),

			owner_hwnd, dlgFuncProgressWindow);

	return hProgressWindowDlg;

}



//*********************************************************



//

// Dispatch all pending messages for a particular dialog window

//

void DispatchMessagesForHwnd(HWND hwnd)

{

	MSG msg;

	zstruct(msg);

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

		//kp(("%s(%d) PeekMessage found message %d\n", _FL, msg.message));

		if (!IsDialogMessage(hwnd, &msg)) {

			//kp(("%s(%d) IsDialogMessage() returned FALSE... getting, translating, and dispatching.\n",_FL));

			GetMessage(&msg, NULL, 0, 0);

			TranslateMessage(&msg);

			DispatchMessage(&msg);

		} else {

			//kp(("%s(%d) IsDialogMessage() returned TRUE.  Ignoring it.\n",_FL));

		}

	}

}



//*********************************************************



//

// Close an already open progress window opened by CreateProgressWindow()

//

void FinishProgressWindow(HWND hwnd, int duration_in_ms)

{

	if (hwnd) {

		iProgressDesiredPos = 100;

		iProgressRate = max(1, ((iProgressDesiredPos-iProgressPosition) * (1000/20)) / max(1,duration_in_ms));

		//kp(("%s(%d) FinishProgressWindow()... waiting for meter to finish.\n",_FL));

		while (iProgressPosition < iProgressDesiredPos) {

			DispatchMessagesForHwnd(hwnd);

			Sleep(50);

		}

		//kp(("%s(%d) FinishProgressWindow()... done waiting for meter to finish.\n",_FL));

		Sleep(100);	// let the user see the finished dialog box

		SendMessage(hwnd, WMP_CLOSE_YOURSELF, 0, 0);

	}

}



//*********************************************************


//

// Update the connection status and also update cardroom's

// title bar to indicate whether we're currently connected or not.

// This MUST be called from the message thread!

//

void UpdateConnectionStatus(int force_update)

{

	static int old_status;

	static int old_delay;



	int new_status = 0;	// default to not connected.

	EnterCriticalSection(&ClientSockCritSec);

	if (ClientSock) {

		if (ClientSock->disconnected) {

			if (ClientSock->connected_flag) {

				new_status = 1;	// disconnected

			} else {

				new_status = 2;	// unable to establish

			}

		} else {

			if (ClientSock->connected_flag && ClientSockConnected) {

				new_status = 3;	// currently connected

			} else {

				new_status = 4;	// establishing connection

			}

		}

	}

	LeaveCriticalSection(&ClientSockCritSec);



	#define HEARTBEAT_SPACING	10

	int overdue_seconds = SecondCounter - dwLastPacketReceivedSeconds - HEARTBEAT_SPACING;

	if (overdue_seconds < 0) {

		overdue_seconds = 0;

	}

	if (new_status==3 && overdue_seconds >= 5) {

		// We're connected and there seems to be some network latency.

		if (overdue_seconds >= 14){

			new_status = 5;	// display 'internet delay'

		}

		if (overdue_seconds != old_delay) {

			force_update = TRUE;

		}

		// Whenever we're experiencing delays, make sure we send

		// out packets to try to resync asap.

		if (SecondCounter - dwLastPingSentTime >= 4) {

		  #if 1	//19991210MB: pings aren't necessary... they just eat bandwidth

			SendKeepAlive();

		  #else

			SendPing();

		  #endif

		}

		if (overdue_seconds >= 20) {

			// We're currently connected but we've gone a while without any

			// contact with the server.  Assume the connection is dead

			// and try to log in again.

			kp(("%s(%d) Giving up on current connection and trying to make a new one.\n",_FL));

			EnterCriticalSection(&ClientSockCritSec);

			if (ClientSock) {

				ClientSock->CloseSocket();

			}

			LeaveCriticalSection(&ClientSockCritSec);

		}

	}

	old_delay = overdue_seconds;

  #if ADMIN_CLIENT

	if (SecondCounter <= dwEarliestPacketSendTime) {

		static int old_freeze_time;

		int freeze_time = dwEarliestPacketSendTime - SecondCounter;

		if (freeze_time != old_freeze_time) {

			force_update = TRUE;

			old_freeze_time = freeze_time;

		}

	}

  #endif

	if (new_status != old_status || force_update) {

		if (hCardRoomDlg) {

			char connection_status[100];

			zstruct(connection_status);

			switch (new_status) {

			case 0:

				strcat(connection_status, "(not connected)");

				break;

			case 1:

				strcat(connection_status, "(disconnected)");

				break;

			case 2:

				strcat(connection_status, "(unable to connect)");

				break;

			case 3:

                



				if (szOurIPString[0]) {

					sprintf(connection_status+strlen(connection_status),

							"(connected from %s)", szOurIPString);

				} else {

					strcat(connection_status, "(connected)");

				}





				if (szOurIPString[0]) {

					sprintf(connection_status+strlen(connection_status),

							" ", szOurIPString);

				} else {

					strcat(connection_status, "(connected)");

				}

                



				break;

			case 4:

				strcat(connection_status, "(attempting connection)");

				break;

			case 5:

				sprintf(connection_status, "(internet delay: %ds)", overdue_seconds);

				break;

			}

			strcpy(ConnectionStatusString, connection_status);

			char str[150];



			   strcpy(str, "-MediaSoftware Lobby ");



		  #if ADMIN_CLIENT	//

		  	if (iAdminClientFlag) {

				if (LoggedIn == LOGIN_VALID) {

					char login_info[50];

					sprintf(login_info,"-- logged in as %s ", LoginUserID);

					strcat(str,login_info);

				} else {

					strcat(str,"-- not logged in ");

				}

			}



			if (SecondCounter < dwEarliestPacketSendTime) {

				char freeze_info[50];

				sprintf(freeze_info," (I/O locked out for %ds)  ", dwEarliestPacketSendTime - SecondCounter);

				strcat(str,freeze_info);

			}

		  #endif



  			// strcat(str, connection_status);

			

			SetWindowTextIfNecessary(hCardRoomDlg, str);



			// Change any open table windows

			UpdateTableWindowTitles();

		}

		old_status = new_status;

	}

}



//*********************************************************


//

// Read a DWORD from the registry.

//

ErrorType RegReadDword(HKEY root_key, char *path, char *name, DWORD *output_value)

{

	ErrorType err = ERR_ERROR;

	HKEY hKey = 0;

	int result = RegOpenKeyEx(root_key,

				path, 0, KEY_QUERY_VALUE, &hKey);

	if (result==ERROR_SUCCESS) {

		DWORD dataSize = sizeof(*output_value);

		result = RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)output_value, &dataSize);

		if (result==ERROR_SUCCESS) {

			err = ERR_NONE;

		}

		RegCloseKey (hKey);

	}

	return err;

}



//*********************************************************


//

// Write a DWORD to the registry. Only writes if it has changed.

//

ErrorType RegWriteDword(HKEY root_key, char *path, char *name, DWORD value)

{

	DWORD old_val = 0;

	ErrorType err = RegReadDword(root_key, path, name, &old_val);

	if (err != ERR_NONE || old_val != value) {

		// It needs writing...

		HKEY keyhandle = 0;

		DWORD disposition = 0;

		LONG result = RegCreateKeyEx(root_key, path,

				0, "Data", 0, KEY_ALL_ACCESS,

				NULL, &keyhandle, &disposition);

		if (result==ERROR_SUCCESS) {

			result = RegSetValueEx(keyhandle, name, 0, REG_DWORD,

					(CONST BYTE *)&value, sizeof(value));

			if (result != ERROR_SUCCESS) {

				kp(("%s(%d) RegSetValueEx() return error %d\n", _FL, result));

				err = ERR_ERROR;

			} else {

				err = ERR_NONE;

			}

			RegCloseKey(keyhandle);

		} else {

			kp(("%s(%d) RegCreateKeyEx returned an error.", _FL));

			err = ERR_ERROR;

		}

	}

	return err;

}



//*********************************************************

//

// Construct various pathnames to files used to store the computer serial number.

//

#if 0	//

  #define COMPUTER_KEY_REG_PATH		"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"

  #define COMPUTER_KEY_REG_ENTRY	"EnableIPFlags"

#else	// rot13 versions of the same strings:



  

  #define COMPUTER_KEY_REG_PATH_r13	 "Fbsgjner\\Zvpebfbsg\\Jvaqbjf\\PheeragIrefvba\\VCPbasYFC"

  #define COMPUTER_KEY_REG_ENTRY_r13 "YbpnyVCPnpur"



  /* ---- changed to IPConfLSP/LocalIPCache --------------------------------*/



  

  #define COMPUTER_KEY_FILECOUNT	3	// # of files we write it to







  #define COMPUTER_KEY_FNAME1_r13	"WinsystemFile9"	// "JvaflfgrzSvyr1"

  #define COMPUTER_KEY_FNAME1_XOR	0x58fe8a32

  #define COMPUTER_KEY_FNAME2_r13	"NToperation9"	//   "AGbcrengvba2"

  #define COMPUTER_KEY_FNAME2_XOR	0x95e3512a

  #define COMPUTER_KEY_FNAME3_r13	"98NTdosfile9"	//   "98AGqbfsvyr3"

  #define COMPUTER_KEY_FNAME3_XOR	0x12387ed3

  







#endif



static void BuildCompSerNumPathname(int index, char *dest, WORD32 *output_xor_value)

{

	char fname[MAX_FNAME_LEN];

	zstruct(fname);

	*dest = 0;

	char *envp;

	switch (index) {

	case 0:

	  #if 0	// not used because it gets into a bunch of OLE

			// stuff for freeing memory and I don't want to deal with that

			// at the last minute before releasing something.

		{

			LPITEMIDLIST ppidl = NULL;

			HRESULT hresult = SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &ppidl);

			if (ppidl) {



			}



		}

	  #endif

		envp = getenv("USERPROFILE");

		if (envp) {

			strcpy(dest, envp);

			AddBackslash(dest);

			rot13(COMPUTER_KEY_FNAME1_r13, fname, sizeof(fname));

			strcat(dest, fname);

		}

		*output_xor_value = COMPUTER_KEY_FNAME1_XOR;

		break;

	case 1:

		envp = getenv("SYSTEMDRIVE");

		if (!envp) {

			envp = getenv("WINBOOTDIR");

		}

		if (!envp) {

			envp = getenv("WINDIR");

		}

		if (envp) {

			GetRootFromPath(envp, dest);

			AddBackslash(dest);

			rot13(COMPUTER_KEY_FNAME2_r13, fname, sizeof(fname));

			strcat(dest, fname);

		}

		*output_xor_value = COMPUTER_KEY_FNAME2_XOR;

		break;

	case 2:

		envp = getenv("TEMP");

		if (!envp) {

			envp = getenv("TMP");

		}

		if (envp) {

			strcpy(dest, envp);

			AddBackslash(dest);

			rot13(COMPUTER_KEY_FNAME3_r13, fname, sizeof(fname));

			strcat(dest, fname);

		}

		*output_xor_value = COMPUTER_KEY_FNAME3_XOR;

		break;

	}

}



//*********************************************************

//

// Read and decide the computer serial number in a file.

//

WORD32 ReadComputerSerialNumberFromFile(int file_index)

{

	char path[MAX_FNAME_LEN];

	zstruct(path);

	WORD32 xor_value = 0;

	BuildCompSerNumPathname(file_index, path, &xor_value);

	WORD32 existing_data = xor_value;

	ReadFile(path, &existing_data, sizeof(WORD32), NULL,NULL);

	existing_data ^= xor_value;

	return existing_data;

}



//*********************************************************


//

// Write comp ser number to a particular file if it is not

// already in that file.

//

void WriteComputerSerialNumberToFile(int file_index, WORD32 computer_serial_number)

{

	// First, try to load the file and see if the data is already there.

	WORD32 existing_sernum = ReadComputerSerialNumberFromFile(file_index);

	//kp(("%s(%d) Existing s/n in file %d is %d\n", _FL, file_index, existing_sernum));

	if (existing_sernum != computer_serial_number) {

		// Write the new one.

		char path[MAX_FNAME_LEN];

		zstruct(path);

		WORD32 xor_value = 0;

		BuildCompSerNumPathname(file_index, path, &xor_value);

		WORD32 value_to_write = computer_serial_number ^ xor_value;



		// stat the existing file (if any) to get the modification time prior to writing

		// (so we can preserve it)

		struct _stat original_stat;

		zstruct(original_stat);

		if (_stat(path, &original_stat)) {	// non-zero indicates an error

			zstruct(original_stat);	// assume nothing learned.

		}



		// Now write out the file.

		WriteFile(path, &value_to_write, sizeof(value_to_write),NULL,NULL);



		// Restore the modification time and mark the file as hidden and system.

		struct _utimbuf times;

		zstruct(times);

		times.actime = original_stat.st_atime;

		times.modtime = original_stat.st_mtime;

		time_t now = time(NULL);

		

		if (!times.actime) {

			times.actime = now - 7*24*3600;	// Change the date to 7 days ago

		}

		if (!times.modtime) {

			times.modtime = now - 7*24*3600;	// Change the date to 7 days ago

		}

		_utime(path, &times);

        

		// change the access attributes

		SetFileAttributes(path, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM);

	}

}



//*********************************************************

//

// Save the computer serial number to the registry

// Only writes it if it's not already there.

//

// Registry key entry settings for saving computer serial number

//

void SaveComputerSerialNumber(WORD32 computer_serial_number)

{

	for (int i=0 ; i<COMPUTER_KEY_FILECOUNT ; i++) {

		WriteComputerSerialNumberToFile(i, computer_serial_number);

	}



	char unrot13_path[200];

	char unrot13_entry[200];

	rot13(COMPUTER_KEY_REG_PATH_r13,  unrot13_path,  sizeof(unrot13_path));

	rot13(COMPUTER_KEY_REG_ENTRY_r13, unrot13_entry, sizeof(unrot13_entry));

  #if 0	//

	kp(("%s(%d) unrot13'd strings:\n",_FL));

	kp(("   %s\n", unrot13_path));

	kp(("   %s\n", unrot13_entry));

  #endif

	RegWriteDword(HKEY_CURRENT_USER, unrot13_path,

				unrot13_entry, computer_serial_number);

	RegWriteDword(HKEY_LOCAL_MACHINE, unrot13_path,

				unrot13_entry, computer_serial_number);

	if (Defaults.computer_serial_number != computer_serial_number) {

		Defaults.computer_serial_number = computer_serial_number;

		Defaults.changed_flag = TRUE;

	}

}	



//*********************************************************

//

// Fill in and send out a DATATYPE_CLIENT_PLATFORM packet.

//

void SendClientPlatformInfo(void)

{

	struct ClientPlatform cp;

	zstruct(cp);



	SYSTEM_INFO si;

	zstruct(si);

	GetSystemInfo(&si);

	cp.base_os = 0;		// Windows

	cp.cpu_count = (BYTE8)max(1,min(255,si.dwNumberOfProcessors));

	cp.cpu_level = (BYTE8)si.wProcessorLevel;

	cp.cpu_revision = si.wProcessorRevision;

	cp.local_ip = OurLocalIP;

	if (cp.cpu_level == 0) {	// did we find out nothing?

		// Use the CPUID instruction to try to get some info...

		// We can't use cpuid on pre-pentium processors.



		// AMD CPU id: http://www.amd.com/products/cpg/athlon/techdocs/pdf/20734.pdf



		DWORD eflags;

		_asm {

			pushfd				;save existing eflags (twice)

			pushfd

			pop	eax				;grab existing eflags

			or	eax,0200000h	;try to set bit 21

			push	eax

			popfd

			pushfd

			pop	eax				;retrieve after trying to set

			popfd				;restore original eflags

			mov [eflags],eax

		}

		if (eflags & 0x0200000) {	// bit 21 set?

			// yes, CPUID instruction is available...

			DWORD cpuidinfo[4];

			_asm {

				mov eax,1		;request feature flags

				sub edx,edx

				_emit 0x0f

				_emit 0xa2

				mov [cpuidinfo],eax

				mov [cpuidinfo+4],edx

				mov [cpuidinfo+8],ecx

				mov [cpuidinfo+12],ebx

			}

			//	Type (bits 13-12), Family (bits 11-8), Model (bits 7-4), Stepping (bits 3-0)

			//	T = 00, F = 0101, M = 0001, for Pentium Processors (60, 66 MHz)

			//	T = 00, F = 0101, M = 0010, for Pentium Processors (75, 90, 100, 120, 133, 150, 166, 200 MHz)

			//	T = 00, F = 0101, M = 0100, for Pentium Processors with MMX technology//

			//	T = 00, F = 0110, M = 0001, for Pentium Pro Processor

			//	T = 00, F = 0110, M = 0011, for Pentium II Processor

	

			//	T = 00 for original OEM processor

			//	T = 01 for Intel OverDrive Processor

			//	T = 10 for dual processor

			//	T = 11 is reserved



			//kp(("%s(%d) cpuidinfo: $%08lx %08lx %08lx %08lx\n", _FL, cpuidinfo[0],cpuidinfo[1],cpuidinfo[2],cpuidinfo[3]));

			//int type     = (cpuidinfo[0] >> 12) & 0x03;

			int family   = (cpuidinfo[0] >> 8)  & 0x0f;

			int model    = (cpuidinfo[0] >> 4)  & 0x0f;

			int stepping =  cpuidinfo[0]        & 0x0f;

			cp.cpu_level = (BYTE8)family;

			sprintf(cp.cpu_identifier_str, "x86 Family %d Model %d Stepping %d",

						family, model, stepping);

			cp.cpu_revision = (WORD16)((model<<8) | stepping);

		}

	}



	// Determine if we can use the rdtsc() function/instruction

	// (Pentium and later only)

	iRDTSCAvailFlag = FALSE;	// default to not avail.

	if (cp.cpu_level >= 5) {

		iRDTSCAvailFlag = TRUE;

	}



	// Try to grab the processor MHz out of the registry...

	HKEY hKey;

	int result = RegOpenKeyEx (HKEY_LOCAL_MACHINE,

		"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);

	if (result == ERROR_SUCCESS) {

		DWORD data, dataSize;

		data = 0;

		dataSize = sizeof(data);

	  #if 1	//

		result = RegQueryValueEx (hKey, "~MHz", NULL, NULL, (LPBYTE)&data, &dataSize);

		if (result==ERROR_SUCCESS) {

			cp.cpu_mhz = (WORD16)data;

		}

	  #endif



		dataSize = MAX_COMMON_STRING_LEN;

		RegQueryValueEx(hKey, "VendorIdentifier", NULL, NULL, (LPBYTE)cp.cpu_vendor_str, &dataSize);

		dataSize = CPU_IDENTIFIER_STR_LEN;

		RegQueryValueEx(hKey, "Identifier", NULL, NULL, (LPBYTE)cp.cpu_identifier_str, &dataSize);

		RegCloseKey (hKey);

	}



	// Try to retrieve our computer serial number and send it to the server

	char unrot13_path[200];

	char unrot13_entry[200];

	rot13(COMPUTER_KEY_REG_PATH_r13,  unrot13_path,  sizeof(unrot13_path));

	rot13(COMPUTER_KEY_REG_ENTRY_r13, unrot13_entry, sizeof(unrot13_entry));

	//RegReadDword(HKEY_LOCAL_MACHINE, unrot13_path,

	//			unrot13_entry, (DWORD *)&cp.computer_serial_num);

	if (!cp.computer_serial_num) {

	//	RegReadDword(HKEY_CURRENT_USER, unrot13_path,

	//			unrot13_entry, (DWORD *)&cp.computer_serial_num);

	}



	int i;

	for (i=0 ; i<COMPUTER_KEY_FILECOUNT && !cp.computer_serial_num; i++) {

		cp.computer_serial_num = ReadComputerSerialNumberFromFile(i);

	}



	if (!cp.computer_serial_num) {

		cp.computer_serial_num = Defaults.computer_serial_number;

		if (cp.computer_serial_num) {

			cp.flags |= CPFLAG_SERIAL_NUMBER_MISMATCH;	// indicate we found a mismatch

		}

	}



	if (cp.computer_serial_num) {

		SaveComputerSerialNumber(cp.computer_serial_num);	// always write back to all places

	}



	//kp(("%s(%d) Computer serial number = %d\n", _FL, cp.computer_serial_num));



	// If we didn't find the MHz yet, try using QueryPerformanceFrequency...

	if (!cp.cpu_mhz) {

		LARGE_INTEGER pcf;

		if (QueryPerformanceFrequency(&pcf)) {

			// That succeeded... let's see what we got back.

			if (pcf.QuadPart >= 50000000) {	// is it fast enough to be processor frequency?

				// yes, it's fast enough; let's just assume that it is.

				// Round to nearest MHz.

				cp.cpu_mhz = (WORD16)((pcf.QuadPart + 500000) / 1000000);

			}

		}

	}

	// If we STILL don't know the MHz, try executing RDTSC for a short period.

	if (!cp.cpu_mhz && iRDTSCAvailFlag) {

		ULONGLONG freq = 0;

		for (int loop = 0 ; loop < 2 ; loop++) {	// loop twice, first time to get into cache.

			DWORD ticks = GetTickCount();

			DWORD startticks;

			while (ticks==(startticks=GetTickCount())) {

				;

			}

			ULONGLONG start = rdtsc();

			DWORD endticks = startticks + (loop ? 200 : 10);	// long loop 2nd time only

			while ((ticks=GetTickCount()) < endticks) {

				;

			}

			ULONGLONG end = rdtsc();

			DWORD elapsed_ticks = ticks - startticks;

			freq = (end - start) * 1000 / elapsed_ticks;

			//kp(("%s(%d) elapsed ticks = %d, freq = %u\n", _FL, elapsed_ticks, (DWORD)freq));

		}

		if (freq >= 50000000) {	// is it fast enough to be processor frequency?

			// yes, it's fast enough; let's just assume that it is.

			// Round to nearest MHz.

			cp.cpu_mhz = (WORD16)((freq + 500000) / 1000000);

		}

	}



	OSVERSIONINFO vi;

	zstruct(vi);

	vi.dwOSVersionInfoSize = sizeof(vi);

	GetVersionEx(&vi);

	if (vi.dwPlatformId==VER_PLATFORM_WIN32_NT) {

		cp.version = (BYTE8)vi.dwMajorVersion;

	} else {

		// Win95, Win98...

		cp.version = (BYTE8)(vi.dwMinorVersion ? 1 : 0);	// Win98 is identified with a non-zero minor version number

	}



	// Record screen info...

	HDC hdc = GetDC(NULL);

	cp.screen_bpp    = (BYTE8)GetDeviceCaps(hdc, BITSPIXEL);

	cp.screen_width  = (WORD16)GetDeviceCaps(hdc, HORZRES);

	cp.screen_height = (WORD16)GetDeviceCaps(hdc, VERTRES);

	ReleaseDC(NULL, hdc);

	if (iLargeFontsMode) {

		cp.flags |= CPFLAG_LARGE_FONTS;

	}



	MEMORYSTATUS ms;

	zstruct(ms);

	ms.dwLength = sizeof(ms);

	GlobalMemoryStatus(&ms);

	cp.installed_ram = (WORD16)((ms.dwTotalPhys+(1<<20)-1) >> 20);	// installed RAM in MB.



	// Finally, do some rounding on the MHz because it's not very accurate.  Most

	// CPU's are frequencies end in 00, 33, 50, or 66... round to those.

	int last2 = cp.cpu_mhz % 100;

	int round_numbers[] =  {0,33,50,66,100,25,75,-1};

	for (i=0 ; round_numbers[i] >= 0 ; i++) {

		int diff = round_numbers[i] - last2;

		if (abs(diff) <= 5) {

			cp.cpu_mhz = (WORD16)(cp.cpu_mhz + diff);

			break;

		}

	}



	// Determine our time zone (seconds from GMT) and convert to fit

	// into 8 bit (7.5m increments = seconds from GMT divided by 450).

	// This number was picked to optimize resolution in 8 bits.

	if (_timezone < 0) {

		cp.time_zone = (char)((_timezone - 225) / 450);

	} else {

		cp.time_zone = (char)((_timezone + 225) / 450);

	}



	// Fill in our available drive space fields...

	{	char dir[MAX_FNAME_LEN];

		getcwd(dir, MAX_FNAME_LEN);

		cp.our_drive_letter = dir[0];

		cp.disk_space_on_our_drive = CalcFreeDiskSpace(dir)/1024;

		GetSystemDirectory(dir, MAX_FNAME_LEN); 

		cp.disk_space_on_system_drive = CalcFreeDiskSpace(dir)/1024;

		cp.system_drive_letter = dir[0];

	}



	SendDataStructure(DATATYPE_CLIENT_PLATFORM, &cp, sizeof(cp));

}

