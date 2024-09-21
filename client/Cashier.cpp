/**********************************************************************************


 Purpose: cashier/cash/credit related  functions and dialogs

***********************************************************************************/



#define DISP 0



#include "stdafx.h"

#include "resource.h"



#ifdef HORATIO

  #define ALLOW_CC_FUNCTIONS	1

#else

  #define ALLOW_CC_FUNCTIONS	(ADMIN_CLIENT || 1)

#endif





int  valid_limitation;

int  cashout_amount=0;



HWND hCCPurchaseDLG = NULL;

HWND hCCCreditDLG = NULL;

HWND hViewTransactionsDLG = NULL;

HWND hEnterPhoneNumberDLG = NULL;	// handle to the 'what's your phone number?' dlg (if open)

extern struct AccountRecord LatestAccountRecordFromServer;



struct MiscClientMessage MiscMsgCCBuyinLimits;	// msg[] contains description of CC buy-in limits for 'buy chips' screen



BOOL CALLBACK dlgFuncAccountCharge(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncAccountCredit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncAccountCredit_1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncEnterPhoneNumber(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



BOOL CALLBACK dlgFuncWesternUnion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncBankDrafts(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncPayPal(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);







BOOL CALLBACK dlgDisableCashierFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//BOOL CALLBACK dlgFuncReadContract(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//extern BOOL CALLBACK dlgFuncMakeRealMoneyReady(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern int backbutton;

extern char samount[20];



#if ADMIN_CLIENT

BOOL CALLBACK dlgEditCreditable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgCheckTracking(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HWND hEditCreditableDLG = NULL;

#endif



static HBITMAP hCashierBgnd;		// handle to background picture for dialog box.

static HPALETTE hCashierPalette;	// handle to palette for bgnd picture

static HFONT hCashierButtonFont;

static HFONT hCashierButtonFontSmall;

static HWND hCashierToolTips;

static struct ProximityHighlightingInfo CashierHighlightList;



struct DlgToolTipText CashierDlgToolTipText[] = {

	IDC_PURCHASE_CHIPS,		"Purchase chips on your credit card",

	IDC_CASH_CHIPS,			"Cash out to your credit card or get a check issued",

	IDC_TRANSACTION_HISTORY,"View your recent transaction history",

	IDC_SETUP_ACCOUNT,		"Set up your account for playing with real money",

	IDCANCEL,				"Leave the cashier screen and go back to the lobby",

	//rgong 04/04/2002

	IDC_DOWNLOAD1,			"Play for real & get $100",

	IDC_DOWNLOAD2,			"Cash bonus for referrals",

	//end rgong
	

	0,0

};



#if !ALLOW_CC_FUNCTIONS

//*********************************************************

//
//

void DisplayNoCCFunctionsMessage(void)

{

	MessageBox(hCashierDlg,

				"The credit card functions will become available\n"

				"when our Real Money tables are ready to open.",

				"Credit Card functions not available",

				MB_OK);

}	

#endif	// !ALLOW_CC_FUNCTIONS



#if ADMIN_CLIENT

//*********************************************************

//
//

// Dialog message handler

//

BOOL CALLBACK dlgFuncSetActiveAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		WinRestoreWindowPos(ProgramRegistryPrefix, "SetActiveAccount", hDlg, NULL, NULL, FALSE, TRUE);

		WinPosWindowOnScreen(hDlg);



		// Fill in the fields for the combo box

		{

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_USERID), LatestAccountRecordFromServer.sdb.user_id);



			HWND combo = GetDlgItem(hDlg, IDC_COMBO_CARDNUMBERS);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0);

			for (int i=0; i <TRANS_TO_RECORD_PER_PLAYER; i++) {

				ClientTransaction *ct = &LatestAccountRecordFromServer.sdb.transaction[i];

				if (ct->transaction_type == CTT_PURCHASE) {

					char str[20];

					zstruct(str);

					sprintf(str, "%04x xxxx xxxx %04x", (ct->partial_cc_number>>16), ct->partial_cc_number & 0x00FFFF);

					// only add it if it's not already there

					int index = SendMessage(combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)str);

					if (index == CB_ERR) {	// it's not there, add it.

						index = SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)str);

						SendMessage(combo, CB_SETITEMDATA, (WPARAM)index, (LPARAM)ct->partial_cc_number);

					}

				}

			}

			// set selection to top of list

			SendMessage(combo, CB_SETCURSEL, (WPARAM)0, 0);

		}



		ShowWindow(hDlg, SW_SHOW);

		return TRUE;



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "SetActiveAccount", hDlg, NULL);

		RemoveKeyboardTranslateHwnd(hDlg);

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:

			{

				// Retrieve the user id and cc number

				struct MiscClientMessage mcm;

				zstruct(mcm);

				mcm.message_type = MISC_MESSAGE_SET_ACTIVE_ACCOUNT_FOR_CC;

				mcm.misc_data_1 = LatestAccountRecordFromServer.sdb.player_id;

				HWND combo = GetDlgItem(hDlg, IDC_COMBO_CARDNUMBERS);

				int index = SendMessage(combo, CB_GETCURSEL, 0, 0);

				mcm.misc_data_2 = SendMessage(combo, CB_GETITEMDATA, (WPARAM)index, 0);

				SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

			}

			DestroyWindow(hDlg);

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			DestroyWindow(hDlg);

			return TRUE;	// We DID process this message

		break;

		}

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}

#endif	 // ADMIN_CLIENT



static void DisplayBadEmailMessage(HWND hDlg)

{

	MessageBox(hDlg,

			"The email address we have on file for you has not been validated.\n\n"

			"If you wish to perform any cashier functions, we must have a valid\n"

			"email address for you.\n\n"

			"To update or validate your email address, go to the Lobby screen,\n"

			"select the Options menu, and select 'Change/Validate Email Address'.",

			"Email Address not validated...",

			MB_OK|MB_ICONINFORMATION);

}

static void DisplayCashierClosedMessage(HWND hDlg)

{

	MessageBox(hDlg,

			"The cashier is temporarily closed.\n\n"

			"You cannot purchase more chips or cash out until\n"

			"the cashier is re-opened.\n\n"

			"These closures usually last just a few minutes while\n"

			"a problem is getting resolved.  Please try again in a\n"

			"few minutes.",

			"Cashier temporarily closed...",

			MB_OK|MB_ICONINFORMATION);

}

//*********************************************************

//
//

// Encode (trivially) a cc number or month/year so they don't appear

// as normal plaintext on the disk.  This is NOT the same as encrypting

// them. it's merely making them less obvious.

// This does not handle 8-bit characters very well.  It's fine for 7-bit.

//

static void ObfuscateString(char *src_str, char *dest, int max_dest_len, int adjustment_value)

{

	char *max_dest = dest + max_dest_len - 1;

	memset(dest, 0, max_dest_len);

	while (*src_str && dest < max_dest) {

		*dest++ = (char)(*src_str++ + adjustment_value);

	}

	*max_dest = 0;	// make doubly-sure it's terminated.

}



static void ObfuscateString(char *src_str, char *dest, int max_dest_len)

{

	ObfuscateString(src_str, dest, max_dest_len, 10);

}



static void UnObfuscateString(char *src_str, char *dest, int max_dest_len)

{

	ObfuscateString(src_str, dest, max_dest_len, -10);

}



/**********************************************************************************

 Function StringToChips

 Date: HK11/14/1999

 Purpose: given a string entered, return corresponding chips (ie cents)

***********************************************************************************/

int StringToChips(char *str)

{

	//19991230MB: filter all non-digit and non-decimal characters first

	char new_str[50];

	zstruct(new_str);

	char *d = new_str;

	char *s = str;

	while (*s && (d-new_str) < sizeof(new_str)) {

		if (isdigit(*s) || *s=='.') {

			*d++ = *s;

			*d = 0;

		}

		s++;

	}

	return (int)floor(fabs(atof(new_str)) * 100.0 + .5);

}



/**********************************************************************************

 Function CALLBACK dlgFuncViewTransactions

 Date: HK11/15/1999

 Purpose: view last 20 client transactions

***********************************************************************************/

BOOL CALLBACK dlgFuncViewTransactions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	int i;

	ClientTransaction *ct;

	char tmp[150];

	char buf[TRANS_TO_RECORD_PER_PLAYER*150];

	HWND hwnd_txt;

	switch (message) {

	case WM_INITDIALOG:

		hViewTransactionsDLG = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

	  #if ADMIN_CLIENT

		if (iAdminClientFlag) {

/* Tony, Nov 28, 2001

			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_TRACKING), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_EDIT_CREDITABLE), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_ACTIVE_ACCOUNT), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TRANSACTION_ORDER), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_STATIC), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_APPLY), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_BLOCK), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_DEFAULT), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_REQ_STATEMENT), SW_SHOW);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_STATIC), 

				"Set Max allowed users for all these cards");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER), "2");

			// Set the range for the spinner (and at the same time invert the

			// actions for the up/down buttons).

			SendMessage(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), UDM_SETBUDDY,

				(WPARAM)(HWND)(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER)), (LPARAM)0);

			SendMessage(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), UDM_SETRANGE,

				(WPARAM)0, (LPARAM) MAKELONG((short)5, (short)1)); */

			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_TRACKING), (LoggedInPrivLevel < ACCPRIV_PIT_BOSS) ? SW_HIDE : SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_EDIT_CREDITABLE), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_ACTIVE_ACCOUNT), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TRANSACTION_ORDER), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_STATIC), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_APPLY), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_BLOCK), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_DEFAULT), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_REQ_STATEMENT), SW_HIDE);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_STATIC), 

				"Set Max allowed users for all these cards");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER), "2");

			// Set the range for the spinner (and at the same time invert the

			// actions for the up/down buttons).

			SendMessage(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), UDM_SETBUDDY,

				(WPARAM)(HWND)(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER)), (LPARAM)0);

			SendMessage(GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_SPINNER), UDM_SETRANGE,

				(WPARAM)0, (LPARAM) MAKELONG((short)5, (short)1));

