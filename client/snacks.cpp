//*********************************************************


//

// snacks.cpp : Bar snack related routines for the client tables

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"



#define SNACK_TABLE_LOCATIONS	5



#define TOTAL_SNACK_BITMAPS	(SNACK_TABLE_LOCATIONS*BAR_SNACK_COUNT)



static HWND hSnackMenuDlg;

static HBITMAP hSnackMenuBgnd;		// handle to background picture for dialog box.

static HPALETTE hSnackMenuPalette;	// handle to palette for bgnd picture

//static HFONT hSnackMenuButtonFont;

//static HFONT hSnackMenuButtonFontSmall;

static HWND hSnackMenuToolTips;

static struct ProximityHighlightingInfo SnackMenuHighlightList;



struct DlgToolTipText SnackMenuDlgToolTipText[] = {

  #if 0	//19991109MB: none are done yet... fill these in later.

	IDC_EXIT,			"Log off the Poker server and close this program",

	IDC_CASHIER,		"The SnackMenu handles transferring money into and out of your account",

	IDC_ACCOUNT_INFO,	"View and change your Account information such as name, address, etc.",

	IDC_GOTO_GAME,		"Go to a table to watch, then sit down to play.",

	IDC_HIDE_PLAY_MONEY,"Hides any tables which aren't using real money",

	IDC_JOIN_WAIT_LIST,	"Join or unjoin the waiting list for the highlighted table",

	IDC_GAME_TYPE_TAB,	"Select different game types",

  #endif

	0,0

};



static int SnackMenuControlBitmaps[][3] =  {

    IDCANCEL,		MISCBITMAP_SNACKMENU_CANCEL_R,		0,

	IDC_NOTHING,	MISCBITMAP_SNACKMENU_NOTHING_R,		0,

	IDC_NO_SMOKING,	MISCBITMAP_SNACKMENU_NOSMOKING_R,	0,

	IDC_MENU_ITEM1,	MISCBITMAP_SNACKMENU_CAKE,			BAR_SNACK_CAKE,

	IDC_MENU_ITEM2,	MISCBITMAP_SNACKMENU_MUFFIN,		BAR_SNACK_MUFFIN,

	IDC_MENU_ITEM3,	MISCBITMAP_SNACKMENU_DOUGHNUTS,		BAR_SNACK_DOUGHNUTS,		

	IDC_MENU_ITEM4,	MISCBITMAP_SNACKMENU_STRAWBERRIES,	BAR_SNACK_STRAWBERRIES,

	IDC_MENU_ITEM5,	MISCBITMAP_SNACKMENU_HOTDOG,		BAR_SNACK_HOTDOG,		

	IDC_MENU_ITEM6,	MISCBITMAP_SNACKMENU_HAMBURGER,		BAR_SNACK_HAMBURGER,		

	IDC_MENU_ITEM7,	MISCBITMAP_SNACKMENU_BEER_DRAFT,	BAR_SNACK_BEER_DRAFT,	

	IDC_MENU_ITEM8,	MISCBITMAP_SNACKMENU_BEER_DOMESTIC,	BAR_SNACK_BEER_DOMESTIC,

	IDC_MENU_ITEM9,	MISCBITMAP_SNACKMENU_BEER_IMPORTED,	BAR_SNACK_BEER_IMPORTED,

	IDC_MENU_ITEM10,MISCBITMAP_SNACKMENU_CHICHI,		BAR_SNACK_CHICHI,		

	IDC_MENU_ITEM11,MISCBITMAP_SNACKMENU_MARTINI,		BAR_SNACK_MARTINI,		

	IDC_MENU_ITEM12,MISCBITMAP_SNACKMENU_COSMO,			BAR_SNACK_COSMO,			

	IDC_MENU_ITEM13,MISCBITMAP_SNACKMENU_LIME_MARG,		BAR_SNACK_LIME_MARG,		

	IDC_MENU_ITEM14,MISCBITMAP_SNACKMENU_PINK_MARG,		BAR_SNACK_PINK_MARG,		

	IDC_MENU_ITEM15,MISCBITMAP_SNACKMENU_RUM_AND_COKE,	BAR_SNACK_RUM_AND_COKE,

	IDC_MENU_ITEM16,MISCBITMAP_SNACKMENU_REDWINE,		BAR_SNACK_REDWINE,		

	IDC_MENU_ITEM17,MISCBITMAP_SNACKMENU_CHAMPAGNE,		BAR_SNACK_CHAMPAGNE,		

	IDC_MENU_ITEM18,MISCBITMAP_SNACKMENU_BRANDY,		BAR_SNACK_BRANDY,		

	IDC_MENU_ITEM19,MISCBITMAP_SNACKMENU_IRISH_COFFEE,	BAR_SNACK_IRISH_COFFEE,

	IDC_MENU_ITEM20,MISCBITMAP_SNACKMENU_TEA,			BAR_SNACK_TEA,			

	IDC_MENU_ITEM21,MISCBITMAP_SNACKMENU_COFFEE,		BAR_SNACK_COFFEE,		

	IDC_MENU_ITEM22,MISCBITMAP_SNACKMENU_ICED_TEA,		BAR_SNACK_ICED_TEA,		

	IDC_MENU_ITEM23,MISCBITMAP_SNACKMENU_COKE,			BAR_SNACK_COKE,			

	IDC_MENU_ITEM24,MISCBITMAP_SNACKMENU_WATER,			BAR_SNACK_WATER,			

	IDC_MENU_ITEM25,MISCBITMAP_SNACKMENU_LEMONADE,		BAR_SNACK_LEMONADE,		

	IDC_MENU_ITEM26,MISCBITMAP_SNACKMENU_CIGAR,			BAR_SNACK_CIGAR,			

	IDC_MENU_ITEM27,MISCBITMAP_SNACKMENU_CIGARETTE,		BAR_SNACK_CIGARETTE,		

	-1,-1,-1

};



