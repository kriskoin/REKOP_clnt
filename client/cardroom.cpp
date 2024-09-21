//****************************************************************
//
// CardRoom.cpp : Card Room related routines for client end
//
//****************************************************************


#define DISP 0
//sdfsfsdf
#include "stdafx.h"

#define PLAY_MONEY_ONLY	0
#define FOR_COMPANY_ONLY    0

#define INCLUDE_HANDS_PER_HOUR	(1)
#define INCLUDE_TABLE_RAKE_PER_HOUR (ADMIN_CLIENT && 1)

HWND hCardRoomDlg;			// handle to cardroom dialog box (if open)
HWND hGameListCtrl;			// handle to Game List view control
HWND hCardRoomToolTips;		// handle to the tooltips control for cardroom dialog box
HWND hGoingToTableDlg;		// handle to "going to table" dlg
HWND hChatMonitorDlg = NULL;// handle to chat monitor dlg

int iCardRoomRedrawNeeded;	// set if cardroom dialog needs redrawing
int iCardRoomDlgMinimizedFlag;	// set if the cardroom dialog is minimized.
ClientDisplayTabIndex GameListTabIndex;	// DISPLAY_TAB_* for the current game list view control

HBITMAP hCardRoomBgnd;		// handle to background picture for dialog box.
//rgong 04/04/2002 - 2
HBITMAP original_hbm;
//end rgong
HPALETTE hCardRoomPalette;	// handle to palette for bgnd picture
HFONT hCardRoomButtonFont;
HFONT hShotClockFont;		// handle to font used on the shot clock

// Current table serial number we're displaying info about.  This
// variable also doubles as the 'current focus game' on the list view control.
WORD32 dwTableInfoSerialNum;

int iDisableRequestGameInfo;	// set if selecting a list view item should NOT request new game info.
#if ADMIN_CLIENT
  int AutoJoinDefaultTable;		// debug: auto-join (and play) the default table.
  static int chat_monitor_index;
  #define MON_BUFFER_LEN ((MAX_CHAT_MSG_LEN+3) * ADMIN_CHAT_LINES)
  char ChatMonitorBuffer[MON_BUFFER_LEN];
  int FreezeChatMonitor = FALSE;
  char ChatMonitorString[MAX_COMMON_STRING_LEN];
#endif

struct CardRoom_TableSummarySerialNums Server_TableSerialNums;
struct CardRoom_TableSummarySerialNums Our_TableSerialNums;
struct MiscClientMessage CardRoom_misc_client_msg;
struct MiscClientMessage Ecash_misc_client_msg;

time_t ShotClockLocalDate;	// shot clock time in local time
time_t ShotClockServerDate;	// shot clock time in server time
char ShotClockMsg1[SHOTCLOCK_MESSAGE_LEN];
char ShotClockMsg2[SHOTCLOCK_MESSAGE_LEN];
WORD32 ShotClockFlags;		// SCUF_* (copied from server)

char CardRoomWindowPosName[30] =  {"CardRoom Window Position"};

static int WaitListButtonState;	// 0='unjoin', 1='join'
static int iMenuBlinkCount;		// # of times left to blink the first menu item

struct DlgToolTipText CardRoomDlgToolTipText[] = {
	IDC_EXIT,			"Log off the Poker server and close this program",
	IDC_CASHIER,		"The Cashier handles transferring money into and out of your account",
	IDC_ACCOUNT_INFO,	"View and change your Account information such as name, address, etc.",
	IDC_GOTO_GAME,		"Go to a table to watch, then sit down to play.",
	IDC_HIDE_PLAY_MONEY,"Hides any tables which aren't using real money",
	IDC_JOIN_WAIT_LIST,	"Join or unjoin the waiting list for the highlighted table",
	IDC_GAME_TYPE_TAB,	"Select different game types",
//	IDC_LINK,	"https://github.com/kriskoinOnline Entertainment Development.",
	IDC_LINK,	"e-Media Poker Help Center.",
  #if ADMIN_CLIENT
	IDC_RECONNECT,		"Debug: disconnect and reconnect to server as if network connection was lost",
  #endif
	0,0
};

static int iGotoTableMenuIDs[MAX_TABLES] =  {
			IDM_GOTO_TABLE1, IDM_GOTO_TABLE2, IDM_GOTO_TABLE3, IDM_GOTO_TABLE4};

// Ordering of the tabs on the tab control...
static ClientDisplayTabIndex GameTypeTabOrders[ACTUAL_CLIENT_DISPLAY_TABS] = {
	DISPLAY_TAB_HOLDEM,
	DISPLAY_TAB_OMAHA_HI,
	DISPLAY_TAB_OMAHA_HI_LO,
	DISPLAY_TAB_STUD7,
	DISPLAY_TAB_STUD7_HI_LO,
	DISPLAY_TAB_ONE_ON_ONE,		// all one-on-one games (all rule types)
  #if 1	//20000725MB: do not enable until MAX_CLIENT_DISPLAY_TABS is increased past 6.
		// Be sure to check all static arrays that use MAX_CLIENT_DISPLAY_TABS and
		// make sure they have enough elements.
	DISPLAY_TAB_TOURNAMENT,		// all tournaments (all rule types)
  #endif
};

