//****************************************************************


//

// WaitList.cpp : Waiting List related routines for client end

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"

#include "time.h"



static WORD32 WaitListTableSerialNum;

static int WaitListTableStakes;



static HWND	  WaitList_hwnd;

HWND   SeatAvail_hwnd;

struct CardRoom_SeatAvail SeatAvail;	// current SeatAvail structure from server

WORD32 SeatAvailSecondCounter;			// SecondCounter when SeatAvail structure received from Server.

#if 0	//19991102MB

static WORD32 iPromptUser_table_serial_number;

#endif



//****************************************************************


//

// Mesage handler for Join Waiting List dialog box

//

BOOL CALLBACK dlgFuncWaitList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[200];



	switch (message) {

	case WM_INITDIALOG:

	{

		// Fill in all our default string values.

		if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
			HWND hDlg;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
		}
		
		
		WaitList_hwnd = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		struct CardRoom_TableSummaryInfo tsi;

		ErrorType err = FindTableSummaryInfo(WaitListTableSerialNum, &tsi);

		if (err) {

			Error(ERR_ERROR, "%s(%d) dlgFuncWaitList() cannot find table %d's info.", _FL, WaitListTableSerialNum);

			return TRUE;

		}



		// enable/disable the individual table selection depending on whether

		// a waiting list is required for that table.

		int enable_single_table = FALSE;

		if (tsi.flags & TSIF_WAIT_LIST_REQUIRED) {

			enable_single_table = TRUE;

		}

		if (tsi.flags & TSIF_TOURNAMENT) {

			enable_single_table = FALSE;

		}



		EnableWindow(GetDlgItem(hDlg, IDC_WAIT_SINGLE_TABLE), enable_single_table);

		ShowWindow(GetDlgItem(hDlg, IDC_WAIT_SINGLE_TABLE_TEXT), enable_single_table ? SW_SHOW : SW_HIDE);



		sprintf(str, "Table %s", tsi.table_name);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_WAIT_SINGLE_TABLE), str);



		if (tsi.waiting_list_length==0)

			strcpy(str, "You are first in line.");

		else if (tsi.waiting_list_length==1)

			strcpy(str, "There is only one player ahead of you.");

		else

			sprintf(str, "There are %d players ahead of you.", tsi.waiting_list_length);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_WAIT_SINGLE_TABLE_TEXT), str);



		WaitListTableStakes = tsi.big_blind_amount;

		if (tsi.flags & TSIF_REAL_MONEY) {

			char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];

			CurrencyString(cs1, tsi.big_blind_amount*GameStakesMultipliers[tsi.game_rules-GAME_RULES_START], (tsi.flags & TSIF_REAL_MONEY) ? CT_REAL : CT_PLAY);

			CurrencyString(cs2, tsi.big_blind_amount*GameStakesMultipliers[tsi.game_rules-GAME_RULES_START]*2, (tsi.flags & TSIF_REAL_MONEY) ? CT_REAL : CT_PLAY);

			sprintf(str, "First available %s/%s table", cs1, cs2);

		} else if (tsi.flags & TSIF_TOURNAMENT) {

			char cs1[MAX_CURRENCY_STRING_LEN];

			sprintf(str, "Next %s tournament", 

					CurrencyString(cs1, tsi.big_blind_amount, CT_REAL));

			EnableWindow(GetDlgItem(hDlg, IDC_MIN_PLAYERS_VALUE), FALSE);

			EnableWindow(GetDlgItem(hDlg, IDC_MIN_PLAYERS_SPINNER), FALSE);

		} else {	// assume play money

			strcpy(str, "First available play money table");

		}

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_WAIT_ANY_TABLE), str);



		if (!Defaults.preferred_min_players) {

			Defaults.preferred_min_players = 2;

		}

		int x = 0;

		static int MaxWaitPlayersPerDisplayTab[ACTUAL_CLIENT_DISPLAY_TABS] =  {8,8,6,2,8,6,10};

		if (tsi.flags & TSIF_TOURNAMENT) {	// tournament table?

			x = tsi.max_player_count;		// always wait for a full table

		} else {	// regular tables...

			x = Defaults.preferred_min_players;

			x = max(2, x);

			x = min(MaxWaitPlayersPerDisplayTab[tsi.client_display_tab_index], x);

		}

		sprintf(str, "%d", x);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_MIN_PLAYERS_VALUE), str);

		// Set the range for the spinner (and at the same time invert the

		// actions for the up/down buttons).

		SendMessage(GetDlgItem(hDlg, IDC_MIN_PLAYERS_SPINNER), UDM_SETRANGE,0,

				(LPARAM)MAKELONG(MaxWaitPlayersPerDisplayTab[tsi.client_display_tab_index],(short)2));



		// Set the default radio button

		CheckDlgButton(hDlg, IDC_WAIT_SINGLE_TABLE, FALSE);

		CheckDlgButton(hDlg, IDC_WAIT_ANY_TABLE, TRUE);



		// If we're already playing at a table of the same type with

		// the same stakes then we're not allowed to join a list for

		// 'any' table like that.  The server will prevent us from

		// doing so, but we should disable the control just to make

		// sure it doesn't confuse the user.

		// Search open tables to see if this is the case.

		for (int i=0 ; i<MAX_TABLES ; i++) {

			if (Table[i].table_serial_number &&

				!Table[i].watching_flag &&

				Table[i].client_display_tab_index==tsi.client_display_tab_index &&

				Table[i].game_rules==tsi.game_rules &&

				Table[i].GameCommonData.big_blind_amount==tsi.big_blind_amount)

			{

				pr(("%s(%d) We're already playing at a $%d/$%d table.  Disabling 'join any' control.\n",

						_FL, tsi.big_blind_amount, tsi.big_blind_amount*2));

				EnableWindow(GetDlgItem(hDlg, IDC_WAIT_ANY_TABLE), FALSE);

				EnableWindow(GetDlgItem(hDlg, IDC_MIN_PLAYERS_VALUE), FALSE);

				EnableWindow(GetDlgItem(hDlg, IDC_MIN_PLAYERS_SPINNER), FALSE);



				// If we're already playing at THIS table, disable that radio button as well

				if (Table[i].table_serial_number==tsi.table_serial_number) {

					EnableWindow(GetDlgItem(hDlg, IDC_WAIT_SINGLE_TABLE), FALSE);

					MessageBox(hDlg,

							"You are already seated at this table.",

							"Cannot join waiting list",

							MB_OK);

					EndDialog(hDlg, 1);

					return FALSE;

				}

			}

		}



		// Finally, change the title of our dialog box to reflect the

		// type of game we're playing.

		sprintf(str, "Join The %s waiting list...", GameRuleNames[tsi.game_rules-GAME_RULES_START]);

		SetWindowTextIfNecessary(hDlg, str);



		// Now show the dialog box.

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	}

	case WM_CLOSE:

		EndDialog(hDlg, 1);

		return TRUE;	// We DID process this message.

	case WM_COMMAND:

		// Process other buttons on the window...

		switch (LOWORD(wParam)) {

		case IDOK:

			{

				pr(("%s(%d) IDOK received.\n",_FL));

				EndDialog(hDlg, 0);



				// Send a request off to the card room.

				char str[10];

				str[0] = 0;

				GetWindowText(GetDlgItem(hDlg, IDC_MIN_PLAYERS_VALUE), str, 10);

				Defaults.preferred_min_players = atoi(str);

				Defaults.changed_flag = TRUE;



				struct CardRoom_TableSummaryInfo tsi;

				FindTableSummaryInfo(WaitListTableSerialNum, &tsi);



				struct CardRoom_JoinWaitList jwl;

				zstruct(jwl);

				if (IsDlgButtonChecked(hDlg, IDC_WAIT_SINGLE_TABLE)) {

					jwl.table_serial_number = WaitListTableSerialNum;

				} else {

					// Any table will do... fill in min_players_required.

					jwl.min_players_required = (BYTE8)Defaults.preferred_min_players;

				}

				jwl.desired_stakes = WaitListTableStakes;

				jwl.status = 1;	// join

				jwl.client_display_tab_index = tsi.client_display_tab_index;

				jwl.game_rules = tsi.game_rules;

				jwl.chip_type = CT_PLAY;

				if (tsi.flags & TSIF_REAL_MONEY) {

					jwl.chip_type = CT_REAL;

				} else if (tsi.flags & TSIF_TOURNAMENT) {

					jwl.chip_type = CT_TOURNAMENT;

				}

				SendDataStructure(DATATYPE_CARDROOM_JOIN_WAIT_LIST, &jwl, sizeof(jwl));



				// If we're already playing at a tournament table and they've joined

				// a tournament waiting list, be sure to tell them they won't get

				// called until they are finished at the previous tournament table.

				if ((tsi.flags & TSIF_TOURNAMENT) && iSeatedAtATournamentTableFlag) {

					MessageBox(NULL,

						"You are limited to playing in one tournament at a time.\n"

						"\n"

						"Your name will be added to the waiting list but you will not\n"

						"be called to any new tournaments until you are finished in\n"

						"your current tournament.",

						"Tournament waiting list information",

						MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

				}

			}

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			pr(("%s(%d) IDCANCEL received.\n",_FL));

			EndDialog(hDlg, 1);

			return TRUE;	// We DID process this message.

		}

		break;

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		WaitList_hwnd = 0;

		break;

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

//
//

// Display the waiting list dialog.  Show existing one

// if it's already open.  Does not return until user finishes.

//

void WaitListDialog(HWND parent, WORD32 table_serial_number)

{

	if (WaitList_hwnd) {

		// Dialog box is already open... top existing one.

		SetForegroundWindow(WaitList_hwnd);

	} else {

		int valid_login = LogInToServer("You must log in before joining waiting lists");

		if (valid_login) {

				// If this is for a real money table, make sure their account
				// has the necessary privilege level.

				static struct CardRoom_TableSummaryInfo tsi;

				zstruct(tsi);

				FindTableSummaryInfo(table_serial_number, &tsi);

				// if it's a real money or tournament table, send them to the real money setup...

				if (tsi.flags & TSIF_REAL_MONEY || tsi.flags & TSIF_TOURNAMENT) {	

					if (LoggedInPrivLevel < ACCPRIV_REAL_MONEY) {

						// priv not high enough yet.  Go to set up.
						InitiateRealMoneyAccountSetup();
						
						return;	// don't bring up waiting list stuff.

					}
				}


				WaitListTableSerialNum = table_serial_number;

				DialogBox(hInst, MAKEINTRESOURCE(IDD_JOIN_WAIT_LIST),

							parent, dlgFuncWaitList);
		}//if login=valid
	}//else
}//WaitListDialog



//*********************************************************

//
//

// Display the unjoin waiting list dialog.  Show existing one

// if it's already open.  Does not return until user finishes.

// Returns IDOK if user decided to unjoin.

//

int UnjoinWaitListDialog(HWND parent, WORD32 table_serial_number, int prompt)

{

	int result = IDCANCEL;

  #if 0	//19991129MB what the hell was I thinking last May?

	//if (WaitList_hwnd) {

	//	// Dialog box is already open... top existing one.

	//	SetForegroundWindow(WaitList_hwnd);

	//} else {

	//	WaitListTableSerialNum = table_serial_number;

	//	result = MessageBox(parent,

	//			"Are you sure you want to unjoin this waiting list?",

	//			"Unjoin Waiting List...",

	//			MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1|MB_APPLMODAL);

	//	if (result==IDYES) {

	//		// OK... lets get off the waiting list.

	//		struct CardRoom_TableSummaryInfo tsi;

	//		FindTableSummaryInfo(table_serial_number, &tsi, NULL);

	//		struct CardRoom_JoinWaitList jwl;

	//		zstruct(jwl);

	//		jwl.table_serial_number = table_serial_number;

	//		jwl.desired_stakes = tsi.big_blind_amount;

	//		jwl.real_money = (BYTE8)((tsi.flags & TSIF_REAL_MONEY) ? TRUE : FALSE);

	//		jwl.status = 0;	// unjoin

	//		SendDataStructure(DATATYPE_CARDROOM_JOIN_WAIT_LIST, &jwl, sizeof(jwl));

	//		zstruct(SeatAvail);	// allow new notifications to come in.

	//	}

	//}

  #else

	if (prompt == 1) {

		result = MessageBox(parent,

				"Are you sure you want to unjoin this waiting list?",

				"Unjoin Waiting List...",

				MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1|MB_APPLMODAL);

	}

	else{

		result=IDYES;

	}





	if (result==IDYES) {

		// OK... lets get off the waiting list.

		struct CardRoom_TableSummaryInfo tsi;

		FindTableSummaryInfo(table_serial_number, &tsi);

		struct CardRoom_JoinWaitList jwl;

		zstruct(jwl);

		jwl.table_serial_number = table_serial_number;

		jwl.desired_stakes = tsi.big_blind_amount;

		jwl.client_display_tab_index = tsi.client_display_tab_index;

		jwl.game_rules = tsi.game_rules;

		if (tsi.flags & TSIF_REAL_MONEY) {

			jwl.chip_type = (BYTE8)CT_REAL;

		} else if (tsi.flags & TSIF_TOURNAMENT) {

			jwl.chip_type = (BYTE8)CT_TOURNAMENT;

		} else {

			jwl.chip_type = (BYTE8)CT_PLAY;

		}

		jwl.status = 0;	// unjoin

		SendDataStructure(DATATYPE_CARDROOM_JOIN_WAIT_LIST, &jwl, sizeof(jwl));

		zstruct(SeatAvail);	// allow new notifications to come in.

	}

  #endif

	return result;

}



//*********************************************************

//
//

// Update the information on the SeatAvail dialog box.

//

void UpdateSeatAvailInfo(HWND hDlg)

{

	char str[300];

	zstruct(str);

	struct CardRoom_TableSummaryInfo tsi;

	ErrorType err = FindTableSummaryInfo(SeatAvail.table_serial_number, &tsi);

	if (err != ERR_NONE) {

		return;	// don't make any changes if we couldn't find the table info

	}

	int time_left = max(0, ((int)SeatAvail.timeout - (int)(SecondCounter - SeatAvailSecondCounter)));

	char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN], cs3[MAX_CURRENCY_STRING_LEN];

	ChipType chip_type = CT_PLAY;

	if (tsi.flags & TSIF_REAL_MONEY) {

		chip_type = CT_REAL;

	}

	if (tsi.flags & TSIF_TOURNAMENT) {

		chip_type = CT_TOURNAMENT;

	}

	CurrencyString(cs1, tsi.big_blind_amount*GameStakesMultipliers[tsi.game_rules - GAME_RULES_START], chip_type);

	CurrencyString(cs2, tsi.big_blind_amount*GameStakesMultipliers[tsi.game_rules - GAME_RULES_START]*2, chip_type);

	zstruct(cs3);

	kp(("%s(%d) Server says we got to skip ahead of %d other players on the waiting list.\n", _FL, SeatAvail.skipped_players));

  #if 1	//20000628MB

	// Use hands per hour to determine if the table is new.  It's not perfect, but

	// it's better than nothing.

	if (!tsi.hands_per_hour || !tsi.avg_pot_size || !tsi.player_count || chip_type==CT_TOURNAMENT)

  #else

	// 20000625HK: Note that this is not always a valid assumption so we might think it's a

	// new table when in fact it's just a not-full table that's calling:

	// (This is a new table... we're not really sure just how many players will join us.)

	// build a descriptive string for the table

	if (SeatAvail.potential_players)

  #endif

	{

        if (chip_type==CT_TOURNAMENT) {

        	CurrencyString(cs1, tsi.big_blind_amount, CT_REAL);

        	CurrencyString(cs2, tsi.tournament_prize_pool, CT_REAL);

    		sprintf(str, "A %s %s tournament is starting at \"%s\" (%s prize pool)\n",

    				cs1, GameRuleNamesShort[tsi.game_rules - GAME_RULES_START], tsi.table_name, cs2);

        } else {

    		sprintf(str, "A new %s/%s %s game is starting at table \"%s\".\n",

    				cs1, cs2, GameRuleNamesShort[tsi.game_rules - GAME_RULES_START], tsi.table_name);

        }

		char others_waiting_str[50];

		zstruct(others_waiting_str);

		int others_waiting = SeatAvail.potential_players-1;

		if (!others_waiting) {

			sprintf(others_waiting_str, ".");

		} else if (others_waiting == 1) {

			sprintf(others_waiting_str, " and one other waiting.");

		} else {

			sprintf(others_waiting_str, " and %d others waiting.", others_waiting);

		}



		if (!SeatAvail.number_of_players) {

			// We're the first.

			if (others_waiting==1) {	// probably a one on one table...

				sprintf(str+strlen(str), "One other player is also waiting.\n");

			} else {

				sprintf(str+strlen(str), "%d other players are also waiting.\n", others_waiting);

			}

		} else if (SeatAvail.number_of_players==1) {

			sprintf(str+strlen(str), "There is one player already seated%s\n", others_waiting_str);

		} else {

			sprintf(str+strlen(str), "There are %d players already seated%s\n",

					SeatAvail.number_of_players, others_waiting_str);

		}

		if (SeatAvail.number_of_players==0 || SeatAvail.number_of_players==1) {

			sprintf(str+strlen(str), "You have %ds to take your seat.", time_left);

		} else {

			sprintf(str+strlen(str), "You have %ds to join them.", time_left);

		}

	} else {

		sprintf(str,

			"There is a seat open at table %s (%s/%s %s).\n"

			"Stats: %d players,  %d%% plrs/flop,  %d hands/hr, %s avg. pot\n"

			"You have %ds to claim your seat.", 

			tsi.table_name, cs1, cs2, GameRuleNamesShort[tsi.game_rules - GAME_RULES_START], tsi.player_count, 

			tsi.players_per_flop, tsi.hands_per_hour, 

			CurrencyString(cs3, tsi.avg_pot_size, (tsi.flags & TSIF_REAL_MONEY) ? CT_REAL:CT_PLAY, FALSE, TRUE), time_left);

	}

	// If they don't have this table open and they don't have

	// room to open another window, change the message.

	if (OpenTableCount() >= MAX_TABLES &&

			(TableIndexFromSerialNumber(SeatAvail.table_serial_number)<0)) {

		strcat(str, "  You must leave a table before you can join a new one.");

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDOK), "OK");

	}

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SEAT_AVAIL_TEXT), str);

}



