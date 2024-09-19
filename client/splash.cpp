//****************************************************************


//

// Splash.cpp: Boot-up splash screen related stuff

//

//****************************************************************



#include "stdafx.h"



HWND hSplashDlg;			// handle to splash dialog box (if open)

HBITMAP hSplashBgnd;		// handle to background picture for dialog box.

HPALETTE hSplashPalette;	// handle to palette for bgnd picture

static int iCurrentConnectionProgress;	// 0-100%

static int iDesiredConnectionProgress;	// 0-100%

int iConnectionStatus;			// CONNSTAT_*



//*********************************************************



//

// Update the current position of the progress meter between

// iCurrentConnectionProgress and iDesiredConnectionProgress.

// This function can be called from WM_TIMER or anywhere else

// at regular intervals.

//

static void UpdateProgressPosition(void)

{

	int new_position = iCurrentConnectionProgress;

	if (iCurrentConnectionProgress < iDesiredConnectionProgress) {

		static DWORD old_ticks;

		DWORD ticks = GetTickCount();

		DWORD elapsed_ticks = min(100, ticks - old_ticks);

		int divider = (iConnectionStatus==CONNSTAT_CONNECTED) ? 20 : 40;

	  #if ADMIN_CLIENT

		if (iAdminClientFlag) {

			divider /= 8;	// much faster animation in admin mode.

		}

	  #endif

		int position_delta = (elapsed_ticks / divider);

		if (position_delta) {

			new_position += position_delta;

			old_ticks = ticks;

		}

	}

	new_position = min(iDesiredConnectionProgress, new_position);

	if (new_position != iCurrentConnectionProgress) {

		// It changed... update the progress meter.

		iCurrentConnectionProgress = new_position;

		if (hSplashDlg) {	// nothing to do if not open.

			SendMessage(GetDlgItem(hSplashDlg, IDC_CONNECTION_PROGRESS), PBM_SETPOS, (WPARAM)iCurrentConnectionProgress, 0);

		}

	}

}	



//*********************************************************

// 1999/07/29 - MB

//

// Update status and progress on the splash screen.

// Pass CONNSTAT_* as a parameter.

//

void UpdateSplashStatus(int connection_status)

{

	if (hSplashDlg) {	// nothing to do if not open.

		PostMessage(hSplashDlg, WMP_UPDATE_SPLASH_STATUS, (WPARAM)connection_status, 0);

	} else {

		iConnectionStatus = connection_status;

	}

}



//*********************************************************

// 1999/09/18 - MB

//

// Handle the WMP_UPDATE_SPLASH_STATUS message.

//

static void ProcessUpdateSplashStatusMessage(int connection_status)

{

	//kp(("%s(%d) UpdateSplashStatus(%d)\n", _FL, connection_status));

	static int failed_count;

	static int prev_ConnectionStatus;



	if (hSplashDlg) {	// nothing to do if not open.

		// If we're connected and the progress meter has reached the top, close the window.

		if (connection_status == CONNSTAT_CONNECTED && iCurrentConnectionProgress==100) {

		  #if 0	//19990806MB

			iCurrentConnectionProgress = 100;

			iDesiredConnectionProgress = 100;

			SendMessage(GetDlgItem(hSplashDlg, IDC_CONNECTION_PROGRESS), PBM_SETPOS, (WPARAM)iCurrentConnectionProgress, 0);

			SetWindowText(GetDlgItem(hSplashDlg, IDC_CONNECTION_STATUS), "Connected.");

			Sleep(100);	// give connected message a moment to display (very brief).

		  #endif

			PostMessage(hSplashDlg, WMP_CLOSE_YOURSELF, 0, 0);

			PostMessage(hInitialWindow, WMP_SHOW_CARDROOM, 0, 0);

			failed_count = 0;	// reset

			return;	// all done.

		}

		static int ProgressTable[] =  {

			0,		// CONNSTAT_DISCONNECTED,

			10,		// CONNSTAT_RESOLVING,

			30,		// CONNSTAT_CONNECTING,

			50,		// CONNSTAT_SETTINGUP,

			85,		// CONNSTAT_EXCHANGING,

			100,	// CONNSTAT_CONNECTED,

		};

		static char *ProgressMsgTable[] = {

			"Initializing...",				// CONNSTAT_DISCONNECTED,

			"Resolving server name...",		// CONNSTAT_RESOLVING,

			"Connecting socket...",			// CONNSTAT_CONNECTING,

			"Setting up connection...",		// CONNSTAT_SETTINGUP,

			"Exchanging startup data...",	// CONNSTAT_EXCHANGING,

			"Synchronizing...",				// CONNSTAT_CONNECTED,

		};

		iDesiredConnectionProgress = ProgressTable[connection_status];

		iConnectionStatus = connection_status;

		// Update controls to display new status

		char *msg = ProgressMsgTable[connection_status];

		if (iDisableServerCommunications) {

			msg = "Not connected.";	// we're not allowed to try connecting to the server

			EnableWindow(GetDlgItem(hSplashDlg, IDC_RECONNECT), FALSE);	// disable the 'retry' button

		}

		SetWindowTextIfNecessary(GetDlgItem(hSplashDlg, IDC_CONNECTION_STATUS), msg);

		UpdateProgressPosition();



		// Show the "I use AOL" button if necessary.

		if (prev_ConnectionStatus > connection_status && prev_ConnectionStatus != CONNSTAT_CONNECTED) {

			// We're starting over...

			failed_count++;

			if (failed_count >= 2) {

				ShowWindow(GetDlgItem(hSplashDlg, IDC_I_USE_AOL), SW_SHOW);

			}

		}

	}	

	if (iConnectionStatus==CONNSTAT_CONNECTED) {

		failed_count = 0;	// reset

	}

	prev_ConnectionStatus = connection_status;



    switch (connection_status) {

	     case 0:   

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_1, "v");

			 break;



	     case 3:   

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_2, "v");

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_3, "v");

			 break;



		case  4 :   

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_4, "v");

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_5, "v");

			 break;



	     case 5:   

			 SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_6, "v");

			 break;





    }     

}



