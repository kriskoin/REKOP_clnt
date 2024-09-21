//****************************************************************

//

// MiscDraw.cpp : Misc. drawing related routines for the client tables

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"

/*

char *MiscBitmapFilenames[MISCBITMAP_COUNT] =  {

	"PlayerID.bmp",

	"PlayerTurn.bmp",

	"SitDown.bmp",

	"LeaveTable.bmp",

	"CardRoom.bmp",

	"chiptray.bmp",

	"chiptray-rm.bmp",

	"menuicon.bmp",

	"menuicon_r.bmp",

	"Play_Button.bmp",

	"Play_Button_d.bmp",

	"Play_Button_b.bmp",



	"TablesButUP.bmp",

	"TablesButRoll.bmp",

	"ListButUP.bmp",

	"ListButRoll.bmp",

	"CashButUP.bmp",

	"CashButRoll.bmp",



	"cshr-buy.bmp",

	"cshr-buy2.bmp",

	"cshr-cashout.bmp",

	"cshr-cashout2.bmp",

	"cshr-history.bmp",

	"cshr-history2.bmp",

	"cshr-leave.bmp",

	"cshr-leave2.bmp",

	"cshr-setup.bmp",

	"cshr-setup2.bmp",



	"menu-cancel_r.bmp",

	"menu-nosmoking_r.bmp",

	"menu-nothing_r.bmp",



	"menu-cake.bmp",		// MISCBITMAP_SNACKMENU_CAKE,

	"menu-muffin.bmp",		// MISCBITMAP_SNACKMENU_MUFFIN,

	"menu-doughnuts.bmp",	// MISCBITMAP_SNACKMENU_DOUGHNUTS,

	"menu-strawberries.bmp",// MISCBITMAP_SNACKMENU_STRAWBERRIES,

	"menu-hotdog.bmp",		// MISCBITMAP_SNACKMENU_HOTDOG,

	"menu-hamburger.bmp",	// MISCBITMAP_SNACKMENU_HAMBURGER,

	"menu-beer-draft.bmp",	// MISCBITMAP_SNACKMENU_BEER_DRAFT,

	"menu-beer-domestic.bmp",//MISCBITMAP_SNACKMENU_BEER_DOMESTIC,

	"menu-beer-imported.bmp",//MISCBITMAP_SNACKMENU_BEER_IMPORTED,

	"menu-chichi.bmp",		// MISCBITMAP_SNACKMENU_CHICHI,

	"menu-martini.bmp",		// MISCBITMAP_SNACKMENU_MARTINI,

	"menu-cosmo.bmp",		// MISCBITMAP_SNACKMENU_COSMO,

	"menu-lime-marg.bmp",	// MISCBITMAP_SNACKMENU_LIME_MARG,

	"menu-pink-marg.bmp",	// MISCBITMAP_SNACKMENU_PINK_MARG,

	"menu-rum_and_coke.bmp",// MISCBITMAP_SNACKMENU_RUM_AND_COKE,

	"menu-redwine.bmp",		// MISCBITMAP_SNACKMENU_REDWINE,

	"menu-champagne.bmp",	// MISCBITMAP_SNACKMENU_CHAMPAGNE,

	"menu-brandy.bmp",		// MISCBITMAP_SNACKMENU_BRANDY,

	"menu-irish_coffee.bmp",// MISCBITMAP_SNACKMENU_IRISH_COFFEE,

	"menu-tea.bmp",			// MISCBITMAP_SNACKMENU_TEA,

	"menu-coffee.bmp",		// MISCBITMAP_SNACKMENU_COFFEE,

	"menu-iced_tea.bmp",	// MISCBITMAP_SNACKMENU_ICED_TEA,

	"menu-coke.bmp",		// MISCBITMAP_SNACKMENU_COKE,

	"menu-water.bmp",		// MISCBITMAP_SNACKMENU_WATER,

	"menu-lemonade.bmp",	// MISCBITMAP_SNACKMENU_LEMONADE,

	"menu-cigar.bmp",		// MISCBITMAP_SNACKMENU_CIGAR,

	"menu-cigarette.bmp",	// MISCBITMAP_SNACKMENU_CIGARETTE,

	



};



*/