//EOTony

	}

	  #endif

		static HFONT font;

		font = CreateFont(

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

			"Courier New"//  LPCTSTR lpszFace         // pointer to typeface name string

		);

		hwnd_txt = GetDlgItem(hDlg, IDC_TEXT);

		if (hwnd_txt && font) {

			SendMessage(hwnd_txt, WM_SETFONT, (WPARAM)font, 0);

		}

		PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		return TRUE;			



	case WMP_UPDATE_PLAYER_INFO:

		{

			zstruct(buf);

			zstruct(tmp);

//Tony			sprintf(tmp," %-21s  %-15s  %10s %-14s %-8s\x0D\x0A", 

//Tony				"Date (local time)", "Type", "Amount", "Reference", "Trans #");

			sprintf(tmp," %-21s  %-16s  %10s %-13s %-14s\x0D\x0A", 

				"     Date (CST)", "Transaction Type", "Amount  ", "Transaction #", "   Details");

			strcat(buf, tmp);

//Tony		sprintf(tmp," %-21s  %-15s  %10s %-14s %-8s\x0D\x0A", 

//Tony			"-----------------", "----", "------", "---------", "-------");

			sprintf(tmp," %-21s  %-16s  %10s %-13s %-14s\x0D\x0A", "---------------------",

				"----------------", "----------", "-------------", "--------------");

			strcat(buf, tmp);

		  int net_cc_purchases = 0;

		  #if ADMIN_CLIENT

			

			int credit_total = 0;

		  #endif

			int transaction_count = 0;



			 for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

			//for (i=TRANS_TO_RECORD_PER_PLAYER-1;  i>=0; i--) {

				ct = &ArToView.sdb.transaction[i];

				/*

				if (!strcmp(ct->unused, "paypal"))

				{

					ct->transaction_type = CCT_CREDIT;

				}

                */

				if (ct->transaction_type) {

					transaction_count++;

				  #if ADMIN_CLIENT

					switch (ct->transaction_type) {

					case CTT_PURCHASE:

						net_cc_purchases += ct->transaction_amount;

						credit_total += ct->credit_left;

						// 20001013HK: take into account if it's disabled from crediting

						if (ct->credit_left > ct->transaction_amount) {

							credit_total -= ct->transaction_amount;

						}

						break;

					case CTT_CREDIT:

						net_cc_purchases -= ct->transaction_amount;

						if (net_cc_purchases <=0)

							net_cc_purchases =0;

						break;

					}

					

                    ClientTransactionDescription(ct, tmp, iAdminClientFlag);

					#endif 



					strcat(tmp, "\r\n");	// append CR/LF

					strcat(buf, tmp);

				}

			}



            net_cc_purchases =0;

			transaction_count=0;



            //net_cc_purchases = ArToView.sdb.transaction[0].credit_left;

			 for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

			//for (i=TRANS_TO_RECORD_PER_PLAYER-1;  i>=0; i--) {

			 if (ArToView.sdb.transaction[i].transaction_type != CTT_NONE ) {

				 transaction_count++;

			 }



		 if (ArToView.sdb.transaction[i].credit_left >0 ) {

				net_cc_purchases = net_cc_purchases + (ArToView.sdb.transaction[i].credit_left);

			}

		 }



		 if (transaction_count) {

				sprintf(tmp, "Your last %d transactions...", transaction_count);

				SetWindowText(hDlg, tmp);

			}

		  #if ADMIN_CLIENT

		  	if (iAdminClientFlag) {

				// more specific title for administrators

				sprintf(tmp, "Last %d transactions for %s...", 

					transaction_count, ArToView.sdb.user_id);

				SetWindowText(hDlg, tmp);

				char cs1[MAX_CURRENCY_STRING_LEN];

				char cs2[MAX_CURRENCY_STRING_LEN];

				zstruct(cs1);

				zstruct(cs2);

				

				if (net_cc_purchases <=0) {

						net_cc_purchases =0;

				}



				/*

				sprintf(buf+strlen(buf), "%40s ---------- %23s ---------\r\n",

						"",

						""); */

				sprintf(buf+strlen(buf), "%40s  %23s \r\n",

						"",

						"");

				



				sprintf(buf+strlen(buf), "%40s %10s %23s %-9s\r\n",

						//Tony					"net cc purchases:",

						"Total direct credit left:",	//Tony, Dec 11, 2001



						CurrencyString(cs1, net_cc_purchases, CT_REAL, TRUE, 0),

						"",

						""/*CurrencyString(cs2, credit_total, CT_REAL, TRUE, 0)*/);

		  	}

		  #endif

			SetWindowText(GetDlgItem(hDlg, IDC_TEXT), buf);

			SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_SETSEL, (WPARAM)-1, 0);

		}

		return TRUE;



	case WM_CTLCOLORSTATIC:	// paint the read-only edit box white

		if ( (HWND)lParam == GetDlgItem(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER) ) {

			return ((int)GetStockObject(WHITE_BRUSH));

		}

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

	  #if ADMIN_CLIENT

		case IDC_EDIT_CREDITABLE:

			if (!hEditCreditableDLG) {

				CreateDialog(hInst, MAKEINTRESOURCE(IDD_EDIT_CREDITABLE), NULL, dlgEditCreditable);

			} else {

				ReallySetForegroundWindow(hEditCreditableDLG);

			}

			return TRUE;

		case IDC_REQ_STATEMENT:

			i = MessageBox(hDlg, "The Credit Card Statement will be emailed to you",

				"Request CC Statement...", MB_OKCANCEL);

			if (i != IDOK) {

				return TRUE;

			}

			struct CCStatementReq ccsr;

			zstruct(ccsr);

			ccsr.transaction_type = CCTRANSACTION_STATEMENT_REQUEST;

			ccsr.player_id = ArToView.sdb.player_id;

			SendDataStructure(DATATYPE_CARDROOM_REQ_CC_STATEMENT, &ccsr, sizeof(ccsr));

			return TRUE;

		case IDC_CHECK_TRACKING:

			if (!hCheckTrackingDLG) {

				CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHECK_TRACKING), NULL, dlgCheckTracking);

			} else {

				ReallySetForegroundWindow(hCheckTrackingDLG);

			}

			return TRUE;

		case IDC_SET_ACTIVE_ACCOUNT:

			CreateDialog(hInst, MAKEINTRESOURCE(IDD_SET_ACTIVE_ACCOUNT), NULL, dlgFuncSetActiveAccount);

			return TRUE;

		case IDC_SET_MAX_ALLOWABLE_APPLY:

		case IDC_SET_MAX_ALLOWABLE_BLOCK:

		case IDC_SET_MAX_ALLOWABLE_DEFAULT:

			{

				int cmd = LOWORD(wParam);

				int value_to_set = -1;	// default

				zstruct(tmp);

				if (cmd == IDC_SET_MAX_ALLOWABLE_BLOCK) {

					value_to_set = 0;

					sprintf(tmp,"This will block out the usage\nof all these credit cards listed above");

				} else if (cmd == IDC_SET_MAX_ALLOWABLE_APPLY) {

					GetDlgItemText(hDlg, IDC_SET_MAX_ALLOWABLE_NUMBER, tmp, 3);

					value_to_set = atoi(tmp);

					sprintf(tmp,"This will set the number of different accounts\n"

						"that can use these credit cards to %d", value_to_set);

				} else {	// default is already set

					sprintf(tmp,"This will set the number of different accounts that\n"

						"can use these credit cards to the current default");

				}

				// now send it to the server...

				int result = MessageBox(hDlg, tmp, "Are you sure?",

					MB_YESNO|MB_APPLMODAL|MB_TOPMOST);

				if (result==IDYES) {

					struct MiscClientMessage mcm;

					zstruct(mcm);

					mcm.message_type = MISC_MESSAGE_SET_MAX_ALLOWABLE_USAGE;

					mcm.misc_data_1 = ArToView.sdb.player_id;

					mcm.misc_data_2 = (WORD32)value_to_set;

					SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				}

			}

			return TRUE;



	  #endif

		case IDOK:

		case IDCANCEL:

			EndDialog(hDlg, LOWORD(wParam));

		}

		return TRUE;	// We DID process this message.

	case WM_DESTROY:

	  #if ADMIN_CLIENT

		if (hEditCreditableDLG) {	// don't keep this around

			DestroyWindow(hEditCreditableDLG);

		}

	  #endif

		hViewTransactionsDLG = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}





/**********************************************************************************

 Function SetSubmitButtonState()

 Date: 20010224HK

 Purpose: set the state of the submit button on the purchase dialog

***********************************************************************************/

void SetSubmitButtonState(HWND hDlg)

{

	char tmp[CCFIELD_LONG+1];

	/* marked by rgong, deal with it later... 03/15/2002

	char tmp1[CCFIELD_LONG+4];

	zstruct(tmp);

	zstruct(tmp1);

	GetDlgText(hDlg, IDC_CARD_NUMBER, tmp1, CCFIELD_LONG);

	int k=0;

	for(int i=0; i<strlen(tmp1);i++){

		if(tmp1[i]!=' '){

			tmp[k]=tmp1[i];

			k++;

		}

	}

	*/

	int valid_cc_num = (strlen(tmp) >= 12);

	GetDlgText(hDlg, IDC_CARD_EXP_MONTH, tmp, CCFIELD_SHORT);

	int valid_month = (strlen(tmp) > 1);

	GetDlgText(hDlg, IDC_CARD_EXP_YEAR, tmp, CCFIELD_SHORT);

	int valid_year = (strlen(tmp) > 1);

	GetDlgText(hDlg, IDC_AMOUNT, tmp, CCFIELD_SHORT);

	int valid_amount = (atoi(tmp) >= 50);

  #if ADMIN_CLIENT

	// for admin (testing), any purchase amount is ok

	if (iAdminClientFlag) {

		valid_amount = TRUE;

	}

  #endif

	int valid_purchase = (valid_cc_num && valid_month && valid_year && valid_amount);

	//for Paypal purchase

	if (IsDlgButtonChecked(hDlg, IDC_RADIO_PAYPAL)) {

		valid_purchase = valid_amount;

	}

	// no we know if it's ok, set the state of the button

	EnableWindow(GetDlgItem(hDlg, IDOK), valid_purchase);

}



static char *FirePayToolTip = "FirePay is the best way to pay. It's free, safe and easy. Sign up now!";

static struct DlgToolTipText PurchDlgToolTipText[] = {

	IDC_BUTTON_FIREPAYSETUP, 	FirePayToolTip,

	IDC_RADIO_FIREPAY, 			FirePayToolTip,

	IDC_STATIC_FIREPAY, 		FirePayToolTip,

	IDC_BUTTON_FIREPAY_INFO,	"More infomation about FirePay",

	IDC_LOCK_ICON, 				"Communications with the server are encrypted securely",

	IDC_CHKBOX_SAVE_CARD_INFO,	"Check here to save this card number information for next time",

	0,0

};

static HWND hPurchDlgToolTips;



/**********************************************************************************

 Function CALLBACK dlgFuncAccountCharge(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK10/4/1999

 Purpose: handle a credit card purchase request

***********************************************************************************/

