//****************************************************************


//

// Table.cpp : Table related routines for client end

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"

#include "hand.h"

#include "poker.h"

#include "roboplay.h"



#define TIME_DRAWING	0

#define INCLUDE_TABLE_OPTIONS	1	// include new (2000/03) table options stuff?



#define CONTROL_TO_SWITCH_FOCUS_TO	(IDC_TYPE_CHAT)		// which control gets SetFocus() when button deselected?



// if the variables are redefined in client.cpp

// and not here, the error will never be caught (such as adding or removing the "volatile"

// part of the definition).  These sorts of things should ALWAYS be put into a header

// file which is included from both places.

// from client.cpp

extern volatile WORD32 RealChipsInBank;

extern volatile WORD32 RealChipsInPlay;

extern volatile WORD32 FakeChipsInBank;

extern volatile WORD32 FakeChipsInPlay;

extern char *ActionStrings[];

extern HWND hCardRoomDlg;	// handle to cardroom dialog box (if open)

extern HWND hBuyInDLG;			// handle to buy-in dialog, if it exists

extern HWND hEnterChatTextDLG;	// handle to dlg for entering chat text



WORD32 dwCheckForInputButtons_Ticks;	// Ticks when we should check if a window needs topping so user can answer input

WORD32 dwIgnoreInputButtons_Ticks;		// if set, we should ignore table input buttons until this time.

//cris 28-12-2003
int tabFlag=0;
//cris 28-12-2003
int GetStakeForPlayer(struct TableInfo *t, int player_index);
int CountPlayingTables(void);

int CheckIfPlayingCurrentGame(struct TableInfo *t);

struct TableInfo *LastTableWindowTopped;	// an indicator of which table window was last topped
void UpdateMymoneyText(struct TableInfo *t);


HBITMAP  hTableBgnds[6];	// bitmap handles for bgnd pics (3 for play money, 3 for real money)

HPALETTE hTablePalettes[6];	// palette handles for bgnd pics (3 for play money, 3 for real money)

HBITMAP hTournTableScript;	// bitmap for "Tournament Table" scripted text for putting on table

HBITMAP hRealMoneyScript;

HBITMAP hPlayMoneyScript;

HBITMAP hTab;


HFONT hTableFonts[MAX_TABLE_FONTS];



HWND hTableOptionsDlg;


//Aqui se definen los colores para las cajas de chat en las mesas

#define CHAT_TEXT_COLOR RGB(255,255,255)   /*(255,255,215)*/

//#define CHAT_BGND_COLOR RGB(153,64,26)	       // ***change for black background***cream color
#define CHAT_BGND_COLOR RGB(106,54,6)	       // Azul oscuro - Dark Blue


HBRUSH hChatBgndBrush;



HHOOK hColorStaticHook;



int iBitmapLoadingProblem;		// set if there was a problem loading at least one bitmap



struct TableInfo Table[MAX_TABLES];

static int ButtonIDs[MAX_BUTTONS_PER_TABLE] = { IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3, IDC_BUTTON4 };

static int InTurnCheckboxIDs[] = {

			IDC_FOLD_IN_TURN,

			IDC_FOLD_IN_TURN2,	// special case: handled seperately

			IDC_CALL_IN_TURN,

			IDC_CALL_IN_TURN2,

			IDC_RAISE_IN_TURN,

			IDC_RAISE_IN_TURN2,

			0,
};



static int SitDownButtonIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_SIT_DOWN_1,

			IDC_SIT_DOWN_2,

			IDC_SIT_DOWN_3,

			IDC_SIT_DOWN_4,

			IDC_SIT_DOWN_5,

			IDC_SIT_DOWN_6,

			IDC_SIT_DOWN_7,

			IDC_SIT_DOWN_8,

			IDC_SIT_DOWN_9,

			IDC_SIT_DOWN_10

};

static int PlayerTextIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_PLAYERINFO_1,

			IDC_PLAYERINFO_2,

			IDC_PLAYERINFO_3,

			IDC_PLAYERINFO_4,

			IDC_PLAYERINFO_5,

			IDC_PLAYERINFO_6,

			IDC_PLAYERINFO_7,

			IDC_PLAYERINFO_8,

			IDC_PLAYERINFO_9,

			IDC_PLAYERINFO_10

};

static int PlayerHoverIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_HOVER_PLAYER1,

			IDC_HOVER_PLAYER2,

			IDC_HOVER_PLAYER3,

			IDC_HOVER_PLAYER4,

			IDC_HOVER_PLAYER5,

			IDC_HOVER_PLAYER6,

			IDC_HOVER_PLAYER7,

			IDC_HOVER_PLAYER8,

			IDC_HOVER_PLAYER9,

			IDC_HOVER_PLAYER10

};



// Table of windows that we want to process WM_CTLCOLORSTATIC for.

#define MAX_COLORSTATIC_WINDOWS	(20*MAX_TABLES)

int iColorStaticWindowCount;

static HWND hColorStaticWindows[MAX_COLORSTATIC_WINDOWS];

static int  iStaticWindowColorTypes[MAX_COLORSTATIC_WINDOWS];



char *GameRuleNames[MAX_GAME_RULES] = {

		"Texas Hold'em",

		"Omaha Hi",

		"Omaha Hi/Lo 8 or better",

		"7-Card Stud",

		"7-Card Stud Hi/Lo 8 or better",

};



char *GameRuleNamesShort[MAX_GAME_RULES] = {

		"Hold'em",

		"Omaha Hi",

		"Omaha H/L8",

		"7CS",

		"7CS H/L8",

};



struct DlgToolTipText TableDlgToolTipText[] = {

  #if ADMIN_CLIENT

	IDC_COMPUTER_PLAY,		"Debug item: choosing 'Computer Play' will cause the client program "

							"to automatically answer all input requests during gameplay",

  #endif

  #if INCLUDE_TABLE_OPTIONS

	IDC_DEALER_CHIP_AREA,	"Click here for a menu of table options",

  #else

	IDC_DEALER_CHIP_AREA,	"Click here to bring more chips to the table",

  #endif

	IDC_MUCK_LOSING_HANDS,	"Automatically muck your hand when you lose",

	//IDC_FOLD_IN_TURN,		"Automatically fold your hand when it's your turn",

	//IDC_QUIET_DEALER,		"Turns off most of the dealer chatter",

	IDC_LEAVE_TABLE,		"Get out of your seat and leave the table",

	IDC_GOTO_CARDROOM,		"Stay in your seat and bring up the Lobby window",

  #if 1	//20000914MB: text inserted later (when this structure is used)

	IDC_SIT_OUT, 			NULL,

	IDC_POST_IN_TURN, 		NULL,

  #else

	IDC_SIT_OUT, 			"Check this box to sit out each new hand",

	IDC_POST_IN_TURN, 		"Automatically post when it's your turn to post",

  #endif

	IDC_SIT_DOWN_1,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_2,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_3,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_4,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_5,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_6,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_7,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_8,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_9,			"Click here to sit down and start playing",

	IDC_SIT_DOWN_10,		"Click here to sit down and start playing",

  #if 0 // 990729HK as per comments from alpha testers -- these get in the way

	IDC_CHAT_BOX,			"Dealer actions and chat messages from other players are displayed here",

	IDC_TYPE_CHAT,			"Type here to send a chat message to other players at this table",

  #endif

	0,0

};



#define CHATMODE_FLAG_DEALER_BLAB	0x0001

#define CHATMODE_FLAG_DEALER_NORMAL	0x0002

#define CHATMODE_FLAG_DEALER_WINNER	0x0004

#define CHATMODE_FLAG_DEALER_ANY	(CHATMODE_FLAG_DEALER_BLAB|CHATMODE_FLAG_DEALER_NORMAL|CHATMODE_FLAG_DEALER_WINNER)

#define CHATMODE_FLAG_USER			0x0008



struct ChatModeData {

	char *title;

	int flags;

} ChatModeData[] = {

	"Dealer: Everything",	CHATMODE_FLAG_DEALER_ANY    | CHATMODE_FLAG_USER,

	"Dealer: Normal",		CHATMODE_FLAG_DEALER_NORMAL | CHATMODE_FLAG_DEALER_WINNER | CHATMODE_FLAG_USER,

	"Dealer: Summary",		CHATMODE_FLAG_DEALER_WINNER | CHATMODE_FLAG_USER,

	"Dealer: Silent",									  CHATMODE_FLAG_USER,

	"No player chat",		CHATMODE_FLAG_DEALER_NORMAL | CHATMODE_FLAG_DEALER_WINNER,

	"Totally silent",		0,

	0,0

};



//*********************************************************

//
//

// Set the keyboard focus to something reasonable for a table window.

//

void SetTableKeyboardFocus(struct TableInfo *t)

{

	// Only change the keyboard focus if we're already the active window,

	// otherwise we'll be popping to the foreground all over the place

	// with no apparent reason.

	if (t->active_table_window) {

		HWND hwnd = GetDlgItem(t->hwnd, CONTROL_TO_SWITCH_FOCUS_TO);

		if (IsWindowEnabled(hwnd)) {

			SetFocus(hwnd);

		} else {

			//20000918MB: choosing something like IDC_DEALER_CHIP_AREA does

			// work on Win2000, but does NOT work on Win98.  It seems that it has

			// to be some sort of control that can actually accept keyboard input.

			SetFocus(GetDlgItem(t->hwnd, IDC_GOTO_CARDROOM));

		}

	}

}



//*********************************************************

//
//

// Add a chat message (generated locally) to our own chat buffer.

// Player chat does NOT go through this function, it was designed

// for locally generated dealer text only.

//

void AddDealerMessage(struct TableInfo *t, int chat_text_type, char *fmt, ...)

{

	struct GameChatMessage gcm;

	zstruct(gcm);

	va_list arg_ptr;

	va_start(arg_ptr, fmt);

	vsprintf(gcm.message, fmt, arg_ptr);

	va_end(arg_ptr);

	gcm.game_serial_number = t->game_serial_number;

	gcm.table_serial_number = t->table_serial_number;

	gcm.text_type = (BYTE8)chat_text_type;

	strcpy(gcm.name, "Dealer");

	if (UpdateChatBuffer(&gcm, t->table_index)) {

		PostMessage(t->hwnd, WMP_REFRESH_CHAT, 0, 0);

	}

}



//*********************************************************

//
//

// Build the list of window handles we want to process WM_CTLCOLORSTATIC for.

//

static void BuildColorStaticTable(void)

{

	iColorStaticWindowCount = 0;

	struct TableInfo *t = Table;

	for (int i=0 ; i<MAX_TABLES ; i++, t++) {

		if (t->hwnd) {

			// Table 1: the darker color stuff...

			static int ids[] =  {

				//IDC_QUIET_DEALER,

				IDC_MUCK_LOSING_HANDS,

                 IDC_MY_MONEY,
 
				IDC_POST_IN_TURN,

				IDC_SIT_OUT,

				IDC_ALL_INS_REMAINING,

			  #if ADMIN_CLIENT

				IDC_COMPUTER_PLAY,

				IDC_MIN_HAND_RANK,

				//IDC_WATCHING_ACTIVATE,

				IDC_LOW_PLAYERS_ACTIVATE,

				//IDC_WATCHING_COUNT,

				IDC_TOTAL_CONNECTIONS,

				IDC_POTS,

				IDC_LIMIT,

				IDC_NEW_LIMIT,
				
				IDC_RAKE,

			  #endif

				0};

			int *p = ids;

			while (*p) {

				if (iColorStaticWindowCount >= MAX_COLORSTATIC_WINDOWS) {

					kp(("%s(%d) ERROR: ran out of room in hColorStaticWindows[]!\n", _FL));

					break;

				}

				iStaticWindowColorTypes[iColorStaticWindowCount] = 0;

				hColorStaticWindows[iColorStaticWindowCount++] = GetDlgItem(t->hwnd, *p++);

			}

			// Table 2: the brighter color stuff...

			static int ids2[] =  {

				//rgong 04/04/2002 - 1

				IDC_HIGH_HAND,
                
				IDC_MY_MONEY,
              
				//end rgong

				IDC_TABLE_GAME_SERIAL_NUM,

				IDC_WAIT_LIST_STATUS,

				IDC_BEST_HAND,

				IDC_LOWEST_HAND,

				IDC_FOLD_IN_TURN,

				IDC_FOLD_IN_TURN2,

				IDC_CALL_IN_TURN,

				IDC_CALL_IN_TURN2,

				IDC_RAISE_IN_TURN,

				IDC_RAISE_IN_TURN2,

				//IDC_TAB,
				
				0};

			p = ids2;

			while (*p) {

				if (iColorStaticWindowCount >= MAX_COLORSTATIC_WINDOWS) {

					kp(("%s(%d) ERROR: ran out of room in hColorStaticWindows[]!\n", _FL));

					break;

				}

				iStaticWindowColorTypes[iColorStaticWindowCount] = 1;

				hColorStaticWindows[iColorStaticWindowCount++] = GetDlgItem(t->hwnd, *p++);

			}

		}

	}

}



//*********************************************************

//
//

// Set the IDC_WAIT_LIST_STATUS string and show window if non-null

//

void SetWaitListStatusString(struct TableInfo *t, char *str)

{

	HWND hwnd = GetDlgItem(t->hwnd, IDC_WAIT_LIST_STATUS);

	if (!str || !str[0]) {

		ShowWindowIfNecessary(hwnd, SW_HIDE);

		zstruct(t->wait_list_status_string);

	} else {

		if (strcmp(str, t->wait_list_status_string)) {	// has it has changed?

			strnncpy(t->wait_list_status_string, str, WAITLIST_STATUS_STR_LEN);

			InvalidateRect(hwnd, NULL, FALSE);

		}

		ShowWindowIfNecessary(hwnd, SW_SHOWNA);

	}

}	



//*********************************************************

//
//

// Redraw entire background, all chips, and all cards

// each time to our off screen bitmap and flag the window

// client area as needing repainting.

//

// Rough drawing queue priorities:

//

//	100 = cards laying flat on the table (small, face down or public cards)

//	150 = bar snacks

//	200 = dealer button when not moving

//	300 = chips sitting on the table

//	400 = animated chips

//	500 = animated cards

//	600 = large private cards

//

void RedrawAllTableGraphics(struct TableInfo *t)

{

  #if ADMIN_CLIENT

	if (RunningManyFlag) return;

  #endif



  #if 0

	static int old_action;

	if (t->ClientState.in_turn_action != old_action) {

		kp(("%s(%d) ***** t->ClientState.in_turn_action changed from %d to %d\n",_FL,old_action,t->ClientState.in_turn_action));

		old_action = t->ClientState.in_turn_action;

	}

  #endif



	EnterCriticalSection(&CardRoomVarsCritSec);

    if (t->hwnd) {

    	EnterCriticalSection(&t->draw_crit_sec);

		if (!t->hwnd) {	// window got closed on us.

	    	LeaveCriticalSection(&t->draw_crit_sec);

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return;

		}

	  #if ADMIN_CLIENT && TIME_DRAWING

		WORD32 start_tsc = (WORD32)rdtsc();

		WORD32 before_draw_queue_tsc = start_tsc;

		WORD32 after_draw_queue_tsc = start_tsc;

	  #endif



    	t->animation_flag = FALSE;  // for now, assume nothing got animated.

		t->blit_queue.Init();



	  #if 0	//19990821MB: testing

		extern HPALETTE hBlitPalette;

		hBlitPalette = t->hpalette;

		//kp(("%s(%d) Setting hBlitPalette to $%08lx\n", _FL, hBlitPalette));

	  #endif



		//20000628MB: if the chip tray is highlighted, draw the highlighted version.

		// This code can easily be moved elsewhere at a later date.

		if (t->proximity_highlight_list.hCurrentHighlight &&

			t->proximity_highlight_list.hCurrentHighlight == GetDlgItem(t->hwnd, IDC_DEALER_CHIP_AREA))

		{

			int bitmap_index = -1;

			switch (t->chip_type) {

				case CT_NONE:

					Error(ERR_INTERNAL_ERROR,"%s(%d) called with CT_NONE", _FL);

					break;

				case CT_PLAY:

					bitmap_index = MISCBITMAP_CHIP_TRAY_R;

					break;

				case CT_REAL:

					bitmap_index = MISCBITMAP_CHIP_TRAY_RM_R;

					break;

				case CT_TOURNAMENT:

					bitmap_index = MISCBITMAP_CHIP_TRAY_RM_R;

					break;

				default:

					Error(ERR_INTERNAL_ERROR,"%s(%d) called with unknown chip_type", _FL);

			}

			POINT pt;

			GetPictureCoordsOnTable(t, IDC_DEALER_CHIP_AREA, &pt);

			t->blit_queue.AddBitmap(MiscBitmaps[bitmap_index], &pt, 0);

		}

		if (t->chip_type==CT_TOURNAMENT && hTournTableScript) {

			POINT pt;

			GetPictureCoordsOnTable(t, IDC_TOURN_TABLE_SCRIPT, &pt);
			
			t->blit_queue.AddBitmap(hTournTableScript, &pt, 0);

		}

		

		

	

	//	if (t->chip_type==CT_PLAY && hPlayMoneyScript) {
   	if (tabFlag) {
			POINT pt;
			GetPictureCoordsOnTable(t, IDC_TOURN_TABLE_SCRIPT, &pt);
			t->blit_queue.AddBitmap(hPlayMoneyScript, &pt, 500);
		}

		

		

		if (t->chip_type==CT_REAL && hRealMoneyScript) {

			POINT pt;

			GetPictureCoordsOnTable(t, IDC_REAL_TABLE_SCRIPT, &pt);

			t->blit_queue.AddBitmap(hRealMoneyScript, &pt, 1);

		}

		

		



	  #if INCLUDE_TABLE_OPTIONS

		if (!Defaults.iBarSnacksDisabled) {

			DrawAllBarSnacks(t);

		}

	  #endif

    	DrawAllCards(t);	// draw all cards (and update animation)

    	DrawAllChips(t);	// draw all chips and dealer button (and update animation)
		

		if (!t->minimized_flag) {	// only do actual drawing if not minimized.

	    	HDC hdc = CreateCompatibleDC(NULL);

			if (hdc) {

		    	SelectObject(hdc, t->hbm_off_screen);	// draw into our off-screen bitmap

				if (t->hbm_background) {

					if (t->hpalette) {

						SelectPalette(hdc, t->hpalette, FALSE);

						RealizePalette(hdc);

					}

				  #if ADMIN_CLIENT && TIME_DRAWING

					before_draw_queue_tsc = (WORD32)rdtsc();

				  #endif

			    	BlitBitmapToDC_nt(t->hbm_background, hdc);	// draw background

				  #if ADMIN_CLIENT && TIME_DRAWING

					after_draw_queue_tsc = (WORD32)rdtsc();

				  #endif

				}

				t->blit_queue.DrawQueue(hdc);	// draw queued objects

		    	DeleteDC(hdc);

			}



			// Redraw the window right now from here.  Don't use WM_PAINT.

			if (!IsIconic(t->hwnd) && t->hbm_off_screen) {

				HDC hdc = GetDC(t->hwnd);

				if (hdc) {

					if (t->hpalette) {

						SelectPalette(hdc, t->hpalette, FALSE);

						RealizePalette(hdc);

					}

					BlitBitmapToDC_nt(t->hbm_off_screen, hdc);

					ReleaseDC(t->hwnd, hdc);

				}

			}

			if (t->invalidate_player_id_boxes) {

				InvalidatePlayerIDBoxes(t);	// force redraw so card remnants disappear.

			}

			if (t->invalidate_top_right_text) {

				// force redraw so drink remnants are dealt with properly

				t->invalidate_top_right_text = FALSE;

				HWND hwnd = GetDlgItem(t->hwnd, IDC_TABLE_GAME_SERIAL_NUM);

				if (hwnd) {

					InvalidateRect(hwnd, NULL, FALSE);

				}

				hwnd = GetDlgItem(t->hwnd, IDC_ALL_INS_REMAINING);

				if (hwnd) {

					InvalidateRect(hwnd, NULL, FALSE);

				}

			}

		}

		t->blit_queue.Init();	// always clear queue when drawing done.



	  #if ADMIN_CLIENT && TIME_DRAWING

		static WORD32 prev_tsc;

		WORD32 now = (WORD32)rdtsc();

		WORD32 non_drawing = start_tsc - prev_tsc;

		WORD32 elapsed_tsc = now - start_tsc;

		WORD32 before_draw_queue = before_draw_queue_tsc - start_tsc;

		WORD32 during_draw_queue = after_draw_queue_tsc - before_draw_queue_tsc;

		WORD32 post_draw_queue = now - after_draw_queue_tsc;

		#define TSC_DIVISOR	10000

		kp(("%s(%d) Elapsed while drawing =%6u/%6u/%6u (total %d), non drawing =%8u (drawing is %5.1f%%)\n",

				_FL,

				before_draw_queue / TSC_DIVISOR,

				during_draw_queue / TSC_DIVISOR,

				post_draw_queue / TSC_DIVISOR,

				elapsed_tsc / TSC_DIVISOR,

				non_drawing / TSC_DIVISOR,

				(double)elapsed_tsc*100.0 / ((double)elapsed_tsc + (double)non_drawing)));

		prev_tsc = (WORD32)rdtsc();

	  #endif



		if (t->animation_disable_flag) {

			t->animation_disable_count = 1;	// always set to non-zero

			//kp(("%s(%d) forcing table->animation_disable_count = %d\n", _FL, t->animation_disable_count));

		} else if (t->animation_disable_count) {	// are we skipping the first few updates?

			t->animation_disable_count--;	// yes, dec counter.

			//kp(("%s(%d) table->animation_disable_count has been decremented to %d\n", _FL, t->animation_disable_count));

		}

		if (t->new_cards_dealt_flag && !t->dealing_flag && t->flop_animation_state==FLOP_ANIM_INIT) {

			t->new_cards_dealt_flag = FALSE;

			// Dealing is finished.  Make sure input buttons are up to date.

			//kp(("%s(%d) %4d: Dealing is finished. t->new_cards_dealt_flag = %d, t->dealing_flag = %d\n", _FL, SecondCounter, t->new_cards_dealt_flag, t->dealing_flag));

			PostMessage(t->hwnd, WMP_UPDATE_YOURSELF, 0,0);

			// Finished dealing -- flush any buffered dealer chat text

			PostMessage(t->hwnd, WMP_FLUSH_DEALER_CHAT_BUFFER, 0, 0);

			if (!t->watching_flag

			   #if ADMIN_CLIENT

				 && !t->computer_play

			   #endif

			) {

				// Top our window so they can see the new cards.

				if (CountPlayingTables() <= 1) {

					//kp(("%s(%d) %4d: Calling ShowTableWindowIfPlaying(%d). CountPlayingTables() = %d\n", _FL, SecondCounter, t->table_serial_number, CountPlayingTables()));

					// 20001116HK: don't top it if we don't want it...

					int top_window_now = TRUE;

					#if MIN_HAND_RANK_POPUP

					  // if there's a setting here and we don't qualify, no popup

					  pr(("%s(%d) Testing... puohr=%d, chr = %d, puphrn = %d, TrHand %s\n", _FL, t->pop_up_on_hand_rank, t->current_hand_rank, t->pop_up_on_hand_rank_number,t->tournament_game_number_str));

					  if (t->pop_up_on_hand_rank && 

						  t->current_hand_rank > t->pop_up_on_hand_rank_number) {	// skip it

							pr(("%s(%d) Skipping...\n", _FL));

							top_window_now = FALSE;

					  }

					#endif

					 if (top_window_now) {

						ShowTableWindowIfPlaying(t);

					 }

				}

			}

		}

    	LeaveCriticalSection(&t->draw_crit_sec);

    }

	LeaveCriticalSection(&CardRoomVarsCritSec);

}



//*********************************************************

//
//

// Return the handle to the top table window (playing or not).

//

HWND FindTopTableWindow(void)

{

	HWND top = NULL;

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd) {

			if (top) {

				// Compare with previous top.

				HWND hwnd = top;	// start at the current top one

				do {

					hwnd = GetNextWindow(hwnd, GW_HWNDPREV);

				} while(hwnd && hwnd != Table[i].hwnd);

				if (hwnd==Table[i].hwnd) {

					// We found ourselves in front of the previous top

					top = Table[i].hwnd;	// we're the new top one.

				}

			} else {

				// No top yet, therefore it must be us.

				top = Table[i].hwnd;

			}

		}

	}

	return top;

}



//*********************************************************

//
//

// Flag a table window as needing redrawing and post a message

// to its message queue if necessary.

//

void FlagRedrawTable(int table_index)

{

	struct TableInfo *t = &Table[table_index];

	if (!t->redraw_needed && !t->minimized_flag) {

		// First time it's getting set... post a message to

		// the message queue for the dialog box to update itself

		// the first time it gets a chance.

		t->redraw_needed = TRUE;

		PostMessage(t->hwnd, WMP_UPDATE_YOURSELF, 0,0);

	}



	// Redraw entire background, all chips, and all cards

	// each time to our off screen bitmap and flag the window

	// client area as needing repainting.

    // It's very important that this gets called for EVERY packet

    // that comes in so that we don't miss any state transitions.

	RedrawAllTableGraphics(t);

}



//****************************************************************

//
//

// Convert a card to an ascii string.

// Also returns output_string as a result.

//

char *CardToString(Card card, char *output_string)

{

	if (card==CARD_NO_CARD) {

		*output_string = 0;

	} else if (card==CARD_HIDDEN) {

		strcpy(output_string, "*");

	} else {

		output_string[0] = cRanks[RANK(card)];

		output_string[1] = cSuits[SUIT(card)];

		output_string[2] = 0;

	}

	return output_string;

}



//*********************************************************

////
// Set the state on a dialog checkbox and invalidate it
// if we changed it.

//

void CheckDlgButtonIfNecessary(HWND hwnd, int control_id, int new_checkbox_state)

{

	int old_state = IsDlgButtonChecked(hwnd, control_id);
	if ((new_checkbox_state && !old_state) || (!new_checkbox_state && old_state)) {
		// change it.
		CheckDlgButton(hwnd, control_id, new_checkbox_state);
		InvalidateRect(GetDlgItem(hwnd, control_id), NULL, FALSE);
	}

}



//*********************************************************

//
//

// Determine if we're still playing in the current game or
// if we're watching, sitting out, or folded.
// Returns TRUE if we're playing in the current hand.
//

int CheckIfPlayingCurrentGame(struct TableInfo *t)

{

	if (!t->hwnd) {	// table window is closed, therefore not playing

		return FALSE;

	}

	if (t->watching_flag) {
        
		return FALSE;	// just watching, not playing

	}



	//20000627MB: if we've got a pending input request that has

	// something in the action mask, just assume we're playing.

	if (t->GamePlayerInputRequest.action_mask) {

		return TRUE;	// at least one action request is pending

	}



	// We're actually playing at this table...

	int our_status = t->GamePlayerData.player_status[

					 t->GamePlayerData.seating_position];



	// If game over, treat as if we're sitting out.

	if (t->GamePlayerData.s_gameover) {

		our_status = PLAYER_STATUS_SITTING_OUT;

	}



	if (our_status==PLAYER_STATUS_PLAYING || our_status==PLAYER_STATUS_ALL_IN) {

		return TRUE;	// we're still playing.

	}

	return FALSE;	// we're out.  not playing.

}	



//*********************************************************

//
//

// Count the number of tables we're currently playing at.

// If we've folded or are sitting out, we're not counted as playing there.

// (see CheckIfPlayingCurrentGame for more details).

//

int CountPlayingTables(void)

{

	int playing_count = 0;

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (CheckIfPlayingCurrentGame(&Table[i])) {

			playing_count++;

		}

	}

	return playing_count;

}



//*********************************************************

//
//

// Determine if a particular table currently has action buttons

// showing for a game (during gameplay).

//

int CheckIfActionButtonsShowing(struct TableInfo *t)

{

	if (!CheckIfPlayingCurrentGame(t)) {

		return FALSE;	// not playing, so can't have action buttons.

	}

  #if 1

	if (t->GamePlayerInputRequest.action_mask) {

		return TRUE;	// at least one button is up and showing

	}

  #else

	for (int i=0 ; i<MAX_BUTTONS_PER_TABLE ; i++) {

		if (t->button_actions[i]) {

			return TRUE;	// at least one button is up and showing

		}

	}

  #endif

	return FALSE;	// nothing showing right now.

}



//*********************************************************
////
// Count the number of tables that currently have game action
// buttons up and showing.  See CheckIfActionButtonsShowing() for
// details of that definition.
//

int CountTablesWithActionButtonsShowing(void)

{

	int count = 0;

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (CheckIfActionButtonsShowing(&Table[i])) {

			count++;

		}

	}

	return count;

}



//*********************************************************

//
//

// Clear all in-turn actions for a table.  Does NOT send new

// ClientState structure to the server but it DOES update the

// checkboxes on the screen (whether they are shown or not).

//

void ClearInTurnActions(struct TableInfo *t)

{

	ClearInTurnActions(t, TRUE);

}



void ClearInTurnActions(struct TableInfo *t, int clear_checkboxes_flag)

{

	if (t->ClientState.in_turn_action && clear_checkboxes_flag) {

		//kp(("%s(%d) Clearing in_turn_action of %d\n",_FL, t->ClientState.in_turn_action));

		// If we're clearing an in-turn action, make sure we top the window

		// so the player can see what's happening...

		//20000627MB: don't do show it if playing more than one table.

		if (CountPlayingTables() <= 1) {

			kp(("%s(%d) %4d: Calling ShowTableWindowIfPlaying(%d). CountPlayingTables() = %d\n", _FL, SecondCounter, t->table_serial_number, CountPlayingTables()));

			ShowTableWindowIfPlaying(t);

		}

	}

	t->ClientState.in_turn_action = 0;

	t->ClientState.in_turn_action_game_state = 0;

	t->ClientState.in_turn_action_last_input_request_serial_number = 0;

	t->ClientState.in_turn_action_amount = 0;

	t->ClientState.in_turn_action_game_serial_number = t->GameCommonData.game_serial_number;

	t->ClientState.fold_in_turn = FALSE;	//20000918MB: make sure this gets cleared too.



	if (clear_checkboxes_flag) {

		CheckDlgButtonIfNecessary(t->hwnd, IDC_FOLD_IN_TURN,   0);

		CheckDlgButtonIfNecessary(t->hwnd, IDC_FOLD_IN_TURN2,  0);

		CheckDlgButtonIfNecessary(t->hwnd, IDC_CALL_IN_TURN,   0);

		CheckDlgButtonIfNecessary(t->hwnd, IDC_CALL_IN_TURN2,  0);

		CheckDlgButtonIfNecessary(t->hwnd, IDC_RAISE_IN_TURN,  0);

		CheckDlgButtonIfNecessary(t->hwnd, IDC_RAISE_IN_TURN2, 0);

	}

}



//*********************************************************
////
// Show of hide a window (SW_SHOWNA or SW_HIDE) if necessary.
// Only hide if it is unchecked.
// Returns TRUE if the window was actually shown.
//

static int ShowOrHideIfUnchecked(HWND hwnd, int id, int show_flag)