//****************************************************************

// 1999/07/28 - MB

//

// Mesage handler for Splash dialog

//

BOOL CALLBACK dlgFuncSplash(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) dlgFuncSplash: message = %d\n",_FL, message));

	switch (message) {

	case WM_INITDIALOG:

		iCurrentConnectionProgress = 0;

		ScaleDialogTo96dpi(hDlg);

		SetWindowText(GetDlgItem(hDlg, IDC_CONNECTION_STATUS), "Initializing...");

		SendMessage(GetDlgItem(hDlg, IDC_CONNECTION_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));

		SendMessage(GetDlgItem(hDlg, IDC_CONNECTION_PROGRESS), PBM_SETPOS, (WPARAM)iCurrentConnectionProgress, 0);

    	SetTimer(hDlg, WM_TIMER, 1000/20, NULL);	// 20Hz animation timer

		hSplashDlg = hDlg;



		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_1, " ");

		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_2, " ");

		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_3, " ");

		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_4, " ");

		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_5, " ");

		SetDlgItemText(hSplashDlg, IDC_STATIC_PROCESSMARK_6, " ");



        break;

	

		return TRUE;



	case WM_COMMAND:

		{

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				DestroyWindow(hDlg);

				if (hCardRoomDlg) {

					PostMessage(hCardRoomDlg, WMP_CLOSE_YOURSELF, 0, 0);

				}

				if (hInitialWindow) {

					PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);

				}

				PostQuitMessage(0);

				return TRUE;	// We DID process this message.

			case IDC_RECONNECT:

				EnterCriticalSection(&ClientSockCritSec);

				extern ClientSocket *ClientSock;

				if (ClientSock) {

					ClientSock->CloseSocket();

				}

				LeaveCriticalSection(&ClientSockCritSec);

				return TRUE;	// We DID process this message.

			case IDC_WEB_PAGE:

				LaunchInternetBrowser("http://www.e-mediasoftware.com/network/network.php");

				return TRUE;	// We DID process this message.

			case IDC_I_USE_AOL:

			  	// There is similar wording in comm.cpp.  Search for AOL

				MessageBox(hDlg,

					"If you are using AOL and having trouble connecting, please be sure\n"

					"to follow these steps when connecting to e-Media Poker:\n"

					"    1) Start up AOL and connect to the internet.\n"

					"    2) On AOL, go to the \"Internet\" menu and select \"Go to Web\".\n"

					"    3) Minimize (don't close) AOL.\n"

					"    4) Start e-Media Poker.\n"

					,

					"e-Media Poker.com", MB_OK|MB_ICONINFORMATION|MB_TOPMOST);

				return TRUE;	// We DID process this message.

			}

		}

		break;

	case WM_DESTROY:

       	KillTimer(hDlg, WM_TIMER);	// remove our timer.

		hSplashDlg = NULL;



       	break;

	case WM_PAINT:

		// If we've got a background image, paint it first.

		{

			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && hSplashBgnd) {

				if (hSplashPalette) {

					SelectPalette(hdc, hSplashPalette, FALSE);

					RealizePalette(hdc);

				}

				BlitBitmapToDC_nt(hSplashBgnd, hdc);

			}

			EndPaint(hDlg, &ps);

		}

		break;

	case WM_QUERYNEWPALETTE:

	case WM_PALETTECHANGED:	// palette has changed.

		//kp(("%s(%d) Table got WM_PALETTECHANGED\n", _FL));

	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us

		    HDC hdc = GetDC(hDlg);

		    HPALETTE holdpal = SelectPalette(hdc, hSplashPalette, FALSE);

		    int i = RealizePalette(hdc);   // Realize drawing palette.

			kp(("%s(%d) Palette changed = %d\n", _FL, i));

		    if (i) {	// Did the realization change?

		        InvalidateRect(hDlg, NULL, TRUE);    // Yes, so force a repaint.

		    }

		    SelectPalette(hdc, holdpal, TRUE);

		    RealizePalette(hdc);

		    ReleaseDC(hDlg, hdc);

		    return i;

	    }

		break;



    case WM_CTLCOLORSTATIC:

		{

			// hdcStatic = (HDC) wParam;   // handle to display context 

			// hwndStatic = (HWND) lParam; // handle to static control 

			// chat_hwnd = GetDlgItem(Table[table_index].hwnd, IDC_CHAT_BOX);

			HWND hwnd = (HWND)lParam;

				RECT r;

				GetWindowRect(hwnd, &r);

				ScreenToClient(GetParent(hwnd), &r);

				POINT pt;

				zstruct(pt);

				HDC hdc = (HDC)wParam;

				//BlitBitmapRectToDC(hCardRoomBgnd, hdc, &pt, &r);

				 //SetTextColor(hdc, RGB(0,0,0));

				 //SetTextColor(hdc, RGB(233,242,49));
				 SetTextColor(hdc, RGB(255,255,255));
				 SetBkMode(hdc, TRANSPARENT);

				//SetBkColor(hdc, RGB(0,0,0));

				return ((int)GetStockObject(NULL_BRUSH));

			}

		

		break;

    

	case WM_ERASEBKGND:

		//if (IsIconic(hDlg) || !hSplashBgnd) {

		//	return FALSE;	// FALSE = we did NOT process this message.

		//}

		//return TRUE;	// TRUE = we DID process this message.

    case WM_TIMER:

		UpdateProgressPosition();

		return TRUE;	// TRUE = we DID process this message.

	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hSplashDlg);

		return TRUE;	// TRUE = we DID process this message.

	case WMP_UPDATE_SPLASH_STATUS:

		ProcessUpdateSplashStatusMessage(wParam);



       	break;

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/07/28 - MB