// The BAR_SNACK_* constants are defined in gamedata.h.  These are the

// only two current places that need to be modified to add new snacks.



static char *SnackFileNames[BAR_SNACK_COUNT] = {

	NULL,

	"Cake",

	"Muffins",

	"Dnuts",

	"StrawberryChocolate",

	"Hotdog",

	"Burger",

	"Pilsner",

	"beer",

	"GreenBeer",

	"ChiChi",

	"Martini",

	"Cosmopolitan",

	"Margarita",

	"MargaritaStrawberry",

	"RumAndCoke",

	"wineRed",

	"Champagne",

	"Brandy",

	"IrishCoffee",

	"Tea",

	"MugCoffee",

	"IceTea",

	"Coke",

	"Water",

	"Lemonade",

	"Cigar2",

	"cigarette",

};



HBITMAP SnackBitmaps[TOTAL_SNACK_BITMAPS];



static int SnackControlIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_SNACK_PLR1,

			IDC_SNACK_PLR2,

			IDC_SNACK_PLR3,

			IDC_SNACK_PLR4,

			IDC_SNACK_PLR5,

			IDC_SNACK_PLR6,

			IDC_SNACK_PLR7,

			IDC_SNACK_PLR8,

			IDC_SNACK_PLR9,

			IDC_SNACK_PLR10,

};



static int SnackOrientations_10[MAX_PLAYERS_PER_GAME] = {0,0,1,1,2,2,3,3,4,4};

static int SnackOrientations_8[MAX_PLAYERS_PER_GAME] = {0,0,1,1,3,3,4,4};



//*********************************************************

// 2000/03/15 - MB

//

// Load the snack bitmaps into memory.



void LoadBarSnackBitmaps(HPALETTE hpal)

{

	

    for (int i=BAR_SNACK_NONE+1 ; i<BAR_SNACK_COUNT ; i++) {

		for (int j=0 ; j<SNACK_TABLE_LOCATIONS ; j++) {

			int index = j*BAR_SNACK_COUNT + i;

			if (!SnackBitmaps[index]) {

				// This one needs loading...

				char fname[MAX_FNAME_LEN];

				zstruct(fname);

				if (j <= 2) {

					if (SnackFileNames[i]) {

						sprintf(fname, "bs%d_%s.bmp", j, SnackFileNames[i]);

						SnackBitmaps[index] = LoadBMPasBitmap(FindFile(fname), hpal);

					}

				} else {

					// Use a flipped version of another bitmap

					SnackBitmaps[index] = DuplicateHBitmapFlipped(SnackBitmaps[(4-j)*BAR_SNACK_COUNT+i]);

				}

				if (!SnackBitmaps[index]) {

					Error(ERR_ERROR, "%s(%d) Could not load snack #%d: '%s'", _FL, i, fname);

					iBitmapLoadingProblem = TRUE;

				}

			}

		}

	}

    

  }



//*********************************************************

// 2000/03/15 - MB

//

// Draw all the snacks at a table.

//

void DrawAllBarSnacks(struct TableInfo *t)