{

	int result = FALSE;

	if (IsDlgButtonChecked(hwnd, id)) {

		show_flag = TRUE;

	}

	HWND cb_hwnd = GetDlgItem(hwnd, id);

	if (show_flag && !IsWindowVisible(cb_hwnd)) {

		result = TRUE;	// we're gonna show it.

	}

	ShowWindowIfNecessary(cb_hwnd, show_flag ? SW_SHOWNA : SW_HIDE);

	return result;

}


//***************************************************
// 31-3-2003
//ccv
//Update el money text with user money in the table
//
void UpdateMymoneyText(struct TableInfo *t)
{   
//	HDC wParam;
	//HWND hDlg;
//	HWND hwnd;
	char str1[20];
	char str2[20];
	char cs[MAX_CURRENCY_STRING_LEN];
	ChipType chip_type_to_display = (t->GameCommonData.flags & GCDF_USE_REAL_MONEY_CHIPS ? CT_REAL : t->chip_type);
	if(t->chip_type ==CT_PLAY){
		  strcpy(str2,"My play money: ");
		}else{
		if(t->chip_type ==CT_TOURNAMENT){
			strcpy(str2,"My Tournament money: "); 
		}else{
		    strcpy(str2,"My real money: ");
        };//if
	};//if
	int stake = GetStakeForPlayer(t, t->GamePlayerData.seating_position);

	if(t->chip_type ==CT_PLAY){
	   sprintf(str1,"%s",CurrencyString(cs, stake, chip_type_to_display, TRUE, 0));
	}else{
	   sprintf(str1,"%s",CurrencyString(cs, stake, chip_type_to_display, TRUE, 0));
	};//if
  
   strcat(str2,str1);
   SetDlgItemText(t->hwnd, IDC_MY_MONEY, str2); 
   ShowWindowIfNecessary(GetDlgItem(t->hwnd,IDC_MY_MONEY), SW_SHOW);

   //J Fonseca	  28/01/2004
   HWND slider = GetDlgItem(t->hwnd, IDC_LIMIT);
   int rmax = stake / 100;
   SendMessage(slider, TBM_SETRANGEMAX, 1, rmax); //Asigna el "Max Range" para el slider
   SendMessage(slider, TBM_SETTICFREQ, 50, 0); //Asigna una frecuencia de 50
   //	moneyflag =0;
};//UpdateMymoneyText





//*********************************************************

//
//

// Update any buttons on our screen during gameplay

// (including all the action buttons)

//

void UpdatePlayingButtons(struct TableInfo *t)

{

	// Update any buttons we're waiting to have pressed.

	// 990810HK: ActionStrings[] have been moved to pplib.cpp

	t->button_1_alternate_meaning = 0;	// 0=none, 1=Join Waiting list, 2=Unjoin Waiting List

	pr(("%s(%d) UpdatePlayingButtons() has been called\n",_FL));

	WORD32 input_bits_left = t->GamePlayerInputRequest.action_mask;

	// we are playing if it's during a game and our status is playing or all in

	//20001005MB: this is now quite right.  Once we get to the show/muck stage, the

	// s_gameflow variable changes to GAMEFLOW_AFTER_GAME, even though there's still

	// input questions coming from the server.  That's what might_get_an_input_this_game

	// is all about (see below).

	int we_are_playing = 

		( (t->GamePlayerData.s_gameflow == GAMEFLOW_DURING_GAME) &&

		  ( (t->GamePlayerData.player_status[t->GamePlayerData.seating_position] == PLAYER_STATUS_PLAYING) ||

		    (t->GamePlayerData.player_status[t->GamePlayerData.seating_position] == PLAYER_STATUS_ALL_IN)

		  )

		);


	//ccv my money text
	if(
		//(t->GamePlayerData.player_status[t->GamePlayerData.seating_position] != PLAYER_STATUS_PLAYING)||
#if 1
		((t->GamePlayerData.s_gameflow != GAMEFLOW_DURING_GAME) &&
		(t->GamePlayerData.s_gameflow != GAMEFLOW_AFTER_GAME) &&
		(t->GamePlayerData.s_gameflow != GAMEFLOW_BEFORE_GAME))
#endif
	//	t->watching_flag 
		)
	{
	 // char ttt[25];
     // strcpy(ttt,"Caca.");
     // SetDlgItemText(t->hwnd, IDC_MY_MONEY, ttt); 
	//  ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MY_MONEY), SW_HIDE);
	}else{
		
	};//if
	//ccv my money text


	int might_get_an_input_this_game =

		( (t->GamePlayerData.s_gameflow == GAMEFLOW_DURING_GAME || t->GamePlayerData.s_gameflow == GAMEFLOW_AFTER_GAME) &&

		  ( (t->GamePlayerData.player_status[t->GamePlayerData.seating_position] == PLAYER_STATUS_PLAYING) ||

		    (t->GamePlayerData.player_status[t->GamePlayerData.seating_position] == PLAYER_STATUS_ALL_IN)

		  )

		);



	//20000908MB: if we're in sit out mode (post/fold in tournaments), we should

	// display the 'I'm back' button all the time.

	char *explanation = NULL;

	if (t->ClientState.sitting_out_flag &&

		(!might_get_an_input_this_game || (t->chip_type==CT_TOURNAMENT && !input_bits_left)))

	{

		pr(("%s(%d) might_get_an_input_this_game = %d.  game_flow = %d, player_status = %d\n",

					_FL, might_get_an_input_this_game,

					t->GamePlayerData.s_gameflow,

					t->GamePlayerData.player_status[t->GamePlayerData.seating_position]));

		t->button_1_alternate_meaning = 3;	// 3="I'm Back"

		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[0], "I'm Back");

		EnableWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[0]), TRUE);

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[0]), SW_SHOWNA);
		
		//ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TAB), SW_HIDE);

		t->button_actions[0] = ACT_NO_ACTION;

		if (t->chip_type==CT_TOURNAMENT) {	// tournament game

			if (t->forced_to_sit_out_flag) {

				explanation =

					"You timed out. You will post and fold automatically every hand.\n"

					"Click \"I'm Back\" when you are ready to play again.";

			} else {	// player selected sit out manually

				explanation =

					"You have chosen to post and fold automatically every hand.\n"

					"Click \"I'm Back\" when you want to get back into play.";

			}

		} else {							// regular game

			if (t->forced_to_sit_out_flag) {

				explanation =

					"You are sitting out because you timed out.\n"

					"Click \"I'm Back\" when you are ready to play again.";

			} else {	// player selected sit out manually

				explanation =

					"You have chosen to sit out.\n"

					"Click \"I'm Back\" when you want to get back into play.";

			}

		}

		we_are_playing = FALSE;	// treat other buttons the same as if we're not playing.

	} else {

		//20000918MB: if we're sitting out from a tournament table, but the

		// client has clicked 'I'm back', give them the welcome back message.

		if (t->chip_type==CT_TOURNAMENT) {

			// (not done yet)

		}

	}

	SetWaitListStatusString(t, explanation);



	// Buttons should not be displayed if the GPIR is not ready or if

	// we are still dealing cards.

	t->button_actions_are_in_turn = FALSE;	// default to not 'in turn'

	int hide_all_in_turn_checkboxes = TRUE;	// default to hiding them all

	if (!t->GamePlayerInputRequest.ready_to_process || t->dealing_flag) {

		input_bits_left = 0;	// none allowed if GPIR is not ready

		int disable_in_turn_buttons = FALSE;

		#define INTURN_DELAY_AFTER_LAST_INPUT_RESULT 4

		if (SecondCounter - t->last_input_result_seconds < INTURN_DELAY_AFTER_LAST_INPUT_RESULT) {

			// it's too soon after we sent the last input result out... don't

			// draw the 'in turn' buttons for a few more seconds.

			disable_in_turn_buttons = TRUE;

			t->redraw_controls_time = t->last_input_result_seconds + INTURN_DELAY_AFTER_LAST_INPUT_RESULT;

		}

		// If we're not likely going to get another turn in this round of betting,

		// disable the 'in turn' buttons.  also disable if we're about to receive an

		// input request

		if (!t->GamePlayerData.expected_turn_this_round ||

			t->GamePlayerData.game_state < START_BETTING_ROUND_1 ||

			t->GamePlayerData.game_state >= END_GAME_START ||

			t->dealing_flag || 

			t->GamePlayerData.seating_position == t->GamePlayerData.p_waiting_player)

		{

			disable_in_turn_buttons = TRUE;

		}

			

		// 990721HK: when there's nothing to show on the buttons, by default we

		// leave them visible but disabled

		if (we_are_playing &&

			t->GamePlayerData.player_status[t->GamePlayerData.seating_position] != PLAYER_STATUS_ALL_IN &&

			t->fold_game_serial_number != t->game_serial_number &&

			!disable_in_turn_buttons)

		{

			//kp(("%s(%d) Updating button %d for 'in turn' mode.\n",_FL, j));

		  	// Use check boxes instead...

			t->button_actions_are_in_turn = TRUE;

			if (t->ClientState.in_turn_action) {

				// An in-turn action has been selected... see if we want to unselect it.

				if (t->ClientState.in_turn_action_game_serial_number != t->GamePlayerData.game_serial_number) {

					// Always clear the in-turn action when the game has changed

					ClearInTurnActions(t);

				}

				// If they have selected 'fold in turn (always)', ignore the game state.

				if (t->ClientState.in_turn_action!=ACT_FOLD || t->ClientState.in_turn_action_amount!=-1) {

					// They have NOT selected 'fold in turn (always)'.

					if (t->ClientState.in_turn_action_game_state != t->GamePlayerData.game_state) {

						// Always clear the in-turn action when the game state doesn't match anymore.

						ClearInTurnActions(t);

					}

				}

				if (t->ClientState.in_turn_action_amount != -1) {

					// Check if the in turn action amount has changed.

					if (t->ClientState.in_turn_action==ACT_CALL &&

						t->ClientState.in_turn_action_amount != t->GamePlayerData.call_amount)

					{

						// Call in turn action amount does not match.  Clear checkbox.

						ClearInTurnActions(t);

					}

					// Check if the in turn action amount has changed.

					if (t->ClientState.in_turn_action==ACT_RAISE &&

						t->ClientState.in_turn_action_amount != t->GamePlayerData.raise_amount)

					{

						// Bet/Raise in turn action amount does not match.  Clear checkbox.

						ClearInTurnActions(t);

					}

				}

			}



			if (!t->ClientState.in_turn_action) {	// if nothing selected, make sure they're unchecked
//cris 28-12-2003
	//		POINT pt;
	//		GetPictureCoordsOnTable(t, IDC_TOURN_TABLE_SCRIPT, &pt);
	//		t->blit_queue.AddBitmap(hPlayMoneyScript, &pt, 0);
//cris 28-12-2003

				//cris 28-12-2003
                   tabFlag=1;
	             //cris 28-12-2003
				RedrawAllTableGraphics(t);
				CheckDlgButtonIfNecessary(t->hwnd, IDC_FOLD_IN_TURN, 0);

				CheckDlgButtonIfNecessary(t->hwnd, IDC_FOLD_IN_TURN2, 0);

				CheckDlgButtonIfNecessary(t->hwnd, IDC_CALL_IN_TURN, 0);

				CheckDlgButtonIfNecessary(t->hwnd, IDC_CALL_IN_TURN2, 0);

				CheckDlgButtonIfNecessary(t->hwnd, IDC_RAISE_IN_TURN, 0);

				CheckDlgButtonIfNecessary(t->hwnd, IDC_RAISE_IN_TURN2, 0);
                RedrawAllTableGraphics(t);
			}

			// Set the text on the 3 checkboxes appropriately...

			// If there's a call amount...

			//	1) change the 'fold/check' button to just 'fold'

			//  2) change the 'call/check' button to just 'check'.

			//  3) change the 'bet/raise' button to just 'raise'

			int show_alternate_fold_checkbox = TRUE;

			//ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TAB), SW_SHOWNA);			

			if (t->GamePlayerData.call_amount) {

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_FOLD_IN_TURN),"Fold");

				// The fold/check checkbox is in 'fold' mode, only show it if

				// if is checked, otherwise it would be the same as the main 'fold'

				// checkbox.

				if (!IsDlgButtonChecked(t->hwnd, IDC_FOLD_IN_TURN)) {

					show_alternate_fold_checkbox = FALSE;

				}

				
							
				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_CALL_IN_TURN),

						"Call any in turn");



				char text[50], money_text[MAX_CURRENCY_STRING_LEN];

				CurrencyString(money_text, t->GamePlayerData.call_amount, t->chip_type);

				sprintf(text, "Call (%s) in turn", money_text);

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_CALL_IN_TURN2),

						text);



				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_RAISE_IN_TURN),

						"Raise any in turn");



				CurrencyString(money_text, t->GamePlayerData.raise_amount, t->chip_type);

				sprintf(text, "Raise (%s) in turn", money_text);

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_RAISE_IN_TURN2),

						text);

			} else {

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_FOLD_IN_TURN),

						"Fold/check in turn");

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_CALL_IN_TURN),

						"Call/check any in turn");

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_CALL_IN_TURN2),

						"Check in turn");

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_RAISE_IN_TURN),

						"Bet/Raise in turn");



				char text[50], money_text[MAX_CURRENCY_STRING_LEN];

				CurrencyString(money_text, t->GamePlayerData.raise_amount, t->chip_type);

				sprintf(text, "Bet (%s) in turn", money_text);

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_RAISE_IN_TURN2),

						text);

				

			}



			for (int j=0; j<MAX_BUTTONS_PER_TABLE ; j++) {

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[j]), SW_HIDE);
				t->button_actions[j] = ACT_NO_ACTION;

			}



			// If the call amount and the raise amount are the same

			// then the pot is capped.  Hide anything that doesn't apply.

			int pot_is_capped = FALSE;

			if (t->GamePlayerData.call_amount==t->GamePlayerData.raise_amount) {

				pot_is_capped = TRUE;

			}



			// Only hide them if they are unchecked.

			int did_show = FALSE;

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_FOLD_IN_TURN,   show_alternate_fold_checkbox);

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_FOLD_IN_TURN2,  TRUE);

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_CALL_IN_TURN,   !pot_is_capped);

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_CALL_IN_TURN2,  TRUE);

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_RAISE_IN_TURN,  !pot_is_capped);

			did_show |= ShowOrHideIfUnchecked(t->hwnd, IDC_RAISE_IN_TURN2, !pot_is_capped);

			// If we showed some new checkboxes, top our table if necessary.

			if (did_show

			   #if ADMIN_CLIENT

				 && !t->computer_play

			   #endif

			) {

				// Top our window so they can see the new checkboxes

				//20000627MB: don't do show it if playing more than one table.

				if (CountPlayingTables() <= 1) {

					//kp(("%s(%d) %4d: Calling ShowTableWindowIfPlaying(%d). CountPlayingTables() = %d\n", _FL, SecondCounter, t->table_serial_number, CountPlayingTables()));

					ShowTableWindowIfPlaying(t);

				}

			}

			hide_all_in_turn_checkboxes = FALSE;		

		}

	}



	// the wait_for_bb button takes over one of the usual checkboxes -- change here if needed

	#define _IDC_WAIT_FOR_BB	IDC_RAISE_IN_TURN2

	if (t->wait_for_bb) {

		SetWindowTextIfNecessary(GetDlgItem(t->hwnd, _IDC_WAIT_FOR_BB), "Wait for Big Blind");

		CheckDlgButtonIfNecessary(t->hwnd, _IDC_WAIT_FOR_BB, t->wait_for_bb);

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, _IDC_WAIT_FOR_BB), SW_SHOWNA);

	}

	// If the in-turn checkboxes all need hiding, make sure that gets done now.

	if (hide_all_in_turn_checkboxes) {
		//cris 28-12-2003
       tabFlag=0;
	   //cris 28-12-2003
		int hid_one = FALSE;

		for (int j=0; InTurnCheckboxIDs[j] ; j++) {
			// 20000627HK: remove flicker for [Wait For BB] -- it's the only exception
			if (t->wait_for_bb && (InTurnCheckboxIDs[j] == _IDC_WAIT_FOR_BB)) {
				// leave it
			} else {
				// Hide them and take note if we hid one.
				BOOL previously_visible = ShowWindowIfNecessary(GetDlgItem(t->hwnd, InTurnCheckboxIDs[j]), SW_HIDE);
				if (previously_visible) {
					hid_one = TRUE;
				}
			};//if (t->wait_for_bb && (InTurnCheckboxIDs[j] == _IDC_WAIT_FOR_BB)) {
		};//	for (int j=0; InTurnCheckboxIDs[j] ; j++) {
        RedrawAllTableGraphics(t);
		if (hid_one) {
			// focus needs clearing from in-turn actions getting acted on by the comm thread.
			SetTableKeyboardFocus(t);	// purely cosmetic
		}
     // RedrawAllTableGraphics(t);
	};//	if (hide_all_in_turn_checkboxes) {

	

	int show_fold_in_turn_checkbox = we_are_playing;

	int remove_wait_for_bb = FALSE;

	int valid_wait_for_bb = 

		( !t->wait_for_bb && 

		  (input_bits_left & 1<<ACT_SIT_OUT_POST) ||

		  (input_bits_left & 1<<ACT_SIT_OUT_BOTH)

		);

	// find the valid SIT_OUT reason that we might use with wait_for_bb handling

	BYTE8 sit_out_reason = ACT_SIT_OUT_POST;

	if (valid_wait_for_bb) {	// we'll need a valid sit-out reason

		if (input_bits_left & 1<<ACT_SIT_OUT_BB) {

			sit_out_reason = ACT_SIT_OUT_BB;

		} else 	if (input_bits_left & 1<<ACT_SIT_OUT_SB) {

			sit_out_reason = ACT_SIT_OUT_SB;

		} else 	if (input_bits_left & 1<<ACT_SIT_OUT_POST) {

			sit_out_reason = ACT_SIT_OUT_POST;

		} else 	if (input_bits_left & 1<<ACT_SIT_OUT_BOTH) {

			sit_out_reason = ACT_SIT_OUT_BOTH;

		} else {

			sit_out_reason = ACT_SIT_OUT_POST;	// can't happen

		}

	}

	

	int last_button_used = 0;



  #if ADMIN_CLIENT && 0	//20010117MB

	// Help clients play a little better: show the fold button slightly

	// earlier if this is not a good hand.

	// *** not yet finished.  Currently just experimenting with what it looks like.

	// Still to be done: only do it for Hold'em and only do it if they don't

	// have a good hand.

	// Also only do it if FOLD is one of the options.

	if (t->GamePlayerInputRequest.input_request_serial_number != t->last_input_request_serial_number_displayed) {

		t->last_input_request_serial_number_displayed = t->GamePlayerInputRequest.input_request_serial_number;

		t->actions_hidden_ticks = GetTickCount() + 100;

	}

	if (t->actions_hidden_ticks && t->actions_hidden_ticks > GetTickCount()) {

		input_bits_left &= (1<<ACT_FOLD);

	} else {

		t->actions_hidden_ticks = 0;

	}

  #endif


	int act=0;
	for (int i=0 ; i<MAX_BUTTONS_PER_TABLE ; i++) {

		// Find the lowest number bit which is still set in input_bits_left.

		WORD32 mask = 0;

		for (int bit = 0 ; bit<32 ; bit++) {

			mask = 1<<bit;
			if (input_bits_left & mask) {

				break;

			}

		}

		
		if (input_bits_left & mask) {	// found something

			show_fold_in_turn_checkbox = FALSE;

			//kp(("%s(%d) Updating button %d for regular (not 'in turn') mode.\n",_FL, i));

			t->button_actions[i] = bit;

			char button_text[100], money_text[50];

			char money_text2[MAX_CURRENCY_STRING_LEN], money_text3[MAX_CURRENCY_STRING_LEN];

			money_text[0] = 0;

			// If this is call/bet/raise, append a dollar amount

			if (bit==ACT_CALL) {

				sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.call_amount, t->chip_type));

			} else if (bit==ACT_BET) {

				sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.bet_amount, t->chip_type));

			} else if (bit==ACT_BRING_IN) {

				// we hide the bring_in amount in the big_blind

				sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GameCommonData.big_blind_amount, t->chip_type));

			} else if (bit==ACT_RAISE) {
				act=ACT_RAISE;
				
				sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.raise_amount, t->chip_type));

			} else if (bit==ACT_CALL_ALL_IN) {

				sprintf(money_text, "\n (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.call_amount, t->chip_type));

			} else if (bit==ACT_BET_ALL_IN) {

				sprintf(money_text, "\n (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.bet_amount, t->chip_type));

			} else if (bit==ACT_RAISE_ALL_IN) {

				sprintf(money_text, "\n (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.raise_amount, t->chip_type));

				// 20000809HK: for SB and BB blinds posting, if we're fed a call amount, display that

			} else if (bit==ACT_POST_SB) {

				remove_wait_for_bb = TRUE;

				sprintf(money_text, " (%s)", 

					CurrencyString(money_text2, 

					(t->GamePlayerInputRequest.call_amount ? t->GamePlayerInputRequest.call_amount : t->GameCommonData.small_blind_amount), 

					t->chip_type));

			} else if (bit==ACT_POST_BB) {

				remove_wait_for_bb = TRUE;

				sprintf(money_text, " (%s)", 

					CurrencyString(money_text2, 

					(t->GamePlayerInputRequest.call_amount ? t->GamePlayerInputRequest.call_amount : t->GameCommonData.big_blind_amount),

					t->chip_type));

			} else if (bit==ACT_POST) {

				remove_wait_for_bb = TRUE;

				sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GameCommonData.big_blind_amount, t->chip_type));

			} else if (bit==ACT_POST_BOTH) {

				remove_wait_for_bb = TRUE;

				sprintf(money_text, " (%s + %s dead)",

						CurrencyString(money_text2, t->GameCommonData.big_blind_amount, t->chip_type),

						CurrencyString(money_text3, t->GameCommonData.small_blind_amount, t->chip_type));

				//20000728MB: try to make it fit the button better

				// " ($10 + $5 dead)" fits fine.

				// " ($20 + $10 dead)" is pretty tight

				// " ($0.50 + $0.25 dead)" is too wide.

				if (t->GameCommonData.big_blind_amount < 100) {

					// Display as " (.50+.25 dead)" (regardless of real/play chip type)

					sprintf(money_text, " (.%d+.%d dead)",

							t->GameCommonData.big_blind_amount,

							t->GameCommonData.small_blind_amount);

				}

			}

			HWND slider = GetDlgItem(t->hwnd, IDC_LIMIT);
			int tic = SendMessage(slider, TBM_GETPOS, 0, 0);
			if (tic <= 0){            
				//sprintf(money_text, " (%s)", CurrencyString(money_text2, t->GamePlayerInputRequest.raise_amount, t->chip_type));

				strcpy(button_text, ActionStrings[bit]);

				strcat(button_text, money_text);
	
				SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[i], button_text);

			}
			EnableWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), TRUE);

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), SW_SHOWNA);	// Make button visible

			last_button_used = i;
			

		} else {	// nothing left... disable button.

			if (!t->button_1_alternate_meaning || i) {

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), SW_HIDE);				

			}

		}

		input_bits_left &= ~mask;

	}

	//J Fonseca    29/01/2003
	if (act == 14){  //Se hizo "Raise"
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LIMIT), SW_SHOW);	
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_NEW_LIMIT), SW_SHOW);	
	}else{
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LIMIT), SW_HIDE);	
		HWND slider = GetDlgItem(t->hwnd, IDC_LIMIT);
		SendMessage(slider, TBM_SETPOS, 0, 0); // Reinicia el "Slider"
		SetDlgItemText(t->hwnd, IDC_NEW_LIMIT, ""); 
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_NEW_LIMIT), SW_HIDE);	
	}

	// 20010303HK: we want the order in 7cs to be [Show Hand] [Show Shuffled Hand] [Muck Hand] but since ShowShuffled was
	// put in after the other two, it's out of order in the ACT_* enumeration and as per the code above, will draw them
	// in that order rather than what we want.  Rather than messing with all the stuff above, or forcing a simultaneous
	// release of a client and server, this code will simply switch those two buttons if that's the situation.  if we ever
	// do a simltaneous release of a client/server, that enumeration in gamedata.h could be changed and the followin code
	// could be removed

	if (t->button_actions[2] == ACT_SHOW_SHUFFLED && t->button_actions[1] == ACT_MUCK_HAND) {	// swap

		// right to left

		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[2], ActionStrings[ACT_MUCK_HAND]);

		t->button_actions[2] = ACT_MUCK_HAND;

		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[1], ActionStrings[ACT_SHOW_SHUFFLED]);

		t->button_actions[1] = ACT_SHOW_SHUFFLED;

	}



	

	if (input_bits_left) {

		Error(ERR_INTERNAL_ERROR, "%s(%d) inputs bits still left after setting buttons. action_mask=$%08lx", _FL, t->GamePlayerInputRequest.action_mask);

	}

	// 20000626HK: wait_for_bb button is spoofed here

	if (valid_wait_for_bb && (last_button_used+1 < MAX_BUTTONS_PER_TABLE) ) { // put one if there's room

		int spoof_button_index = last_button_used+1;

		// we'll use the valid sit-out reason figured out above

		t->button_actions[spoof_button_index] = sit_out_reason;

		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[spoof_button_index], "Wait for\nBig Blind");

		EnableWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[spoof_button_index]), TRUE);

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[spoof_button_index]), SW_SHOWNA);	// Make button visible

		//ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TAB), SW_HIDE);
	}

	if (remove_wait_for_bb) {

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, _IDC_WAIT_FOR_BB), SW_HIDE);	// Make button invisible

		t->wait_for_bb = FALSE;

	}

  #if 1	//20000908MB

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_SIT_OUT), SW_SHOWNA);

  #else

	// 20000811HK: added flag so it is never shown on a tournament table

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_SIT_OUT), t->tournament_table ? SW_HIDE : SW_SHOWNA);

  #endif

  #if ADMIN_CLIENT



	if ((iAdminClientFlag) &&(LoggedInPrivLevel >= 40 ) ) {

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY), SW_SHOWNA);

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY_SECS), SW_SHOWNA);

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY_SECS_SPIN), SW_SHOWNA);

	  #if MIN_HAND_RANK_POPUP

		// only for tourney tables as this makes no sense on regular tables

		if (t->tournament_table) {

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK), SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK_EDIT), SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK_SPIN), SW_SHOWNA);

		}

	  #endif

	}

    



  #endif

  #if 0	//20000616MB: what the hell was this for?  I think it's been unused for ages.

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_ACTIVATE_CHAT), SW_SHOWNA);

  #endif



	// Enable/disable the IDC_POST_IN_TURN checkbox depending on whether

	// we're sitting out or not.

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POST_IN_TURN),

				(t->ClientState.sitting_out_flag ||

				 t->zero_ante_stud_flag ||

				 t->wait_for_bb) ? SW_HIDE : SW_SHOWNA);



	// We're actually playing at this table...

	// Enable/disable the IDC_FOLD_IN_TURN/IDC_MUCK_LOSING_HANDS checkbox

	// depending on whether we're currently playing.

	// If game over, treat as if we're sitting out.

	int our_status = t->GamePlayerData.player_status[t->GamePlayerData.seating_position];

	if (t->GamePlayerData.s_gameover) {

		our_status = PLAYER_STATUS_SITTING_OUT;

	}

	if (our_status==PLAYER_STATUS_PLAYING || our_status==PLAYER_STATUS_ALL_IN) {

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MUCK_LOSING_HANDS), SW_SHOWNA);

	} else {

		ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MUCK_LOSING_HANDS), SW_HIDE);

		show_fold_in_turn_checkbox = FALSE;

	}



	// If it doesn't make sense for the fold in turn checkbox to be displayed

	// in the current game state, turn it off.

	if (t->GamePlayerData.game_state < DEAL_POCKETS ||

		t->GamePlayerData.game_state >= END_GAME_START)

	{

		show_fold_in_turn_checkbox = FALSE;

	}



	// Update the state and visibility of the fold in turn checkbox.
	if (!t->ClientState.in_turn_action) {	// if nothing selected, make sure it's unchecked
		CheckDlgButtonIfNecessary(t->hwnd, IDC_FOLD_IN_TURN2, 0);
	}

//	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_FOLD_IN_TURN2), show_fold_in_turn_checkbox ? SW_SHOWNA : SW_HIDE);
}



//*********************************************************

//
//

// Update any buttons on our screen during watching

//

void UpdateWatchingButtons(struct TableInfo *t)

{

	pr(("%s(%d) UpdateWatchingButtons() has been called\n",_FL));
  	int i=0;	// always start with button 0.


	// If we must join a waiting list to get on this table, display
	// that button now.
	if (t->table_serial_number && t->watching_flag && !t->sit_down_allowed) {
		t->button_actions[i] = ACT_NO_ACTION;
		struct CardRoom_TableSummaryInfo tsi;
		FindTableSummaryInfo(t->table_serial_number, &tsi);
		if (tsi.user_waiting_list_pos) {	// are we already on the waiting list?
			t->button_1_alternate_meaning = 2;	// 0=none, 1=Join Waiting list, 1=Unjoin Waiting List
			SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[i], "Unjoin wait list");
		} else {
			t->button_1_alternate_meaning = 1;	// 0=none, 1=Join Waiting list, 1=Unjoin Waiting List
			SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[i], "Waiting List");
		} //if (tsi.user_waiting_list_pos) {	// are we already on the waiting list?
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), SW_SHOWNA);	// Make button visible
		
		//ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TAB), SW_HIDE);
		i++;
	} ;//if (t->table_serial_number && t->watching_flag && !t->sit_down_allowed)



	for ( ; i<MAX_BUTTONS_PER_TABLE ; i++) {
		t->button_actions[i] = ACT_NO_ACTION;
		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[i], "");
        ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), SW_HIDE);	// Make button invisible		
	}//for



	for (int j=0; InTurnCheckboxIDs[j] ; j++) {
		ShowWindowIfNecessary(GetDlgItem(t->hwnd, InTurnCheckboxIDs[j]), SW_HIDE);			
	}//for

	//ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_FOLD_IN_TURN2), SW_HIDE);	// special case... not in InTurnCheckboxIDs[] array
	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_SIT_OUT), SW_HIDE);		
 #if ADMIN_CLIENT
	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY), SW_HIDE);
	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY_SECS), SW_HIDE);
	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY_SECS_SPIN), SW_HIDE);
	#if MIN_HAND_RANK_POPUP
		 ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK), SW_HIDE);
		 ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK_EDIT), SW_HIDE);
		 ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MIN_HAND_RANK_SPIN), SW_HIDE);
	#endif //MIN_HAND_RANK_POPUP
 #endif //ADMIN_CLIENT

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_MUCK_LOSING_HANDS), SW_HIDE);

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POST_IN_TURN), SW_HIDE);

};//void UpdateWatchingButtons



//*********************************************************

//
//

// Calculate and return the number of chips for a player's

// current stake.  To be displayed on the player id boxes

// or the hovertext.

// Returns -1 if we can't be absolutely sure of the value.

//

int GetStakeForPlayer(struct TableInfo *t, int player_index)