BOOL CALLBACK dlgFuncAccountCharge(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	struct AccountRecord ar;

	switch (message) {	

	case WM_INITDIALOG:

		hCCPurchaseDLG = hDlg;

		ScaleDialogTo96dpi(hDlg);

		hPurchDlgToolTips = OpenToolTipWindow(hDlg, PurchDlgToolTipText);

		WinRestoreWindowPos(ProgramRegistryPrefix, "Purchase", hDlg, NULL, NULL, FALSE, TRUE);

		{

			// Set the bitmap on the lock icon button

			HBITMAP hbm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_LOCK), IMAGE_BITMAP, 0,0,0);

			//kp(("%s(%d) hbm = $%08lx\n", _FL, hbm));

			SendMessage(GetDlgItem(hDlg, IDC_LOCK_ICON), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);

			// Set the FirePayLogo on the FirePay button

			//hbm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PDLG_FIREPAY), IMAGE_BITMAP, 0,0,0);

			//SendMessage(GetDlgItem(hDlg, IDC_BUTTON_FIREPAY_LOGO), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);

			// Set the two radio button bitmaps

			//hbm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PDLG_VISA), IMAGE_BITMAP, 0,0,0);

			//SendMessage(GetDlgItem(hDlg, IDC_BUTTON_VISA), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);

			//hbm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PDLG_MASTERCARD), IMAGE_BITMAP, 0,0,0);

			//SendMessage(GetDlgItem(hDlg, IDC_BUTTON_MASTERCARD), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);

			//hbm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PDLG_FIREPAY), IMAGE_BITMAP, 0,0,0);

			//SendMessage(GetDlgItem(hDlg, IDC_RADIO_FIREPAY), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);



			// 20010224HK: credit card expiry combo added

			// months

			//SendMessage(GetDlgItem(hDlg, IDC_RADIO_FIREPAY), BM_SETCHECK, BST_CHECKED, 0);

			SetDlgItemText(hDlg, IDC_MSGTEXT, "e-Media Poker recommends that you choose Firepay or Paypal as a way to make VISA, MasterCard or Bank account transfer deposits into your Real Money Account. Visa and MasterCard Purchases are charged a 5.5% processing fee which can be refunded through our fee refund program.");

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

			CheckDlgButton(hDlg, IDC_RADIO_PAYPAL, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_NUMBER), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_MONTH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CHKBOX_SAVE_CARD_INFO), SW_SHOW);

			//SetSubmitButtonState(hDlg);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), "Firepay Number:");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), "Firepay Expiry: ");

			SetFocus(GetDlgItem(hDlg, IDC_CARD_NUMBER));	// next item should be selected (credit card number)





			char tmp[10];

			zstruct(tmp);

			HWND combo = GetDlgItem(hDlg, IDC_CARD_EXP_MONTH);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0);

			for (int i=1; i < 13; i++) {

				sprintf(tmp, "%02d", i);

		        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)tmp);

			}

			SendMessage(combo, CB_SETCURSEL, (WPARAM)20, 0);	// we set it to 20, which is out of range (invalid)

			// years											

			zstruct(tmp);

			combo = GetDlgItem(hDlg, IDC_CARD_EXP_YEAR);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0);

			for (int j=1; j < 11; j++) {

				sprintf(tmp, "%4d", j+2000);

		        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)tmp);

			}

			SendMessage(combo, CB_SETCURSEL, (WPARAM)20, 0);



		}

			



	  #if ADMIN_CLIENT

	    if (iAdminClientFlag) {

			//ShowWindow(GetDlgItem(hDlg, IDC_CARD_TEST1), SW_SHOW);

			//ShowWindow(GetDlgItem(hDlg, IDC_CARD_TEST2), SW_SHOW);

			//ShowWindow(GetDlgItem(hDlg, IDC_CARD_TEST3), SW_SHOW);

	    }

	  #endif		

		char str[500];

		zstruct(str);

		ar = LoggedInAccountRecord;	// fill in defaults

		sprintf(str, 

					"%s\n"

					"%s\n"

					"%s%s%s"

					"%s, %s, %s\n"

					"%s",

					ar.sdb.full_name,

					ar.sdb.mailing_address1, "",

					ar.sdb.mailing_address2,

					ar.sdb.mailing_address2[0] ? "\n" : "",

					ar.sdb.city,

					ar.sdb.mailing_address_state,

					ar.sdb.mailing_address_country,

					ar.sdb.mailing_address_postal_code

		);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_MailingAddress, str);



		sprintf(str,

					"e-Media Poker-Ecash charges a %.02f%% processing fee.  "

					"See the cashier screen or our web site for automatic fee "

					"refund details.  Some banks may treat this transaction as "

					"a cash advance.",

			SavedAccountInfo.cc_purchase_fee_rate / 100.0);

		//SetDlgItemTextIfNecessary(hDlg, IDC_PURCHASE_FEE, str);



		// Display their buy-in limits

		if (MiscMsgCCBuyinLimits.msg[0]) {

			SetDlgItemTextIfNecessary(hDlg, IDC_BUYIN_LIMITS, MiscMsgCCBuyinLimits.msg);

		}



		if (iCCProcessingEstimate >= 20) {

			char str[300];

			zstruct(str);

			sprintf(str,

					"Credit card approvals are currently taking unusually long\n"

					"to process.\n"

					"\n"

					"At present, the wait is expected to be %d minutes.\n"

					"\n"

					"Delays such as this one are rare and we make every effort to\n"

					"ensure they are resolved quickly."

					, iCCProcessingEstimate);

			MessageBox(hCashierDlg,

					str,

					"The cashier is very busy...",

					MB_OK|MB_APPLMODAL|MB_TOPMOST);

		}



		// Have they used firepay before?

		/* SDBRECORD_FLAG_USED_FIREPAY never been used since change on 04/09/2002 by rgong

		if (LoggedInAccountRecord.sdb.flags & SDBRECORD_FLAG_USED_FIREPAY) {

			// Change button text from "Set up FirePay account" to something

			// more relevant for a second time user.

			SetDlgItemTextIfNecessary(hDlg, IDC_BUTTON_FIREPAYSETUP,

					"Go to FirePay\n"

					"    web site");

		}

		*/



		// Handle IDC_CHKBOX_SAVE_CARD_INFO

		if (Defaults.save_cc_information_flag) {

			CheckDlgButton(hDlg, IDC_CHKBOX_SAVE_CARD_INFO, TRUE);

			char str[CCFIELD_LONG];

			UnObfuscateString(Defaults.saved_cc_number, str, CCFIELD_LONG);

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_NUMBER, str);

			UnObfuscateString(Defaults.saved_cc_month, str, CCFIELD_LONG);

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_MONTH, str);

			UnObfuscateString(Defaults.saved_cc_year, str, CCFIELD_LONG);

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_YEAR, str);

			switch (Defaults.saved_cc_type) {

			case CCTYPE_VISA:

				CheckDlgButton(hDlg, IDC_RADIO_VISA, TRUE);

				break;

			case CCTYPE_MASTERCARD:

				CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, TRUE);

				break;

			case CCTYPE_FIREPAY:

				CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

				break;

			}

		}

		SetSubmitButtonState(hDlg);

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "Purchase", hDlg, NULL);

		hCCPurchaseDLG = NULL;

		CloseToolTipWindow(hPurchDlgToolTips);		// Close our tooltip window

		break;



    case WM_DRAWITEM:

		// Owner draw control... draw it.

		{

			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			int bitmap_index = 0;

			switch (dis->CtlID) {

			case IDC_BUTTON_PAYPAL_LOGO:

				bitmap_index = MISCBITMAP_CASHIER_PAYPAL;

		        //MISCBITMAP_CASHIER_VISA_MASTERCARD,;

				break;

			case IDC_BUTTON_FIREPAY_LOGO:

				bitmap_index = MISCBITMAP_CASHIER_FIREPAY;

				break;			

			case IDC_BUTTON2:

				bitmap_index = MISCBITMAP_CASHIER_WESTERN_UNION;

				break;

			case IDC_BUTTON3:

				bitmap_index = MISCBITMAP_CASHIER_BANK_DRAFT;

				break;

			case IDC_BUTTON_VISA:

				bitmap_index = MISCBITMAP_CASHIER_VISA;

				break;

			case IDC_BUTTON_MASTERCARD:

				bitmap_index = MISCBITMAP_CASHIER_MASTERCARD;

				break;

			case IDC_ABOUT_FIREPAY:

				bitmap_index = MISCBITMAP_CASHIER_ABOUTFIREPAY;

				break;

			case IDC_SIGN_FIREPAY:

				bitmap_index = MISCBITMAP_CASHIER_SIGNFIREPAY;

				break;

			case IDC_ABOUT_PAYPAL:

				bitmap_index = MISCBITMAP_CASHIER_ABOUTPAYPAL;

				break;

			case IDC_TRANSACTION_HISTORY:

				bitmap_index = MISCBITMAP_CASHIER_HISTORY_UP;

				break;

			case IDCANCEL:

				bitmap_index = MISCBITMAP_CASHIER_LEAVE_UP;

				break;

			}

			if (dis->hwndItem==CashierHighlightList.hCurrentHighlight) {

				// The mouse is over this button...

				bitmap_index++;

			}

			DrawButtonItemWithBitmap(MiscBitmaps[bitmap_index], dis, bitmap_index);

		}

		return TRUE;	//    

	

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

	  #if ADMIN_CLIENT

		case IDC_CARD_TEST1:		

		case IDC_CARD_TEST2:		

		case IDC_CARD_TEST3:		

			if (LOWORD(wParam) == IDC_CARD_TEST1) {	// first set of test data

				SetDlgItemText(hDlg, IDC_CARD_NUMBER, "4050259000500040");

				CheckDlgButton(hDlg, IDC_RADIO_VISA, TRUE);

				CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, FALSE);

				SetDlgItemText(hDlg, IDC_AMOUNT, "5");	// amount (in $)

				SetDlgItemText(hDlg, IDC_CARD_NAME, "Fernando Valenzuela");

				SetDlgItemText(hDlg, IDC_CARD_EXP_MONTH, "12");

				SetDlgItemText(hDlg, IDC_CARD_EXP_YEAR, "2002");

			} else if (LOWORD(wParam) == IDC_CARD_TEST2) {	// second set of test data

				SetDlgItemText(hDlg, IDC_CARD_NUMBER, "5151301400246102");

				CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, TRUE);

				CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, FALSE);

				SetDlgItemText(hDlg, IDC_AMOUNT, "50");	// amount (in $)

				SetDlgItemText(hDlg, IDC_CARD_NAME, "Fernando Valenzuela");

				SetDlgItemText(hDlg, IDC_CARD_EXP_MONTH, "12");

				SetDlgItemText(hDlg, IDC_CARD_EXP_YEAR, "2002");

			} else if (LOWORD(wParam) == IDC_CARD_TEST3) {	// third set of test data

				CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

				SetDlgItemText(hDlg, IDC_AMOUNT, "3");	// amount (in $)

				SetDlgItemText(hDlg, IDC_CARD_NAME, "Fernando Fireuela");

				SetDlgItemText(hDlg, IDC_CARD_NUMBER, "6015580202908480");

				SetDlgItemText(hDlg, IDC_CARD_EXP_MONTH, "02");

				SetDlgItemText(hDlg, IDC_CARD_EXP_YEAR, "2003");

			}

			SetSubmitButtonState(hDlg);

			break;

	  #endif



		case IDC_LOCK_ICON:

			LaunchInternetBrowser(BASE_URL"secureconnection.php");

			break;



		case IDC_RADIO_VISA:

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_PAYPAL, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_SLASH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_NUMBER), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_MONTH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CHKBOX_SAVE_CARD_INFO), SW_HIDE);

			//CheckDlgButton(hDlg, IDC_CHKBOX_SAVE_CARD_INFO, FALSE);

			SetSubmitButtonState(hDlg);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), "Visa Number:");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), "Visa Expiry:");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_NUMBER, "");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_MONTH, "");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_YEAR, "");

			SetFocus(GetDlgItem(hDlg, IDC_CARD_NUMBER));	// next item should be selected (credit card number)

			break;

		case IDC_RADIO_MASTERCARD:

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_PAYPAL, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_SLASH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_NUMBER), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_MONTH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CHKBOX_SAVE_CARD_INFO), SW_HIDE);

			//CheckDlgButton(hDlg, IDC_CHKBOX_SAVE_CARD_INFO, FALSE);

			SetSubmitButtonState(hDlg);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), "MasterCard Number:");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), "MasterCard Expiry: ");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_NUMBER, "");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_MONTH, "");

			SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_YEAR, "");

			SetFocus(GetDlgItem(hDlg, IDC_CARD_NUMBER));	// next item should be selected (credit card number)

			break;

		case IDC_RADIO_FIREPAY:

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_PAYPAL, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_SLASH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_NUMBER), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_MONTH), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR), SW_SHOW);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CHKBOX_SAVE_CARD_INFO), SW_SHOW);

			SetSubmitButtonState(hDlg);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), "Firepay Number:");

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), "Firepay Expiry: ");

			if (Defaults.save_cc_information_flag) {

				CheckDlgButton(hDlg, IDC_CHKBOX_SAVE_CARD_INFO, TRUE);

				char str[CCFIELD_LONG];

				UnObfuscateString(Defaults.saved_cc_number, str, CCFIELD_LONG);

				SetDlgItemTextIfNecessary(hDlg, IDC_CARD_NUMBER, str);

				UnObfuscateString(Defaults.saved_cc_month, str, CCFIELD_LONG);

				SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_MONTH, str);

				UnObfuscateString(Defaults.saved_cc_year, str, CCFIELD_LONG);

				SetDlgItemTextIfNecessary(hDlg, IDC_CARD_EXP_YEAR, str);

			}



			SetFocus(GetDlgItem(hDlg, IDC_CARD_NUMBER));	// next item should be selected (credit card number)

			break;

		case IDC_RADIO_PAYPAL:

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT1), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TEXT2), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_SLASH), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_NUMBER), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_MONTH), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR), SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CHKBOX_SAVE_CARD_INFO), SW_HIDE);

			//CheckDlgButton(hDlg, IDC_CHKBOX_SAVE_CARD_INFO, FALSE);

			SetSubmitButtonState(hDlg);

			SetFocus(GetDlgItem(hDlg, IDC_AMOUNT));	// next item should be selected (paypal amount)

			break;



	  #if 0	//20010313MB

		case IDC_RADIO_FIREPAY:

			// adjust radio buttons as well

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

			SetSubmitButtonState(hDlg);

			SetFocus(GetDlgItem(hDlg, IDC_CARD_NUMBER));	// next item should be selected (credit card number)

			break;

	  #endif



		case IDC_ABOUT_FIREPAY:

			LaunchInternetBrowser("http://www.firepay.com/about/index.shtml");

			break;

		case IDC_SIGN_FIREPAY:

			LaunchInternetBrowser("http://www.firepay.com/about/faq3.shtml");

			break;

		case IDC_ABOUT_PAYPAL:

			LaunchInternetBrowser("https://www.paypal.com");

			break;

		/*

		case IDC_BUTTON_FIREPAYSETUP:

			CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

			SetSubmitButtonState(hDlg);

			// Have they used firepay before?

			if (LoggedInAccountRecord.sdb.flags & SDBRECORD_FLAG_USED_FIREPAY) {

				LaunchInternetBrowser("http://www.firepay.com");

			} else {

				LaunchInternetBrowser("http://signup.firepay.com");

			}

			break;

		*/

		case IDC_BUTTON1:

			DialogBox(hInst, MAKEINTRESOURCE(IDD_PAYPAL), hDlg, dlgFuncPayPal);

           	EndDialog(hDlg, IDOK);

            break; 





		case IDC_BUTTON2:

			DialogBox(hInst, MAKEINTRESOURCE(IDD_Western_Union), hDlg, dlgFuncWesternUnion);

            break; 



		case IDC_BUTTON3:

			DialogBox(hInst, MAKEINTRESOURCE(IDD_Bank_Draft), hDlg, dlgFuncBankDrafts);

            break; 

        case IDC_BUTTON4:

			LaunchInternetBrowser(BASE_URL"real_money.php");

			break;



		case IDC_BUTTON_FIREPAY_INFO:

			{

				CheckDlgButton(hDlg, IDC_RADIO_VISA, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_MASTERCARD, FALSE);

				CheckDlgButton(hDlg, IDC_RADIO_FIREPAY, TRUE);

				HRESULT result = MessageBox(hDlg,

					"Three easy steps to using FirePay:\n"

					"\n"

					"Step 1:\tClick 'Set up your FirePay account' and sign up for your FirePay\n"

					"\taccount on the FirePay web site.\n"

					"\n"

					"Step 2:\tDeposit funds into your FirePay account using your credit card.\n"

					"\n"

					"Step 3:\tBuy e-Media Poker  chips from the cashier screen using your FirePay\n"

					"\taccount number.\n"

					"\n"

					"Would you like to go to the FirePay web site now?",

					"FirePay info...",

					MB_YESNO|MB_ICONINFORMATION);

				if (result==IDYES) {

					LaunchInternetBrowser("http://signup.firepay.com");

				}

			}

			break;



		case IDC_CARD_EXP_MONTH:

			pr(("mess = %08x, HIWORD(wParam) = %d, LOWORD(wParam) = %d, HIWORD(lParam) = %d, LOWORD(lParam) = %d\n",

				message, HIWORD(wParam),LOWORD(wParam),HIWORD(lParam),LOWORD(lParam)));

			// 8 was picked as the message to trap because we get one of those when a selection is made from the dropdown

			// but not when a selection is made via keyboard.  We don't want this jumping to the next thing because another

			// keystroke may be forthcoming

			if (HIWORD(wParam) == 8) {

				SetSubmitButtonState(hDlg);

				SetFocus(GetDlgItem(hDlg, IDC_CARD_EXP_YEAR));	// next item should be selected

			}

			break;



		case IDC_CARD_EXP_YEAR:

			if (HIWORD(wParam) == 8) {

				SetSubmitButtonState(hDlg);

				SetFocus(GetDlgItem(hDlg, IDC_AMOUNT));	// next item should be selected

			}

			break;



		case IDC_CARD_NUMBER:

			char cc_tmp[CCFIELD_LONG+4];

			zstruct(cc_tmp);

			GetDlgText(hDlg, IDC_CARD_NUMBER, cc_tmp, CCFIELD_LONG);



			if (strlen(cc_tmp)>16){

				keybd_event( VK_BACK,

                      0x45,

                      KEYEVENTF_EXTENDEDKEY | 0,

                      0 );



				// Simulate a key release

				//keybd_event( VK_BACK,

                //      0x45,

                //      KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,

                //      0);

			}

			break;

			

		case IDC_AMOUNT:

			if (HIWORD(wParam)==EN_CHANGE) {

				// contents have changed

				SetSubmitButtonState(hDlg);

			}

			break;



		case IDOK:

			// read and post if verified

			HWND hSubmitProgressDlg;

			hSubmitProgressDlg = NULL;

			CCTransaction cct;

			zstruct(cct);

			char card_type[25];

			zstruct(card_type);

			char str[500];

			zstruct(str);

			char curr_str[MAX_CURRENCY_STRING_LEN];

			zstruct(curr_str);

			int num, rc;

			if (IsDlgButtonChecked(hDlg, IDC_RADIO_VISA)) {

				cct.card_type = CCTYPE_VISA;

				strcpy(card_type, "VISA");

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_MASTERCARD)) {

				cct.card_type = CCTYPE_MASTERCARD;

				strcpy(card_type, "MasterCard");

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_FIREPAY)) {

				cct.card_type = CCTYPE_FIREPAY;

				strcpy(card_type, "FirePay");

			} else {

				cct.card_type = CCTYPE_UNKNOWN;

				strcpy(card_type, "UNKNOWN CARD TYPE");

			}

			cct.transaction_type = CCTRANSACTION_FIREPAY_PURCHASE;

			if (IsDlgButtonChecked(hDlg, IDC_RADIO_PAYPAL)) {

				cct.transaction_type = CCTRANSACTION_PURCHASE;

				strcpy(card_type, "PAYPAL");

			}

		  #if 0	// unused field for now

			GetDlgText(hDlg, IDC_CARD_NAME, cct.card_name, CCFIELD_LONG);

			GetDlgText(hDlg, IDC_CARD_NAME, cct.user_id, CCFIELD_LONG);	/// !!!

		  #endif

			if (cct.card_type==CCTYPE_UNKNOWN){

				sprintf(cct.card_number, "%s", "");

				sprintf(cct.card_exp_month, "%s", "");

				sprintf(cct.card_exp_year, "%s", "");

			} else {

				GetDlgText(hDlg, IDC_CARD_NUMBER, cct.card_number, CCFIELD_LONG);

				GetDlgText(hDlg, IDC_CARD_EXP_MONTH, cct.card_exp_month, CCFIELD_SHORT);

				GetDlgText(hDlg, IDC_CARD_EXP_YEAR, cct.card_exp_year, CCFIELD_SHORT);

			}

			GetDlgText(hDlg, IDC_AMOUNT, cct.amount, CCFIELD_SHORT);



			sprintf(samount,"%s", cct.amount);  //smount is used by paypal web page, because dlgFuncPayPal

												//is no longer used



			num = StringToChips(cct.amount);	// make +ve and rounded (it's in pennies for ecash)

			if (!num) {	// it's 0

				return TRUE;	// We DID process this message.

			}

			sprintf(cct.amount,"%d",num);

			

			if (cct.card_type!=CCTYPE_UNKNOWN){

			if (strlen(cct.card_number) < 8) {

				return TRUE;

			}

			// !!!! some local validation for valid card numbers, etc should go here

			// do some fiddling with what was entered to make it look better

			num = atoi(cct.card_exp_month);

			if (num < 1 || num > 12) {	// no good

				return TRUE;

			}

			sprintf(cct.card_exp_month,"%02d",num);

			num = atoi(cct.card_exp_year);

			// do some more validation on the year eventually



		  #if 0	// 20010224HK

			// remove all this in a few months

			num = num % 100;

			if (num > 95) {

				num += 1900;

			} else {

				num += 2000;

			}

		  #endif

			sprintf(cct.card_exp_year,"%d",num);

		}

			// will normally be a 4-digit year

			ar = LoggedInAccountRecord;	// fill in defaults

			//rgong Marked out this confirm dialog

			/*

			sprintf(str, 

				"%s\n"

				"%s\n"

				"%s%s%s"

				"%s, %s, %s\n"

				"%s\n\n"

				"Card type: %s\n"

				"Card number:%s\n"

				"Exp: %s / %s\n"

				"Purchase amount: %s",

				ar.sdb.full_name,

				ar.sdb.mailing_address1, "",

				ar.sdb.mailing_address2,

				ar.sdb.mailing_address2[0] ? "\n" : "",

				ar.sdb.city,

				ar.sdb.mailing_address_state,

				ar.sdb.mailing_address_country,

				ar.sdb.mailing_address_postal_code,

				card_type, cct.card_number,

				cct.card_exp_month, cct.card_exp_year,

				CurrencyString(curr_str, atoi(cct.amount), CT_REAL, TRUE)

			);



			rc = MessageBox(hDlg, str, "Verify Credit Card Purchase...", 

				MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

			if (rc == IDOK) {	// post it

			*/

				SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

				

				// fluff: progress bar

				hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Chip Purchase Request to server...",

					60,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %

	

				if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 200);

				}



				// Save the card number if the user requested it,

				// otherwise blank it out.

				if (cct.card_type==CCTYPE_FIREPAY) {

				Defaults.saved_cc_type = 0;

					zstruct(Defaults.saved_cc_number);

					zstruct(Defaults.saved_cc_month);

					zstruct(Defaults.saved_cc_year);

					Defaults.save_cc_information_flag = (BYTE8)IsDlgButtonChecked(hDlg, IDC_CHKBOX_SAVE_CARD_INFO);

					if (Defaults.save_cc_information_flag) {

					Defaults.saved_cc_type = (BYTE8)cct.card_type;

					ObfuscateString(cct.card_number,    Defaults.saved_cc_number, CCFIELD_LONG);

					ObfuscateString(cct.card_exp_month, Defaults.saved_cc_month,  CCFIELD_SHORT);

					ObfuscateString(cct.card_exp_year,  Defaults.saved_cc_year,   CCFIELD_SHORT);

				}

				Defaults.changed_flag = TRUE;

				WriteDefaults(FALSE);

				}

				//EndDialog(hDlg, IDOK);

			//rgong, contnue marking out the confirm dialog

			/*

			} else { 

				// hit CANCEL, may want to correct something -- don't abort

			}

			*/

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			rc = MessageBox(hDlg, "Are you sure you want to cancel this transaction?",

				"Cancel Transaction...", 

				MB_YESNO|MB_APPLMODAL|MB_TOPMOST);

			if (rc == IDYES) {

				EndDialog(hDlg, IDCANCEL);



				//20010313MB:

				// Special case: if they unchecked the 'save card info' checkbox

				// and they're leaving the dialog box, clear the card info

				// even though they hit cancel, otherwise there's no way at all

				// for the user to clear the card info without doing a purchase.

				if (!IsDlgButtonChecked(hDlg, IDC_CHKBOX_SAVE_CARD_INFO)) {

					// It's cleared.  Make sure everything gets cleared.

					Defaults.save_cc_information_flag = FALSE;

					Defaults.saved_cc_type = 0;

					zstruct(Defaults.saved_cc_number);

					zstruct(Defaults.saved_cc_month);

					zstruct(Defaults.saved_cc_year);

					Defaults.changed_flag = TRUE;

					WriteDefaults(FALSE);

				}

			}

			return TRUE;	// We DID process this message

		}

		break;

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