char *MiscBitmapFilenames[MISCBITMAP_COUNT] =  {

	"Media\\btn_ply_id.bmp",

	"Media\\btn_ply_turn.bmp",

	"Media\\btn_sitdown.bmp",

	"Media\\btn_leavetable.bmp",

	"Media\\btn_lobby.bmp",

	"Media\\chp_trayon.bmp",

	"Media\\chp_trayon2.bmp",

	// "menuicon.bmp",

	// "menuicon_r.bmp",

	"Media\\chp_trayon2.bmp",

	"Media\\chp_trayon2.bmp",

	"Media\\chp_trayon2.bmp",

	"Media\\chp_trayon2.bmp",

	// "Media\\btn_ply_btn.bmp",

	// "Media\\btn_ply_btn_d.bmp",
    
	"Media\\btn_ply_btn_b.bmp",//#11

	"Media\\btn_tbl_btn_up.bmp",

	"Media\\btn_tbl_btn_roll.bmp",
	//cristian 07-22-2003
/*	"Media\\emedialogoup.bmp",//#14

	"Media\\emedialogoroll.bmp",//#15  >>>>>>>ojo reorder  */

	"Media\\helpup.bmp",//#14

	"Media\\helpover.bmp",//#15  >>>>>>>ojo reorder  */
	
	//end cristian 07-22-2003

	"Media\\btn_lst_btn_up.bmp",

	"Media\\btn_lst_btn_roll.bmp",

	//rgong 04/04/2002 - 2 

	"Media\\btn_news.bmp",//16

	"Media\\btn_news_details.bmp",

	//rgong

	"Media\\btn_csh_btn_up.bmp",

	"Media\\btn_csh_btn_roll.bmp",//#19

	"Media\\btn_csh_buy.bmp",

	"Media\\btn_csh_buy2.bmp",//21

	"Media\\btn_csh_cashout.bmp",

	"Media\\btn_csh_cashout2.bmp",

	//rgong 04/04/2002

	"Media\\red.bmp",

	"Media\\red_on.bmp",

	"Media\\green.bmp",

	"Media\\green_on.bmp",//#27

	//end rgong

	"Media\\btn_csh_history.bmp",

	"Media\\btn_csh_history2.bmp",

	"Media\\btn_csh_leave.bmp",

	"Media\\btn_csh_leave2.bmp",

	"Media\\btn_csh_paypal.bmp",//32

	"Media\\btn_csh_bank_draft.bmp",

	"Media\\btn_csh_western_union.bmp",

	"Media\\btn_csh_visa_mastercard.bmp",
	 

	//rgong

	"Media\\btn_csh_firepay.bmp",//36

	"Media\\visa.bmp",

	"Media\\mastercard.bmp",

	"Media\\about_paypal.bmp",

	"Media\\about_firepay.bmp",//40

	"Media\\sign_firepay.bmp",

	//end rgong

	"Media\\btn_csh_setup.bmp",

	"Media\\btn_csh_setup2.bmp",  

};







	









HBITMAP MiscBitmaps[MISCBITMAP_COUNT];



//*********************************************************

//
//

// Load the misc bitmaps into memory.

//

void LoadMiscBitmaps(int start_bitmap_index, int end_bitmap_index, HPALETTE hpal)

{

	// Load the individual cards...

	//kp(("%s(%d) Loading bitmaps %d to %d\n",_FL, start_bitmap_index, end_bitmap_index));

	for (int i=start_bitmap_index ; i<=end_bitmap_index ; i++) {

		if (!MiscBitmaps[i]) {

			// This one needs loading.

            
    
			MiscBitmaps[i] = LoadBMPasBitmap(FindMediaFile(MiscBitmapFilenames[i]), hpal);
            //MessageBox(NULL,FindMediaFile(MiscBitmapFilenames[i]),"TEST",MB_OK);
			if (!MiscBitmaps[i]) {

				Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, MiscBitmapFilenames[i]);

				iBitmapLoadingProblem = TRUE;

			}

			//kp(("%s(%d) MiscBitmap[%d] = $%08lx\n", _FL, MiscBitmaps[i]));

		}

	}

}