//

// Open the splash dialog window

//

void OpenSplashDialog(void)

{

	// Display the dialog box

	if (!hSplashDlg 

		#if ADMIN_CLIENT

			&& !RunningManyFlag && !AutoJoinDefaultTable

		#endif //ADMIN_CLIENT

	) {

		// Load the background pic

		if (!hSplashBgnd ) {

			// Load the palette for the bgnd pic if necessary

			hSplashPalette = LoadPaletteIfNecessary(FindFile("splash.act"));
			hSplashBgnd = LoadJpegAsBitmap(FindMediaFile("Media\\src_splash.jpg"), hSplashPalette);

		}



		CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONNECTING),

						  	NULL, dlgFuncSplash);

		//kp(("%s(%d) hSplashDlg = $%08lx GetLastError() = %d\n", _FL, hSplashDlg, GetLastError()));

	}

	if (hSplashDlg) {

		SetWindowText(hSplashDlg, "Connecting to Poker Server");

		ShowWindow(hSplashDlg, SW_SHOW);	// make it visible.

	}

}



//****************************************************************

// 1999/07/28 - MB

//

// Mesage handler for PromptFriends dialog

//

static HWND hPromptFriendsDlg;			// handle to PromptFriends dialog box (if open)

static HBITMAP hPromptFriendsBgnd;		// handle to background picture for dialog box.