/**********************************************************************************

 BOOL CALLBACK dlgFuncEnterPhoneNumber()

 Date: 20010226HK

 Purpose: allow user to enter his phone number

***********************************************************************************/

BOOL CALLBACK dlgFuncEnterPhoneNumber(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		// the struct holds 8 bytes (PHONE_NUM_LEN) encoded, meaning 16 actual characters

		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PHONE_NUMBER), EM_LIMITTEXT, PHONE_NUM_EXPANDED_LEN, 0L);

		hEnterPhoneNumberDLG = hDlg;

		return TRUE;



	case WM_DESTROY:

		hEnterPhoneNumberDLG = NULL;

		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDC_EDIT_PHONE_NUMBER:

			if (HIWORD(wParam)==EN_CHANGE) {

				// contents have changed

				FixUpPhoneNumberEditControl(hDlg);

				char str[CCFIELD_LONG];

				zstruct(str);

				GetDlgItemText(hDlg, IDC_EDIT_PHONE_NUMBER, str, CCFIELD_LONG);

				EnableWindow(GetDlgItem(hDlg, IDOK), strlen(str) >= 7);	// assume it's ok if 7 digits

			}

			break;

		case IDOK:

			struct MiscClientMessage mcm;

			zstruct(mcm);

			mcm.message_type = MISC_MESSAGE_SEND_PHONE_NUMBER;

			char str[CCFIELD_LONG+1];

			zstruct(str);

			GetDlgItemText(hDlg, IDC_EDIT_PHONE_NUMBER, mcm.msg, CCFIELD_LONG);	// dealing with it will take place

			SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));



			EndDialog(hDlg, IDOK);

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			return TRUE;	// We DID process this message

		}

		break;

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