{

	int priority_table[MAX_PLAYERS_PER_GAME] =

		{1,2,3,4,5,5,4,3,2,1};

	int priority_table_7cs[MAX_PLAYERS_PER_GAME] =

		{1,2,3,4,4,3,2,1,0,0};



	int *pri_table = priority_table;

	int *orientations = SnackOrientations_10;

	if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

		pri_table = priority_table_7cs;

		orientations = SnackOrientations_8;

	}



  #if 0

	kp1(("%s(%d) Choosing random bar snacks to display\n",_FL));

	static DWORD old_SecondCounter;

	if (SecondCounter > old_SecondCounter+1) {

		old_SecondCounter = SecondCounter;

		for (int i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			t->GameCommonData.bar_snack[i] = (BYTE8)random(BAR_SNACK_COUNT);

		}

	}

  #endif



	for (int i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		if (t->GameCommonData.bar_snack[i] != t->old_bar_snack[i]) {

			// one of them changed... force a redraw of all the upper

			// layer windows (ovals, text, etc.)

			t->old_bar_snack[i] = t->GameCommonData.bar_snack[i];

			if (i==0 || i==1) {

				t->invalidate_top_right_text = TRUE;

			}

		}



		if (t->GameCommonData.bar_snack[i] != BAR_SNACK_NONE) {

			if (t->no_smoking_flag &&

				t->GameCommonData.bar_snack[i] >= BAR_SNACK_SMOKES_START &&

				t->GameCommonData.bar_snack[i] <= BAR_SNACK_SMOKES_END)

			{

				continue;	// don't draw any of the smokes

			}

			POINT pt;

			GetPictureCoordsOnTable(t, SnackControlIDs[i], &pt);

			// adjust coords to roughly center the bmp file

			pt.x -= 27;

			pt.y -= 27;

			int index = t->GameCommonData.bar_snack[i] +

						orientations[i]*BAR_SNACK_COUNT;

			pr(("%s(%d) Drawing snack %d ($%08lx) at %d,%d\n",

					_FL, index, SnackBitmaps[index], pt.x, pt.y));

			t->blit_queue.AddBitmap(SnackBitmaps[index], &pt, 150 + pri_table[i]);

		}

	}

}



#if 0	//20000616MB

//*********************************************************

// 2000/05/27 - MB

//

// A snack has been selected.  Close the menu and send

// the selection to the server.

//

static void SelectSnack(HWND hDlg, struct TableInfo *t, int snack_selected)

{

	DestroyWindow(hDlg);

	if (t) {

		t->bar_snack = snack_selected;

		t->no_smoking_flag = IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_SMOKING);

		if (t->no_smoking_flag != Defaults.iNoSmokingFlag) {

			Defaults.iNoSmokingFlag = t->no_smoking_flag;

			Defaults.changed_flag = TRUE;

		}

		struct MiscClientMessage mcm;

		zstruct(mcm);

		mcm.message_type = MISC_MESSAGE_CHOOSE_BAR_SNACK;

		mcm.table_serial_number = t->table_serial_number;

		mcm.misc_data_1 = t->bar_snack;

		//kp(("%s(%d) mcm.misc_data_1 = %d\n",_FL,mcm.misc_data_1));

		SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

	}

}

#endif



//*********************************************************

// 2000/05/26 - MB

//

// dialog callback function for the snack menu

//