void LoadLobbyBitmaps(HPALETTE hpal)

{

//	LoadMiscBitmaps(MISCBITMAP_GOTO_TABLE_UP, MISCBITMAP_LINK_LOBBY_HOVER, hpal);
	LoadMiscBitmaps(MISCBITMAP_GOTO_TABLE_UP, MISCBITMAP_CASHIER_ROLL, hpal);

//	LoadMiscBitmaps(MISCBITMAP_WAITLIST_UP,MISCBITMAP_WAITLIST_ROLL, hpal);
//LoadMiscBitmaps(MISCBITMAP_CASHIER_UP,MISCBITMAP_CASHIER_ROLL, hpal);

}

void LoadTableBitmaps(HPALETTE hpal)

{

	LoadMiscBitmaps(MISCBITMAP_PLAYER_ID, MISCBITMAP_PLAY_BUTTON_B, hpal);

}

void LoadCashierBitmaps(HPALETTE hpal)

{

	LoadMiscBitmaps(MISCBITMAP_CASHIER_BUY_UP, MISCBITMAP_CASHIER_SETUP_ROLL, hpal);

}

void LoadSnackMenuBitmaps(HPALETTE hpal)

{

	LoadMiscBitmaps(MISCBITMAP_SNACKMENU_START, MISCBITMAP_SNACKMENU_END, hpal);

}



//*********************************************************

//
//

// Draw a button control that had "owner draw" and "transparent" set

// in the resource file.  Use the specified bitmap image index (see MISCBITMAP_*)

//

void DrawButtonItemWithBitmap(struct TableInfo *t, LPDRAWITEMSTRUCT dis, int bitmap_index)

{

	DrawButtonItemWithBitmap(t, dis, bitmap_index, NULL);

}	

void DrawButtonItemWithBitmap(HBITMAP bgnd_bitmap, LPDRAWITEMSTRUCT dis, int bitmap_index)

{

	DrawButtonItemWithBitmap(NULL, dis, bitmap_index, bgnd_bitmap);

}	

void DrawButtonItemWithBitmap(struct TableInfo *t, LPDRAWITEMSTRUCT dis, int bitmap_index, HBITMAP bgnd_bitmap)

{

	// First, grab a piece of the background image to fill the

	// entire rectangle for this control...

	POINT dest_pt = *(LPPOINT)&dis->rcItem;

	RECT src_rect;

	zstruct(src_rect);

	GetWindowRect(dis->hwndItem, &src_rect);

	//kp(("%s(%d) Item %d: Src_rect = %d,%d,%d,%d\n", _FL, dis->itemID, src_rect.left, src_rect.top, src_rect.right, src_rect.bottom));

	ScreenToClient(GetParent(dis->hwndItem), &src_rect);

	//kp(("%s(%d) Item %d: Src_rect = %d,%d,%d,%d (after ScreenToClient())\n", _FL, dis->itemID, src_rect.left, src_rect.top, src_rect.right, src_rect.bottom));

	if (t && t->hbm_off_screen) {	// drawing to a table?

		EnterCriticalSection(&t->draw_crit_sec);

		BlitBitmapRectToDC(t->hbm_off_screen, dis->hDC, &dest_pt, &src_rect);

		LeaveCriticalSection(&t->draw_crit_sec);

	} else if (bgnd_bitmap) {

	//	BlitBitmapRectToDC(bgnd_bitmap, dis->hDC, &dest_pt, &src_rect);

	} 

	//rgong 04/04/2002 - 2

	else if (original_hbm){

     	BlitBitmapRectToDC(bgnd_bitmap, dis->hDC, &dest_pt, &src_rect);

	} 

	//end rgong

	else if (hCardRoomBgnd) {

		// No table... must be cardroom

		BlitBitmapRectToDC(hCardRoomBgnd, dis->hDC, &dest_pt, &src_rect);

	}



	if (bitmap_index >= 0) {

		// Now blit the control bitmap itself into place...

		if (dis->itemState & ODS_SELECTED) {

			dest_pt.x += 1;

			dest_pt.y += 1;

		}

		if (MiscBitmaps[bitmap_index]) {

			BlitBitmapToDC(MiscBitmaps[bitmap_index], dis->hDC, &dest_pt);

		}

	}

}