/**********************************************************************************

 Function CALLBACK dlgFuncAccountCredit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 Date: HK11/10/1999

 Purpose: handle a cashout request

***********************************************************************************/

BOOL CALLBACK dlgFuncAccountCredit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[CCFIELD_LONG];

	int credit_guess = 0, i;

	static guess;

	switch (message) {

	case WM_INITDIALOG:

		if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

			ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ADMIN), SW_SHOW);

			ShowWindow(GetDlgItem(hDlg, IDC_EDIT_CREDITADMIN), SW_SHOW);

		}

		hCCCreditDLG = hDlg;



		SetDlgItemTextIfNecessary(hDlg, IDC_TEXT_1, "This page is used for requesting a Real Money cash out from your e-Media Poker Account.\n\nIf you made deposits using Firepay and Visa Credit Card, these deposits will be credited first, on a first deposited, first credited basis. The net amount for these types of deposits is displayed in the \"Real Money available for direct credit\" field.");

		SetDlgItemTextIfNecessary(hDlg, IDC_TEXT_2, "For Mastercard, PayPal, poker game bonuses and winnings and other deposit types you will be given the choice of receiving your Cash-Out via PayPal or by Check. Cash-Out's must be greater than $50 and are limited to one per week. If you choose to receive your Cash-Out by Check, you may incur fees from your bank and it will arrive in approximately 10-15 days. We recommend that you use PayPal for your Cash-Out's as you will not incur any fees and will receive your funds within 1-3 days.\n\nIf you think you may be depositing money back into your e-Media Poker account in the near future, you may want to consider leaving the money in your Player Account.");

		for (i= TRANS_TO_RECORD_PER_PLAYER-1; i>=0; i--) {

		   if (LoggedInAccountRecord.sdb.transaction[i].credit_left >0 ) {

				credit_guess += LoggedInAccountRecord.sdb.transaction[i].credit_left;

			}

		}



		SetDlgItemTextIfNecessary(hDlg, IDC_CREDIT_GUESS, 

			CurrencyString(str, credit_guess, CT_REAL, TRUE));

		guess= credit_guess;

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_BANK, 

			CurrencyString(str, RealChipsInBank, CT_REAL, TRUE));

		//rgong

		char epts_str[8];

		sprintf(epts_str, "%d", CreditFeePoints);

		SetDlgItemTextIfNecessary(hDlg, IDC_EPTS2, epts_str);

		//end rgong

			

		return TRUE;



	case WM_DESTROY:

		hCCCreditDLG = NULL;

		break;

	

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

	

		case IDOK:

           

			// read and post if verified

			HWND hSubmitProgressDlg;

			hSubmitProgressDlg = NULL;

			CCTransaction cct;

			zstruct(cct);

			if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

				// GetDlgItemText(hDlg, IDC_EDIT_CREDITADMIN, str, CCFIELD_LONG);

				// cct.admin_transaction_number_override = atoi(str);

			}

			cct.transaction_type = CCTRANSACTION_FIREPAY_CASHOUT;

			if (cct.admin_transaction_number_override) {

				cct.transaction_type = CCTRANSACTION_ADMIN_CREDIT;

			}

			GetDlgItemText(hDlg, IDC_AMOUNT, cct.amount, CCFIELD_SHORT);

			// get the credit amount

			long amount;

			amount = StringToChips(cct.amount);	// make +ve and rounded (it's in pennies for ecash)

			// bound its range if it's non admin

			if (cct.transaction_type != CCTRANSACTION_ADMIN_CREDIT) {

				amount = min(amount, (int)RealChipsInBank);

				// if (amount < 100) {	// less than $1, dismiss

				//  	return TRUE;	// We DID process this message.

				// }

			}

			

			cashout_amount= amount;

			

			if (cashout_amount >0 && cashout_amount <= credit_guess ) {

			

				sprintf(cct.amount, "%d", amount);

				char str[100];

				zstruct(str);

				char curr_str[MAX_CURRENCY_STRING_LEN];

				if (cct.transaction_type == CCTRANSACTION_ADMIN_CREDIT) {

				sprintf(str,"Request credit against transaction %d for %s", 

					cct.admin_transaction_number_override, CurrencyString(curr_str, amount, CT_REAL, TRUE) );

				} else {

				sprintf(str,"Request credit for %s", CurrencyString(curr_str, amount, CT_REAL, TRUE) );

				}

			

				sprintf(cct.amount, "%d", amount);

				// fluff: progress bar

				hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Cash Out request to server...",

					60,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %



				if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 200);

				}

				

				sprintf(cct.unused , "paypal");

				//sprintf(cct.unused , "check");



				SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

				EndDialog(hDlg, IDOK);	// get rid of the original dlg

				return TRUE;	// We DID process this message.



			}  

            

			/*if (cashout_amount < 5000) {

			  DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_DECLINE_SMALL1), hDlg, dlgFuncWesternUnion);

			  return TRUE;

			}

            */



		     if (LoggedInPrivLevel < 50 ) {

				 // allen ko 2002-04-10

				if ((cashout_amount <  (guess  + 5000)) &&(cashout_amount >= guess )) {

                 // allen ko 2002-04-10

					DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_DECLINE_SMALL2), hDlg, dlgFuncWesternUnion);

					return TRUE;

				}

			}



			

			if (LoggedInPrivLevel < 50 )	{

               if (cashout_amount >= (guess + 5000)  ) {  

			        DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREDIT1), hDlg, dlgFuncAccountCredit_1);

					EndDialog(hDlg, IDOK);	// get rid of the original dlg

					return TRUE;	// We DID process this message.

			

				}

			}

			else	{

				if ( cashout_amount > guess   ) {  

						DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREDIT1), hDlg, dlgFuncAccountCredit_1);

						EndDialog(hDlg, IDOK);	// get rid of the original dlg

						return TRUE;	// We DID process this message.

				}

			}

            

			sprintf(cct.amount, "%d", amount);

			char str[100];

			zstruct(str);

			char curr_str[MAX_CURRENCY_STRING_LEN];

			if (cct.transaction_type == CCTRANSACTION_ADMIN_CREDIT) {

				sprintf(str,"Request credit against transaction %d for %s", 

					cct.admin_transaction_number_override, CurrencyString(curr_str, amount, CT_REAL, TRUE) );

			} else {

				sprintf(str,"Request credit for %s", CurrencyString(curr_str, amount, CT_REAL, TRUE) );

			}

			

			sprintf(cct.amount, "%d", amount);

				// fluff: progress bar

			hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Cash Out request to server...",

					60,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %



			if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 200);

			}

				

			sprintf(cct.unused , "paypal");

            //sprintf(cct.unused , "check");



			SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

			EndDialog(hDlg, IDOK);	// get rid of the original dlg

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			int rc;

			rc = MessageBox(hDlg, "Are you sure you want to cancel this transaction?",

				"Cancel Transaction...", 

				MB_YESNO|MB_APPLMODAL|MB_TOPMOST);

			if (rc == IDYES) {

				EndDialog(hDlg, IDCANCEL);

			}

			return TRUE;	// We DID process this message

		}

		break;

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



/**********************************************************************************

 Function CALLBACK dlgCashierFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK10/3/1999

 Purpose: dlg function for cashier screen

***********************************************************************************/

BOOL CALLBACK dlgFuncCashier(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[400];

	// char str2[CCFIELD_LONG];

	zstruct(str);

	// int rc;

	switch (message) {

	case WM_INITDIALOG:

		hCashierDlg = hDlg;

		ScaleDialogTo96dpi(hDlg);

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(hDlg, WMP_UPDATE_REALBANK_CHIPS, 0, 0);

		SendMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		zstruct(CashierHighlightList);

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_PURCHASE_CHIPS));

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_CASH_CHIPS));

		//rgong 04/04/2002

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_DOWNLOAD1));

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_DOWNLOAD2));

		//end rgong

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_TRANSACTION_HISTORY));

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDC_SETUP_ACCOUNT));

		AddProximityHighlight(&CashierHighlightList, GetDlgItem(hDlg, IDCANCEL));

		WinRestoreWindowPos(ProgramRegistryPrefix, "Cashier", hDlg, NULL, NULL, FALSE, TRUE);

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

		// Do the same thing as clicking 'Leave Cashier'

		DestroyWindow(hDlg);

		return TRUE;	// message handled

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDC_SETUP_ACCOUNT:



			MakeAccountRealMoneyReady();
/*
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DISABLE_CASHIER), HWND_DESKTOP,

							(DLGPROC)dlgDisableCashierFunc, NULL);
							*/

			return TRUE;

			

		break;

	

		case IDC_PURCHASE_CHIPS:

		

		/*

			#if ALLOW_CC_FUNCTIONS

			if (hCCPurchaseDLG) {	

				ReallySetForegroundWindow(hCCPurchaseDLG);

			} else if ((ShotClockFlags & SCUF_CLOSE_CASHIER)

			  #if ADMIN_CLIENT

				&& !iAdminClientFlag

			  #endif

			) {

				DisplayCashierClosedMessage(hDlg);

			} else if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {

				// Invalid email address on file for this person...

				// Disallow this cashier function.

				DisplayBadEmailMessage(hDlg);

			} else {

				char str1[100];

				char str2[100];

				int rc;

						

				// 20000205HK: don't allow purchase if he's got pending check $

				if (PendingCheck || PendingPaypal) {	// ask for refund

					sprintf(str, "You have a pending check in the amount of %s that has not yet been mailed.\n\n"

						"and/or\n\nYou have a pending PayPal cashout in the amount of %s that has not yet been sent to you.\n\n"

						"If you wish to purchase chips, these amounts must first be refunded to you.\n\n"

						"If you still wish to receive a check or PayPal for a partial amount, you may cash out again.\n\n"

						"Proceed with refunds(s)?", 

						CurrencyString(str1, PendingCheck, CT_REAL, TRUE),

						CurrencyString(str2, PendingPaypal, CT_REAL, TRUE));

					rc = MessageBox(hDlg, str, "Refund Pending Check and/or PayPal...", 

						MB_YESNO|MB_APPLMODAL|MB_TOPMOST);

					if (rc == IDYES) {	// proceed with refund

						struct MiscClientMessage mcm;

						zstruct(mcm);

						mcm.message_type = MISC_MESSAGE_CASHOUT_REFUND;

						SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

					}

				} else {

					DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CHARGE), hDlg, dlgFuncAccountCharge);

				}

			}

		  #else

			DisplayNoCCFunctionsMessage();

		  #endif

		   

			//DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DISABLE_CASHIER), HWND_DESKTOP,

			//			(DLGPROC)dlgDisableCashierFunc, NULL);
						

				return TRUE;

		*/		

			//J Fonseca    16/01/2004
			LaunchInternetBrowser("https://www.e-Mediasoftware.com/cashier");
			return TRUE;	// We DID process this message.

