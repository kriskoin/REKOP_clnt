//****************************************************************

//

// News.cpp : download and display news for client end

//

//****************************************************************

#define _MT

#define DISP 0





#include "stdafx.h"

#include <process.h>

#include "resource.h"



#define NEWS_BASE_URL	BASE_URL2

#define NEWS_FNAME   "message.txt" 



static int iNewsThreadRunning = FALSE;

#define MAX_MSG_LEN		1000

char msg[MAX_MSG_LEN];

extern HBITMAP original_hbm;			

BOOL CALLBACK dlgNews(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//*********************************************************

// 1999/07/27 - MB

//

// Remove all control characters from a string

//

void StripControlChars(char *str)

{

	if (str) {

		char *dest = str;

		while (*str) {

			if ((BYTE)*str >= ' ') {	// space or greater?

				*dest++ = *str++;

			} else {

				str++;

			}

		}

		*dest = 0;

	}

}



//*********************************************************

// 1999/07/27 - MB

//

// Display a news file stored locally

//

void DisplayNewsFile(char *fname)

{

	// First, read it in and parse it.

	FILE *fd = fopen(fname, "rb");

	//kp(("%s(%d) fd = $%08lx.  fname='%s'\n", _FL, fd, fname));

	if (fd) {

		// Format:

		//	1st line contains serial number

		//	2nd contains title

		//	remainder contain text.

		int serial_number = 0;

		#define MAX_LINE_LEN	300

		char str[MAX_LINE_LEN];

		zstruct(str);

		char *result = fgets(str, MAX_LINE_LEN, fd);

		if (!result) {

			goto closefile;	// file is empty.

		}

		StripControlChars(result);

		// Make sure the first line only contains digits... that way we

		// won't display anything when a 404 error is reported by a proxy server.

		{

			char *p = str;

			while (*p) {

				if (!isdigit(*p)) {

					kp(("%s(%d) news file appears to have crap in it.\n",_FL));

					goto closefile;	// assume file is full of crap.

				}

				p++;

			}

		}

		// It appears to be a serial number.

		serial_number = atoi(result);

		pr(("%s(%d) Serial number = %d\n", _FL, serial_number));

		if (serial_number != Defaults.news_serial_number)

		{

			// It's a new news item.  Display it

			Defaults.news_serial_number = serial_number;

			Defaults.changed_flag = TRUE;

			result = fgets(str, MAX_LINE_LEN, fd);

			#define MAX_TITLE_LEN	100

			char title[MAX_TITLE_LEN];

			title[0] = 0;

			if (result) {

				StripControlChars(str);

				strnncpy(title, str, MAX_TITLE_LEN);

			}



			// Concatenate all remaining lines into a single string.

			zstruct(msg);

			int msg_len = 0;

			do {

				zstruct(str);

				result = fgets(str, MAX_LINE_LEN, fd);

				//kp(("%s(%d) result = %d\n", _FL, result));

				if (result) {

					//StripControlChars(str);

					strnncpy(msg+msg_len, str, MAX_MSG_LEN-msg_len);

					msg_len = strlen(msg);

					//kp(("%s(%d) msg_len = %d\n", _FL, msg_len));

				}

			} while(result && msg_len<MAX_MSG_LEN-1);

		  #if 0	//20000531MB

			kp(("%s(%d) Done... final strlen(msg) = %d\n", _FL, strlen(msg)));

			kp(("%s(%d) hCardRoomDlg = $%08lx, title = '%s'\n",  _FL, hCardRoomDlg, title));

			kp(("%s(%d) start of msg = '%-50.50s'\n",  _FL, msg));

			khexdump(msg, strlen(msg), 16, 1);

			MessageBox(hCardRoomDlg, msg, title,

					MB_ICONINFORMATION |

					MB_OK | MB_SETFOREGROUND | MB_TOPMOST);



		  #endif

		//Allen 2002-4-2

			//20000531HK -- this works...

			//News Display

			/*

			MessageBox(NULL, msg, title,

					MB_ICONINFORMATION |

					MB_OK | MB_SETFOREGROUND | MB_TOPMOST);

			

			*/

		

			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWS), HWND_DESKTOP,

						(DLGPROC)dlgNews, NULL);

		//Allen 2002-4-2

			//kp(("%s(%d) back from MessageBox().\n", _FL));

		}



closefile:

		fclose(fd);

	} else {

		Error(ERR_ERROR, "%s(%d) Could not read news file '%s'", _FL, fname);

	}

}