{

	if (t->GameCommonData.game_serial_number!=t->GamePlayerData.game_serial_number) {
		return -1;	// we can't be sure.
	}



  #if 0	//20000914MB
	if (t->sequential_index_gcd >= t->sequential_index_gpd) {
		// gcd is more recent than gpd.  Stakes are probably invalid.
		WORD32 ticks = GetTickCount();
		kp(("%s %s(%d) GCD is more recent than GPD. Blanking stakes. Game=%d. Blanking. gcd age=%dms, gpd age=%dms\n",
				TimeStr(), _FL,
				t->GameCommonData.game_serial_number,
				ticks - t->ticks_when_gcd_arrived,
				ticks - t->ticks_when_gpd_arrived));
		return -1;	// we can't be sure.
	};//if (t->sequential_index_gcd >= t->sequential_index_gpd)
  #endif

	int chips = t->GameCommonData.chips[player_index];

	// Subtract how much they've bet so far this round chips_bet_total.

	chips -= t->GamePlayerData.chips_bet_total[player_index];

	if (chips < 0) {
	  #if DEBUG
		WORD32 ticks = GetTickCount();
		kp(("%s %s(%d) Warning: calculated a negative chips amount (%d - %d = %d) for player %d. Game=%d. Blanking. gcd age=%dms, gpd age=%dms\n",
				TimeStr(), _FL,
				t->GameCommonData.chips[player_index],
				t->GamePlayerData.chips_bet_total[player_index],
				chips,
				player_index,
				t->GameCommonData.game_serial_number,
				ticks - t->ticks_when_gcd_arrived,
				ticks - t->ticks_when_gpd_arrived));
	  #endif
		return -1;	// we can't be sure.
	};//	if (chips < 0)
	return chips;
};//int GetStakeForPlayer




//*********************************************************
////
// Set the text for a player ID box
//

void UpdatePlayerIDBoxText(struct TableInfo *t, int player_index)

{
	 

	if (t->iLastActionStatus[player_index]) {

		// show last action instead...

		char *last_action_strings[ACT_ACTION_TYPE_COUNT] =  {

				NULL,			// ACT_NO_ACTION,		//  0 no action

				"Post",			// ACT_POST,			//  1 (used for new players - usually BB amount)

				"Post SB",		// ACT_POST_SB,			//  2 player posts the small blind

				"Post BB",		// ACT_POST_BB,			//  3 player posts the big blind

				"Post Blinds", 	// ACT_POST_BOTH,		//  4 (used for missed blinds, SB is dead).

				"Sit out",		// ACT_SIT_OUT_SB,		//  5 player didn't want to post SB

				"Sit out", 		// ACT_SIT_OUT_BB,		//  6 player didn't want to post BB

				"Sit out", 		// ACT_SIT_OUT_POST,	//  7 player didn't want to post initial post

				"Sit out", 		// ACT_SIT_OUT_BOTH,	//  8 player didn't want to post both

				"Fold",			// ACT_FOLD,			//  9 player folds

				"Call",			// ACT_CALL,			// 10 player calls

				"Check",		// ACT_CHECK,			// 11 player checks

				"Bring in",		// ACT_BRING_IN,		// 12 player brings in action low

				"Bet",			// ACT_BET,				// 13 player bets

				"Raise",		// ACT_RAISE,			// 14 player raises

				"Bet all-in",	// ACT_BET_ALL_IN,		// 15 player bets all in

				"Call all-in",	// ACT_CALL_ALL_IN,		// 16 player calls all in

				"Raise all-in",	// ACT_RAISE_ALL_IN,	// 17 players raises all in

				NULL,			// ACT_SHOW_HAND,		// 18 at the end of a game, player shows hand

				NULL,			// ACT_TOSS_HAND,		// 19 after winning the hand, player doesn't show

				"Muck",			// ACT_MUCK_HAND,		// 20 after losing the hand, player doesn't show

				"Ante",			// ACT_POST_ANTE,		// 21 player posts an ante

				"Sit out",		// ACT_SIT_OUT_ANTE,	// 22 player doesn't post an ante

		}; //last_action_strings definition

		int action = t->GamePlayerData.last_action[player_index];

		if (action) {

			char *s = NULL;

			if (action < ACT_ACTION_TYPE_COUNT) {

				s = last_action_strings[action];

			};//if (action < ACT_ACTION_TYPE_COUNT)

			if (s) {
				SetDlgItemTextIfNecessary(t->hwnd, PlayerTextIDs[player_index], s);
				
			} else {
				// No text for this action.  Revert to normal
				t->iLastActionStatus[player_index] = 0;
			};//if s

			

		};//if action
	


	};//if (t->iLastActionStatus[player_index])



	if (!t->iLastActionStatus[player_index]) {
		char str[200];
		
		if (t->GamePlayerData.player_status[player_index]==PLAYER_STATUS_SITTING_OUT ||
			t->GamePlayerData.player_status[player_index]==PLAYER_STATUS_NOT_ENOUGH_MONEY) {
			strcpy(str,"");

			sprintf(str, "%s\n(sitting out)", t->GameCommonData.name[player_index]);
		} else {
			// normal... name and stake
			char cs[MAX_CURRENCY_STRING_LEN];
			// 20000812HK: possibly override chip type for display purposes
			ChipType chip_type_to_display = (t->GameCommonData.flags & GCDF_USE_REAL_MONEY_CHIPS ? CT_REAL : t->chip_type);
			int stake = GetStakeForPlayer(t, player_index);
			if (stake >= 0) {
				sprintf(str, "%s\n%s", t->GameCommonData.name[player_index],CurrencyString(cs, stake, chip_type_to_display, FALSE, -1));
			} else {
				sprintf(str, "%s", t->GameCommonData.name[player_index]);			
			};//if (stake >= 0)
		};//if (t->GamePlayerData.player.....

		
	
		SetDlgItemTextIfNecessary(t->hwnd, PlayerTextIDs[player_index], str);

		
	};//if (!t->iLastActionStatus[player_index])

    
};//UpdatePlayerIDBoxText



//*********************************************************

//
//

// Set the last action status indicator state.

//

void SetLastActionStatus(struct TableInfo *t, int status, int player_index)

{

	t->iLastActionStatus[player_index] = status;

	UpdatePlayerIDBoxText(t, player_index);

}



//*********************************************************

//
//

// Update the last action indicators for each player if necessary

//

void UpdateLastActionIndicator(struct TableInfo *t)

{

	DWORD ticks = GetTickCount();

	if (ticks - t->last_action_update_ticks > 100) {

		// Time to update.

		t->last_action_update_ticks = ticks;

		for (int i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			if (t->iLastActionStatus[i]) {

				t->iLastActionStatus[i]--;

				if (!t->iLastActionStatus[i]) {

					// done animating... put back to normal

					UpdatePlayerIDBoxText(t, i);

				} else {

					// Make sure it gets redrawn in the new status

			        if (!t->animation_disable_flag) {    // only update if not minimized

						InvalidateRect(GetDlgItem(t->hwnd, PlayerTextIDs[i]), NULL, FALSE);

					}

				}

			}

		}

	}

}



//*********************************************************

//
//

// Invalidate all the player id boxes on a table (so they get redrawn)

//

void InvalidatePlayerIDBoxes(struct TableInfo *t)

{

	for (int i=0 ; i<t->max_players_per_table ; i++) {

		HWND hwnd = GetDlgItem(t->hwnd, PlayerTextIDs[i]);

		if (hwnd) {

			InvalidateRect(hwnd, NULL, FALSE);

		}

	}

	t->invalidate_player_id_boxes = FALSE;

}



//*********************************************************

//
//

// Set the title bar for a table window

//

void UpdateTableWindowTitle(struct TableInfo *t)

{

	if (t->hwnd) {

		char str[200];

		char cs[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];



		// cosmetics for game title; has to do with sb/bb labels in Stud7

		int display_factor = GameStakesMultipliers[t->game_rules - GAME_RULES_START];



		switch (t->chip_type) {

			case CT_NONE:

				Error(ERR_INTERNAL_ERROR,"%s(%d) called with CT_NONE", _FL);

				break;

			case CT_PLAY:

				sprintf(str, "Play Money %s/%s %s - Table %s",

					CurrencyString(cs, t->GameCommonData.big_blind_amount*display_factor, t->chip_type),

					CurrencyString(cs2, t->GameCommonData.big_blind_amount*display_factor*2, t->chip_type),

					GameRuleNames[t->game_rules - GAME_RULES_START],

					TableNameFromSerialNumber(t->table_serial_number));

				break;

			case CT_REAL:

				sprintf(str, "%s/%s %s - Table \"%s\"",

					CurrencyString(cs, t->GameCommonData.big_blind_amount*display_factor, t->chip_type),

					CurrencyString(cs2, t->GameCommonData.big_blind_amount*display_factor*2, t->chip_type),

					GameRuleNames[t->game_rules - GAME_RULES_START],

					TableNameFromSerialNumber(t->table_serial_number));

				break;

			case CT_TOURNAMENT:

				{

					struct CardRoom_TableSummaryInfo tsi;

					zstruct(tsi);

					FindTableSummaryInfo(t->table_serial_number, &tsi);

					sprintf(str, "%s %s Tournament - Table \"%s\"",

						CurrencyString(cs, tsi.big_blind_amount, CT_REAL),

						GameRuleNames[t->game_rules - GAME_RULES_START],

						TableNameFromSerialNumber(t->table_serial_number));

				}

				break;

			default:

				Error(ERR_INTERNAL_ERROR,"%s(%d) called with unknown chip_type", _FL);

				break;

		}

		strcat(str, " ");

		//strcat(str, ConnectionStatusString);

	  #if ADMIN_CLIENT	//19990823MB

	  	if (iAdminClientFlag) {

			if (LoggedIn == LOGIN_VALID) {

				char login_info[50];

				sprintf(login_info," -- logged in as %s", LoginUserID);

				strcat(str,login_info);

			} else {

				strcat(str," -- not logged in");

			}

		}

	  #endif



		SetWindowTextIfNecessary(t->hwnd, str);

	}

}



//*********************************************************

//
//

// Use the elapsed ticks since it became a new player's turn to

// determine whether to highlight a player during his turn or not.

// Returns TRUE or FALSE for whether it should be highlighted.

//

int DetermineCurrentPlayerHighlightState(struct TableInfo *t)

{

	// First, check if the current highlight has changed.

	if (t->GamePlayerData.p_waiting_player != t->last_highlighted_waiting_player) {

		// New player getting highlighted.

		t->highlighted_waiting_player_start_ticks = GetTickCount();

		t->last_highlighted_waiting_player = t->GamePlayerData.p_waiting_player;

	}

	WORD32 elapsed = GetTickCount() - t->highlighted_waiting_player_start_ticks;

	int highlighted = ((elapsed / 450) + 1) % 6 ? TRUE : FALSE;

	return highlighted;

}



//****************************************************************

//
//

// Redraw a table given all its current information in the Table[] array

// Note that this is a higher level than actually drawing the graphics.

// This function WILL call RedrawAllTableGraphics() to do the low level

// stuff and start animating anything necessary.

// If all you want to do is update the animated graphics (because the

// text and controls have not changed), just call RedrawAllTableGraphics().

// This function does NOT actually paint the background, it just updates

// the offscreen bitmap.

//

void RedrawTable(int table_index)

{

	char str[200];

	pr(("%s(%d) RedrawTable() has been called\n",_FL));

	struct TableInfo *t = &Table[table_index];

	t->redraw_needed = FALSE;	// always clear at top of function.

	t->tournament_table = (t->GameCommonData.flags & GCDF_TOURNAMENT);

	t->chip_type = (t->GameCommonData.flags & GCDF_REAL_MONEY) ? CT_REAL : CT_PLAY;

//	t->GamePlayerData.seating_position = 

	/*

	if (t->GameCommonData.flags == GCDF_REAL_MONEY ||

		t->GameCommonData.flags == GCDF_TOURNAMENT

		)

	{

        t->chip_type = CT_REAL;

	} 

	else if ( t->GameCommonData.flags == GCDF_WAIT_LIST_REQUIRED ||

		t->GameCommonData.flags ==0 ) 

	{

	  t->chip_type = CT_NONE;

	} else 

	{

	    t->chip_type = CT_PLAY;

	}

    */

	if (t->GameCommonData.flags & GCDF_TOURNAMENT) {

		t->chip_type = CT_TOURNAMENT;

	}

	if ((t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) &&

		!t->GameCommonData.small_blind_amount)

	{

		// It's a zero ante stud game.  Auto-ante should be hidden

		t->zero_ante_stud_flag = TRUE;

	} else {

		t->zero_ante_stud_flag = FALSE;

	}

	char cs[MAX_CURRENCY_STRING_LEN];

	int new_game_flag = FALSE;



	if (t->hwnd) {	// If the window exists...

		// Set window title bar

		UpdateTableWindowTitle(t);



		// Choose this table as our 'preferred stakes' and 'preferred game type'.

		Defaults.changed_flag = TRUE;

		Defaults.preferred_display_tab_index = t->client_display_tab_index;

		Defaults.preferred_stakes = t->GameCommonData.big_blind_amount;



		if (t->game_serial_number != t->GameCommonData.game_serial_number) {

			// This is a new game... reset a few things.

			t->prev_game_serial_number = t->game_serial_number;

			t->game_serial_number = t->GameCommonData.game_serial_number;

			new_game_flag = TRUE;

			RefreshChatText(table_index);

		}

	   #if 0	//19991019MB: no longer needed here

		// If we've got an in turn action set and the game state has changed,

		// clear it so we don't carry in-turn actions across betting rounds.

		if (new_game_flag

		 || (t->ClientState.in_turn_action && t->GamePlayerData.game_state != t->ClientState.in_turn_action_game_state)

		) {

			t->button_actions_are_in_turn = FALSE;

			ClearInTurnActions(t);

		}

	   #endif



		// check if we're sitting out and set the checkbox appropriately

	  #if 0	//20000908MB

		if (t->tournament_table) {

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_SIT_OUT), SW_HIDE);

		} else

	  #endif

		{

			CheckDlgButtonIfNecessary(t->hwnd, IDC_SIT_OUT, t->ClientState.sitting_out_flag);

		}

		

		// If there is no waiting list for this table, we can sit down any time.

		char *empty_seat_tooltip = "Empty Seat";

		if (t->GameCommonData.table_serial_number) {

			if (t->GameCommonData.flags & GCDF_WAIT_LIST_REQUIRED) {

				// If we're not on the waiting list, we can't sit down.

				struct CardRoom_TableSummaryInfo tsi;

				FindTableSummaryInfo(t->table_serial_number, &tsi);

				if (SeatAvail.table_serial_number != t->GameCommonData.table_serial_number) {

					//kp(("%s(%d) We haven't been called to this table... sit_down_allowed should be FALSE.\n",_FL));

					t->sit_down_allowed = FALSE;	// we're not allowed to sit down.

					empty_seat_tooltip = "Reserved by waiting list";

				}

			} else {

				// 20000811HK: change a few things if this is a tournament table (like hide buttons)

				if (t->GameCommonData.flags & GCDF_TOURNAMENT_HAS_STARTED) {

					t->sit_down_allowed = FALSE;

					empty_seat_tooltip = "Player busted out";

					struct CardRoom_TableSummaryInfo tsi;

					zstruct(tsi);

					FindTableSummaryInfo(t->table_serial_number, &tsi);

					if (tsi.tournament_state==TOURN_STATE_FINISHED) {

						empty_seat_tooltip = "";	// blank out when tournament over.

					}

				} else {

					// If this is a tournament table and we're seated elsewhere at

					// a different tournament, we're not allowed to sit down.

					if ((t->GameCommonData.flags & GCDF_TOURNAMENT) && iSeatedAtATournamentTableFlag) {

						t->sit_down_allowed = FALSE;

					} else {

						t->sit_down_allowed = TRUE;	// we can sit down in any empty seat.

					}

				}

			}

		}

		// update the chat input box

		if (LoggedIn == LOGIN_VALID) {

			if (!IsWindowEnabled(GetDlgItem(t->hwnd, IDC_TYPE_CHAT))) {

				EnableWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TYPE_CHAT), TRUE);

				SetDlgItemTextIfNecessary(t->hwnd, IDC_TYPE_CHAT, "");

			}

			if (!t->displayed_chat_instruction) {

				t->displayed_chat_instruction = TRUE;

				SetDlgItemTextIfNecessary(t->hwnd, IDC_TYPE_CHAT, "Type chat text here...");

			}

		} else {

			if (IsWindowEnabled(GetDlgItem(t->hwnd, IDC_TYPE_CHAT))) {

				SetDlgItemTextIfNecessary(t->hwnd, IDC_TYPE_CHAT, "You must be logged in to chat...");

				EnableWindowIfNecessary(GetDlgItem(t->hwnd, IDC_TYPE_CHAT), FALSE);

			}

		}



	  #if 0	//19990514MB

		kp1(("%s(%d) Forcing sit_down_allowed = TRUE all the time (for testing)\n",_FL));

		t->sit_down_allowed = TRUE;	// we can sit down in any empty seat.

	  #endif



		t->blinking_player_flag = FALSE;		// default to not blinking a player id box

		for (int i=0 ; i<t->max_players_per_table ; i++) {

			if (t->GameCommonData.player_id[i] || t->GameCommonData.name[i][0]) {

				// Somebody is sitting in this seat (possibly a computer player)

				int highlight_player = FALSE;

				if (t->GamePlayerData.s_gameover) {

					// Game is over... highlight any winners

					if (t->GamePlayerData.chips_won[i] > 0) {

						highlight_player = TRUE;

					}

				} else if (t->GamePlayerData.game_serial_number && !t->dealing_flag) {

					// game in progress... highlight current player

					if (i==t->GamePlayerData.p_waiting_player) {

						highlight_player = DetermineCurrentPlayerHighlightState(t);

						t->blinking_player_status = (BYTE8)highlight_player;

						t->blinking_player_flag = TRUE;

					}

				}



				// Test if we should initiate a last action indicator...

				if (t->GamePlayerData.last_action[i]==ACT_NO_ACTION) {

					// Never animate ACT_NO_ACTION

					t->last_action[i] = t->GamePlayerData.last_action[i];

				}

				if (t->last_action[i] != t->GamePlayerData.last_action[i]) {

					// Spoof the dealer text that used to be sent from the server...

					if (t->GamePlayerData.last_action[i] < ACT_ACTION_TYPE_COUNT) {

						char *last_action_strings[ACT_ACTION_TYPE_COUNT] =  {

							NULL,				// ACT_NO_ACTION,		//  0 no action

							"posts",			// ACT_POST,			//  1 (used for new players - usually BB amount)

							"posts the small blind",// ACT_POST_SB,			//  2 player posts the small blind

							"posts the big blind",	// ACT_POST_BB,			//  3 player posts the big blind

							"posts both blinds",// ACT_POST_BOTH,		//  4 (used for missed blinds, SB is dead).

							"sits out",			// ACT_SIT_OUT_SB,		//  5 player didn't want to post SB

							"sits out", 		// ACT_SIT_OUT_BB,		//  6 player didn't want to post BB

							"sits out", 		// ACT_SIT_OUT_POST,	//  7 player didn't want to post initial post

							"sits out", 		// ACT_SIT_OUT_BOTH,	//  8 player didn't want to post both

							"folds",			// ACT_FOLD,			//  9 player folds

							"calls",			// ACT_CALL,			// 10 player calls

							"checks",			// ACT_CHECK,			// 11 player checks

							NULL,				// ACT_BRING_IN,		// 12 player brings in action (low or high)

							"bets",				// ACT_BET,				// 13 player bets

							"raises",			// ACT_RAISE,			// 14 player raises

							"bets all-in",		// ACT_BET_ALL_IN,		// 15 player bets all in

							"calls all-in",		// ACT_CALL_ALL_IN,		// 16 player calls all in

							"raises all-in",	// ACT_RAISE_ALL_IN,	// 17 players raises all in

							NULL,				// ACT_SHOW_HAND,		// 18 at the end of a game, player shows hand

							NULL,				// ACT_TOSS_HAND,		// 19 after winning the hand, player doesn't show

							"mucks",			// ACT_MUCK_HAND,		// 20 after losing the hand, player doesn't show

							"posts the ante",	// ACT_POST_ANTE,		// 21 player posts an ante

							"sits out",			// ACT_SIT_OUT_ANTE,	// 22 player doesn't post an ante

						};

						char *s = last_action_strings[t->GamePlayerData.last_action[i]];

						if (s) {

						  #if 1	//19991209MB

							AddDealerMessage(t, CHATTEXT_DEALER_BLAB, "%s %s", t->GameCommonData.name[i], s);

						  #else

							AddDealerMessage(t, CHATTEXT_DEALER_BLAB, "%s %s <---new from client", t->GameCommonData.name[i], s);

						  #endif

						}

					}



					SetLastActionStatus(t, 16, i);	// start a new indicator.
				
					

					t->last_action[i] = t->GamePlayerData.last_action[i];

					// If they just checked, play a sound effect.

					if (t->last_action[i]==ACT_CHECK && t->active_table_window && !Defaults.iSoundsDisabled) {

						PPlaySound(SOUND_CHECK);

					}

				}


				UpdatePlayerIDBoxText(t, i);

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_HIDE);	// Make button invisible
		
				// If a card is available to be shown from this seat, don't

				// display the player name control.

				if (t->game_rules != GAME_RULES_STUD7 &&

					t->game_rules != GAME_RULES_STUD7_HI_LO &&

					t->GamePlayerData.cards[i][0]!=CARD_HIDDEN && 

					t->GamePlayerData.cards[i][0]!=CARD_NO_CARD) {

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

				} else {

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_SHOWNA);		// Make text   visible

				}



				// Retrieve the rect for that box and set the tool tip for it.

				if (t->GameCommonData.city[i][0]) {

					sprintf(str, "%s (%s)", t->GameCommonData.name[i], t->GameCommonData.city[i]);

				} else {

					sprintf(str, "%s", t->GameCommonData.name[i]);

				}

				int stake = GetStakeForPlayer(t, i);

				if (stake >= 0) {

					CurrencyString(cs, stake, t->chip_type);

					strcat(str, " ");

					strcat(str, cs);

				}



				// If they're disconnected, add 'disconnected' to the string...

				if (t->GamePlayerData.disconnected_flags & (1<<i)) {

				///	strcat(str, " (disconnected)");

				} else if (t->GamePlayerData.player_status[i]==PLAYER_STATUS_SITTING_OUT ||

						   t->GamePlayerData.player_status[i]==PLAYER_STATUS_NOT_ENOUGH_MONEY ||

						   t->GamePlayerData.player_status[i]==PLAYER_STATUS_EMPTY) {

					// they're sitting out, add 'sitting out'...

					strcat(str, " (sitting out)");

				}

				SetToolTipText(t->hwnd, PlayerHoverIDs[i], str, &t->player_info_tooltips_initialized[i]);

				SetToolTipText(t->hwnd, PlayerTextIDs[i], str, &t->player_id_tooltips_initialized[i]);
              



				// Enable/disable the control depending on whether it should

				// be highlighted.

				EnableWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), highlight_player);



			} else {

				/* allen 4-16-2001 */

				struct CardRoom_TableSummaryInfo tsi;

				zstruct(tsi);

				FindTableSummaryInfo(t->table_serial_number, &tsi);

				if (t->table_serial_number != tsi.table_serial_number) {

					kp(("%s(%d) Warning: got table summary info for wrong table! (%d vs %d)\n",

								_FL, t->table_serial_number, tsi.table_serial_number));

				}

					//zstruct(tsi);

					//str[0] = 0;

					char * pdest;

					bool five_max;

					pdest = strstr( tsi.table_name, "(5 Max)" );

					{

					   if( pdest != NULL )

						   five_max=true;

					   else

						   five_max=false;

					}





				/* allen 4-16-2001 */



				// Somebody is NOT sitting in this seat (it's empty)

				// If we're watching this table and we're allowed to sit down,

				// display the 'sit down' buttons.

				// 990622HK:  exception, since we're using the same hold'em layout as for heads-up,

				// we only want two active seats available -- seats 3 and 8.  the rest should be disabled

				//20000727MB: two different sets of heads up seats, depending on whether

				// we're using the 10 seat layout or the 8 seat layout.

				if ((t->GameCommonData.flags & GCDF_ONE_ON_ONE) && 

						i != t->heads_up_seat_1 && 

						i != t->heads_up_seat_2)

				{

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_HIDE);	// Make button invisible

				} else if (!t->GameCommonData.table_serial_number) {

					// No data has arrived for this table yet.

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_HIDE);	// Make button invisible

				} else if (t->watching_flag && !t->sit_down_allowed) {

					// We cannot join directly... we must use the waiting list.

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_HIDE);	// Make button invisible

				  #if 0	// 20000626HK: if we have a reserved button, make the seat totally empty

					SetDlgItemTextIfNecessary(t->hwnd, PlayerTextIDs[i], "Reserved");

					EnableWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), FALSE);

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_SHOWNA);	// Make text   visible

				  #else

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

				  #endif

				} else if (t->watching_flag && t->sit_down_allowed) {

					// We can sit down ourselves.  Display a 'sit down' button.

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

					SetDlgItemTextIfNecessary(t->hwnd, PlayerTextIDs[i], "");





					//Modified by Allen Ko 4-16-2001

//					ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_SHOWNA);	// Make button visible

					if (five_max == false) {
						ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_SHOWNA);	// Make button visible
					}

					else

					{

						int tmp = fmod(i,2);



						if (tmp ==0){

	                      ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_SHOWNA);	// Make button visible											  
						}

					}

					//Modified by Allen Ko 4-16-2001

				} else {

					// We're already playing, display the seat as being empty.

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), SW_HIDE);	// Make button invisible

					ShowWindowIfNecessary(GetDlgItem(t->hwnd, PlayerTextIDs[i]), SW_HIDE);		// Make text   invisible

 
				}



				// Retrieve the rect for that box and set the tool tip for it.


				SetToolTipText(t->hwnd, PlayerHoverIDs[i], empty_seat_tooltip, &t->player_info_tooltips_initialized[i]);

				SetToolTipText(t->hwnd, PlayerTextIDs[i],  empty_seat_tooltip, &t->player_id_tooltips_initialized[i]);

			}

		}



		// Fill in the pot tooltips

		sprintf(str, "%s", CurrencyString(cs, t->GamePlayerData.pot[0], t->chip_type));
		/*********************** J Fonseca   13/11/2003 ***********************/
		//Aqui es donde se calcula el Pot

		HDC hdc;

		if (t->GamePlayerData.pot[0] <= 0){
			sprintf(str, "");
			MoveWindow(GetDlgItem(t->hwnd, IDC_POTS),10,10,10,10,0);
			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POTS), SW_HIDE);	
		}else{						
			MoveWindow(GetDlgItem(t->hwnd, IDC_POTS),380,250,40,20,0);			
			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POTS), SW_RESTORE);	
		}
		SetDlgItemTextIfNecessary(t->hwnd, IDC_POTS, str);	
		


		sprintf(str, "%s", CurrencyString(cs, t->GamePlayerData.rake_total, t->chip_type));		
		if (t->GamePlayerData.rake_total <= 0) sprintf(str, "");
		SetDlgItemTextIfNecessary(t->hwnd, IDC_RAKE, str);	
			

		/*********************** J Fonseca   13/11/2003 ***********************/
		
		SetToolTipText(t->hwnd, IDC_MAIN_POT_HOVER,

				t->GamePlayerData.pot[0] ? str : NULL,

				&t->pot_tooltips_initialized[0]);

		

		sprintf(str, "Side pot: %s", CurrencyString(cs, t->GamePlayerData.pot[1], t->chip_type));

		SetToolTipText(t->hwnd, IDC_SIDE_POT1_HOVER,

				t->GamePlayerData.pot[1] ? str : NULL,

				&t->pot_tooltips_initialized[1]);

		
		


		// Total up all money in all remaining pots and display it all here in the 3rd pot.

		int pot_total = 0;

		for (i=2 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			pot_total += t->GamePlayerData.pot[i];

		}

		sprintf(str, "Side pot: %s", CurrencyString(cs, pot_total, t->chip_type));

		SetToolTipText(t->hwnd, IDC_SIDE_POT2_HOVER,

				pot_total ? str : NULL,

				&t->pot_tooltips_initialized[2]);



	  #if DEBUG && 0

		struct CardRoom_TableSummaryInfo tsi;

		zstruct(tsi);

		FindTableSummaryInfo(t->table_serial_number, &tsi, NULL);

		kp(("%s(%d) table %d: wait=%d, pos=%d, sit_down_allowed=%d\n",

				_FL, tsi.table_serial_number, tsi.waiting_list_length, tsi.user_waiting_list_pos, t->sit_down_allowed));

	  #endif



		// Show/hide the snack menu icon according to whether we're allowed to bring

		// it up right now or not.

		int hide_it = (t->watching_flag || CheckIfPlayingCurrentGame(t) || Defaults.iBarSnacksDisabled);

		HWND h = GetDlgItem(t->hwnd, IDC_SNACK_MENU);

		int visible = IsWindowVisible(h);

		if (hide_it && visible) {

			//kp(("%s(%d) hiding snack menu icon\n", _FL));

			ShowWindow(h, SW_HIDE);

		} else if (!hide_it && !visible) {

			//kp(("%s(%d) showing snack menu icon\n", _FL));

			ShowWindow(h, SW_SHOWNA);

//***change 11/09/01 for remove the snack.

		}



		//kp(("%s(%d) iConnectionStatus = %d\n", _FL, iConnectionStatus));

		if (iConnectionStatus != CONNSTAT_CONNECTED) {

			// Disconnected.  Hide all buttons.

			ClearInTurnActions(t);

			for (int i=0 ; i<MAX_BUTTONS_PER_TABLE ; i++) {

				t->button_actions[i] = ACT_NO_ACTION;

				SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[i], "");

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, ButtonIDs[i]), SW_HIDE);	// Make button invisible
				
			}

		//	SetWaitListStatusString(t, "*** TRYING TO RE-CONNECT TO SERVER ***");
				SetWaitListStatusString(t, " Connection  Delay detected");

		} else if (t->watching_flag) {

			UpdateWatchingButtons(t);

			// If there's a waiting list, update our position on it.

			if (!t->GameCommonData.table_serial_number)  {

				// No data has been received for this table yet.

				SetWaitListStatusString(t, NULL);	// Hide waiting list status text

			} else if (t->sit_down_allowed) {

				SetWaitListStatusString(t, "To start playing, please select a seat.");

			} else {

				struct CardRoom_TableSummaryInfo tsi;

				zstruct(tsi);

				FindTableSummaryInfo(t->table_serial_number, &tsi);

				
				if (t->table_serial_number != tsi.table_serial_number) {

					kp(("%s(%d) Warning: got table summary info for wrong table! (%d vs %d)\n",

								_FL, t->table_serial_number, tsi.table_serial_number));

					zstruct(tsi);

					str[0] = 0;

				} else {

					if (t->chip_type==CT_TOURNAMENT) {

						char buy_in_str[MAX_CURRENCY_STRING_LEN];

						CurrencyString(buy_in_str, tsi.big_blind_amount, CT_REAL);

						if (tsi.waiting_list_length==0) {

							sprintf(str, "The waiting list for tournament table %s is empty.", buy_in_str);

						} else if (tsi.waiting_list_length==1) {

							sprintf(str, "There is currently one player on the %s tournament waiting list.", buy_in_str);

						} else {

							sprintf(str, "There are %d players on the %s tournament waiting list.", tsi.waiting_list_length, buy_in_str);

						}

					} else {

						if (tsi.waiting_list_length==0) {

							strcpy(str, "The waiting list is empty.");

						} else if (tsi.waiting_list_length==1) {

							strcpy(str, "There is one player on the waiting list.");

						} else {

							sprintf(str, "There are %d players on the waiting list.", tsi.waiting_list_length);

						}

					}

					if (tsi.user_waiting_list_pos == 0) {

						strcat(str, "\nPress 'Waiting List' to join the queue.");

					} else if (tsi.user_waiting_list_pos == 1) {

						strcat(str, "\nYou are first in line.");

					} else if (tsi.user_waiting_list_pos) {

						char str2[40];

						sprintf(str2, "\nYou are number %d in line.", tsi.user_waiting_list_pos);

						strcat(str, str2);

					}

				}

				SetWaitListStatusString(t, str);

			}

		} else {

		  #if ADMIN_CLIENT

			AutoJoinDefaultTable = FALSE;	// we're sitting somewhere... no need to join again.

		  #endif

		  #if 0	//20000908MB

			// If we're sitting out, tell them how to sit back in.

			if (t->ClientState.sitting_out_flag && 

					!t->wait_for_bb &&

					t->GamePlayerData.player_status[t->GamePlayerData.seating_position]==PLAYER_STATUS_SITTING_OUT) {

				SetWaitListStatusString(t, "To start playing, uncheck 'Sit out'");

			} else {

				SetWaitListStatusString(t, NULL);

			}

		  #endif

			UpdatePlayingButtons(t);

		}



		// Set the game serial number text.

		str[0] = 0;

		if (t->GameCommonData.game_serial_number) {

			char str2[50];

			//sprintf(str, "Current game\n     #%s", IntegerWithCommas(str2,t->GameCommonData.game_serial_number));
			  sprintf(str, " Current game     #%s", IntegerWithCommas(str2,t->GameCommonData.game_serial_number));
			// 20000827HK -- put the tournament summary string here

			if (t->tournament_game_number_str[0]) {	// something there...?

				sprintf(str+strlen(str), "\n%s", t->tournament_game_number_str);

			} else {

				if (t->prev_game_serial_number) {

					sprintf(str+strlen(str), "\nPrevious\n      #%s", IntegerWithCommas(str2,t->prev_game_serial_number));

				}

			}

		}

		SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_TABLE_GAME_SERIAL_NUM), str);
	 		
		//gong 04/04/2002 - 1 -- display current high hand

		str[0]=0;

		unsigned char c[5];

		for (int y=0;y<5;y++) c[y] = t->GameCommonData.hihand[y];

		char str_tmp[128];

		zstruct(str_tmp);

		//sprintf(str_tmp, "Current high hand\n%s\n", t->GameCommonData.hihand_name);
		//sprintf(str_tmp, "Current high hand %s\n", t->GameCommonData.hihand_name);

		char card[3];

		char card_num;

		char card_type;

		for (int x=0; x<5; x++) {

			card_num=c[x]>>4;

			if (card_num == 1) {

				card_num = 'A';

			} else if (card_num == 10) {

				card_num = 'T';

			} else if (card_num == 11) {

				card_num = 'J';

			} else if (card_num == 12) {

				card_num = 'Q';

			} else if (card_num == 13) {

				card_num = 'K';

			} else {

				card_num += '0';

			}

			card_type=t->GameCommonData.hihand[x] & 0x0f;

			if (card_type == 1) {

				card_type = 's';

			} else if (card_type == 2) {

				card_type = 'h';

			} else if (card_type == 3) {

				card_type = 'd';

			} else if (card_type == 4) {

				card_type = 'c';

			}

			card[0]=card_num;

			card[1]=card_type;

			card[2]=0;

			strcat(str_tmp, card);

			if(x<4) strcat(str_tmp, " - ");

		}
		SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_HIGH_HAND), str_tmp);
        if(
			((t->GamePlayerData.seating_position==0)&&(t->GamePlayerData.s_gameflow!=GAMEFLOW_DURING_GAME) ) ||
			(t->GamePlayerData.player_status[t->GamePlayerData.seating_position]!=PLAYER_STATUS_PLAYING)
			){
               SetDlgItemText(t->hwnd, IDC_MY_MONEY, ""); 
		}else{
			if(t->GamePlayerData.player_status[t->GamePlayerData.seating_position]==PLAYER_STATUS_PLAYING){
			   UpdateMymoneyText (t);
			}else{
				
				 SetDlgItemText(t->hwnd, IDC_MY_MONEY, ""); 
			};//if
		};//if

		// If the game serial number changed or we're in a game over state,

		// make sure we reset the 'fold in turn' checkbox.

		if (t->ClientState.in_turn_action_game_serial_number != t->GameCommonData.game_serial_number ||

			t->GamePlayerData.s_gameover)

		{

			ClearInTurnActions(t);

		}



		// show our best hand so far.

		if (t->watching_flag) {

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_BEST_HAND), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LOWEST_HAND), SW_HIDE);

		} else {

			// 991113HK: only display description for play money tables

			int show_hand = (t->chip_type == CT_PLAY);

		  #if ADMIN_CLIENT

			if (iAdminClientFlag) {

				show_hand = TRUE;

			}

		  #endif

			// 20001177HK: added s_gameover flag (don't call this between hands)

			if (show_hand && !t->GamePlayerData.s_gameover) {

				char hi_hand[50], lo_hand[50];
				char myMoney[50];

				zstruct(hi_hand);

				zstruct(lo_hand);
				zstruct(myMoney);

				OurHandDescription(t, hi_hand, lo_hand);	// describe our best hands up to now

				// set text for best high hand

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_BEST_HAND), hi_hand);
             //ccv
			//	strcpy(myMoney,"Cristican");
            
            //    SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_MY_MONEY), myMoney);
            //ccv   
				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_BEST_HAND), hi_hand[0] ? SW_SHOWNA : SW_HIDE);

				// set text for best low hand

				SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_LOWEST_HAND), lo_hand);

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LOWEST_HAND), lo_hand[0] ? SW_SHOWNA : SW_HIDE);

			} else {

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_BEST_HAND), SW_HIDE);

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LOWEST_HAND), SW_HIDE);

			}

		}



		if (LoggedIn == LOGIN_VALID) {

			sprintf(str, "All-Ins remaining: %d", SavedAccountInfo.all_in_count);

			SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_ALL_INS_REMAINING), str);

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_ALL_INS_REMAINING), SW_SHOWNA);

		} else {

			ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_ALL_INS_REMAINING), SW_HIDE);

		}



	  #if ADMIN_CLIENT && 0

		if (iAdminClientFlag) {

			//sprintf(str, "Watching: %d", t->GameCommonData.watching_count);

			//SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_WATCHING_COUNT), str);

			sprintf(str, "Total: %d", t->GameCommonData.players_saw_pocket);

			SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_TOTAL_CONNECTIONS), str);

		}

	  #endif

	}

	// redraw all graphics again and blit to screen now that some windows

	// may have been shown or hidden

	RedrawAllTableGraphics(t);

}