/*			MessageBox(hDlg,"The Cashier is momentarily down \n","e-Media Poker",MB_OK); 
			return TRUE; */

			break;		



		case IDC_CASH_CHIPS:

/*          #if ALLOW_CC_FUNCTIONS

			if (hCCCreditDLG) {	

				ReallySetForegroundWindow(hCCCreditDLG);

			} else if ((ShotClockFlags & SCUF_CLOSE_CASHIER)

			  #if ADMIN_CLIENT

				&& !iAdminClientFlag

			  #endif

			) {

				DisplayCashierClosedMessage(hDlg);

			} else if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {

				// Invalid email address on file for this person...

				// Disallow this cashier function.

				DisplayBadEmailMessage(hDlg);

			} else {

				DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREDIT), hDlg, dlgFuncAccountCredit);

			}

		  #else

			DisplayNoCCFunctionsMessage();

		  #endif
/*
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DISABLE_CASHIER), HWND_DESKTOP,

						(DLGPROC)dlgDisableCashierFunc, NULL);
						*/

			
			//J Fonseca  16/01/2004
			LaunchInternetBrowser("https://www.e-Mediasoftware.com/cashier");
			return TRUE; 

			break;		



		case IDC_TRANSACTION_HISTORY:

			//DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DISABLE_CASHIER), HWND_DESKTOP,

			//			(DLGPROC)dlgDisableCashierFunc, NULL);

			//	return TRUE;

		  

/*JF
		  #if ALLOW_CC_FUNCTIONS

			ArToView = LoggedInAccountRecord;

			DialogBox(hInst, MAKEINTRESOURCE(IDD_VIEW_TRANSACTIONS), hDlg, dlgFuncViewTransactions);

		  #else

			DisplayNoCCFunctionsMessage();

		  #endif
JF*/
			//J Fonseca 16/01/2004
			LaunchInternetBrowser("https://www.e-Mediasoftware.com/cashier");
			return TRUE;

			break;

		case IDCANCEL:	// Close window?

			DestroyWindow(hDlg);

			break;

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

			HWND hwnd = (HWND)lParam;	// hwnd of window getting drawn

			if (hwnd==GetDlgItem(hDlg, IDC_REAL_TOTAL) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_BANK) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_PENDING) ||

				hwnd==GetDlgItem(hDlg, IDC_REAL_TABLE) ||

				hwnd==GetDlgItem(hDlg, IDC_CASHIER_PLAYERINFO) ||

				hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT) ||

				hwnd==GetDlgItem(hDlg, IDC_STAR)

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

				BlitBitmapRectToDC(hCashierBgnd, hdc, &pt, &r);

				if (hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT) && hCashierButtonFontSmall) {

					SelectObject(hdc, hCashierButtonFontSmall);

				} else if (hCashierButtonFont) {

					SelectObject(hdc, hCashierButtonFont);

				}

				if (hwnd==GetDlgItem(hDlg, IDC_CASHIER_PLAYERINFO) ||

					hwnd==GetDlgItem(hDlg, IDC_CCFEE_CREDIT_TEXT) ||

					hwnd==GetDlgItem(hDlg, IDC_STAR))

				{

				  #if 1	//19991111MB

					SetTextColor(hdc, RGB(255,255,255));	//*** change 11/09/01(255,235,110) draw text in brighter gold

				  #else

					SetTextColor(hdc, RGB(255,228,82));	// draw text in gold

				  #endif

				} else {

					SetTextColor(hdc, RGB(255,255,255));		//*** chamge 11/09/01(0,0,0) draw text in black

				}

				SetBkMode(hdc, TRANSPARENT);

				return ((int)GetStockObject(NULL_BRUSH));

			}

		}

		break;

	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "Cashier", hDlg, NULL);

		RemoveKeyboardTranslateHwnd(hDlg);

		CloseToolTipWindow(hCashierToolTips);	// Close our tooltip window

		hCashierDlg = NULL;

		return TRUE;	// TRUE = we DID process this message.

	case WM_DRAWITEM:

		// Owner draw control... draw it.

		{

			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			int bitmap_index = 0;

			switch (dis->CtlID) {

			case IDC_SETUP_ACCOUNT:

				bitmap_index = MISCBITMAP_CASHIER_SETUP_UP;

				break;

			case IDC_PURCHASE_CHIPS:

				bitmap_index = MISCBITMAP_CASHIER_BUY_UP;

				break;

			case IDC_CASH_CHIPS:

				bitmap_index = MISCBITMAP_CASHIER_CASHOUT_UP;

				break;

			//rgong 04/04/2002

			case IDC_DOWNLOAD1:

				bitmap_index = MISCBITMAP_CASHIER_DOWNLOAD1_UP;

				break;

			case IDC_DOWNLOAD2:

				bitmap_index = MISCBITMAP_CASHIER_DOWNLOAD2_UP;

				break;			

			//end rgong

			case IDC_TRANSACTION_HISTORY:

				bitmap_index = MISCBITMAP_CASHIER_HISTORY_UP;

				break;

			case IDCANCEL:

				bitmap_index = MISCBITMAP_CASHIER_LEAVE_UP;

				break;

			}

			if (dis->hwndItem==CashierHighlightList.hCurrentHighlight) {

				// The mouse is over this button...

				bitmap_index++;

			}

			DrawButtonItemWithBitmap(hCashierBgnd, dis, bitmap_index);

		}

		return TRUE;	// TRUE = we did process this message.

	case WM_ERASEBKGND:

		if (IsIconic(hDlg) || !hCashierBgnd) {

			return FALSE;	// FALSE = we did NOT process this message.

		}

		return TRUE;	// TRUE = we DID process this message.

	case WM_MOUSEMOVE:

		UpdateMouseProximityHighlighting(&CashierHighlightList, hDlg, MAKEPOINTS(lParam));

		return TRUE;	// TRUE = we did process this message.

	case WM_PAINT:

		// If we've got a background image, paint it first.

		{

			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hDlg, &ps);

			if (hdc && !IsIconic(hDlg) && hCashierBgnd) {

				if (hCashierPalette) {

					SelectPalette(hdc, hCashierPalette, FALSE);

					RealizePalette(hdc);

				}

				BlitBitmapToDC_nt(hCashierBgnd, hdc);

			}

			EndPaint(hDlg, &ps);

		}

		break;

	case WM_QUERYNEWPALETTE:

	case WM_PALETTECHANGED:	// palette has changed.

		//kp(("%s(%d) Table got WM_PALETTECHANGED\n", _FL));

	    if ((HWND)wParam!=hDlg || message!=WM_PALETTECHANGED) {	// only do something if the message wasn't from us

		    HDC hdc = GetDC(hDlg);

		    HPALETTE holdpal = SelectPalette(hdc, hCashierPalette, FALSE);

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



	case WMP_MISC_CLIENT_MSG:

		ShowMiscClientMsg(hDlg, &Ecash_misc_client_msg, "Message from e-Media Poker Cashier:");

		return TRUE;	// TRUE = we did process this message.



	case WMP_UPDATE_PLAYER_INFO:

		// Set the text for IDC_CASHIER_PLAYERINFO to indicate the user id,

		// mailing address, etc.

		if (LoggedInPrivLevel < ACCPRIV_REAL_MONEY) {

			sprintf(str, "Player ID: %s\n\nEmail: %s\n\n"

					"Your account is not yet set up to\n"

					"play with real money.",

					LoginUserID,

					LoggedInAccountRecord.sdb.email_address

			);

		} else {

			int hands_to_play;

			if (PendingCredit>0) {

				hands_to_play = 100-GoodRakedGames;

			} else {

				hands_to_play = 0;

			}

			sprintf(str, "Player ID: %s\n\nEmail: %s\n\nMailing Address:\n\n"

					"   %s\n"

					"   %s\n"

					"%s%s%s"

					//"   %s, %s\n   %s %s\n\n* Hands to Play: %d",
					"   %s, %s\n   %s %s\n\n",
					LoginUserID,

					LoggedInAccountRecord.sdb.email_address,

					LoggedInAccountRecord.sdb.full_name,

					LoggedInAccountRecord.sdb.mailing_address1,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "   " : "",

					LoggedInAccountRecord.sdb.mailing_address2,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "\n" : "",

					LoggedInAccountRecord.sdb.city,

					LoggedInAccountRecord.sdb.mailing_address_state,

					LoggedInAccountRecord.sdb.mailing_address_country,

					LoggedInAccountRecord.sdb.mailing_address_postal_code//,

					//hands_to_play
					

			);

		}

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_CASHIER_PLAYERINFO), str);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_STAR), "*");

		// Show/hide the various buttons depending on their account status

		if (LoggedInPrivLevel < ACCPRIV_REAL_MONEY) {

			// Account not ready... display a 'set up for real money' button.

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_SETUP_ACCOUNT),		SW_SHOWNA);

			ShowWindow(GetDlgItem(hDlg, IDC_PURCHASE_CHIPS),	 SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_CASH_CHIPS), 		 SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_TRANSACTION_HISTORY),SW_HIDE);

		} else {

			// Account is normal and real money ready... display all regular buttons

			ShowWindow(GetDlgItem(hDlg, IDC_SETUP_ACCOUNT),		SW_HIDE);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_PURCHASE_CHIPS),		SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_CASH_CHIPS), 		SW_SHOWNA);

			ShowWindowIfNecessary(GetDlgItem(hDlg, IDC_TRANSACTION_HISTORY),SW_SHOWNA);

		}

		return TRUE;	// TRUE = we DID process this message.



	case WMP_UPDATE_REALBANK_CHIPS:

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_BANK,

				CurrencyString(str, RealChipsInBank, CT_REAL, TRUE));

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_TABLE,

				CurrencyString(str, RealChipsInPlay, CT_REAL, TRUE));

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_PENDING,

				CurrencyString(str, PendingCredit, CT_REAL, TRUE));

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_TOTAL,

				CurrencyString(str, RealChipsInBank+RealChipsInPlay+PendingCredit, CT_REAL, TRUE));



		return TRUE;	// TRUE = we DID process this message.

	}

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

//
//

// Open and/or show the cashier screen.  Make sure the client is

// logged in first.  This function must be called from the message thread.

//