//*********************************************************

// 1999/05/23 - MB

//

// Entry point for the news thread.

//

void _cdecl NewsThreadEntryPoint(void *args)
//void _cdecl NewsThreadEntryPoint(void)

{

  #if INCL_STACK_CRAWL

	volatile int top_of_stack_signature = TOP_OF_STACK_SIGNATURE;	// for stack crawl

  #endif

	iNewsThreadRunning = TRUE;

	//kp(("%s(%d) News thread has now started.\n",_FL));



  #if 0	//19990727MB

	Defaults.news_serial_number = -1;

	DisplayNewsFile(NEWS_FNAME);

  #else

	HINTERNET inet_hndl = InternetOpen(szAppName,

			INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);


	if (!inet_hndl) {

		Error(ERR_ERROR, "%s(%d) InternetOpen() failed.  Error = %d\n",_FL,GetLastError());

		return;

	}


	ErrorType err = WriteFileFromUrl(inet_hndl, NEWS_BASE_URL NEWS_FNAME, NEWS_FNAME);

	if (err==ERR_NONE) {

		// We've got the file... do something with it.

		DisplayNewsFile(NEWS_FNAME);

	} else {

		//kp(("%s(%d) Unable to retrieve '%s'\n", _FL, NEWS_BASE_URL NEWS_FNAME));

		Error(ERR_ERROR, "%s(%d) Unable to retrieve '%s'", _FL, NEWS_BASE_URL NEWS_FNAME);

	}



	InternetCloseHandle(inet_hndl);

  #endif

	//kp(("%s(%d) News thread is now exiting.\n",_FL));

	iNewsThreadRunning = FALSE;

	NOTUSED(args);

  #if INCL_STACK_CRAWL

	NOTUSED(top_of_stack_signature);

  #endif

}



//*********************************************************

// 1999/07/27 - MB

//

// Retrieve and display news from our web site.  The work is done

// by another thread, so this function will return immediately.

//

void DisplayNewsFromWebSite(void)

{

	// Create a separate thread to grab the news file and deal with it...
	
	if (!iNewsThreadRunning) {
    	_beginthread(NewsThreadEntryPoint, 0, 0);
	}

}





/**********************************************************************************

 Function CALLBACK dlgAboutFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK1999/06/24

 Purpose: handler for About box

***********************************************************************************/