//****************************************************************

//
//

// Handle the user pressing the 'Leave Table' button

// return: FALSE: no problem, table was closed.

//			TRUE: close was cancelled.

//

int HandleLeaveTableButton(struct TableInfo *table_ptr)

{

	return HandleLeaveTableButton(table_ptr, TRUE);

}



int HandleLeaveTableButton(struct TableInfo *table_ptr, int show_cardroom_flag)

{

	int result = IDOK;	// default to "OK".



	// Four possibilities:

	// 1. In between games.  Leaving now is OK.

	// 2. During game, already folded.  No worries for the user,

	//    but we don't actually leave the game until the game is over.

	// 3. During game, user still playing.  Leaving now will force

	//    them to fold.  We need to warn them.

	// 4. We're watching, and just want to leave.



  #if ADMIN_CLIENT

	// if Ctrl key is down, tell the cardroom to shut everything down too

	int shutdown_flag = FALSE;

	if (iAdminClientFlag && (GetKeyState(VK_CONTROL) & 0x80)) {

		shutdown_flag = TRUE;

		goto leavetable;

	}

  #endif



	// Get current status

	if (table_ptr->watching_flag) {

		// We're just watching... unjoin and get out of here.

		PostMessage(table_ptr->hwnd, WMP_CLOSE_YOURSELF, 0, 0);

		// Tell server we're leaving.

		// Send a DATATYPE_CARDROOM_JOIN_TABLE packet.

		struct CardRoom_JoinTable jt;

		zstruct(jt);

		jt.table_serial_number = table_ptr->table_serial_number;

		jt.status = JTS_UNJOIN;	// request to leave this table.

		SendDataStructure(DATATYPE_CARDROOM_JOIN_TABLE, &jt, sizeof(jt));

		if (show_cardroom_flag) {

			PostMessage(hInitialWindow, WMP_SHOW_CARDROOM, 0, 0);

		}

		return FALSE;	// nothing more to do.

	}



  #if 0	//20000713MB

	if (CheckIfPlayingCurrentGame(table_ptr)

		  #if ADMIN_CLIENT

			&& !RunningManyFlag

		  #endif

	) {

		result = MessageBox(table_ptr->hwnd,

				"If you leave now, your hand will be folded.",

				"Leave table...",

				MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL);

	}

  #else

   #if ADMIN_CLIENT

	if (!RunningManyFlag)

   #endif

	{

		if (table_ptr->chip_type==CT_TOURNAMENT) {

			// Tournament table.

			if (table_ptr->GameCommonData.table_tourn_state == TTS_WAITING) {

				// The tournament has not yet started.

				result = MessageBox(table_ptr->hwnd,

					"If you leave the table BEFORE the tournament has started,\n"

					"you will receive a full refund of your buy-in and fee.\n"

					"\n"

					"If you leave the table after the tournament starts, you\n"

					"will automatically post blinds and fold in turn until\n"

					"you run out of tournament chips.\n"

					"\n"

					"Do you still wish to leave this table?",

					"Leaving a tournament...",

					MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL);

			} else if (table_ptr->GameCommonData.table_tourn_state != TTS_FINISHED) {

				result = MessageBox(table_ptr->hwnd,

					"If you leave the table before the tournament is finished,\n"

					"you will automatically post blinds and fold in turn until\n"

					"you run out of tournament chips.\n"

					"\n"

					"Do you still wish to leave this table?",

					"Leaving a tournament...",

					MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL);

			}

		} else {

			// non-tournament table.

			if (CheckIfPlayingCurrentGame(table_ptr)) {

				result = MessageBox(table_ptr->hwnd,

						"If you leave now, your hand will be folded.",

						"Leave table...",

						MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL);

			} else {

				result = MessageBox(table_ptr->hwnd,

						"Are you sure you want to leave your seat?",

						"Leave table...",

						MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL);

			}

		}

	}

  #endif

	if (result==IDYES || result==IDOK) {

#if ADMIN_CLIENT

leavetable:;

#endif

		ClearInTurnActions(table_ptr);

		// Tell server we're leaving.

		table_ptr->ClientState.post_in_turn = FALSE;

		table_ptr->ClientState.post_in_turn_game_serial_number = 0;

		table_ptr->ClientState.fold_in_turn = TRUE;	// always set it just to be safe

		table_ptr->ClientState.in_turn_action_game_serial_number = table_ptr->game_serial_number;

		table_ptr->ClientState.leave_table = TRUE;

		SendClientStateInfo(table_ptr);

		PostMessage(table_ptr->hwnd, WMP_CLOSE_YOURSELF, 0, 0);

	  #if ADMIN_CLIENT

		if (shutdown_flag) {

			if (!hCardRoomDlg) {

				SendMessage(hInitialWindow, WMP_SHOW_CARDROOM, 0, 0);

			}

			PostMessage(hCardRoomDlg,   WMP_CLOSE_YOURSELF,0, 0);

		} else

	  #endif

		{

			if (show_cardroom_flag) {

				PostMessage(hInitialWindow, WMP_SHOW_CARDROOM, 0, 0);

			}

		}

		return FALSE;

	} else {

		return TRUE;	// cancel

	}

}



//*********************************************************

//
//

// Handle a WM_DRAWITEM message for a table.  We get a Table

// pointer, a control ID, and ptr to a DRAWITEMSTRUCT.

// WM_DRAWITEM messages come for Owner Draw controls.

//

void HandleTableDrawItem(struct TableInfo *t, int control_id, LPDRAWITEMSTRUCT dis)

{

	//kp(("%s(%d) Get WM_DRAWITEM for control #%d. State = %d\n", _FL, control_id, dis->itemState));

	// Determine which bitmap to draw there and how to draw it.

	if (control_id==IDC_DEALER_CHIP_AREA) {

		int bitmap_index = -1;

		if (dis->hwndItem==t->proximity_highlight_list.hCurrentHighlight) {

			// The mouse is over us... use the rollover version

			switch (t->chip_type) {

			case CT_NONE:

				Error(ERR_INTERNAL_ERROR,"%s(%d) called with CT_NONE", _FL);

				break;

			case CT_PLAY:

				bitmap_index = MISCBITMAP_CHIP_TRAY_R;

				break;

			case CT_REAL:

				bitmap_index = MISCBITMAP_CHIP_TRAY_RM_R;

				break;

			case CT_TOURNAMENT:

				bitmap_index = MISCBITMAP_CHIP_TRAY_RM_R;

				break;

			default:

				Error(ERR_INTERNAL_ERROR,"%s(%d) called with unknown chip_type", _FL);

				break;

			}

		}

		DrawButtonItemWithBitmap(t, dis, bitmap_index);

		return;

	}

	if (control_id==IDC_SNACK_MENU) {

		int bitmap_index = MISCBITMAP_MENU_BUTTON;

		if (dis->hwndItem==t->proximity_highlight_list.hCurrentHighlight) {

			// The mouse is over us... use the rollover version

			bitmap_index = MISCBITMAP_MENU_BUTTON_R;

		}

		DrawButtonItemWithBitmap(t, dis, bitmap_index);

		return;

	}



	HWND hwnd = GetDlgItem(t->hwnd, control_id);

	for (int i=0 ; i<t->max_players_per_table ; i++) {

		// It might be a sit-down button... test all of them.

		if (control_id==SitDownButtonIDs[i]) {

			// Found a match.

			DrawButtonItemWithBitmap(t, dis, MISCBITMAP_SITDOWN);

			return;

		}

		// It might be a player ID box...

		if (control_id==PlayerTextIDs[i]) {

			// Found a match.  If it's highlighted, use the highlighted

			// version of the bitmap.

			// The highlight flag selects whether the window is enabled

			// or disabled, so use the window enable state as a flag.

			// Also, make the button state always the same so that it

			// does not appear to get 'depressed' when you click on it.

			dis->itemState = 0;

			HFONT font_array[3];

			// If this is part of a last action indicator update,

			// choose different fonts...

		//Aqui se le cambia el color a los botones de las sillas       J Fonseca  13/04/2004

		//	COLORREF color = RGB(0,0,0);	// default to black
		//	COLORREF color = RGB(151,151,151);	// default to black
			COLORREF color = RGB(255,255,255);	// default to white

			if (t->iLastActionStatus[i]) {

				int status = t->iLastActionStatus[i];

				if (status <= 1) {

					font_array[0] = hTableFonts[0];

				} else if (status <= 2) {

					font_array[0] = hTableFonts[1];

				} else if (status <= 3) {

					font_array[0] = hTableFonts[2];

				} else if (status <= 4) {

					font_array[0] = hTableFonts[3];

				} else  {

					font_array[0] = hTableFonts[4];

				}

				font_array[1] = NULL;

				// Scale from black to gold as it fades out

				#define FADE_OUT_RANGE	6

				if (status <= FADE_OUT_RANGE) {

					color = RGB(0,0,0) ; //CalculateIntermediateColor(RGB(203,167,47), color, status, FADE_OUT_RANGE);

				}

			} else {

				// Normal... show name larger than stake.

				font_array[0] = hTableFonts[1];		// name in larger font



				// 20001214MB: if the name is quite wide, use the smaller font for the

				// name as well (so it doesn't go past the edges of the ovals).

				int caps_count = 0;

				char *s = t->GameCommonData.name[i];

				while (*s) {

					if (isupper(*s) || *s=='@' || *s=='_') {

						caps_count++;

					}

					s++;

				}

			  #if 1	//20010104MB: assume caps use twice as much space

				int approx_width = strlen(t->GameCommonData.name[i]) + caps_count;

				if (approx_width > 12) {

					font_array[0] = hTableFonts[0];		// use smaller font

				}

			  #else

				int approx_width = strlen(t->GameCommonData.name[i]) + caps_count / 2;

				if (approx_width > 12) {

					font_array[0] = hTableFonts[0];		// use smaller font

				}

			  #endif



				font_array[1] = hTableFonts[0];		// stake in smaller font

				font_array[2] = NULL;

			}

			static COLORREF color_array[4] = {

				RGB(0,0,0),			// normal text
				//RGB(255,255,255),			// normal text

				RGB(173,143,67),	// normal shadow

				RGB(255,228,82),	// disabled text

				RGB(170,141,25)		// disabled shadow

			};

			color_array[0] = color;

			color_array[1] = (DWORD)-1;	// no shadow

			DrawButtonItemWithBitmapAndText(t, dis, 

					IsWindowEnabled(hwnd) ? MISCBITMAP_PLAYER_TURN : MISCBITMAP_PLAYER_ID, font_array, color_array);

			return;

		}

	}

	if (control_id==IDC_GOTO_CARDROOM) {

		DrawButtonItemWithBitmap(t, dis, MISCBITMAP_CARDROOM);

		return;

	}

	if (control_id==IDC_LEAVE_TABLE) {

		DrawButtonItemWithBitmap(t, dis, MISCBITMAP_LEAVETABLE);

		return;

	}

	if (control_id==IDC_BUTTON1 || control_id==IDC_BUTTON2 || control_id==IDC_BUTTON3 || control_id==IDC_BUTTON4) {

		int bitmap_index = MISCBITMAP_PLAY_BUTTON_B;	// use bright version by default
		HFONT font = hTableFonts[1];

		if (t->button_actions_are_in_turn) {
			bitmap_index = MISCBITMAP_PLAY_BUTTON;
			font = hTableFonts[5];	// use a smaller font
		};//if(t->button_actions_are_in_turn) 

		if (dis->itemState & ODS_DISABLED) {
			bitmap_index = MISCBITMAP_PLAY_BUTTON_D;	// use dark version
			font = hTableFonts[5];	// use a smaller font
		};//if(dis->itemState & ODS_DISABLED)
		DrawButtonItemWithBitmapAndText(t, dis, bitmap_index, font);
	
	};//if (control_id==IDC_BUTTON1 || control_id==IDC_BUTTON2 || control_id==IDC_BUTTON3 ...
    
}



#if INCLUDE_TABLE_OPTIONS

//*********************************************************

//
//

// Mesage handler for table options window

//

BOOL CALLBACK dlgFuncTableOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	struct TableInfo *t = (struct TableInfo *)GetWindowLong(hDlg, GWL_USERDATA);

	//kp(("%s(%d) t = $%08lx\n",_FL,t));

	switch (message) {

	case WM_INITDIALOG:

		// note: t (the table pointer) is not yet initialized properly

		hTableOptionsDlg = hDlg;

		WinRestoreWindowPos(ProgramRegistryPrefix, "TableOptions", hDlg, NULL, NULL, FALSE, TRUE);

		AddKeyboardTranslateHwnd(hDlg);

		// Make ourselves a topmost window

		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		return TRUE;

	case WM_COMMAND:

		{

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				//kp(("%s(%d) IDCANCEL received.  Cancelling upgrade.\n",_FL));

				DestroyWindow(hDlg);

				return TRUE;	// We DID process this message

			case IDM_REQUEST_ALLIN_RESET:

			case IDC_REQUEST_HAND_HISTORY:

				// relay to cardroom for handling...

				DestroyWindow(hDlg);

				PostMessage(hCardRoomDlg, message, wParam, lParam);

				return TRUE;	// We DID process this message

			case IDC_REQUEST_MORE_CHIPS:

				DestroyWindow(hDlg);

				if (t && t->tournament_table) {	// no rebuys supported yet
					MessageBox(t->hwnd,
						"Re-buys are not allowed in this tournament",
						"No re-buy allowed",
						MB_OK|MB_TOPMOST);
				} else {
					if (t && !t->watching_flag) {
						//20000712HK: t->minimum_buy_in was set with the last buy in we did at this table
						BuyIntoTable(JTS_REBUY, t, t->minimum_buy_in, 0);
					
					};//if (t && !t->watching_flag) 
				};//if (t && t->tournament_table)

				return TRUE;	// We DID process this message

		  #if 0	//20000616MB

			case IDC_SNACKMENU:

				DestroyWindow(hDlg);

				OpenSnackMenuWindow(t);

				return TRUE;	// We DID process this message

		  #endif

			}

		}

		break;

	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "TableOptions", hDlg, NULL);

		RemoveKeyboardTranslateHwnd(hDlg);

		hTableOptionsDlg = NULL;

		return TRUE;	// We DID process this message

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	}

    return FALSE;	// We did NOT process this message.

}

#endif



//****************************************************************
////
// Mesage handler for Table layout -- all games
//

BOOL CALLBACK dlgFuncTableLayoutWindow(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	pr(("mess = %08x, HIWORD(wParam) = %d, LOWORD(wParam) = %d, HIWORD(lParam) = %d, LOWORD(lParam) = %d\n",

		message, HIWORD(wParam),LOWORD(wParam),HIWORD(lParam),LOWORD(lParam)));

	if (message==WM_INITDIALOG) {

		// WM_INITDIALOG messages must be processed even if we don't

		// know which table it is for.



		// Add the system menu and our icon.  See Knowledge base

		// article Q179582 for more details.

		{

			HICON hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CLIENT),

					IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),

                    GetSystemMetrics(SM_CYSMICON), 0);

			if(hIcon) {

				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			}

		}

		ScaleDialogTo96dpi(hDlg);

		SetWindowSize(hDlg, 800, 562);

      // SetWindowText(GetDlgItem(hDlg, IDC_MY_MONEY), "NADIE");

	  #if 0

		kp(("%s(%d) Setting slow WM_TIMER\n",_FL));

    	SetTimer(hDlg, WM_TIMER, 2000, NULL);	// 2s time for testing

	  #else

	   #if 1	//20000217MB

    	SetTimer(hDlg, WM_TIMER, 1000/73, NULL);	// 73Hz animation timer (4 times 18.2)

	   #else

    	SetTimer(hDlg, WM_TIMER, 1000/37, NULL);	// 37Hz animation timer (approx. twice 18.2)

	   #endif

	  #endif

		AddKeyboardTranslateHwnd(hDlg);

		// the message we switch to is handled elsewhere...

		if (LoggedIn != LOGIN_VALID) {

			SetDlgItemTextIfNecessary(hDlg, IDC_TYPE_CHAT, "You must be logged in to chat...");

		}

		// Change the font in the chat window so that 5 lines will fit.

		{

			HWND hwnd = GetDlgItem(hDlg, IDC_CHAT_BOX);

		  #if 0	//20000303MB
			extern HWND hChatTextWindow;
			hChatTextWindow = hwnd;
			kp1(("%s(%d) Keeping window $%08lx as hChatTextWindow\n",_FL,hChatTextWindow));
		  #endif

			// Try to pick a suitable font
			static HFONT chatwindowfont;
			if (!chatwindowfont) {
				RECT r;

				zstruct(r);

				GetWindowRect(hwnd, &r);

				int chat_window_height = r.bottom - r.top - 4;	// subtract an extra 2 pixels top and bottom for border

				//kp(("%s(%d) Chat window height = %d pixels\n", _FL, chat_window_height));
				int desired_font_height = chat_window_height / 5;	// try to make room for 5 lines
				chatwindowfont = CreateFont(

						desired_font_height,//  int nHeight,             // logical height of font

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

						DEFAULT_PITCH|FF_SWISS,	//  DWORD fdwPitchAndFamily,  // pitch and family

						NULL	//  LPCTSTR lpszFace         // pointer to typeface name string

				);

			}

			if (chatwindowfont) {

				SendMessage(hwnd, WM_SETFONT, (WPARAM)chatwindowfont, 0);

			}

		}



		// limit text user can enter in chat text input box

		SendMessage(GetDlgItem(hDlg, IDC_TYPE_CHAT), EM_LIMITTEXT, MAX_CHAT_MSG_LEN, 0L);



		// Fill in the fields for the quiet dealer combo box and pick a default

		HWND combo = GetDlgItem(hDlg, IDC_COMBO_CHAT_MODE);

	    SendMessage(combo, CB_RESETCONTENT, 0, 0);

		struct ChatModeData *cmd = ChatModeData;

		while (cmd->title) {

			//kp(("%s(%d) Adding '%s'\n", _FL, cmd->title));

	        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)cmd->title);

			cmd++;

		}

		SendMessage(combo, CB_SETCURSEL, (WPARAM)Defaults.iQuietDealerType, 0);



	  #if INCLUDE_TABLE_OPTIONS

		// Add a few new menu options...

		HMENU hm = GetSystemMenu(hDlg, FALSE);

		if (hm) {

			int insert_position = 0;

			MENUITEMINFO mii;

			zstruct(mii);

			mii.cbSize = sizeof(mii);

			mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;

			mii.fType = MFT_STRING;

			mii.fState = MFS_ENABLED;

			mii.wID = (WORD)IDC_DEALER_CHIP_AREA;

			mii.dwTypeData = "Table options...";

			InsertMenuItem(hm, insert_position++, MF_BYPOSITION, &mii);

			mii.fType = MFT_SEPARATOR;

			mii.wID = (WORD)0;

			mii.dwTypeData = 0;

			InsertMenuItem(hm, insert_position++, MF_BYPOSITION, &mii);

		}

	  #endif	// INCLUDE_TABLE_OPTIONS



	  #if ADMIN_CLIENT

		if ((iAdminClientFlag) && (LoggedInPrivLevel >= 40 ))  {

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS), SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS_SPIN), SW_SHOWNA);

			SetWindowText(GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS), "2");

			SendMessage(GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS_SPIN), UDM_SETRANGE,

				0, (LPARAM)MAKELONG((short)20, (short)0));

		  #if MIN_HAND_RANK_POPUP

//			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_MIN_HAND_RANK), SW_SHOWNA);