static HPALETTE hPromptFriendsPalette;	// handle to palette for bgnd picture



BOOL CALLBACK dlgFuncPromptFriends(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) dlgFuncPromptFriends: message = %d\n",_FL, message));

	switch (message) {

	case WM_INITDIALOG:

		ScaleDialogTo96dpi(hDlg);

		hPromptFriendsDlg = hDlg;

		ShowWindow(GetDlgItem(hDlg, IDC_CONNECTION_PROGRESS), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_CONNECTION_STATUS), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_RECONNECT), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_WEB_PAGE), SW_HIDE);

        ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_1), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_2), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_3), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_4), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_5), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSSETENCE_6), SW_HIDE);



		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_1), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_2), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_3), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_4), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_5), SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_PROCESSMARK_6), SW_HIDE);



		

		return TRUE;

	case WM_DESTROY:

		hPromptFriendsDlg = NULL;

		break;

	case WM_PAINT:

		// If we've got a background image, paint it first.

		{

			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && hPromptFriendsBgnd) {

				if (hPromptFriendsPalette) {

					SelectPalette(hdc, hPromptFriendsPalette, FALSE);

					RealizePalette(hdc);

				}

				BlitBitmapToDC_nt(hPromptFriendsBgnd, hdc);

			}

			EndPaint(hDlg, &ps);

		}

		break;

	case WM_QUERYNEWPALETTE:

	case WM_PALETTECHANGED:	// palette has changed.

		//kp(("%s(%d) Table got WM_PALETTECHANGED\n", _FL));

	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us

		    HDC hdc = GetDC(hDlg);

		    HPALETTE holdpal = SelectPalette(hdc, hPromptFriendsPalette, FALSE);

		    int i = RealizePalette(hdc);   // Realize drawing palette.

			kp(("%s(%d) Palette changed = %d\n", _FL, i));

		    if (i) {	// Did the realization change?

		        InvalidateRect(hDlg, NULL, TRUE);    // Yes, so force a repaint.

		    }

		    SelectPalette(hdc, holdpal, TRUE);

		    RealizePalette(hdc);

		    ReleaseDC(hDlg, hdc);

		    return i;

	    }

		break;

	case WM_ERASEBKGND:

		if (IsIconic(hDlg) || !hPromptFriendsBgnd) {

			return FALSE;	// FALSE = we did NOT process this message.

		}

		return TRUE;	// TRUE = we DID process this message.

	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hPromptFriendsDlg);

		return TRUE;	// TRUE = we DID process this message.

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/07/28 - MB

//

// Prompt user to tell their friends

//

void PromptUserToTellTheirFriends(void)

{

	static int did_prompt = FALSE;

	

	if (did_prompt || iUserLoggingOffSystemFlag) {

		return;	// never do it twice.

	}

  #if ADMIN_CLIENT && 0

  	if (iAdminClientFlag) {

		return;	// never do it in admin mode

  	}

  #endif

	#define DAYS_BETWEEN_EXIT_SPLASH	6

	if (time(NULL) < Defaults.last_exit_splash_display_time + (DAYS_BETWEEN_EXIT_SPLASH*24*3600))

	{

		// It's too soon... don't display it again.

		//return;

	}

	Defaults.last_exit_splash_display_time = time(NULL);

	Defaults.changed_flag = TRUE;



	did_prompt = TRUE;



	// Load the background pic



	if (!hPromptFriendsBgnd ) {

		// Load the palette for the bgnd pic if necessary

		hPromptFriendsPalette = LoadPaletteIfNecessary(FindFile("src_exit.act"));
		hPromptFriendsBgnd = LoadJpegAsBitmap(FindFile("data\\media\\src_exit.jpg"), hPromptFriendsPalette);
	}



	//if (!hPromptFriendsBgnd) {

	//	return;	// if pic not loaded, don't bother displaying dialog box.

	//}

	CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONNECTING),

					  	NULL, dlgFuncPromptFriends);

	const exit_delay_time = 3000;

	if (hPromptFriendsDlg) {

		ShowWindow(hPromptFriendsDlg, SW_SHOW);	// make it visible.

		// Display it for 4 seconds

		DWORD start_ticks = GetTickCount();

		while (GetTickCount() - start_ticks < exit_delay_time) {

			DispatchMessagesForHwnd(hPromptFriendsDlg);

			Sleep(100);

		}

		DestroyWindow(hPromptFriendsDlg);

	}

}