WORD32 dwBroadcastDestPlayerID;
BOOL CALLBACK dlgFuncRequestHandHistory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK dlgDisableCashierFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#if ADMIN_CLIENT
BOOL CALLBACK dlgFuncEnterBroadcastMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgFuncChangeShotClock(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgFuncChangeAllIns(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgFuncChatMonitor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgAdminInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgDownLoadClient(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

static struct ChatEntry AdminChatMonitor[ADMIN_CHAT_LINES];
#endif

static struct ProximityHighlightingInfo CardRoomHighlightList;

// Table list column definitions:
enum TableListColumn {
	TLC_TABLE_NAME,
	TLC_GAME_RULES,
	TLC_STAKES,
	TLC_PLAYER_COUNT,
	TLC_WAIT_COUNT,
	TLC_HANDS_PER_HOUR,
	TLC_PLAYERS_PER_FLOP,
	TLC_PLAYERS_PER_4TH,
	TLC_AVERAGE_POT,
	TLC_BUY_IN,
	TLC_POOL,
	TLC_STATUS,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	TLC_RAKE_PER_HOUR,
  #endif

	MAX_TABLE_LIST_COLUMNS
};

static char *TableListColumnHeadings[MAX_TABLE_LIST_COLUMNS] = {
	"Table",		// TLC_TABLE_NAME,
	"Game",			// TLC_GAME_RULES,
	"Stakes",		// TLC_STAKES,
	"Plrs",			// TLC_PLAYER_COUNT,
	"Wait",			// TLC_WAIT_COUNT,
	"H/hr",			// TLC_HANDS_PER_HOUR,
	"Plrs/Flop",	// TLC_PLAYERS_PER_FLOP,
	"Plrs/4th",		// TLC_PLAYERS_PER_4TH,
	"Avg Pot",		// TLC_AVERAGE_POT,
	"Buy-In",		// TLC_BUY_IN,
	"Pool",			// TLC_POOL,
	"Status",		// TLC_STATUS,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	"$/hr",			// TLC_RAKE_PER_HOUR,
  #endif
};

static int TableListColumnWidthPercentages[MAX_TABLE_LIST_COLUMNS] = {
	21,	//	"Table",		// TLC_TABLE_NAME,
	18,	//	"Game",			// TLC_GAME_RULES,
	15,	//	"Stakes",		// TLC_STAKES,
	9,	//	"Plrs",			// TLC_PLAYER_COUNT,
	11,	//	"Wait",			// TLC_WAIT_COUNT,
	10,	//	"H/hr",			// TLC_HANDS_PER_HOUR,
	14,	//	"Plrs/Flop",	// TLC_PLAYERS_PER_FLOP,
	14,	//	"Plrs/4th",		// TLC_PLAYERS_PER_4TH,
	13,	//	"Avg Pot",		// TLC_AVERAGE_POT,
	14,	//	"Buy-In",		// TLC_BUY_IN,
	9,	//	"Pool",			// TLC_POOL,
	23,	//	"Status",		// TLC_STATUS,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	10,	//	"$/hr",			// TLC_RAKE_PER_HOUR,
  #endif
};

// Profile's for which tabs contain which columns...
static int HoldemColumnList[] = {
	TLC_TABLE_NAME,
	TLC_STAKES,
	TLC_PLAYER_COUNT,
	TLC_AVERAGE_POT,
	TLC_PLAYERS_PER_FLOP,
	TLC_WAIT_COUNT,
	TLC_HANDS_PER_HOUR,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	TLC_RAKE_PER_HOUR,
  #endif
	-1	// end of list.	
};

static int StudColumnList[] = {
	TLC_TABLE_NAME,
	TLC_STAKES,
	TLC_PLAYER_COUNT,
	TLC_AVERAGE_POT,
	TLC_PLAYERS_PER_4TH,
	TLC_WAIT_COUNT,
	TLC_HANDS_PER_HOUR,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	TLC_RAKE_PER_HOUR,
  #endif
	-1	// end of list.	
};

static int OneOnOneColumnList[] = {
	TLC_TABLE_NAME,
	TLC_GAME_RULES,
	TLC_STAKES,
	TLC_PLAYER_COUNT,
	TLC_WAIT_COUNT,
  #if INCLUDE_TABLE_RAKE_PER_HOUR
	TLC_RAKE_PER_HOUR,
  #endif
	-1	// end of list.	
};

static int TournamentColumnList[] = {
	TLC_TABLE_NAME,
	TLC_GAME_RULES,
	TLC_BUY_IN,
	TLC_PLAYER_COUNT,
	TLC_WAIT_COUNT,
	TLC_STATUS,
  #if INCLUDE_TABLE_RAKE_PER_HOUR && 0
	TLC_RAKE_PER_HOUR,
  #endif
	-1	// end of list.	
};

static int *TabColumnProfiles[ACTUAL_CLIENT_DISPLAY_TABS] = {
	HoldemColumnList,	// DISPLAY_TAB_HOLDEM
	HoldemColumnList,	// DISPLAY_TAB_OMAHA_HI,
	StudColumnList,		// DISPLAY_TAB_STUD7,
	OneOnOneColumnList,		// DISPLAY_TAB_ONE_ON_ONE,		// all one-on-one games (all rule types)
	HoldemColumnList,	// DISPLAY_TAB_OMAHA_HI_LO,
	StudColumnList,		// DISPLAY_TAB_STUD7_HI_LO,
  #if 1	//20000725MB: do not enable until MAX_CLIENT_DISPLAY_TABS is increased past 6.
		// Be sure to check all static arrays that use MAX_CLIENT_DISPLAY_TABS and
		// make sure they have enough elements.
	TournamentColumnList, // DISPLAY_TAB_TOURNAMENTS,	// all tournaments (all rule types)
  #endif
};

static int iCurrentColumnCount;
static int CurrentColumnList[MAX_TABLE_LIST_COLUMNS];	// TLC_* list for current tab
static int CurrentColumnIndex[MAX_TABLE_LIST_COLUMNS];	// map TLC_* to the column index
int iGameListSortKey=TLC_GAME_RULES;	// current column index they want to sort by (default to stakes)
int iGameListSortDir;					// 0=ascending, 1=descending

//*********************************************************
//
// Update any menu items we're blinking on this screen
//
static void UpdateMenuBlinks(void)
{
	if (iMenuBlinkCount) {	// any blinking left to do?
		iMenuBlinkCount--;
		HiliteMenuItem(hCardRoomDlg, GetMenu(hCardRoomDlg), 0,
			 MF_BYPOSITION | ((iMenuBlinkCount&1)?MF_HILITE:MF_UNHILITE));
	}
}

//*********************************************************
//
// Add/Remove any menu items for the various open tables
//
void UpdateTableMenuItems(void)
{
	// First, remove them all, then add any for open table windows.
	HMENU hm = GetMenu(hCardRoomDlg);	// get HMENU for menu bar
	hm = GetSubMenu(hm, 0);	// get first menu off menu bar
	if (hm) {
		for (int i=0 ; i<MAX_TABLES ; i++) {
			DeleteMenu(hm, iGotoTableMenuIDs[i], MF_BYCOMMAND);
		}
		int insert_position = 11;	// put starting at index 10 down from the top.
		int inserted_flag = FALSE;
		MENUITEMINFO mii;
		zstruct(mii);
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		for (i=0 ; i<MAX_TABLES ; i++) {
			if (Table[i].hwnd) {
				mii.wID = (WORD)iGotoTableMenuIDs[i];
				char str[100];
				sprintf(str, "&%d Table %s", i+1, TableNameFromSerialNumber(Table[i].table_serial_number));
				mii.dwTypeData = str;
				InsertMenuItem(hm, insert_position, MF_BYPOSITION, &mii);
				inserted_flag = TRUE;
				insert_position++;
			}
		}
		if (!inserted_flag) {
			// Nothing added... add '(no open table windows)' as a greyed out item
			mii.wID = (WORD)iGotoTableMenuIDs[0];
			mii.dwTypeData = "(no open table windows)";;
			mii.fState = MFS_GRAYED;
			InsertMenuItem(hm, insert_position, MF_BYPOSITION, &mii);
		}
	}
}

//*********************************************************
//
// Convert seconds_to_go to a string for the shot clock
//
void ConvertSecondsToString(int seconds_to_go, char *dest_str, int display_seconds_flag, int display_short_units_flag, int display_field_count)
{
	if (!display_seconds_flag || seconds_to_go >= 120) {
		seconds_to_go += 59;	// round up to next highest minute.
		display_seconds_flag = FALSE;
	}
	int days = seconds_to_go / (24*3600);
	seconds_to_go -= days*24*3600;
	int hours = seconds_to_go / 3600;
	seconds_to_go -= hours * 3600;
	int minutes = seconds_to_go / 60;
	seconds_to_go -= minutes * 60;
	dest_str[0] = 0;
	if (days) {
		if (display_short_units_flag) {
			sprintf(dest_str+strlen(dest_str), "%dd ", days);
		} else {
			sprintf(dest_str+strlen(dest_str), "%d day%s ", days, days==1 ? "" : "s");
		}
	}
	if (days || hours) {
		if (display_short_units_flag) {
			sprintf(dest_str+strlen(dest_str), "%dh ", hours);
		} else {
			sprintf(dest_str+strlen(dest_str), "%d hour%s ", hours, hours==1 ? "" : "s");
		}
	}
	if (!display_seconds_flag || minutes) {
		if (display_short_units_flag) {
			sprintf(dest_str+strlen(dest_str), "%dm", minutes);
		} else {
			sprintf(dest_str+strlen(dest_str), "%d minute%s", minutes, minutes==1 ? "" : "s");
		}
		if (display_seconds_flag) {
			strcat(dest_str, " ");
		}
	}
	if (display_seconds_flag) {
		if (display_short_units_flag) {
			sprintf(dest_str+strlen(dest_str), "%ds", seconds_to_go);
		} else {
			sprintf(dest_str+strlen(dest_str), "%d second%s", seconds_to_go, seconds_to_go==1 ? "" : "s");
		}
	}
	NOTUSED(display_field_count);
}

//*********************************************************
//
// Update the text in the shot clock windows base on
// ShotClockLocalDate and ShotClockMsg
//
void UpdateShotClockText(void)
{
	static DWORD last_test_time;
	DWORD ticks = GetTickCount();
	if (ticks < last_test_time + 500) {	// not too often...
		return;
	}
	last_test_time = ticks;

	char alternate_source_string[SHOTCLOCK_MESSAGE_LEN+40];
	zstruct(alternate_source_string);
	char temp_str[SHOTCLOCK_MESSAGE_LEN+40];
	zstruct(temp_str);

	long seconds_to_go = 0;
	if (ShotClockLocalDate) {
		seconds_to_go = ShotClockLocalDate - time(NULL);
		if (seconds_to_go < 0) {
			seconds_to_go = 0;
		}
	}

  #if ADMIN_CLIENT	//20001006MB
	// Play shot clock alarm sounds if necessary
	if (iAdminClientFlag) {
		static long old_seconds_to_go;
		long prev_old = old_seconds_to_go;
		old_seconds_to_go = seconds_to_go;
		if (Defaults.iShotClockAlarm && ShotClockServerDate) {
			if (prev_old > 60 && prev_old < 100 && seconds_to_go <= 60 && seconds_to_go > 30) {
				PPlaySound(SOUND_SHOTCLOCK_60S);
			}
			if (prev_old > 0 && prev_old < 60 && seconds_to_go==0) {
				pr(("%s %s(%d) prev_old = %d, seconds_to_go = %d, pid = %d\n",
						TimeStr(), _FL, prev_old, seconds_to_go, getpid()));
				PPlaySound(SOUND_SHOTCLOCK_EXPIRED);
			}
		}
	}
  #endif

	for (int i=0 ; i<2 ; i++) {
		char *src = i ? ShotClockMsg2 : ShotClockMsg1;
		int item = i ? IDC_SHOT_CLOCK_ETA : IDC_SHOT_CLOCK_MESSAGE;
		int display_it = TRUE;

		//20001214MB: Replace any &'s with &&
		src = DelimitStringForTextControl(src, alternate_source_string, sizeof(alternate_source_string));
        
        

		// Parse any % signs in the source string.
		char *t = NULL;
		forever {
			// Does the source string contain %t for encoding the time to go?
			t = strstr(src, "%t");
			if (t) {
				// Yes, the string contains %t.  Only display it if the
				// shot clock has not yet expired.
				if (seconds_to_go <= 0) {
					display_it = FALSE;
					break;
				} else {
					// replace %t? with the time to go string.
					strnncpy(temp_str, src, t-src+1);
					char *t2 = t+2;
					int display_fields = 9;	// default to max
					if (isdigit(t[2])) {
						display_fields = t[2]-'0';
						t2++;
					}
					ConvertSecondsToString(seconds_to_go,
							temp_str + strlen(temp_str),
							ShotClockFlags & SCUF_DISPLAY_SECONDS,
							ShotClockFlags & SCUF_USE_SHORT_UNITS,
							display_fields);
					strcat(temp_str, t2);	// add remainder of source message
					strnncpy(alternate_source_string, temp_str, sizeof(alternate_source_string));
					src = alternate_source_string;
				}
				continue;
			}

			// Does the source string contain %c? for encoding the clock?
			t = strstr(src, "%c");
			if (t) {
				// Yes, the string contains %c.
				// replace %c with the time string.
				strnncpy(temp_str, src, t-src+1);
				char *t2 = t+2;
				int time_zone = 0;	// default to GMT
				if (isdigit(t[2])) {
					time_zone = t[2]-'0';
					t2++;
				}
				time_t now = time(NULL) - time_zone * 3600;
				struct tm tm;
				zstruct(tm);
				gmtime(&now, &tm);
				char *am_pm_string = NULL;
				int hour = tm.tm_hour;
				if (hour < 0) {
					hour += 24;
				}
				if (ShotClockFlags & SCUF_24HOUR_CLOCK) {	// display 24 hour clock
					sprintf(temp_str + strlen(temp_str), "%02d:%02d",
							hour, tm.tm_min);
				} else {	// display am/pm
					am_pm_string = hour >= 12 ? "pm" : "am";
					hour = hour % 12;
					if (!hour) {
						hour = 12;
					}
					sprintf(temp_str + strlen(temp_str), "%d:%02d",
							hour, tm.tm_min);
				}
				if (ShotClockFlags & SCUF_DISPLAY_SECONDS) {
					sprintf(temp_str + strlen(temp_str), ":%02d",
							tm.tm_sec);
				}
				if (am_pm_string) {
					strcat(temp_str, am_pm_string);
				}
				strcat(temp_str, t2);	// add remainder of source message
				strnncpy(alternate_source_string, temp_str, sizeof(alternate_source_string));
				src = alternate_source_string;
				continue;
			}
			// Does the source string contain %d? for encoding the date?
			t = strstr(src, "%d");
			if (t) {
				// Yes, the string contains %d.
				// replace %d with the date string (for today's date)
				strnncpy(temp_str, src, t-src+1);
				char *t2 = t+2;
				int time_zone = 0;	// default to GMT
				if (isdigit(t[2])) {
					time_zone = t[2]-'0';
					t2++;
				}
				time_t now = time(NULL) - time_zone * 3600;
				struct tm tm;
				zstruct(tm);
				gmtime(&now, &tm);
				char *month_names[12] =  {
					"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"
				};
				sprintf(temp_str + strlen(temp_str), "%s %d",
							month_names[tm.tm_mon], tm.tm_mday);
				if (ShotClockFlags & SCUF_DISPLAY_YEAR) {
					sprintf(temp_str + strlen(temp_str), ", %d",
							tm.tm_year + 1900);
				}
				strcat(temp_str, t2);	// add remainder of source message
				strnncpy(alternate_source_string, temp_str, sizeof(alternate_source_string));
				src = alternate_source_string;
				continue;
			}
			break;	// nothing found.  done looping.
		}
		if (display_it) {
			SetWindowTextIfNecessary(GetDlgItem(hCardRoomDlg, item), src);
			ShowWindow(GetDlgItem(hCardRoomDlg, item), SW_SHOW);
		} else {
			ShowWindow(GetDlgItem(hCardRoomDlg, item), SW_HIDE);
		}
	}
}

//*********************************************************
//
// Update the shot clock info received from the server
//
void UpdateShotClock(void)
{
	//kp(("%s(%d) UpdateShotClock()\n",_FL));
	// Calculate a CardRoom_TableSummaryList pointer from the packet.  Remember to skip
	// over all the headers.
	Packet *p = TableSummaryListPackets[GameListTabIndex];
	if (!p) {
		return;	// nothing to do.
	}
	struct CardRoom_TableSummaryList *ts = (struct CardRoom_TableSummaryList *)
				((char *)(p->user_data_ptr)+sizeof(struct DataPacketHeader));

	if (ts->shotclock_date) {	// the shot clock SHOULD be displayed...
		if (ts->shotclock_date==(WORD32)ShotClockServerDate) {	// same date?
			// Date is the same... take the minimum ETA (in case there were transport delays)
			time_t new_local_date = time(NULL) + ts->shotclock_eta;
			ShotClockLocalDate = min(ShotClockLocalDate, new_local_date);
		} else {	// date changed... always recalc local date for eta
			ShotClockLocalDate = time(NULL) + ts->shotclock_eta;
			ShotClockServerDate = ts->shotclock_date;
		}
		// Now we've got ShotClockLocalDate and ShotClockServerDate set.  Grab the msg
		strnncpy(ShotClockMsg1, ts->shotclock_msg1, SHOTCLOCK_MESSAGE_LEN);
		strnncpy(ShotClockMsg2, ts->shotclock_msg2, SHOTCLOCK_MESSAGE_LEN);
		ShotClockFlags = ts->shotclock_flags;
	} else {
		// The shot clock should NOT be displayed
		ShotClockServerDate = ShotClockLocalDate = 0;
		ShotClockMsg1[0] = 0;
		ShotClockMsg2[0] = 0;
		ShotClockFlags = 0;
	}
	// Update the text in the window and show the windows.
	iCCProcessingEstimate = ts->cc_processing_estimate;	// save most recent estimate (in minutes)
	iMaxTournamentTables = ts->max_tournament_tables;	// save max tournament table count
	UpdateShotClockText();
}	

//*********************************************************
//
// Flag the cardroom window needs redrawing and post a message
// to its message queue if necessary.
//
void FlagRedrawCardRoom(void)
{
	if (!iCardRoomRedrawNeeded) {
		// First time it's getting set... post a message to
		// the message queue for the dialog box to update itself
		// the first time it gets a chance.
		iCardRoomRedrawNeeded = TRUE;
		PostMessage(hCardRoomDlg, WMP_UPDATE_YOURSELF, 0,0);
	}
}

#if INCLUDE_TABLE_RAKE_PER_HOUR
//*********************************************************
//
// Set the current mode for the rake/hour column
// 0 = rake / hr / table
// 1 = rake / hr / plr
//
int iRakePerHourType;	// 0=rake / hr / table, 1=rake/hr/plr
void ChangeRakePerHourColumn(int new_method)
{
	iRakePerHourType = new_method;
    //kp(("%s(%d) Changing column %d to type %d\n", _FL, CurrentColumnIndex[TLC_RAKE_PER_HOUR], iRakePerHourType));
  #if 0	//20000731MB: never worked; didn't bother to figure out why
	ListView_SetItemText(hGameListCtrl,
			CurrentColumnIndex[TLC_RAKE_PER_HOUR], 0,
            iRakePerHourType ? "$/plr" : TableListColumnHeadings[TLC_RAKE_PER_HOUR]);
  #else
    TableListColumnHeadings[TLC_RAKE_PER_HOUR] = iRakePerHourType ? "$/plr" : "$/hr";
  #endif
}
#endif

#if ADMIN_CLIENT
//*********************************************************
//
// Set the current mode for the hands/hour column
// 0 = hands/hr (normal)
// 1 = average player response time (in tenths of seconds)
//
int iHandsPerHourType;	// 0=hands/hr, 1=avg response time
void ChangeHandsPerHourColumn(int new_method)
{
	iHandsPerHourType = new_method;
    TableListColumnHeadings[TLC_HANDS_PER_HOUR] = iHandsPerHourType ? "avg" : "H/hr";
}
#endif

//*********************************************************
//
// Callback for sorting the table list view
//
int CALLBACK TableListViewCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int result = 0;
	struct CardRoom_TableSummaryInfo tsi1, tsi2;
	zstruct(tsi1);
	zstruct(tsi2);
	FindTableSummaryInfo(lParam1, &tsi1);
	FindTableSummaryInfo(lParam2, &tsi2);

	// Define the sort order to use after our primary sort key.
	static int sort_order[] =  {
		100, 		// always play money/real money first
		0,			// user-defined top level criteria
		// Tiebreaker ordering...
		TLC_GAME_RULES,
		TLC_STAKES,
		TLC_STATUS,
		TLC_PLAYER_COUNT,
		TLC_WAIT_COUNT,
		TLC_AVERAGE_POT,
		TLC_HANDS_PER_HOUR,
		TLC_TABLE_NAME,
		TLC_PLAYERS_PER_FLOP,
		-1
	};
	sort_order[1] = iGameListSortKey;	// 2nd key is what they requested.
	int sort_key_index = 0;

	do {
		result = 0;
		int sort_key = sort_order[sort_key_index++];
		if (sort_key < 0) {
			break;	// all done.
		}
		switch (sort_key) {
		case TLC_TABLE_NAME:
			result = strcmp(tsi1.table_name, tsi2.table_name);
			break;
		case TLC_STAKES:
			result = (tsi2.flags & TSIF_REAL_MONEY) - (tsi1.flags & TSIF_REAL_MONEY);
			if (!result) {
				result = (int)tsi2.big_blind_amount*GameStakesMultipliers[tsi2.game_rules - GAME_RULES_START] -
						 (int)tsi1.big_blind_amount*GameStakesMultipliers[tsi1.game_rules - GAME_RULES_START];
			}
			break;
		case TLC_BUY_IN:
			result = (tsi2.flags & TSIF_REAL_MONEY) - (tsi1.flags & TSIF_REAL_MONEY);
			if (!result) {
				result = (int)tsi2.big_blind_amount*GameStakesMultipliers[tsi2.game_rules - GAME_RULES_START] -
						 (int)tsi1.big_blind_amount*GameStakesMultipliers[tsi1.game_rules - GAME_RULES_START];
			}
			break;
		case TLC_STATUS:
		  #if 0	//20000922MB
			// Sort in this order: playing, finished, waiting.
			{
				int s1 = tsi1.tournament_state;
				int s2 = tsi2.tournament_state;
				if (s1==TOURN_STATE_WAITING) {
					s1 = TOURN_STATE_FINISHED+1;
				}
				if (s2==TOURN_STATE_WAITING) {
					s2 = TOURN_STATE_FINISHED+1;
				}
				if (s1==s2) {	// same state?
					// sort by level
					kp1(("%s(%d) sorting by tournament_hand_number could be backwards... it's untested until the new server goes online\n", _FL));
					result = tsi2.tournament_hand_number - tsi1.tournament_hand_number;
				} else {
					result = s1 - s2;
				}
			}
		  #else
			// Sort in this order: finished, playing, waiting.
			result = (int)tsi2.tournament_state - (int)tsi1.tournament_state;
			if (!result && tsi2.tournament_state==TOURN_STATE_PLAYING) {
				result = (int)tsi1.player_count - (int)tsi2.player_count;
			}
			if (!result) {
				// sort by level (highest level on top)
				result = tsi2.tournament_hand_number - tsi1.tournament_hand_number;
			}
		  #endif
			break;
		case TLC_GAME_RULES:
			result = (int)tsi1.game_rules - (int)tsi2.game_rules;
			break;
		case TLC_PLAYER_COUNT:
			result = (int)tsi2.player_count - (int)tsi1.player_count;
		  #if ADMIN_CLIENT
			if (iAdminClientFlag) {
				if (!result) {
					result = (int)tsi2.watching_count - (int)tsi1.watching_count;
				}
			}
		  #endif
			if ((tsi1.flags & TSIF_TOURNAMENT) && tsi1.tournament_state==TOURN_STATE_PLAYING) {
				// sort playing tournament games backwards by player count
				result = -result;
			}
			break;
		case TLC_AVERAGE_POT:
			result = (int)tsi2.avg_pot_size - (int)tsi1.avg_pot_size;
			break;
		case TLC_PLAYERS_PER_FLOP:
		case TLC_PLAYERS_PER_4TH:
			result = (int)tsi1.players_per_flop - (int)tsi2.players_per_flop;
			break;
		case TLC_WAIT_COUNT:
			{
				// Something without a waiting list is always better than something
				// with a waiting list (even if we're first on the list).
				result = (int)((tsi1.flags & TSIF_WAIT_LIST_REQUIRED) - (tsi2.flags & TSIF_WAIT_LIST_REQUIRED));
				if (!result) {
					result = (int)tsi1.waiting_list_length - (int)tsi2.waiting_list_length;
					if (tsi1.waiting_list_length && tsi2.waiting_list_length) {
						// Both have waiting lists... pick the one we're closest
						// to the front of the line on.
						int wp1 = tsi1.user_waiting_list_pos ? (int)tsi1.user_waiting_list_pos : tsi1.waiting_list_length;
						int wp2 = tsi2.user_waiting_list_pos ? (int)tsi2.user_waiting_list_pos : tsi2.waiting_list_length;
						result = wp1 - wp2;
					}
				}
			}
			break;
		case TLC_HANDS_PER_HOUR:
		  #if ADMIN_CLIENT
			if (iHandsPerHourType) {
				result = (int)tsi2.avg_response_time - (int)tsi1.avg_response_time;
				break;
			}
		  #endif
			result = (int)tsi2.hands_per_hour - (int)tsi1.hands_per_hour;
			break;
	  #if INCLUDE_TABLE_RAKE_PER_HOUR
		case TLC_RAKE_PER_HOUR:
            if (iRakePerHourType) {
                // rake/hr/plr
                int c1 = max(1, tsi1.player_count);
                int c2 = max(1, tsi2.player_count);
                int r1 = (int)tsi1.rake_per_hour / c1;
                int r2 = (int)tsi2.rake_per_hour / c2;
                result = r2 - r1;
            } else {
    			result = (int)tsi2.rake_per_hour - (int)tsi1.rake_per_hour;
            }
			break;
	  #endif
		case 100:	// real money first, play money second
			result = (int)(tsi2.flags & TSIF_REAL_MONEY) - (int)(tsi1.flags & TSIF_REAL_MONEY);
			break;
		}
		// Step up to the next sort key.
	} while (result==0);

	if (iGameListSortDir) {	// invert order?
		result = -result;
	}
	NOTUSED(lParamSort);
	return result;
}

//*********************************************************
//
// Sort the table list view control
//
void SortTableList(void)
{
	ListView_SortItems(hGameListCtrl, TableListViewCompareFunc, 0);
}

//*********************************************************
//
// Find a table given its serial number and fill in a COPY of
// its CardRoom_TableSummaryInfo structure.
// Returns an error if it cannot find it.
//
ErrorType FindTableSummaryInfo(WORD32 table_serial_number, struct CardRoom_TableSummaryInfo *output_table_summary_info)
{
	// We must check each game type...
	EnterCriticalSection(&CardRoomVarsCritSec);
	for (int client_display_tab_index=0 ; client_display_tab_index<ACTUAL_CLIENT_DISPLAY_TABS ; client_display_tab_index++) {
		if (TableSummaryListPackets[client_display_tab_index]) {
			struct CardRoom_TableSummaryList *ts = (struct CardRoom_TableSummaryList *)
						((char *)(TableSummaryListPackets[client_display_tab_index])->user_data_ptr+sizeof(struct DataPacketHeader));
			for (int i=0 ; i<(int)ts->table_count ; i++) {
				struct CardRoom_TableSummaryInfo *t = &ts->tables[i];
				// Does this table match?
				if (t->table_serial_number==table_serial_number) {
					// Found it.
					*output_table_summary_info = *t;
					//19991126MB: compensate for the server sometimes not giving us
					// the full length of the waiting list right away.
					output_table_summary_info->waiting_list_length =
							max(output_table_summary_info->waiting_list_length, output_table_summary_info->user_waiting_list_pos);

					LeaveCriticalSection(&CardRoomVarsCritSec);
					return ERR_NONE;
				}
			}
		}
	}
	LeaveCriticalSection(&CardRoomVarsCritSec);
	// Not found....
	zstruct(*output_table_summary_info);
	return ERR_ERROR;	// not found.
}

//*********************************************************
//
// Find a table given its serial number and return its name
// as a string.  Never EVER returns NULL.  Always returns
// something (even if it's just "unknown").
// Note: there's only one copy of the resulting string, therefore
// if this function is to be called from more than one thread,
// this code will need to be changed.
//
char *TableNameFromSerialNumber(WORD32 table_serial_number)
{
	static struct CardRoom_TableSummaryInfo tsi;
	ErrorType err = FindTableSummaryInfo(table_serial_number, &tsi);
	if (err) {
		// Table could not be found.
		return "unknown";
	}
	return tsi.table_name;
}

#if ADMIN_CLIENT
//*********************************************************
//
// Save any new shot clock messages.  Keep the last bunch seen.
//
#define SHOTCLOCK_MESSAGES_TO_KEEP	20
char SavedShotClockMessages[4][SHOTCLOCK_MESSAGES_TO_KEEP][SHOTCLOCK_MESSAGE_LEN];

void SaveShotClockMessages( char *shot_clock_msg1,
							char *shot_clock_msg2,
							char *shot_clock_msg1_expired,
							char *shot_clock_msg2_expired)
{
	// Check if it already exists...
	for (int line=0 ; line<4 ; line++) {
		char *msg = NULL;
		switch (line) {
		case 0:
			msg = shot_clock_msg1;
			break;
		case 1:
			msg = shot_clock_msg2;
			break;
		case 2:
			msg = shot_clock_msg1_expired;
			break;
		case 3:
			msg = shot_clock_msg2_expired;
			break;
		}
		for (int i=0 ; i<SHOTCLOCK_MESSAGES_TO_KEEP ; i++) {
			if (!stricmp(msg, SavedShotClockMessages[line][i])) {
				// Found it... remove it.
				if (i<SHOTCLOCK_MESSAGES_TO_KEEP-1) {
					// Scroll others down to fill the gap.
					int items_left = SHOTCLOCK_MESSAGES_TO_KEEP - i - 1;
					memmove(SavedShotClockMessages[line][i], SavedShotClockMessages[line][i+1], SHOTCLOCK_MESSAGE_LEN*items_left);
				}
				// Zero the last one.
				memset(SavedShotClockMessages[line][SHOTCLOCK_MESSAGES_TO_KEEP-1], 0, SHOTCLOCK_MESSAGE_LEN);
			}
		}
		// Add it to the top of the list. Scroll down to make room.
		memmove(SavedShotClockMessages[line][1],	// dest
				SavedShotClockMessages[line][0],	// src
				SHOTCLOCK_MESSAGE_LEN*(SHOTCLOCK_MESSAGES_TO_KEEP-1));
		// Now save the new one in the first slot.
		strnncpy(SavedShotClockMessages[line][0], msg, SHOTCLOCK_MESSAGE_LEN);
	}
}

#endif	// ADMIN_CLIENT

//****************************************************************
//
// Select a default table for this user.
// Search criteria:
// 1. Choose same game type as user selected last time.
// 2. Choose same game stakes as user selected last time.
// 3. Choose a table with at least 2 empty seats.
// 4. Choose the table with the most players.
// 5. If that table is not playing, try looking for one with
//    only one empty seat.
//
WORD32 SelectDefaultTable(ClientDisplayTabIndex client_display_tab_index)
{
	return SelectDefaultTable(client_display_tab_index, FALSE);
}

WORD32 SelectDefaultTable(ClientDisplayTabIndex client_display_tab_index, int force_stakes_match)
{
	WORD32 best_table = 0;

	int preferred_stakes = Defaults.preferred_stakes;

	EnterCriticalSection(&CardRoomVarsCritSec);
	if (!TableSummaryListPackets[client_display_tab_index]) {
		LeaveCriticalSection(&CardRoomVarsCritSec);
		return best_table;	// no table list available yet.
	}
	struct CardRoom_TableSummaryList *ts = (struct CardRoom_TableSummaryList *)
				((char *)(TableSummaryListPackets[client_display_tab_index])->user_data_ptr+sizeof(struct DataPacketHeader));

	pr(("%s(%d) Selecting a suitable default table. client_display_tab_index=%d, stakes=%d\n",_FL,client_display_tab_index,preferred_stakes));

	struct CardRoom_TableSummaryInfo *bt = NULL;	// best table
	for (int i=0 ; i<(int)ts->table_count ; i++) {
		struct CardRoom_TableSummaryInfo *t = &ts->tables[i];
		// Does this table have room?
		if (t->player_count >= t->max_player_count) {
			// this table is full.  Not suitable.
			//kp(("%s(%d) SelectDefaultTable: Table %d is full (%d/%d players).\n", _FL, t->table_serial_number, t->player_count, t->max_player_count));
			continue;
		}
	  #if 0	//19990517MB
		// Is this table playing?
		if (t->player_count < t->max_player_count/3) {
			// not enough people
			continue;
		}
	  #endif
		if (bt && bt->big_blind_amount==preferred_stakes) {
			// We've already got one with the right stakes... anything
			// new must also be the correct stakes.
			if (t->big_blind_amount==preferred_stakes) {	// correct stakes?
				// both are same stakes... pick table with
				// more players and room for one more.
				if (t->player_count > bt->player_count &&
					t->player_count < t->max_player_count) {
					bt = t;
				}
			}
		} else {
			// We don't have anything with the right stakes yet.
			if (t->big_blind_amount==preferred_stakes) {	// correct stakes?
				// correct stakes... pick this one.
				bt = t;
			} else if (!force_stakes_match) {
				// both are wrong stakes, pick whichever has more players
				// and room for one more.
				if (!bt || (t->player_count > bt->player_count &&
							t->player_count < t->max_player_count)) {
					bt = t;
				}
			}
		}
	}
	if (!bt && ts->table_count) {
		bt = &ts->tables[0];
	}
	if (bt) {
		best_table = bt->table_serial_number;
		pr(("%s(%d) SelectDefaultTable: best table = %d, # of players = %d\n",
					_FL, bt->table_serial_number, bt->player_count));
	}
	LeaveCriticalSection(&CardRoomVarsCritSec);
	return best_table;
}

//****************************************************************
//
// Update the game view list control to reflect the most recent
// data received from the server.
// Does not create the list view control, it only updates an existing one.
//
void UpdateGameListViewItems(void)
{
	if (!hGameListCtrl || iCardRoomDlgMinimizedFlag) {
		return;	// nothing to do... list view control does not exist or is not visible.
	}
	pr(("%s(%d) UpdateGameListViewItems() has been called.\n",_FL));
	EnterCriticalSection(&CardRoomVarsCritSec);
	if (!TableSummaryListPackets[GameListTabIndex]) {
		// no data for this game type.  Empty the list.
		//kp(("%s(%d) No data for game type %d... emptying list.\n", _FL, GameListTabIndex));
		// If there was a previous list view control, destroy it.
		ListView_DeleteAllItems(hGameListCtrl);
		LeaveCriticalSection(&CardRoomVarsCritSec);
		return;
	}

	// Calculate a CardRoom_TableSummaryList pointer from the packet.  Remember to skip
	// over all the headers.
	struct CardRoom_TableSummaryList *ts = (struct CardRoom_TableSummaryList *)
				((char *)(TableSummaryListPackets[GameListTabIndex])->user_data_ptr+sizeof(struct DataPacketHeader));

	// Keep track of our current scroll index.
	int previous_top_index = ListView_GetTopIndex(hGameListCtrl);

  #if INCLUDE_TABLE_RAKE_PER_HOUR
	ChangeRakePerHourColumn(iRakePerHourType);
  #endif
  #if ADMIN_CLIENT
	ChangeHandsPerHourColumn(iHandsPerHourType);
  #endif
	// Add items to the list view control...
	LVITEM lvi;
	zstruct(lvi);
	lvi.mask = LVIF_TEXT|LVIF_PARAM;
  #if 0	//20000731MB
	int on_a_waiting_list = FALSE;		// not joined waiting lists yet
	int found_playing_table = FALSE;	// no playing tables found yet.
	int lowest_stakes = 1000000;
	int lowest_stakes_serial_number = 0;
  #endif
	if (ts->table_count==0) {
		ListView_DeleteAllItems(hGameListCtrl);
		ListView_SetItemCount(hGameListCtrl, 1);
		if (GameListTabIndex==DISPLAY_TAB_TOURNAMENT) {
			// lvi.pszText = "Not open";
			lvi.pszText = "Closed";
		} else {
			lvi.pszText = "No tables";
		}
		ListView_InsertItem(hGameListCtrl, &lvi);
	} else {
		// The TRY_TO_PRESERVE_ITEMS code code does not presently work
		// because tables are sorted as we add them and therefore the
		// item numbers don't come back in order.  Since I wasn't sure
		// how to work around those problems, I've left this project
		// for a later date.
		#define TRY_TO_PRESERVE_ITEMS	0
	  #if TRY_TO_PRESERVE_ITEMS
		int previous_item_count = ListView_GetItemCount(hGameListCtrl);
	  #else
		ListView_DeleteAllItems(hGameListCtrl);
	  #endif
		ListView_SetItemCount(hGameListCtrl, ts->table_count);
		for (int i=0 ; i<(int)ts->table_count ; i++) {
			struct CardRoom_TableSummaryInfo *ti = &ts->tables[i];
			if (Defaults.iHidePlayMoneyTables &&
				!(ti->flags & TSIF_REAL_MONEY) &&
				!(ti->flags & TSIF_TOURNAMENT))
			{
				continue;	// hide play money tables.
			}

			ChipType chip_type = CT_NONE;
			if (ti->flags & TSIF_REAL_MONEY) {
				chip_type = CT_REAL;
			} else if (ti->flags & TSIF_TOURNAMENT) {
				chip_type = CT_TOURNAMENT;
			} else {
				chip_type = CT_PLAY;	// assume for now
			}

		  #if 0	//20000731MB
			// If this table is playing, flag that we found a playing table.
			if (ti->player_count >= 2 && chip_type != CT_TOURNAMENT) {	//!!! this is a shortcut for now
				found_playing_table = TRUE;
			}
			if (ti->big_blind_amount < lowest_stakes) {
				lowest_stakes = ti->big_blind_amount;
				lowest_stakes_serial_number = ti->table_serial_number;
			}
		  #endif

			// Set the main item (leftmost column) - always TLC_TABLE_NAME
			lvi.pszText = ti->table_name;
			lvi.lParam  = ti->table_serial_number;
		  #if TRY_TO_PRESERVE_ITEMS	//19990716MB
		  	int index;
			if (i >= previous_item_count) {	// adding new items to the end?
				index = ListView_InsertItem(hGameListCtrl, &lvi);
				kp(("%s(%d) Added new item #%d.  Got back index %d\n", _FL, i, index));
			} else {
				// Updating an existing item
				index = lvi.iItem = i;
				BOOL success = ListView_SetItem(hGameListCtrl, &lvi);
				if (!success) {
					kp(("%s(%d) ListView_SetItem for item %d failed.\n", _FL, i));
				}
			}
		  #else
			int index = ListView_InsertItem(hGameListCtrl, &lvi);
		  #endif
			//kp(("%s(%d) index %d is Table '%s' (%d)\n",_FL, index, ti->table_name, lvi.lParam));
			if (index >= 0) {
				// Loop through and do each column...
				char str[80];
				zstruct(str);

				for (int j=1 ; j<iCurrentColumnCount ; j++) {
					switch (CurrentColumnList[j]) {
					case TLC_STAKES:
						//Aqui se ponen los rangos de las mesas
						if (ti->flags & TSIF_REAL_MONEY) {
							int label_multiplier = GameStakesMultipliers[ti->game_rules - GAME_RULES_START];
							char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];
							sprintf(str, "%s/%s",
									CurrencyString(cs1, label_multiplier*ti->big_blind_amount, CT_REAL),
									CurrencyString(cs2, label_multiplier*ti->big_blind_amount*2, CT_REAL));
						} else {
						//	strcpy(str, "play");
							int label_multiplier = GameStakesMultipliers[ti->game_rules - GAME_RULES_START];
							char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];
							sprintf(str, "%s/%s",
									CurrencyString(cs1, label_multiplier*ti->big_blind_amount, CT_PLAY),
									CurrencyString(cs2, label_multiplier*ti->big_blind_amount*2, CT_PLAY));

						}
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_BUY_IN:
						{
							char cs1[MAX_CURRENCY_STRING_LEN], cs2[MAX_CURRENCY_STRING_LEN];
							if (iLargeFontsMode) {
								sprintf(str, "%s+%s",
										CurrencyString(cs1, ti->big_blind_amount,   CT_REAL),
										CurrencyString(cs2, ti->small_blind_amount, CT_REAL));
							} else {
								sprintf(str, "%s + %s",
										CurrencyString(cs1, ti->big_blind_amount,   CT_REAL),
										CurrencyString(cs2, ti->small_blind_amount, CT_REAL));
							}
						}
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_STATUS:
						switch (ti->tournament_state) {
						case TOURN_STATE_WAITING:
							{
							  #if 1	//20000922MB: we don't care how long the waiting list is; lots of inelibigle people are on it
								int waiting_count = max(0, ti->max_player_count - ti->player_count);
							  #else
								int waiting_count = max(1, ti->max_player_count - ti->player_count - max(ti->waiting_list_length,ti->user_waiting_list_pos));
							  #endif
								pr(("%s(%d) waiting_count = %d, ti->waiting_list_length = %d\n", _FL, waiting_count, ti->waiting_list_length));
								if (iLargeFontsMode) {
		 							sprintf(str, "Waiting for %d", waiting_count);
								} else {
		 							sprintf(str, "Waiting for %d plr%s",
											waiting_count, waiting_count==1 ? "" : "s");
								}
							}
							break;
						case TOURN_STATE_PLAYING:
							if (ti->tournament_hand_number) {
								#define T_MAX_LEVELS		8
								#define T_GAMES_PER_LEVEL	10
								// Calculate the one-based level
								int level = 1+min(T_MAX_LEVELS-1, (ti->tournament_hand_number - 1) / T_GAMES_PER_LEVEL);
								sprintf(str, "Level %s", szRomanNumerals[level]);							
							} else {
								strnncpy(str, "Starting", sizeof(str));
							}
							break;
						case TOURN_STATE_FINISHED:
							strnncpy(str, "Finished", sizeof(str));
							break;
					  #if ADMIN_CLIENT
						default:
							sprintf(str, "TOURN_STATE_%d", ti->tournament_state);
							break;
					  #endif
						}
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_PLAYER_COUNT:
						sprintf(str, "%d", ti->player_count);
					  #if ADMIN_CLIENT
					  	if (iAdminClientFlag && ti->watching_count) {
							sprintf(str, "%d/%d", ti->player_count, ti->watching_count);
					  	}
					  #endif
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_AVERAGE_POT:
						CurrencyString(str, ti->avg_pot_size, chip_type, FALSE, TRUE);
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_PLAYERS_PER_FLOP:
					case TLC_PLAYERS_PER_4TH:
						sprintf(str, "%d%%", ti->players_per_flop);
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_WAIT_COUNT:
						if (ti->user_waiting_list_pos) {
							// We're on the waiting list... show our position.
							if (iLargeFontsMode) {
								sprintf(str, "%dof%d", ti->user_waiting_list_pos, max(ti->waiting_list_length,ti->user_waiting_list_pos));
							} else {
								sprintf(str, "%d of %d", ti->user_waiting_list_pos, max(ti->waiting_list_length,ti->user_waiting_list_pos));
							}
						  #if 0	//20000731MB
							on_a_waiting_list = TRUE;
						  #endif
						} else if (ti->waiting_list_length || (ti->flags & TSIF_WAIT_LIST_REQUIRED)) {
							// Waiting list exists... display length
							sprintf(str, "%d", ti->waiting_list_length);
						} else {
							// No waiting list at all... leave it blank.
							str[0] = 0;
						}
						ListView_SetItemText(hGameListCtrl, index, j, str);
						break;

					case TLC_HANDS_PER_HOUR:
					  #if ADMIN_CLIENT
						if (iHandsPerHourType) {
							if (ti->avg_response_time) {
								sprintf(str, "%.1fs", ti->avg_response_time / 10.0);
							} else {
								str[0] = 0;
							}
							ListView_SetItemText(hGameListCtrl, index, j, str);
							break;
						}
					  #endif
						{
							//int max_hands_per_hour = 120;  
							int max_hands_per_hour=2000; //J Fonseca 120+
							if (ti->hands_per_hour > max_hands_per_hour) {
								sprintf(str, "%d+", max_hands_per_hour);
							} else if (ti->hands_per_hour) {
								sprintf(str, "%d", ti->hands_per_hour);
							} else {
								str[0] = 0;
							}
							ListView_SetItemText(hGameListCtrl, index, j, str);
						}
						break;

					case TLC_GAME_RULES:
						{
							static char *short_rule_names[] = {
								"Hold'em",
								"Omaha Hi",
								"Omaha H/L",
								"7 Stud",
								"7 Stud H/L",
							};
							ListView_SetItemText(hGameListCtrl, index, j, short_rule_names[ti->game_rules - GAME_RULES_START]);
						}
						break;

				  #if INCLUDE_TABLE_RAKE_PER_HOUR
					case TLC_RAKE_PER_HOUR:
						if (iAdminClientFlag && LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {
                            int r = 0;
							if (ti->rake_per_hour) {
                                if (iRakePerHourType) {
                                    int c1 = max(1, ti->player_count);
                                    r = (int)ti->rake_per_hour / c1;
                                    // Treat as play money (no leading $)
    								CurrencyString(str, r, CT_PLAY, TRUE, 0);
                                } else {
    								CurrencyString(str, ti->rake_per_hour, chip_type, FALSE, 1);
                                }
							} else {
								str[0] = 0;
							}
							ListView_SetItemText(hGameListCtrl, index, j, str);
						}
						break;
				  #endif
					default:
						kp(("%s(%d) Warning: unknown column type %d (TLC_*)\n", _FL, CurrentColumnList[j]));
						break;
					}
				}
				if (ti->table_serial_number == dwTableInfoSerialNum) {
					// This is the item that was previously selected.
					// Make this table the current selection again.
					iDisableRequestGameInfo++;	// don't request new copy just because we set the button
					ListView_SetItemState(hGameListCtrl, index,
							LVIS_FOCUSED | LVIS_SELECTED,
							LVIS_FOCUSED | LVIS_SELECTED);
					iDisableRequestGameInfo--;
				}
			}
		}
	}
	SortTableList();

	// Scroll to our old position
	int vert_spacing = ListView_GetItemSpacing(hGameListCtrl, TRUE) >> 16;
	ListView_Scroll(hGameListCtrl, 0, vert_spacing*previous_top_index);

	// Set the # of users connected and active tables
	char str[50];
	int count = max(1, ts->total_players);	// never all zero to be displayed.
	sprintf(str, "%d player%s \n%d active table%s ",
			count, count==1 ? "" : "s",
			ts->active_tables, ts->active_tables==1 ? "" : "s");
  #if ADMIN_CLIENT	
	if (iAdminClientFlag) {
		// Totally different string
		int play_tables = ts->active_tables - ts->active_real_tables;
	  #if 0	//19991223MB
		char rake_rate[30];
		zstruct(rake_rate);
		if (RakeRate) {
			sprintf(rake_rate, "$%d rake/hr. ", RakeRate);
		}
	  #endif
		if ( LoggedInPrivLevel >= 40  ) {     
		sprintf(str, "%d player%s, %d unseated, %d idle \n"
					 "%d real table%s, %d play table%s \n"
					 "%d accounts ",
				count, count==1 ? "" : "s",
				ts->unseated_players, ts->idle_players,
				ts->active_real_tables, ts->active_real_tables==1 ? "" : "s",
				play_tables, play_tables==1 ? "" : "s",
				//rake_rate,
				ts->number_of_accounts);
		}
		else
		{
		
			sprintf(str, "%d Players Connected%s, \n"
					 	 "%d Tables in Use",
				count, count==1 ? "" : " ",
				play_tables + ts->active_real_tables
				//rake_rate,
				);
		
		}
	}
  #endif
	SetWindowTextIfNecessary(GetDlgItem(hCardRoomDlg, IDC_USER_COUNT), str);

  #if 0	//19991102MB
	int game_list_type = GameListTabIndex;	// always retrieve inside the CritSec
  #endif
	LeaveCriticalSection(&CardRoomVarsCritSec);

  #if 0	//19991102MB
	// If we didn't find any playing tables, prompt the user to join
	// a waiting list.
	if (!iPromptedUserToJoinWaitList[game_list_type] &&
				lowest_stakes_serial_number &&
				!found_playing_table &&
				!on_a_waiting_list &&
				game_list_type != GAME_HEADS_UP_HOLDEM
			  #if ADMIN_CLIENT
				&& !RunningManyFlag
				&& !iAdminClientFlag
			  #endif
			) {
		iPromptedUserToJoinWaitList[game_list_type] = TRUE;
		PostMessage(hCardRoomDlg, WMP_PROMPT_JOIN_WAITLIST, lowest_stakes_serial_number, 0);
	}
  #endif
}

//****************************************************************
////
// Update the table summary info control to reflect the most recent
// data received from the server.
//
void UpdateTableInfoItems(void)
{
	if (!hCardRoomDlg || iCardRoomDlgMinimizedFlag) {
		return;	// nothing to do... dialog box does not exist.
	}
	pr(("%s(%d) UpdateTableInfoItems() has been called.\n",_FL));
	EnterCriticalSection(&CardRoomVarsCritSec);

	HWND hdr_hwnd = GetDlgItem(hCardRoomDlg, IDC_TABLE_INFO_HEADER);
	HWND info_hwnd = GetDlgItem(hCardRoomDlg, IDC_TABLE_INFO);
	if (!dwTableInfoSerialNum) {	// no table selected?
		SetWindowTextIfNecessary(hdr_hwnd, "");
		SetWindowTextIfNecessary(info_hwnd, "no table selected");
		LeaveCriticalSection(&CardRoomVarsCritSec);
		return;
	}

	char player_string[100];
	zstruct(player_string);
	char temp1[100];
	zstruct(temp1);
	char plrname[100];
	zstruct(plrname);
	char title[40];
	zstruct(title);
	sprintf(title, "%s:", TableNameFromSerialNumber(dwTableInfoSerialNum));
	SetWindowTextIfNecessary(hdr_hwnd, title);
	// Search for information about the table.
	if (TableInfo) {	// array allocated?
		for (int i=0 ; i<TableInfo_table_count ; i++) {
			if (TableInfo[i].table_serial_number == dwTableInfoSerialNum) {
				// Got it.  Fill in the info for it.
				char info_string[1000];
				info_string[0] = 0;
				int player_count = 0;
				int waiting_player_count = 0;
				struct CardRoom_TableSummaryInfo tsi;
				zstruct(tsi);
				FindTableSummaryInfo(TableInfo[i].table_serial_number, &tsi);

				for (int j=0 ; j<PLAYERS_PER_TABLE_INFO_STRUCTURE ; j++) {
					// Are we finished looking at seated players?
					if (j==MAX_PLAYERS_PER_GAME && !player_count) {
						strcat(info_string, "(no players seated)\r\n");
					}
					if (TableInfo[i].players[j].player_id) {	// someone in this seat?
						char *cityptr;
						if (TableInfo[i].players[j].flags & 0x02) {	// disconnected?
							//cityptr = "Disconnected";
							cityptr = TableInfo[i].players[j].city;
						} else {
							cityptr = TableInfo[i].players[j].city;
						}
						//20001214MB: Replace any &'s with &&
						cityptr = DelimitStringForTextControl(cityptr, temp1, sizeof(temp1));
						DelimitStringForTextControl(TableInfo[i].players[j].name, plrname, sizeof(plrname));

						if (!(TableInfo[i].players[j].flags & 0x01)) {	// waiting?
							// Seated...
							if (!player_count) {
								strcat(info_string, "Seated:\r\n");
							}
							player_count++;
							char cs1[MAX_CURRENCY_STRING_LEN];
							ChipType chip_type = CT_NONE;
							if (tsi.flags & TSIF_REAL_MONEY) {
								chip_type = CT_REAL;
							} else if (tsi.flags & TSIF_TOURNAMENT) {
								chip_type = CT_TOURNAMENT;
							} else {
								chip_type = CT_PLAY;	// assume for now
							}
							// 20000812HK: possibly override chip type for display purposes
							ChipType chip_type_to_display = chip_type;
							if (chip_type == CT_TOURNAMENT && tsi.tournament_state == TOURN_STATE_WAITING) {
								chip_type_to_display = CT_REAL;
							}
							CurrencyString(cs1, TableInfo[i].players[j].chips, chip_type_to_display);
							if (cityptr[0]) {
								sprintf(player_string, "%s (%s), %s\r\n",
										plrname,
										cityptr, cs1);
							} else {
								sprintf(player_string, "%s, %s\r\n",
										plrname, cs1);
							}
							strcat(info_string, player_string);
						} else {
							// Waiting...
							if (!waiting_player_count) {
								strcat(info_string, "\r\nWaiting List:\r\n");
							}
							waiting_player_count++;
							char *being_called_str = "";
							if (TableInfo[i].players[j].flags & 0x04) {
								// player is being called to the table right now
								being_called_str = "** ";
							}
							char *already_seated_str = "";
						  #if ADMIN_CLIENT
						  	if (iAdminClientFlag) {
								if (TableInfo[i].players[j].flags & 0x08) {
									// player is already seated elsewhere
									already_seated_str = " *";
								}
						  	}
						  #endif

							if (cityptr[0]) {
								sprintf(player_string, "   %s%s (%s)%s\r\n",
										being_called_str,
										plrname,
										cityptr,
										already_seated_str);
							} else {
								sprintf(player_string, "   %s%s%s\r\n",
										being_called_str,
										plrname,
										already_seated_str);
							};//(cityptr[0]) 
							strcat(info_string, player_string);
						}
					}
				}

				if (tsi.watching_count) {
					char str[30];
					sprintf(str, "\r\nWatching: %d\r\n", tsi.watching_count);
					strcat(info_string, str);
				}

			  #if 0
				kp1(("%s(%d) Adding lots of extra \\n's to IDC_TABLE_INFO to test scrolling.\n", _FL));
				strcat(info_string, "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
			  #endif
				SetWindowTextIfNecessary(info_hwnd, info_string);
				LeaveCriticalSection(&CardRoomVarsCritSec);
				return;
			}
		}
	}

	// Didn't find it.  Assume the server is sending it.
	SetWindowTextIfNecessary(info_hwnd, "(retrieving info...)");
	LeaveCriticalSection(&CardRoomVarsCritSec);
}

//*********************************************************
////
// Update the various buttons on the cardroom window.  Enable/disable
// as appropriate and change the text on them as appropriate.
//
void UpdateCardRoomButtons(void)
{
	char str[200];
	pr(("%s(%d) UpdateCardRoomButtons() has been called. hCardRoomDlg=$%08lx\n",_FL,hCardRoomDlg));
	EnterCriticalSection(&CardRoomVarsCritSec);
	if (!hCardRoomDlg) {	// dialog not showing?
		LeaveCriticalSection(&CardRoomVarsCritSec);
		return;
	}

	if (dwTableInfoSerialNum) {	// A currently selected table?
		sprintf(str, "Go To\nTable %s", TableNameFromSerialNumber(dwTableInfoSerialNum));
		HWND button_hwnd = GetDlgItem(hCardRoomDlg, IDC_GOTO_GAME);
		//SetWindowTextIfNecessary(button_hwnd, str);
		EnableWindow(button_hwnd, TRUE);
		// Make that button the default button.
		PostMessage(hCardRoomDlg, DM_SETDEFID, (WPARAM)IDC_GOTO_GAME, 0);
		PostMessage(button_hwnd, BM_SETSTYLE,
				(WPARAM)LOWORD(BS_DEFPUSHBUTTON|BS_OWNERDRAW), (LPARAM)TRUE);
		SetWindowTextIfNecessary(button_hwnd, str);
	  #if 0	//19990831MB
		// Make sure the next button over has a style OTHER than
		// default.
		PostMessage(GetDlgItem(hCardRoomDlg, IDC_WAIT_LIST_STATUS),
				BM_SETSTYLE, (WPARAM)LOWORD(BS_PUSHBUTTON|BS_OWNERDRAW), (LPARAM)TRUE);
	  #endif

		// Enable/disable the 'Waiting List' button depending on whether
		// this table has a waiting list.
		struct CardRoom_TableSummaryInfo tsi;
		FindTableSummaryInfo(dwTableInfoSerialNum, &tsi);
		button_hwnd = GetDlgItem(hCardRoomDlg, IDC_JOIN_WAIT_LIST);
		if (tsi.user_waiting_list_pos) {
			SetWindowTextIfNecessary(button_hwnd, "Unjoin\nwait list...");
			WaitListButtonState = 0;	// 0=unjoin, 1=join
		} else {
			SetWindowTextIfNecessary(button_hwnd, "Join The\nWaiting List...");
			WaitListButtonState = 1;	// 0=unjoin, 1=join
		}
		pr(("%s(%d) tsi.user_waiting_list_pos = %d, WaitListButtonState = %d\n", _FL, tsi.user_waiting_list_pos, WaitListButtonState));
		// If we're already playing at this table, we should disable
		// the waiting list button because it does not apply.
		int enable_flag = TRUE;
		int index = TableIndexFromSerialNumber(dwTableInfoSerialNum);
		if (index >= 0) {
			if (!Table[index].watching_flag) {
				// The window is open and we're not watching.  We must
				// be playing.
				enable_flag = FALSE;
			}
		}
	  #if 0	//19990720MB
		if (!(tsi.flags & TSIF_WAIT_LIST_REQUIRED)) {
			enable_flag = FALSE;
		}
	  #endif
		EnableWindow(button_hwnd, enable_flag);

	  #if 0	//19990825MB
		// Update the user prompt text depending on the current highlight
		str[0] = 0;
		if (index >= 0) {
			strcat(str, "Press 'Go to table' to go back to the highlighted table.");
		} else {
			strcat(str, "Press 'Go to table' to go to the highlighted table.");
		}
		if (enable_flag) {
			if (tsi.user_waiting_list_pos) {
				strcat(str, "\nPress 'Unjoin wait list' to remove yourself from the table's waiting list.");
			} else {
				strcat(str, "\nPress 'Wait List' to add yourself to the table's waiting list.");
			}
		}
		SetWindowTextIfNecessary(GetDlgItem(hCardRoomDlg, IDC_CURRENT_OPTIONS_TEXT), str);
	  #endif
	} else {	// no currently selected table...
		// Disable the 'go to table' button
		HWND button_hwnd = GetDlgItem(hCardRoomDlg, IDC_GOTO_GAME);
		//SetWindowTextIfNecessary(button_hwnd, "Go to table");                  Go to...                             Table
		SetWindowTextIfNecessary(button_hwnd, "Go to...\nTable"); 
		EnableWindow(button_hwnd, FALSE);

		// Disable the 'Waiting List' button
		EnableWindow(GetDlgItem(hCardRoomDlg, IDC_JOIN_WAIT_LIST), FALSE);

	  #if 0	//19990825MB
		SetWindowTextIfNecessary(GetDlgItem(hCardRoomDlg, IDC_CURRENT_OPTIONS_TEXT),
					"Select a table from the list");
	  #endif
	}
  #if 0	//19991110MB
	// set the login/logout button properly
	if (LoggedIn == LOGIN_VALID) {
		ShowWindow(GetDlgItem(hCardRoomDlg, IDC_LOG_IN), SW_HIDE);
	} else {
//		ShowWindow(GetDlgItem(hCardRoomDlg, IDC_LOG_IN), SW_SHOW);
		// should be enabled for login only if we have a valid connection to the server
		int connected = ClientSock && ClientSock->connected_flag;
		EnableWindow(GetDlgItem(hCardRoomDlg,IDC_LOG_IN), connected);
	}
  #endif

	LeaveCriticalSection(&CardRoomVarsCritSec);
}

//****************************************************************
////
// Select a new game type on the tab control.  Includes adding
// the list box control to the tab window.
// Converts the game type to a tab index.
//
void SelectGameTypeTab(HWND hDlg, ClientDisplayTabIndex client_display_tab_index)
{
//	kp(("%s(%d) SelectGameTypeTab(HWND, %d) has been called.\n",_FL, tab_number));

	// If there was a previous list view control, destroy it.
	if (hGameListCtrl) {
		DestroyWindow(hGameListCtrl);
		hGameListCtrl = NULL;
	}

	// Change the current tab selection...
	HWND tab_hwnd = GetDlgItem(hDlg, IDC_GAME_TYPE_TAB);
    
    int tab_number = 0;
	for (int i=0 ; i<ACTUAL_CLIENT_DISPLAY_TABS ; i++) {
		if (client_display_tab_index==GameTypeTabOrders[i]) {
			tab_number = i;
			break;
		}
	}

	TabCtrl_SetCurSel(tab_hwnd, tab_number);
	if (GameListTabIndex != tab_number) {
		// A new tab was selected.
		pr(("%s(%d) A new tab was selected.\n", _FL));
		dwTableInfoSerialNum = 0;	// we no longer have a default.
	}
	GameListTabIndex = client_display_tab_index;
	CheckForNewGameList(GameListTabIndex);	// ask server for the most recent data if necessary

	// Choose a new default table for this game type.
	dwTableInfoSerialNum = SelectDefaultTable(client_display_tab_index);
	pr(("%s(%d) Tab %d: dwTableInfoSerialNum = %d\n", _FL, client_display_tab_index, dwTableInfoSerialNum));

	// Retrieve the rect for the entire tab control, then use
	// TabCtrl_AdjustRect to adjust the rect to the client area.
	RECT r, r1;
	zstruct(r1);
	GetWindowRect(tab_hwnd, &r1);
	r = r1;	// keep original rect
	TabCtrl_AdjustRect(tab_hwnd, FALSE, &r);
	r.left   -= r1.left;
	r.right  -= r1.left;
	r.top    -= r1.top;
	r.bottom -= r1.top;
	r.top   += 3;	// inset it a few extra pixels from the tab control header.
	r.right -= 2;	// inset it a few extra pixels from the right side.
	pr(("%s(%d) Client area rect = %d,%d,%d,%d\n", _FL, r.left, r.top, r.right, r.bottom));

	// Create a new List View control.
	hGameListCtrl = CreateWindow(WC_LISTVIEW, "",
	        WS_CHILD | LVS_REPORT | LVS_SORTASCENDING | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
	        tab_hwnd, NULL, hInst, NULL);
	pr(("%s(%d) hGameListCtrl = $%08lx\n", _FL, hGameListCtrl));
	if (hGameListCtrl) {
	  #if 1	//19990819MB
		// Disable the background
		ListView_SetBkColor(hGameListCtrl, CLR_NONE);
		


	  #endif
    
        //Aqui se cambia el color de la lista de mesas
		
		//brown
	//	ListView_SetBkColor(hGameListCtrl, 0x0014237B);
		ListView_SetBkColor(hGameListCtrl, 0x0006366A);
		ListView_SetTextBkColor(hGameListCtrl, 0x0014237B);
		ListView_SetTextColor(hGameListCtrl, 0x00FFFFFF);		

	/*	
		//blue
		ListView_SetBkColor(hGameListCtrl, 0x00820000);
		ListView_SetTextBkColor(hGameListCtrl, 0x00820000);
		ListView_SetTextColor(hGameListCtrl, 0x00FFFFFF); 
*/


		SendMessage(hGameListCtrl, WM_SETFONT, (WPARAM)hCardRoomButtonFont, 0);



		// Add our columns to the list view control...
		iCurrentColumnCount = 0;
		zstruct(CurrentColumnList);
		int *src_column_list = TabColumnProfiles[client_display_tab_index];

		// Calculate the total desired width for the columns we're being
		// asked to display in this profile.
		int total_width = 0;
		int scroll_bar_width = iLargeFontsMode ? 18 : 16;
		int usable_width = (r.right - r.left) - scroll_bar_width;	// leave room for scroll bar if necessary
		i = 0;

		// manual overrides for large font mode...
		if (iLargeFontsMode) {
			TableListColumnHeadings[TLC_AVERAGE_POT] = "AvgPot";
		}
		while (src_column_list[i] >= 0) {
			int column = src_column_list[i++];
		  #if INCLUDE_TABLE_RAKE_PER_HOUR
		  	if (!iAdminClientFlag || LoggedInPrivLevel < ACCPRIV_ADMINISTRATOR) {
				if (column==TLC_RAKE_PER_HOUR) {	// $/hr column?
					continue;	// yes, skip it.
				}
		  	}
		  #endif
			CurrentColumnList[iCurrentColumnCount] = column;
			total_width += TableListColumnWidthPercentages[column];
			iCurrentColumnCount++;
		}

		//kp(("%s(%d) %d columns, total_width = %d, usable pixels = %d\n", _FL, iCurrentColumnCount, total_width, usable_width));
      #if INCLUDE_TABLE_RAKE_PER_HOUR
		ChangeRakePerHourColumn(iRakePerHourType);
      #endif
	  #if ADMIN_CLIENT
		ChangeHandsPerHourColumn(iHandsPerHourType);
	  #endif

		for (i=0 ; i<iCurrentColumnCount ; i++) {
			int column = CurrentColumnList[i];
		  #if INCLUDE_TABLE_RAKE_PER_HOUR
		  	if (!iAdminClientFlag || LoggedInPrivLevel < ACCPRIV_ADMINISTRATOR) {
				if (column==TLC_RAKE_PER_HOUR) {	// $/hr column?
					continue;	// yes, skip it.
				}
		  	}
		  #endif

			LVCOLUMN lvc;
			zstruct(lvc);
			lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
			lvc.fmt  = LVCFMT_LEFT;
			lvc.pszText = TableListColumnHeadings[column];
			lvc.cx = (TableListColumnWidthPercentages[column] * usable_width) / total_width;
			CurrentColumnIndex[column] = ListView_InsertColumn(hGameListCtrl, i, &lvc);
		}

		UpdateGameListViewItems();	// display latest info from server.

	  #if 0	//19990810MB: no longer needed now that we pick a good default.
		// Select a default item.  For now, just choose item 0.
		// In the future, we'll want to choose any table of the
		// same stakes as they were playing last time, as long as
		// there is at least one vacant seat.
		int default_item_index = 0;
		ListView_SetItemState(hGameListCtrl, default_item_index, LVIS_SELECTED, LVIS_SELECTED);
		// Make sure we can see our default item. Scroll if necessary.
		ListView_EnsureVisible(hGameListCtrl, default_item_index, FALSE);
	  #endif
	}
	ShowWindow(hGameListCtrl, SW_SHOW);
	SetFocus(hGameListCtrl);

	// Check/uncheck the menu items for the current game type
	HMENU hm = GetMenu(hCardRoomDlg);
	CheckMenuItem(hm, IDM_SHOW_HOLDEM,		client_display_tab_index==DISPLAY_TAB_HOLDEM      ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hm, IDM_SHOW_OMAHA,		client_display_tab_index==DISPLAY_TAB_OMAHA_HI    ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hm, IDM_SHOW_OMAHA_HI_LO, client_display_tab_index==DISPLAY_TAB_OMAHA_HI_LO ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hm, IDM_SHOW_7CS,			client_display_tab_index==DISPLAY_TAB_STUD7       ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hm, IDM_SHOW_7CS_HI_LO,	client_display_tab_index==DISPLAY_TAB_STUD7_HI_LO ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hm, IDM_SHOW_1ON1,		client_display_tab_index==DISPLAY_TAB_ONE_ON_ONE  ? MF_CHECKED : MF_UNCHECKED);
	RequestTableInfo(dwTableInfoSerialNum, TRUE);	// always request the latest info
	UpdateTableInfoItems();		// redraw the info items for current table.
	UpdateCardRoomButtons();	// change the various buttons to reflect current selection
}

//****************************************************************
////
// Initialize the tab control for the card room window.  It should
// contain all the games that are available to be played.
//
void InitGameTypeTabControl(HWND hDlg)
{
	HWND tab_hwnd = GetDlgItem(hDlg, IDC_GAME_TYPE_TAB);
	if (!tab_hwnd) {
		Error(ERR_INTERNAL_ERROR, "%s(%d)", _FL);
		return ;
	}
	SendMessage(tab_hwnd, TCM_DELETEALLITEMS, 0, 0);

	// Change the size of the tabs so they fill the entire top of the
	// control (to avoid leaving a grey area to the right).

	RECT r;
	GetWindowRect(tab_hwnd, &r);
	int cx = (r.right - r.left) / ACTUAL_CLIENT_DISPLAY_TABS;
	TabCtrl_GetItemRect(tab_hwnd, 0, &r);
	TabCtrl_SetItemSize(tab_hwnd, cx, r.bottom - r.top);
     
	// Add our tabs...
	for (int i=0 ; i<ACTUAL_CLIENT_DISPLAY_TABS ; i++) {
		// Small font mode version
		static char *TabTitles[MAX_CLIENT_DISPLAY_TABS] = {
				"Hold'em",
				"Omaha Hi",
				"7 Card Stud",
				"1 on 1",
				"Omaha H/L",
				"7 Stud H/L",
				"Tourney",
		};
		// Large font mode version
		static char *TabTitles_Short[MAX_CLIENT_DISPLAY_TABS] = {
				"Hold'em",
				"Omaha Hi",
				"7 Stud",
				"1 on 1",
				"Omaha H/L",
				"7Stud H/L",
				"Tourney",
		};
		TCITEM tci;
		zstruct(tci);
		tci.mask = TCIF_TEXT|TCIF_PARAM;
		if (iLargeFontsMode) {
			tci.pszText = TabTitles_Short[GameTypeTabOrders[i]];
		} else {
			tci.pszText = TabTitles[GameTypeTabOrders[i]];
		}
		tci.lParam = GameTypeTabOrders[i];	// keep track of which game type this is.
		SendMessage(tab_hwnd, TCM_INSERTITEM, (WPARAM)i, (LPARAM)&tci);
	}
	pr(("%s(%d) After adding tabs... TabCtrl_GetItemCount() = %d\n", _FL, TabCtrl_GetItemCount(tab_hwnd)));
	// If the user has a favourite game type, make it the default tab.
	if (Defaults.preferred_display_tab_index < 0 || Defaults.preferred_display_tab_index >= ACTUAL_CLIENT_DISPLAY_TABS)
		Defaults.preferred_display_tab_index = DISPLAY_TAB_HOLDEM;
	SelectGameTypeTab(hDlg, Defaults.preferred_display_tab_index);	// display stuff for default tab.

	SetBkColor(GetDC(tab_hwnd), RGB(0,0,0));

}

//****************************************************************
////
// Mesage handler for 'going to table' Window
//
BOOL CALLBACK dlgFuncGoingToTable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//kp(("%s(%d) dlg $%08lx got message = %d\n",_FL, hDlg, message));
	switch (message) {
	case WM_INITDIALOG:
		//kp(("%s(%d) dialog $%08lx got WM_INITDIALOG\n", _FL, hDlg));
		hGoingToTableDlg = hDlg;
		return TRUE;

  #if 0	//19990805MB
	case WM_COMMAND:
		kp(("%s(%d) dialog $%08lx got WM_COMMAND: id = %d\n", _FL, hDlg, LOWORD(wParam)));
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDCANCEL:
				//kp(("%s(%d) IDCANCEL received.\n",_FL));
				DestroyWindow(hDlg);
			    return TRUE;	// We DID process this message.
			}
		}
		break;
  #endif
	case WMP_CLOSE_YOURSELF:
	case WM_CLOSE:	
		DestroyWindow(hDlg);
	    return TRUE;	// We DID process this message.
	case WM_DESTROY:
		//kp(("%s(%d) dialog $%08lx got WM_DESTROY\n", _FL, hDlg));
		//DestroyWindow(hDlg);
		hGoingToTableDlg = NULL;
	    return TRUE;	// We DID process this message.
	}
	NOTUSED(hDlg);
	NOTUSED(wParam);
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}