//			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_MIN_HAND_RANK_SPIN), SW_SHOWNA);
		

			SetWindowText(GetDlgItem(hDlg, IDC_MIN_HAND_RANK_EDIT), "4");

			SendMessage(GetDlgItem(hDlg, IDC_MIN_HAND_RANK_SPIN), UDM_SETRANGE,

				0, (LPARAM)MAKELONG((short)8, (short)1));

		  #endif //MIN_HAND_RANK_POPUP

			//SendMessage(GetDlgItem(hDlg, IDC_CHAT_POPUP), BM_SETCHECK, BST_CHECKED, 0);

			//SendMessage(GetDlgItem(hDlg, IDC_POST_IN_TURN), BM_SETCHECK, BST_CHECKED, 0);

		}

	  #endif //Admin Client

		return TRUE;

	}



	// Figure out which table this message is for.

	for (int table_index = 0 ; table_index < MAX_TABLES ; table_index++) {

		if (Table[table_index].hwnd == hDlg)

			break;

	}

	if (table_index >= MAX_TABLES) {

	  #if DEBUG

		if (message==WM_ERASEBKGND) {

			kp(("%s(%d) WM_ERASEBKGND messages are getting lost here.\n",_FL));

		}

	  #endif



	  #if DISP	//19990614MB

		kp(("%s(%d) Got dialog message (%d) for a table we don't have open (not usually a problem)\n", _FL, message));

	  #endif

		return FALSE;	// We did NOT process this message.

	}

	struct TableInfo *t = &Table[table_index];

	//kp(("Table $%08lx: message = %4d, wParam = %d, lParam = %d\n", t, message, wParam, lParam));

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_LIMIT), SW_HIDE);	

	switch (message) {

  #if 0	//19990712MB: note... this is handled up above for tables.

	case WM_INITDIALOG:

		return TRUE;

  #endif

	case WM_ACTIVATE:

		{
            HWND hwnd = (HWND)lParam;	// hwnd of window getting drawn
			int fActive = LOWORD(wParam);

			BOOL fMinimized = (BOOL)HIWORD(wParam);
          
			if (fActive!=WA_INACTIVE && !fMinimized) {	// we got activated...

				t->active_table_window = TRUE;

				t->animation_disable_flag = Defaults.iAnimationDisabled;

				// any time we get activated we should redraw the whole window to

				// make sure nothing gets left half-drawn due to bugs in the video

				// drivers or elsewhere that seem to forget about refreshing

				// certain areas of the screen.

				//kp(("%s(%d) WM_ACTIVATE: Calling InvalidateRect() for table.\n",_FL));

				InvalidateRect(hDlg, NULL, FALSE);

				//kp(("%s(%d) WM_ACTIVATE: Done calling InvalidateRect() for table.\n",_FL));

			} else {

				t->active_table_window = FALSE;

				t->animation_disable_flag =  TRUE;

			}

			//kp(("%s(%d) t->active_table_window = %d\n", _FL, t->active_table_window));
			#if 1
			//ccv my money text
			if (hwnd==GetDlgItem(hDlg, IDC_MY_MONEY)) {
				
				
				SetTextColor((HDC)wParam, RGB(255,255,255));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return ((int)GetStockObject(NULL_BRUSH));
			}
            //ccv my money text	
           #endif  
		#if 0
			//ME CAGO
			//ccv my money text
			if (hwnd==GetDlgItem(hDlg, IDC_FOLD_IN_TURN)) {
				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_FOLD_IN_TURN), SW_SHOW);		
				SetTextColor((HDC)wParam, RGB(255,255,255));
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetWindowText(GetDlgItem(hDlg, IDC_FOLD_IN_TURN), "Cris");
				return ((int)GetStockObject(NULL_BRUSH));
			}
            //ccv my money text	
         #endif  
			

		}



				

		SetTableKeyboardFocus(t);	// purely cosmetic

		pr(("%s %s(%d) table %d, time = %4d, active_table_window = %d\n",

				TimeStr(), _FL, t->table_serial_number, SecondCounter, t->active_table_window));

		t->last_activated_time = SecondCounter;	// keep track of when last activated/deactivated

		MaskCache_FreeAll();	// force re-build of any video memory masks
		

		     

		return TRUE;	// TRUE = we did process this message.



	case WM_CLOSE:

		// Do the same thing as clicking 'Leave Table'

		HandleLeaveTableButton(t);

		break;



  #if 0	//19990820MB

	case BM_SETSTATE:

		kp(("%s(%d) Got BM_SETSTATE. hDlg = $%08lx\n",_FL,hDlg));

		break;

  #endif



  #if 0	//19990820MB: we don't seem to get these message... presumably

  	// because we're the owner of the edit box and the edit box owns

	// the scroll bar.

	case WM_CTLCOLORSCROLLBAR:

		// hdcStatic = (HDC) wParam;   // handle to display context 

		// hwndStatic = (HWND) lParam; // handle to static control 

		{

			HDC hdc = (HDC)wParam;


            SetTextColor(hdc, RGB(255,255,255));	// draw text in gold

			SetBkColor(hdc, RGB(50,40,5));
			
			return ((int)GetStockObject(WHITE_BRUSH));	// use white for testing

		}

		break;

  #endif

	//J Fonseca		28/01/2004
	case WM_HSCROLL: { 
		
		HWND slider = GetDlgItem(t->hwnd, IDC_LIMIT);
		
		int tic = SendMessage(slider, TBM_GETPOS, 0, 0);  //Optiene el tic seleccionado
		char tc[20];
		char button_text[20];
		char cs[MAX_CURRENCY_STRING_LEN];
		sprintf(tc,"%s",CurrencyString(cs, (tic * 100), CT_REAL, TRUE, 0));
		SetDlgItemText(t->hwnd, IDC_NEW_LIMIT, tc);
		
		
		sprintf(tc,"(%d)",tic);
		strcpy(button_text, "Raise ");
		strcat(button_text, tc);

		SetDlgItemTextIfNecessary(t->hwnd, ButtonIDs[2], button_text);
		t->GamePlayerInputRequest.raise_amount = tic * 100;
		RedrawAllTableGraphics(t);  // redraw all graphics in new positions
	}
	break;	
	//J Fonseca		28/01/2004

	
	case WM_CTLCOLORBTN:

		// This message is sent to all buttons before drawing.  Return a 

		// null background brush so that we don't get grey drawn just before

		// our bitmap is going to be drawn.

		//kp(("%s(%d) Got WM_CTLCOLORBTN\n",_FL));

		return ((int)GetStockObject(NULL_BRUSH));



	case WM_CTLCOLOREDIT:

	case WM_CTLCOLORSTATIC:

		// hdcStatic = (HDC) wParam;   // handle to display context 

		// hwndStatic = (HWND) lParam; // handle to static control 

		{

			HWND hwnd = (HWND)lParam;	// hwnd of window getting drawn

			if (hwnd == GetDlgItem(hDlg, IDC_CHAT_BOX) ||

				hwnd == GetDlgItem(hDlg, IDC_COMBO_CHAT_MODE) ||

			  #if ADMIN_CLIENT

				hwnd == GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS) ||

	  			  #if MIN_HAND_RANK_POPUP

					hwnd == GetDlgItem(hDlg, IDC_MIN_HAND_RANK_EDIT) ||

				  #endif

			  #endif

				hwnd == GetDlgItem(hDlg, IDC_TYPE_CHAT)) {

				// Chat box... draw background in white

				SetBkColor((HDC)wParam, CHAT_BGND_COLOR);


		    	SetTextColor((HDC)wParam, CHAT_TEXT_COLOR);


				return ((int)hChatBgndBrush);

			}

			if (hwnd==GetDlgItem(hDlg, IDC_DEALER_CHIP_AREA)) {

				return ((int)GetStockObject(NULL_BRUSH));

			}

            

			for (int i=0 ; i<iColorStaticWindowCount ; i++) {

				if (hwnd==hColorStaticWindows[i]) {	// found a match!

					// Checkbox... draw text area as transparent

					// draw bitmapped background and change the text color.

				  #if 0	//19990823MB

					char str[100];

					GetWindowText(hwnd, str, 100);

					str[100-1] = 0;

					kp(("%s(%d) got WM_CTLCOLORSTATIC for $%08lx. Text='%s'\n",_FL,hwnd,str));

				  #endif

					HDC hdc = (HDC)wParam;

					RECT r;

					GetWindowRect(hwnd, &r);

					ScreenToClient(GetParent(hwnd), &r);

					// Blit this rect from our background into the display area.

					POINT pt;

					zstruct(pt);

					if (t->hbm_off_screen) {

						EnterCriticalSection(&t->draw_crit_sec);

						BlitBitmapRectToDC(t->hbm_off_screen, hdc, &pt, &r);

						LeaveCriticalSection(&t->draw_crit_sec);

					}//if

					if (iStaticWindowColorTypes[i]) {

						SetTextColor(hdc, RGB(255,255,255));	// draw text in gold

					} else {

						SetTextColor(hdc, RGB(255,255,255));	// draw text in gold
					
					}//if

					SetBkMode(hdc, TRANSPARENT);



					// If this is don't let WiIDC_WAIT_LIST_STATUS, ndows draw the text,

					// do it ourselves (Windows can't center two lines properly).

					if (hwnd==GetDlgItem(hDlg, IDC_WAIT_LIST_STATUS)) {

						int cx = (r.left + r.right) / 2 - r.left;

						int cy = (r.top + r.bottom) / 2 - r.top;

					    SetTextColor(hdc, RGB(0,0,0));	// draw text in black
                        //SetTextColor(hdc,RGB(255,255,255));
						DrawCenteredText(hdc, t->wait_list_status_string, cx+1, cy+1);

						SetTextColor(hdc, RGB(255,255,255));	// draw text in gold 
						DrawCenteredText(hdc, t->wait_list_status_string, cx, cy);

					}//if
                    
					return ((int)GetStockObject(NULL_BRUSH));

				}//if

			}//for
			#if 0
			  //ccv my money text
		    	if (hwnd==GetDlgItem(hDlg, IDC_MY_MONEY)) {

			    	SetTextColor((HDC)wParam, RGB(255,255,255));
					SetBkMode((HDC)wParam, TRANSPARENT);
					UpdateMymoneyText(t);
			    	return ((int)GetStockObject(NULL_BRUSH));
				}
            //ccv my money text
           #endif

		}

		

		break;



	case WM_COMMAND:

		{

			int line_length, line_count;

			char tmp_chat_line[MAX_CHAT_MSG_LEN];

			pr(("WM_COMMAND : %d / %d\n", (int)(wParam), (int)lParam));

			int button = LOWORD(wParam);



		  #if 0	// unused at present.

			// Check if this is a button notification (BN_*)

			if (HIWORD(wParam)==BN_CLICKED) {

				kp(("%s(%d) Received BN_CLICKED for button %d\n", _FL, button));

			}

		  #endif

		

			// Check if this is one of the GPIResult related buttons

			// and process it accordingly.

			for (int i=0 ; i<MAX_BUTTONS_PER_TABLE ; i++) {
				
				if (button == ButtonIDs[i]) {	// found a match
					
					ClearInTurnActions(t);

					// 20000811HK: SetFocus call is cosmetic -- after hiding some other

					// controls for tournament tables, the DealerChat selector was being

					// focused on when a button was clicked.  This sets the focus on something

					// we know isn't visible, so nothing is highlighted blue or outlined, etc.

					//kp1(("%s(%d) SetFocus() call doesn't always work -- see src and comments\n", _FL));

					/// !!! this still needs to be looked at

					// 20000908MB: SetFocus() may not work if the control is disabled or hidden.

					// Perhaps something else should be selected or the IDC_FLOP control

					// should be looked into to see if it is disabled or hidden.

					// 20000912MB: SetFocus(NULL) could also be tested.

					SetTableKeyboardFocus(t);	// purely cosmetic

					if (t->watching_flag) {

						// We're only watching... buttons are interpreted differently.

						pr(("%s(%d) Button %d was pressed while watching.\n",_FL, i));

						if (i==0) {	// "Waiting List"?

							if (LoggedIn != LOGIN_VALID) {

								LogInToServer("You must be logged in before joining a waiting list");

								if (LoggedIn != LOGIN_VALID) {

									break;	// message handled

								};//if

							};//if

							// 0=none, 1=Join Waiting list, 2=Unjoin Waiting List

							switch (t->button_1_alternate_meaning) {

							case 1:

								WaitListDialog(hDlg, t->table_serial_number);

								break;

							case 2:

								UnjoinWaitListDialog(hDlg, t->table_serial_number,1);

								break;

							};//switch

							// ask server for the most recent data if necessary

							// (so the dialog has the most up to date info)

							CheckForNewGameList(t->client_display_tab_index);	

						};//if

					} else {

						//20000628MB: during game play, if we just switched

						// windows from another playing table that overlaps the

						// button areas of this one, we may want to ignore button

						// presses for the first little bit.

						// If that's the case, discard the button press here.

						if (dwIgnoreInputButtons_Ticks > GetTickCount()) {

							kp(("%s(%d) Ignoring button press\n", _FL));

							return TRUE;	// We DID process this message.

						}//if

						dwIgnoreInputButtons_Ticks = 0;	// always reset to zero



						if (i==0 && t->button_1_alternate_meaning==3) {	// I'm back?

							//kp(("%s(%d) Detected 'I'm Back' being pressed. Changing t->forced_to_sit_out_flag from %d to FALSE.\n", _FL, t->forced_to_sit_out_flag));

							// They clicked 'I'm Back'.

							t->button_1_alternate_meaning = 0;	// clear it.

							t->ClientState.sitting_out_flag = FALSE;

							t->forced_to_sit_out_flag = FALSE;	// changed manually.  clear flag indicating the server forced us,

							CheckDlgButtonIfNecessary(hDlg, IDC_SIT_OUT, BST_UNCHECKED);

							// Tell the server our new state.

							SendClientStateInfo(t);

							FlagRedrawTable(t->table_index);

							return TRUE;	// We DID process this message.

						};//if

						//kp(("%s(%d) Button %d was pressed.  Mapping action %d to it.\n",_FL,button,t->button_actions[i]));

						if (t->GamePlayerInputRequest.game_serial_number &&

							t->button_actions[i] != ACT_NO_ACTION) {

							// HK990605 we may want to trap certain actions locally...

							int action = t->button_actions[i];

							switch (action) {

							case ACT_FOLD:

								//19991230MB: if checking is an option, ask them

								// if they're sure.

								// Note that this code won't work if 'fold' is the

								// last button, but at present it's always the first

								// button.

								if (t->button_actions[i+1]==ACT_CHECK) {
                                 //Disable Fold Free
									if (MessageBox(hDlg,

											"It is free to CHECK\n\n"

											"Are you sure you want to fold?",

											"Confirm fold",

											MB_YESNO|MB_TOPMOST)==IDNO)

									{

										action = ACT_CHECK;	// check instead.
									}else{
										action = ACT_FOLD;	// fold instead.
									}
									

								};//(t->button_actions[i+1]==ACT_CHECK)
                                //ccv --money text
				               //  UpdateMymoneyText(t);
						        //ccv --money text
								break;

							// we trapped a sit out message of some sort

							case ACT_SIT_OUT_SB:

							case ACT_SIT_OUT_BB:

							case ACT_SIT_OUT_POST:

							case ACT_SIT_OUT_BOTH:

							case ACT_SIT_OUT_ANTE:

								// 20000626HK: check to see if we trapped a "WAIT FOR BB" button. This can be

								// done by checking if this button has the SAME action as another button

								if (i && 

									(t->button_actions[i] == t->button_actions[i-1]) ||

									// check the button two away, it might be that one

									(i >= 2 && t->button_actions[i] == t->button_actions[i-2]))

								{

									// clicked wait for BB

									//kp(("%s(%d) Detected 'wait for bb' clicked.\n", _FL));

									t->wait_for_bb = TRUE;

									// this does not sit you out as far as the server is concerned

								} else {

									// 990605HK if player hit a sitout button, set his sit out checkbox

									// 20000811HK: be sure it can't happen with a tournament table

									if (!t->tournament_table) {

										t->ClientState.sitting_out_flag = (BYTE8)TRUE;

										//kp(("%s(%d) changing t->forced_to_sit_out_flag from %d to FALSE.\n", _FL, t->forced_to_sit_out_flag));

										t->forced_to_sit_out_flag = FALSE;	// changed manually.  clear flag indicating the server forced us,

										CheckDlgButtonIfNecessary(hDlg, IDC_SIT_OUT, BST_CHECKED);

										// Tell the server our new state.

										SendClientStateInfo(t);

										// Enable/disable the IDC_POST_IN_TURN checkbox depending on whether

										// we're sitting out or not.

										ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POST_IN_TURN),

												(t->ClientState.sitting_out_flag || t->zero_ante_stud_flag) ? SW_HIDE : SW_SHOWNA);

									}//if

								}//if
								
								break;

							}

							// actual processing of button press is done here

							ProcessGPIResult(t, action);
							


							//29990627MB: Set a timer for when we should check if another

							// table should be topped (now that our actions have been

							// selected).  This is especially important in two table play.

							// We should allow enough time for the action we just selected

							// to get to the server and back so we can see that the action

							// has occurred, and then we should look for any other windows

							// which need to be topped so the user can act on the buttons.

							dwCheckForInputButtons_Ticks = GetTickCount() + 500;	// n ms

							//kp(("%s(%d) %4d: Just acted on action button %d for table %d\n", _FL, SecondCounter, action, t->table_serial_number));



							// If they selected 'post' or 'ante' for a second

							// time, display a dialog box prompting them

							// to choose 'auto post'.

							switch (action) {

							case ACT_POST:			//  1 (used for new players - usually BB amount)

							case ACT_POST_SB:		//  2 player posts the small blind

							case ACT_POST_BB:		//  3 player posts the big blind

							case ACT_POST_BOTH:		//  4 (used for missed blinds: SB is dead).

							case ACT_POST_ANTE:		// 21 player posts an ante

							case ACT_POST_ALL_IN:	// 23 player is posting all-in

								t->wait_for_bb = FALSE;	// obviously no longer waiting

								static int iManualPostCount = 0;

								static int iPostTipShown = FALSE;

								if (++iManualPostCount >= 2 && !iPostTipShown) {

									iPostTipShown = TRUE;

									if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

										MessageBox(hDlg,

											"You may select the 'Auto-post ante' checkbox to\n"

											"automatically ante when it is your turn.\n\n"

											"The other players will appreciate it.",

											"Tip for speeding up game play...",

											MB_OK|MB_APPLMODAL|MB_TOPMOST);

									} else {

										MessageBox(hDlg,

											"You may select the 'Auto-post blind' checkbox to\n"

											"have your blinds automatically posted when it is\n"

											"your turn to post.\n\n"

											"The other players will appreciate it.",

											"Tip for speeding up game play...",

											MB_OK|MB_APPLMODAL|MB_TOPMOST);

									};//if

								};//if
							

								break;

							}

						}

					}

					return TRUE;	// We DID process this message.

				}

			}



			// Check if this is one of the 'Sit Down' buttons

			// and process it accordingly.

			for (i=0 ; i<t->max_players_per_table ; i++) {

				if (button == SitDownButtonIDs[i]) {	// found a match

					pr(("%s(%d) Button to sit down in seat %d was pressed.\n",_FL,i));

					// attempt sitting down

					int allowed_to_sit = LogInToServer("You must be logged in before sitting down to play");

					if (!allowed_to_sit) {

						return TRUE;

					}

					// If this is for a real money table, make sure their account

					// has the necessary privilege level.

					if ( (t->chip_type == CT_REAL) && LoggedInPrivLevel < ACCPRIV_REAL_MONEY) {

						// priv not high enough yet.  Go to set up.

						InitiateRealMoneyAccountSetup();

						return TRUE;

					}



					// If we've gotten a table sit-down message from the

					// server, display it now.  Otherwise, use our default

					// one.

					int result = IDOK;

					//kp(("%s(%d) Table sitdown message starts with: '%.20s'\n", _FL, t->MiscMsgSitdown.msg));

					if (t->MiscMsgSitdown.msg[0]) {

						result = MessageBox(hDlg, t->MiscMsgSitdown.msg,

								"Before taking your seat...",

								MB_OKCANCEL|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

					} else {

					  #if 0	//20000427MB: this code is now somewhat out of date.

						// If this is a real money table, remind them that this

						// is a real money table.

						if (t->chip_type == CT_REAL) {

							if (t->game_type==GAME_OMAHA || t->game_type==GAME_OMAHA_HI_LO) {

								result = MessageBox(hDlg,

									"You are reminded that this table is\n"

									"playing for REAL MONEY.\n\n"

									"3 ALL INs allowed per 24 hour period\n"

									"due to Disconnects or Time Outs. See\n"

									"web site for further details.\n\n"

									"To see all called hands on the river, request\n"

									"a hand history from the lobby screen.\n\n"

									"Please remember that in Omaha you must\n"

									"use exactly two of your pocket cards and\n"

									"combine them with exactly three board cards.\n"

									"See web site for further details.\n\n"

									"English only at the tables and no foul language.\n\n"

									"Thank you.",

									"Real Money reminder",

									MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

							} else {

								result = MessageBox(hDlg,

									"You are reminded that this table is\n"

									"playing for REAL MONEY.\n\n"

									"3 ALL INs allowed per 24 hour period\n"

									"due to Disconnects or Time Outs. See\n"

									"web site for further details.\n\n"

									"To see all called hands on the river, request\n"

									"a hand history from the lobby screen.\n\n"

									"English only at the tables and no foul language.\n\n"

									"Thank you.",

									"Real Money reminder",

									MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

							}

						}

					  #endif

					}

					if (result!=IDOK) {

						// They did not click 'OK'.

						return TRUE;

					}



					// we want there to only be one of these up on the screen at one time

					if (hBuyInDLG) { // there already is one

						SetTableKeyboardFocus(t);	// purely cosmetic

						MessageBox(hDlg,

							"Please complete buying in at other tables before sitting down",

							"Buying in on more than one table...",

							MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

						ReallySetForegroundWindow(hBuyInDLG);	// top the current buy-in dlg

						return TRUE;	// We DID process this message.

					}



					EnableSitDownButtons(t->table_serial_number, FALSE);

					// figure out minimum buy in allowed for this table at this moment

					int minimum_required;

					if (t->minimum_buy_in) {	// something there, use it

						minimum_required = t->minimum_buy_in;

					} else 	{

						minimum_required = t->GameCommonData.big_blind_amount*

							USUAL_MINIMUM_TIMES_BB_ALLOWED_TO_SIT_DOWN*

							GameStakesMultipliers[t->game_rules - GAME_RULES_START];

					}

					// for tournament tables, it's always small+big

					if (t->tournament_table) {

						minimum_required = 

							t->GameCommonData.big_blind_amount+t->GameCommonData.small_blind_amount;

					}

					

					// if he's almost out of chips and at the 3/6HE (or lower stakes),

					// let him buy in

					if (t->GameCommonData.big_blind_amount <= SMALLEST_STAKES_BIG_BLIND && 

						t->client_display_tab_index==DISPLAY_TAB_HOLDEM)

					{

						int chips_left;

						if (t->chip_type == CT_REAL) {

							chips_left = RealChipsInBank + RealChipsInPlay;

						} else {

							chips_left = FakeChipsInBank + FakeChipsInPlay;

						}

						minimum_required = min(minimum_required, chips_left);

						minimum_required = max(minimum_required, t->GameCommonData.big_blind_amount);

						// 20000128HK: round it down so extra pennies don't cause problems

						minimum_required -= minimum_required % 100;

					}

					// attempt to sit down
					if (!BuyIntoTable(JTS_JOIN, t, minimum_required, i)) {
						// false = we didn't buy in -- do some cosmetics
						EnableSitDownButtons(t->table_serial_number, TRUE);
					
						SetTableKeyboardFocus(t);	// purely cosmetic
					};/// attempt to sit down

					return TRUE;	// We DID process this message.

				}

			}

			// Process all other buttons on the window...

			switch (LOWORD(wParam)) {

		  #if 0	//20000303MB

			case IDC_CHAT_BOX:	// the chat display box

				kp(("%s(%d) mess = %08x, HIWORD(wParam) = %d, LOWORD(wParam) = %d, HIWORD(lParam) = %d, LOWORD(lParam) = %d\n",

						_FL, message, HIWORD(wParam),LOWORD(wParam),HIWORD(lParam),LOWORD(lParam)));

				switch (HIWORD(wParam)) {

				case EN_VSCROLL:

					kp(("%s(%d) EN_VSCROLL received for IDC_CHAT_BOX\n",_FL));

					break;

				}

				return TRUE;	// We DID process this message.

		  #endif

			case IDC_TYPE_CHAT:		// the chat input box

				switch (HIWORD(wParam)) {

				case EN_SETFOCUS:

					if (!Table[t->table_index].already_selected_chatbox) {

						Table[t->table_index].already_selected_chatbox = TRUE;

						SetDlgItemTextIfNecessary(hDlg, IDC_TYPE_CHAT, "");

					}

					break;

				case EN_UPDATE:

					// there's been a change in the contents of the chat box

					line_length = SendMessage((HWND)lParam, EM_LINELENGTH,0,0);

					line_count  = SendMessage((HWND)lParam, EM_GETLINECOUNT,0,0);

					if (!line_length && Table[t->table_index].currently_typing_chat) {	

						// top line is blank -- he's backspaced out

						// restore normal scrolling/updating

						Table[t->table_index].currently_typing_chat = FALSE;

						//Table[t->table_index].chat_scroll_disabled = FALSE;

						// ShowWindowIfNecessary((HWND)lParam, SW_HIDE);

						RefreshChatText(t->table_index);

						//SetFocus(hDlg);	// take focus away from here

						break;

					} 

					// there's something there -- act on it if <enter> was pressed

					if (line_count > 1) {

						// retrieve the text line

						char chat_msg[MAX_CHAT_MSG_LEN+1];

						chat_msg[0] = '\0';	// blank, we'll build from here

						// special character in front of dealer

						*(WORD *)tmp_chat_line = MAX_CHAT_MSG_LEN; // set buffer size

						int chars = SendMessage((HWND)lParam, EM_GETLINE,0,(LPARAM)tmp_chat_line);

						chars = min(chars, MAX_CHAT_MSG_LEN);

						tmp_chat_line[chars] = '\0';	// we need to null terminate it

						strcat(chat_msg, tmp_chat_line);

						// if the player edited his input and didn't return to the end of the

						// input line, the line is now split.. add the stuff on the 2nd line

						*(WORD *)tmp_chat_line = MAX_CHAT_MSG_LEN; // set buffer size

						chars = SendMessage((HWND)lParam, EM_GETLINE,1,(LPARAM)tmp_chat_line);

						chars = min(chars, MAX_CHAT_MSG_LEN);

						tmp_chat_line[chars] = '\0';	// we need to null terminate it

						if (strlen(chat_msg) + strlen(tmp_chat_line) < MAX_CHAT_MSG_LEN) {

							strcat(chat_msg, tmp_chat_line);

						}

						// filter out any special characters that may have been spoofed

						#define CHAT_SPECIAL_CHARACTER	187 // �

					  #if 0

						for(int j = 32; j < 255;j++) {

							kp(("** %d = %c *****\n",j,j));								

						}

					  #endif

						char *p = chat_msg;

						while (*p) {

							// no spoofing the dealer prompt

							if (*p == '�') {

								*p = '?';// turn it into a '?'

							}

							// change tabs and other control characters to spaces

							if (*p < ' ') {	// something less than ASCII 32 (space)

								*p = ' ';// turn it into a space

							}

							p++;

						}



						// if 

						// post the chat message (if there's something there)

						if (strlen(chat_msg)) {

							struct GameChatMessage gcm;

							zstruct(gcm);

							gcm.table_serial_number = t->table_serial_number;

							gcm.game_serial_number = t->GameCommonData.game_serial_number;

							// all chat text posted from players is of type CHATTEXT_PLAYER

							gcm.text_type = CHATTEXT_PLAYER;

							strnncpy(gcm.message, chat_msg, MAX_CHAT_MSG_LEN);

							SendDataStructure(DATATYPE_CHAT_MSG, &gcm, sizeof(gcm));

							// it's sent, restore scrolling

							// ShowWindow((HWND)lParam, SW_HIDE);

						}

						SendMessage((HWND)lParam, WM_SETTEXT,0,0);

						Table[t->table_index].currently_typing_chat = FALSE;

						//Table[t->table_index].chat_scroll_disabled = FALSE;

						RefreshChatText(t->table_index);

					}

					break;

				case EN_VSCROLL:

					//kp(("***** VSCROLL ***\n"));

					break;

				default:

					//kp(("??????? %d ????????\n", HIWORD(wParam)));

					break;

				}

				return TRUE;	// We DID process this message.



			case IDC_COMBO_CHAT_MODE:

				if (HIWORD(wParam)==CBN_SELCHANGE) {

					// Chat mode selection has changed...

					t->quiet_dealer = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

					Defaults.iQuietDealerType = t->quiet_dealer;

					Defaults.changed_flag = TRUE;

					//kp(("%s(%d) Detected chat mode change: new mode = %d\n", _FL, t->quiet_dealer));

					// refresh the text box

					RefreshChatText(t->table_index);

					SetTableKeyboardFocus(t);	// purely cosmetic

				}

				return TRUE;	// We DID process this message.

			case IDC_DEALER_CHIP_AREA:

				// If we're already playing, allow them to bring more chips to

				// the table.

			  #if INCLUDE_TABLE_OPTIONS	//20000315MB

				if (!hTableOptionsDlg) {

					HWND h = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TABLE_OPTIONS), NULL, dlgFuncTableOptions);

					//kp(("%s(%d) Setting USERDATA to $%08lx for window $%08lx\n", _FL, t, h));

					SetWindowLong(h, GWL_USERDATA, (long)t);

					ShowWindow(h, SW_SHOW);

					if (t->watching_flag) {

						EnableWindow(GetDlgItem(h, IDC_REQUEST_MORE_CHIPS), FALSE);

					}

				} else {

					ShowWindow(hTableOptionsDlg, SW_SHOWNORMAL);

					ReallySetForegroundWindow(hTableOptionsDlg);

				}

			  #else

					// if table options are disabled, do nothing

				}

			  #endif

				return TRUE;	// We DID process this message.

			case IDC_LEAVE_TABLE:

				HandleLeaveTableButton(t);

				return TRUE;	// We DID process this message.

			case IDC_GOTO_CARDROOM:

				PostMessage(hInitialWindow, WMP_SHOW_CARDROOM, 0, 0);

				return TRUE;	// We DID process this message.

			case IDC_SIT_OUT:

				t->ClientState.sitting_out_flag = (BYTE8)IsDlgButtonChecked(hDlg, IDC_SIT_OUT);

				if (!t->ClientState.sitting_out_flag) {

					//kp(("%s(%d) changing t->forced_to_sit_out_flag from %d to FALSE.\n", _FL, t->forced_to_sit_out_flag));

					t->forced_to_sit_out_flag = FALSE;	// cleared manually. clear flag indicating the server forced us.

				}

				pr(("%s(%d) Detected 'Sit out' button pressed. New state = %d\n",_FL,t->sitting_out_flag));

				// Tell the server our new state.

				SendClientStateInfo(t);

				// If they set the check box, answer any pending GPIRequests

				// that had sit out as an option.

				// See also comm.cpp where the input requests come in.

				// There's similar code that may need updating in

				// parallel to this code.

				if (t->ClientState.sitting_out_flag) {

					// 20000626HK: can't be waiting for bb if we're sitting out

					t->wait_for_bb = FALSE;

					if (t->button_1_alternate_meaning==3) {	// I'm back?

						t->button_1_alternate_meaning = 0;	// clear it.

					}

					if (t->chip_type==CT_TOURNAMENT) {

						if (t->GamePlayerInputRequest.game_serial_number) {

							if (t->GamePlayerInputRequest.action_mask & (1<<ACT_CHECK)) {

								// Check is an option.  Do it.

								ProcessGPIResult(t, ACT_CHECK);
							

							} else if (t->GamePlayerInputRequest.action_mask & (1<<ACT_FOLD)) {

								// Fold is an option.  Do it.

								ProcessGPIResult(t, ACT_FOLD);
								

							}

						}

					} else {

						for (int j=0; j<MAX_BUTTONS_PER_TABLE ; j++) {

							// Regular table: treat as a sit out checkbox.

							if (t->button_actions[j] == ACT_SIT_OUT_SB   ||

								t->button_actions[j] == ACT_SIT_OUT_BB   ||

								t->button_actions[j] == ACT_SIT_OUT_POST ||

								t->button_actions[j] == ACT_SIT_OUT_BOTH ||

								t->button_actions[j] == ACT_SIT_OUT_ANTE)

							{

								ProcessGPIResult(t, t->button_actions[j]);
							

							}

						}

					}

				}

				// Enable/disable the IDC_POST_IN_TURN checkbox depending on whether

				// we're sitting out or not.

				ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POST_IN_TURN),

						(t->ClientState.sitting_out_flag ||

						 t->zero_ante_stud_flag ||

						 t->wait_for_bb) ? SW_HIDE : SW_SHOWNA);

			  #if 0	//20000908MB

				// Invalidate the checkbox so that it redraws properly

				// with the bitmapped background.

				InvalidateRect(GetDlgItem(hDlg, LOWORD(wParam)), NULL, FALSE);

			  #else	// redraw the whole table (other things may have changed)

				FlagRedrawTable(t->table_index);

			  #endif

				return TRUE;	// We DID process this message.



			case IDC_SNACK_MENU:

				OpenSnackMenuWindow(t);

				return TRUE;	// We DID process this message



		  #if ADMIN_CLIENT

			case IDC_COMPUTER_PLAY:

				t->computer_play = IsDlgButtonChecked(hDlg, IDC_COMPUTER_PLAY);

				pr(("%s(%d) Detected 'computer play' button pressed. New state = %d\n",_FL,t->computer_play));

				// Invalidate the checkbox so that it redraws properly

				// with the bitmapped background.

				InvalidateRect(GetDlgItem(hDlg, LOWORD(wParam)), NULL, FALSE);

				return TRUE;	// We DID process this message.

			  #if MIN_HAND_RANK_POPUP

				case IDC_MIN_HAND_RANK:

					t->pop_up_on_hand_rank = IsDlgButtonChecked(hDlg, IDC_MIN_HAND_RANK);

					{

						char str[5];

						zstruct(str);

						GetDlgItemText(hDlg, IDC_MIN_HAND_RANK_EDIT, str, 5);

						t->pop_up_on_hand_rank_number = atoi(str);

					}

					pr(("%s(%d) Detected 'pop up rank' change. New state = %d with rank %d\n",

						_FL,t->pop_up_on_hand_rank, t->pop_up_on_hand_rank_number));

					// Invalidate the checkbox so that it redraws properly

					// with the bitmapped background.

					InvalidateRect(GetDlgItem(hDlg, LOWORD(wParam)), NULL, FALSE);

					return TRUE;	// We DID process this message.

			  #endif

		  #endif

			case IDC_CHAT_LIST:

				pr(("%s(%d) IDC_CHAT_LIST received.  We don't know what to do about that yet.\n",_FL));

				break;

			case IDC_MUCK_LOSING_HANDS:

				t->ClientState.muck_losing_hands = (BYTE8)IsDlgButtonChecked(hDlg, IDC_MUCK_LOSING_HANDS);

				//kp(("%s(%d) t->ClientState.muck_losing_hands = %d\n", _FL, t->ClientState.muck_losing_hands));

				SendClientStateInfo(t);

				return TRUE;	// We DID process this message.

			case IDC_FOLD_IN_TURN:

			case IDC_FOLD_IN_TURN2:

			case IDC_CALL_IN_TURN:

			case IDC_CALL_IN_TURN2:

			case IDC_RAISE_IN_TURN:

			case IDC_RAISE_IN_TURN2:

				// If the button is not visible, there's no way we should

				// allow it to be acted on.

				pr(("%s(%d) Got IDC_*_IN_TURN* (%d)... checked = %d, visible=%d\n",

						_FL, button, IsDlgButtonChecked(hDlg, button),

						IsWindowVisible(GetDlgItem(hDlg, button))));

				if (IsWindowVisible(GetDlgItem(hDlg, button)))

				{

					// 20000625HK: handling for wait_for_bb

					if (t->wait_for_bb && LOWORD(wParam) == _IDC_WAIT_FOR_BB) {

						// probably unchecked "Wait for BB"

						t->wait_for_bb = FALSE;

						// get rid of it

						CheckDlgButtonIfNecessary(hDlg, _IDC_WAIT_FOR_BB,  FALSE);

						FlagRedrawTable(t->table_index);		// redraw our window.

						return TRUE; // don't bother with the rest

					}

			

					// If they checked it, uncheck any others.

					if (IsDlgButtonChecked(hDlg, button)) {

						// Something got checked.

						CheckDlgButtonIfNecessary(hDlg, IDC_FOLD_IN_TURN,  button==IDC_FOLD_IN_TURN);

						CheckDlgButtonIfNecessary(hDlg, IDC_FOLD_IN_TURN2, button==IDC_FOLD_IN_TURN2);

						CheckDlgButtonIfNecessary(hDlg, IDC_CALL_IN_TURN,  button==IDC_CALL_IN_TURN);

						CheckDlgButtonIfNecessary(hDlg, IDC_CALL_IN_TURN2, button==IDC_CALL_IN_TURN2);

						CheckDlgButtonIfNecessary(hDlg, IDC_RAISE_IN_TURN, button==IDC_RAISE_IN_TURN);

						CheckDlgButtonIfNecessary(hDlg, IDC_RAISE_IN_TURN2,button==IDC_RAISE_IN_TURN2);

						switch (button) {

						case IDC_FOLD_IN_TURN:	// fold/check if possible

							t->ClientState.in_turn_action = ACT_FOLD;

							t->ClientState.in_turn_action_amount = 0;

							break;

						case IDC_FOLD_IN_TURN2:	// fold no matter what

							t->ClientState.in_turn_action = ACT_FOLD;

							t->ClientState.in_turn_action_amount = -1;

							break;

						case IDC_CALL_IN_TURN:	// call/check any amount

							t->ClientState.in_turn_action = ACT_CALL;

							t->ClientState.in_turn_action_amount = -1;

							break;

						case IDC_CALL_IN_TURN2:	// call/check a certain amount

							t->ClientState.in_turn_action = ACT_CALL;

							t->ClientState.in_turn_action_amount = t->GamePlayerData.call_amount;

							break;

						case IDC_RAISE_IN_TURN:	// bet/raise any amount

							t->ClientState.in_turn_action = ACT_RAISE;

							t->ClientState.in_turn_action_amount = -1;

							break;

						case IDC_RAISE_IN_TURN2:// bet/raise a certain amount

							t->ClientState.in_turn_action = ACT_RAISE;

							t->ClientState.in_turn_action_amount = t->GamePlayerData.raise_amount;

							break;

						}

					} else {

						// Something got unchecked...

						t->ClientState.in_turn_action = 0;	// clear it by default.

						t->ClientState.in_turn_action_amount = 0;

						InvalidateRect(GetDlgItem(hDlg, button), NULL, FALSE);

					}

					t->ClientState.in_turn_action_game_serial_number = t->GameCommonData.game_serial_number;

					t->ClientState.in_turn_action_game_state = t->GamePlayerData.game_state;

					t->ClientState.in_turn_action_last_input_request_serial_number = t->last_input_request_serial_number;

					FlagRedrawTable(t->table_index);		// redraw our window.

					SendClientStateInfo(t);

					// No more processing for this button press.

				}

				pr(("%s(%d) done processing IDC_*_IN_TURN* (%d)... checked = %d.  in_turn_action is now %d\n", _FL, button, IsDlgButtonChecked(hDlg, button), t->ClientState.in_turn_action));

				return TRUE;	// We DID process this message.



			case IDC_POST_IN_TURN:

				if (t && t->tournament_table) {

					// post in turn must always be checked in tournament mode.

					CheckDlgButtonIfNecessary(hDlg, IDC_POST_IN_TURN, TRUE);

					return TRUE;	// We DID process this message

				}



				t->ClientState.post_in_turn = (BYTE8)IsDlgButtonChecked(hDlg, IDC_POST_IN_TURN);

				pr(("%s(%d) Detected 'Post in turn' button pressed. New state = %d\n",_FL,t->ClientState.post_in_turn));

				

				// Tell the server our new state.

				// 20000626HK: filter out telling the server to autopost if we're waiting for bb

				if (!t->wait_for_bb) {

					t->ClientState.post_in_turn_game_serial_number = t->GameCommonData.game_serial_number;

					SendClientStateInfo(t);

				}

				// If they set the check box, answer any pending GPIRequests

				// that had post as an option.

				// See also comm.cpp where the input requests come in.

				// There's similar code that may need updating in

				// parallel to this code.

				if (t->ClientState.post_in_turn) {

					for (int j=0; j<MAX_BUTTONS_PER_TABLE ; j++) {

						if (t->button_actions[j] == ACT_POST ||

							t->button_actions[j] == ACT_POST_SB ||

							t->button_actions[j] == ACT_POST_BB ||

							t->button_actions[j] == ACT_POST_BOTH ||

							t->button_actions[j] == ACT_POST_ANTE) {

								ProcessGPIResult(t, t->button_actions[j]);

						}

					}

				}

				// Invalidate the checkbox so that it redraws properly

				// with the bitmapped background.

				InvalidateRect(GetDlgItem(hDlg, LOWORD(wParam)), NULL, FALSE);

				return TRUE;	// We DID process this message.

			}

		}

		break;

	case WM_DESTROY:

		{

			pr(("%s(%d) Got WM_DESTROY\n",_FL));

			RemoveKeyboardTranslateHwnd(hDlg);

        	KillTimer(hDlg, WM_TIMER);	// remove our timer.

			// Save this window position...

			char table_name[30];

			sprintf(table_name, "Table %d Position", table_index+1+iProgramInstanceCount);

			WinStoreWindowPos(ProgramRegistryPrefix, table_name, hDlg, NULL);

			EnterCriticalSection(&CardRoomVarsCritSec);

			CloseToolTipWindow(t->ToolTipHwnd);		// Close our tooltip window

			EnterCriticalSection(&t->draw_crit_sec);

			DeleteObject(t->hbm_off_screen);

			t->hbm_off_screen = 0;

			t->hwnd = NULL;	// make sure everyone else thinks this table is empty now

			t->table_serial_number = 0;

			LeaveCriticalSection(&t->draw_crit_sec);

			PPDeleteCriticalSection(&t->draw_crit_sec);

			PPDeleteCriticalSection(&t->chat_crit_sec);

			zstruct(Table[table_index]);

			UpdateTableMenuItems();

			BuildColorStaticTable();

			LeaveCriticalSection(&CardRoomVarsCritSec);

			pr(("%s(%d) Finished handling WM_DESTROY\n",_FL));



			if (hTableOptionsDlg) {

				DestroyWindow(hTableOptionsDlg);

			}

		}

		return TRUE;	// We DID process this message.

	case WM_DRAWITEM:

		// Owner draw control... draw it.

		HandleTableDrawItem(t, (int)wParam, (LPDRAWITEMSTRUCT)lParam);

		return TRUE;	// We DID process this message.

	case WM_INITMENU:

		// Disable the maximize and size menu items on our system menu.

		EnableMenuItem((HMENU)wParam, SC_MAXIMIZE, MF_BYCOMMAND|MF_GRAYED);

		EnableMenuItem((HMENU)wParam, SC_SIZE,     MF_BYCOMMAND|MF_GRAYED);

		return TRUE;	// We DID process this message.



  #if 0	//20000307MB: these do not appear to be received at present.

	case WM_KEYDOWN:

		kp(("%s(%d) WM_KEYDOWN received.\n",_FL));

		return TRUE;	// We DID process this message.

  #endif



	case WM_LBUTTONDOWN:

	case WM_NCLBUTTONDOWN:

		// Left mouse button was pressed... always try to top ourselves.

		PostMessage(hDlg, WMP_TOP_YOURSELF_IF_NECESSARY, 0, 0);

		return FALSE;	// treat as if we didn't process this message.



  #if 1	//20000627MB

	case WM_MOUSEMOVE:

		if (t) {

			HWND old_highlight = t->proximity_highlight_list.hCurrentHighlight;

			UpdateMouseProximityHighlighting(&t->proximity_highlight_list, hDlg, MAKEPOINTS(lParam));

			if (old_highlight &&

				old_highlight != t->proximity_highlight_list.hCurrentHighlight)

			{

				// Something became unhighlighted... force a full redraw

				FlagRedrawTable(t->table_index);

			}

		}

		return TRUE;	// TRUE = we did process this message.

  #endif



  #if 0	//20000627MB

	case WM_MOUSEACTIVATE:

		kp(("%s(%d) %4d: Received WM_MOUSEACTIVATE for window %d\n", _FL, SecondCounter, t->table_serial_number));

		break;	// handle normally

  #endif