//*********************************************************

//
//

// Draw a button control that had "owner draw" set

// in the resource file.  Use the specified bitmap image

// index (see MISCBITMAP_*) and also draw the text.

//

void DrawButtonItemWithBitmapAndText(struct TableInfo *t, LPDRAWITEMSTRUCT dis, int bitmap_index, HFONT hfont)

{

	static COLORREF color_array[4] = {

		//	RGB(0,0,0),			// normal text
           	RGB(255,255,255),			// normal text white
		  #if 0	//19990928MB

			RGB(173,143,67),	// normal shadow

			RGB(148,122,22),	// disabled text

			RGB(170,141,25)		// disabled shadow

		  #else
            //RGB(255,249,177),
			RGB(0,0,0),	// normal shadow

			RGB(0,0,0),	// disabled text

		//	RGB(250,220,91)		// disabled shadow
            RGB(0,0,0)
		  #endif

	};

	DrawButtonItemWithBitmapAndText(t, dis, bitmap_index, hfont, color_array);

}



void DrawButtonItemWithBitmapAndText(struct TableInfo *t, LPDRAWITEMSTRUCT dis, int bitmap_index, HFONT hfont, COLORREF *color_array)

{

	HFONT font_array[2];

	font_array[0] = hfont;

	font_array[1] = NULL;

	DrawButtonItemWithBitmapAndText(t, dis, bitmap_index, font_array, color_array);

}



void DrawButtonItemWithBitmapAndText(struct TableInfo *t, LPDRAWITEMSTRUCT dis, int bitmap_index, HFONT *font_array, COLORREF *color_array)

{

	int w = dis->rcItem.left + dis->rcItem.right;

	int h = dis->rcItem.top  + dis->rcItem.bottom;

	// Create an off-screen DC to draw into so that it doesn't flicker.

	HDC hdc     = CreateCompatibleDC(dis->hDC);

	HBITMAP hbm = CreateBitmap(w, h, GetDeviceCaps(hdc, PLANES), GetDeviceCaps(hdc, BITSPIXEL), NULL);

	HBITMAP old_hbm = (HBITMAP)SelectObject(hdc, hbm);

	SetMapMode(hdc, GetMapMode(dis->hDC));

	//kp(("%s(%d) hdc = $%08lx, hbm = $%08lx\n", _FL, hdc, hbm));

  #if 0

	BITMAP bm;

	zstruct(bm);

	GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);

	kp(("%s(%d) temp off-screen hbitmap size = %dx%dx%d\n", _FL, bm.bmWidth,bm.bmHeight,bm.bmBitsPixel));

  #endif



	DRAWITEMSTRUCT dis2 = *dis;

	dis2.hDC = hdc;		// do drawing to this HDC instead.

	dis2.rcItem.left = 0;

	dis2.rcItem.top = 0;

	dis2.rcItem.right = w;

	dis2.rcItem.bottom = h;



	DrawButtonItemWithBitmap(t, &dis2, bitmap_index);

	// Now draw the text on the button

	char str[100];

	GetWindowText(dis->hwndItem, str, 100);

	//kp(("%s(%d) hwnd = $%08lx, text = '%s'\n", _FL, dis->hwndItem, str));

	SetBkMode(hdc, TRANSPARENT);



	// Draw it once to the lower right in a gold color to make

	// it look like a shadow.

	int cx = w / 2;

	int cy = h / 2;

	if (dis->itemState & ODS_SELECTED) {

		cx++;

		cy++;

	}

	cy -= 2;	// text seems to be too low on most buttons... move it back up.



	if (dis->itemState & ODS_DISABLED) {

		if (color_array[3] != -1) {

			SetTextColor(hdc, color_array[3]);

			DrawCenteredText(hdc, str, cx+1, cy+1, font_array);

		}

		// Now draw it again in our desired location using something dark

		SetTextColor(hdc, color_array[2]);
	

	} else {

		if (color_array[1] != -1) {

			SetTextColor(hdc, color_array[1]);

			DrawCenteredText(hdc, str, cx+1, cy+1, font_array);

		}

		// Now draw it again in our desired location using black

		SetTextColor(hdc, color_array[0]);
	

	}

	DrawCenteredText(hdc, str, cx, cy, font_array);



	// Now that we're done drawing... blit the result back to

	// the original HDC.

	BOOL success = BitBlt(dis->hDC, 0, 0, w, h, hdc, 0, 0, SRCCOPY);

	if (!success) {

		kp(("%s(%d) BitBlt() failed (error %d). dest=$%08lx, src=$%08lx, size=%dx%d\n",

				_FL,GetLastError(), dis->hDC, hdc, w, h));

	}

	SelectObject(hdc, old_hbm);	// unselect our bitmap

	DeleteObject(hbm);			// delete our bitmap

	DeleteDC(hdc);				// delete our DC

}	