BOOL CALLBACK dlgFuncSnackMenu(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	struct TableInfo *t = (struct TableInfo *)GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:

		// note: t (the table pointer) is not yet initialized properly

		hSnackMenuDlg = hDlg;

		ScaleDialogTo96dpi(hDlg);

		WinRestoreWindowPos(ProgramRegistryPrefix, "SnackMenu", hDlg, NULL, NULL, FALSE, TRUE);

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(hDlg, WMP_UPDATE_REALBANK_CHIPS, 0, 0);

		SendMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		zstruct(SnackMenuHighlightList);

		{

			int *p = &SnackMenuControlBitmaps[0][0];

			while (*p != -1) {

				AddProximityHighlight(&SnackMenuHighlightList, GetDlgItem(hDlg, *p));

				p += 3;

			}

		}

		// Make ourselves a topmost window

		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_ACTIVATE:

		if (LOWORD(wParam)!=WA_INACTIVE) {	// we got activated...

			// any time we get activated we should redraw the whole window to

			// make sure nothing gets left half-drawn due to bugs in the video

			// drivers or elsewhere that seem to forget about refreshing

			// certain areas of the screen.

			InvalidateRect(hDlg, NULL, FALSE);

		}

		return TRUE;	// message handled

	case WM_CLOSE:

		// Do the same thing as clicking 'cancel'

		DestroyWindow(hDlg);

		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDCANCEL:	// Close window?

			DestroyWindow(hDlg);

		    return TRUE;	// message was processed

	  #if 0	//20000616MB

		case IDC_NOTHING:

			SelectSnack(hDlg, t, BAR_SNACK_NONE);

		    return TRUE;	// message was processed

		case IDC_NO_SMOKING:	// invert the no smoking checkbox

			CheckDlgButton(hDlg, IDC_CHKBOX_NO_SMOKING,

					IsDlgButtonChecked(hDlg,IDC_CHKBOX_NO_SMOKING) ? BST_UNCHECKED : BST_CHECKED);

		    return TRUE;	// message was processed

	  #endif

		}

		// Search the table of snack controls...

		{

			int *p = SnackMenuControlBitmaps[0];

			while (*p != -1) {

				int id = LOWORD(wParam);

				if (id == p[0]) {

					// Found a match.... select this snack.

					if (t) {

						// If they selected no smoking, don't send to server.

						// Otherwise, send their selection to the server.

						if (id != IDC_NO_SMOKING) {

							t->bar_snack = p[2];	// fetch the related BAR_SNACK_* value

							struct MiscClientMessage mcm;

							zstruct(mcm);

							mcm.message_type = MISC_MESSAGE_CHOOSE_BAR_SNACK;

							mcm.table_serial_number = t->table_serial_number;

							mcm.misc_data_1 = t->bar_snack;

							//kp(("%s(%d) mcm.misc_data_1 = %d\n",_FL,mcm.misc_data_1));

							SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

						} else {

							CheckDlgButton(hDlg, IDC_CHKBOX_NO_SMOKING,

									IsDlgButtonChecked(hDlg,IDC_CHKBOX_NO_SMOKING) ? BST_UNCHECKED : BST_CHECKED);

						}

						t->no_smoking_flag = IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_SMOKING);

						if (t->no_smoking_flag != Defaults.iNoSmokingFlag) {

							Defaults.iNoSmokingFlag = t->no_smoking_flag;

							Defaults.changed_flag = TRUE;

						}

					}

					DestroyWindow(hDlg);

				    return TRUE;	// message was processed

				}

				p += 3;

			}

		}

	    return TRUE;	// message was processed



	case WM_CTLCOLORBTN:

		// This message is sent to all buttons before drawing.  Return a 

		// null background brush so that we don't get grey drawn just before

		// our bitmap is going to be drawn.

		//kp(("%s(%d) Got WM_CTLCOLORBTN\n",_FL));

		return ((int)GetStockObject(NULL_BRUSH));



	case WM_CTLCOLOREDIT:

	case WM_CTLCOLORSTATIC:

		{

		  #if 0	//20000526MB

			HWND hwnd = (HWND)lParam;	// hwnd of window getting drawn

			if (hwnd==GetDlgItem(hDlg, IDC_REAL_TOTAL) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_BANK) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_PENDING) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_TABLE) ||

				hwnd==GetDlgItem(hDlg, IDC_SNACKMENU_PLAYERINFO) ||

				hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT)

			) {	// found a match!

				// Drawing a text item... draw bitmapped background

				// and change the text color.

				RECT r;

				GetWindowRect(hwnd, &r);

				ScreenToClient(GetParent(hwnd), &r);

				// Blit this rect from our background into the display area.

				POINT pt;

				zstruct(pt);

				HDC hdc = (HDC)wParam;

				BlitBitmapRectToDC(hSnackMenuBgnd, hdc, &pt, &r);

				if (hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT) && hSnackMenuButtonFontSmall) {

					SelectObject(hdc, hSnackMenuButtonFontSmall);

				} else if (hSnackMenuButtonFont) {

					SelectObject(hdc, hSnackMenuButtonFont);

				}

				if (hwnd==GetDlgItem(hDlg, IDC_SNACKMENU_PLAYERINFO) ||

					hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT))

				{

				  #if 1	//19991111MB

					SetTextColor(hdc, RGB(255,235,110));// draw text in brighter gold

				  #else

					SetTextColor(hdc, RGB(255,228,82));	// draw text in gold

				  #endif

				} else {

					SetTextColor(hdc, RGB(0,0,0));		// draw text in black

				}

				SetBkMode(hdc, TRANSPARENT);

				return ((int)GetStockObject(NULL_BRUSH));

			}

		  #endif

		}

		break;

	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "SnackMenu", hDlg, NULL);

		RemoveKeyboardTranslateHwnd(hDlg);

		CloseToolTipWindow(hSnackMenuToolTips);	// Close our tooltip window

		hSnackMenuDlg = NULL;

		return TRUE;	// TRUE = we DID process this message.

	case WM_DRAWITEM:

		// Owner draw control... draw it.

		{

			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			int bitmap_index = -1;

			if (dis->hwndItem==SnackMenuHighlightList.hCurrentHighlight) {

				// The mouse has highlighted one of our buttons... figure out

				// which one and select the appropriate bitmap.

				int *p = &SnackMenuControlBitmaps[0][0];

				while (*p != -1) {

					if (*p == (int)dis->CtlID) {

						bitmap_index = p[1];

						break;

					}

					p += 3;

				}

			}

			DrawButtonItemWithBitmap(hSnackMenuBgnd, dis, bitmap_index);

		}

		return TRUE;	// TRUE = we did process this message.

	case WM_ERASEBKGND:

		if (IsIconic(hDlg) || !hSnackMenuBgnd) {

			return FALSE;	// FALSE = we did NOT process this message.

		}

		return TRUE;	// TRUE = we DID process this message.

	case WM_MOUSEMOVE:

		UpdateMouseProximityHighlighting(&SnackMenuHighlightList, hDlg, MAKEPOINTS(lParam));

		return TRUE;	// TRUE = we did process this message.

	case WM_PAINT:

		// If we've got a background image, paint it first.

		{

			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && hSnackMenuBgnd) {

				if (hSnackMenuPalette) {

					SelectPalette(hdc, hSnackMenuPalette, FALSE);

					RealizePalette(hdc);

				}

				BlitBitmapToDC_nt(hSnackMenuBgnd, hdc);

			}

			EndPaint(hDlg, &ps);

		}

		break;

	case WM_QUERYNEWPALETTE:

	case WM_PALETTECHANGED:	// palette has changed.

		kp(("%s(%d) Snack menu got WM_PALETTECHANGED\n", _FL));

	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us

		    HDC hdc = GetDC(hDlg);

		    HPALETTE holdpal = SelectPalette(hdc, hSnackMenuPalette, FALSE);

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



	}

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 2000/05/26 - MB