/*	

	case WM_MOUSEACTIVATE:

		// t->chat_scroll_disabled = lParam;	// set to non-zero (TRUE) and tell us where

		break;

*/

  #if ADMIN_CLIENT

	case WM_RBUTTONUP:



		if ( LoggedInPrivLevel > 20  ) {     // ACCPRIV_ADMINISTRATOR = 40



				// Bring up player edit dialog when right clicked on player id box

				if (iAdminClientFlag) {

					POINTS pts = MAKEPOINTS(lParam);

					POINT pt;

					pt.x = pts.x;

					pt.y = pts.y;

					RECT r;

					zstruct(r);

					GetClientRect(hDlg, &r);

					POINT ptc = pt;

					ScreenToClient(hDlg, &ptc);	// ptc should contain the point in client coordinates

					if (hDlg==GetActiveWindow() && PtInRect(&r, pt)) {

						HWND find_hwnd = ChildWindowFromPointEx(hDlg, pt, CWP_SKIPINVISIBLE);

						for (int i=0; i < MAX_PLAYERS_PER_GAME; i++) {

							if (find_hwnd == GetDlgItem(hDlg, PlayerTextIDs[i])) {

								AutoLookupPlayerID = t->GameCommonData.player_id[i];

								break;

							}

						}

					}

					if (AutoLookupPlayerID) {

						PostMessage(hCardRoomDlg, WM_COMMAND, (WPARAM)IDM_EDIT_USER_ACCOUNTS, (LPARAM)0);

					}

				}

			}

		break;

  #endif



  #if ADMIN_CLIENT

	case WM_NOTIFY:

		{

			//kp(("%s(%d) Got WM_NOTIFY\n",_FL));

			NMUPDOWN *n = (LPNMUPDOWN)lParam;

			if (n->hdr.hwndFrom==GetDlgItem(hDlg, IDC_COMPUTER_PLAY_SECS_SPIN)) {

				t->computer_play_seconds = n->iPos + n->iDelta;

				t->computer_play_seconds = max(0,t->computer_play_seconds);

				//kp(("%s(%d) Got notify for spin control. Seconds = %d\n",_FL, t->computer_play_seconds));

				break;

			}

	      #if MIN_HAND_RANK_POPUP

			if (n->hdr.hwndFrom==GetDlgItem(hDlg, IDC_MIN_HAND_RANK_SPIN)) {

				t->pop_up_on_hand_rank_number = n->iPos + n->iDelta;

				t->pop_up_on_hand_rank_number = max(1, t->pop_up_on_hand_rank_number);

				t->pop_up_on_hand_rank_number = min(8, t->pop_up_on_hand_rank_number);

			}

			pr(("%s(%d) Detected 'pop up rank' change. New state = %d with rank %d\n",

				_FL,t->pop_up_on_hand_rank, t->pop_up_on_hand_rank_number));

		  #endif

		}

		break;

  #endif

	case WM_PAINT:

		// If we've got an off-screen bitmap to draw from, paint it first.

		//kp(("%s(%d) Got WM_PAINT\n",_FL));

		{

			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && t->hbm_off_screen) {

				EnterCriticalSection(&t->draw_crit_sec);

				if (t->hpalette) {

					SelectPalette(hdc, t->hpalette, FALSE);

					RealizePalette(hdc);

				}

				BlitBitmapToDC_nt(t->hbm_off_screen, hdc);
				LeaveCriticalSection(&t->draw_crit_sec);

			}

           
			EndPaint(hDlg, &ps);

		}

		return TRUE;	// We DID process this message.

	case WM_QUERYNEWPALETTE:

	case WM_PALETTECHANGED:	// palette has changed.

		//kp(("%s(%d) Table got WM_PALETTECHANGED\n", _FL));

	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us

		    HDC hdc = GetDC(hDlg);

		    HPALETTE holdpal = SelectPalette(hdc, t->hpalette, FALSE);

		    int i = RealizePalette(hdc);   // Realize drawing palette.

			//kp(("%s(%d) Palette changed = %d\n", _FL, i));

		    if (i) {	// Did the realization change?

		        InvalidateRect(hDlg, NULL, TRUE);    // Yes, so force a repaint.

		    }

		    SelectPalette(hdc, holdpal, TRUE);

		    RealizePalette(hdc);

		    ReleaseDC(hDlg, hdc);

		    return i;

	    }

		return TRUE;	// We DID process this message.



	// notes on the chat edit-box scrolling:  it seems to be a little tricky trapping messages

	// for the scroll bar inside an editbox inside this dialog -- the notifications don't directly

	// make it out this far.  here's how it's working:  when the thumb is pressed, we get a

	// WM_MOUSEACTIVATE with a cursor position.  when the corresponding WM_SETCURSOR (with that

	// lParam having changed) we know the thumb has been released

/*	case WM_SETCURSOR:

		//kp(("END SCROLL : %d / %d\n", (int)wParam, (int)lParam));

		if (!Table[t->table_index].currently_typing_chat && 

				t->chat_scroll_disabled && lParam != t->chat_scroll_disabled) {

			t->chat_scroll_disabled = FALSE;

			// RefreshChatText(table_index);

		}

		break;*/

	case WM_SYSCOMMAND:	// system menu commands...

		switch (wParam) {

		case IDC_DEALER_CHIP_AREA:	// reflect as a regular WM_COMMAND

			PostMessage(hDlg, WM_COMMAND, wParam, lParam);

			return TRUE;	// we DID process this message.

		}

		break;

    case WM_TIMER:

		// If the blinking player id box changed state, flag a redraw right now.
		

		if (t->blinking_player_flag) {
			BYTE8 highlight_flag = (BYTE8)DetermineCurrentPlayerHighlightState(t);

			pr(("%s(%d) highlight_flag = %d, blinking status = %d\n", _FL, highlight_flag, t->blinking_player_status));

			if (highlight_flag != t->blinking_player_status) {

				t->redraw_controls_time = SecondCounter;	// do it now.

				pr(("%s(%d) Redraw control now\n",_FL, t->redraw_controls_time));

			}

		}



		// If a count-down to redrawing the controls was set, check it now.

		if (t->redraw_controls_time && SecondCounter >= t->redraw_controls_time) {

			t->redraw_controls_time = 0;

			//kp(("%s(%d) Calling FlagRedrawTable()\n",_FL));

			FlagRedrawTable(t->table_index);

		}

	  #if ADMIN_CLIENT

		if (t->actions_hidden_ticks && t->actions_hidden_ticks > GetTickCount()) {

			FlagRedrawTable(t->table_index);

		}

	  #endif



        if (!t->animation_disable_flag) {   // only update if not minimized

	        if (t->animation_flag) {    	// are we animating anything?

	            RedrawAllTableGraphics(t);  // redraw all graphics in new positions

	        }

		}

		UpdateLastActionIndicator(t);	// update last action indicator if necessary



		// Check if the chat scroll position changed...

		if (SecondCounter != t->chat_last_scroll_position_check) {

			t->chat_last_scroll_position_check = SecondCounter;

			int new_position = SendMessage(GetDlgItem(hDlg, IDC_CHAT_BOX), EM_GETFIRSTVISIBLELINE, 0, 0);

			if (new_position != t->chat_scroll_position) {

				t->chat_scroll_position = new_position;

				t->chat_last_user_scroll_time = SecondCounter;

				//kp(("%s(%d) Chat window was just scrolled to line %d\n", _FL, t->chat_scroll_position));

			}

		};//if (SecondCounter != t->chat_last_scroll_position_check) 



		// Check if it's time to pop up the tourn summary email request

		if (t->tourn_summary_request_time && SecondCounter >= t->tourn_summary_request_time) {

			// 20000910HK: pop up with cardroom as owner so we don't lose it if the table closes

			// on 2nd thought, better that the table owns it -- it's ok as long as the table

			// doesn't vanish right away

		  #if 1	

			AskRequestTournamentEmail(hDlg, &t->misc_client_msg);

		  #else

			AskRequestTournamentEmail(hCardRoomDlg, &t->misc_client_msg);

		  #endif

			t->tourn_summary_request_time = 0;

		};//if (t->tourn_summary_request_time && SecondCounter



		//20000616MB: If we're watching and this window is not active, and

		// has not been active for a while, close it.  This prevents people

		// from accidently leaving windows open in the background and tying up

		// bandwidth.

		if (t->watching_flag &&

			!t->active_table_window &&

			SecondCounter - t->last_activated_time > 8*60

		) {

		  #if ADMIN_CLIENT

			if (iAdminClientFlag) {

				kp(("%s %s(%d) In non-admin we would have closed table %d due to inactivity.\n",

							TimeStr(), _FL, t->table_serial_number));

				t->last_activated_time = SecondCounter;	// keep track of when last activated/deactivated

			} else {

				HandleLeaveTableButton(t, FALSE);

			}//if iAdminClientFlag

		  #else

			HandleLeaveTableButton(t, FALSE);

		  #endif //ADMIN_CLIENT

		};//if (t->watching_flag &&....



		//20000627MB: if it's time to check for any windows that need to be

		// topped due to action buttons being displayed, check for that here.

		if (dwCheckForInputButtons_Ticks && GetTickCount() >= dwCheckForInputButtons_Ticks) {

			dwCheckForInputButtons_Ticks = 0;	// reset

			// If the most recently topped one has action buttons, top it.

			if (LastTableWindowTopped && CheckIfActionButtonsShowing(LastTableWindowTopped)) {

				//kp(("%s(%d) %4d: Calling ShowTableWindowIfPlaying(%d)\n", _FL, SecondCounter, LastTableWindowTopped->table_serial_number));

				ShowTableWindowIfPlaying(LastTableWindowTopped);

			} else {

				// Scan for any windows with action buttons.  Top the first one

				// (even if it's not us).

				for (int i=0 ; i<MAX_TABLES ; i++) {

					if (CheckIfActionButtonsShowing(&Table[i])) {

						//kp(("%s(%d) %4d: Calling ShowTableWindowIfPlaying(%d)\n", _FL, SecondCounter, Table[i].table_serial_number));

						ShowTableWindowIfPlaying(&Table[i]);

						break;

					}//if

				}//for

			}// if (LastTableWindowTopped && CheckIfActionButtonsShowing(LastTableWindowTopped))

		};//if(dwCheckForInputButtons_Ticks && GetTickCount() >= dwCheckForInputButtons_Ticks)



	  #if ADMIN_CLIENT

		// Evaluate computer play if it's time...

		// If we're still dealing cards, delay our answer.

		if (t->dealing_flag && t->computer_play_answer_time) {

			t->computer_play_answer_time = SecondCounter + t->computer_play_answer_delay;

		};//if

		if (t->GamePlayerInputRequest.game_serial_number &&

				t->computer_play_answer_time &&

				SecondCounter >= t->computer_play_answer_time) {

		  #if 0	// very agressive play: good for one on one against other computers

			struct GamePlayerInputRequest *gpir = &t->GamePlayerInputRequest;

			struct GamePlayerInputResult *result = &t->GamePlayerInputResult;

			ActionType action = ACT_NO_ACTION;

		  	if (gpir->action_mask & (1<<ACT_BET)) {

		  		action = ACT_BET;

		  	} else if (gpir->action_mask & (1<<ACT_RAISE)) {

		  		action = ACT_RAISE;

		  	};//if (gpir->action_mask & (1<<ACT_BET)) 

			if (action != ACT_NO_ACTION) {

				// Now that we've picked an answer, fill in the input result.

				zstruct(*result);

				result->game_serial_number			= gpir->game_serial_number;

				result->table_serial_number			= gpir->table_serial_number;

				result->input_request_serial_number	= gpir->input_request_serial_number;

				result->seating_position			= gpir->seating_position;

				result->action = (BYTE8)action;

				result->ready_to_process = TRUE;

			} else {

				EvalComputerizedPlayerInput(&(t->GameCommonData),

					&(t->GamePlayerData), &(t->GamePlayerInputRequest),

					&(t->GamePlayerInputResult));

			};//(action != ACT_NO_ACTION)

		  #else	
			EvalComputerizedPlayerInput(&(t->GameCommonData),

				&(t->GamePlayerData), &(t->GamePlayerInputRequest),

				&(t->GamePlayerInputResult));
			

		  #endif.// very agressive play: good for one on one against other computers


			t->computer_play_answer_time = 0;

			zstruct(t->GamePlayerInputRequest);	// clear request immediately after processing an input

			SendGamePlayerInputResult(&(t->GamePlayerInputResult));	// send it to server

			FlagRedrawTable(t->table_index);	// redraw our window.

		};//if

	  #endif //admin client

	  #if 0

	  	kp1((ANSI_WHITE_ON_GREEN"%s(%d) WARNING: entering comm critsec and sleeping from WM_TIMER to look for bugs!\n", _FL));

		Sleep(50);

		//kp(("%s(%d) Critical sections owned by this thread at WM_TIMER:\n",_FL));

		//DisplayOwnedCriticalSections();

		EnterCriticalSection(&CardRoomVarsCritSec);

		EnterCriticalSection(&t->chat_crit_sec);

		EnterCriticalSection(&t->draw_crit_sec);

		EnterCriticalSection(&ClientSockCritSec);

		Sleep(50);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&t->draw_crit_sec);

		LeaveCriticalSection(&t->chat_crit_sec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		Sleep(50);

	  #endif

		return TRUE;	// TRUE = we DID process this message.

	case WM_ERASEBKGND:

		if (IsIconic(hDlg) || !t->hbm_background) {

			return FALSE;	// FALSE = we did NOT erase the background ourselves

		}

		//kp(("%s(%d) table got WM_ERASEBKGND... returning we DID erase the background ourselves.\n", _FL));

		return TRUE;	// TRUE = we DID erase the background ourselves

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_VSCROLL:

		//kp(("%s(%d) WM_VSCROLL received.\n",_FL));

		break;

	case WM_WINDOWPOSCHANGING:

		{

			WINDOWPLACEMENT wp;

			zstruct(wp);

			wp.length = sizeof(wp);

			GetWindowPlacement(hDlg, &wp);

			int old_min_flag = t->minimized_flag;

			if (wp.showCmd == SW_HIDE ||

				wp.showCmd == SW_MINIMIZE ||

				wp.showCmd == SW_SHOWMINIMIZED) {

				t->minimized_flag = TRUE;

			} else {

				t->minimized_flag = FALSE;

			}

			pr(("%s(%d) WM_WINDOWPOSCHANGED (showCmd=%d).  new t->minimized_flag = %d\n", _FL, wp.showCmd, t->minimized_flag));

			if (old_min_flag && !t->minimized_flag) {

				// Always redraw when no longer minimized.

				FlagRedrawTable(table_index);

			}

		}

		return TRUE;	// TRUE = we did process this message.

	case WMP_FLUSH_DEALER_CHAT_BUFFER:

		// flush buffered dealer text

		if (FlushDealerChatBuffer(t->table_index)) {

			// Something got flushed... update the display

			RefreshChatText(t->table_index);

		}

		return TRUE;	// TRUE = we did process this message.

	case WMP_REFRESH_CHAT:

		RefreshChatText(t->table_index);
		

		return TRUE;	// TRUE = we did process this message.

	case WMP_SET_CHAT_FOCUS:

	  #if 1	//20000918MB

		SetTableKeyboardFocus(t);	// purely cosmetic

	  #else

		SetFocus(GetDlgItem(t->hwnd, IDC_TYPE_CHAT));

	  #endif

		return TRUE;			// we DID process this message.

	case WMP_UPDATE_YOURSELF:	// application defined message: update display of our dialog box
		RedrawTable(table_index);

		return TRUE;			// we DID process this message.

	case WMP_CLOSE_YOURSELF:	// application defined message: close our dialog box

		pr(("%s(%d) Got WMP_CLOSE_YOURSELF ($%04x)... calling DestroyWindow()\n",_FL, WMP_CLOSE_YOURSELF));

		DestroyWindow(hDlg);	// destroy ourselves.

		pr(("%s(%d) Back from DestroyWindow().\n",_FL));

		return TRUE;			// we DID process this message.

	case WMP_MISC_CLIENT_MSG:

		ShowMiscClientMsg(hDlg, &t->misc_client_msg, NULL);

		return TRUE;	// TRUE = we did process this message.

	case WMP_REQUEST_TOURNAMENT_EMAIL:

	  t->tourn_summary_request_time = SecondCounter + 5;	// pop it up in 5 seconds

		return TRUE;

	

	case WMP_BUY_MORE_CHIPS:

		{

			struct BuyInDLGInfo *bi = ((BuyInDLGInfo *)(lParam));

			// t->minimum_buy_in was set when this message was posted

			BuyIntoTable(JTS_REBUY, bi->ti, t->minimum_buy_in, bi->seating_position);
         
			if (bi) {

				free(bi);	// allocated in comm.cpp

			}

		}

		return TRUE;	// TRUE = we did process this message.

	case WMP_TOP_YOURSELF_IF_NECESSARY:

		BringWindowToTop(hDlg);

		return TRUE;	// TRUE = we did process this message.

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

//
//

// Find a open table window's index given the table serial number.

// Returns -1 if not found.

//

int TableIndexFromSerialNumber(WORD32 table_serial_number)

{

	EnterCriticalSection(&CardRoomVarsCritSec);

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd && Table[i].table_serial_number==table_serial_number) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return i;	// found it.

		}

	}

	LeaveCriticalSection(&CardRoomVarsCritSec);

	return -1;	// not found.

}



//*********************************************************

//
//

// Take a game serial number and return the appropriate pointer into

// the Table[] array.  Returns NULL if the table index could not be found.

// See also TableIndexFromSerialNumber().

//

struct TableInfo *TablePtrFromSerialNumber(WORD32 table_serial_number)

{

	int i = TableIndexFromSerialNumber(table_serial_number);

	if (i >= 0) {

		return &Table[i];

	}

	return NULL;

}	



//*********************************************************

//
//

// Count the number of open table windows.

//

int OpenTableCount(void)

{

	int count = 0;

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd)

			count++;

	}

	return count;

}



//*********************************************************

//
//

// Hook procedure for WH_CALLWNDPROC.  Used to trap BM_SETSTATE

// and BM_SETCHECK messages for checkbox windows so that WM_CTLCOLORSTATIC

// can draw them properly.

// Checkboxes must be invalidated when those messages arrive so that the

// text gets redrawn (rather than just the background).

//

LRESULT CALLBACK ColorStaticHookProc(int nCode, WPARAM wParam, LPARAM lParam)

{

    if (nCode >= 0) {

	    MSG *msg = (MSG *)lParam;

		// Check if this is one of the messages we're watching for...

		switch (msg->message) {

		case BM_SETSTATE:

		case BM_SETCHECK:

		case WM_LBUTTONUP:

		case WM_LBUTTONDOWN:

		case WM_LBUTTONDBLCLK:

		case WM_KEYUP:

		case WM_KEYDOWN:

			//kp(("%s(%d) Got a particular msg subset... scanning %d hwnd's\n", _FL, iColorStaticWindowCount));

			for (int i=0 ; i<iColorStaticWindowCount ; i++) {

				if (msg->hwnd==hColorStaticWindows[i]) {	// found a match!

					//kp(("%s(%d) Found match. Invalidating the window.\n",_FL));

					InvalidateRect(msg->hwnd, NULL, FALSE);

					break;

				}

			}

			break;

		}



	  #if 0	//19990827MB

		// Search our array to see if hwnd is in it.  If so, invalidate it.

		//kp(("%s(%d) Got message %d... scanning %d hwnd's\n", _FL, msg->message, iColorStaticWindowCount));

		for (int i=0 ; i<iColorStaticWindowCount ; i++) {

			if (msg->hwnd==hColorStaticWindows[i]) {	// found a match!

				kp(("%s(%d) Got message %d... for window $%08lx. Invalidating window.\n",_FL, msg->message,msg->hwnd));

				InvalidateRect(msg->hwnd, NULL, FALSE);

				break;

			}

		}

	  #endif

    }

	return CallNextHookEx(hColorStaticHook, nCode, wParam, lParam);

}	



//*********************************************************

//
//

// Update the title bars for all open tables

//

void UpdateTableWindowTitles(void)

{

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd) {

			UpdateTableWindowTitle(&Table[i]);

		}

	}

}



//*********************************************************

//
//

// Top/show a table window if we're currently playing a game

// at that table.  DON'T do it if a DIFFERENT table window

// already has action buttons showing.

//

void ShowTableWindowIfPlaying(struct TableInfo *t)

{

	if (CheckIfPlayingCurrentGame(t)) {

		// Check if any OTHER tables have action buttons showing already.

		int i;

		for (i=0 ; i<MAX_TABLES ; i++) {

			if (&Table[i] != t) {	// it's not us...

				if (CheckIfActionButtonsShowing(&Table[i])) {

					// someone else has action buttons showing. Don't top.

					//kp(("%s(%d) %4d: Table %d has buttons showing; aborting WMP_SHOW_TABLE_WINDOW posting for table %d.\n", _FL, SecondCounter, Table[i].table_serial_number, t->table_serial_number));

					return;

				}

			}

		}



		// We're going to top the window.



		if (CountPlayingTables() > 1) {

			// If the button areas of two playing windows overlap and the new window

			// is not already at the top of the z-order (between the two), then

			// the new window will be coming up in front of the old window and it's

			// possible the user was going to press an in-turn action on the old window.

			// If that's the case, ignore button presses on the new window for

			// a short period of time.



			HWND hwnd = FindTopTableWindow();

			if (hwnd != t->hwnd) {	// It's not already us...

				// Compare coordinates and see if there's any overlap in the button area.

				//kp(("%s(%d) %4d: Topping a new table window.\n", _FL, SecondCounter));

				RECT r1, r2;

				zstruct(r1);

				zstruct(r2);

				GetWindowRect(hwnd, &r1);

				GetWindowRect(t->hwnd, &r2);

				int xdiff = abs(r1.left - r2.left);

				int ydiff = abs(r1.top - r2.top);

				//kp(("%s(%d) xdiff = %d, ydiff = %d\n", _FL, xdiff, ydiff));

				if (xdiff <= 466 && ydiff <= 42) {

					// the button areas overlap at least a little bit.

					//kp(("%s(%d) NOTE: ignoring buttons for 400ms\n", _FL));

					dwIgnoreInputButtons_Ticks = GetTickCount() + 400;

				}

			}

		}



		dwCheckForInputButtons_Ticks = 0;	// reset

		//kp(("%s(%d) %4d: posting WMP_SHOW_TABLE_WINDOW for window %d\n", _FL, SecondCounter, t->table_serial_number));

		PostMessage(hInitialWindow, WMP_SHOW_TABLE_WINDOW, FALSE, t->table_serial_number);

	}

}