//*********************************************************
////
// Open window and ask server to join a table.
//
void JoinTable(WORD32 table_serial_number)
{
	// only allow them to open another window if we have room for it.

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
		LeaveCriticalSection(&ClientSockCritSec);

		//hide cardroom
		//ShowWindow(hCardRoomDlg, SW_HIDE);
		CheckForUpgrade(FALSE);
	}else{
		int table_count = 0;
		for (int i=0 ; i<MAX_TABLES ; i++) {
			if (Table[i].hwnd) {
				if (Table[i].table_serial_number==table_serial_number) {
					// We've already got this one open... don't worry about
					// opening too many windows.
					break;
				}//if
				table_count++;
			}//if
		}//for
		if (table_count >= MAX_TABLES) {
				MessageBox(hCardRoomDlg,
					"You have too many tables open.\n\n"
					"Please close some of your other table\n"
					"windows before joining a new table.",
					"Too many tables open",
					MB_OK);
			return;	// don't attempt to join
		}//if
		// If necessary, minimize the cardroom window any time we switch to
		// a table.
	  #if 0	// 990907HK: moved to comm.cpp 
		if (!Defaults.keep_cardroom_window_open) {
			ShowWindow(hCardRoomDlg, SW_MINIMIZE);
		}
	  #endif

		EnterCriticalSection(&CardRoomVarsCritSec);
		int result = ShowTableWindow(table_serial_number);
		if (!result && !hGoingToTableDlg) {
			// Not shown... bring up the 'going to table' window.
			CreateDialog(hInst, MAKEINTRESOURCE(IDD_GOING_TO_TABLE),
							  	NULL, dlgFuncGoingToTable);
			//kp(("%s(%d) hGoingToTableDlg = $%08lx GetLastError() = %d\n", _FL, hGoingToTableDlg, GetLastError()));
		  #if 0	//19990727MB
			if (hGoingToTableDlg) {
				kp(("%s(%d) Calling WinPosWindowOnScreen()...\n", _FL));
				WinPosWindowOnScreen(hGoingToTableDlg);
				kp(("%s(%d) Calling ShowWindow()...\n", _FL));
				ShowWindow(hGoingToTableDlg, SW_SHOW);
				kp(("%s(%d) Done showing window.\n", _FL));
			}
		  #endif
		}//if
		LeaveCriticalSection(&CardRoomVarsCritSec);

		// Send a DATATYPE_CARDROOM_JOIN_TABLE packet.
		struct CardRoom_JoinTable jt;
		zstruct(jt);
		jt.table_serial_number = table_serial_number;
		jt.status = 2;	// request to watch this table.
		//ccv my money text
		jt.seating_position=20;
		//ccv my money text
		SendDataStructure(DATATYPE_CARDROOM_JOIN_TABLE, &jt, sizeof(jt));
	};
}//join table