BOOL CALLBACK dlgNews(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[120];

	zstruct(str);

	

	switch (message) {

	case WM_INITDIALOG:

		// window title

		ScaleDialogTo96dpi(hDlg);

		AddKeyboardTranslateHwnd(hDlg);

			// hdcStatic = (HDC) wParam;   // handle to display context 

			// hwndStatic = (HWND) lParam; // handle to static control 

			// chat_hwnd = GetDlgItem(Table[table_index].hwnd, IDC_CHAT_BOX);

		{

		char blank_message[550];

		int i=strlen(msg);

		char *p1,*p2;

		p1=msg;

		p2=blank_message;

		*p2=' ';

		p2++;

		for (i=0; i<550; i++)

		{

			if (*p1 =='\n') {

				*p2= *p1 ; 			

				p2++;

				p1++;

				*p2 = ' ';

				p2++;

                

			} else {

			    *p2= *p1 ; 			

				p2++;

				p1++;

			}

		}



     

		SetDlgItemText(hDlg, IDC_STATIC_NEWS, blank_message);

		ShowWindowIfNecessary(GetDlgItem(hDlg, IDOK), SW_SHOW);

		ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_NEWS_DETAILS), SW_SHOW);

		}	

		// PPlaySound(SOUND_NEWS);



		return TRUE;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:

			//PPlaySound(SOUND_NO_SOUND);

			EndDialog(hDlg, IDOK);

			return TRUE;

		case IDC_NEWS_DETAILS:

			EndDialog(hDlg, IDCANCEL);
			LaunchInternetBrowser("http://www.e-mediasoftware.com/whatsnew/whatsnew.php");

			return TRUE;

		}

		break;



	case WM_PAINT:

		// If we've got a background image, paint it first.

		{

		

			original_hbm = (HBITMAP)LoadImage(NULL,

			FindFile("data\\media\\src_news.bmp"), IMAGE_BITMAP, 0, 0,

			LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);



			if (!original_hbm) {

				//kp(("%s(%d) LoadBMPasBitmap: Failed to load %s using LoadImage().  Error = %d\n", _FL, fname, GetLastError()));

				return NULL;

			}





			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && original_hbm) {

				//if (hPromptFriendsPalette) {

					//SelectPalette(hdc, original_hbm, FALSE);

					//RealizePalette(hdc);

				//}

			{

			 HWND hwnd = GetDlgItem(hDlg, IDC_STATIC_NEWS);

			 static HFONT font = CreateFont(

					14,	//  int nHeight,             // logical height of font

					0,	//  int nWidth,              // logical average character width

					0,	//  int nEscapement,         // angle of escapement

					0,	//  int nOrientation,        // base-line orientation angle

					FW_DONTCARE,		//  int fnWeight,            // font weight

					0,	//  DWORD fdwItalic,         // italic attribute flag

					0,	//  DWORD fdwUnderline,      // underline attribute flag

					0,	//  DWORD fdwStrikeOut,      // strikeout attribute flag

					ANSI_CHARSET,		//  DWORD fdwCharSet,        // character set identifier

					OUT_DEFAULT_PRECIS,	//  DWORD fdwOutputPrecision,  // output precision

					CLIP_DEFAULT_PRECIS,//  DWORD fdwClipPrecision,  // clipping precision

					DEFAULT_QUALITY,	//  DWORD fdwQuality,        // output quality

					DEFAULT_PITCH|FF_MODERN,	//  DWORD fdwPitchAndFamily,  // pitch and family

					"Arial" //  LPCTSTR lpszFace         // pointer to typeface name string

				);

				

				if (font) {

					SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);

				}

			    }

				BlitBitmapToDC_nt(original_hbm, hdc);

			}

			EndPaint(hDlg, &ps);

		}

		break;

    case WM_CTLCOLORSTATIC:

		{

		

    		 HDC hdc = (HDC)wParam;

			 SetTextColor(hdc, RGB(225,225,225)/*RGB(225,220,140)*/);

			 SetBkMode(hdc, TRANSPARENT);

			 return ((int)GetStockObject(NULL_BRUSH));

		}





	 case WM_DRAWITEM:

		// Owner draw control... draw it.

		{

			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			int bitmap_index = 0;

			switch (dis->CtlID) {

				

			case IDC_NEWS_DETAILS:

				bitmap_index = MISCBITMAP_NEWS_DETAILS;

				break;

			case IDOK:

				bitmap_index = MISCBITMAP_NEWS_BUTTON;

				break;			

			}

			DrawButtonItemWithBitmap(MiscBitmaps[bitmap_index], dis, bitmap_index);

		}



		return TRUE;	//    

	

		/*

	case WM_CTLCOLORSTATIC:

		{

			// hdcStatic = (HDC) wParam;   // handle to display context 

			// hwndStatic = (HWND) lParam; // handle to static control 

			// chat_hwnd = GetDlgItem(Table[table_index].hwnd, IDC_CHAT_BOX);

			HWND hwnd = (HWND)lParam;



			// First, use LoadImage() to load it in as 24bpp.

			HBITMAP original_hbm = (HBITMAP)LoadImage(NULL,

					FindFile("data\\media\\src_news.bmp"), IMAGE_BITMAP, 0, 0,

					LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

			if (!original_hbm) {

				//kp(("%s(%d) LoadBMPasBitmap: Failed to load %s using LoadImage().  Error = %d\n", _FL, fname, GetLastError()));

				return NULL;

			}



			if (original_hbm ) {

				//kp(("%s(%d) Got WM_CTLCOLORSTATIC for IDC_TABLE_INFO\n",_FL));

				// Drawing the table info item... draw bitmapped background

				// and change the text color.

				RECT r;

				GetWindowRect(hwnd, &r);

				ScreenToClient(GetParent(hwnd), &r);

				// Blit this rect from our background into the display area.

				POINT pt;

				zstruct(pt);

				HDC hdc = (HDC)wParam;

				BlitBitmapRectToDC(original_hbm, hdc, &pt, &r);

				SetTextColor(hdc, RGB(255,255,255));

				SetBkMode(hdc, TRANSPARENT);

				return ((int)GetStockObject(NULL_BRUSH));

			}

		}

		break;

    */

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;

	}

	NOTUSED(lParam);

    return FALSE;

}