//****************************************************************

//
//

// Restore and activate an existing table window if it exists.

// returns 0 if the window was not found, 1 if it was.

//

int ShowTableWindow(WORD32 table_serial_number)

{

	return ShowTableWindow(table_serial_number, TRUE);

}



int ShowTableWindow(WORD32 table_serial_number, int steal_focus_flag)

{

	// First, check if we've already got this table window

	// open.  If so, top it.

	//kp(("%s(%d) %4d: ShowTableWindow(%d, %d) was just called from somewhere.\n", _FL, SecondCounter, table_serial_number, steal_focus_flag));

	int i = TableIndexFromSerialNumber(table_serial_number);

	if (i>=0) {

		// We've got it open already.

		struct TableInfo *t = &Table[i];

		LastTableWindowTopped = t;

		if (steal_focus_flag) {

			ShowWindow(t->hwnd, SW_SHOWNORMAL);

			ReallySetForegroundWindow(t->hwnd);

		} else {

			ShowWindow(t->hwnd, SW_SHOWNOACTIVATE);

			SetWindowPos(t->hwnd, HWND_TOPMOST,   0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

			SetWindowPos(t->hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

		}

		return TRUE;	// found and activated the window

	}

	return FALSE;	// could not find the window.

}



//*********************************************************

//
//

// Display an error message that some bitmaps were unable to load.

//

void DisplayBitmapLoadingError(void)

{

	MessageBox(NULL,

			"UNABLE TO OPEN TABLE\n"

			"\n"

			"There was a problem loading some of the bitmaps that are required\n"

			"to display this table correctly.\n"

			"\n"

			"We recommend that you STOP PLAYING until this problem is resolved.\n"

			"\n"

			"Some possible causes include:\n"

			"\n"

			"1.\tYour computer is running low on memory.  Try restarting\n"

			"\tyour computer and trying again.\n"

			"\n"

			"2.\tSome files got deleted somehow.  Try going to the Lobby screen\n"

			"\tand selecting 'Check for program update' on the help menu.\n"

			"\n"

			"If the problems persist, please feel free to send an email to\n"

			"support@https://github.com/kriskoinfor more things to try.",



			"Some bitmaps did not load...",

			MB_ICONHAND|MB_APPLMODAL|MB_TOPMOST);

}



//****************************************************************

//
//

// Create a new table window

//

void OpenNewTableWindow(WORD32 table_serial_number,

		ClientDisplayTabIndex client_display_tab_index,

		GameRules game_rules,

		int watching_flag, ChipType chip_type, int join_table_flags)

{

	int one_on_one = 0; //J Fonseca
    int _7_card=0; //J Fonseca

	int i = TableIndexFromSerialNumber(table_serial_number);

	if (i>=0) {	// is it already open?

		// We've already got the window open... make sure the

		// watching_flag is updated.

		Table[i].last_activated_time = SecondCounter;

		Table[i].watching_flag = watching_flag;



		CloseGoingToTableDialog();



		//20000921MB: Force our ClientStateInfo to be sent to the server

		// every time we open a table, regardless of whether we think it's

		// identical to last time or not.

		zstruct(Table[i].PrevClientState);

		Table[i].ClientState.leave_table = FALSE;	//20000921MB: always clear when opening.

		SendClientStateInfo(&Table[i]);



		// Sit down buttons should be enabled if we're watching

		EnableSitDownButtons(table_serial_number, watching_flag);

		return;	// Found it and activated it. No more work to do.

	}



	if (iBitmapLoadingProblem) {	// have we failed to load any bitmaps already?

		CloseGoingToTableDialog();

		DisplayBitmapLoadingError();

		return;

	}



	// Find an empty slot in our array.

	for (i=0 ; i<MAX_TABLES ; i++) {

		if (!Table[i].table_serial_number) {

			break;

		}

	}



	// Did we find space?

	if (i >= MAX_TABLES) {

		Error(ERR_ERROR, "%s(%d) Client: Attempt to open more tables than we have room for.",_FL);

		kp(("%s(%d)  Tried to open a window for table %d.\n",_FL, table_serial_number));

		kp(("%s(%d)  We already have these windows open: ", _FL));

		for (int j=0 ; j<MAX_TABLES ; j++) {

			kp(("  %d,", Table[j].table_serial_number));

		}

		kp(("\n"));

		CloseGoingToTableDialog();

		return;	// ignore attempt.

	}



	// Put it in slot i.

	struct TableInfo *t = &Table[i];

	zstruct(*t);

	// Chat box init

	PPInitializeCriticalSection(&t->chat_crit_sec, CRITSECPRI_CHAT, "table#??? chat");

	PPInitializeCriticalSection(&t->draw_crit_sec, CRITSECPRI_DRAW, "table#??? draw");

	t->table_index = i;	// fill this in again

	t->table_serial_number = table_serial_number;

	t->watching_flag = watching_flag;

	t->game_rules = game_rules;

	t->client_display_tab_index = client_display_tab_index;

	t->animation_disable_count = 50;

	// don't animate the first few things we receive.  Note that this number will be put

	// into a more reasonable range when a GamePlayerData record is received.

	//kp(("%s(%d) table->animation_disable_count = %d\n", _FL, t->animation_disable_count));



	t->max_players_per_table = 10;

	t->heads_up_seat_1 = HEADS_UP_SEAT_1-1;

	t->heads_up_seat_2 = HEADS_UP_SEAT_2-1;

	if (game_rules==GAME_RULES_STUD7 || game_rules==GAME_RULES_STUD7_HI_LO) {

		t->max_players_per_table = 8;

	  #if 0	//20000728MB

		t->heads_up_seat_1 = 2;

		t->heads_up_seat_2 = 5;

	  #endif

	}

  #if 1	//20000207MB => 20000713HK

	t->chip_type = chip_type;

  #else

	//struct CardRoom_TableSummaryInfo tsi;

	//zstruct(tsi);

	//ErrorType err = FindTableSummaryInfo(table_serial_number, &tsi, NULL);

	//if (err) {

	//	// Sleep and try again.

	//	Sleep(500);

	//	err = FindTableSummaryInfo(table_serial_number, &tsi, NULL);

	//}

	//if (err) {

	//	Error(ERR_ERROR, "%s(%d) Can't determine real/play money flag when opening table", _FL);

	//	t->real_money = TRUE;	// always err on the side of caution.

	//} else{

	//	t->real_money = (tsi.flags & TSIF_REAL_MONEY) ? TRUE : FALSE;

	//}

  #endif



	// Fill all card entries with CARD_NO_CARD

	memset(t->GamePlayerData.cards, 		CARD_NO_CARD, sizeof(Card)*MAX_PLAYERS_PER_GAME*MAX_PRIVATE_CARDS);

	memset(t->GamePlayerData.common_cards,	CARD_NO_CARD, sizeof(Card)*MAX_PUBLIC_CARDS);

	memset(t->display_cards,       		 	CARD_NO_CARD, sizeof(Card)*MAX_PLAYERS_PER_GAME*MAX_PRIVATE_CARDS);

	memset(t->dealt_cards,       		 	CARD_NO_CARD, sizeof(Card)*MAX_PLAYERS_PER_GAME*MAX_PRIVATE_CARDS);

	memset(t->display_common_cards,        	CARD_NO_CARD, sizeof(Card)*MAX_PUBLIC_CARDS);



	t->active_table_window = TRUE;	// for now, assume we're the active window (messages keep this up to date from now on)

	t->animation_disable_flag = Defaults.iAnimationDisabled;

	t->no_smoking_flag = Defaults.iNoSmokingFlag;

	t->last_activated_time = SecondCounter;	// keep track of when last activated/deactivated


	
	
		if (game_rules == GAME_RULES_STUD7 || game_rules==GAME_RULES_STUD7_HI_LO) {	// we need a 7-card stud layout

			if (join_table_flags & JOINTABLE_FLAG_ONE_ON_ONE) {

				t->hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_STUD7_LAYOUT_1ON1),

						NULL, dlgFuncTableLayoutWindow);

			} else {

				t->hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_STUD7_LAYOUT_LARGE),
	
						NULL, dlgFuncTableLayoutWindow);
	
			}

		} else { // we need a hold'em/omaha layout

			t->hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HOLDEM_LAYOUT_LARGE),

					NULL, dlgFuncTableLayoutWindow);

		}

	

		// Hide cardroom

		if (hCardRoomDlg && !Defaults.keep_cardroom_window_open) {

			ShowWindow(hCardRoomDlg, SW_MINIMIZE);

		}


	


	// Load the background pic if necessary

  #if ADMIN_CLIENT

	if (!RunningManyFlag)

  #endif

	{

		/* 

		int index = 0;

		char *fname = "table";

		if (join_table_flags & JOINTABLE_FLAG_ONE_ON_ONE) {

			fname = "1on1table";

			index = 2;

		} else if (t->max_players_per_table==8) {

			fname = "7cstable";

			index = 1;

		}

		

		*/





		int index = 0;

		char *fname = "media\\src_table";

		if (join_table_flags & JOINTABLE_FLAG_ONE_ON_ONE) {

			fname = "media\\src_1on1_table";

			index = 2;

			one_on_one = 1;

		} else if (t->max_players_per_table==8) {

			fname = "media\\src_7cs_table";

			index = 1;

			_7_card =1;

		}

		



		

		if (t->chip_type == CT_REAL || t->chip_type==CT_TOURNAMENT) {

			 index += 3;	// 2nd set is for real money

		}



		//kp(("%s(%d) t->chip_type = %d, index = %d\n", _FL, t->chip_type, index));

		//if (!hTableBgnds[index]) {

			char fname2[MAX_FNAME_LEN];

			//modified by allne 9-27-2001

		/*	sprintf(fname2, "%s%s.act", fname, (t->chip_type != CT_PLAY) ? "-rm" : "");

			hTablePalettes[index] = LoadPaletteIfNecessary(FindFile(fname2));

			sprintf(fname2, "%s%s.jpg", fname, (t->chip_type != CT_PLAY) ? "-rm" : "");

			hTableBgnds[index] = LoadJpegAsBitmap(FindFile(fname2), hTablePalettes[index]);

			
          

			sprintf(fname2, "%s%s.act", fname, (t->chip_type == CT_REAL) ? "_rm" : "");

			hTablePalettes[index] = LoadPaletteIfNecessary(FindMediaFile(fname2));

			sprintf(fname2, "%s%s.jpg", fname, (t->chip_type == CT_REAL) ? "_rm" : "");

			hTableBgnds[index] = LoadJpegAsBitmap(FindMediaFile(fname2), hTablePalettes[index]);

		
*/


			//sprintf(fname2, "%s%s.act", fname, (t->chip_type == CT_REAL) ? "" : "");

			hTablePalettes[index] = LoadPaletteIfNecessary(FindMediaFile(fname2));

			sprintf(fname2, "%s%s.jpg", fname, (t->chip_type == CT_REAL) ? "" : "");

			hTableBgnds[index] = LoadJpegAsBitmap(FindMediaFile(fname2), hTablePalettes[index]);

		//}

		t->hbm_background = hTableBgnds[index];

		t->hpalette = hTablePalettes[index];

	}



	// Create the off-screen drawing bitmap for this table

	t->hbm_off_screen = DuplicateHBitmap(t->hbm_background);



	// Load chips and cards if necessary

  #if ADMIN_CLIENT

	if (!RunningManyFlag)

  #endif

	{

		

    	if (t->chip_type==CT_TOURNAMENT) {

			// Tournament tables need both real AND tournament chips loaded,

			// therefor make sure the real chips are loaded.			

			if (!hTournTableScript) {

			//	hTournTableScript = LoadBMPasBitmap(FindFile("tourn_table.bmp"), t->hpalette);

			}


		if (!hTournTableScript) {

			//	hTournTableScript = LoadBMPasBitmap(FindMediaFile("media\\tourn_table.bmp"), t->hpalette);

		}

			//LoadChipBitmaps(t->hpalette, CT_REAL);

		}


       if (t->chip_type==CT_PLAY) {

			if (!hPlayMoneyScript) {

			//	hPlayMoneyScript = LoadBMPasBitmap(FindMediaFile("media\\tab.bmp"), t->hpalette);

			}

		
		}

		

			if (!hRealMoneyScript) {

			//hRealMoneyScript = LoadBMPasBitmap(FindMediaFile("media\\real_money.bmp"), t->hpalette);

			}

		if (_7_card)
			hPlayMoneyScript = LoadBMPasBitmap(FindMediaFile("media\\tab_7stud.bmp"), t->hpalette);		

		if (one_on_one) //J Fonseca    30/12/2003
			hPlayMoneyScript = LoadBMPasBitmap(FindMediaFile("media\\tab1on1.bmp"), t->hpalette);
		else
			hPlayMoneyScript = LoadBMPasBitmap(FindMediaFile("media\\tab.bmp"), t->hpalette);
			
		LoadChipBitmaps(t->hpalette, t->chip_type);

		LoadCardBitmaps(t->hpalette);

		LoadTableBitmaps(t->hpalette);

	  #if INCLUDE_TABLE_OPTIONS

	  #endif

	}


	if (!hChatBgndBrush) {

		hChatBgndBrush = CreateSolidBrush(CHAT_BGND_COLOR);	// cream color

	}



	// Choose some fonts for the various text on the table...

	for (int j=0 ; j<MAX_TABLE_FONTS ; j++) {

		if (!hTableFonts[j]) {

			int font_heights[MAX_TABLE_FONTS] =  {

				/*

				14,

				16,

				18,

				20,

				22,

				15,	*/

				// used for text on the 'in turn' buttons

				16,

				16,

				17,

				17,

				18,

				17,

			};

			int font_weights[MAX_TABLE_FONTS] =  {

				FW_NORMAL,

				FW_SEMIBOLD,

				FW_BOLD,

				FW_BOLD,

				FW_BOLD,

				FW_SEMIBOLD,	// used for text on the 'in turn' buttons				

			};

			for (int attempt=0 ; attempt < 2 && !hTableFonts[j]; attempt++) {

				hTableFonts[j] = CreateFont(

					font_heights[j],//  int nHeight,             // logical height of font

					0,	//  int nWidth,              // logical average character width

					0,	//  int nEscapement,         // angle of escapement

					0,	//  int nOrientation,        // base-line orientation angle

					font_weights[j],//  int fnWeight,            // font weight

					0,	//  DWORD fdwItalic,         // italic attribute flag

					0,	//  DWORD fdwUnderline,      // underline attribute flag

					0,	//  DWORD fdwStrikeOut,      // strikeout attribute flag

					ANSI_CHARSET,		//  DWORD fdwCharSet,        // character set identifier

					OUT_DEFAULT_PRECIS,	//  DWORD fdwOutputPrecision,  // output precision

					CLIP_DEFAULT_PRECIS,//  DWORD fdwClipPrecision,  // clipping precision

					DEFAULT_QUALITY,	//  DWORD fdwQuality,        // output quality

					DEFAULT_PITCH|FF_ROMAN,	//  DWORD fdwPitchAndFamily,  // pitch and family
					"Arial"    // LPCTSTR lpszFace 

//					attempt==0 ? "Times New Roman" : NULL	//  LPCTSTR lpszFace         // pointer to typeface name string

				);

				//kp(("%s(%d) attempt = %d, font = $%08lx\n", _FL, attempt, hTableFonts[j]));

			}

			//kp(("%s(%d) hTableFonts[%d] = $%08lx\n", _FL, j, hTableFonts[j]));

		}

	}



	// Install our hook procedure if it has not been installed.

	if (!hColorStaticHook) {

		hColorStaticHook = SetWindowsHookEx(WH_GETMESSAGE, ColorStaticHookProc,

					NULL, GetCurrentThreadId());

	}



	if (SeatAvail.table_serial_number && SeatAvail.table_serial_number==table_serial_number) {

		// We probably got opened/joined because a seat became available.

		// That means we're also allowed to sit down at this table.

		t->sit_down_allowed = TRUE;	// we can sit down in any empty seat.

	}

	pr(("%s(%d) OpenNewTableWindow: SeatAvail.table_serial_number = %d, t->sit_down_allowed = %d\n",

				_FL, SeatAvail.table_serial_number, t->sit_down_allowed));



	//debug: if we joined automatically, then default to computer play

  #if ADMIN_CLIENT	//19990725MB

	t->computer_play_seconds = 1;

	if (AutoJoinDefaultTable && iAdminClientFlag) {

		t->computer_play = TRUE;

		PostMessage(GetDlgItem(t->hwnd, IDC_COMPUTER_PLAY), BM_SETCHECK, BST_CHECKED, 0);

	}

  #endif

	// Set the 'Quiet Dealer' checkbox according to Defaults.

	t->quiet_dealer = Defaults.iQuietDealerType;

  #if 0	//19991121MB

	if (t->quiet_dealer) {

		SendMessage(GetDlgItem(t->hwnd, IDC_QUIET_DEALER), BM_SETCHECK, BST_CHECKED, 0);

	}

  #endif

	// Make muck_losing_hands default to on

	t->ClientState.muck_losing_hands = (BYTE8)TRUE;

	PostMessage(GetDlgItem(t->hwnd, IDC_MUCK_LOSING_HANDS), BM_SETCHECK, BST_CHECKED, 0);



	SetWindowTextIfNecessary(t->hwnd, "Table window");

	EnableSitDownButtons(table_serial_number, watching_flag);

	ShowWindowIfNecessary(GetDlgItem(t->hwnd, IDC_SNACK_MENU), t->watching_flag ? SW_HIDE : SW_SHOWNA);

	FlagRedrawTable(i);



	AddProximityHighlight(&t->proximity_highlight_list, GetDlgItem(t->hwnd, IDC_SNACK_MENU));

	AddProximityHighlight(&t->proximity_highlight_list, GetDlgItem(t->hwnd, IDC_DEALER_CHIP_AREA));



	// Restore the position of this table's window

	char table_name[30];

	sprintf(table_name, "Table %d Position", i+1+iProgramInstanceCount);

	WinRestoreWindowPos(ProgramRegistryPrefix, table_name, t->hwnd, NULL, NULL, FALSE, TRUE);

	// make sure it's on screen in the current screen resolution

	WinPosWindowOnScreen(t->hwnd);



	// Create a tooltip control/window for use within this dialog box

	// and add all our tool tips to it.

	char *sit_out_tooltip = "Check this box to sit out each new hand";

	char *post_in_turn_tooltip = "Automatically post when it's your turn to post";

	if (chip_type==CT_TOURNAMENT) {

		sit_out_tooltip = "Automatically post your blinds and fold your hand when it's your turn";

		post_in_turn_tooltip = "Auto-post is always ON in tournaments";

	}

	struct DlgToolTipText *dttt = TableDlgToolTipText;

	while (dttt->id) {

		if (dttt->id==IDC_SIT_OUT) {

			dttt->text = sit_out_tooltip;

		}

		if (dttt->id==IDC_POST_IN_TURN) {

			dttt->text = post_in_turn_tooltip;

		}

		dttt++;

	}

	t->ToolTipHwnd = OpenToolTipWindow(t->hwnd, TableDlgToolTipText);



	CloseGoingToTableDialog();



	if (MinimizeWindowsFlag) {

		ShowWindow(t->hwnd, SW_MINIMIZE);

	} else {

		ShowWindow(t->hwnd, SW_SHOW);

		ReallySetForegroundWindow(t->hwnd);

	}

	SetTableKeyboardFocus(t);	// purely cosmetic



	// Send our current client_state for this table to the server.

	SendClientStateInfo(t);

	pr(("%s(%d) Table[%d].hwnd = $%08lx\n", _FL, i, t->hwnd));



	UpdateTableMenuItems();

	BuildColorStaticTable();



	// finally, if there's a saved GameCommonData for this table which arrived

	// while we were opening this window, use it.

	if (!t->GameCommonData.table_serial_number &&

		SavedGameCommonData.table_serial_number==t->table_serial_number)

	{

		//kp(("%s(%d) Using SavedGameCommonData\n",_FL));

		t->NewestGameCommonData = SavedGameCommonData;

		t->GameCommonData = SavedGameCommonData;

		zstruct(SavedGameCommonData);

	}



	// Rename the 'sit out' checkbox in tournament play (since you're not really

	// allowed to sit out at all during a tournament game).

	if (chip_type==CT_TOURNAMENT) {

		SetWindowTextIfNecessary(GetDlgItem(t->hwnd, IDC_SIT_OUT), "Post && Fold in turn");

		CheckDlgButtonIfNecessary(t->hwnd, IDC_POST_IN_TURN, TRUE);

		//EnableWindowIfNecessary(GetDlgItem(t->hwnd, IDC_POST_IN_TURN), FALSE);

	}



	if (iBitmapLoadingProblem) {	// have we failed to load any bitmaps already?

		PostMessage(t->hwnd, WMP_CLOSE_YOURSELF, 0, 0);

		DisplayBitmapLoadingError();

	}



	SendClientStateInfo(&Table[i]);



	// Finally, set the focus.  Do this with a post that will get processed

	// after everything else in the message queue has been dealt with.

	PostMessage(t->hwnd, WMP_SET_CHAT_FOCUS, 0, 0);

}



/**********************************************************************************

 Function RedrawLoginDlgNumbers(HWND hDlg)

 Date: HK1999/07/29

 Purpose: support function for dlg function below

***********************************************************************************/

void RedrawLoginDlgNumbers(HWND hDlg)

{

	char str[100];

	struct TableInfo *ti = NULL;

	int display_factor;

	int chips, minimum_required, big_blind, small_blind, suggest_amount;

	char cs[MAX_CURRENCY_STRING_LEN];

	char cs1[MAX_CURRENCY_STRING_LEN];

	GetDlgItemText(hDlg, IDC_STATIC_CHIPS, str, 50);

	//MessageBox(hDlg, str, "File Not Found", MB_OK);
	chips = atoi(str);

	// retrieve the table info pointer

	GetDlgItemText(hDlg, IDC_STATIC_TABLE_INFO, str, 50);

	ti = (struct TableInfo *)(atol(str));

	big_blind = ti->GameCommonData.big_blind_amount;

	small_blind = ti->GameCommonData.small_blind_amount;

	// retrieve the minimum allowed

	GetDlgItemText(hDlg, IDC_STATIC_MINIMUM, str, 50);

	if (ti->tournament_table) {

		minimum_required = small_blind+big_blind;

	} else {

		minimum_required = atoi(str);

	}

	if (ti->tournament_table) {

			sprintf(str,"%s Total", CurrencyString(cs, small_blind+big_blind, CT_REAL));	// static tournament buy-in amount

		SetDlgItemTextIfNecessary(hDlg, IDC_BUY_IN_BUTTON_MAX, str);

		sprintf(str,"You currently have %s cash available", 

			CurrencyString(cs1, chips, CT_REAL));	// available balance

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_BANKROLL, str);



	} else {

		// note: MINIMUM_TIMES_BB_ALLOWED_TO_SIT_DOWN = 10 (pplib.h)
		
		if (ti->chip_type == CT_REAL) {

			sprintf(str,"Full bankroll (%s)", CurrencyString(cs, chips, ti->chip_type ,FALSE,-1));	// set to amount we were handed

		} else {

			sprintf(str,"Total play money (%s)",CurrencyString(cs, chips, ti->chip_type, FALSE,-1));	// set to amount we were handed

		}

		SetDlgItemTextIfNecessary(hDlg, IDC_BUY_IN_BUTTON_MAX, str);

	}

	display_factor = GameStakesMultipliers[ti->game_rules - GAME_RULES_START];

	// suggested amount (in edit box) = (40 * big blind) round up to nearest 100

	suggest_amount = (40*big_blind*display_factor)+9999;	// build a number we can round down

	suggest_amount -= suggest_amount % 10000;

	if (suggest_amount > chips) {	// too much, just set to minimum

	  #if 1	//19990904MB

		suggest_amount = chips;

	  #else

		suggest_amount = minimum_required;

	  #endif

	}



  #if 0	//20000715MB

	// this is still in pennies -- convert to dollars

	if (IsDlgButtonChecked(hDlg, IDC_BUY_IN_BUTTON_USR)) {

		sprintf(str,"%d", suggest_amount/100);	// set to amount we came up with

		SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_CHIPS, str);

		EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_EDIT_CHIPS), (suggest_amount >= minimum_required ? TRUE : FALSE));

		SendMessage(GetDlgItem(hDlg, IDC_EDIT_CHIPS), EM_SETSEL,(WPARAM)0,(LPARAM)-1);

		SetFocus(GetDlgItem(hDlg, IDC_EDIT_CHIPS));

	} else {

		zstruct(str);

		SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_CHIPS, str);

		EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_EDIT_CHIPS), FALSE);

	}

	EnableWindowIfNecessary(GetDlgItem(hDlg, IDOK),(chips >= minimum_required ? TRUE : FALSE));

	EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_BUY_IN_BUTTON_MAX),(chips >= minimum_required ? TRUE : FALSE));

	EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_BUY_IN_BUTTON_USR),(chips >= minimum_required ? TRUE : FALSE));

  #else	// HK's new version

	// this is still in pennies -- convert to dollars

	if (ti->tournament_table) {

		if (chips < minimum_required) {

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_MAX, FALSE);

		}

		EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_BUY_IN_BUTTON_MAX),(chips >= minimum_required ? TRUE : FALSE));

		int ok_state = IsDlgButtonChecked(hDlg, IDC_BUY_IN_BUTTON_MAX);

		EnableWindowIfNecessary(GetDlgItem(hDlg, IDOK), ok_state);

	} else {

		if (IsDlgButtonChecked(hDlg, IDC_BUY_IN_BUTTON_USR)) {

			sprintf(str,"%d", suggest_amount/100); // set to amount we came up with

			SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_CHIPS, str);

			EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_EDIT_CHIPS), (suggest_amount >= minimum_required ? TRUE : FALSE));

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_CHIPS), EM_SETSEL,(WPARAM)0,(LPARAM)-1);

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_CHIPS));

		} else {

			zstruct(str);

			SetDlgItemTextIfNecessary(hDlg, IDC_EDIT_CHIPS, str);

			EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_EDIT_CHIPS), FALSE);

		}

	}

  #endif

}



/**********************************************************************************

 Function CALLBACK dlgTableBuyInFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK1999/05/16

 Purpose: request buy-in at a table -- we'll be prompted for how many chips

***********************************************************************************/