#if PLAY_MONEY_ONLY
//*********************************************************
////
// Display a message about real money becoming available
// down the road.
// Return TRUE if the table was real money, FALSE if it was play money,
//
int RealMoneyTableMsg(WORD32 table_serial_number)
{
	struct CardRoom_TableSummaryInfo ti;
	zstruct(ti);
	ErrorType err = FindTableSummaryInfo(table_serial_number, &ti, NULL);
	if (err != ERR_NONE) {
		return FALSE;
	}
	if (ti.flags & TSIF_REAL_MONEY) {
		MessageBox(hCardRoomDlg,
				"The real money tables are not yet open.\n\n"
				"Please feel free to try out our play money tables.",
				"Real money tables opening soon",
				MB_OK);
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif	// PLAY_MONEY_ONLY

//*********************************************************
////
// Close the 'going to table' dialog if it is open.
// This can be called from any thread (it's non-blocking).
//
void CloseGoingToTableDialog(void)
{
	if (hGoingToTableDlg) {
		PostMessage(hGoingToTableDlg, WMP_CLOSE_YOURSELF, 0, 0);
	}
}	

//*********************************************************
////
// If the cardroom window is open and this client version
// is newer than the last official release, assume we're a beta
// and that the upgrade menu item must be changed.
//
void ChangeUpgradeMenuToRevertIfNecessary(void)
{
	if (hCardRoomDlg && ServerVersionInfo.new_client_version.build &&
		ClientVersionNumber.build > ServerVersionInfo.new_client_version.build) {
		// We're newer than the latest version on the server... this means
		// we're probably a beta version.
		pr(("%s(%d) beta version detected (%d.%02d build %d)\n",_FL,
				ClientVersionNumber.major, ClientVersionNumber.minor,
				ClientVersionNumber.build & 0x00FFFF));

		HMENU hm = GetMenu(hCardRoomDlg);	// get HMENU for menu bar
		hm = GetSubMenu(hm, 2);	// get 3rd menu off menu bar
		if (hm) {
			DeleteMenu(hm, IDM_CHECK_FOR_UPDATE, MF_BYCOMMAND);

			MENUITEMINFO mii;
			zstruct(mii);
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;
			mii.fType = MFT_STRING;
			mii.fState = MFS_ENABLED;
			mii.wID = (WORD)IDM_CHECK_FOR_UPDATE;
			mii.dwTypeData = "Revert to last officially released version";

			// InsertMenuItem(hm, insert_position, MF_BYPOSITION, &mii);
		}
	}
}

//*********************************************************
////
// Handle a request from the user to have their all-ins reset.
//
void HandleAllInResetRequest(HWND hDlg)
{
	// only allow them to do it if they have one or fewer all-ins left.
	if (SavedAccountInfo.all_in_count >= SavedAccountInfo.all_ins_allowed) {
		MessageBox(hDlg,"You have not used any of your All-Ins.\n\n"
						"There is no need to have them reset.",
						"All-Ins", MB_OK|MB_TOPMOST);
		return;
	}
	if (SavedAccountInfo.all_in_count > 1) {
		char str[400];
		zstruct(str);
		sprintf(str,
				"You still have %d All-Ins remaining.\n\n"
				"When get down to one or zero you may\n"
				"request to have them reset.",
				SavedAccountInfo.all_in_count);
		MessageBox(hDlg, str, "All-Ins", MB_OK|MB_TOPMOST);
		return;
	}

	int result = MessageBox(hDlg,
			"Do you wish to send an email to customer support\n"
			"which asks them to manually reset your All-In count?",
			"All-Ins", MB_YESNO|MB_TOPMOST);
	if (result==IDYES) {
		// Send the request off to the server.
		struct MiscClientMessage mcm;
		zstruct(mcm);
		mcm.message_type = MISC_MESSAGE_REQ_ALLIN_RESET;
		SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

		MessageBox(hDlg,
			"Your reset request has been sent.",
			"All-Ins", MB_OK|MB_TOPMOST);
	}
}

#if ADMIN_CLIENT
/**********************************************************************************
 Function ProcessNewAdminInfoBlock(void);
 Date: CR01/01/12
 Purpose: deal with a block of admin text that just arrived from the server
***********************************************************************************/
void ProcessNewAdminInfoBlock(void)
{
	if (hInfoBlock) {
		PostMessage(hInfoBlock, WMP_UPDATE_YOURSELF, 0, 0);
		ShowWindow(hInfoBlock, SW_SHOWNORMAL);
		ReallySetForegroundWindow(hInfoBlock);
	} else {
		CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADMIN_INFO), NULL, dlgAdminInfo);
	}
}

/**********************************************************************************
 Function BadTimeForIntensiveServerRequest(void)
 Date: CR00/10/14
 Purpose: returns TRUE and pops up a message if we shouldn't do a check run now
***********************************************************************************/
int BadTimeForIntensiveServerRequest(char *title, char *msg)
{
  #ifdef HORATIO
	return FALSE;
  #endif	
  
  time_t tt = time(NULL);
  struct tm *t = localtime(&tt);
  int minutes = t->tm_min;
  if (minutes >= 55 || minutes < 10) {	// we're in the x:h:55:00 to h:09:59 range
	MessageBox(hCardRoomDlg, msg, title, MB_OK|MB_ICONWARNING|MB_TOPMOST);
	return TRUE;
  }
  return FALSE;	// means it's ok to go ahead
}
#endif



//****************************************************************
////
// Mesage handler for Card Room Window

BOOL CALLBACK dlgFuncCardRoom(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
//	kp(("%s(%d) message = %d\n",_FL, message));
	switch (message) {
	case WM_INITDIALOG:
		hCardRoomDlg = hDlg;
		ScaleDialogTo96dpi(hDlg);
		// original one 
		// SetWindowSize(hDlg, 800, 560);
		SetWindowSize(hDlg, 800, 560);
        
		// Add the system menu and our icon.  See Knowledge base
		// article Q179582 for more details.
		{
			HICON hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CLIENT),
					IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                    GetSystemMetrics(SM_CYSMICON), 0);
			if(hIcon) {
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			}

			HMENU hm = GetMenu(hCardRoomDlg);
			// CheckMenuItem(hm, IDC_HIDE_PLAY_MONEY, Defaults.iHidePlayMoneyTables ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hm, IDM_ENABLE_4COLOR_DECK, Defaults.i4ColorDeckEnabled ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hm, IDM_ENABLE_ANIMATION, Defaults.iAnimationDisabled ? MF_UNCHECKED : MF_CHECKED);
			CheckMenuItem(hm, IDM_ENABLE_SOUNDS, Defaults.iSoundsDisabled ? MF_UNCHECKED : MF_CHECKED);
			CheckMenuItem(hm, IDM_ENABLE_BAR_SNACKS, Defaults.iBarSnacksDisabled ? MF_UNCHECKED : MF_CHECKED);
			CheckMenuItem(hm, IDM_OPTIMIZE_GRAPHICS, Defaults.iDisableGraphicOptimizations ? MF_UNCHECKED : MF_CHECKED);
		}
		InitGameTypeTabControl(hDlg);
		SetTimer(hDlg, WM_TIMER, 500, NULL);	// .5 second timer
		AddKeyboardTranslateHwnd(hDlg);
	  #if ADMIN_CLIENT
		if (iAdminClientFlag) {
		  #if 0	//19990930MB: moved to menu
			ShowWindow(GetDlgItem(hCardRoomDlg, IDC_RECONNECT), SW_SHOW);
			ShowWindow(GetDlgItem(hCardRoomDlg, IDC_BROADCAST_MESSAGE), SW_SHOW);
		  #endif

			ShowWindow(GetDlgItem(hCardRoomDlg, IDC_PAUSE_SENDING), SW_SHOW);

#if COMPILE_ADMIN

			HMENU hm = GetMenu(hCardRoomDlg);	// get HMENU for menu bar
			
			hm = GetSubMenu(hm, 2);	// get 1nd menu off menu bar
			if (hm) {
				MENUITEMINFO mii;
				zstruct(mii);
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID|MIIM_SUBMENU;
				mii.fState = MFS_ENABLED;
				mii.hSubMenu = hm;
				int pos = 12 ;
				mii.wID = 0;
				mii.dwTypeData = "Administrator";
				InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);		
				hm = GetSubMenu(hm, 12);	// get 1nd menu off menu bar
				if (hm) {
					MENUITEMINFO mii;
					zstruct(mii);
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;
					mii.fState = MFS_ENABLED;
					int pos = 0 ;		
					mii.fType = MFT_SEPARATOR;
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					mii.fType = MFT_STRING;
					mii.wID = (WORD)IDD_CHAT_MONITOR;
					mii.dwTypeData = "Admin: Chat Monitor";
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					mii.wID = (WORD)IDM_EDIT_USER_ACCOUNTS;
					mii.dwTypeData = "Admin: Edit &User Accounts";
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					mii.wID = (WORD)IDC_BROADCAST_MESSAGE;
					mii.dwTypeData = "Admin: Send Broadcast Message";
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					mii.fType = MFT_SEPARATOR;
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					mii.fType = MFT_STRING;
					mii.wID = (WORD)IDM_CHANGE_ALLINS;
					mii.dwTypeData = "Admin: Change Global All-Ins";
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
					#if COMPILE_ADMIN_SHOT
						mii.wID = (WORD)IDM_CHANGE_SHOTCLOCK;
						mii.dwTypeData = "Admin: Change Shot Clock Message";
						InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
						mii.wID = (WORD)IDD_ADMIN_STATS;
						mii.dwTypeData = "Admin: Show server stats";
						InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
						mii.wID = (WORD)IDM_START_CHECK_RUN;
						mii.dwTypeData = "Admin: Start check run (harmless)";
						InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii); 
                    #endif
					mii.wID = (WORD)IDC_RECONNECT;
					mii.dwTypeData = "Debug: reconnect";
					InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
				}
				}
#endif
			
//			ShowWindow(GetDlgItem(hCardRoomDlg, IDC_REQUEST_HAND_HISTORY), SW_SHOW);
			if (AutoShowServerStats) {
				PostMessage(hDlg, WM_COMMAND, IDD_ADMIN_STATS, 0);
			}

		  #if 0	//20000509MB: not yet supported :(
			//20000509MB: make window semitransparent
			SetWindowLong(hDlg, GWL_EXSTYLE, WS_EX_LAYERED);
			SetLayeredWindowAttributes(hDlg, 0, 200, LWA_ALPHA);
		  #endif
		}
	  #endif
		ChangeUpgradeMenuToRevertIfNecessary();
		ShowWindow(GetDlgItem(hCardRoomDlg, IDC_USER_COUNT), SW_SHOW);
		if (hShotClockFont) {
			//kp(("%s(%d) hShotClockFont = $%08lx\n", _FL, hShotClockFont));
			SendMessage(GetDlgItem(hDlg, IDC_SHOT_CLOCK_MESSAGE), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			SendMessage(GetDlgItem(hDlg, IDC_SHOT_CLOCK_ETA), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			SendMessage(GetDlgItem(hDlg, IDC_TABLE_INFO), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			
		}
		UpdateShotClock();		
		Defaults.iHidePlayMoneyTables = FALSE ;
		Defaults.changed_flag = TRUE;
		FlagRedrawCardRoom();
		

		return TRUE;

	case WM_INITMENU:
		// Disable the maximize and size menu items on our system menu.
		EnableMenuItem((HMENU)wParam, SC_MAXIMIZE, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem((HMENU)wParam, SC_SIZE,     MF_BYCOMMAND|MF_GRAYED);
		break;

	case WM_CLOSE:
		PostMessage(hDlg, WMP_CLOSE_YOURSELF, 0, 0);
		break;

	case WM_SYSCOMMAND:
		switch (wParam) {
		case WMP_SHOW_ABOUT_DLG :
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), HWND_DESKTOP,
				(DLGPROC)dlgAboutBoxFunc, NULL);
			break;
		}
		break;	
	case WM_ACTIVATE:
		if (LOWORD(wParam)!=WA_INACTIVE) {	// we got activated...
			// any time we get activated we should redraw the whole window to
			// make sure nothing gets left half-drawn due to bugs in the video
			// drivers or elsewhere that seem to forget about refreshing
			// certain areas of the screen.
			InvalidateRect(hDlg, NULL, FALSE);
		}
		MaskCache_FreeAll();	// force re-build of any video memory masks
		return TRUE;	// message handled
	case WM_COMMAND:
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDC_ACCOUNT_INFO:
				pr(("%s(%d) IDC_ACCOUNT_INFO received.\n",_FL));
				return TRUE;	// message handled
			case IDM_OPTIONS_ALLINS:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
					Sleep(500);	// give other packets a chance to arrive.
				}
				if (LoggedIn == LOGIN_VALID) {
					char str[400];
					zstruct(str);
					sprintf(str,
							"You have %d All-In%s remaining right now.\n\n"
							"You get %d All-In%s per 24 hour period.",
							SavedAccountInfo.all_in_count,
							SavedAccountInfo.all_in_count==1 ? "" : "s",
							SavedAccountInfo.all_ins_allowed,
							SavedAccountInfo.all_ins_allowed==1 ? "" : "s");
					MessageBox(hDlg, str, "All-Ins", MB_OK);
				}
				return TRUE;	// message handled
			case IDM_OPTIONS_SHOW_CONNECT_MSG:
				//Defaults.news_serial_number = 0;	// force display regardless of serial number
				//DisplayNewsFromWebSite();
				//return TRUE;	// message handled
				LaunchInternetBrowser(BASE_URL);
				return TRUE;
			case IDC_CASHIER:
              pr(("%s(%d) IDC_CASHIER received.\n",_FL));
			  #if 0
				MessageBox(hDlg,
						"The cashier functions will become available when\n"
						"real money tables are launched.\n",
						"Cashier...",
						MB_OK);
			  #else
				OpenCashierScreen();	// open/show the cashier screen
				
			/*	MessageBox(hDlg,
						"The Cashier is momentarily down \n",
						"e-Media Poker",
						MB_OK);  */
			  #endif
			  
              // DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DISABLE_CASHIER), HWND_DESKTOP,
			  //       		(DLGPROC)dlgDisableCashierFunc, NULL);
			  return TRUE;	// message handled
			case IDC_EXIT:

				// we're logged in? -- handle it
				if (LoggedIn == LOGIN_VALID) {
					if (!WaitListButtonState) {
						UnjoinWaitListDialog(hDlg, dwTableInfoSerialNum, 0 );
					}
                } 

				pr(("%s(%d) IDC_EXIT received.\n",_FL));
				PostMessage(hDlg, WMP_CLOSE_YOURSELF, 0, 0);
				return TRUE;	// We DID process this message.
			case IDC_GAME_TYPE_TAB:
				pr(("%s(%d) IDC_GAME_TYPE_TAB received.\n",_FL));
				return TRUE;	// We DID process this message.
			case IDC_GOTO_GAME:
				pr(("%s(%d) IDC_GOTO_GAME received.\n",_FL));
				// If a game has been selected... send a request to watch
				// this game.
				if (dwTableInfoSerialNum) {
				  #if PLAY_MONEY_ONLY
				  	// Test if this is a real money table.  If so, display a
					// message and don't join.
					if (RealMoneyTableMsg(dwTableInfoSerialNum)) {
						return TRUE;	// We DID process this message.
					}
				  #endif
					// If we're already joined to this table, show the window.
					JoinTable(dwTableInfoSerialNum);
				}
				return TRUE;	// We DID process this message.
			case IDC_HIDE_PLAY_MONEY:
  			  
   
			  #if 1	//19990816MB
				{
					HMENU hm = GetMenu(hCardRoomDlg);
					Defaults.iHidePlayMoneyTables = GetMenuState(hm, IDC_HIDE_PLAY_MONEY, 0) & MF_CHECKED ? FALSE : TRUE;
					CheckMenuItem(hm, IDC_HIDE_PLAY_MONEY, Defaults.iHidePlayMoneyTables ? MF_CHECKED : MF_UNCHECKED);
				}
			  #else
				pr(("%s(%d) Detected 'Hide play money' checkbox pressed. New state = %d\n",_FL,IsDlgButtonChecked(hDlg, IDC_HIDE_PLAY_MONEY)));
				Defaults.iHidePlayMoneyTables = IsDlgButtonChecked(hDlg, IDC_HIDE_PLAY_MONEY);
			  #endif
				Defaults.changed_flag = TRUE;
				FlagRedrawCardRoom();
				return TRUE;	// We DID process this message.
			case IDC_JOIN_WAIT_LIST:
				pr(("%s(%d) IDC_JOIN_WAIT_LIST received.  WaitListButtonState = %d\n",_FL,WaitListButtonState));
			  #if PLAY_MONEY_ONLY
			  	// Test if this is a real money table.  If so, display a
				// message and don't join.
				if (RealMoneyTableMsg(dwTableInfoSerialNum)) {
					return TRUE;	// We DID process this message.
				}
			  #endif
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in before joining a waiting list");
				}
				// we're logged in? -- handle it
				if (LoggedIn == LOGIN_VALID) {
					if (WaitListButtonState) {
						if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
							HWND hDlg;
							DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
						}else{						
							WaitListDialog(hDlg, dwTableInfoSerialNum);
						}
					} else {
						UnjoinWaitListDialog(hDlg, dwTableInfoSerialNum, 1);
					}
				}
				return TRUE;	// We DID process this message.
			case IDC_LOG_IN:
				    pr(("%s(%d) IDC_LOG_IN received.\n",_FL));
					LogInToServer("Enter your User ID and password...");				
				return TRUE;	// We DID process this message.
			case IDC_NEWS:
				LaunchInternetBrowser("http://www.e-mediasoftware.com/news/news.php");
				return TRUE;	// We DID process this message.
			case IDM_NETWORK_STATUS:
				LaunchInternetBrowser("http://www.e-mediasoftware.com");
				return TRUE;	// We DID process this message.
			case IDC_ONLINE_HELP:
				LaunchInternetBrowser("http://www.e-mediasoftware.com/support/support.php");
				return TRUE;	// We DID process this message.
			case IDC_PROMOTIONUPDATES:
				LaunchInternetBrowser("http://www.e-mediasoftware.com/promotions/promotions.php");		
				/* if (
					 ((ClientVersionNumber.major  & 0x0000FFFF )< (ServerVersionInfo.min_client_version.major  & 0x0000FFFF)) ||
					 ((ClientVersionNumber.minor  & 0x0000FFFF )< (ServerVersionInfo.min_client_version.minor  & 0x0000FFFF)) ||
					 (ServerVersionInfo.min_client_version.flags )
					
					) {

	                	iDisableServerCommunications = FALSE;

		                 EnterCriticalSection(&ClientSockCritSec);

		                 if (ClientSock) {
			                 ClientSock->CloseSocket();
						 };//if

		                 LeaveCriticalSection(&ClientSockCritSec);
						 int res=MessageBox(hCardRoomDlg, "New e-Media Poker Version Found \n Do you want to process the upgrader ?", "e-Media Poker Upgrader",MB_YESNO);
						 if(res==IDYES){
		                      CheckForUpgrade(FALSE);
							  //CheckForUpgrade(TRUE);
						 };//if to process the upgrader
				 }else{
					 //we have the lastest version
                      MessageBox(hCardRoomDlg, "There's no new updates at the moment, Thanks", 

								 "e-Media Poker Upgrader",MB_OK);
				 };//if*/
	
				return TRUE;	// We DID process this message.

            case IDC_LINK:
				LaunchInternetBrowser("http://www.e-mediasoftware.com/support/support.php");
				return TRUE;	// We DID process this message.

	

			case IDC_CONTACT_MANAGEMENT:

				if ((ClientVersionNumber.build & 0x0000FFFF ) == (ServerVersionInfo.new_client_version.build & 0x0000FFFF)) {   
					MessageBox(hDlg, "Your e-Media Poker Client Software is the current version.\n You do not need to upgrade at this time. Enjoy the Games!", "Pokercosm.com", MB_OK);				    
				}
				else if (((ClientVersionNumber.build & 0x0000FFFF ) < (ServerVersionInfo.new_client_version.build & 0x0000FFFF))&&
						 ((ClientVersionNumber.build & 0x0000FFFF ) >= (ServerVersionInfo.min_client_version.build & 0x0000FFFF)))           
				{   
				   DialogBox(hInst, (LPCTSTR)IDD_DOWNLOAD_INQUIRY, NULL, (DLGPROC)dlgDownLoadClient);  
                   
				}
				else if ((ClientVersionNumber.build & 0x0000FFFF ) < (ServerVersionInfo.min_client_version.build & 0x0000FFFF)) {				
				    DialogBox(hInst, (LPCTSTR)IDD_DOWNLOAD_MUST, NULL, (DLGPROC)dlgDownLoadClient);  
				}

				return TRUE;	// We DID process this message.
			case IDM_CHECK_FOR_UPDATE:
				// If ctrl and alt are pressed, check for a beta client.
				{
					static int beta_mode = FALSE;
					if ((GetKeyState(VK_MENU) & 0x80) && (GetKeyState(VK_CONTROL) & 0x80)) {
						//kp(("%s(%d) CTRL and ALT pressed while checking for update... looking for beta version\n",_FL));
						beta_mode = TRUE;
					}
					if (beta_mode) {
						if (MessageBox(hDlg,
							"You have requested to check for a BETA client\n"
							"upgrade.  Beta versions have not finished going\n"
							"through our quality assurance department and may\n"
							"contain unexpected problems.\n\n"
							"Our support department is interested in hearing\n"
							"about any problems you may have, but they may not\n"
							"be able to help you resolve them.\n\n"
							"You should only proceed with the Beta download\n"
							"if you understand these risks and accept the\n"
							"responsibility of running a partly untested program.",
							"BETA DOWNLOAD WARNING",
							MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST|MB_ICONSTOP)==IDOK)
						{
							strcpy(ServerVersionInfo.new_version_string, "Beta client");
							strcpy(ServerVersionInfo.new_ver_auto_url,
									BASE_URL2 "client-beta/upgrinfo.txt.gz");
						} else {
							return TRUE;	// We DID process this message.
						}
					}
	
					CheckForUpgrade(FALSE);
					//CheckForUpgrade(TRUE);
					return TRUE;	// We DID process this message.
				}
			case IDM_ABOUT:
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), HWND_DESKTOP,
						(DLGPROC)dlgAboutBoxFunc, NULL);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_HOLDEM:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_HOLDEM);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_OMAHA:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_OMAHA_HI);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_OMAHA_HI_LO:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_OMAHA_HI_LO);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_7CS:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_STUD7);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_7CS_HI_LO:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_STUD7_HI_LO);
				return TRUE;	// We DID process this message.
			case IDM_SHOW_1ON1:
				SelectGameTypeTab(hDlg, DISPLAY_TAB_ONE_ON_ONE);
				return TRUE;	// We DID process this message.
			case IDM_GOTO_TABLE1:
				if (!Defaults.keep_cardroom_window_open) {
					ShowWindow(hCardRoomDlg, SW_MINIMIZE);
				}
				ShowTableWindow(Table[0].table_serial_number);
				return TRUE;	// We DID process this message.
			case IDM_GOTO_TABLE2:
				if (!Defaults.keep_cardroom_window_open) {
					ShowWindow(hCardRoomDlg, SW_MINIMIZE);
				}
				ShowTableWindow(Table[1].table_serial_number);
				return TRUE;	// We DID process this message.
			case IDM_GOTO_TABLE3:
				if (!Defaults.keep_cardroom_window_open) {
					ShowWindow(hCardRoomDlg, SW_MINIMIZE);
				}
				ShowTableWindow(Table[2].table_serial_number);
				return TRUE;	// We DID process this message.
			case IDM_GOTO_TABLE4:
				if (!Defaults.keep_cardroom_window_open) {
					ShowWindow(hCardRoomDlg, SW_MINIMIZE);
				}
				ShowTableWindow(Table[3].table_serial_number);
				return TRUE;	// We DID process this message.
		  #if 0	//19990819MB
			case IDC_TABLE_INFO:
				// message for the table info window
				kp(("%s(%d) Got WM_COMMAND: IDC_TABLE_INFO\n",_FL));
				if (HIWORD(wParam)==EN_VSCROLL) {	// notify code = VSCROLL?
					kp(("%s(%d) got EN_VSCROLL for IDC_TABLE_INFO\n",_FL));
					InvalidateRect((HWND)lParam, NULL, TRUE);
				}
				return TRUE;	// We DID process this message.
		  #endif
		  #if ADMIN_CLIENT
			case IDC_RECONNECT:
				// This is a complete kludge of a debug feature...
				// we want it simulate a network problem as closely
				// as possible, so let's just close our socket.
				{
					EnterCriticalSection(&ClientSockCritSec);
					extern ClientSocket *ClientSock;
					if (ClientSock) {
						ClientSock->CloseSocket();
					}
					LeaveCriticalSection(&ClientSockCritSec);
				}
				return TRUE;	// We DID process this message.
			case IDD_ADMIN_STATS:
				OpenAdminStatsWindow();
				return TRUE;	// We DID process this message.
			case IDD_CHAT_MONITOR:
				if (!hChatMonitorDlg) {
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHAT_MONITOR), NULL, dlgFuncChatMonitor);
				} else {
					ShowWindow(hChatMonitorDlg, SW_SHOWNORMAL);
					ReallySetForegroundWindow(hChatMonitorDlg);
				}
				return TRUE;	// We DID process this message.
			case IDC_BROADCAST_MESSAGE:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					dwBroadcastDestPlayerID = 0;
					DialogBox(hInst, MAKEINTRESOURCE(IDD_BROADCAST_MESSAGE), NULL, dlgFuncEnterBroadcastMessage);
				}
				return TRUE;	// We DID process this message.
			case IDM_EDIT_USER_ACCOUNTS:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					OpenAdminEditAccountDialog();
				}
				return TRUE;	// We DID process this message.
			case IDC_PAUSE_SENDING:
				dwEarliestPacketSendTime = max(SecondCounter, dwEarliestPacketSendTime) + 5;
				return TRUE;	// We DID process this message.
			case IDM_CHANGE_ALLINS:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_GLOBAL_ALLINS), NULL, dlgFuncChangeAllIns);
				}
				return TRUE;	// We DID process this message.
			case IDM_CHANGE_SHOTCLOCK:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