/*void OpenCashierScreen(void)

{

	if (hCashierDlg) {		// already open, just top it.

		ReallySetForegroundWindow(hCashierDlg);

		return;

	}



	// verify we're logged in before we open it.

	if (LoggedIn != LOGIN_VALID) {

		LogInToServer("You must be logged in before visiting the cashier");

	}

	if (LoggedIn != LOGIN_VALID) {

		return;	// not logged in... can't go to cashier.

	}

	if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
		HWND hDlg;
		DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
	}else{
	// Load the background pic

		if (!hCashierBgnd) {

			// Load the palette for the bgnd pic if necessary

			hCashierPalette = LoadPaletteIfNecessary(FindFile("cashier.act"));

	    

			// Change the JPEG file location

			//  hCashierBgnd = LoadJpegAsBitmap(FindFile("cashier.jpg"), hCashierPalette);

			hCashierBgnd = LoadJpegAsBitmap(FindMediaFile("media\\src_cashier.jpg"), hCashierPalette);

		}


		LoadCashierBitmaps(hCashierPalette);


		if (!hCashierButtonFont) {

			hCashierButtonFont = CreateFont(

				16,	//  int nHeight,             // logical height of font

				0,	//  int nWidth,              // logical average character width

				0,	//  int nEscapement,         // angle of escapement

				0,	//  int nOrientation,        // base-line orientation angle

				FW_NORMAL,//  int fnWeight,            // font weight

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

		if (!hCashierButtonFontSmall) {

			hCashierButtonFontSmall = CreateFont(

				14,	//  int nHeight,             // logical height of font

				0,	//  int nWidth,              // logical average character width

				0,	//  int nEscapement,         // angle of escapement

				0,	//  int nOrientation,        // base-line orientation angle

				FW_NORMAL,//  int fnWeight,            // font weight

				0,	//  DWORD fdwItalic,         // italic attribute flag

				0,	//  DWORD fdwUnderline,      // underline attribute flag

				0,	//  DWORD fdwStrikeOut,      // strikeout attribute flag

				ANSI_CHARSET,		//  DWORD fdwCharSet,        // character set identifier

				OUT_DEFAULT_PRECIS,	//  DWORD fdwOutputPrecision,  // output precision

				CLIP_DEFAULT_PRECIS,//  DWORD fdwClipPrecision,  // clipping precision

				DEFAULT_QUALITY,	//  DWORD fdwQuality,        // output quality

			  #if 1	//19991111MB

				DEFAULT_PITCH|FF_ROMAN,	//  DWORD fdwPitchAndFamily,  // pitch and family

			  #else

				DEFAULT_PITCH|FF_SWISS,	//  DWORD fdwPitchAndFamily,  // pitch and family

			  #endif

				NULL	//  LPCTSTR lpszFace         // pointer to typeface name string

			);

		}



		// Display the dialog box

		if (!hCashierDlg) {

			#if 0		// Should we have a parent?

				CreateDialog(hInst, MAKEINTRESOURCE(IDD_CASHIER),

							hCardRoomDlg, dlgFuncCashier);

			#else

				CreateDialog(hInst, MAKEINTRESOURCE(IDD_CASHIER),

						  	NULL, dlgFuncCashier);

			#endif

			if (hCashierDlg) {

			// make sure it's on screen in the current screen resolution

				WinPosWindowOnScreen(hCashierDlg);

				// Create a tooltip control/window for use within this dialog box
				// and add all our tool tips to it.

				hCashierToolTips = OpenToolTipWindow(hCashierDlg, CashierDlgToolTipText);

				ShowWindow(hCashierDlg, SW_SHOW);

			} else {

				Error(ERR_ERROR, "%s(%d) Cashier dialog failed to open.  GetLastError() = %d\n", _FL, GetLastError());
	
			}

		}
	}

}*/


void OpenCashierScreen(void){

	if (hCashierDlg) {		// already open, just top it.

		ReallySetForegroundWindow(hCashierDlg);

		return;

	}

	// verify we're logged in before we open it.
	if (LoggedIn != LOGIN_VALID) {

		LogInToServer("You must be logged in before visiting the cashier");

	}

	if (LoggedIn != LOGIN_VALID) {
		return;	// not logged in... can't go to cashier.
	}


	if (LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) {		
		HWND hDlg;
		DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANGE_EMAIL), hDlg, dlgFuncChangeEmail);
	}else{
		if (LoggedInPrivLevel < ACCPRIV_REAL_MONEY) 
			MakeAccountRealMoneyReady();
		else
			LaunchInternetBrowser("https://www.e-Mediasoftware.com/cashier");
			
	}

}



/**********************************************************************************

 Function CALLBACK dlgDisableCashierFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: 2001/09/20

 Purpose: handler for Disable Cashier

***********************************************************************************/

BOOL CALLBACK dlgDisableCashierFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[120];

	zstruct(str);

	switch (message) {

	case WM_INITDIALOG:

		/*

		// window title

		AddKeyboardTranslateHwnd(hDlg);

		SetWindowTextIfNecessary(hDlg,"Real money functions are currently disabled...");

		

		// build info

		sprintf(str,"Compiled: %s - %s", __DATE__,__TIME__);

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_DATETIME, str);

		// client version

		sprintf(str,"Client version %d.%02d (build %d)",

			ClientVersionNumber.major,ClientVersionNumber.minor,

			ClientVersionNumber.build & 0x0000FFFF);

		

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT1, "Real money functions are currently disabled...");

				// server version (received from server)

		if (ServerVersionInfo.server_version.build) {

			sprintf(str,"Server version %d.%02d (build %d)\n",

				ServerVersionInfo.server_version.major,

				ServerVersionInfo.server_version.minor,

				ServerVersionInfo.server_version.build & 0x0000FFFF);

		} else {

			strcpy(str, "Server version unknown.");

		}

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT2, str);



		// Build up some a string containing our connection data.

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_ABOUT3, szConnectionTypeString);

		*/

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









/*BOOL CALLBACK dlgFuncRealMoneyAccountSetup1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		if (backbutton ==1){

			CheckDlgButton(hDlg, IDC_CHECK1, TRUE);	// always default to checked.

			CheckDlgButton(hDlg, IDC_CHECK10, TRUE);	// always default to checked.

			CheckDlgButton(hDlg, IDC_CHECK2, TRUE);	// always default to checked.

		}



		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_COMMAND:

		{

			switch (LOWORD(wParam)) {

			case IDC_BUTTON1:

					MessageBox(hDlg,

					"More Information - Age Limit\n"

					"\n"

					"Players of e-Media Poker must be at least 21 years of age to \n"

					"use any of the e-Media Poker software or services.  Anyone proven to  \n"

					"be under this age limit will be removed from this site immediately.\n"

					,

					"e-Media Poker", MB_OK|MB_TOPMOST);



				return TRUE;	// We DID process this message

			case IDC_BUTTON2:



				DialogBox(hInst, MAKEINTRESOURCE(IDD_READ_CONTRACT), hDlg, dlgFuncReadContract);

				return TRUE;	// We DID process this message





			case IDCANCEL:

				

				EndDialog(hDlg, LOWORD(wParam));

				return TRUE;	// We DID process this message

			case IDC_VALIDATE:

				if (!IsDlgButtonChecked(hDlg, IDC_CHECK1)||

					!IsDlgButtonChecked(hDlg, IDC_CHECK2)||

					!IsDlgButtonChecked(hDlg, IDC_CHECK10))

				{

                   MessageBox(hDlg,

					"You must meet the three requirements to continue.\n"

					"\n"

					,

					"e-Media Poker", MB_OK|MB_TOPMOST);



                } else{ 



						EndDialog(hDlg, LOWORD(wParam));

						DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREATE2), hCashierDlg, dlgFuncMakeRealMoneyReady);
						
						return TRUE;	// We DID process this message

				}

			}

		}

		break;

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

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





BOOL CALLBACK dlgFuncReadContract(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			AddKeyboardTranslateHwnd(hDlg);

			char *file = (char *)LoadFile("data/RealMoneyPlayAgreement.txt", NULL);

			if (!file) {

				EndDialog(hDlg, IDCANCEL);	// could not load the agreement

			}

			SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), file);

			free(file);

			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_SETSEL, (WPARAM)-1, 0);

		}

		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:

		case IDCANCEL:

			EndDialog(hDlg, LOWORD(wParam));

		}

		return TRUE;	// We DID process this message.

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}
*/


BOOL CALLBACK dlgFuncWesternUnion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			/*

			AddKeyboardTranslateHwnd(hDlg);

			char *file = (char *)LoadFile("data/RealMoneyPlayAgreement.txt", NULL);

			if (!file) {

				EndDialog(hDlg, IDCANCEL);	// could not load the agreement

			}

			SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), file);

			free(file);

			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_SETSEL, (WPARAM)-1, 0);

			*/

		}

		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDC_BUTTON1:

			LaunchInternetBrowser(BASE_URL"real_money.php");

		case IDOK:

		case IDCANCEL:

			EndDialog(hDlg, LOWORD(wParam));

		}

		return TRUE;	// We DID process this message.

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}





BOOL CALLBACK dlgFuncBankDrafts(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			/*

			AddKeyboardTranslateHwnd(hDlg);

			char *file = (char *)LoadFile("data/RealMoneyPlayAgreement.txt", NULL);

			if (!file) {

				EndDialog(hDlg, IDCANCEL);	// could not load the agreement

			}

			SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), file);

			free(file);

			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_SETSEL, (WPARAM)-1, 0);

			*/

		}

		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:

		case IDCANCEL:

			EndDialog(hDlg, LOWORD(wParam));

		}

		return TRUE;	// We DID process this message.

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}







BOOL CALLBACK dlgFuncPayPal(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			/*

			AddKeyboardTranslateHwnd(hDlg);

			char *file = (char *)LoadFile("data/RealMoneyPlayAgreement.txt", NULL);

			if (!file) {

				EndDialog(hDlg, IDCANCEL);	// could not load the agreement

			}

			SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), file);

			free(file);

			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_SETSEL, (WPARAM)-1, 0);

			*/

		}

		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:



			EndDialog(hDlg, LOWORD(wParam));

			return TRUE;



		case IDC_BUTTON1:

			LaunchInternetBrowser(BASE_URL"real_money.php");

			return TRUE;



		case IDSUBMIT:

			

			CCTransaction cct;

			zstruct(cct);

			HWND hSubmitProgressDlg;

			hSubmitProgressDlg = NULL;

		

			char card_type[25];

			zstruct(card_type);

			char str[500];

			zstruct(str);

			char curr_str[MAX_CURRENCY_STRING_LEN];

			zstruct(curr_str);

			int num;

		    AccountRecord ar;



            /*

			

			if (amount>600){

					MessageBox(hDlg,

							"$600 U.S. Dollars is the maximum amount\nthat you can deposit at a time. \nPlease reenter an amount.",

							"e-Media Poker",

							MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

				}



			if (amount<50){

					MessageBox(hDlg,

							"$50 U.S. Dollars is the maximum amount\nthat you can deposit at a time. \nPlease reenter an amount.",

							"e-Media Poker",

							MB_OK|MB_ICONWARNING|MB_APPLMODAL|MB_TOPMOST);

				}

				

			if ((amount>=5)&&(amount<=600)) {

				  // LaunchInternetBrowser("C:\\Documents and Settings\\Administrator\\Local Settings\\Temp\\paypal.html");

				}

            

            */

                   // read and post if verified

			cct.card_type = CCTYPE_UNKNOWN;

			strcpy(card_type, "UNKNOWN CARD TYPE");

			

			cct.transaction_type = CCTRANSACTION_PURCHASE;

		  

		  	GetDlgText(hDlg, IDC_EDIT1, cct.amount, CCFIELD_SHORT);

			//strcpy(samount,cct.amount);

			sprintf(samount,"%s", cct.amount);

			// !!!! some local validation for valid card numbers, etc should go here

			// do some fiddling with what was entered to make it look better

			num = StringToChips(cct.amount);	// make +ve and rounded (it's in pennies for ecash)

			if (!num) {	// it's 0

				return TRUE;	// We DID process this message.

			}

			sprintf(cct.amount,"%d",num);

			

			ar = LoggedInAccountRecord;	// fill in defaults

			sprintf(str, 

				"%s\n"

				"%s\n"

				"%s%s%s"

				"%s, %s, %s\n"

				"%s\n\n"

				"Purchase amount: %s",

				ar.sdb.full_name,

				ar.sdb.mailing_address1, "",

				ar.sdb.mailing_address2,

				ar.sdb.mailing_address2[0] ? "\n" : "",

				ar.sdb.city,

				ar.sdb.mailing_address_state,

				ar.sdb.mailing_address_country,

				ar.sdb.mailing_address_postal_code,

				CurrencyString(curr_str, atoi(cct.amount), CT_REAL, TRUE)

			);



			// rc = MessageBox(hDlg, str, "Verify Credit Card Purchase...", 

			//	MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

			// if (rc == IDOK) {	// post it

				SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

				// fluff: progress bar

				hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Chip Purchase Request to server...",

					60,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %

	

				if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 200);

				}



				

				



				EndDialog(hDlg, IDOK);

			//} else { 

				// hit CANCEL, may want to correct something -- don't abort

		    //}

			

			

		    //EndDialog(hDlg, LOWORD(wParam));

		}

		return TRUE;	// We DID process this message.

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	case WMP_LIMITATION_APPROVED:

		MessageBox(NULL, " ", "Verify Credit Card Purchase...", 

		MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);  

      	break;



	case WMP_LIMITATION_LARGE:

		MessageBox(NULL, " ", "Verify Credit Card Purchase...", 

		MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);  

		break;



	case WMP_LIMITATION_SMALL:

		MessageBox(NULL, " ", "Verify Credit Card Purchase...", 

		MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);  

		break;



	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}