BOOL CALLBACK dlgTableBuyInFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[300];

	struct TableInfo *ti = NULL;

	struct BuyInDLGInfo *bi = NULL;

	int display_factor;

	int chips, minimum_required, big_blind, small_blind;

	int out_of_play_chips = FALSE;

	char cs[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN], cs3[MAX_CURRENCY_STRING_LEN];

	struct CardRoom_TableSummaryInfo tsi;

	zstruct(tsi);

	zstruct(str);

	GetDlgItemText(hDlg, IDC_STATIC_TABLE_INFO, str, 50);

	ti = (struct TableInfo *)(atol(str));	// THIS IS A HACK!  USE SetWindowLong() and GWL_USERDATA!



	switch (message) {

	case WM_INITDIALOG:

		if (Defaults.iDefaultBuyInWithOther) {

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_MAX, FALSE);

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_USR, TRUE);

		} else {

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_MAX, TRUE);

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_USR, FALSE);

		}

		hBuyInDLG = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		bi = (BuyInDLGInfo *)lParam;

		chips = bi->chips;

		chips -= chips % 100;	// trim pennies

		minimum_required = bi->minimum_allowed;

		ti = bi->ti;

		// for tournaments, turn off the [OK] by default (will be turned on by checkbox)

		if (ti->tournament_table) {

			EnableWindowIfNecessary(GetDlgItem(hDlg, IDOK), FALSE);

			CheckDlgButton(hDlg, IDC_BUY_IN_BUTTON_MAX, FALSE);

		}

		FindTableSummaryInfo(ti->table_serial_number, &tsi);

		display_factor = GameStakesMultipliers[ti->game_rules - GAME_RULES_START];

		big_blind = ti->GameCommonData.big_blind_amount;

		small_blind = ti->GameCommonData.small_blind_amount;

		if ( (ti->chip_type == CT_PLAY) && (FakeChipsInBank + FakeChipsInPlay < REQ_MORE_FREE_CHIPS_LEVEL) ) {

			out_of_play_chips = TRUE;

		}

		// set actual titles, text, etc

		if (bi->buy_in_type==JTS_REBUY) {

			SetWindowTextIfNecessary(hDlg, "Bring more chips to the table...");

		}

		if (out_of_play_chips) {

			SetDlgItemTextIfNecessary(hDlg, ID_MISC_CHIP_ACTION_BUTTON, "I need more chips!");

		} else {

			SetDlgItemTextIfNecessary(hDlg, ID_MISC_CHIP_ACTION_BUTTON, "Where are my chips?");

		}

		// build the header to the buy-in dlg

		if (ti->tournament_table) {

			sprintf(str, "%s\nTable \"%s\"\n%d players, %s chips each",

				GameRuleNames[ti->game_rules - GAME_RULES_START],

				TableNameFromSerialNumber(ti->table_serial_number),

				tsi.max_player_count,

				CurrencyString(cs3, STARTING_TOURNAMENT_CHIPS, CT_TOURNAMENT));

		} else {

			sprintf(str, "%s %s/%s\n%s\nTable \"%s\"\n(minimum required: %s)",

				(ti->chip_type == CT_REAL ? "Real money": "Play chips"),

				CurrencyString(cs, big_blind*display_factor, ti->chip_type),

				CurrencyString(cs2, big_blind*display_factor*2, ti->chip_type),

				GameRuleNames[ti->game_rules - GAME_RULES_START],

				TableNameFromSerialNumber(ti->table_serial_number),

				CurrencyString(cs3, minimum_required, ti->chip_type));

		}

		SetDlgItemTextIfNecessary(hDlg, IDC_BUYIN_HEADER, str);

		// tournament prize details

		if (ti->tournament_table) {

			// tournament total prize pool

			sprintf(str, "Prize pool: %s", CurrencyString(cs, tsi.tournament_prize_pool, CT_REAL));

			SetDlgItemTextIfNecessary(hDlg, IDC_GROUP_TOURNAMENT_PRIZES, str);

			sprintf(str, "   1st:   %s\n   2nd:  %s\n   3rd:   %s",

				// these constants should come from a prize breakdown matrix somewhere?

				CurrencyString(cs,  (50*tsi.tournament_prize_pool)/100, CT_REAL),

				CurrencyString(cs2, (30*tsi.tournament_prize_pool)/100, CT_REAL),

				CurrencyString(cs3, (20*tsi.tournament_prize_pool)/100, CT_REAL));

			SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_PRIZE_DESCRIPTION, str);



			sprintf(str, "%s Buy-In + %s Entry Fee",

				CurrencyString(cs, big_blind, CT_REAL),

				CurrencyString(cs2, small_blind, CT_REAL));

			SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_BUYIN_DETAILS, str);

		}

		// hide the table info pointer so we can use it later

		sprintf(str, "%u", ti);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_TABLE_INFO, str);

		// hide chips too

		sprintf(str, "%d", chips);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_CHIPS, str);

		// hide if low in play chips

		sprintf(str, "%d", out_of_play_chips);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_LOW_PLAY_CHIPS, str);

		// hide minimum allowed

		sprintf(str, "%d", minimum_required);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_MINIMUM, str);

		// do the drawing

		RedrawLoginDlgNumbers(hDlg);



		if (out_of_play_chips) {

			MessageBox(hDlg,

				"You are running low on free play chips, but are eligible for more!\n"

				"Just click on the [I need more chips!] button on the Buy-In box.\n",

				"Out of play chips!",

			MB_OK|MB_APPLMODAL|MB_TOPMOST);

		}



		// Make ourselves a topmost window

		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		return FALSE;



	case WM_ACTIVATE:

		// 20000713HK: when we become active for whatever reason, the focus should be on

		// where we enter numbers

		if (LOWORD(wParam)) { // was activated

			if (ti && ti->tournament_table) {

				// select the 'buy in' checkbox.

				SetFocus(GetDlgItem(hDlg, IDC_BUY_IN_BUTTON_MAX));

			} else {

				SetFocus(GetDlgItem(hDlg, IDC_EDIT_CHIPS));

			}

			return TRUE;

		}

		break;



	// WMP_UPDATE_BANK_CHIPS gets sent when the number of chips in the bank

	// changes for some reason... when a player leaves another table or 

	// when e-cash $$ "arrives"

	case WMP_UPDATE_REALBANK_CHIPS:

	case WMP_UPDATE_FAKEBANK_CHIPS:

		// we'll ignore the message if it's for the kinds of chips we're not using

		// hide chips in dlg

		//MessageBox(hDlg, "Por aqui", "File Not Found", MB_OK);
		if (ti->tournament_table) {

			if (message != WMP_UPDATE_REALBANK_CHIPS) {

				return TRUE;

			}

		} else {

			if (message == WMP_UPDATE_REALBANK_CHIPS && ti->chip_type == CT_PLAY) {

				return TRUE;

			}

			if (message == WMP_UPDATE_FAKEBANK_CHIPS && ti->chip_type == CT_REAL) {

				return TRUE;

			}

		}

		chips = (WORD32)lParam;

		sprintf(str, "%d", chips);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_CHIPS, str);

		RedrawLoginDlgNumbers(hDlg);

		return TRUE;



	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_BUY_IN_BUTTON_MAX ||

			LOWORD(wParam) == IDC_BUY_IN_BUTTON_USR) {

				RedrawLoginDlgNumbers(hDlg);

			break;

		}

		switch (LOWORD(wParam)) {

		case ID_MISC_CHIP_ACTION_BUTTON:						

			GetDlgItemText(hDlg, IDC_STATIC_LOW_PLAY_CHIPS, str, 20);

			out_of_play_chips = atoi(str);

			if (out_of_play_chips && !AskedForMorePlayChips) {	// request for more play chips

				AskedForMorePlayChips = TRUE;

				kp(("%s(%d) out of play chips = %d\n", _FL,out_of_play_chips));

				MessageBox(hDlg,

				"We're very pleased you're enjoying your experience playing at e-Media Poker.\n\n"

				"Your new racks of play chips are on their way!\n\n"

				"All we ask in return is that you tell a friend or aquaintance about how much\n"

				"fun you're having at e-Media Poker.",

				"I need more chips!",

				MB_OK|MB_ICONQUESTION|MB_APPLMODAL|MB_TOPMOST);

				// send the request to the server

				struct MiscClientMessage mcm;

				zstruct(mcm);

				mcm.message_type = MISC_MESSAGE_CL_REQ_FREE_CHIPS;

				SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				SetDlgItemText(hDlg, ID_MISC_CHIP_ACTION_BUTTON, "Where are my chips?");

			} else {	// 

				MessageBox(hDlg,

					"Chips at other tables will arrive in your account after the\n"

					"current hand is finished at that table.\n"

					"\n"

					"If you already requested more chips to be brought to this table,\n"

					"they will arrive after the current hand.\n",

					"Where are my chips?",

					MB_OK|MB_ICONQUESTION|MB_APPLMODAL|MB_TOPMOST);

			}

			break;

				

		case IDOK:

			// retrieve the table info strucutre and everything else we need

			GetDlgItemText(hDlg, IDC_STATIC_TABLE_INFO, str, 50);

			ti = (struct TableInfo *)(atol(str));

			big_blind = ti->GameCommonData.big_blind_amount;

			small_blind = ti->GameCommonData.small_blind_amount;

			display_factor = GameStakesMultipliers[ti->game_rules - GAME_RULES_START];

			// retrieve the minimum allowed

			GetDlgItemText(hDlg, IDC_STATIC_MINIMUM, str, 50);

			minimum_required = atoi(str);

			// now check what we were passed

			if (!ti->tournament_table) {

				if (IsDlgButtonChecked(hDlg, IDC_BUY_IN_BUTTON_MAX)) {

					Defaults.iDefaultBuyInWithOther = FALSE;

					GetDlgItemText(hDlg, IDC_STATIC_CHIPS, str, 50);

					chips = atoi(str);

				} else {

					Defaults.iDefaultBuyInWithOther = TRUE;

					GetDlgItemText(hDlg, IDC_EDIT_CHIPS, str, 50);

					// this is dollars -- convert to pennies

					chips = 100*atoi(str);

				}

			} else { // override for tournament table

				chips = big_blind+small_blind;

			}			

			if (chips < minimum_required) {

				if (chips) {	// trying to buy in with something, but not enough

					sprintf(str, "Sorry, you requested %s in chips, the minimum required is %s",

						CurrencyString(cs, chips, ti->chip_type),

						CurrencyString(cs2, minimum_required, ti->chip_type));

					MessageBox(hDlg, str, "Buy-In is less than minimum...",

						MB_OK|MB_APPLMODAL|MB_TOPMOST);

					return FALSE;	// try again

				} else {	// it's zero, assume abort on purpose

					// allow fall through; treated as cancel

				}

			}

			EndDialog(hDlg, chips);	// return chip amount through RC

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, 0);	// cancel is logically eq to buying in with zero chips

			return TRUE;

		}

		break;



	case WMP_CLOSE_YOURSELF:

		EndDialog(hDlg, 0);	// logically eq to buying in with zero chips

		return TRUE;

		

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		hBuyInDLG = NULL;

		return TRUE;

	}

    return FALSE;

}



//*********************************************************

//
//

// Determine if a line of chat should be displayed given the

// current chat display mode and the text type.

// Returns TRUE for display, FALSE for don't display.

//

int DetermineIfChatShouldDisplay(struct TableInfo *t, struct ChatEntry *e)

{

	int display_mask = ChatModeData[t->quiet_dealer].flags;

	switch (e->text_type) {

	case CHATTEXT_DEALER_BLAB:

	case CHATTEXT_DEALER_BLAB_NOBUFFER:

		if (display_mask & CHATMODE_FLAG_DEALER_BLAB) {

			return TRUE;	// display it.

		}

		break;

	case CHATTEXT_DEALER_NORMAL:

	case CHATTEXT_DEALER_NORMAL_NOBUFFER:

		if (display_mask & CHATMODE_FLAG_DEALER_NORMAL) {

			return TRUE;	// display it.

		}

		break;

	case CHATTEXT_PLAYER:

		if (display_mask & CHATMODE_FLAG_USER) {

			return TRUE;	// display it.

		}

		break;

	case CHATTEXT_DEALER_WINNER:

		if (display_mask & CHATMODE_FLAG_DEALER_WINNER) {

			if (!(display_mask & CHATMODE_FLAG_DEALER_NORMAL)) {

				// normal is OFF... don't display old games.

				//kp(("%s(%d) e->game_serial_number = %d\n", _FL, e->game_serial_number));

				if (e->game_serial_number < t->prev_game_serial_number) {

					return FALSE;	// don't display it.

				}

			}

			return TRUE;	// display it.

		}

		break;

	case CHATTEXT_ADMIN:

		return TRUE;	// always display ALL admin stuff

	default:

		kp(("%s(%d) Unknown chat text_type (%d)... defaulting to DISPLAY.\n", _FL, e->text_type));

		return TRUE;	// display it.

	}

	return FALSE;	// do not display.

}



/**********************************************************************************

 Function CALLBACK dlgTournSummaryRecFunc()

 Date: HK00/08/29

 Purpose: callback for tournament summary requester

***********************************************************************************/

BOOL CALLBACK dlgTournSummaryRecFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	struct MiscClientMessage *mcm;

	int placed;

	WORD32 serial_number;

	char str[150];

	zstruct(str);

	switch (message) {



	case WM_INITDIALOG:

		mcm = (MiscClientMessage *)lParam;

		// store the table serial number for later reference

		serial_number = mcm->table_serial_number;

		sprintf(str, "%d", mcm->table_serial_number);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_SERIALNO, str);

		// deal with the rest of the internal data

		placed = mcm->misc_data_1;

		SetDlgItemTextIfNecessary(hDlg, IDC_TEXT, mcm->msg);

		zstruct(str);

		if (placed > 2) {	

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDYES), SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDNO), SW_SHOWNA);

			sprintf(str,

					"Would you like a tournament summary email sent\n"

					"to you when the tournament is complete?");

		} else { // 1st or 2nd place, tournament is OVER

			sprintf(str,

					"You will be sent a tournament summary email.");

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDOK), SW_SHOWNA);

		}

		SetDlgItemTextIfNecessary(hDlg, IDC_REQUEST_SUMMARY_QUESTION, str);

		hTourSummaryDlg = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		// Make ourselves a topmost window

		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		return TRUE;	// TRUE = give keyboard control to default control (wParam)



	case WM_COMMAND:

		{

			int action = LOWORD(wParam);

			switch (action) {

			case IDOK:	// the hidden OK button

				DestroyWindow(hDlg);

				return TRUE;

			case IDYES:

			case IDNO:

				struct MiscClientMessage mcm_out;

				zstruct(mcm_out);

				mcm_out.message_type = MISC_MESSAGE_REQ_TOURN_SUMMARY_EMAIL;

				GetDlgItemText(hDlg, IDC_STATIC_SERIALNO, str, 50);

				mcm_out.table_serial_number = atol(str);

				mcm_out.misc_data_1 = (action == IDNO ? FALSE : TRUE);

				SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm_out, sizeof(mcm_out));

				DestroyWindow(hDlg);

				return TRUE;

			}

		}

		break;



	case WMP_CLOSE_YOURSELF:

		DestroyWindow(hDlg);

		return TRUE;

		

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		hTourSummaryDlg = NULL;

		return TRUE;

	}

    return FALSE;

}



/**********************************************************************************

 Function AskRequestTournamentEmail()

 Date: HK00/08/29

 Purpose: pop up the modeless dlg asking if we want to receive a tournament summary email

***********************************************************************************/

void AskRequestTournamentEmail(HWND parent, struct MiscClientMessage *mcm)

{

	if (hTourSummaryDlg) {

		SetFocus(hTourSummaryDlg);

	} else {

		PPlaySound(SOUND_WAKE_UP);	// could be a better sound

		hTourSummaryDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TOURN_EMAIL),

			parent, (DLGPROC)dlgTournSummaryRecFunc, (LPARAM)(mcm));

	}

}



/**********************************************************************************

 Function UpdateChatBuffer(struct GameChatMessage *gcm)

 Date: HK1999/06/20

 Purpose: add a line of text to our local chat buffer

 Returns: TRUE if we should do an update now, FALSE if we don't need to

          We might not need to if we just added something which won't show up

		  with Quiet Dealer turned on

***********************************************************************************/

int UpdateChatBuffer(struct GameChatMessage *gcm, int table_index)

{

	if (table_index < 0) {

		Error(ERR_ERROR, "%s(%d) Invalid Table Index trying to update chat buffer",_FL);

		return FALSE;

	}



	struct TableInfo *t = &Table[table_index];

	// 20000827HK: we're going to trap tournament dealer betting-round update messages and display

	// them in the slot for Previous hand #

	if (t->tournament_table && !stricmp(gcm->name,"Dealer") && 

		!strncmp(gcm->message,"Level ",6))

	{

		strnncpy(t->tournament_game_number_str, gcm->message, MAX_TGNS);

	  #if 0	// let this sift through to the deal chat

		return FALSE;	// no chat update

	  #endif

	}

	EnterCriticalSection((&t->chat_crit_sec));

	// in the event that there's a card animation in progress, don't update any

	// dealer chat text (player text is ok) -- instead, jam it into a temp string

	// which can be dealt with later

	if (t->dealing_flag) {

		// capture the string? (buffer for later)

		if (gcm->text_type == CHATTEXT_DEALER_BLAB || gcm->text_type == CHATTEXT_DEALER_NORMAL) {

			// yes, from the dealer -- find last slot

			for (int i = 0; i < MAX_BUFFERED_DEALER_CHAT_LINES; i++) {

				if (!t->BufferedDealerChat[i].message[0]) { //  blank slot

					strnncpy(t->BufferedDealerChat[i].name, gcm->name, MAX_COMMON_STRING_LEN);

					strnncpy(t->BufferedDealerChat[i].message, gcm->message, MAX_CHAT_MSG_LEN);

					t->BufferedDealerChat[i].text_type = gcm->text_type;

					t->BufferedDealerChat[i].game_serial_number = gcm->game_serial_number;

					LeaveCriticalSection(&(t->chat_crit_sec));

					return FALSE;	// nothing to redraw for now

				}

			}

			// if we're here, we didn't find a blank slot for this message

			Error(ERR_INTERNAL_ERROR, "%s(%d) Ran out of dealer buffer room",_FL);

		}

	} else {

		// flush anything that may be there pending

		FlushDealerChatBuffer(table_index);

	}

	int chat_buffer_index = t->chat_buffer_index;

	struct ChatEntry *e = &t->ChatBuffer[chat_buffer_index];

	// Keep track of the characters we're deleting from the display buffer...

	if (e->message[0] && DetermineIfChatShouldDisplay(t, e)) {

		t->chat_lines_deleted++;	// one more line removed

		// Add enough chars to compensate for the prefix, the ": ", and the CR/LF.

		t->chat_chars_deleted += strlen(e->message) + strlen(e->name) + 5;

	}



	strnncpy(e->name, gcm->name, MAX_COMMON_STRING_LEN);

	strnncpy(e->message, gcm->message, MAX_CHAT_MSG_LEN);

	e->text_type = gcm->text_type;

	e->game_serial_number = gcm->game_serial_number;

	// increment index for next call

	t->chat_buffer_index = (chat_buffer_index+1) % CHAT_LINES;

	// figure out situations where we might not want to force a refresh

	LeaveCriticalSection(&(t->chat_crit_sec));

	// we should tell the table to redraw the text if it will be displayed.

	return DetermineIfChatShouldDisplay(t, e);

}



/**********************************************************************************

 Function FlushDealerChatBuffer(int table_index)

 Date: HK1999/09/01

 Purpose: flush out any pending dealer text for this table

 Returns TRUE if something was actually flush to the chat buffer.

***********************************************************************************/

int FlushDealerChatBuffer(int table_index)

{

	int result = FALSE;

	struct TableInfo *t = &Table[table_index];

	EnterCriticalSection(&(t->chat_crit_sec));

	int chat_buffer_index = t->chat_buffer_index;

	for (int j = 0; j < MAX_BUFFERED_DEALER_CHAT_LINES; j++) {

		if (t->BufferedDealerChat[j].message[0]) {

			strnncpy(t->ChatBuffer[chat_buffer_index].name, t->BufferedDealerChat[j].name, MAX_COMMON_STRING_LEN);

			strnncpy(t->ChatBuffer[chat_buffer_index].message, t->BufferedDealerChat[j].message, MAX_CHAT_MSG_LEN);

			t->ChatBuffer[chat_buffer_index].text_type = t->BufferedDealerChat[j].text_type;

			t->ChatBuffer[chat_buffer_index].game_serial_number = t->BufferedDealerChat[j].game_serial_number;

			// now blank it for next time

			BLANK(t->BufferedDealerChat[j].name);

			BLANK(t->BufferedDealerChat[j].message);

			// increment index for next call

			t->chat_buffer_index = (chat_buffer_index+1) % CHAT_LINES;

			chat_buffer_index = t->chat_buffer_index;

			result = TRUE;

		} else {

			// nothing there -- so there's nothing after it either...

			break;

		}

	}

	LeaveCriticalSection(&(t->chat_crit_sec));

	return result;

}



/**********************************************************************************

 Function RefreshChatText(int table_index)

 Date: HK1999/06/20

 Purpose: redraw our chat box with correct text

***********************************************************************************/

void RefreshChatText(int table_index)

{

	#define BUFFER_LEN ((MAX_CHAT_MSG_LEN+MAX_COMMON_STRING_LEN+6) * CHAT_LINES + 10)

	char buf[BUFFER_LEN];

	zstruct(buf);

	struct TableInfo *t = &Table[table_index];

	EnterCriticalSection(&(t->chat_crit_sec));

	// first, build the current buffer 

	int chat_buffer_index = t->chat_buffer_index;

	// chat_buffer_index currently points to the oldest line we have

	char str[MAX_CHAT_MSG_LEN+MAX_COMMON_STRING_LEN+6];

	for (int index = 0; index < CHAT_LINES; index++) {

		int current_line = (chat_buffer_index+index) % CHAT_LINES;

		if (t->ChatBuffer[current_line].text_type) { // something there?

			if (!DetermineIfChatShouldDisplay(t, &t->ChatBuffer[current_line])) {

				// It's filtered... skip over it.

				continue;

			}

			sprintf(str, "%c%s: %s", 

				CHAT_SPECIAL_CHARACTER,	// defined above near line 950

				t->ChatBuffer[current_line].name,

				t->ChatBuffer[current_line].message);

			strcat(str, "\x0D\x0A");	// append CR/LF

			strcat(buf, str);

		}

	}

	// peel off last cr/lf pair (cosmetic)

	if (strlen(buf) >= 2) {

		buf[strlen(buf)-2] = 0;

	}

	LeaveCriticalSection(&(t->chat_crit_sec));



	// buffer is built; copy it into the chat window

	// scroll to bottom if we're allowed to

	HWND chat_hwnd = GetDlgItem(t->hwnd, IDC_CHAT_BOX);



	// fetch and save the current selection

	DWORD saved_sel_start = 0;

	DWORD saved_sel_end = 0;

	SendMessage(chat_hwnd, EM_GETSEL, (WPARAM)&saved_sel_start, (LPARAM)&saved_sel_end);

	// compensate for any chars that got deleted since last time...

	//kp(("%s(%d) selection was %d to %d... %d chars got deleted.\n",_FL,saved_sel_start,saved_sel_end,t->chat_chars_deleted));

	if (saved_sel_start >= t->chat_chars_deleted) {

		saved_sel_start -= t->chat_chars_deleted;

	} else {

		saved_sel_start = 0;

	}

	if (saved_sel_end >= t->chat_chars_deleted) {

		saved_sel_end -= t->chat_chars_deleted;

	} else {

		saved_sel_end = 0;

	}

	t->chat_chars_deleted = 0;

	//kp(("%s(%d) selection is now %d to %d\n",_FL,saved_sel_start,saved_sel_end));



	// determine the first line of text we want to scroll to.

	int first_line = 0;

	if (SecondCounter - t->chat_last_user_scroll_time >= CHAT_FREEZE_SECONDS) {

		// It's been a while... we should scroll to the end.

		first_line = CHAT_LINES*2;	// try to scroll past the bottom

	} else {

		// preserve the current first line...

		first_line = SendMessage(chat_hwnd, EM_GETFIRSTVISIBLELINE, 0, 0);

		first_line -= max(0,t->chat_lines_deleted);

	}

	t->chat_lines_deleted = 0;	// reset



	// Change the buffer and restore selection

	SendMessage(chat_hwnd, WM_SETTEXT,	  (WPARAM)0, (LPARAM)buf);	// this sets the text

	SendMessage(chat_hwnd, EM_SETSEL,	  (WPARAM)saved_sel_start, (LPARAM)saved_sel_end);

	int current_first_line = SendMessage(chat_hwnd, EM_GETFIRSTVISIBLELINE, 0, 0);

	pr(("%s(%d) current first line = %d, desired = %d, scrolling %+d lines\n",

				_FL, current_first_line, first_line, first_line - current_first_line));

	SendMessage(chat_hwnd, EM_LINESCROLL, (WPARAM)0, (LPARAM)(first_line - current_first_line));



	t->chat_scroll_position = SendMessage(chat_hwnd, EM_GETFIRSTVISIBLELINE, 0, 0);

}



/**********************************************************************************

 Function EnableSitDownButtons(int table_serial_number, int enable_flag)

 Date: HK1999/07/14

 Purpose: called when we're about to try to sit down -- or we've stopped trying

 Note:    T/F in the paremeter -- this is used to set the state of the sit down buttons

***********************************************************************************/

void EnableSitDownButtons(WORD32 table_serial_number, int enable_flag)

{

	//kp(("%s(%d) EnableSitDownButtons(%d, %d) called.\n",_FL,table_serial_number, enable_flag));

	// find the hWnd of this table

	struct TableInfo *t = TablePtrFromSerialNumber(table_serial_number);

	if (t && t->hwnd) {

		//kp(("%s(%d) t->hwnd = $%08lx\n", _FL, t->hwnd));

		for (int i=0 ; i < t->max_players_per_table ; i++) {

			EnableWindowIfNecessary(GetDlgItem(t->hwnd, SitDownButtonIDs[i]), enable_flag);

		}

	}

}



/**********************************************************************************

 Function BuyIntoTable(JointTableStatus jts, TableInfo *t, WORD32 minimum_required, WORD32 chips, int seating_postion)

 Date: HK1999/08/15

 Purpose: do processing for buying chips on a table (or rebuying)

 Returns: number of chips we bought

***********************************************************************************/

int BuyIntoTable(JoinTableStatus jts, struct TableInfo *t, WORD32 minimum_required, int seating_position)

{

	int chips = 0;

	// if we happen to get here while there's still a buy-in dlg open somewhere, focus it and leave

	if (hBuyInDLG) { // there already is one

		SetFocus(hBuyInDLG);

		return chips;

	}

	// 990819HK: if the table was shut down just before we got here, we've got no context

	// for buying in -- just abort

	if (!t || !t->table_serial_number) {

		return 0;

	}

	struct BuyInDLGInfo bi;

	zstruct(bi);

	bi.ti = t;

	bi.minimum_allowed = minimum_required;

	bi.seating_position = seating_position;  

	bi.buy_in_type = jts;

  #if ADMIN_CLIENT	// 20000120HK

	if (iAdminClientFlag && LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

		bi.minimum_allowed = 0;

	}

  #endif

	int bank_chips = ( (t->chip_type == CT_REAL || t->tournament_table) ? RealChipsInBank : FakeChipsInBank);

	bi.chips = bank_chips;

	int dlg_template = (t->tournament_table ? IDD_BUY_IN_TOURN : IDD_BUY_IN);

	do {


		chips = DialogBoxParam(hInst, MAKEINTRESOURCE(dlg_template),

			t->hwnd, (DLGPROC)dlgTableBuyInFunc, (LPARAM)(&bi));

		// these could change while the dlg is up

		bank_chips = ( (t->chip_type == CT_REAL) ? RealChipsInBank : FakeChipsInBank);

	} while (chips !=0 && (chips > bank_chips || chips < (int)bi.minimum_allowed));

	if (chips) {	// something other than 0 or cancel

		struct CardRoom_JoinTable jt;

		zstruct(jt);

		jt.table_serial_number = t->table_serial_number;

		jt.status = (BYTE8)jts;	// request to join this table.

		jt.buy_in_chips = chips;

		jt.seating_position = (BYTE8)seating_position;

		// Send the appropriate JTS packet.

		SendDataStructure(DATATYPE_CARDROOM_JOIN_TABLE, &jt, sizeof(jt));

		if (t->table_serial_number==SeatAvail.table_serial_number) {

			zstruct(SeatAvail);	// allow new Seat Avail notifications to come in.

		}

		t->minimum_buy_in = 0;	// reset it -- it'll be properly set next time

	}
    
	return chips;
}



/**********************************************************************************

 Function *OurHandDescription(TableInfo *t)

 Date: HK1999/07/26

 Purpose: return an ascii description of our hand in the out parameter

***********************************************************************************/

void OurHandDescription(struct TableInfo *t, char *high_hand_out, char *low_hand_out)

{

	// blank the output string if we don't build a description

	*high_hand_out = 0;

	*low_hand_out = 0;

	// build the pocket hand

	Hand h_pocket;

	pr(("%s(%d) OurHandDescription called with seating position %d, s_gameover = %d s_gameflow = %d\n", _FL, 

		t->GamePlayerData.seating_position, t->GamePlayerData.s_gameover, t->GamePlayerData.s_gameflow));

	for (int i=0; i < MAX_PRIVATE_CARDS; i++) {

		Card pocket_card = t->GamePlayerData.cards[t->GamePlayerData.seating_position][i];

		if (pocket_card != CARD_NO_CARD && pocket_card != CARD_HIDDEN) {

			h_pocket.Add(pocket_card);

		}

	}

	// build the flop

	Hand h_flop;

	for (int j=0; j < MAX_PUBLIC_CARDS; j++) {

		Card flop_card = t->GamePlayerData.common_cards[j];

		if (flop_card != CARD_NO_CARD && flop_card != CARD_HIDDEN) {

			h_flop.Add(flop_card);

		}

	}

	int pocket_count = h_pocket.CardCount();

	int flop_count = h_flop.CardCount();

	// for whatever we're testing, there must be at least 5 cards to work with

	GameRules game_rules = (GameRules)t->GameCommonData.game_rules;

	Hand best_hand;

	Hand low_hand;

	int testable_hand = TRUE;

	switch (game_rules) {

	case GAME_RULES_HOLDEM:

		// full pocket and 3+ flop

		if (pocket_count != 2 || flop_count < 3) {

			testable_hand = FALSE;

		  #if ADMIN_CLIENT

			if (iAdminClientFlag) {

				if (pocket_count==2 && flop_count==0) {

					// display the starting hand ranking from Sklansky's book

					// (pages 13/14 - The First Two Cards)

					struct PocketRanking {

						Card card1, card2;

						int suited_rank, non_suited_rank;

					} pocket_rankings[] = {

						//				Suited	NotSuited

						// First, the pairs (suited/nonsuited field is ignored)

						Ace,	Ace,	1,		1,

						King,	King,	1,		1,

						Queen,	Queen,	1,		1,

						Jack,	Jack,	1,		1,

						Ten,	Ten,	2,		2,

						Nine,	Nine,	3,		3,

						Eight,	Eight,	4,		4,

						Seven,	Seven,	5,		5,

						Six,	Six,	6,		6,

						Five,	Five,	6,		6,

						Four,	Four,	7,		7,

						Three,	Three,	7,		7,

						Two,	Two,	7,		7,



						// Non-Pair table

						Ace,	King,	1,		2,

						Ace,	Queen,	2,		3,

						Ace,	Jack,	2,		4,

						Ace,	Ten,	3,		6,

						Ace,	Nine,	5,		8,

						Ace,	Eight,	5,		-1,

						Ace,	Seven,	5,		-1,

						Ace,	Six,	5,		-1,

						Ace,	Five,	5,		-1,

						Ace,	Four,	5,		-1,

						Ace,	Three,	5,		-1,

						Ace,	Two,	5,		-1,



						King,	Queen,	2,		4,

						King,	Jack,	3,		5,

						King,	Ten,	4,		6,

						King,	Nine,	6,		8,

						King,	Eight,	7,		-1,

						King,	Seven,	7,		-1,

						King,	Six,	7,		-1,

						King,	Five,	7,		-1,

						King,	Four,	7,		-1,

						King,	Three,	7,		-1,

						King,	Two,	7,		-1,



						Queen,	Jack,	3,		5,

						Queen,	Ten,	4,		6,

						Queen,	Nine,	5,		8,

						Queen,	Eight,	7,		-1,



						Jack,	Ten,	3,		5,

						Jack,	Nine,	4,		7,

						Jack,	Eight,	6,		8,

						Jack,	Seven,	8,		-1,



						Ten,	Nine,	4,		7,

						Ten,	Eight,	5,		8,

						Ten,	Seven,	7,		-1,



						Nine,	Eight,	4,		7,

						Nine,	Seven,	5,		-1,

						Nine,	Six,	8,		-1,



						Eight,	Seven,	5,		8,

						Eight,	Six,	6,		-1,

						Eight,	Five,	8,		-1,



						Seven,	Six,	5,		8,

						Seven,	Five,	6,		-1,

						Seven,	Four,	8,		-1,



						Six,	Five,	5,		8,

						Six,	Four,	7,		-1,



						Five,	Four,	6,		8,

						Five,	Three,	8,		-1,



						Four,	Three,	7,		-1,

						Four,	Two,	8,		-1,



						Three,	Two,	8,		-1,



						CARD_NO_CARD, CARD_NO_CARD, 0, 0

					};

					struct PocketRanking *pr = pocket_rankings;

					Card card1 = h_pocket.GetCard(0);

					Card card2 = h_pocket.GetCard(1);

					
					int group = -1;

					while (pr->card1 != CARD_NO_CARD) {

						if ((RANK(card1)==pr->card1 &&

							 RANK(card2)==pr->card2) ||

							(RANK(card2)==pr->card1 &&

							 RANK(card1)==pr->card2))

						{

							// Found a match.

							group = pr->non_suited_rank;

							if (SUIT(card1)==SUIT(card2)) {	// suits match

								group = pr->suited_rank;

							}

							break;

						}

						pr++;

					}

					if (group >= 0) {

						sprintf(high_hand_out, "My Best Hand: %d", group);

						t->current_hand_rank = group;// 1 is best

					} else {	

						strcpy(high_hand_out,  "My Best Hand: -");	// default

						t->current_hand_rank = 10;	// 10 is worse than anything

					}

				}

			}

		  #endif

		}

		break;

	case GAME_RULES_STUD7_HI_LO:

	case GAME_RULES_STUD7:

		// at least 5 cards

		if (pocket_count < 5) {

			testable_hand = FALSE;

		}

		break;

	case GAME_RULES_OMAHA_HI_LO:

	case GAME_RULES_OMAHA_HI:

		// full pocket and 3+ flop

		if (pocket_count !=4 || flop_count < 3) {

			testable_hand = FALSE;

		}

		break;

	default:

		if (game_rules) {

			Error(ERR_INTERNAL_ERROR, "%s(%d) Invalid game type (%d) trying to describe hand",_FL, game_rules);

		}

		return;

	}

	// hand is valid to describe..?

	if (testable_hand) {

		Poker *poker = new Poker;

		if (poker) {

			poker->FindBestHand(game_rules, h_pocket, h_flop, &best_hand, &low_hand);

			poker->GetHandCompareDescription(&best_hand, NULL, high_hand_out, TRUE);

			int show_low = (game_rules == GAME_RULES_OMAHA_HI_LO || game_rules == GAME_RULES_STUD7_HI_LO);

			if (show_low && poker->ValidLowHand(&low_hand)) {

				poker->GetHandCompareDescription(&low_hand, NULL, low_hand_out, FALSE);

			}

			delete poker;

		} else {

			Error(ERR_INTERNAL_ERROR, "%s(%d) Couldn't create Poker object",_FL);

		}

	}

}