//					DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_SHOTCLOCK), NULL, dlgFuncChangeShotClock);
				}
				return TRUE;	// We DID process this message.
			case IDM_START_CHECK_RUN:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					if (!BadTimeForIntensiveServerRequest("I don't think so...", "Any time between h:55 and h:10 is a bad time for a check run!\n")) {	// returns TRUE if we should abort
						struct MiscClientMessage mcm;
						zstruct(mcm);
						mcm.message_type = MISC_MESSAGE_START_CHECK_RUN;
						SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));
						MessageBox(NULL,
								"A 'begin check run' request has been sent to the\n"
								"server.  You should get a list of pending checks\n"
								"displayed momentarily.",
								"Check Run Started...",
								MB_OK|MB_TOPMOST);
					}
				}
				return TRUE;	// We DID process this message.
		  #endif
			case IDM_CHANGE_EMAIL:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
				}
				return TRUE;	// We DID process this message.
			case IDM_CHANGE_ADDRESS:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					if (!hUpdateAddressDlg) {
						DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_EDIT_ADDRESS), hCardRoomDlg, dlgFuncUpdateAddressInfo);
					} else {
						ReallySetForegroundWindow(hUpdateAddressDlg);
					}
				}
				return TRUE;	// We DID process this message.
			case IDM_CHANGE_PASSWORD:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_PASSWORD), hDlg, dlgFuncChangePassword);
				}
				return TRUE;	// We DID process this message.

			/*case IDM_RETRIEVE_PASSW:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_RETRIEVE_PASSW), hDlg, dlgFuncRetrievePassword);
				return TRUE; */
            
			case IDM_ENABLE_4COLOR_DECK:
				Defaults.i4ColorDeckEnabled ^= TRUE;
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
				CheckMenuItem(GetMenu(hCardRoomDlg), IDM_ENABLE_4COLOR_DECK, Defaults.i4ColorDeckEnabled ? MF_CHECKED : MF_UNCHECKED);
				return TRUE;	// We DID process this message.
			//Robert 03/01/2002
			case IDM_TABLE_THEMES:
				//LaunchInternetBrowser("http://www.e-mediasoftware.com/themes.php");
				return TRUE;
			//End Robert
			case IDM_ENABLE_ANIMATION:
				Defaults.iAnimationDisabled ^= TRUE;
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
				CheckMenuItem(GetMenu(hCardRoomDlg), IDM_ENABLE_ANIMATION, Defaults.iAnimationDisabled ? MF_UNCHECKED : MF_CHECKED);
				return TRUE;	// We DID process this message.
			case IDM_ENABLE_SOUNDS:
				Defaults.iSoundsDisabled ^= TRUE;
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
				CheckMenuItem(GetMenu(hCardRoomDlg), IDM_ENABLE_SOUNDS, Defaults.iSoundsDisabled ? MF_UNCHECKED : MF_CHECKED);
				return TRUE;	// We DID process this message.
			case IDM_ENABLE_BAR_SNACKS:
				Defaults.iBarSnacksDisabled ^= TRUE;
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
				CheckMenuItem(GetMenu(hCardRoomDlg), IDM_ENABLE_BAR_SNACKS, Defaults.iBarSnacksDisabled ? MF_UNCHECKED : MF_CHECKED);
				return TRUE;	// We DID process this message.
			case IDM_OPTIMIZE_GRAPHICS:
				Defaults.iDisableGraphicOptimizations ^= TRUE;
				iNVidiaBugWorkaround = Defaults.iDisableGraphicOptimizations;
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
//				CheckMenuItem(GetMenu(hCardRoomDlg), IDM_OPTIMIZE_GRAPHICS, Defaults.iDisableGraphicOptimizations ? MF_UNCHECKED : MF_CHECKED);
//*** change 11/09/01 for remove "optimize graohics" item.
				MessageBox(hDlg, "This menu options allows you to turn some graphics\n"
								 "optimizations off to work around some video driver\n"
								 "bugs which cause graphics display problems on some\n"
								 "video cards.\n"
								 "\n"
								 "You must exit and restart the client for\n"
								 "this change to take effect.",
								 "Graphics optimization change",
								 MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_APPLMODAL);
				return TRUE;	// We DID process this message.
			case IDM_REQUEST_ALLIN_RESET:
				// verify we're logged in before we can do this
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in.");
				}
				if (LoggedIn == LOGIN_VALID) {
				  #if 1	//20000317MB: don't top cardroom if called from elsewhere
					HandleAllInResetRequest(NULL);
				  #else
					HandleAllInResetRequest(hDlg);
				  #endif
				}
				return TRUE;	// We DID process this message.
			case IDC_REQUEST_HAND_HISTORY:
				if (LoggedIn != LOGIN_VALID) {
					LogInToServer("You must be logged in before requesting a hand history");
				}
				// we're logged in? -- handle it
				if (LoggedIn == LOGIN_VALID) {
					if (hReqHandHistory) {
						ReallySetForegroundWindow(hReqHandHistory);
					} else {
						if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {
							MessageBox(NULL,
									"The email address we have on file for you has not been validated.\n\n"
									"If you wish to receive hand histories, please go to the Options\n"
									"menu and select 'Change/Validate Email Address'.",
									"Email Address not validated...",
									MB_OK|MB_ICONINFORMATION|MB_TOPMOST);
						} else {
							CreateDialog(hInst, MAKEINTRESOURCE(IDD_REQ_HAND_HISTORY), NULL, dlgFuncRequestHandHistory);
						}
					}
				}
				return TRUE;	// We DID process this message.
			}
		}
		break;

	case WM_CTLCOLORBTN:
		// This message is sent to all buttons before drawing.  Return a 
		// null background brush so that we don't get grey drawn just before
		// our bitmap is going to be drawn.
		//kp(("%s(%d) Got WM_CTLCOLORBTN\n",_FL));
		return ((int)GetStockObject(NULL_BRUSH));

	case WM_CTLCOLORSTATIC:
		{
			// hdcStatic = (HDC) wParam;   // handle to display context 
			// hwndStatic = (HWND) lParam; // handle to static control 
			// chat_hwnd = GetDlgItem(Table[table_index].hwnd, IDC_CHAT_BOX);
			HWND hwnd = (HWND)lParam;
			if (hCardRoomBgnd && hwnd == GetDlgItem(hDlg, IDC_TABLE_INFO) ||
								 hwnd == GetDlgItem(hDlg, IDC_TABLE_INFO_HEADER) ||
								 hwnd == GetDlgItem(hDlg, IDC_USER_COUNT) ||
								 hwnd == GetDlgItem(hDlg, IDC_SHOT_CLOCK_MESSAGE) ||
								 hwnd == GetDlgItem(hDlg, IDC_SHOT_CLOCK_ETA)) {
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
				BlitBitmapRectToDC(hCardRoomBgnd, hdc, &pt, &r);
				SetTextColor(hdc, RGB(255,255,255));
				SetBkMode(hdc, TRANSPARENT);
				return ((int)GetStockObject(NULL_BRUSH));
			}
		}
		break;
	case WM_DRAWITEM:
		// Owner draw control... draw it.
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			int bitmap_index = 0;
			switch (dis->CtlID) {
			case IDC_GOTO_GAME:
				bitmap_index = MISCBITMAP_GOTO_TABLE_UP;
				break;
			case IDC_JOIN_WAIT_LIST:
				bitmap_index = MISCBITMAP_WAITLIST_UP;
				break;
			case IDC_CASHIER:
				  bitmap_index = MISCBITMAP_CASHIER_UP;
				break;
            case IDC_LINK:
				  bitmap_index = MISCBITMAP_LINK_LOBBY_NORMAL;
				break;
			
			}
			if (dis->hwndItem==CardRoomHighlightList.hCurrentHighlight) {
				// The mouse is over this button...
				bitmap_index++;
			}
			//kp(("%s(%d) bitmap_index = %d\n", _FL, bitmap_index));