BOOL CALLBACK dlgFuncAccountCredit_1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	static int credit_guess = 0, i;

	static int paypal_direct_amount;

	static int remaining_amount;

    static char pay_method[15];

	char str[CCFIELD_LONG];

	char str1[CCFIELD_LONG];

    

	

    char strmessage[150];

	

	char strmessage_1[150];

	char strmessage_check[150];

    char addressmessage[150];



	switch (message) {

	case WM_INITDIALOG:

	credit_guess = 0;
     //ccv
	//sprintf(pay_method,"paypal");
    sprintf(pay_method,"cACA");
	//ccv


	

  	

 	for (i= TRANS_TO_RECORD_PER_PLAYER-1; i>=0; i--) {

		   if (LoggedInAccountRecord.sdb.transaction[i].credit_left >0 ) {

				credit_guess += LoggedInAccountRecord.sdb.transaction[i].credit_left;

			}

		}



	

     if (credit_guess <=0) {

			credit_guess =0;

		}



	/*	

	for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

			if (LoggedInAccountRecord.sdb.transaction[i].transaction_type == CTT_PURCHASE) {

				// 20010202: make sure it's not disabled before we add it in (credit amt > purchase means disabled)

				if (LoggedInAccountRecord.sdb.transaction[i].credit_left <= LoggedInAccountRecord.sdb.transaction[i].transaction_amount) {

					credit_guess += LoggedInAccountRecord.sdb.transaction[i].credit_left;

				}

			}

		}

		*/

		if ( cashout_amount < credit_guess)

		{

             paypal_direct_amount=cashout_amount;

		}

		else

		{

			 paypal_direct_amount=credit_guess;

		}

        

		remaining_amount=cashout_amount-paypal_direct_amount;



        sprintf(strmessage_1,"%s will be credited to your Credit Card(s) .\n\nPlease choose one of the cash-out options below to receive the remaining %s you have requested.\nPaypal is the preferred method for money transfers at e-Media Poker."

			,CurrencyString(str, paypal_direct_amount, CT_REAL, TRUE) , CurrencyString(str1, remaining_amount, CT_REAL, TRUE));

		

		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_FirstMessage, 

			strmessage_1);

		

		/*

		SetDlgItemTextIfNecessary(hDlg, IDC_CREDIT_GUESS, 

			CurrencyString(str, credit_guess, CT_REAL, TRUE));

		SetDlgItemTextIfNecessary(hDlg, IDC_REAL_BANK, 

			CurrencyString(str, RealChipsInBank, CT_REAL, TRUE));

		*/

		CheckDlgButton(hDlg, IDC_RADIO1, TRUE);

        

             sprintf(strmessage,"%s will be transfered to the following"

			" Paypal account:\n\nYou will receive "

			"the funds in the next 1-3 days.", CurrencyString(str, remaining_amount,CT_REAL, TRUE));

		

   		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_1, 

			strmessage);





		sprintf(addressmessage,"\nEmail       :%s\nPlayerID  :%s\n", LoggedInAccountRecord.sdb.email_address,LoggedInAccountRecord.sdb.user_id);

		

   		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_2, 

			addressmessage);





   

		return TRUE;



	case WM_DESTROY:

		hCCCreditDLG = NULL;

		break;

	

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		

		case IDC_RADIO2:



		 sprintf(strmessage_check, "A check for %s will be sent to the following address:"

			        "\n\nPlease allow 2-3 "

					"weeks for delivery.",

					CurrencyString(str, remaining_amount, CT_REAL, TRUE)					

			);

         

   		 SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_1, 

			strmessage_check);

         sprintf(pay_method,"check");

		 

         /*

		 sprintf(addressmessage, 

					"%s\n%s\n%s\n"

					"%s,%s\n%s%s\n",

					LoggedInAccountRecord.sdb.full_name,

					LoggedInAccountRecord.sdb.mailing_address1,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "   " : "",

					LoggedInAccountRecord.sdb.mailing_address2,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "\n" : "",

					LoggedInAccountRecord.sdb.city,

					LoggedInAccountRecord.sdb.mailing_address_state,

					LoggedInAccountRecord.sdb.mailing_address_country,

					LoggedInAccountRecord.sdb.mailing_address_postal_code

					

			);

         

           */

         	sprintf(addressmessage, "%s\n"

					

					"%s\n"

					"%s%s%s"

					"%s, %s\n%s, %s\n",

					

					LoggedInAccountRecord.sdb.full_name,

					LoggedInAccountRecord.sdb.mailing_address1,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "" : "",

					LoggedInAccountRecord.sdb.mailing_address2,

					LoggedInAccountRecord.sdb.mailing_address2[0] ? "\n" : "",

					LoggedInAccountRecord.sdb.city,

					LoggedInAccountRecord.sdb.mailing_address_state,

					LoggedInAccountRecord.sdb.mailing_address_country,

					LoggedInAccountRecord.sdb.mailing_address_postal_code

					

			);





   		 SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_2, 

			addressmessage);





        return TRUE;

	 	case IDC_RADIO1:

        sprintf(strmessage,"%s will be transfered to the following"

			"Paypal account:\n\nYou will receive "

			"the funds in the next 1-3 days.", CurrencyString(str, remaining_amount,CT_REAL, TRUE));

		

   		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_1, 

			strmessage);





		sprintf(addressmessage,"\nEmail       :%s\nPlayerID  :%s\n", LoggedInAccountRecord.sdb.email_address,LoggedInAccountRecord.sdb.user_id);

   		SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_2, 

			addressmessage);



		sprintf(pay_method,"paypal");



		return TRUE;

		case IDOK:

			// read and post if verified

			HWND hSubmitProgressDlg;

			hSubmitProgressDlg = NULL;

			CCTransaction cct;

			zstruct(cct);

			if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

				//GetDlgItemText(hDlg, IDC_EDIT_CREDITADMIN, str, CCFIELD_LONG);

				//cct.admin_transaction_number_override = atoi(str);

			}

			//Allen 2002-03-18

			cct.transaction_type = CCTRANSACTION_FIREPAY_CASHOUT; // CCTRANSACTION_CASHOUT; 

			if (cct.admin_transaction_number_override) {

				cct.transaction_type = CCTRANSACTION_ADMIN_CREDIT;

			}

			GetDlgItemText(hDlg, IDC_AMOUNT, cct.amount, CCFIELD_SHORT);

			// get the credit amount

			long amount;

			amount = StringToChips(cct.amount);	// make +ve and rounded (it's in pennies for ecash)

			// bound its range if it's non admin

			if (cct.transaction_type != CCTRANSACTION_ADMIN_CREDIT) {

				amount = min(amount, (int)RealChipsInBank);

				if (amount < 100) {	// less than $1, dismiss

					// return TRUE;	// We DID process this message.

				}

			}

			

			char str[100];

			zstruct(str);

			char curr_str[MAX_CURRENCY_STRING_LEN];

			if (cct.transaction_type == CCTRANSACTION_ADMIN_CREDIT) {

				sprintf(str,"Request credit against transaction %d for %s", 

					cct.admin_transaction_number_override, CurrencyString(curr_str, amount, CT_REAL, TRUE) );

			} else {

				sprintf(str,"Request credit for %s", CurrencyString(curr_str, amount, CT_REAL, TRUE) );

			}

			int rc;

			rc = IDOK;

			/*

				MessageBox(hDlg, str, "Verify Cash Out Request...", 

				MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

			*/

			if (rc == IDOK) {	// post it



				//paypal_direct_amount=5100;

				sprintf(cct.amount, "%d", paypal_direct_amount);

				//sprintf(cct.unused,"paypal");

                sprintf(cct.unused,"paypal");

				// fluff: progress bar

				

				/*

				hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Cash Out request to server...",

					60,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %



				if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 200);

				}

				*/

				

				if (paypal_direct_amount >=1) {

				   SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

				}

                //sleep(100) ;

				//sprintf(cct.unused,"check");

			



				// fluff: progress bar

				hSubmitProgressDlg = CreateProgressWindow(hDlg,

					"Submitting Cash Out request to server...",

					100,	// start at 0%, go up to 60% while waiting

					2400);	// # of ms for indicator to get to that %



				if (hSubmitProgressDlg) {

					FinishProgressWindow(hSubmitProgressDlg, 3000);

				}

				//Sleep(2000);

				//remaining_amount=5001;

				//Added By allen 2002-03-18

				cct.transaction_type = CCTRANSACTION_CASHOUT; 

				sprintf(cct.unused,pay_method);

				sprintf(cct.amount, "%d", remaining_amount /*cashout_amount*/);

				if (remaining_amount >= 1) {

				   SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

				}

                





				

				// 20010226HK: ask for phone number if warranted

				//char guess_str[CCFIELD_LONG+1];

				//zstruct(guess_str);

				//GetDlgItemText(hDlg, IDC_CREDIT_GUESS, guess_str, CCFIELD_LONG);



				

				//int guess = atoi(guess_str+1);	// skip past the '$'

				//if (guess*100 < amount) {	// looks like a check will be issued (amount is in $, guess in pennies)

					// ask for the phone number if there isn't one entered

				//	char _phone_number[PHONE_NUM_EXPANDED_LEN+1];

			//		zstruct(_phone_number);

			//		DecodePhoneNumber(_phone_number, LoggedInAccountRecord.sdb.phone_number);

					

					/*

					if (strlen(_phone_number) < 5) {

						if (!hEnterPhoneNumberDLG) {

							CreateDialog(hInst, MAKEINTRESOURCE(IDD_SUBMIT_PHONE_NUMBER), NULL, dlgFuncEnterPhoneNumber);

						}

						// make sure it always comes to the foreground

						if (hEnterPhoneNumberDLG) {

							ShowWindow(hEnterPhoneNumberDLG, SW_SHOW);

							SetForegroundWindow(hEnterPhoneNumberDLG);

						}

					}

					*/



			//	}

				EndDialog(hDlg, IDOK);	// get rid of the original dlg

			} else { 

				// hit CANCEL, may want to correct something -- don't abort

			}

			return TRUE;	// We DID process this message.

		case IDCANCEL:

			rc = MessageBox(hDlg, "Are you sure you want to cancel this transaction?",

				"Cancel Transaction...", 

				MB_YESNO|MB_APPLMODAL|MB_TOPMOST);

			if (rc == IDYES) {

				EndDialog(hDlg, IDCANCEL);

			}

			return TRUE;	// We DID process this message

		}

		break;

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}