//

// Open and/or show the snack menu screen.

// This function must be called from the message thread.

//

void OpenSnackMenuWindow(struct TableInfo *t)

{

	if (hSnackMenuDlg) {		// already open, just top it.

		ReallySetForegroundWindow(hSnackMenuDlg);

		return;

	}



	// Load the background pic

	if (!hSnackMenuBgnd) {

		// Load the palette for the bgnd pic if necessary

		hSnackMenuPalette = LoadPaletteIfNecessary(FindFile("snackmenu.act"));

		//kp(("%s(%d) hSnackMenuPalette = $%08lx\n", _FL, hSnackMenuPalette));

		// Change the JPEG file location

		//	hSnackMenuBgnd = LoadJpegAsBitmap(FindFile("snackmenu.jpg"), hSnackMenuPalette);

			hSnackMenuBgnd = LoadJpegAsBitmap(FindMediaFile("media\\src_snackmenu.jpg"), hSnackMenuPalette);
	}



	LoadSnackMenuBitmaps(hSnackMenuPalette);



	// Display the dialog box

	if (!hSnackMenuDlg) {

	  #if 0		// Should we have a parent?

		CreateDialog(hInst, MAKEINTRESOURCE(IDD_SNACKMENU),

							hCardRoomDlg, dlgFuncSnackMenu);

	  #else

		CreateDialog(hInst, MAKEINTRESOURCE(IDD_SNACKMENU),

						  	NULL, dlgFuncSnackMenu);

	  #endif

		if (hSnackMenuDlg) {

			// make sure it's on screen in the current screen resolution

			WinPosWindowOnScreen(hSnackMenuDlg);

			// Create a tooltip control/window for use within this dialog box

			// and add all our tool tips to it.

			hSnackMenuToolTips = OpenToolTipWindow(hSnackMenuDlg, SnackMenuDlgToolTipText);

			CheckDlgButton(hSnackMenuDlg, IDC_CHKBOX_NO_SMOKING, t->no_smoking_flag);

			SetWindowLong(hSnackMenuDlg, GWL_USERDATA, (long)t);

			ShowWindow(hSnackMenuDlg, SW_SHOW);

		} else {

			Error(ERR_ERROR, "%s(%d) SnackMenu dialog failed to open.  GetLastError() = %d\n", _FL, GetLastError());

		}

	}

}





//*********************************************************

// 2000/05/27 - MB

//

// Close a snack menu if it's open

//

void CloseSnackMenuWindow(void)

{

	if (hSnackMenuDlg) {

		PostMessage(hSnackMenuDlg, WM_CLOSE, 0, 0);

	}

}