/*		static COLORREF color_array[4] = {
				RGB(249,241,202),	// normal text
				RGB(115,20,24),		// normal shadow
			  #if 1	//20000627MB: colors changed because proximity highlighting
			  		// now relies on being allowed to disable buttons.
				RGB(249,241,202),	// disabled text
				RGB(115,20,24),		// disabled shadow			 
			 #else
				RGB(169,160,140),	// disabled text
				RGB(115,20,24)		// disabled shadow
			  #endif 
			};
*/

		//Aqui se cambia el color de fondo de las letras de los botones del lobby      J Fonseca   13/04/2004
		static COLORREF color_array[4] = {
				RGB(249,241,202),	// normal text
				RGB(0,0,85),		// normal shadow
			  #if 1	//20000627MB: colors changed because proximity highlighting
			  		// now relies on being allowed to disable buttons.
				RGB(249,241,202),	// disabled text
				RGB(0,0,85),		// disabled shadow			 
			 #else
				RGB(169,160,140),	// disabled text
				RGB(0,0,85)		// disabled shadow
			  #endif 
		};
  

			  DrawButtonItemWithBitmapAndText(NULL, dis, bitmap_index, hCardRoomButtonFont, color_array);
			
		}
		return TRUE;	// TRUE = we did process this message.
	case WM_MOUSEMOVE:
		UpdateMouseProximityHighlighting(&CardRoomHighlightList, hDlg, MAKEPOINTS(lParam));
		return TRUE;	// TRUE = we did process this message.

	case WM_NOTIFY:
		{
			NMHDR *nmhdr = (LPNMHDR)lParam;

			// Check for user clicking on our tabs...
			if (nmhdr->idFrom==IDC_GAME_TYPE_TAB && nmhdr->code==TCN_SELCHANGE) {
				// A new tab was selected on our game type tab control.
				pr(("%s(%d) got TCN_SELCHANGE for game type tab control. New tab = %d\n",_FL,TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_GAME_TYPE_TAB))));
				int tab_index = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_GAME_TYPE_TAB));
				SelectGameTypeTab(hDlg, GameTypeTabOrders[tab_index]);
			}

			// Check for user clicking on list view items...
			if (nmhdr->hwndFrom==hGameListCtrl) {	// it's from our list view control...
				switch (nmhdr->code) {
				case LVN_COLUMNCLICK:
					{
						// One of the column headers was clicked.
						int column_index = ((LPNMLISTVIEW)lParam)->iSubItem;
						pr(("%s(%d) Column %d was clicked.\n",_FL, column_index));
						if (column_index >= 0 && column_index < MAX_TABLE_LIST_COLUMNS) {
							int new_sort_key = CurrentColumnList[column_index];
							if (new_sort_key == iGameListSortKey) {	// same as before?
								iGameListSortDir ^= 1;	// invert sort direction.
							  #if INCLUDE_TABLE_RAKE_PER_HOUR
								if (!iGameListSortDir && new_sort_key == TLC_RAKE_PER_HOUR) {
									iRakePerHourType ^= 1;
									ChangeRakePerHourColumn(iRakePerHourType);
								  #if 0	//20000817MB: crashes... didn't bother to figure out why
									SelectGameTypeTab(hDlg, GameListTabIndex);
								  #else
                                    UpdateGameListViewItems();
								  #endif
								}
							  #endif
							  #if ADMIN_CLIENT
								if (!iGameListSortDir && new_sort_key == TLC_HANDS_PER_HOUR) {
									iHandsPerHourType ^= 1;
									ChangeHandsPerHourColumn(iHandsPerHourType);
                                    UpdateGameListViewItems();
								}
							  #endif
							} else {
								iGameListSortDir = 0;	// sort ascending
								iGameListSortKey = new_sort_key;
							}
							SortTableList();
						}
					}
					break;
				case LVN_ITEMCHANGED:
					pr(("%s(%d) Item %d has changed. New state = %d\n", _FL,
							((LPNMLISTVIEW)lParam)->iItem,
							((LPNMLISTVIEW)lParam)->uNewState));
					if (((LPNMLISTVIEW)lParam)->uNewState & LVIS_FOCUSED) {
						// This item received the focus.
						//kp(("%s(%d) Table %d is now the focus.\n",_FL,((LPNMLISTVIEW)lParam)->lParam));
						dwTableInfoSerialNum = ((LPNMLISTVIEW)lParam)->lParam;
						UpdateTableInfoItems();		// redraw the info items for new table.
						UpdateCardRoomButtons();	// change the various buttons to reflect current selection

						// Request a new copy of the info from the server and subscribe to it.
						pr(("%s(%d) LVN_ITEMCHANGED: new focus is table %d.  iDisableRequestGameInfo=%d\n", _FL, dwTableInfoSerialNum,iDisableRequestGameInfo));
						if (!iDisableRequestGameInfo) {
							pr(("%s(%d) Calling RequestTableInfo(%d)\n",_FL,dwTableInfoSerialNum));
							RequestTableInfo(dwTableInfoSerialNum, TRUE);
						}
					}
					break;
				case NM_DBLCLK:
					// An item as double clicked.  Simulate pressing the
					// go to table' button.
					PostMessage(hDlg, WM_COMMAND, IDC_GOTO_GAME, 0);
					break;
				}
			}
		}
		break;
	case WM_DESTROY:
		// Save this window position...
		pr(("%s(%d) WM_DESTROY\n",_FL));
		RemoveKeyboardTranslateHwnd(hDlg);
		KillTimer(hDlg, WM_TIMER);	// remove our timer.
		WinStoreWindowPos(ProgramRegistryPrefix, CardRoomWindowPosName, hDlg, NULL);
		if (hGameListCtrl) {
			DestroyWindow(hGameListCtrl);
			hGameListCtrl = NULL;
		}
		CloseToolTipWindow(hCardRoomToolTips);
		hCardRoomToolTips = NULL;
		hCardRoomDlg = NULL;
		PostQuitMessage(0);
		break;
	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPLACEMENT wp;
			zstruct(wp);
			wp.length = sizeof(wp);
			GetWindowPlacement(hDlg, &wp);
			//kp(("%s(%d) Cardroom wp.showCmd = %d\n", _FL, wp.showCmd));
			int old_min_flag = iCardRoomDlgMinimizedFlag;
			if (wp.showCmd == SW_HIDE ||
				wp.showCmd == SW_MINIMIZE ||
				wp.showCmd == SW_SHOWMINIMIZED) {
				iCardRoomDlgMinimizedFlag = TRUE;
			} else {
				iCardRoomDlgMinimizedFlag = FALSE;
			}
			pr(("%s(%d) WM_WINDOWPOSCHANGED.  new MinimizedFlag = %d\n", _FL, iCardRoomDlgMinimizedFlag));
			if (old_min_flag && !iCardRoomDlgMinimizedFlag) {
				// Always check to see if we need to refresh the game
				// list if we're no longer minimized.
				CheckForNewGameList(GameListTabIndex);
				FlagRedrawCardRoom();	// always force a refresh
			}
		}
		return TRUE;	// TRUE = we did process this message.
	case WM_PAINT:
		// If we've got a background image, paint it first.
		{

			if (LoggedInPrivLevel >= 40 ) 
			{
			  ShowWindow(GetDlgItem(hDlg, IDC_PAUSE_SENDING), SW_SHOW);
			}
			else
			{
			  ShowWindow(GetDlgItem(hDlg, IDC_PAUSE_SENDING), SW_HIDE);
			}
 
            #if FOR_COMPANY_ONLY
                        
            static AddMenu=0;
			if ( AddMenu == 0 ) {
			
				if ( LoggedInPrivLevel >= 40  ) {     // ACCPRIV_ADMINISTRATOR = 40
               	AddMenu++;
 
				HMENU hm = GetMenu(hCardRoomDlg);	// get HMENU for menu bar
				
				hm = GetSubMenu(hm, 2);	// get 1nd menu off menu bar
					if (hm) {
						MENUITEMINFO mii;
						zstruct(mii);
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID|MIIM_SUBMENU;
						mii.fState = MFS_ENABLED;
						mii.hSubMenu = hm;
						int pos = 13 ;

						mii.wID = 0;
						mii.dwTypeData = "Administrator";
						InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							
						hm = GetSubMenu(hm, 12);	// get 1nd menu off menu bar
						if (hm) {
							MENUITEMINFO mii;
							zstruct(mii);
							mii.cbSize = sizeof(mii);
							mii.fMask = MIIM_TYPE|MIIM_STATE|MIIM_ID;
							mii.fState = MFS_ENABLED;
							int pos = 0 ;
							
							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.fType = MFT_STRING;

							mii.wID = (WORD)IDD_CHAT_MONITOR;
							mii.dwTypeData = "Admin: Chat Monitor";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDM_EDIT_USER_ACCOUNTS;
							mii.dwTypeData = "Admin: Edit &User Accounts";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDC_BROADCAST_MESSAGE;
							mii.dwTypeData = "Admin: Send Broadcast Message";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);

							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.fType = MFT_STRING;

							mii.wID = (WORD)IDM_CHANGE_ALLINS;
							mii.dwTypeData = "Admin: Change Global All-Ins";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDM_CHANGE_SHOTCLOCK;
							mii.dwTypeData = "Admin: Change Shot Clock Message";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDD_ADMIN_STATS;
							mii.dwTypeData = "Admin: Show server stats";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDM_START_CHECK_RUN;
							mii.dwTypeData = "Admin: Start check run (harmless)";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.wID = (WORD)IDC_RECONNECT;
							mii.dwTypeData = "Debug: reconnect";
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);

							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(hm, pos++, MF_BYPOSITION, &mii);
							mii.fType = MFT_STRING;

						}
					}
				}
			}

           #endif



			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hDlg, &ps);
			if (hdc && !IsIconic(hDlg) && hCardRoomBgnd) {
				if (hCardRoomPalette) {
					SelectPalette(hdc, hCardRoomPalette, FALSE);
					RealizePalette(hdc);
				}
				BlitBitmapToDC_nt(hCardRoomBgnd, hdc);
			  #if 0	// test the DarkenRect() function
				RECT r =  {10,10,100,100};
				DarkenRect(hdc, &r);
			  #endif
			}

		
			EndPaint(hDlg, &ps);
		}
		break;
	case WM_QUERYNEWPALETTE:
	case WM_PALETTECHANGED:	// palette has changed.
		//kp(("%s(%d) Table got WM_PALETTECHANGED\n", _FL));
	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us
		    HDC hdc = GetDC(hDlg);
		    HPALETTE holdpal = SelectPalette(hdc, hCardRoomPalette, FALSE);
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
	  #if 0	//19990819MB
		kp(("%s(%d) Got WM_ERASEBKGND for window $%08lx (hGameListCtrl=$%08lx)\n",_FL,hDlg,hGameListCtrl));
		if (hDlg==hGameListCtrl)
		{
			kp(("%s(%d) Got WM_ERASEBKGND for game list control\n",_FL));
			if (hCardRoomBgnd) {
				// Drawing the game list control... draw bitmapped background
				RECT r;
				GetWindowRect(hDlg, &r);
				ScreenToClient(GetParent(hDlg), &r);
				// Blit this rect from our background into the display area.
				POINT pt;
				zstruct(pt);
				BlitBitmapRectToDC(hCardRoomBgnd, (HDC)wParam, &pt, &r);
			}
			return TRUE;	// TRUE = we DID process this message.
		}
	  #endif
		if (IsIconic(hDlg) || !hCardRoomBgnd) {
			return FALSE;	// FALSE = we did NOT process this message.
		}
		return TRUE;	// TRUE = we DID process this message.
	case WM_TIMER:
	  #if ADMIN_CLIENT
		if (AutoJoinDefaultTable) {
			static DWORD last_test_time;
			if (SecondCounter >= last_test_time + 5) {	// not too often...
				last_test_time = SecondCounter;
				// Debug: auto-join a table if we aren't joined to any
				// First, we must request a new table list.  When it is received,
				// we'll automatically be joined by that code.
				int table_count = 0;
				for (int i=0 ; i<MAX_TABLES ; i++) {
					if (Table[i].table_serial_number) {
						table_count++;	// we're joined to this one.
					}
				}
				if (!table_count) {
					CheckForNewGameList(GameListTabIndex);	// ask server for the most recent data if necessary
				}
			}
		}
	  #endif	// ADMIN_CLIENT
		UpdateShotClockText();	// update the text for the shot clock if necessary
		UpdateMenuBlinks();		// update any menu items we're blinking.
	  #if 0	//19990907MB
		kp1(("%s(%d) Playing a sound every WM_TIMER\n",_FL));
		PPlaySound(SOUND_CHIPS_BETTING);
	  #endif
	  #if 0
	  	kp1((ANSI_WHITE_ON_GREEN"%s(%d) WARNING: entering comm critsec and sleeping from WM_TIMER to look for bugs!\n", _FL));
		Sleep(200);
		//kp(("%s(%d) Critical sections owned by this thread at WM_TIMER:\n",_FL));
		//DisplayOwnedCriticalSections();
		EnterCriticalSection(&CardRoomVarsCritSec);
		EnterCriticalSection(&ClientSockCritSec);
		Sleep(100);
		LeaveCriticalSection(&ClientSockCritSec);
		LeaveCriticalSection(&CardRoomVarsCritSec);
		Sleep(100);
	  #endif
		return TRUE;	// TRUE = we DID process this message.
	case WM_SIZING:
		WinSnapWhileSizing(hDlg, message, wParam, lParam);
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	case WMP_CLOSE_YOURSELF:	// application defined message: close our dialog box
		{

			// we're logged in? -- handle it
		    if ( !WaitListButtonState) {
				UnjoinWaitListDialog(hDlg, dwTableInfoSerialNum, 0);
			}
        
			pr(("%s(%d) Got WMP_CLOSE_YOURSELF ($%04x)... calling DestroyWindow()\n",_FL, WMP_CLOSE_YOURSELF));

		  #if ADMIN_CLIENT
			// Debug only: If the ctrl key is pressed, send a message to
			// the server as well which asks it to shut		down all clients and
			// itself.
			int shutdown_flag = FALSE;
			if (iAdminClientFlag && (GetKeyState(VK_MENU) & 0x80)) {
				// exit program immediately as if we got terminated by the task
				// manager.  This is used for simulating client crashes and stuff.
				_exit(0);
			}
			if (iAdminClientFlag && (GetKeyState(VK_CONTROL) & 0x80)) {
				// Send a DATATYPE_SHUTDOWN_MSG packet.
				struct ShutDownMsg sdm;
				zstruct(sdm);
				SendDataStructure(DATATYPE_SHUTDOWN_MSG, &sdm, sizeof(sdm));
				wParam = TRUE;	// don't leave each individual table.
				shutdown_flag = TRUE;
			}
		  #endif
			// If any tables are open, close them first.
			// wParam is set to TRUE if we want to bypass this code.
			if (!wParam) {
				int keep_shutting_down = LeaveTables();
				if (!keep_shutting_down) {
					return TRUE;	// we handled this message
				}
			}
			// Exiting was not cancelled while closing tables, so continue
			// with exiting.
			ShowWindow(hDlg, SW_HIDE);
		  #if ADMIN_CLIENT
			if (!shutdown_flag) {
				PromptUserToTellTheirFriends();
			}
		  #else
			PromptUserToTellTheirFriends();
		  #endif
			PromptUserToTellTheirFriends();
			DestroyWindow(hDlg);			// destroy ourselves.
			DestroyWindow(hInitialWindow);	// close down our parent window
			if (hCardRoomBgnd) {			// Delete background image if loaded.
				DeleteObject(hCardRoomBgnd);
				hCardRoomBgnd = NULL;
			}
		  #if ADMIN_CLIENT
			if (shutdown_flag) {
				Sleep(1000);	// give shutdown request time to be sent out.
			}
		  #endif
			pr(("%s(%d) Back from DestroyWindow().\n",_FL));
		}
		return TRUE;	// TRUE = we did process this message.
  #if ADMIN_CLIENT
	case WMP_INFO_BLOCK_ARRIVED:	// admin only: an information block has arrived
		ProcessNewAdminInfoBlock();
		return TRUE;
	
	case WMP_FORCE_REFRESH_MONITOR:
		RefreshMonitorChatText(TRUE);
		return TRUE;	// TRUE = we did process this message.

	case WMP_REFRESH_MONITOR_CHAT:
		RefreshMonitorChatText();
		return TRUE;	// TRUE = we did process this message.
  #endif
	case WMP_SHOW_YOURSELF:
		ShowWindow(hDlg, SW_SHOWNORMAL);
		return TRUE;	// TRUE = we did process this message.
	case WMP_UPDATE_GAME_LIST:
		pr(("%s(%d) got WMP_UPDATE_GAME_LIST for game type %d\n", _FL, lParam));
		if (lParam==GameListTabIndex) {// are we displaying this game type?
			UpdateShotClock();
			UpdateGameListViewItems();
			UpdateCardRoomButtons();	// change the various buttons to reflect current selection
		}
		return TRUE;	// TRUE = we did process this message.
	case WMP_UPDATE_YOURSELF:
		// Special case used when we come back from being minimized.
		iCardRoomRedrawNeeded = FALSE;
		UpdateGameListViewItems();
		UpdateCardRoomButtons();	// change the various buttons to reflect current selection
		UpdateTableInfoItems();
		UpdateShotClock();
		return TRUE;	// TRUE = we did process this message.
	case WMP_UPDATE_TABLE_INFO:	// a table info structure was updated.
		if (lParam==(LPARAM)dwTableInfoSerialNum) {
			UpdateTableInfoItems();
		}
		return TRUE;	// TRUE = we did process this message.
	case WMP_SEAT_AVAIL:
		UpdateSeatAvailDialog(PickWindowForRandomMessage(hDlg));
		return TRUE;	// TRUE = we did process this message.
	case WMP_MISC_CLIENT_MSG:
		// If there is a table active, make the table the owner of the message
		// rather than the lobby.  If the lobby pops up too much while playing, it's
		// really annoying.
		HWND parent_window = PickWindowForRandomMessage(hDlg);
		// we might want to force a parent window for this misc message to display
		if (CardRoom_misc_client_msg.message_type == MISC_MESSAGE_TOURNAMENT_SHUTDOWN_NOTE) {
			parent_window = hCardRoomDlg;
		}
		ShowMiscClientMsg(parent_window, &CardRoom_misc_client_msg, NULL);
		return TRUE;	// TRUE = we did process this message.
  #if 0	//19991102MB
	case WMP_PROMPT_JOIN_WAITLIST:
		// Prompt the user to join a waiting list.  wParam contains
		// the table_serial_num for the default table.
		PromptUserToJoinWaitList(wParam);
		return TRUE;	// TRUE = we did process this message.
  #endif
	}
    return FALSE;	// We did NOT process this message.
}

//*********************************************************
////
// Open the cardroom dialog window
//
void OpenCardRoom(void)
{
  #if 1	//19990823MB
	// Display a warning if this is a palette display mode
	HDC hdc = GetDC(NULL);	// get DC for primary display
	int raster_caps = GetDeviceCaps(hdc, RASTERCAPS);
	ReleaseDC(NULL, hdc);
	if (raster_caps & RC_PALETTE) {	// palette device?
		MessageBox(NULL,
					"This program was designed for displays of 32768 or more colors (16-bit).\n"
					"\n"
					"You are currenly running in a display mode with 256 or fewer colors.\n\n"
					"The program will work, but there may be graphical corruption and\n"
					"awkward pauses in game play.\n"
					"\n"
					"We advise you to switch to a screen display mode with more colors (16-bit,\n"
					"24-bit, or 32-bit) and then restart this program.",
					"Display mode problem...",
					MB_OK|MB_APPLMODAL|MB_TOPMOST);
	}
  #endif

	// Load the background pic
	if (!hCardRoomBgnd
		  #if ADMIN_CLIENT
			&& !RunningManyFlag
		  #endif
	) {
		// Load the palette for the bgnd pic if necessary
		hCardRoomPalette = LoadPaletteIfNecessary(FindFile("cardroom.act"));
		//hCardRoomPalette = LoadPaletteIfNecessary(FindFile("cc.pla"));
		// hCardRoomBgnd = LoadJpegAsBitmap(FindFile("cardroom.jpg"), hCardRoomPalette);
		hCardRoomBgnd = LoadJpegAsBitmap(FindMediaFile("media\\src_lobby.jpg"), hCardRoomPalette);
	}

	LoadLobbyBitmaps(hCardRoomPalette);

	if (!hCardRoomButtonFont) {
		hCardRoomButtonFont = CreateFont(
        
		 //		16,	//  int nHeight,             // logical height of font
		        14,	//  int nHeight,             // logical height of font
				0,	//  int nWidth,              // logical average character width
				0,	//  int nEscapement,         // angle of escapement
				0,	//  int nOrientation,        // base-line orientation angle
				FW_NORMAL,//  int fnWeight,            // font weight
				//FW_BOLD, //  int fnWeight,            // font weight
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

	// Only pick a shot clock font when in large fonts mode.
	if (!hShotClockFont && iLargeFontsMode) {
		hShotClockFont = CreateFont(
				13,	//  int nHeight,             // logical height of font
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
			  #if 1	//20000628MB
				DEFAULT_PITCH|FF_SWISS,	//  DWORD fdwPitchAndFamily,  // pitch and family
				NULL
			  #else
				DEFAULT_PITCH|FF_SWISS,	//  DWORD fdwPitchAndFamily,  // pitch and family
				"MS Sans Serif"	//  LPCTSTR lpszFace         // pointer to typeface name string
			  #endif
		);
		//kp(("%s(%d) hShotClockFont = $%08lx\n", _FL, hShotClockFont));
	}

	// Display the dialog box
	if (!hCardRoomDlg) {
		zstruct(CardRoomHighlightList);
	  #if 0		// Should we have a parent?
		CreateDialog(hInst, MAKEINTRESOURCE(IDD_CARDROOM),
							hInitialWindow, dlgFuncCardRoom);
	  #else
		CreateDialog(hInst, MAKEINTRESOURCE(IDD_CARDROOM),
						  	NULL, dlgFuncCardRoom);
	  #endif
		pr(("%s(%d) hCardRoomDlg = $%08lx GetLastError() = %d\n", _FL, hCardRoomDlg, GetLastError()));
		if (hCardRoomDlg) {
			if (iProgramInstanceCount) {
				// If this is a second instance, restore from a different name.
				char str[10];
				sprintf(str, " %d", iProgramInstanceCount+1);
				strcat(CardRoomWindowPosName, str);
			}
			WinRestoreWindowPos(ProgramRegistryPrefix, CardRoomWindowPosName, hCardRoomDlg, NULL, NULL, FALSE, TRUE);
		  #if 0	//19990819MB: no longer needed
			// modify the system menu -- add an "About" option
			HMENU hMenu = GetSystemMenu(hCardRoomDlg, FALSE);
			AppendMenu(hMenu, MF_SEPARATOR, 0,	0);
			AppendMenu(hMenu, MF_STRING, WMP_SHOW_ABOUT_DLG, "&About...");
		  #endif
			AddProximityHighlight(&CardRoomHighlightList, GetDlgItem(hCardRoomDlg, IDC_GOTO_GAME));
			AddProximityHighlight(&CardRoomHighlightList, GetDlgItem(hCardRoomDlg, IDC_JOIN_WAIT_LIST));
			AddProximityHighlight(&CardRoomHighlightList, GetDlgItem(hCardRoomDlg, IDC_CASHIER));
			//cris 07-23-2003
            AddProximityHighlight(&CardRoomHighlightList, GetDlgItem(hCardRoomDlg, IDC_LINK));
			// end cris 07-23-2003
//*** change 11/09/01 for test version
			// make sure it's on screen in the current screen resolution
			WinPosWindowOnScreen(hCardRoomDlg);
			PostMessage(hInitialWindow, WMP_UPDATE_CONNECT_STATUS, 0, 0);
			if (MinimizeWindowsFlag) {
				ShowWindow(hCardRoomDlg, SW_MINIMIZE);
			} else {
				ShowWindow(hCardRoomDlg, SW_SHOW);
			  #if 0	//20000726MB: has no effect... why?
				BringWindowToTop(hCardRoomDlg);
			  #endif
			}
			SetFocus(hGameListCtrl);	// we'd always like this to be our default focus

			// Create a tooltip control/window for use within this dialog box
			// and add all our tool tips to it.
			hCardRoomToolTips = OpenToolTipWindow(hCardRoomDlg, CardRoomDlgToolTipText);

			iMenuBlinkCount = 4;	// blink the first menu a few times.
		} else {
			Error(ERR_ERROR, "%s(%d) CardRoom dialog failed to open.  GetLastError() = %d\n", _FL, GetLastError());
		}
	}
}

#if ADMIN_CLIENT
/**********************************************************************************
 Function CALLBACK dlgAdminInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 Date: CR01/01/12
 Purpose: handle for admin info block
***********************************************************************************/
BOOL CALLBACK dlgAdminInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		hInfoBlock = hDlg;
		AddKeyboardTranslateHwnd(hDlg);
		WinRestoreWindowPos(ProgramRegistryPrefix, "AdminInfo", hDlg, NULL, NULL, FALSE, TRUE);
		WinPosWindowOnScreen(hDlg);
		// Change the font in the display window to non-proportional
		{
			HWND hwnd = GetDlgItem(hDlg, IDC_ADMIN_INFO_EDIT);
			static HFONT infowindowfont;
			if (!infowindowfont) {
				RECT r;
				zstruct(r);
				GetWindowRect(hwnd, &r);
				infowindowfont = CreateFont(
					11,//  int nHeight,             // logical height of font
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
					"Lucida Console" //  LPCTSTR lpszFace         // pointer to typeface name string
				);
			}
			if (infowindowfont) {
				SendMessage(hwnd, WM_SETFONT, (WPARAM)infowindowfont, 0);
			}

		}
		ShowWindow(hDlg, SW_SHOW);
		PostMessage(hInfoBlock, WMP_UPDATE_YOURSELF, 0, 0);
		return FALSE;

	case WMP_UPDATE_YOURSELF:
		SetDlgItemTextIfNecessary(hDlg, IDC_ADMIN_INFO_EDIT, AdminInfo.info);
		// Make sure it's put at the top...
		SetWindowPos(hDlg, HWND_TOPMOST,   0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		return TRUE;
	
	case WM_DESTROY:
		WinStoreWindowPos(ProgramRegistryPrefix, "AdminInfo", hDlg, NULL);
		hInfoBlock = NULL;
		RemoveKeyboardTranslateHwnd(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			DestroyWindow(hDlg);
		    return TRUE;	// message was processed
		}
		break;
	case WM_SIZING:
		WinSnapWhileSizing(hDlg, message, wParam, lParam);
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	}
	NOTUSED(wParam);
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}
#endif

/**********************************************************************************
 Function CALLBACK dlgAboutFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 Date: CR1999/06/24
 Purpose: handler for About box
***********************************************************************************/
BOOL CALLBACK dlgAboutBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char str[120];
	zstruct(str);
	switch (message) {
	case WM_INITDIALOG:
		// window title
		AddKeyboardTranslateHwnd(hDlg);
		SetWindowTextIfNecessary(hDlg,"About https://github.com/kriskoinclient...");
		// build info
		sprintf(str,"Compiled: %s - %s", __DATE__,__TIME__);
		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_DATETIME, str);
		// client version

		
		
		sprintf(str,"Client version %d.%02d (build %d)",
			ClientVersionNumber.major,ClientVersionNumber.minor,
			ClientVersionNumber.build & 0x0000FFFF);
		

        /*
		sprintf(str,"Client version %d.%01d (build %d)",
			2,0,
			ClientVersionNumber.build & 0x0000FFFF);
		*/
		
		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT1, str);
		// server version (received from server)
		if (ServerVersionInfo.server_version.build) {
			   
		      /*----------------------------------------------------   
				sprintf(str,"Server version %d.%02d (build %d)\n",
				ServerVersionInfo.server_version.major,
				ServerVersionInfo.server_version.minor,
				ServerVersionInfo.server_version.build & 0x0000FFFF);
              ----------------------------------------------------*/   
		              
				sprintf(str,"Server version %d.%01d (build %d)\n",
				2,
				0,
				ServerVersionInfo.server_version.build & 0x0000FFFF);
        
		} else {
			strcpy(str, "Server version unknown.");
		}
		
		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT2, str);

		// Build up some a string containing our connection data.
		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT3, szConnectionTypeString);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	case WM_DESTROY:
		RemoveKeyboardTranslateHwnd(hDlg);
		return TRUE;
	}
	NOTUSED(lParam);
    return FALSE;
}


/**********************************************************************************
 Function LeaveTables(void)
 Date: CR1999/06/28
 Purpose: in sequence, simulate a LeaveTable push on all tables we're joined to
 Returns: TRUE if we left all tables in the sequence, FALSE if we aborted (hit CANCEL)
***********************************************************************************/
int LeaveTables(void)
{
	RemoveBuyInDLG();	// if we're buying in somewhere, abort it
	for (int i=0 ; i<MAX_TABLES ; i++) {
		if (Table[i].hwnd) {
			int cancel_flag = HandleLeaveTableButton(&Table[i]);
			if (cancel_flag) {
				// The user decided not to exit.
				return FALSE;	// FALSE = abort...
			}
		}
	}
	return TRUE;	// we closed/handled them all
}

#if ADMIN_CLIENT
//*********************************************************
////
// Round a time_t to the nearest n value (in seconds)
//
time_t Nearesttime(time_t t, int rounding_interval_in_seconds)
{
	time_t adjust = t % rounding_interval_in_seconds;
	if (adjust > rounding_interval_in_seconds / 2) {
		adjust -= rounding_interval_in_seconds;
	}
	return t - adjust;
}

//*********************************************************
////
// Convert time_t to SYSTEMTIME and back
//
void Convert_time_t_to_SYSTEMTIME(time_t t, SYSTEMTIME *sys_time)
{
	FILETIME ft;
	zstruct(ft);
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
	zstruct(*sys_time);
	FileTimeToSystemTime(&ft, sys_time);
}	
void Convert_SYSTEMTIME_to_time_t(SYSTEMTIME *sys_time, time_t *t)
{
	FILETIME ft;
	zstruct(ft);
	SystemTimeToFileTime(sys_time, &ft);
    ULONGLONG ll = (ULONGLONG)ft.dwLowDateTime + ((ULONGLONG)ft.dwHighDateTime << 32);
	*t = (time_t)((ll - 116444736000000000) / 10000000);
}

//*********************************************************
////
// Get the date and time pickers on the shot clock update dialog
//
static time_t GetShotClockDlgDate(HWND hDlg)
{
	time_t result = 0;

	SYSTEMTIME systime_date;
	SYSTEMTIME systime_time;
	SYSTEMTIME systime;
	zstruct(systime_time);
	zstruct(systime_date);
	zstruct(systime);
	DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEPICKER), &systime_date);
	DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_TIMEPICKER), &systime_time);
	// Combine the date and time...
	systime = systime_time;
	systime.wYear      = systime_date.wYear;
	systime.wMonth     = systime_date.wMonth;
	systime.wDayOfWeek = systime_date.wDayOfWeek;
	systime.wDay       = systime_date.wDay;
	Convert_SYSTEMTIME_to_time_t(&systime, &result);

	// Adjust by the local time zone so that we get GMT
	TIME_ZONE_INFORMATION tzi;
	zstruct(tzi);
  #if 1	//20000403MB: Windows 2000 doesn't seem to adjust the bias
		// according to DST automatically; we have to do it ourselves.
	DWORD zone = GetTimeZoneInformation(&tzi);
	long bias = tzi.Bias;
	if (zone==TIME_ZONE_ID_DAYLIGHT) {
		bias += tzi.DaylightBias;
		//kp(("%s(%d) DST in effect. Additional bias = %d\n", _FL, tzi.DaylightBias));
	}
  #else
	GetTimeZoneInformation(&tzi);
	long bias = tzi.Bias;
  #endif
	//kp(("%s(%d) bias = %d hours\n", _FL, bias/60));
	result += bias*60;

	return result;
}

//*********************************************************
////
// Update the time zone and time to go examples on the change
// shot clock dialog.
//
void UpdateTimeZoneExamples(HWND hDlg)
{
	char str[200];
	zstruct(str);
	time_t now = time(NULL);
	time_t current_setting = GetShotClockDlgDate(hDlg);
	int seconds_to_go = current_setting - now;
	if (seconds_to_go > 0) {
		ConvertSecondsToString(seconds_to_go,
				str,
				IsDlgButtonChecked(hDlg, IDC_CHKBOX_DISPLAY_SECONDS),
				IsDlgButtonChecked(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS),
			  	9
		);

		struct tm tm;
		struct tm *t;
		time_t temp_t = current_setting - SERVER_TIMEZONE;
		t = gmtime(&temp_t, &tm);
		sprintf(str+strlen(str), "\n%02d:%02d CST", t->tm_hour, t->tm_min);

	  #if DAYLIGHT_SAVINGS_TIME	//20000927MB: daylight savings time
		temp_t = current_setting - 4*3600;
		t = gmtime(&temp_t, &tm);
		sprintf(str+strlen(str), ", %02d:%02d EDT", t->tm_hour, t->tm_min);
	  #else	// standard time
		temp_t = current_setting - 5*3600;
		t = gmtime(&temp_t, &tm);
		sprintf(str+strlen(str), ", %02d:%02d EST", t->tm_hour, t->tm_min);
	  #endif
		temp_t = current_setting;
		t = gmtime(&temp_t, &tm);
		sprintf(str+strlen(str), ", %02d:%02d GMT", t->tm_hour, t->tm_min);

		SetDlgItemTextIfNecessary(hDlg, IDC_TIMEZONE_EXAMPLES, str);
	} else {
		SetDlgItemTextIfNecessary(hDlg, IDC_TIMEZONE_EXAMPLES, "(time expired)");
	}
}