//*********************************************************

//
//

// Draw a string centered on a point.  Unlike the Windows DrawText()

// function, this one can handle more than one line of text.

//

void DrawCenteredText(HDC hdc, char *str, int cx, int cy)

{

	DrawCenteredText(hdc, str, cx, cy, NULL);

}



void DrawCenteredText(HDC hdc, char *str, int cx, int cy, HFONT *font_array)

{

	// Count the # of lines of text.

	int line_count = 1;

	char *s = str;

	while (*s) {

		if (*s=='\n') {

			line_count++;

		}

		s++;

	}

	//kp(("%s(%d) String consists of %d lines of text.\n", _FL, line_count));



	int total_height = 0;

	int total_internal_leading = 0;

	HFONT *f = font_array;

	for (int i=0 ; i<line_count ; i++) {

		if (f && *f) {	// select the next font, if specified.

			SelectObject(hdc, *f);

			f++;	// move pointer to point for next line.

		}



		// Determine the height of this line of text.

		TEXTMETRIC tm;

		zstruct(tm);

		GetTextMetrics(hdc, &tm);

		int font_height = tm.tmHeight - tm.tmInternalLeading;

		total_internal_leading += tm.tmInternalLeading;

		//kp(("%s(%d) Font height = %d-%d = %d\n", _FL, tm.tmHeight, tm.tmInternalLeading, font_height));

		if (!font_height) {

			font_height = 16;	// pick something reasonable if we got zero.

		}

		total_height += font_height;

	}

	// Pick some starting y coords (for the first line of text).

	int y = cy - total_height / 2 - total_internal_leading / (line_count*2);



	// Now loop through each line of text and center the x coords.

	s = str;	// start over at the beginning of the string.

	f = font_array;

	while (*s) {

		// Count the number of characters in this line of the string

		int char_count = 0;

		char *s2 = s;

		while (*s2 && *s2!='\n') {

			s2++;

			char_count++;

		}

		// Select the font for this line

		if (f && *f) {	// select the next font, if specified.

			SelectObject(hdc, *f);

			f++;	// move pointer to point for next line.

		}



		// Determine the height of this line of text.

		TEXTMETRIC tm;

		zstruct(tm);

		GetTextMetrics(hdc, &tm);

		int font_height = tm.tmHeight - tm.tmInternalLeading;

		if (!font_height) {

			font_height = 16;	// pick something reasonable if we got zero.

		}



		// Determine the width of this string of characters.

		SIZE size;

		zstruct(size);

		GetTextExtentPoint32(hdc, s, char_count, &size);



		// Draw the text.

		TextOut(hdc, cx - size.cx / 2, y, s, char_count);



		y += font_height;

		s += char_count;

		if (*s=='\n') {

			s++;	// skip over the newline at the end of our line.

		}

	}

}	