//****************************************************************

//
//

// Mesage handler for Seat Avail dialog box

//

BOOL CALLBACK dlgFuncSeatAvail(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		// Fill in all our default string values.

		SeatAvail_hwnd = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		UpdateSeatAvailInfo(hDlg);

		// Now show the dialog box.

		ShowWindow(hDlg, SW_SHOW);

		SetForegroundWindow(hDlg);

		SetWindowPos(hDlg, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);

		SetTimer(hDlg, WM_TIMER, 1000, NULL);	// one second timer

		return TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, 1);

		return TRUE;	// We DID process this message.

	case WM_COMMAND:

		// Process other buttons on the window...

		switch (LOWORD(wParam)) {

		case IDOK:

			pr(("%s(%d) 'Go to table' pressed.\n",_FL));

			JoinTable(SeatAvail.table_serial_number);

			// If the table is already up, let them sit down now.

			{

				int i = TableIndexFromSerialNumber(SeatAvail.table_serial_number);

				if (i>=0) {

					//kp(("%s(%d) Setting sit_down_allowed = TRUE for table %d\n", _FL, SeatAvail.table_serial_number));

					Table[i].sit_down_allowed = TRUE;	// allow to sit down.

					FlagRedrawTable(i);	//19991120MB: possibly missing... might need to be called.

				}

			}

			EndDialog(hDlg, 0);

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			pr(("%s(%d) 'Take me off the list' pressed.\n",_FL));

			if (UnjoinWaitListDialog(hDlg, SeatAvail.table_serial_number, 1 )==IDYES)  {

				zstruct(SeatAvail);	// clear it out immediately now that we're not on the list anymore.

				EndDialog(hDlg, 1);

			}

			return TRUE;	// We DID process this message.

		case IDRETRY:

			pr(("%s(%d) 'Give me another minute' pressed.\n",_FL));

			EndDialog(hDlg, 1);

			// If this is a tournament table, give them a more forceful message

			// that indicates that other people cannot start until this seat

			// is taken or given up.

			{

				struct CardRoom_TableSummaryInfo tsi;

				zstruct(tsi);

				FindTableSummaryInfo(SeatAvail.table_serial_number, &tsi);

				if (tsi.flags & TSIF_TOURNAMENT) {

					MessageBox(NULL,

						"In consideration of your fellow players,\n"

						"please decide quickly.",

						"Other players are waiting",

						MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

				}

			}

			return TRUE;	// We DID process this message.

		}

		break;

	case WM_TIMER:	// these should arrive in 1s intervals.

		UpdateSeatAvailInfo(hDlg);

		break;

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		SeatAvail_hwnd = 0;

		KillTimer(hDlg, WM_TIMER);

		break;

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

//
//

// Display/update the 'seat avail' dialog box whenever something

// new arrives from the server.

//

void UpdateSeatAvailDialog(HWND parent)

{

	pr(("%s(%d) UpdateSeatAvailDialog() has been called. timeout=%d, table #%d\n",_FL, SeatAvail.timeout, SeatAvail.table_serial_number));

	// If time ran out, close anything we might have open.

	if (!SeatAvail.timeout) {

		// Time ran out on us.

		int i = TableIndexFromSerialNumber(SeatAvail.table_serial_number);

		if (i>=0) {

			Table[i].sit_down_allowed = FALSE;	// we're no longer allowed to sit down.

		}

		if (SeatAvail_hwnd) {

			PostMessage(SeatAvail_hwnd, WM_CLOSE, 0, 0);

		}

		zstruct(SeatAvail);

		// We don't need to cancel the waiting list request... the server

		// will automatically take us off the list if we don't answer in time.

	} else {

		// play a sound to wake them up

		PPlaySound(SOUND_WAKE_UP);	// 'bigger' sound

		if (SeatAvail_hwnd) {

			// Dialog box is already open... top existing one.

			UpdateSeatAvailInfo(SeatAvail_hwnd);

			SetForegroundWindow(SeatAvail_hwnd);

			SetWindowPos(SeatAvail_hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);

		} else {

			DialogBox(hInst, MAKEINTRESOURCE(IDD_SEAT_AVAIL),

							parent, dlgFuncSeatAvail);

		}

	}

}



#if 0	//19991102MB

//****************************************************************

//
//

// Mesage handler for Prompt user to join a waiting list dialog box

//

BOOL CALLBACK dlgFuncPromptJoinWaitList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

	{

		// Fill in all our default string values.

		// "There aren't any tables currently playing.

		// We've already got 2 players on a waiting list

		// for a $3/$6 game starting at 7:00pm (or earlier

		// if there is enough demand).  Would you like to

		// be added to that waiting list now?"

		struct CardRoom_TableSummaryInfo tsi;

		PokerGame game_type;

		ErrorType err = FindTableSummaryInfo(iPromptUser_table_serial_number, &tsi, &game_type);

		if (err) {

			Error(ERR_ERROR, "%s(%d) dlgFuncWaitList() cannot find table %d's info.", _FL, WaitListTableSerialNum);

			return TRUE;

		}



		char str[200];

		char str2[150];

		strcpy(str, "There are no tables currently playing.  ");

		if (tsi.waiting_list_length==0)

			strcat(str, "We have");

		else if (tsi.waiting_list_length==1)

			strcat(str, "We already have 1 player on");

		else {

			sprintf(str2, "We already have %d players on", tsi.waiting_list_length);

			strcat(str, str2);

		}

		// Calculate the time of the top of the next hour.

		time_t now = time(NULL);

		struct tm *t = localtime(&now);

		int hour = t->tm_hour+1;

		if (hour > 23) hour = 0;

		char *ampm = hour >= 12 ? "pm" : "am";

		hour = hour % 12;

		if (!hour)

			hour = 12;

		char game_type_str[100];

		// CT_TOURNAMENT NOT YET HANDLED !!!

		if (tsi.flags & TSIF_REAL_MONEY) {

			char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];

			sprintf(game_type_str, "%s/%s", 

					CurrencyString(cs1, tsi.big_blind_amount*GameStakesMultipliers[game_type], tsi.flags & TSIF_REAL_MONEY),

					CurrencyString(cs2, tsi.big_blind_amount*GameStakesMultipliers[game_type]*2, tsi.flags & TSIF_REAL_MONEY));

		} else {

			strcpy(game_type_str, "play money");

		}

		sprintf(str2, " a waiting list for a %s game starting at "

					"%d:%02d%s (or earlier if there is enough demand).  "

					"Would you like to be added to the waiting list now?",

					game_type_str, hour, 0, ampm);

		strcat(str, str2);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), str);



		// Now show the dialog box.

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	}

	case WM_CLOSE:

		EndDialog(hDlg, 1);

		return TRUE;	// We DID process this message.

	case WM_COMMAND:

		// Process other buttons on the window...

		switch (LOWORD(wParam)) {

		case IDOK:

			{

				pr(("%s(%d) IDOK received.\n",_FL));

				EndDialog(hDlg, 0);

				WaitListDialog(hCardRoomDlg, iPromptUser_table_serial_number);

			}

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			pr(("%s(%d) IDCANCEL received.\n",_FL));

			EndDialog(hDlg, 1);

			return TRUE;	// We DID process this message.

		}

		break;

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

//
//

// Display a dialog box to prompt the user to join a waiting

// list for the first game of a particular type.  This is designed

// to encourage them to come back at a pre-determined time (the

// top of the hour) and start a table.

//

void PromptUserToJoinWaitList(WORD32 table_serial_number)

{

	//kp(("%s(%d) PromptUserToJoinWaitList(%d) has been called.\n", _FL, table_serial_number));

	iPromptUser_table_serial_number = table_serial_number;

	DialogBox(hInst, MAKEINTRESOURCE(IDD_PROMPT_JOIN_WAIT_LIST),

						hCardRoomDlg, dlgFuncPromptJoinWaitList);

}

#endif