//*********************************************************
////
// Set the date and time pickers on the shot clock update dialog
//
static void SetShotClockDlgDate(HWND hDlg, time_t when)
{
	SYSTEMTIME systime;
	zstruct(systime);
	Convert_time_t_to_SYSTEMTIME(when, &systime);
  #if 0
	time_t test = 0;
	Convert_SYSTEMTIME_to_time_t(&systime, &test);
	kp(("%s(%d) time conversion error = %d (%lu vs %lu)\n", _FL, when - test, when, test));
  #endif
	SYSTEMTIME localtime;
	SystemTimeToTzSpecificLocalTime(NULL, &systime, &localtime);
	DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATEPICKER), GDT_VALID, &localtime);
	DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_TIMEPICKER), GDT_VALID, &localtime);
	UpdateTimeZoneExamples(hDlg);
}

//*********************************************************
////
// Allow the administrator to change the global all-ins
// reset time and total number available.
//
BOOL CALLBACK dlgFuncChangeAllIns(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//kp(("%s(%d) dlg $%08lx got message = %d\n",_FL, hDlg, message));
	switch (message) {
	case WM_INITDIALOG:
		WinRestoreWindowPos(ProgramRegistryPrefix, "ChangeAllIns", hDlg, NULL, NULL, FALSE, TRUE);
		AddKeyboardTranslateHwnd(hDlg);
		//SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Server shutdown/upgrade scheduled");
		SetShotClockDlgDate(hDlg, SavedAccountInfo.all_in_reset_time);
		SendMessage(GetDlgItem(hDlg, IDC_ALLINS_ALLOWED_SPINNER), UDM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELONG(5,0)));
		SendMessage(GetDlgItem(hDlg, IDC_ALLINS_ALLOWED_SPINNER), UDM_SETPOS, (WPARAM)0, (LPARAM)(MAKELONG(SavedAccountInfo.all_ins_allowed,0)));
		SendMessage(GetDlgItem(hDlg, IDC_MAX_GOOD_ALLINS_ALLOWED_SPINNER), UDM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELONG(5,0)));
		SendMessage(GetDlgItem(hDlg, IDC_MAX_GOOD_ALLINS_ALLOWED_SPINNER), UDM_SETPOS, (WPARAM)0, (LPARAM)(MAKELONG(SavedAccountInfo.good_all_ins_allowed_for_auto_reset,0)));
		return TRUE;	// TRUE = set keyboard focus appropriately

	case WM_COMMAND:
		//kp(("%s(%d) dialog $%08lx got WM_COMMAND\n", _FL, hDlg));
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDOK:
				// Read the time and text from the control and send it
				// to the server.  Use the ShotClockUpdate structure
				// with a flag to indicate this is an all-in setting
				struct ShotClockUpdate scu;
				zstruct(scu);
				scu.shotclock_time = GetShotClockDlgDate(hDlg);
				scu.flags |= SCUF_GLOBAL_ALLIN_SETTING;
				scu.misc_value1 = GetDlgTextInt(hDlg, IDC_ALLINS_ALLOWED);
				scu.misc_value2 = GetDlgTextInt(hDlg, IDC_MAX_GOOD_ALLINS_ALLOWED);
				SendDataStructure(DATATYPE_SHOTCLOCK_UPDATE, &scu, sizeof(scu));
				EndDialog(hDlg, 1);
			    return TRUE;	// We DID process this message.
			case IDCANCEL:
				//kp(("%s(%d) IDCANCEL received.\n",_FL));
				EndDialog(hDlg, 0);
			    return TRUE;	// We DID process this message.
			case IDC_SETTIME_NOW:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+2*60, 5*60));
			    return TRUE;	// We DID process this message.
			}
		}
		break;
	case WM_DESTROY:
		WinStoreWindowPos(ProgramRegistryPrefix, "ChangeAllIns", hDlg, NULL);
		RemoveKeyboardTranslateHwnd(hDlg);
	    return TRUE;	// We DID process this message.
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	}
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}

//*********************************************************
////
// Allow the administrator to change the shot clock message.
//
BOOL CALLBACK dlgFuncChangeShotClock(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//kp(("%s(%d) dlg $%08lx got message = %d\n",_FL, hDlg, message));
	switch (message) {
	case WM_INITDIALOG:
		WinRestoreWindowPos(ProgramRegistryPrefix, "ChangeShotClock", hDlg, NULL, NULL, FALSE, TRUE);
		AddKeyboardTranslateHwnd(hDlg);
		SetTimer(hDlg, WM_TIMER, 500, NULL);	// .5 second timer
		//SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Server shutdown/upgrade scheduled");
		{
			time_t new_time = time(NULL);
			if (ShotClockServerDate > new_time) {
				new_time = ShotClockServerDate;
			}
			SetShotClockDlgDate(hDlg, new_time);
		}

		// init the max tournament tables spinner
		#define MAX_MAX_TOURNAMENT_TABLES	50
		{
			HWND hSpinner = GetDlgItem(hDlg, IDC_MAX_TOURN_TABLES_SPINNER);
			SendMessage(hSpinner, UDM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELONG(MAX_MAX_TOURNAMENT_TABLES,1)));
			SendMessage(hSpinner, UDM_SETPOS, (WPARAM)0, (LPARAM)(MAKELONG(iMaxTournamentTables,0)));
		}

		CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES, (ShotClockFlags & SCUF_ANNOUNCE_AT_TABLES)      ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE, (ShotClockFlags & SCUF_SHUTDOWN_WHEN_DONE)      ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,  (ShotClockFlags & SCUF_SHUTDOWN_IS_BRIEF)       ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER, (ShotClockFlags & SCUF_SHUTDOWN_AUTO_INSTALLNEW)? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,(ShotClockFlags & SCUF_CLOSE_CASHIER)		     ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_CASHIER_AUTO_UP_DOWN,(ShotClockFlags & SCUF_ECASH_AUTO_SHUTDOWN)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED,!(ShotClockFlags & SCUF_CLOSE_TOURNAMENTS)  ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED,!(ShotClockFlags & SCUF_NO_TOURNAMENT_SITDOWN)  ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN,(ShotClockFlags & SCUF_TOURNAMENTS_OPEN)    ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS, (ShotClockFlags & SCUF_DISPLAY_SECONDS)     ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,(ShotClockFlags & SCUF_USE_SHORT_UNITS)  ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHKBOX_ALARM, Defaults.iShotClockAlarm ? BST_CHECKED : BST_UNCHECKED);

		if (hShotClockFont) {
			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT1_EXPIRED), WM_SETFONT, (WPARAM)hShotClockFont, 0);
			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2_EXPIRED), WM_SETFONT, (WPARAM)hShotClockFont, 0);
		}
		SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, SavedShotClockMessages[0][0]);
		SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, SavedShotClockMessages[1][0]);
		SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, SavedShotClockMessages[2][0]);
		SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, SavedShotClockMessages[3][0]);
		SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_LIMITTEXT, SHOTCLOCK_MESSAGE_LEN-1, 0L);
		SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2), EM_LIMITTEXT, SHOTCLOCK_MESSAGE_LEN-1, 0L);
		SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT1_EXPIRED), EM_LIMITTEXT, SHOTCLOCK_MESSAGE_LEN-1, 0L);
		SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2_EXPIRED), EM_LIMITTEXT, SHOTCLOCK_MESSAGE_LEN-1, 0L);

		// Fill the combo box with any available saved shot clock messages
		{
			for (int i=0 ; i<SHOTCLOCK_MESSAGES_TO_KEEP ; i++) {
				if (SavedShotClockMessages[0][i][0]) {
			        SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), CB_INSERTSTRING, (unsigned)-1, (LPARAM)SavedShotClockMessages[0][i]);
				}
				if (SavedShotClockMessages[1][i][0]) {
			        SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2), CB_INSERTSTRING, (unsigned)-1, (LPARAM)SavedShotClockMessages[1][i]);
				}
				if (SavedShotClockMessages[2][i][0]) {
			        SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT1_EXPIRED), CB_INSERTSTRING, (unsigned)-1, (LPARAM)SavedShotClockMessages[2][i]);
				}
				if (SavedShotClockMessages[3][i][0]) {
			        SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT2_EXPIRED), CB_INSERTSTRING, (unsigned)-1, (LPARAM)SavedShotClockMessages[3][i]);
				}
			}
		}

		if (LoggedInPrivLevel < ACCPRIV_SUPER_USER) {
			// disable the various shutdown related shotclock things
			EnableWindow(GetDlgItem(hDlg, IDC_SHUTDOWN_WHEN_DONE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_SHUTDOWN_IS_BRIEF), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_RESTART_NEW_SERVER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_MSG_SHUTDOWN), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_MSG_SHUTDOWN2), FALSE);
		}
		if (LoggedInPrivLevel < ACCPRIV_ADMINISTRATOR) {
		  #if 0	//20000924MB
			EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_CLOSE_CASHIER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_CASHIER_AUTO_UP_DOWN), FALSE);
		  #endif
			EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED), FALSE);
		}
		{
			int allowed = TRUE;
			if (!IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED)) {
				allowed = FALSE;
			}
			if (LoggedInPrivLevel < ACCPRIV_ADMINISTRATOR) {
				allowed = FALSE;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED), allowed);
			EnableWindow(GetDlgItem(hDlg, IDC_MAX_TOURN_TABLES), allowed);
			EnableWindow(GetDlgItem(hDlg, IDC_MAX_TOURN_TABLES_SPINNER), allowed);
			
		}
		return TRUE;	// TRUE = set keyboard focus appropriately

	case WM_COMMAND:
		//kp(("%s(%d) dialog $%08lx got WM_COMMAND\n", _FL, hDlg));
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDC_APPLY:
			case IDOK:
				// Read the time and text from the control and send it to the server to be broadcast.
				struct ShotClockUpdate scu;
				zstruct(scu);
				scu.shotclock_time = GetShotClockDlgDate(hDlg);
				if (IsDlgButtonChecked(hDlg, IDC_ANNOUNCE_AT_TABLES)) {
					scu.flags |= SCUF_ANNOUNCE_AT_TABLES;
				}
				if (IsDlgButtonChecked(hDlg, IDC_SHUTDOWN_WHEN_DONE)) {
					scu.flags |= SCUF_SHUTDOWN_WHEN_DONE;
				}
				if (IsDlgButtonChecked(hDlg, IDC_SHUTDOWN_IS_BRIEF)) {
					scu.flags |= SCUF_SHUTDOWN_IS_BRIEF;
				}
				if (IsDlgButtonChecked(hDlg, IDC_RESTART_NEW_SERVER)) {
					scu.flags |= SCUF_SHUTDOWN_AUTO_INSTALLNEW;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_CLOSE_CASHIER)) {
					scu.flags |= SCUF_CLOSE_CASHIER;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_CASHIER_AUTO_UP_DOWN)) {
					scu.flags |= SCUF_ECASH_AUTO_SHUTDOWN;
				}
				if (!IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED)) {
					scu.flags |= SCUF_CLOSE_TOURNAMENTS;
				}
				if (!IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED)) {
					scu.flags |= SCUF_NO_TOURNAMENT_SITDOWN;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_DISPLAY_SECONDS)) {
					scu.flags |= SCUF_DISPLAY_SECONDS;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS)) {
					scu.flags |= SCUF_USE_SHORT_UNITS;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN)) {
					scu.flags |= SCUF_TOURNAMENTS_OPEN;
				}
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENT_EMERGENCY_SHUTDOWN)) {
					scu.flags |= SCUF_EMERGENCY_TOURNAMENT_SHUTDOWN;
				}

				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY)) {
					GetDlgText(hDlg, IDC_MESSAGE_TEXT , scu.shotclock_message1_expired, SHOTCLOCK_MESSAGE_LEN);
					GetDlgText(hDlg, IDC_MESSAGE_TEXT2, scu.shotclock_message2_expired, SHOTCLOCK_MESSAGE_LEN);
					SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, scu.shotclock_message1_expired);
					SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, scu.shotclock_message2_expired);
				}

				//kp(("%s(%d) scu.flags = %02x\n", _FL, scu.flags));
				scu.max_tournament_tables = (WORD16)(max(0,min(MAX_MAX_TOURNAMENT_TABLES,GetDlgTextInt(hDlg, IDC_MAX_TOURN_TABLES))));
				GetDlgText(hDlg, IDC_MESSAGE_TEXT, scu.shotclock_message1, SHOTCLOCK_MESSAGE_LEN);
				GetDlgText(hDlg, IDC_MESSAGE_TEXT2, scu.shotclock_message2, SHOTCLOCK_MESSAGE_LEN);
				GetDlgText(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, scu.shotclock_message1_expired, SHOTCLOCK_MESSAGE_LEN);
				GetDlgText(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, scu.shotclock_message2_expired, SHOTCLOCK_MESSAGE_LEN);

				if (scu.flags & SCUF_EMERGENCY_TOURNAMENT_SHUTDOWN) {
					// They've asked to do an emergency tournament shutdown!  That's unusual
					if (MessageBox(hDlg,
							"Selecting the Emergency Tournamenent Shutdown checkbox\n"
							"will cancel all tournaments currently in progress\n"
							"immediately after the current game finishes and will\n"
							"then issue tournament cancellation emails.\n"
							"\n"
							"Players will receive refunds based on their chip\n"
							"amounts, etc. as well as their fee refund.\n"
							"\n"
							"This should ONLY be done when continuing tournaments\n"
							"will have severe consequences.\n"
							"\n"
							"PLEASE BE SURE THIS IS WHAT YOU WANT TO DO!\n"
							"\n"
							"Do you still wish to send this update?",
							"You will cancel any tournaments in progress!",
							MB_OKCANCEL|MB_DEFBUTTON2) != IDOK)
					{
						// They didn't select "ok".  Don't send it.
						return TRUE;	// we DID process
					}
					scu.flags |=  SCUF_CLOSE_TOURNAMENTS | SCUF_NO_TOURNAMENT_SITDOWN;
					scu.flags &= ~SCUF_TOURNAMENTS_OPEN;
				}
				SendDataStructure(DATATYPE_SHOTCLOCK_UPDATE, &scu, sizeof(scu));
				if (LOWORD(wParam)==IDOK) {
					EndDialog(hDlg, 1);
				}
			    return TRUE;	// We DID process this message.
			case IDCANCEL:
				//kp(("%s(%d) IDCANCEL received.\n",_FL));
				EndDialog(hDlg, 0);
			    return TRUE;	// We DID process this message.
			case IDC_DATE_NOW:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)-90, 60));	// make it already "expired".
			    return TRUE;	// We DID process this message.
			case IDC_DATE_5MIN:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+5*60, 1*60));
			    return TRUE;	// We DID process this message.
			case IDC_DATE_10MIN:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+10*60, 5*60));
			    return TRUE;	// We DID process this message.
			case IDC_DATE_15MIN:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+15*60, 5*60));
			    return TRUE;	// We DID process this message.
			case IDC_DATE_20MIN:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+20*60, 5*60));
			    return TRUE;	// We DID process this message.
			case IDC_DATE_30MIN:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+30*60, 5*60));
			    return TRUE;	// We DID process this message.
			case IDC_DATE_TOP_OF_HOUR:
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+30*60, 60*60));
			    return TRUE;	// We DID process this message.
			case IDC_MSG_SHUTDOWN:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "30 second server shutdown in:");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "%t9");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, "30 second server shutdown");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "now in progress");
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+8*60, 1*60));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_CHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_CHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_CHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_CHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN,BST_UNCHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_SHUTDOWN2:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "30-60 minute server shutdown in:");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "%t9");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, "30-60 minute server shutdown");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "now in progress");
				SetShotClockDlgDate(hDlg, Nearesttime(time(NULL)+20*60, 5*60));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_CHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_CHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN,BST_UNCHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_SERVER_BACK_UP:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Server is back up and ready to play");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "");
				SetShotClockDlgDate(hDlg, time(NULL));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_CHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_WELCOME:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Welcome to e-Media Poker!");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "");
				SetShotClockDlgDate(hDlg, time(NULL));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_CHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_CONNECTION_PROBS:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Some network problems occurring");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "");
				SetShotClockDlgDate(hDlg, time(NULL));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_CHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_CLOSE_CASHIER:
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT, "Cashier is temporarily closed");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "");
				SetShotClockDlgDate(hDlg, time(NULL));
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CLOSE_CASHIER,BST_CHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_CASHIER_AUTO_UP_DOWN,BST_CHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_CHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_TOURN_OPEN1:
			case IDC_MSG_TOURN_OPEN2:
			  #if 0	//20010125MB
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "Opening in %t9");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "Now open!");
			  #else
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "Tournaments in %t9");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "Tournaments are now open!");
			  #endif
				{
					time_t open_time = Nearesttime(time(NULL)-12*60*60, 24*60*60);	// calculate GMT midnight today (in the past)
					if (LOWORD(wParam)==IDC_MSG_TOURN_OPEN1) {
						open_time += 21*3600;	// 21:00 GMT
					} else {
						open_time += 6*3600;	// 06:00 GMT
					}
				  #if DAYLIGHT_SAVINGS_TIME
					open_time -= 1*3600;
				  #endif
					if (open_time < time(NULL)) {
						open_time += 24*3600;
					}
					SetShotClockDlgDate(hDlg, open_time);
				}
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_CHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN,BST_CHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_MSG_TOURN_CLOSE1:
			case IDC_MSG_TOURN_CLOSE2:
			  #if 0	//20010125MB
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "Now open!");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "");
			  #else
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2, "Tournaments are now open!");
				SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, "Tournaments: 4-9pm & 1-4am EST");
			  #endif
				{
					time_t open_time = Nearesttime(time(NULL)-12*60*60, 24*60*60);	// calculate GMT midnight today (in the past)
					if (LOWORD(wParam)==IDC_MSG_TOURN_CLOSE1) {
						open_time += 2*3600;	// 02:00 GMT
					} else {
						open_time += 9*3600;	// 09:00 GMT
					}
				  #if DAYLIGHT_SAVINGS_TIME
					open_time -= 1*3600;
				  #endif
					if (open_time < time(NULL)) {
						open_time += 24*3600;
					}
					SetShotClockDlgDate(hDlg, open_time);
				}
				CheckDlgButton(hDlg, IDC_ANNOUNCE_AT_TABLES,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_WHEN_DONE,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_SHUTDOWN_IS_BRIEF,   BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_RESTART_NEW_SERVER,  BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SECONDS,BST_CHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_DISPLAY_SHORT_UNITS,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY,BST_UNCHECKED);
				CheckDlgButton(hDlg, IDC_CHKBOX_TOURNAMENTS_OPEN,BST_UNCHECKED);
			    return TRUE;	// We DID process this message.
			case IDC_CHKBOX_SAME_AS_PRE_EXPIRY:
				if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_SAME_AS_PRE_EXPIRY)) {
					GetDlgText(hDlg, IDC_MESSAGE_TEXT , scu.shotclock_message1_expired, SHOTCLOCK_MESSAGE_LEN);
					GetDlgText(hDlg, IDC_MESSAGE_TEXT2, scu.shotclock_message2_expired, SHOTCLOCK_MESSAGE_LEN);
					SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT1_EXPIRED, scu.shotclock_message1_expired);
					SetDlgItemTextIfNecessary(hDlg, IDC_MESSAGE_TEXT2_EXPIRED, scu.shotclock_message2_expired);
				}
			    return TRUE;	// We DID process this message.
			case IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED:
				{
					int allowed = TRUE;
					if (!IsDlgButtonChecked(hDlg, IDC_CHKBOX_TOURNAMENT_SITDOWN_ALLOWED)) {
						allowed = FALSE;
					}
					if (LoggedInPrivLevel < ACCPRIV_ADMINISTRATOR) {
						allowed = FALSE;
					}
					EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_TOURNAMENT_STARTING_ALLOWED), allowed);
					EnableWindow(GetDlgItem(hDlg, IDC_MAX_TOURN_TABLES), allowed);
					EnableWindow(GetDlgItem(hDlg, IDC_MAX_TOURN_TABLES_SPINNER), allowed);
				}
			    return TRUE;	// We DID process this message.
			case IDC_CHKBOX_ALARM:
				Defaults.iShotClockAlarm = IsDlgButtonChecked(hDlg, IDC_CHKBOX_ALARM);
				Defaults.changed_flag = TRUE;
				WriteDefaults(FALSE);
			    return TRUE;	// We DID process this message.
			}
		}
		break;
	case WM_DESTROY:
		KillTimer(hDlg, WM_TIMER);	// remove our timer.
		WinStoreWindowPos(ProgramRegistryPrefix, "ChangeShotClock", hDlg, NULL);
		RemoveKeyboardTranslateHwnd(hDlg);
	    return TRUE;	// We DID process this message.
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	case WM_TIMER:
		UpdateTimeZoneExamples(hDlg);
		break;
	}
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}

//****************************************************************
////
// Mesage handler for 'Enter Broadcast Message' Window
//
BOOL CALLBACK dlgFuncEnterBroadcastMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//kp(("%s(%d) dlg $%08lx got message = %d\n",_FL, hDlg, message));
	switch (message) {
	case WM_INITDIALOG:
		WinRestoreWindowPos(ProgramRegistryPrefix, "SendBroadcast", hDlg, NULL, NULL, FALSE, TRUE);
		if (dwBroadcastDestPlayerID) {
			char str[200];
			sprintf(str, "Enter message to broadcast to player $%08lx", dwBroadcastDestPlayerID);
			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_BROADCAST_DEST), str);
			// messages sent to one player can only be blasts, not Dealer
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_BROADCAST_DEALER), FALSE);
		} else {
			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_BROADCAST_DEST),
				"Enter message to broadcast to ALL connected clients:");
		}
		SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_LIMITTEXT, MAX_MISC_CLIENT_MESSAGE_LEN-1, 0L);
		// by default, it's an online blast
		CheckDlgButton(hDlg, IDC_RADIO_BROADCAST_BLAST, TRUE);
		// By default, broadcast chat goes to all table types.
		CheckDlgButton(hDlg, IDC_SEND_TO_REAL_MONEY, TRUE);
		CheckDlgButton(hDlg, IDC_SEND_TO_PLAY_MONEY, TRUE);
		CheckDlgButton(hDlg, IDC_SEND_TO_TOURNAMENT, TRUE);
		return TRUE;	// TRUE = set keyboard focus appropriately

	case WM_COMMAND:
		//kp(("%s(%d) dialog $%08lx got WM_COMMAND\n", _FL, hDlg));
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDOK:
				if (IsDlgButtonChecked(hDlg, IDC_RADIO_BROADCAST_BLAST)) {	// online blast
					// Read the text from the control and send it to the server to be blasted
					struct MiscClientMessage mcm;
					zstruct(mcm);
					mcm.message_type = MISC_MESSAGE_CL_REQ_BROADCAST_MESSAGE;
					mcm.misc_data_1 = dwBroadcastDestPlayerID;
					GetDlgItemText(hDlg, IDC_MESSAGE_TEXT, mcm.msg, MAX_MISC_CLIENT_MESSAGE_LEN);
					SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));
				} else { // IDC_RADIO_DEALER (send as special-case chat text)
					struct GameChatMessage gcm;
					zstruct(gcm);
					gcm.table_serial_number = 0;// special case (for everyone)
					gcm.game_serial_number = 0;	// special case (for everyone)
					gcm.text_type = CHATTEXT_ADMIN;
					if (IsDlgButtonChecked(hDlg, IDC_SEND_TO_REAL_MONEY)) {
						gcm.flags |= GCMF_SEND_TO_REAL_TABLES;
					}
					if (IsDlgButtonChecked(hDlg, IDC_SEND_TO_PLAY_MONEY)) {
						gcm.flags |= GCMF_SEND_TO_PLAY_TABLES;
					}
					if (IsDlgButtonChecked(hDlg, IDC_SEND_TO_TOURNAMENT)) {
						gcm.flags |= GCMF_SEND_TO_TOURNAMENT_TABLES;
					}
					if (!(gcm.flags & GCMF_SEND_TO_ALL_TABLES)) {
						// No destination tables selected.  Error.
						MessageBox(hDlg, "You must select at least one destination table type.",
									"What were you thinking?", MB_OK|MB_ICONSTOP);
					    return TRUE;	// We DID process this message.
					}
					GetDlgItemText(hDlg, IDC_MESSAGE_TEXT, gcm.message, MAX_CHAT_MSG_LEN);
					SendDataStructure(DATATYPE_CHAT_MSG, &gcm, sizeof(gcm));
				}
				EndDialog(hDlg, 1);
			    return TRUE;	// We DID process this message.
			case IDCANCEL:
				//kp(("%s(%d) IDCANCEL received.\n",_FL));
				EndDialog(hDlg, 0);
			    return TRUE;	// We DID process this message.
			case IDC_RADIO_BROADCAST_BLAST:
				/*
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_REAL_MONEY), FALSE);
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_PLAY_MONEY), FALSE);
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_TOURNAMENT), FALSE);
				*/
			    return TRUE;	// We DID process this message.
			case IDC_RADIO_BROADCAST_DEALER:
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_REAL_MONEY), TRUE);
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_PLAY_MONEY), TRUE);
				EnableWindowIfNecessary(GetDlgItem(hDlg, IDC_SEND_TO_TOURNAMENT), TRUE);
			    return TRUE;	// We DID process this message.
			}
		}
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	case WM_DESTROY:
		WinStoreWindowPos(ProgramRegistryPrefix, "SendBroadcast", hDlg, NULL);
		break;
	}
	NOTUSED(hDlg);
	NOTUSED(wParam);
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}
#endif

/**********************************************************************************
 Function RedrawHandRequestDlg(HWND hDlg
 Date: CR1999/08/18
 Purpose: do some common redrawing on the Hand Req dlg
***********************************************************************************/
void RedrawHandRequestDlg(HWND hDlg)
{
	#define HAND_HISTORY_EMAIL_BYTES	1500
	#define REQ_HAND_DEFAULT_HISTORY_NUM 5
	HWND hSpinner = GetDlgItem(hDlg, IDC_SPIN_HAND_REQ);
	int num = SendMessage(hSpinner, UDM_GETPOS, (WPARAM)0, (LPARAM)0);
	char str[50];
	if (IsDlgButtonChecked(hDlg, IDC_RADIO_REQ_SINGLE_HAND)) {
		sprintf(str,"(Estimated email size: %.0dK)", (HAND_HISTORY_EMAIL_BYTES/1000));
	} else {
		sprintf(str,"(Estimated email size: %.0dK)", (HAND_HISTORY_EMAIL_BYTES * num)/1000);
	}
	SetDlgItemText(hDlg, IDC_STATIC_REQ_SIZE, str);
	sprintf(str, "%s", num == 1 ? "hand" : "hands");
	SetDlgItemText(hDlg, IDC_STATIC_REQ_HANDS, str);
}


/**********************************************************************************
 Function CALLBACK dlgFuncRequestHandHistory(...)
 Date: CR1999/08/10
 Purpose: requester for hand histories
***********************************************************************************/
BOOL CALLBACK dlgFuncRequestHandHistory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		{
			hReqHandHistory = hDlg;
			// by default, select the spinner
			CheckDlgButton(hDlg, IDC_RADIO_REQ_SINGLE_HAND, FALSE);
			CheckDlgButton(hDlg, IDC_RADIO_REQ_MULTIPLE_HANDS, TRUE);
			// init the spinner control
			HWND hSpinner = GetDlgItem(hDlg, IDC_SPIN_HAND_REQ);
			SendMessage(hSpinner, UDM_SETBUDDY, (WPARAM)(GetDlgItem(hDlg,IDC_EDIT_HAND_REQ_SPIN)),(LPARAM)0);
			SendMessage(hSpinner, UDM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELONG(100,1)));
			SendMessage(hSpinner, UDM_SETPOS, (WPARAM)0, (LPARAM)(MAKELONG(REQ_HAND_DEFAULT_HISTORY_NUM,0)));
			RedrawHandRequestDlg(hDlg);
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_HAND_REQ_SPIN));
			if (LoggedInPrivLevel >= ACCPRIV_CUSTOMER_SUPPORT) {
				CheckDlgButton(hDlg, IDC_ADMIN_FLAG, TRUE);
				ShowWindow(GetDlgItem(hDlg, IDC_ADMIN_FLAG), SW_SHOW);
			} else {
				CheckDlgButton(hDlg, IDC_ADMIN_FLAG, TRUE);
			}
			AddKeyboardTranslateHwnd(hDlg);
			// Make ourselves a topmost window
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		  #if ADMIN_CLIENT
			if (LoggedInPrivLevel >= ACCPRIV_CUSTOMER_SUPPORT) {
				EnableWindow(GetDlgItem(hDlg, IDC_REQ_HH_APPLY), TRUE);
				ShowWindow(GetDlgItem(hDlg, IDC_REQ_HH_APPLY), SW_SHOW);
			}
		  #endif
		}
		return FALSE;

	case WM_CTLCOLORSTATIC:	// paint the read-only spin-edit box white
		if (IsDlgButtonChecked(hDlg, IDC_RADIO_REQ_MULTIPLE_HANDS) &&
			(HWND)lParam == GetDlgItem(hDlg, IDC_EDIT_HAND_REQ_SPIN)) {
			return ((int)GetStockObject(WHITE_BRUSH));
		}
		break;

	case WM_VSCROLL:
		RedrawHandRequestDlg(hDlg);
		break;
	
	case WM_DESTROY:
		RemoveKeyboardTranslateHwnd(hDlg);
		hReqHandHistory = NULL;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_RADIO_REQ_SINGLE_HAND ||
			LOWORD(wParam) == IDC_RADIO_REQ_MULTIPLE_HANDS)
		{
			// redraw (enable/disable) relevant controls
			RedrawHandRequestDlg(hDlg);
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_REQ_SINGLE_HAND)) {
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_REQ_HAND_NUM), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_SPIN_HAND_REQ), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HAND_REQ_SPIN), FALSE);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_REQ_HAND_NUM));
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_REQ_HAND_NUM), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_SPIN_HAND_REQ), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HAND_REQ_SPIN), TRUE);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_HAND_REQ_SPIN));
			}
		}		
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDC_REQ_HH_APPLY:
			struct CardRoom_ReqHandHistory crhh;
			zstruct(crhh);
			if (LoggedInPrivLevel >= ACCPRIV_CUSTOMER_SUPPORT) {
				crhh.admin_flag = (BYTE8)IsDlgButtonChecked(hDlg, IDC_ADMIN_FLAG);
			}
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_REQ_SINGLE_HAND)) {
				char str[50];
				zstruct(str);
				GetDlgItemText(hDlg, IDC_EDIT_REQ_HAND_NUM, str, 20);
				crhh.hand_number = atoi(str);
				crhh.request_type = HHRT_INDIVIDUAL_HAND;
			} else {
				HWND hSpinner = GetDlgItem(hDlg, IDC_SPIN_HAND_REQ);
				crhh.hand_number = SendMessage(hSpinner, UDM_GETPOS, (WPARAM)0, (LPARAM)0);
				crhh.request_type = HHRT_LAST_N_HANDS;
			}
		  #if 0// !!!!!!!!!! TESTING ONLY !!!!!!!!!!!
			{
				for (int i=95000; i > 1; i--) {
					crhh.hand_number = i;
					crhh.request_type = HHRT_INDIVIDUAL_HAND;
					SendDataStructure(DATATYPE_CARDROOM_REQ_HAND_HISTORY, &crhh, sizeof(crhh));
					kp1(("%s(%d) Req = %d GET RID OF THIS !!! \n", _FL, i));	
					Sleep(200);	// never eat all the cpu time
				}
			}
		  #endif	// !!!!!!!!!!!!!!!!!!!!

			if (crhh.hand_number) {	// non-zero of some sort -- process
				HWND hSubmitProgressDlg = CreateProgressWindow(hDlg,
						"Submitting hand history request to server...",
						60,	// start at 0%, go up to 60% while waiting
						1000);	// # of ms for indicator to get to that %
				SendDataStructure(DATATYPE_CARDROOM_REQ_HAND_HISTORY, &crhh, sizeof(crhh));
				if (hSubmitProgressDlg) {
					FinishProgressWindow(hSubmitProgressDlg, 200);
				}
			}
			if (LOWORD(wParam) == IDC_REQ_HH_APPLY) {
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_REQ_HAND_NUM));
			} else {
				DestroyWindow(hDlg);
			}
			MessageBox(NULL,
				"You will receive an Email response to your request;\nThis may take a few minutes.",
				"Hand History Request Submitted...",
				MB_OK|MB_TOPMOST);
		    return TRUE;	// message was processed
		case IDCANCEL:
			DestroyWindow(hDlg);
		    return TRUE;	// message was processed
		}
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	}

	NOTUSED(wParam);
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}

#if ADMIN_CLIENT
/**********************************************************************************
 Function CALLBACK dlgFuncChatMonitor
 Date: CR00/01/29
 Purpose: handler for chat monitor window
***********************************************************************************/
BOOL CALLBACK dlgFuncChatMonitor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hSlider;
	char str[10];
	switch (message) {
	case WM_INITDIALOG:
		hChatMonitorDlg = hDlg;
		AddKeyboardTranslateHwnd(hDlg);
		CheckDlgButton(hDlg, IDC_CHECK_MONITOR_PLAY, Defaults.iAlertPlayTables);
		CheckDlgButton(hDlg, IDC_CHECK_MONITOR_REAL, Defaults.iAlertRealTables);
		CheckDlgButton(hDlg, IDC_CHECK_MONITOR_TOURN, Defaults.iAlertTournTables);
		CheckDlgButton(hDlg, IDC_CHAT_MONITOR_FREEZE, FreezeChatMonitor);
		CheckDlgButton(hDlg, IDC_CHECK_MONITOR_SOUND, Defaults.iAlertSound);
		CheckDlgButton(hDlg, IDC_CHECK_MONITOR_POPUP, Defaults.iAlertPopUp);
		zstruct(ChatMonitorString);
		PostMessage(hCardRoomDlg, WMP_REFRESH_MONITOR_CHAT, 0, 0);
		WinRestoreWindowPos(ProgramRegistryPrefix, "ChatMonitor", hDlg, NULL, NULL, FALSE, TRUE);
		WinPosWindowOnScreen(hDlg);
		// set slider to current position
		hSlider = GetDlgItem(hDlg, IDC_MONITOR_SLIDER);
		SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 9));
		SendMessage(hSlider, TBM_SETPOS, TRUE, Defaults.iAlertMonitorLevel);
		sprintf(str, "%d", Defaults.iAlertMonitorLevel+1);
		SetDlgItemText(hDlg, IDC_STATIC_ALERT_LEVEL, str);
		// Change the font in the display window to non-proportional
		{
			HWND hwnd = GetDlgItem(hDlg, IDC_CHAT_MONITOR_VIEW);
			static HFONT chatwindowfont;
			if (!chatwindowfont) {
				RECT r;
				zstruct(r);
				GetWindowRect(hwnd, &r);
				chatwindowfont = CreateFont(
					11,//  int nHeight,             // logical height of font
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
					"Lucida Console" //  LPCTSTR lpszFace         // pointer to typeface name string
				);
			}
			if (chatwindowfont) {
				SendMessage(hwnd, WM_SETFONT, (WPARAM)chatwindowfont, 0);
			}

		}
		SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
		ShowWindow(hDlg, SW_SHOW);
		// Make sure it's put at the top...
		SetWindowPos(hDlg, HWND_TOPMOST,   0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		return FALSE;

	case WM_HSCROLL:
		hSlider = GetDlgItem(hDlg, IDC_MONITOR_SLIDER);
		Defaults.iAlertMonitorLevel = SendMessage(hSlider, TBM_GETPOS, 0, 0);
		sprintf(str, "%d", Defaults.iAlertMonitorLevel+1);
		SetDlgItemText(hDlg, IDC_STATIC_ALERT_LEVEL, str);
		PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
		break;

	case WM_DESTROY:
		WinStoreWindowPos(ProgramRegistryPrefix, "ChatMonitor", hDlg, NULL);
		hChatMonitorDlg = NULL;
		RemoveKeyboardTranslateHwnd(hDlg);
		break;

	case WM_CTLCOLORSTATIC:	// paint the read-only edit box white
		if ( (HWND)lParam == GetDlgItem(hDlg, IDC_CHAT_MONITOR_VIEW) ) {
			return ((int)GetStockObject(WHITE_BRUSH));
		}
		break;
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHATMON_RESET:
			zstruct(ChatMonitorString);
			SetDlgItemText(hDlg, IDC_CHATMON_EDIT, "");
			PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDC_CHATMON_APPLY:
			GetDlgItemText(hDlg, IDC_CHATMON_EDIT, ChatMonitorString, MAX_COMMON_STRING_LEN);
			PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDC_CHECK_MONITOR_PLAY:
			Defaults.iAlertPlayTables = IsDlgButtonChecked(hDlg, IDC_CHECK_MONITOR_PLAY);
			PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDC_CHECK_MONITOR_REAL:
			Defaults.iAlertRealTables = IsDlgButtonChecked(hDlg, IDC_CHECK_MONITOR_REAL);
			PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDC_CHECK_MONITOR_TOURN:
			Defaults.iAlertTournTables = IsDlgButtonChecked(hDlg, IDC_CHECK_MONITOR_TOURN);
			PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDC_CHECK_MONITOR_POPUP:
			Defaults.iAlertPopUp = IsDlgButtonChecked(hDlg, IDC_CHECK_MONITOR_POPUP);
			return TRUE;
		case IDC_CHECK_MONITOR_SOUND:
			Defaults.iAlertSound = IsDlgButtonChecked(hDlg, IDC_CHECK_MONITOR_SOUND);
			return TRUE;
		case IDC_CHAT_MONITOR_CLEAR_ALERTS:
			{
				int result = MessageBox(hDlg,
						"Are you sure you want to clear all alerts up to now?",
						"Clear Alerts", MB_YESNO);
				if (result==IDYES) {
					for (int index = ADMIN_CHAT_LINES-1; index >= 0; index--) {
						if (AdminChatMonitor[index].text_type >= ALERT_1) { // zap it
							zstruct(AdminChatMonitor[index]);
						}
					}
					PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
					SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
				}
			}
			return TRUE;
		case IDC_CHAT_MONITOR_FREEZE:
			FreezeChatMonitor = IsDlgButtonChecked(hDlg, IDC_CHAT_MONITOR_FREEZE);
			if (!FreezeChatMonitor) {
				PostMessage(hCardRoomDlg, WMP_FORCE_REFRESH_MONITOR, 0, 0);
			}
			SetFocus(GetDlgItem(hDlg, IDC_CHATMON_EDIT));
			return TRUE;
		case IDCANCEL:
			DestroyWindow(hDlg);
		    return TRUE;	// message was processed
		}
		break;
	case WM_SIZING:
		WinSnapWhileSizing(hDlg, message, wParam, lParam);
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	}
	NOTUSED(wParam);
	NOTUSED(lParam);
    return FALSE;	// We did NOT process this message.
}

/**********************************************************************************
 Function UpdateAdminChatBuffer(struct GameChatMessage *gcm)
 Date: CR00/02/06
 Purpose: add a line to admin chat monitor buffer
 Returns: T/F whether a redraw is needed
***********************************************************************************/
int UpdateAdminChatBuffer(struct GameChatMessage *gcm)
{
	/* -- 2999 -- * <--- oldest ADMIN_CHAT_LINES */
	/*|          |*/
	/*|   admin  |*/
	/*|          |*/
	/* -- 1999 -- * <--- ADMIN_CHAT_LINES - ALERT_CUTOFF */
	/*|          |*/
	/*|          |*/
	/*|   chat   |*/
	/*|          |*/
	/*|          |*/
	/*|          |*/
	/* --- 0 ---- * <--- newest */
	#define ALERT_CUTOFF	1000	// 20000103HK: changed from 500
		
	// we need to figure out where we're going to scroll the whole buffer... so we run from the top down to see what's important
	int lines_to_scroll = ADMIN_CHAT_LINES-1;	// assume whole buffer (-last line which is being scrolled off)
	int found_scroll_spot = TRUE;	// assume there's room
	if (AdminChatMonitor[lines_to_scroll].text_type >= ALERT_6 && AdminChatMonitor[lines_to_scroll].text_type <= ALERT_10) {	// may need to keep it
		found_scroll_spot = FALSE;	// there may not be room
		while (lines_to_scroll > ADMIN_CHAT_LINES-ALERT_CUTOFF) {
			if (AdminChatMonitor[lines_to_scroll].text_type >= ALERT_6 && AdminChatMonitor[lines_to_scroll].text_type <= ALERT_10) { // we want to keep this line
				lines_to_scroll--;
			} else {	// good spot, we'll scroll to here
				found_scroll_spot = TRUE;
				break;
			}
		}
	}

	if (!found_scroll_spot) {	// we'll have to lose the oldest alert, just scroll everything
		lines_to_scroll = ADMIN_CHAT_LINES-1;	// assume whole buffer (-last line which is being scrolled off)
	}
	// new stuff goes to slot 0	
	memcpy(&AdminChatMonitor[1], &AdminChatMonitor[0], sizeof(ChatEntry)*lines_to_scroll);
	strnncpy(AdminChatMonitor[0].message, gcm->message, MAX_CHAT_MSG_LEN);
	BYTE8 t_type = gcm->text_type;
	AdminChatMonitor[0].text_type = t_type;
	// increment index for next call
	// decide whether we need a redraw or not
	if (t_type == CHATTEXT_MONITOR_PLAY && Defaults.iAlertPlayTables) {
		return TRUE;
	}
	if (t_type == CHATTEXT_MONITOR_REAL && Defaults.iAlertRealTables) {
		return TRUE;
	}
	if (t_type == CHATTEXT_MONITOR_TOURN && Defaults.iAlertTournTables) {
		return TRUE;
	}
	// 20000531HK: L10 alert override is handled seperately
	int alert_override = FALSE;
	if (t_type == ALERT_10) {
		static time_t last_alert10_time;
		time_t time_now = time(NULL);
		if (difftime(time_now, last_alert10_time) > 60) {	// once per minute is enough
			last_alert10_time = time_now;
			alert_override = TRUE;
		}
	}
	if (t_type >= ALERT_1 && 
		t_type < ALERT_10 && 
		Defaults.iAlertMonitorLevel <= t_type-ALERT_1) {
		if (Defaults.iAlertSound || alert_override) {
			PPlaySound(SOUND_WAKE_UP);		// 'bigger' sound
		}
		if (Defaults.iAlertPopUp || alert_override) {
			if (hChatMonitorDlg) {
				ShowWindow(hChatMonitorDlg, SW_SHOWNOACTIVATE);
				SetWindowPos(hChatMonitorDlg, HWND_TOPMOST,   0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				SetWindowPos(hChatMonitorDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			} else {
				PostMessage(hCardRoomDlg, WM_COMMAND, (WPARAM)IDD_CHAT_MONITOR, (LPARAM)0);
			}
		}
		return TRUE;
	}
	return FALSE;
}

/**********************************************************************************
 Function RefreshMonitorChatText
 Date: CR00/02/06
 Purpose: redraw our monitor chat box with correct text
***********************************************************************************/
void RefreshMonitorChatText(void)
{
	RefreshMonitorChatText(FALSE);
}

void RefreshMonitorChatText(int force_update_now)
{
	// don't bother if it doesn't exist or if it's minimized
	if (!hChatMonitorDlg || IsIconic(hChatMonitorDlg)) {
		return;
	}
	// frozen means no update for sure
	if (FreezeChatMonitor) {
		return;
	}
	// 20000604HK: shortcut for freezing when it's too much -- should be fixed properly
	static time_t last_chat_upate_time;
	time_t time_now = time(NULL);
	if (!force_update_now) {	// don't bother testing if we want a refresh for sure
		if (difftime(time_now, last_chat_upate_time) == 0) {	// every second or more is OK
			return;
		}
	}
	zstruct(ChatMonitorBuffer);
	// first, build the current buffer 
	char str[MAX_CHAT_MSG_LEN];
	for (int index = ADMIN_CHAT_LINES-1; index >= 0; index--) {
		int current_line = index;
		if (AdminChatMonitor[current_line].text_type) { // something there?
			int t_type = AdminChatMonitor[current_line].text_type;
			// apply filters
			if (t_type == CHATTEXT_MONITOR_PLAY && !Defaults.iAlertPlayTables) {
				continue;
			}
			if (t_type == CHATTEXT_MONITOR_REAL && !Defaults.iAlertRealTables) {
				continue;
			}
			if (t_type == CHATTEXT_MONITOR_TOURN && !Defaults.iAlertTournTables) {
				continue;
			}
			if (t_type >= ALERT_1 && 
				t_type <= ALERT_10 && 
				Defaults.iAlertMonitorLevel > t_type-ALERT_1) {
				continue;
			}
			
			if (ChatMonitorString[0]) {	// apply text filter
				char test_message[MAX_CHAT_MSG_LEN];
				char test_string[MAX_COMMON_STRING_LEN];
				zstruct(test_message);
				zstruct(test_string);
				int i;
				for (i = 0; i < (int)strlen(AdminChatMonitor[current_line].message); i++) {
					if (i == MAX_CHAT_MSG_LEN) {
						break;
					}
					test_message[i] = (char)tolower(AdminChatMonitor[current_line].message[i]);
				}
				for (i = 0; i < (int)strlen(ChatMonitorString); i++) {
					if (i == MAX_COMMON_STRING_LEN) {
						break;
					}
					test_string[i] = (char)tolower(ChatMonitorString[i]);
				}
				if (!strstr(test_message, test_string)) {
					continue;
				}
			}

			strnncpy(str, AdminChatMonitor[current_line].message, sizeof(str)-3);
			// Trim any control characters from the end of the string.
			while (strlen(str) && str[strlen(str)-1] < 32)
				str[strlen(str)-1] = 0;	// trim last char
			strcat(str, "\r\n");	// append cr/lf
			strcat(ChatMonitorBuffer, str);
		}
	}
	// peel off last cr/lf pair (cosmetic)
	if (strlen(ChatMonitorBuffer) >= 2) {
		ChatMonitorBuffer[strlen(ChatMonitorBuffer)-2] = 0;
	}
	// buffer is built; copy it into the chat window
	// scroll to bottom if we're allowed to
	// note -- chat_scroll_disabled is never TRUE, for now....
	if (hChatMonitorDlg) {
		HWND chat_hwnd = GetDlgItem(hChatMonitorDlg, IDC_CHAT_MONITOR_VIEW);
		SendMessage(chat_hwnd, WM_SETTEXT, (WPARAM)0,(LPARAM)ChatMonitorBuffer);	// this sets the text
		SendMessage(chat_hwnd, EM_LINESCROLL, (WPARAM)0, (LPARAM)ADMIN_CHAT_LINES*2);	
//		SendMessage(chat_hwnd, EM_LINESCROLL, (WPARAM)0, (LPARAM)37);	
/*		int lines_to_scroll = SendMessage(chat_hwnd, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
		int window_height = SendMessage(chat_hwnd, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0);
		window_height = 25;
		SendMessage(chat_hwnd, EM_LINESCROLL, (WPARAM)0, (LPARAM)lines_to_scroll-window_height);	
*/
	  #if USE_RICH_EDIT_FOR_CHAT_MONITOR && 0	//20000225MB
		// The edit control has been reverted to the old one in the .rc file!!
		// Regular edit control:
		//    EDITTEXT        IDC_EDIT1,2,1,394,267,ES_MULTILINE | WS_VSCROLL
		// RichEdit2.0 edit control:
		//    CONTROL    "",IDC_EDIT1,"RichEdit20A",
        //            WS_BORDER | WS_VSCROLL | WS_TABSTOP | 0x804,
        //        2,1,394,267
		// Experiment with setting the background text color
		CHARFORMAT fmt;
		zstruct(fmt);
		fmt.cbSize = sizeof(fmt);
		fmt.dwMask = CFM_COLOR;
		fmt.crTextColor = RGB(200,0,0);
		SendMessage(chat_hwnd, EM_SETSEL, (WPARAM)strlen(ChatMonitorBuffer)/2, (LPARAM)-1);
		SendMessage(chat_hwnd, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&fmt);
		SendMessage(chat_hwnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
	  #endif

	}
	// as per converstation with Mike, update the time now instead of where we test
	last_chat_upate_time = time(NULL);
}
#endif	// ADMIN_CLIENT



/**********************************************************************************

***********************************************************************************/
BOOL CALLBACK dlgDownLoadClient(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
	case WM_INITDIALOG:
		AddKeyboardTranslateHwnd(hDlg);
		SetDlgItemText(hDlg, IDC_STATIC_ABOUT3, "e-Media Poker is currently installing some new software features to enhance your Poker Experience.\n\nThis will only take a moment. In fact, players with high speed internet connections will barely have time to read this screen before the e-Media Poker games restart with all of the new features in place.");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, IDOK);
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);

			return TRUE;
		}
		break;

	case WM_DESTROY:
		RemoveKeyboardTranslateHwnd(hDlg);
		return TRUE;
	}
	NOTUSED(wParam);
	NOTUSED(lParam);
	
    return FALSE;
}

