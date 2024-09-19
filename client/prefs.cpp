//****************************************************************


//

// Prefs.cpp : user preferences and account settings

//

//****************************************************************



#define DISP 0

#include "afxdb.h"
#include "stdafx.h"


#include "stdafx.h"

#include "resource.h"


#if ADMIN_CLIENT

  #include <sys/types.h>	// used by file 'stat' function in email search dlg

  #include <sys/stat.h>		// used by file 'stat' function in email search dlg

#endif	// ADMIN_CLIENT



#if ADMIN_CLIENT



//--- prediction stuff ---

void Pred_LogNewStats(time_t src_data_time,

		int players,

		int rake_per_hour,

		int rake_today,

		int games_today,

		int gross_cc_today,

		int new_accounts_today,

		int *output_seconds_into_interval,

		int *output_current_interval);

void Pred_GetPredictionInfo(struct Pred_Data *normal_output,

		struct Pred_Data *peak_output,

		int *output_peak_players_interval,

		float *output_min_players,

		int *output_min_players_interval,

		int seconds_into_interval);

//--- end prediction stuff ---

#endif	// ADMIN_CLIENT



extern BOOL CALLBACK dlgFuncViewTransactions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern void ConvertSecondsToString(int seconds_to_go, char *dest_str, int display_seconds_flag, int display_short_units_flag, int display_field_count);

BOOL CALLBACK dlgFuncReadContract(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



struct AccountRecord LoggedInAccountRecord;	// AccountRecord sent from server (if any)

struct MiscClientMessage MiscMsgCreateAccountWarning;// msg[] contains warning before setting up for real money

struct AccountRecord ArToView;	// AccountRecord about to be viewed



static HWND hCreateAccountDlg;	// handle to the create new account dialog (if open)

static HWND hSubmitProgressDlg;	// handle to the 'submitting request to server' dialog (if open)

static HWND hMakeRealMoneyDlg;	// handle to the 'make account real money ready' dialog (if open)



char TmpEmail[100];

int  retval;

//char*  missingfield[5];
char*  missingfield[6];

#define TAM 250
char vname[TAM];

char vaddress[TAM];

char vcity[TAM];

char vprovince[TAM];

char vcountry[TAM];





HWND hUpdateAddressDlg;			// handle to the 'update address info' dialog (if open)

static struct DlgToolTipText EditAccountToolTipText[] = {

	IDC_ATLEAST21,		"You must be at least 21 years old to play. You must be 21 to click this box.",

	IDC_FULLNAME,		"Enter your full name (e.g. 'John Smith').  This is for our records and for mailing checks out only.",

	IDC_PASSWORD,		"It is important to remember your password and keep it in a safe place.",

	IDC_PASSWORD2,		"It is important to remember your password and keep it in a safe place.",

	IDC_USERID,			"This is the name that will be displayed in the games.",

	IDC_CITY,			"This is the city that will be displayed in the games.",

	IDC_EMAIL,			"Enter your email address (e.g. joe@Motorola.com).",

	IDC_EMAIL2,			"Enter your email address (e.g. joe@Motorola.com).",

	0,0

};



static struct DlgToolTipText MakeRealMoneyReadyToolTipText[] = {

	IDC_USERID,			"Please enter you User ID",

	IDC_EMAIL,			"Change/enter your correct email address (eg:fred@hotmail.com)",

	IDC_PASSWORD,		"Enter a password to of your Real Money Account",

	IDC_PASSWORD2,		"Please reenter the password",

	IDC_FULLNAME,		"Enter your first real name and not your playerID (eg: Fred)",

	IDC_LASTNAME,		"Enter your last real name and not your playerID (eg: Smith)",

	IDC_ADDRESS_LINE1,	"Enter your street address(eg:Suite 2b-123 Main Street)",

	IDC_ADDRESS_LINE2,	"Optional-enter a secondary street address/box number(eg:Box 21)",

	IDC_CITY,			"Enter the city or town you live in (eg: Jackson City)",

	IDC_STATE,			"Please enter the state or province you live in (initials are OK)",

	IDC_COUNTRY,		"Select your Country from the pull down list",

	IDC_POSTAL_CODE,	"Optional - enter the zip or postal code for your address(eg: 12345 or J5G 3H1)",

	IDC_MALE,			"Please choose male or female",

	IDC_FEMALE,			"Please choose male or female",

	0,0

};



static char SubmittedUserID[MAX_PLAYER_USERID_LEN];	// user id we submitted
static int iDisplayedBadEmailNotice = FALSE;



#if ADMIN_CLIENT

HWND hAdminStats = NULL;			// admin stats window (if open)

HWND hAdminEditAccountDlg = NULL;

HWND hTransferMoneyDlg = NULL;

HWND hChatMonitor = NULL;

HWND hCheckTrackingDLG = NULL;

HWND hEmailSearchDLG = NULL;

HWND hInfoBlock = NULL;			// handle to admin info block window

HWND hReqPlayerInfoDLG = NULL;	// handle to the player info request dlg

HWND hReqXferEmailDlg = NULL;	// handle to req xfer email dlg



WORD32 AutoLookupPlayerID;	// used for autolookups

struct AccountRecord LatestAccountRecordFromServer;

static struct AccountRecord LatestXferSrcRecordFromServer;

static struct AccountRecord LatestXferDestRecordFromServer;

static int iTransferMoneyRealMoneyFlag;	// set if transfer dialog should be for real money

static WORD32 dwTransferMoneySourcePlayerID;

static WORD32 dwTransferMoneyDestPlayerID;

static WORD32 dwComputerSerialNumToBlock;

struct AdminStats AdminStats;

struct AdminInfoBlock AdminInfo;



#define MAX_REMEMBERED_ADMIN_PLAYERIDS	30	// size of back/forth queue of user ids (admin only)

static WORD32 AdminPlayerIDs[MAX_REMEMBERED_ADMIN_PLAYERIDS];

static int AdminPlayerIDIndex;

#define MAX_PLAYERIDS_SPIN 100	// range is 0-100 for values that spinner returns

int old_playerid_spinner = -1;









static int CheckRunPageNumber = 0;	// used for what page we're on in a check run



BOOL CALLBACK dlgReqPlayerInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgReqXferEmail(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



void Convert_time_t_to_SYSTEMTIME(time_t t, SYSTEMTIME *sys_time);

void Convert_SYSTEMTIME_to_time_t(SYSTEMTIME *sys_time, time_t *t);



#endif	// admin client





#if INCL_EMAIL_INVALIDATING	//20000210MB: include email invalidating code?

int iInvalidateEmailAddresses;

char *szUserIDsToInvalidate[] = {

//		"yossarian",

//		"yourtoad",

//		"zeeker",

//		"zemos",

		NULL

};

char **szUserIDToInvalidate = szUserIDsToInvalidate;

#endif	// INCL_EMAIL_INVALIDATING



int backbutton=0;



//rgong 04/09/2002 

extern char VendorCode[6];

//end rgong



#if 0	//19991120MB: not yet tested.

//*********************************************************

// 1999/11/20 - MB

//

// Trim an arbitrary list of characters from a string

//

void TrimCharsFromString(char *str, char *trim_chars)

{

	while (*trim_chars) {

		char *s = str;

		while (*s) {

			if (*s==*trim_chars) {	// found one of the ones we're supposed to remove.

				memmove(s, s+1, strlen(s+1)+1);

			} else {

				s++;

			}

		}

		trim_chars++;

	}

}

#endif



//*********************************************************

// 1999/08/18 - MB

//

// Validate an individual string to make sure it has something in it.

// return: TRUE for valid, FALSE for invalid.

//

static int ValidateString(char *str, int min_str_len, int max_str_len, char *string_name)

{

	if (!strlen(str)){
	
		return FALSE;
	
	}
	
	// trim leading spaces and control characters...

	str[max_str_len-1] = 0;

	while (*str > 0 && *str <= ' ') {

		memmove(str, str+1, max_str_len-1);	// shift left.

	}

	// trim trailing spaces...

	while (strlen(str) && str[strlen(str)-1] == ' ') {

		str[strlen(str)-1] = 0;

	}

	if (strlen(str) < (size_t)min_str_len) {

		char msg[200];

		sprintf(msg, "Your %s must be at least %d characters long.", string_name, min_str_len);

		return FALSE;

	}

	return TRUE;

}



//*********************************************************

// 1999/11/10 - MB

//

// Validate an individual email address to make sure it looks OK.

// return: TRUE for valid, FALSE for invalid.

//

int ValidateEmail(HWND hDlg, char *email_str)

{

	if (!ValidateString(email_str, 7, MAX_EMAIL_ADDRESS_LEN, "Email address")) {

		return FALSE;

	}

	// Perform some more checks on the email address to make sure it's close to valid.

	// It must have exactly one '@'

	char *p1 = strchr(email_str, '@');

	char *p2 = strrchr(email_str, '@');

	if (!p1 || p1 != p2) {

		MessageBox(hDlg, "Email addresses must have exactly one '@' character.", "Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);

		return FALSE;

	}



	// No commas, no spaces, semicolon, colon, brackets, etc.

	if (strpbrk(email_str, " ,;:[]()*<>%!")) {

		MessageBox(hDlg,

				"Email addresses cannot contain commas, spaces,\n"

				"and several special characters.",

				"Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);

		return FALSE;

	}



	// No leading '-' allowed

	if (email_str[0]=='-') {

		MessageBox(hDlg,

				"Email addresses cannot start with a '-' character.",

				"Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);

		return FALSE;

	}



	// There must be something before the '@'

	if (p1==email_str) {

		MessageBox(hDlg, "Email addresses must have a name before the '@' character.", "Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);

		return FALSE;

	}

	int domain_ok = TRUE;	// default to TRUE.

	// There must be at least one '.' after the @

	p2 = strrchr(p1, '.');	// locate final '.'

	if (!p2) {

		domain_ok = FALSE;

	} else {

		// There must be at least two alphas after the final .

		if (strlen(p2) < 3) {	// 3 = '.' plus two more characters.

			domain_ok = FALSE;

		}

	}

	// There must be at least one alpha after the @ (before the final '.')

	p2 = p1+1;

	while (*p2 && *p2!='.' && !isalpha(*p2)) {

		p2++;

	}

	if (!isalpha(*p2)) {

		domain_ok = FALSE;

	}



	if (!domain_ok) {

		MessageBox(hDlg, "Email addresses must have a proper domain\nname after the '@' character.", "Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);

		return FALSE;

	}

	return TRUE;	// valid.

}



//*********************************************************

// 1999/08/18 - MB

//

// Validate whether the user typed in something legal for all

// the required fields.

// return: TRUE for valid, FALSE for invalid

//

static int ValidateNewAccountFields(HWND hDlg, struct AccountRecord *ar)

{

	

	missingfield[0]=vname;

	missingfield[1]=vaddress;

    missingfield[2]=vcity;

	missingfield[3]=vprovince;

	missingfield[4]=vcountry;
	
    for (int vi=0; vi<TAM; vi++){

		vcountry[vi]=0;

        vname[vi]=0;

		vcity[vi]=0;

		vprovince[vi]=0;

		vaddress[vi]=0;
	}

   	int flagPass =0;

	int index =0;

    int retval=TRUE;
		
	if (!ValidateString(ar->sdb.user_id,   5, MAX_PLAYER_USERID_LEN, "User ID")) {
		strcpy(missingfield[index], "Player ID: Must be 5 letters minimum");
		index++;
		retval=FALSE;
	}

	if (!ValidateString(ar->sdb.password, 5, MAX_PLAYER_PASSWORD_LEN, "Password")) {
		strcpy(missingfield[index], "Password: Must be 5 letters minimum");
		index++;
        flagPass++;
		retval=FALSE;
	}

	// Make sure the two passwords match.

	char password2[MAX_PLAYER_PASSWORD_LEN];
	password2[0] = 0;
	GetDlgItemText(hDlg, IDC_PASSWORD2, password2, MAX_PLAYER_PASSWORD_LEN);
	if (strcmp(ar->sdb.password, password2)) {
		if (flagPass){
			    strcat(missingfield[index], " and your passwords do not match.");	
			}else{
		     	strcpy(missingfield[index], "Password: Do not match. Please retype them.");
		        index++;
		};

		retval=FALSE;

	}//if

/*
	if (!ValidateString(ar->sdb.secure_phrase, 5, MAX_PLAYER_SECURE_PHRASE_LEN, "Secure Phrase")) {
//		strcpy (missingfield[index],"Secure Phrase: Must be 5 letters min.");
		index++;
		retval=FALSE;
	}
*/

	if (!ValidateString(ar->sdb.city,      2, MAX_COMMON_STRING_LEN, "Home City")) {
		//strcpy(missingfield[index], "City: Must be 2 letters minimum");
		strcpy (missingfield[index],"City: Must be 2 letters min.");
		index++;
		retval=FALSE;
	}

#if 1
//email validator
	int flagEmail=0;
	//email size
	strcpy(TmpEmail, ar->sdb.email_address);
	if (!ValidateString(TmpEmail, 7, MAX_EMAIL_ADDRESS_LEN, "Email address")) {
        strcpy(missingfield[index], "Email: Must be 7 letters minimum");
		index++;
		flagEmail++;
		retval= FALSE; 
	}else{
        //@ caracter in the email address
		flagEmail=0;
		char *p1 = strchr(TmpEmail, '@');
		char *p2 = strrchr(TmpEmail, '@');
		if (!p1 || p1 != p2) {
		    strcpy(missingfield[index], "Email: addresses must have exactly one '@' character.");
		    index++;	
			retval= FALSE;
		}//if

		//Name Before @ caracter
		if (p1==TmpEmail) {
			strcpy(missingfield[index], "Email: addresses must have a name before the '@' character.");
			index++;		   
			retval=FALSE;
		};//name before @ caracter

        if(retval){//solo para discrimar
		// No commas, no spaces, semicolon, colon, brackets, etc.
			if (strpbrk(TmpEmail, " ,;:[]()*<>%!")) {
				 strcpy(missingfield[index], "Email: addresses cannot contain commas, spaces,and several special characters ( ,;:[]()*<>%!)."); 
				index++;
				retval= FALSE;
			};//	// No commas, no spaces, semicolon, colon, brackets, etc.
		};// if (retval)

			// No leading '-' allowed
		if(retval){
			if (TmpEmail[0]=='-') {
				strcpy(missingfield[index], "Email: addresses cannot start with a '-' character."); 
				index++;
				retval= FALSE;
			}
		};//if ret val

        //check domain
	    if(retval){	
			int domain_ok = TRUE;	// default to TRUE.
			// There must be at least one '.' after the @
			p2 = strrchr(p1, '.');	// locate final '.'
			if (!p2) {
				domain_ok = FALSE;
			} else {
				// There must be at least two alphas after the final .
				if (strlen(p2) < 3) {	// 3 = '.' plus two more characters.
					domain_ok = FALSE;
				}
			}
			// There must be at least one alpha after the @ (before the final '.')
			p2 = p1+1;
			while (*p2 && *p2!='.' && !isalpha(*p2)) {
				p2++;
			}
			if (!isalpha(*p2)) {
		  	   domain_ok = FALSE;
			}

			if (!domain_ok) {
			//	MessageBox(hDlg, "Email addresses must have a proper domain\nname after the '@' character.", "Email validation problem", MB_OK|MB_ICONHAND|MB_APPLMODAL);
				strcpy(missingfield[index], "Email:addresses must have a proper domain\nname after the '@' character."); 
				index++;
				retval= FALSE;
			}//if
             //match the password
			if(retval){
				char email2[100];
				GetDlgItemText(hDlg, IDC_EMAIL2, email2, 100);
				if (strcmp(ar->sdb.email_address, email2)) {

					strcpy(missingfield[index], "Your email do not match. Please retype them."); 
					retval=FALSE;
				}
			};//if ret val
         
		};//if ret val

	};//if (!ValidateString(hDlg, TmpEmail, 7, MAX_EMAIL_ADDRESS_LEN, "Email address")) 

#endif

	return retval;

}



//*********************************************************

// 1999/08/13 - MB

//

// Read all the fields from the ACCOUNT_CREATE dialog box

//

void ReadNewAccountFields(HWND hDlg, struct AccountRecord *ar)

{

	// Fetch account info fields...

	GetDlgItemText(hDlg, IDC_FULLNAME, ar->sdb.full_name, MAX_PLAYER_FULLNAME_LEN);

	GetDlgItemText(hDlg, IDC_LASTNAME, ar->sdb.last_name, MAX_PLAYER_LASTNAME_LEN);
	
	GetDlgItemText(hDlg, IDC_PASSWORD, ar->sdb.password, MAX_PLAYER_PASSWORD_LEN);

	GetDlgItemText(hDlg, IDC_USERID, ar->sdb.user_id, MAX_PLAYER_USERID_LEN);

	GetDlgItemText(hDlg, IDC_CITY, ar->sdb.city, MAX_COMMON_STRING_LEN);

	GetDlgItemText(hDlg, IDC_EMAIL, ar->sdb.email_address, MAX_EMAIL_ADDRESS_LEN);

//	GetDlgItemText(hDlg, IDC_SECURE_PHRASE, ar->sdb.secure_phrase, MAX_PLAYER_SECURE_PHRASE_LEN);

	GetDlgItemText(hDlg, IDC_REFERED_BY, ar->sdb.refered_by, 64);

	GetDlgItemText(hDlg, IDC_COMMENTS, ar->sdb.comments, 255);

	int index = SendMessage(GetDlgItem(hDlg, IDC_HEART_ABOUT_US), CB_GETCURSEL, 0, 0);

	ar->sdb.about_us = index;


	ar->sdb.dont_use_email1 = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_CHKBOX_NOJUNKMAIL));

	ar->sdb.dont_use_email2 = !(BYTE8)(IsDlgButtonChecked(hDlg, IDC_CHKBOX_EMAIL_WHEN_REAL));

	ar->sdb.gender = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_FEMALE) ? GENDER_FEMALE : GENDER_MALE);

	// Fetch marketing info fields...

	ar->mrkinfo_banner_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_BANNER);

	GetDlgItemText(hDlg, IDC_BANNER_STR, ar->mrkinfo_banner_str, MAX_COMMON_STRING_LEN);

	ar->mrkinfo_magazine_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_MAGAZINE);

	GetDlgItemText(hDlg, IDC_MAGAZINE_STR, ar->mrkinfo_magazine_str, MAX_COMMON_STRING_LEN);

	ar->mrkinfo_newsgroup_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_NEWSGROUP);

	GetDlgItemText(hDlg, IDC_NEWSGROUP_STR, ar->mrkinfo_newsgroup_str, MAX_COMMON_STRING_LEN);

	ar->mrkinfo_website_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_WEBSITE);

	GetDlgItemText(hDlg, IDC_WEBSITE_STR, ar->mrkinfo_website_str, MAX_COMMON_STRING_LEN);

	ar->mrkinfo_other_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_OTHER);

	GetDlgItemText(hDlg, IDC_OTHER_STR, ar->mrkinfo_other_str, MAX_COMMON_STRING_LEN);

	ar->mrkinfo_word_of_mouth_checked = (BYTE8)IsDlgButtonChecked(hDlg, IDC_WORDOFMOUTH);

}


/*
struct HeardOptions{

	char *option;	

	char *option_code;		

} HeardOptions[] =  {

	"              --- Please select ---",					"",

	"Interner Search",		"AF",
	"Magazine",					"",
	"Television"				"",
	"Radio",					"",
	"Newspaper",				"",
	"Gambling Portal",			"",
	"Other Internet Website",	"",
	"Outdoor Advertising",		"",
	"Refered by Friend",		"",
	"Other",					"",
	NULL, NULL

};
*/
			

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


//*********************************************************

// 1999/08/13 - MB

//

// Mesage handler for account settings dialog window

//

BOOL CALLBACK dlgFuncAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	static HWND tooltiphwnd;



	switch (message) {

	case WM_INITDIALOG:

		hCreateAccountDlg = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		tooltiphwnd = OpenToolTipWindow(hDlg, EditAccountToolTipText);

		{

			// Limit the # of characters in the various edit boxes

			static int max_edit_lengths[] =  {

				IDC_FULLNAME, MAX_PLAYER_FULLNAME_LEN,

				IDC_LASTNAME, MAX_PLAYER_LASTNAME_LEN,

				IDC_PASSWORD, MAX_PLAYER_PASSWORD_LEN,

			  #if 1	//19990819MB: specify lower limit so it fits on the player id boxes

				IDC_USERID, 13,

			  #else

				IDC_USERID, MAX_PLAYER_USERID_LEN,

			  #endif

				IDC_CITY, MAX_COMMON_STRING_LEN,

				IDC_EMAIL, MAX_EMAIL_ADDRESS_LEN,

				IDC_COMMENTS, 255,

				IDC_BANNER_STR, MAX_COMMON_STRING_LEN,

				IDC_MAGAZINE_STR, MAX_COMMON_STRING_LEN,

				IDC_NEWSGROUP_STR, MAX_COMMON_STRING_LEN,

				IDC_WEBSITE_STR, MAX_COMMON_STRING_LEN,

				IDC_OTHER_STR, MAX_COMMON_STRING_LEN,

				0,0

			};

			int *p = max_edit_lengths;

			while (*p) {

				SendMessage(GetDlgItem(hDlg, p[0]), EM_LIMITTEXT, p[1]-1, 0L);
				p += 2;

			}

			HWND combo = GetDlgItem(hDlg, IDC_HEART_ABOUT_US);
			
			SendMessage(combo, CB_RESETCONTENT, 0, 0);
			
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"              --- Please select ---");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Internet Search");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Magazine");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Television");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Radio");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Newspaper" );
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Gambling Portal");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Other Internet Website");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Outdoor Advertising");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Refered by Friend");
			SendMessage(combo, CB_ADDSTRING, 0,(LPARAM)"Other");

			
			SendMessage(combo, CB_SETCURSEL, 0 , 0); 

	}

		
		return TRUE;

	case WM_COMMAND:

		{

		
			switch (LOWORD(wParam)) {

			case IDOK:

				//kp(("%s(%d) IDOK received.\n",_FL));

				struct AccountRecord ar;

				zstruct(ar);

				ReadNewAccountFields(hDlg, &ar);

				//rgong 04/09/2002

				strcpy(ar.vendor_code, VendorCode);
				//end rgong
				
				//ricardoGANG 22/8/2003
				
				char idAffiliate[MAX_COMMON_STRING_LEN];
				strcpy(idAffiliate,"idAffiliate - Clear");
				//leer archivo af.dat para obtener el id.
				Affiliate af;
				FILE *afDat;
				afDat= fopen("af.dat","rb");
				if (afDat) {
					while(!(feof(afDat))){
						zstruct(af);
						fread(&af,sizeof(af),1,afDat);
						if(!(feof(afDat))){
							strcpy(idAffiliate,af.ID);
						}
					}
					fclose(afDat);
				} else {
					MessageBox(hDlg, "A file was not found, please download again.", "File Not Found", MB_OK);
					PostMessage(hInitialWindow,WMP_CLOSE_YOURSELF,0,0);
				}
				

				
				strnncpy(ar.sdb.idAffiliate,idAffiliate,11);
				//MessageBox(hDlg, ar.sdb.refered_by, "refered", MB_OK);
									
				//end ricardoGANG 22/8/2003

				//NOTE:  Must validate the idAffiliate field also, not implemented yet..


               #if  1
//ccv			  #if !ADMIN_CLIENT || 1

				if (ValidateNewAccountFields(hDlg, &ar))

			  #else

			  	kp1(("%s(%d) WARNING: NEW ACCOUNT FIELDS NOT VALIDATED!\n",_FL));

			  #endif

				{

					// DialogBox(hInst, MAKEINTRESOURCE(IDD_EMAIL_VALIDATION), hDlg, dlgFuncValidateEmail);


					// It validated... send it off to the server...

					hSubmitProgressDlg = CreateProgressWindow(hDlg,

							"Submitting account request to server...",

							60,	// start at 0%, go up to 60% while waiting

							7000);	// # of ms for indicator to get to that %



					ar.usage = ACCOUNTRECORDUSAGE_CREATE;

					strnncpy(SubmittedUserID, ar.sdb.user_id, MAX_PLAYER_USERID_LEN);

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

					LoggedInAccountRecord = ar;   ///////////

				} else{

				

				char str[200];

#if 0
				strcpy(str,"The following fields must contain information before continuing:\n\n");
				for (int i=0;i<=4;i++){
					if (strcmp(missingfield[i],"")){
					   strcat(str,missingfield[i]);
					   strcat(str,"\n");
					};//if
				};//for

#else
               strcpy(str,"The following fields must contain information before continuing:\n\n");  
               strcat(str,missingfield[0]);
			   strcat(str,"\n");
			   strcat(str,missingfield[1]);
			   strcat(str,"\n");
			   strcat(str,missingfield[2]);
			   strcat(str,"\n");
			   strcat(str,missingfield[3]);
			   strcat(str,"\n");
			   strcat(str,missingfield[4]);
			   strcat(str,"\n");

			
#endif                
				

				MessageBox( hDlg, str, "Verify Information",

	 			MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST );

				}



				return TRUE;	// We DID process this message.

			case IDCANCEL:

				//kp(("%s(%d) IDCANCEL received.  Cancelling upgrade.\n",_FL));

				EndDialog(hDlg, IDCANCEL);

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_DESTROY:

		CloseToolTipWindow(tooltiphwnd);	// Close our tooltip window

		tooltiphwnd = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		hCreateAccountDlg = NULL;

		return TRUE;	// We DID process this message

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WMP_MISC_CLIENT_MESSAGE:

		{

			struct MiscClientMessage *mcm = (struct MiscClientMessage *)lParam;

			pr(("%s(%d) received WMP_MISC_CLIENT_MESSAGE. mcm=$%08lx, mcm->message_type=%d, mcm->misc_data_1=%d\n",

					_FL, mcm, mcm->message_type, mcm->misc_data_1));



			// Finish animating the progress window and close the window in 200ms

			// (does not return until complete.)

			FinishProgressWindow(hSubmitProgressDlg, 200);

			hSubmitProgressDlg = NULL;

			switch (mcm->misc_data_1) {

			case 0:	// success... account created successfully.

				// Copy new defaults for the login dialog

				strnncpy(LoginUserID, SubmittedUserID, MAX_PLAYER_USERID_LEN);

				//strnncpy(LoggedInAccountRecord.sdb.email_address, SubmittedUserEmail, MAX_EMAIL_ADDRESS_LEN);
			
				
				LoginPassword[0] = 0;	// always clear the password

				EndDialog(hDlg, IDRETRY);	// send retry to Login dialog

				break;

			default:	// some kind of error... display the message

				ShowMiscClientMsg(hDlg, mcm, "Error");

				break;

			}

			pr(("%s(%d) Clearing misc client message after processing.\n",_FL));

			zstruct(*mcm);	// make sure it gets wiped out and not used again

			//kp(("%s(%d) finished processing WMP_MISC_CLIENT_MESSAGE\n",_FL));

		}

		return TRUE;	// We DID process this message

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/08/19 - MB

//

// Handle the receipt of a misc client message from the server

// related to creating an account.

// note: this is coming from the comm thread, rather than the

// window message thread.

//

void HandleCreateAccountResult(struct MiscClientMessage *mcm)

{

	pr(("%s(%d) HandleCreateAccountResult()\n",_FL));

	static struct MiscClientMessage local_mcm;

	local_mcm = *mcm;	// make a local copy (note that there's only one!)

	if (hCreateAccountDlg) {

		PostMessage(hCreateAccountDlg, WMP_MISC_CLIENT_MESSAGE, 0, (LPARAM)&local_mcm);

	} else {

		kp(("%s(%d) Warning: mcm for creating a new account received with no dialog open.\n",_FL));

	}

	pr(("%s(%d) HandleCreateAccountResult() done\n",_FL));

}



//*********************************************************

// 1999/08/13 - MB

//

// Create a new account.

// Returns IDCANCEL, IDOK, or IDRETRY.

//

int CreateNewAccount(void)

{

	if (!hCreateAccountDlg) {	// only display one copy

		return DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREATE), NULL, dlgFuncAccount);
	}

	return IDCANCEL;

}









//***** Country Codes *****

char *DisallowedCountries[] =  {

	"AF",	// "Afghanistan",

	"AO",	// "Angola",

	"CR",	// "Costa Rica",

	"CU",	// "Cuba",

	"BB",	// "Barbados",

	"IQ",	// "Iraq",

	"IR",	// "Iran (Islamic Republic of)",

	"KP",	// "Korea, Democratic People's Republic of", // North

	"LY",	// "Libyan Arab Jamahiriya",

	"SD",	// "Sudan",

	"SY",	// "Syrian Arab Republic",

	"VG",	// "Virgin Islands (British)",

	NULL

};


// Note: we've got up to 15 characters (16 less a nul) to store the

// the country name in the player record.

// The master copy of this list is in client\prefs.cpp

struct CountryCodes {

	char *abbrev15;	// 15 letter abbreviation

	char *abbrev2;	// 2 letter USPS abbreviation

	char *name;		// full name (for user display only)

} CountryCodes[] =  {

	"",					"",			"----- Select Your Country -----",

	"Afghanistan",		"AF",		"Afghanistan",

	"Albania",			"AL",		"Albania",

	"Algeria",			"DZ",		"Algeria",

	"American Samoa",	"AS",		"American Samoa",

	"Andorra",			"AD",		"Andorra",

	"Angola",			"AO",		"Angola",

	"Anguilla",			"AI",		"Anguilla",

	"Antarctica",		"AQ",		"Antarctica",

	"AG",				"AG",		"Antigua and Barbuda",

	"Argentina",		"AR",		"Argentina",

	"Armenia",			"AM",		"Armenia",

	"Aruba",			"AW",		"Aruba",

	"Australia",		"AU",		"Australia",

	"Austria",			"AT",		"Austria",

	"Azerbaijan",		"AZ",		"Azerbaijan",

	"Bahamas",			"BS",		"Bahamas",

	"Bahrain",			"BH",		"Bahrain",

	"Bangladesh",		"BD",		"Bangladesh",

	"Barbados",			"BB",		"Barbados",

	"Belarus",			"BY",		"Belarus",

	"Belgium",			"BE",		"Belgium",

	"Belize",			"BZ",		"Belize",

	"Benin",			"BJ",		"Benin",

	"Bermuda",			"BM",		"Bermuda",

	"Bhutan",			"BT",		"Bhutan",

	"Bolivia",			"BO",		"Bolivia",

	"BA",				"BA",		"Bosnia and Herzegovina",

	"Botswana",			"BW",		"Botswana",

	"Bouvet Island",	"BV",		"Bouvet Island",

	"Brazil",			"BR",		"Brazil",

	"IO",				"IO",		"British Indian Ocean Territory",

	"BN",				"BN",		"Brunei Darussalam",

	"Bulgaria",			"BG",		"Bulgaria",

	"Burkina Faso",		"BF",		"Burkina Faso",

	"Burundi",			"BI",		"Burundi",

	"Canada",			"CA",		"Canada",

	"Cambodia",			"KH",		"Cambodia",

	"Cameroon",			"CM",		"Cameroon",

	"Cape Verde",		"CV",		"Cape Verde",

	"Cayman Islands",	"KY",		"Cayman Islands",

	"CF",				"CF",		"Central African Republic",

	"Chad",				"TD",		"Chad",

	"Chile",			"CL",		"Chile",

	"China",			"CN",		"China",

	"Christmas Island",	"CX",		"Christmas Island",

	"Cocos",			"CC",		"Cocos (Keeling - Islands)",

	"Colombia",			"CO",		"Colombia",

	"Comoros",			"KM",		"Comoros",

	"Congo",			"CG",		"Congo",

	"Cook Islands",		"CK",		"Cook Islands",

	"Costa Rica",		"CR",		"Costa Rica",

	"Cote D'Ivoire",	"CI",		"Cote D'Ivoire",

	"Croatia",			"HR",		"Croatia (local name: Hrvatska)",

	"Cuba",				"CU",		"Cuba",

	"Cyprus",			"CY",		"Cyprus",

	"Czech Republic",	"CZ",		"Czech Republic",

	"Denmark",			"DK",		"Denmark",

	"Djibouti",			"DJ",		"Djibouti",

	"Dominica",			"DM",		"Dominica",

	"DO",				"DO",		"Dominican Republic",

	"East Timor",		"TP",		"East Timor",

	"Ecuador",			"EC",		"Ecuador",

	"Egypt",			"EG",		"Egypt",

	"El Salvador",		"SV",		"El Salvador",

	"GQ",				"GQ",		"Equatorial Guinea",

	"Eritrea",			"ER",		"Eritrea",

	"Estonia",			"EE",		"Estonia",

	"Ethiopia",			"ET",		"Ethiopia",

	"France",			"FR",		"France",

	"Finland",			"FI",		"Finland",

	"Falkland Islnds",	"FK",		"Falkland Islands (Malvinas)",

	"Faroe Islands",	"FO",		"Faroe Islands",

	"Fiji",				"FJ",		"Fiji",

	"French Guiana",	"GF",		"French Guiana",

	"PF",				"PF",		"French Polynesia",

	"TF",				"TF",		"French Southern Territories",

	"Germany",			"DE",		"Germany",

	"Greece",			"GR",		"Greece",

	"Gabon",			"GA",		"Gabon",

	"Gambia",			"GM",		"Gambia",

	"Georgia",			"GE",		"Georgia",

	"Ghana",			"GH",		"Ghana",

	"Gibraltar",		"GI",		"Gibraltar",

	"Greenland",		"GL",		"Greenland",

	"Grenada",			"GD",		"Grenada",

	"Guadeloupe",		"GP",		"Guadeloupe",

	"Guam",				"GU",		"Guam",

	"Guatemala",		"GT",		"Guatemala",

	"Guinea",			"GN",		"Guinea",

	"Guinea-Bissau",	"GW",		"Guinea-Bissau",

	"Guyana",			"GY",		"Guyana",

	"Haiti",			"HT",		"Haiti",

	"HM",				"HM",		"Heard and Mc Donald Islands",

	"Honduras",			"HN",		"Honduras",

	"Hong Kong",		"HK",		"Hong Kong",

	"Hungary",			"HU",		"Hungary",

	"Italy",			"IT",		"Italy",

	"Iceland",			"IS",		"Iceland",

	"India",			"IN",		"India",

	"Indonesia",		"ID",		"Indonesia",

	"Iran",				"IR",		"Iran (Islamic Republic of)",

	"Iraq",				"IQ",		"Iraq",

	"Ireland",			"IE",		"Ireland",

	"Israel",			"IL",		"Israel",

	"Jamaica",			"JM",		"Jamaica",

	"Japan",			"JP",		"Japan",

	"Jordan",			"JO",		"Jordan",

	"Kazakhstan",		"KZ",		"Kazakhstan",

	"Kenya",			"KE",		"Kenya",

	"Kiribati",			"KI",		"Kiribati",

	"KP",				"KP",		"Korea, Democratic People's Republic of", // North

	"KR",				"KR",		"Korea, Republic of",	// South

	"Kuwait",			"KW",		"Kuwait",

	"Kyrgyzstan",		"KG",		"Kyrgyzstan",

	"Lao PDR",			"LA",		"Lao People's Democratic Republic",

	"Latvia",			"LV",		"Latvia",

	"Lebanon",			"LB",		"Lebanon",

	"Lesotho",			"LS",		"Lesotho",

	"Liberia",			"LR",		"Liberia",

	"LY",				"LY",		"Libyan Arab Jamahiriya",

	"Liechtenstein",	"LI",		"Liechtenstein",

	"Lithuania",		"LT",		"Lithuania",

	"Luxembourg",		"LU",		"Luxembourg",

	"Mexico",			"MX",		"Mexico",

	"Macau",			"MO",		"Macau",

	"MK",				"MK",		"Macedonia, the former Yugoslav Republic of",

	"Madagascar",		"MG",		"Madagascar",

	"Malawi",			"MW",		"Malawi",

	"Malaysia",			"MY",		"Malaysia",

	"Maldives",			"MV",		"Maldives",

	"Mali",				"ML",		"Mali",

	"Malta",			"MT",		"Malta",

	"MarshallIslands",	"MH",		"Marshall Islands",

	"Martinique",		"MQ",		"Martinique",

	"Mauritania",		"MR",		"Mauritania",

	"Mauritius",		"MU",		"Mauritius",

	"Mayotte",			"YT",		"Mayotte",

	"FX",				"FX",		"Metropolitan France",

	"FM",				"FM",		"Micronesia, Federated States of",

	"MD",				"MD",		"Moldova, Republic of",

	"Monaco",			"MC",		"Monaco",

	"Mongolia",			"MN",		"Mongolia",

	"Montserrat",		"MS",		"Montserrat",

	"Morocco",			"MA",		"Morocco",

	"Mozambique",		"MZ",		"Mozambique",

	"Myanmar",			"MM",		"Myanmar",

	"Namibia",			"NA",		"Namibia",

	"Nauru",			"NR",		"Nauru",

	"Nepal",			"NP",		"Nepal",

	"Netherlands",		"NL",		"Netherlands",

	"New Caledonia",	"NC",		"New Caledonia",

	"New Zealand",		"NZ",		"New Zealand",

	"Nicaragua",		"NI",		"Nicaragua",

	"Niger",			"NE",		"Niger",

	"Nigeria",			"NG",		"Nigeria",

	"Niue",				"NU",		"Niue",

	"Norfolk Island",	"NF",		"Norfolk Island",

	"MP",				"MP",		"Northern Mariana Islands",

	"Norway",			"NO",		"Norway",

	"Oman",				"OM",		"Oman",

	"Pakistan",			"PK",		"Pakistan",

	"Palau",			"PW",		"Palau",

	"Panama",			"PA",		"Panama",

	"Papua NewGuinea",	"PG",		"Papua New Guinea",

	"Paraguay",			"PY",		"Paraguay",

	"Peru",				"PE",		"Peru",

	"Philippines",		"PH",		"Philippines",

	"Pitcairn",			"PN",		"Pitcairn",

	"Poland",			"PL",		"Poland",

	"Portugal",			"PT",		"Portugal",

	"Puerto Rico",		"PR",		"Puerto Rico",

	"Qatar",			"QA",		"Qatar",

	"Reunion",			"RE",		"Reunion",

	"Romania",			"RO",		"Romania",

	"Russian Fed.",		"RU",		"Russian Federation",

	"Rwanda",			"RW",		"Rwanda",

	"Spain",			"ES",		"Spain",

	"Switzerland",		"CH",		"Switzerland",

	"Sweden",			"SE",		"Sweden",

	"KN",				"KN",		"Saint Kitts and Nevis",

	"Saint Lucia",		"LC",		"Saint Lucia",

	"VC",				"VC",		"Saint Vincent and the Grenadines",

	"Samoa",			"WS",		"Samoa",

	"San Marino",		"SM",		"San Marino",

	"ST",				"ST",		"Sao Tome and Principe",

	"Saudi Arabia",		"SA",		"Saudi Arabia",

	"Senegal",			"SN",		"Senegal",

	"Seychelles",		"SC",		"Seychelles",

	"Sierra Leone",		"SL",		"Sierra Leone",

	"Singapore",		"SG",		"Singapore",

	"Slovakia",			"SK",		"Slovakia (Slovak Republic)",

	"Slovenia",			"SI",		"Slovenia",

	"Solomon Islands",	"SB",		"Solomon Islands",

	"Somalia",			"SO",		"Somalia",

	"South Africa",		"ZA",		"South Africa",

	"Sri Lanka",		"LK",		"Sri Lanka",

	"St. Helena",		"SH",		"St. Helena",

	"PM",				"PM",		"St. Pierre and Miquelon",

	"Sudan",			"SD",		"Sudan",

	"Suriname",			"SR",		"Suriname",

	"SJ",				"SJ",		"Svalbard and Jan Mayen Islands",

	"Swaziland",		"SZ",		"Swaziland",

	"SY",				"SY",		"Syrian Arab Republic",

	"Taiwan",			"TW",		"Taiwan",

	"Tajikistan",		"TJ",		"Tajikistan",

	"TZ",				"TZ",		"Tanzania, United Republic of",

	"Thailand",			"TH",		"Thailand",

	"Netherlands",		"NL",		"The Netherlands",

	"Togo",				"TG",		"Togo",

	"Tokelau",			"TK",		"Tokelau",

	"Tonga",			"TO",		"Tonga",

	"TT",				"TT",		"Trinidad and Tobago",

	"Tunisia",			"TN",		"Tunisia",

	"Turkey",			"TR",		"Turkey",

	"Turkmenistan",		"TM",		"Turkmenistan",

	"TC",				"TC",		"Turks and Caicos Islands",

	"Tuvalu",			"TV",		"Tuvalu",

	"United States",	"US",		"United States",

	"United Kingdom",	"GB",		"United Kingdom",

	"Uganda",			"UG",		"Uganda",

	"Ukraine",			"UA",		"Ukraine",

	"AE",				"AE",		"United Arab Emirates",

//	"UM",				"UM",		"United States Minor Outlying Islands",

	"Uruguay",			"UY",		"Uruguay",

	"Uzbekistan",		"UZ",		"Uzbekistan",

	"Vanuatu",			"VU",		"Vanuatu",

	"VA",				"VA",		"Vatican City State",

	"Venezuela",		"VE",		"Venezuela",

	"Viet Nam",			"VN",		"Viet Nam",

	"VG",				"VG",		"Virgin Islands (British)",

	"VI",				"VI",		"Virgin Islands (U.S.)",

	"WF",				"WF",		"Wallis And Futuna Islands",

	"Western Sahara",	"EH",		"Western Sahara",

	"Yemen",			"YE",		"Yemen",

	"Yugoslavia",		"YU",		"Yugoslavia",

	"Zaire",			"ZR",		"Zaire",

	"Zambia",			"ZM",		"Zambia",

	"Zimbabwe",			"ZW",		"Zimbabwe",

	NULL, NULL, NULL

};


//*********************************************************

// 2003/11/04 - J. Fonseca

//

// Read and validate the fields for date from the Real Money account setup dialog box

// return: TRUE for valid, FALSE for invalid

int Validate_Date(char *day, char *month, char *year){


	int res = true;

	if ((strlen(day) == 0) && (strlen(month) == 0) && (strlen(year) == 0)){
		return false;	
	}else{
		if ((strlen(day) < 2) || (strlen(month) < 2) || (strlen(year) < 4)){
			return false;
		}
	}
	
	if ((atoi(day) < 1) || (atoi(month) < 1) || (atoi(year) < 1)){
		return false;
	}else{
		switch(atoi(month)){
			
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12: 
				if (atoi(day) > 31) res=false;
				
			break; 

			case 4:
			case 6:
			case 9:
			case 11: 
				if (atoi(day) > 30) res=false;				
			break; 

			case 2: 
				if ((atoi(year) % 4 ==0) && (atoi(day) > 29)) 
					res=false;
				
				if ((atoi(year) % 4 != 0) && (atoi(day) > 28))
					res=false;
			break; 

			default:
				res=false;			
			break;			

		}//swith 
		return res;
	}//else  
}


//*********************************************************

// 1999/11/10 - MB

//

// Read all the fields from the Real Money account setup dialog box

// return: TRUE for valid, FALSE for invalid

// 20001003HK: added address_only flag (doesn't copy full name or email)

int ReadRealMoneyAccountFields(HWND hDlg, struct AccountRecord *ar, int address_only)

{

	// Fetch account info fields...

  char day[3];
  char month[3];
  char year[5];
  char date[14];

  #if 0	//19991111MB

	GetDlgItemText(hDlg, IDC_PASSWORD,		ar->sdb.password_rm, MAX_PLAYER_PASSWORD_LEN);

  #endif

	if (!address_only) {

		GetDlgItemText(hDlg, IDC_EMAIL,			ar->sdb.email_address, MAX_EMAIL_ADDRESS_LEN);

		GetDlgItemText(hDlg, IDC_FULLNAME,		ar->sdb.full_name, MAX_PLAYER_FULLNAME_LEN);

		GetDlgItemText(hDlg, IDC_LASTNAME,		ar->sdb.last_name, MAX_PLAYER_LASTNAME_LEN);
	}

	GetDlgItemText(hDlg, IDC_ADDRESS_LINE1, ar->sdb.mailing_address1, MAX_PLAYER_ADDRESS_LEN);

	GetDlgItemText(hDlg, IDC_ADDRESS_LINE2, ar->sdb.mailing_address2, MAX_PLAYER_ADDRESS_LEN);

	GetDlgItemText(hDlg, IDC_CITY, 			ar->sdb.city, MAX_COMMON_STRING_LEN);

	GetDlgItemText(hDlg, IDC_STATE,			ar->sdb.mailing_address_state, MAX_COMMON_STRING_LEN);

	GetDlgItemText(hDlg, IDC_POSTAL_CODE,	ar->sdb.mailing_address_postal_code, MAX_COMMON_STRING_LEN);

	GetDlgItemText(hDlg, IDC_USER_MI,	ar->sdb.user_mi, 3);

	/*******J Fonseca   03/11/2003************/
	GetDlgItemText(hDlg, IDC_DAY,	day, 3);
	GetDlgItemText(hDlg, IDC_MONTH,	month, 3);
	GetDlgItemText(hDlg, IDC_YEAR,	year, 5);

	strcpy(date, year);
	strcat(date, "-");
	strcat(date, month);
	strcat(date, "-");
	strcat(date, day);

	strncpy(ar->sdb.birth_date, date,MAX_COMMON_STRING_LEN);

	ar->sdb.gender = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_FEMALE) ? GENDER_FEMALE : GENDER_MALE);

	if (IsDlgButtonChecked(hDlg, IDC_FEMALE)){
		strcpy(ar->sdb.gender1,"Female"); 
	}
	else{
		strcpy(ar->sdb.gender1,"Male");
	}
	/*******J Fonseca  03/11/2003************/
	
	GetDlgItemText(hDlg, IDC_PHONE,	ar->sdb.phone_number, PHONE_NUM_LEN);
	GetDlgItemText(hDlg, IDC_PHONE2,ar->sdb.alternative_phone, PHONE_NUM_LEN);
	
	char _phone_number[PHONE_NUM_EXPANDED_LEN+2];

	zstruct(_phone_number);

	GetDlgText(hDlg, IDC_EDIT_PHONE_NUMBER, _phone_number, PHONE_NUM_EXPANDED_LEN+1);

	//EncodePhoneNumber(ar->sdb.phone_number, _phone_number);



	int country_index = SendMessage(GetDlgItem(hDlg, IDC_COMBO_COUNTRY), CB_GETCURSEL, 0, 0);

	strnncpy(ar->sdb.mailing_address_country, CountryCodes[country_index].abbrev15, MAX_COMMON_STRING_LEN);



	// Determine if this country is allowed or not.

	for (int i=0 ; DisallowedCountries[i] ; i++) {

		if (!stricmp(DisallowedCountries[i], CountryCodes[country_index].abbrev2)) {

			// This country is not allowed.

			MessageBox(hDlg,

				"We are sorry, but residents of your country are\n"

				"not permitted to play at e-Media Poker.\n"

				"\n"

				"Write to support@https://github.com/kriskoinif you need\n"

				"more information about this restriction.\n",

				"Country not permitted",

				MB_OK|MB_ICONHAND|MB_APPLMODAL);

			return FALSE;

		}

	}



	return TRUE;

}




//*********************************************************

// 1999/08/18 - MB

//

// Validate whether the user typed in something legal for all

// the required fields.

// return: TRUE for valid, FALSE for invalid

//

static int ValidateRealMoneyAccountFields(struct AccountRecord *ar)

{
	
    missingfield[0]=vname;

	missingfield[1]=vaddress;

    missingfield[2]=vcity;

	missingfield[3]=vprovince;

	missingfield[4]=vcountry;

    for (int vi=0; vi<50; vi++)

	{

		vcountry[vi]=0;

        vname[vi]=0;

		vcity[vi]=0;

		vprovince[vi]=0;

		vaddress[vi]=0;

	}



	int retval= TRUE;  

	int index =0;


//	if (strlen(ar->sdb.full_name) > 0){
		if (!ValidateString(ar->sdb.full_name, 2, MAX_PLAYER_FULLNAME_LEN, "Full Name")) {
	
			strcpy(missingfield[index], "Player Name");

			index++;

			retval=FALSE;
		}

//	}

	if (!ValidateString(ar->sdb.last_name, 2, MAX_PLAYER_LASTNAME_LEN, "Last Name")) {

		strcpy(missingfield[index], "Last Name");

		index++;

		retval=FALSE;

	}



	if (strlen(ar->sdb.birth_date ) < 1) {

		strcpy(missingfield[index], "Birth Date");

		index++;

		retval=FALSE;

	}


	if (!ValidateString(ar->sdb.mailing_address1, 5, MAX_PLAYER_ADDRESS_LEN, "Address")) {

		strcpy(missingfield[index], "Mailing Address");

		index++;

		retval=FALSE;

	}

	if (!ValidateString(ar->sdb.city,  2, MAX_COMMON_STRING_LEN, "City")) {

		strcpy(missingfield[index], "City/Province");

		index++;

		retval=FALSE;

	}

	

	if ((strcmp(ar->sdb.mailing_address_country, "United States") == 0) || (strcmp(ar->sdb.mailing_address_country, "Canada") == 0)) {
		if (strlen(ar->sdb.mailing_address_state) < 2) {
			strcpy(missingfield[index], "State");
			index++;
			retval=FALSE;

		}

		if (strlen(ar->sdb.mailing_address_postal_code) < 2) {
			strcpy(missingfield[index], "ZIP/Postal Code");
			index++;
			retval=FALSE;

		}
    }


	if (strlen(ar->sdb.mailing_address_country) < 2) {
		strcpy(missingfield[index], "Country");
		retval=FALSE;
	}
    
	

	return retval;

}



/**********************************************************************************

 void FixUpPhoneNumberEditControl()

 Date: 20010306HK

 Purpose: called when something is typed into the control -- remove illegal characters

***********************************************************************************/

void FixUpPhoneNumberEditControl(HWND hDlg)

{

	char src_str[PHONE_NUM_EXPANDED_LEN+2];

	char out_str[PHONE_NUM_EXPANDED_LEN+2];

	zstruct(src_str);

	zstruct(out_str);

	GetDlgText(hDlg, IDC_EDIT_PHONE_NUMBER, src_str, PHONE_NUM_EXPANDED_LEN+1);

	for (int i=0; i < PHONE_NUM_EXPANDED_LEN+1; i++) {

		if (!strchr("01234567890 ()+-", src_str[i])) {

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PHONE_NUMBER), EM_SETSEL, (WPARAM)(INT)i, (LPARAM)(INT)i+1);

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PHONE_NUMBER), EM_REPLACESEL, (WPARAM)(BOOL)FALSE, (LPARAM)(LPCTSTR)"");

		}

	}

}



/**********************************************************************************

 Function CALLBACK dlgFuncUpdateAddressInfo()

 Date: HK00/10/03

 Purpose: allow the user to update his own address information (similar to setting up for real $)

***********************************************************************************/

BOOL CALLBACK dlgFuncUpdateAddressInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	static HWND tooltiphwnd;

	switch (message) {

	case WM_INITDIALOG:

		hUpdateAddressDlg = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		tooltiphwnd = OpenToolTipWindow(hDlg, MakeRealMoneyReadyToolTipText);

		{

			// Limit the # of characters in the various edit boxes

			static int max_edit_lengths[] =  {

				IDC_FULLNAME,		MAX_PLAYER_FULLNAME_LEN,

				IDC_LASTNAME,		MAX_PLAYER_LASTNAME_LEN,

				IDC_ADDRESS_LINE1,	MAX_PLAYER_ADDRESS_LEN,

				IDC_ADDRESS_LINE2,	MAX_PLAYER_ADDRESS_LEN,

				IDC_CITY,			MAX_COMMON_STRING_LEN,

				IDC_STATE,			MAX_COMMON_STRING_LEN,

				IDC_COUNTRY,		MAX_COMMON_STRING_LEN,

				IDC_POSTAL_CODE,	MAX_COMMON_STRING_LEN,

				IDC_PHONE,			PHONE_NUM_LEN,

				0,0

			};

			int *p = max_edit_lengths;

			while (*p) {

				SendMessage(GetDlgItem(hDlg, p[0]), EM_LIMITTEXT, p[1]-1, 0L);

				p += 2;

			}

				
			// Fill in any fields we have info for...

			SetWindowText(GetDlgItem(hDlg, IDC_USERID), LoginUserID);

			SetWindowText(GetDlgItem(hDlg, IDC_FULLNAME), LoggedInAccountRecord.sdb.full_name);

			SetWindowText(GetDlgItem(hDlg, IDC_LASTNAME), LoggedInAccountRecord.sdb.last_name);
			
			SetWindowText(GetDlgItem(hDlg, IDC_ADDRESS_LINE1), LoggedInAccountRecord.sdb.mailing_address1);

			SetWindowText(GetDlgItem(hDlg, IDC_ADDRESS_LINE2), LoggedInAccountRecord.sdb.mailing_address2);

			SetWindowText(GetDlgItem(hDlg, IDC_CITY),  LoggedInAccountRecord.sdb.city);

			SetWindowText(GetDlgItem(hDlg, IDC_STATE), LoggedInAccountRecord.sdb.mailing_address_state);

			SetWindowText(GetDlgItem(hDlg, IDC_POSTAL_CODE), LoggedInAccountRecord.sdb.mailing_address_postal_code);

			SetWindowText(GetDlgItem(hDlg, IDC_PHONE), LoggedInAccountRecord.sdb.phone_number);

			SetWindowText(GetDlgItem(hDlg, IDC_PHONE2), LoggedInAccountRecord.sdb.alternative_phone);

			SetWindowText(GetDlgItem(hDlg, IDC_EMAIL), LoggedInAccountRecord.sdb.email_address);

			SetWindowText(GetDlgItem(hDlg, IDC_USER_MI), LoggedInAccountRecord.sdb.user_mi);

			char seps[]   = "-";
			char *token;

		    token = strtok( LoggedInAccountRecord.sdb.birth_date, seps );

			SetWindowText(GetDlgItem(hDlg, IDC_YEAR), token);
			token = strtok( NULL, seps );      //Get next token:
			SetWindowText(GetDlgItem(hDlg, IDC_MONTH), token);
			token = strtok( NULL, seps );      //Get next token:
			SetWindowText(GetDlgItem(hDlg, IDC_DAY), token);
      
			
			if (!strcmp(LoggedInAccountRecord.sdb.gender1,"Female"))
				CheckDlgButton(hDlg, IDC_FEMALE,BST_CHECKED);						
			else
				CheckDlgButton(hDlg, IDC_MALE,BST_CHECKED);							
			


			if (LoggedInPrivLevel >= ACCPRIV_PLAY_MONEY) {

				EnableWindow(GetDlgItem(hDlg, IDC_CITY), TRUE);

			}

			if (LoggedInPrivLevel >= ACCPRIV_REAL_MONEY) {

				EnableWindow(GetDlgItem(hDlg, IDC_ADDRESS_LINE1), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_ADDRESS_LINE2), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_CITY), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_STATE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_POSTAL_CODE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_COUNTRY), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_PHONE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_PHONE2), TRUE);

			}

			// Fill in the fields for the country combo box and pick a default

			HWND combo = GetDlgItem(hDlg, IDC_COMBO_COUNTRY);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0);

			struct CountryCodes *cc = CountryCodes;

			int count = 0;

			int country_index = 0;

			while (cc->name) {

				//kp(("%s(%d) Adding country '%s'\n", _FL, cc->name));

		        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)cc->name);

				if (!stricmp(cc->abbrev15, LoggedInAccountRecord.sdb.mailing_address_country)) {

					country_index = count;

				}

				cc++;

				count++;

			}

			SendMessage(combo, CB_SETCURSEL, (WPARAM)country_index, 0);
			      
		}
		return TRUE;

	case WM_COMMAND:

		{

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {



			case IDOK:

				struct AccountRecord ar;
				ar = LoggedInAccountRecord;	// fill in defaults

				// we only care if the information looks valid on a Real Money account

				char date[12];
				char day[3];
				char month[3];
				char year[5];
				char temp[2];



			
				GetDlgItemText(hDlg, IDC_FULLNAME, ar.sdb.full_name, MAX_PLAYER_FULLNAME_LEN);
				GetDlgItemText(hDlg, IDC_LASTNAME, ar.sdb.last_name, MAX_PLAYER_LASTNAME_LEN);
				GetDlgItemText(hDlg, IDC_CITY, ar.sdb.city, MAX_COMMON_STRING_LEN);
				GetDlgItemText(hDlg, IDC_PHONE, ar.sdb.phone_number, PHONE_NUM_LEN);
				GetDlgItemText(hDlg, IDC_PHONE2, ar.sdb.alternative_phone, PHONE_NUM_LEN);
				GetDlgItemText(hDlg, IDC_USER_MI, ar.sdb.user_mi, 3);
				
				GetDlgItemText(hDlg, IDC_MONTH,month, 3);				
				GetDlgItemText(hDlg, IDC_DAY,day, 3);
				GetDlgItemText(hDlg, IDC_YEAR,year, 5);
					
				strcpy(date, year);
				strcat(date, "-");
				strcat(date, month);
				strcat(date, "-");
				strcat(date, day);

				strncpy(ar.sdb.birth_date, date,MAX_COMMON_STRING_LEN);

				
				if (LoggedInPrivLevel == ACCPRIV_PLAY_MONEY) {
					

			
					if (!ValidateRealMoneyAccountFields(&ar)) {

					// data validation problem... don't continue.

						char str[200];

  						strcpy(str,"The following fields must contain information before continuing:\n\n");
						for (int i=0;i<=4;i++){
							if (strcmp(missingfield[i],"")){
								strcat(str,missingfield[i]);
								strcat(str,"\n");
							};//if
						};//for

						 MessageBox(hDlg, str, "Verify Information",
						  
									MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST); 

						  return true;	// done processing this message. 

					} 
					
					if (!Validate_Date(day, month, year)){
						MessageBox(hDlg, "              Invalid Date \nDate Format:  mm/dd/yyyy", "Verify Information", MB_OK|MB_APPLMODAL|MB_TOPMOST);
						return true;
					}
					
					

					if (IsDlgButtonChecked(hDlg, IDC_FEMALE)){
						strcpy(ar.sdb.gender1,"Female"); 
					}
					else{
						strcpy(ar.sdb.gender1,"Male");
					}
					

					if (strlen(ar.sdb.city)==0) {

					    MessageBox(hDlg, "Please Input the City Address.", "Verify Information",

								MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST); 

						return FALSE;

					}

				
				int  result=MessageBox(hDlg,"Are you sure that you wish to complete\n"

									"this setup and that all information\n"

									"you have provided is correct and accurate?",

									"e-Media Poker", MB_YESNO|MB_APPLMODAL);



				if (result==IDYES) {

						// fluff: progress bar

						HWND hSubmitProgressDlg = NULL;

						hSubmitProgressDlg = CreateProgressWindow(hDlg,

							"Submitting Address Change to server...",

							60,	// start at 0%, go up to 60% while waiting

							1600);	// # of ms for indicator to get to that %

						
//						MessageBox(hDlg,"You must meet the three requirements to continue.\n\n","e-Media Poker", MB_OK|MB_TOPMOST);
						
						ar.usage = ACCOUNTRECORDUSAGE_SUBMIT_ADDRESS;	// update our settings

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						if (hSubmitProgressDlg) {

							FinishProgressWindow(hSubmitProgressDlg, 200);

						}

						EndDialog(hDlg, IDOK);

					}


				}





				if (LoggedInPrivLevel > ACCPRIV_PLAY_MONEY) {

					if (!ReadRealMoneyAccountFields(hDlg, &ar, TRUE)) {

						// data validation problem... don't continue.

						return TRUE;	// done processing this message.

					}



					if (!ValidateRealMoneyAccountFields(&ar)) {

					// data validation problem... don't continue.

						char str[200];

  						strcpy(str,"The following fields must contain information before continuing:\n\n");
						for (int i=0;i<=4;i++){
							if (strcmp(missingfield[i],"")){
								strcat(str,missingfield[i]);
								strcat(str,"\n");
							};//if
						};//for

						 MessageBox(hDlg, str, "Verify Information",
						  
									MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST); 

						  return true;	// done processing this message. 

					} 				

				

				
				if (!Validate_Date(day, month, year)){
					MessageBox(hDlg, "              Invalid Date \nDate Format:  mm/dd/yyyy", "Verify Information", MB_OK|MB_APPLMODAL|MB_TOPMOST);
					return true;
				}

				  
				// Ask the user for one last confirmation of the data...
				 int  result=MessageBox(hDlg,"Are you sure that you wish to complete\n"

									"this setup and that all information\n"

									"you have provided is correct and accurate?",

									"e-Media Poker", MB_YESNO|MB_APPLMODAL);



					if (result==IDYES) {

						// fluff: progress bar

						HWND hSubmitProgressDlg = NULL;

						hSubmitProgressDlg = CreateProgressWindow(hDlg,

							"Submitting Address Change to server...",

							60,	// start at 0%, go up to 60% while waiting

							1600);	// # of ms for indicator to get to that %

						ar.usage = ACCOUNTRECORDUSAGE_SUBMIT_ADDRESS;	// update our settings

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						if (hSubmitProgressDlg) {

							FinishProgressWindow(hSubmitProgressDlg, 200);

						}

						EndDialog(hDlg, IDOK);

					}

				}

				return TRUE;	// We DID process this message.





			case IDCANCEL:

				EndDialog(hDlg, IDCANCEL);

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_DESTROY:

		CloseToolTipWindow(tooltiphwnd);	// Close our tooltip window

		tooltiphwnd = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		hUpdateAddressDlg = NULL;

		return TRUE;	// We DID process this message

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/11/08 - MB

//

// Mesage handler for the 'Make account real money ready' dialog window

//

BOOL CALLBACK dlgFuncMakeRealMoneyReady(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	static HWND tooltiphwnd;

	static int contract_read = IDCANCEL;

	switch (message) {

	case WM_INITDIALOG:

		hMakeRealMoneyDlg = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		tooltiphwnd = OpenToolTipWindow(hDlg, MakeRealMoneyReadyToolTipText);

		contract_read = IDCANCEL;

		{

			// Limit the # of characters in the various edit boxes

			static int max_edit_lengths[] =  {

				IDC_USERID,		    MAX_COMMON_STRING_LEN,
	
				IDC_PASSWORD,		MAX_PLAYER_PASSWORD_LEN,

				IDC_PASSWORD2,		MAX_PLAYER_PASSWORD_LEN,

				IDC_EMAIL,			MAX_EMAIL_ADDRESS_LEN,

				IDC_FULLNAME,		MAX_PLAYER_FULLNAME_LEN,

				IDC_LASTNAME,		MAX_PLAYER_LASTNAME_LEN,

				IDC_ADDRESS_LINE1,	MAX_PLAYER_ADDRESS_LEN,

				IDC_ADDRESS_LINE2,	MAX_PLAYER_ADDRESS_LEN,

				IDC_CITY,			MAX_COMMON_STRING_LEN,

				IDC_STATE,			MAX_COMMON_STRING_LEN,

				IDC_COUNTRY,		MAX_COMMON_STRING_LEN,

				IDC_POSTAL_CODE,	MAX_COMMON_STRING_LEN,

				IDC_PHONE,			PHONE_NUM_LEN,

				IDC_PHONE2,			PHONE_NUM_LEN,

				IDC_MONTH,          3,

				IDC_DAY,            3,

				IDC_YEAR,           5,

				0,0

			};

			int *p = max_edit_lengths;

			while (*p) {

				SendMessage(GetDlgItem(hDlg, p[0]), EM_LIMITTEXT, p[1]-1, 0L);

				p += 2;

			}

			// Fill in any fields we have info for...

			SetWindowText(GetDlgItem(hDlg, IDC_USERID), LoginUserID);

			SetWindowText(GetDlgItem(hDlg, IDC_EMAIL), LoggedInAccountRecord.sdb.email_address);

			SetWindowText(GetDlgItem(hDlg, IDC_FULLNAME), LoggedInAccountRecord.sdb.full_name);

			SetWindowText(GetDlgItem(hDlg, IDC_LASTNAME), LoggedInAccountRecord.sdb.last_name);

			SetWindowText(GetDlgItem(hDlg, IDC_ADDRESS_LINE1), LoggedInAccountRecord.sdb.mailing_address1);

			SetWindowText(GetDlgItem(hDlg, IDC_ADDRESS_LINE2), LoggedInAccountRecord.sdb.mailing_address2);

			SetWindowText(GetDlgItem(hDlg, IDC_CITY),  LoggedInAccountRecord.sdb.city);

			SetWindowText(GetDlgItem(hDlg, IDC_STATE), LoggedInAccountRecord.sdb.mailing_address_state);

			SetWindowText(GetDlgItem(hDlg, IDC_POSTAL_CODE), LoggedInAccountRecord.sdb.mailing_address_postal_code);

			SetWindowText(GetDlgItem(hDlg, IDC_PHONE), LoggedInAccountRecord.sdb.phone_number);

			SetWindowText(GetDlgItem(hDlg, IDC_USER_MI), LoggedInAccountRecord.sdb.user_mi);
		
			// Fill in the fields for the country combo box and pick a default

			HWND combo = GetDlgItem(hDlg, IDC_COMBO_COUNTRY);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0);

			struct CountryCodes *cc = CountryCodes;

			int count = 0;

			int country_index = 0;

			while (cc->name) {

				//kp(("%s(%d) Adding country '%s'\n", _FL, cc->name));

		        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)cc->name);

				if (!stricmp(cc->abbrev15, LoggedInAccountRecord.sdb.mailing_address_country)) {

					country_index = count;

				}

				cc++;

				count++;

			}

			//SendMessage(combo, CB_SETCURSEL, (WPARAM)country_index, 0);
			SendMessage(combo, CB_SETCURSEL, 218, 0); //Default  United States

		}

		return TRUE;

	case WM_COMMAND:

		{

			// Process other buttons on the window...

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
			
			
			case IDOK:

				struct AccountRecord ar;
				ar = LoggedInAccountRecord;	// fill in defaults

				

				if (!ReadRealMoneyAccountFields(hDlg, &ar, FALSE)) {

					// data validation problem... don't continue.
									
					return TRUE;	// done processing this message.

				}

				
				if (!IsDlgButtonChecked(hDlg, IDC_CHECK1) || 

					!IsDlgButtonChecked(hDlg, IDC_CHECK11) ||

					!IsDlgButtonChecked(hDlg, IDC_CHECK10))	{ 

                   MessageBox(hDlg,"You must meet the three requirements to continue.\n\n","e-Media Poker", MB_OK|MB_TOPMOST);
				} 				

		
				if (!ValidateRealMoneyAccountFields(&ar)) {

					// data validation problem... don't continue.

					char str[200];

  				    strcpy(str,"The following fields must contain information before continuing:\n\n");
					  for (int i=0;i<=4;i++){
						if (strcmp(missingfield[i],"")){
							strcat(str,missingfield[i]);
							strcat(str,"\n");
						};//if
					  };//for

					 MessageBox(hDlg, str, "Verify Information",
					  
								MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST); 

					  return true;	// done processing this message. 

				} 

			
			/*******J Fonseca   04/11/2003************/				
				  char day[3];
				  char month[3];
				  char year[5];
  
				  GetDlgItemText(hDlg, IDC_DAY,	day, 3);
				  GetDlgItemText(hDlg, IDC_MONTH,	month, 3);
				  GetDlgItemText(hDlg, IDC_YEAR,	year, 5);
				  if (!Validate_Date(day, month, year)){
					MessageBox(hDlg, "              Invalid Date \nDate Format:  mm/dd/yyyy", "Verify Information", MB_OK|MB_APPLMODAL|MB_TOPMOST);
					return true;
				  }
			    

				  /*******J Fonseca   04/11/2003************/


				// Ask the user for one last confirmation of the data...

				{	char str[500];

					zstruct(str);

					sprintf(str, "Your account will be created with the following information:\n\n"

								"Email: %s\n\n"

								"Name and Address:\n"

								"   %s\n"

								"   %s\n"

								"%s%s%s"

								"   %s, %s, %s\n"

								"   %s\n\n"

								"Is the above information correct?",

								ar.sdb.email_address,

								ar.sdb.full_name,

								ar.sdb.last_name,

								ar.sdb.mailing_address1,

								ar.sdb.mailing_address2[0] ? "   " : "",

								ar.sdb.mailing_address2,

								ar.sdb.mailing_address2[0] ? "\n" : "",

								ar.sdb.city,

								ar.sdb.mailing_address_state,

								ar.sdb.mailing_address_country,

								ar.sdb.mailing_address_postal_code

					);

					int result = IDYES;// MessageBox(hDlg, str, "Verify Information",

								//MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

				

				    result=MessageBox(hDlg,"Are you sure that you wish to complete\n"

									"this setup and that all information\n"

									"you have provided is correct and accurate?",

									"e-Media Poker", MB_YESNO|MB_APPLMODAL);

				

					if (result==IDYES) {

						iDisplayedBadEmailNotice = TRUE;		// don't display bad email message this session

						ar.usage = ACCOUNTRECORDUSAGE_UPDATE;	// update our settings

						strcpy(ar.sdb.user_id,LoginUserID);

									
						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						EndDialog(hDlg, IDOK);

						backbutton=0;

						LoggedInAccountRecord = ar;  //////

					}

				}

				

				return TRUE;	// We DID process this message.

			case IDCANCEL:

				EndDialog(hDlg, IDCANCEL);

				backbutton=1;

	        	/*retval=DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_REALMONEY_ACCOUNT_SETUP1), HWND_DESKTOP,

						(DLGPROC)dlgFuncRealMoneyAccountSetup1, NULL);
				*/
				OpenCashierScreen();
       			

				return TRUE;	// We DID process this message

			case IDC_READ_DISCLOSURE:

				//contract_read = DialogBox(hInst, MAKEINTRESOURCE(IDD_READ_CONTRACT), hDlg, dlgFuncReadContract);

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_DESTROY:

		CloseToolTipWindow(tooltiphwnd);	// Close our tooltip window

		tooltiphwnd = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		hMakeRealMoneyDlg = NULL;

		return TRUE;	// We DID process this message

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/11/08 - MB

//

// Make sure our account is real money ready.

// Returns IDCANCEL, IDOK, or IDRETRY.

//

int MakeAccountRealMoneyReady(void)

{

	if (!hMakeRealMoneyDlg) {

    BOOL retval; 



		//if (MiscMsgCreateAccountWarning.msg[0]) {

			// Display a warning box... (text comes from the server)

		//	MessageBox(hCashierDlg, MiscMsgCreateAccountWarning.msg,

		//			"Setting up for Real Money...", MB_OK|MB_ICONASTERISK|MB_TOPMOST|MB_APPLMODAL);

		//}


		DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREATE2), hCashierDlg, dlgFuncMakeRealMoneyReady);
        /*retval=DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_REALMONEY_ACCOUNT_SETUP1), HWND_DESKTOP,

						(DLGPROC)dlgFuncRealMoneyAccountSetup1, NULL);

    */

	}

	ReallySetForegroundWindow(hMakeRealMoneyDlg);

	return IDCANCEL;

}



//*********************************************************

// 1999/11/10 - MB

//

// Intiate the process for the user to set up their account for

// real money.  This is a function called by the message thread

// to prompt the user about what to do, then bring up the cashier

// for them.

//

void InitiateRealMoneyAccountSetup(void)

{

	int result = MessageBox(hInitialWindow,

			"Your account is not yet ready to play\n"

			"at a Real Money table.\n\n"

			"The cashier can help you get it set up.",

			"Real Money account setup...",

			MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

	if (result==IDOK) {  //J Fonseca   30/01/2003
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_CREATE2), hCashierDlg, dlgFuncMakeRealMoneyReady);
		//OpenCashierScreen();
	}

}



//*********************************************************

// 1999/09/23 - MB

//

// Process the receipt of a new account record from the server

// (this is called from the comm thread and must not block)

//

ErrorType ProcessAccountRecord(struct AccountRecord *ar, int input_structure_len, int packet_type)

{

	if (sizeof(*ar) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) AccountRecord was wrong length (%d vs %d)",_FL,sizeof(*ar),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	if (packet_type==DATATYPE_ACCOUNT_RECORD) {	// destined for admin editing?

	  #if ADMIN_CLIENT

	   #if INCL_EMAIL_INVALIDATING	//20000210MB: include email invalidating code?

		// If we're trying to invalidate a ton of email addresses, check for

		// that here.

		if (*szUserIDToInvalidate && iInvalidateEmailAddresses &&

					!stricmp(ar->sdb.user_id, *szUserIDToInvalidate)) {

			// Mark email as invalid and re-submit

			if (!(ar->sdb.flags & SDBRECORD_FLAG_EMAIL_BOUNCES)) {

				ar->sdb.flags |= SDBRECORD_FLAG_EMAIL_BOUNCES;

				struct AccountRecord ar2;

				ar2 = *ar;

				ar2.usage = ACCOUNTRECORDUSAGE_MODIFY;

				kp(("%s(%d) Marked email invalid for %s\n", _FL, ar->sdb.user_id));

				SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar2, sizeof(ar2));

			}

			// move on to next user id in list

			szUserIDToInvalidate++;

			if (*szUserIDToInvalidate) {

				// Request the next one...

				struct AccountRecord ar2;

				zstruct(ar2);

				ar2.usage = ACCOUNTRECORDUSAGE_LOOKUP_USERID;

				strnncpy(ar2.sdb.user_id, *szUserIDToInvalidate, MAX_PLAYER_USERID_LEN);

				kp(("%s(%d) Auto-retrieving account for %s\n", _FL, *szUserIDToInvalidate));

				SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar2, sizeof(ar2));

			}

		}

	   #endif	// INCL_EMAIL_INVALIDATING



	  #if 0	//20000214MB

		// If this was one of the special accounts, keep track of some details.

		#define MIN_TIME_BETWEEN_SAVING_DATA	(8*60)

		time_t now = time(NULL);

		long min_time_diff = MIN_TIME_BETWEEN_SAVING_DATA;

		if (!RakeRate) {

			min_time_diff = 3*60;

		}

		if (!stricmp(ar->sdb.user_id, "Rake") && (now - RakeSaveTime > min_time_diff)) {

			int new_rake = ar->sdb.real_in_bank;

			if (RakeSaveAmount) {

				RakeRate = (new_rake - RakeSaveAmount) * 36 / (now - RakeSaveTime);

			}

			RakeSaveTime = now;

			RakeSaveAmount = new_rake;

		}

	  #endif



		// add it to the back-and-forth queue (zero is most recent)

	  #if ADMIN_CLIENT

		// sift up

		int already_in_list = FALSE;	// see if we have it already

		for (int i=0;i < MAX_REMEMBERED_ADMIN_PLAYERIDS; i++) {

			if (ar->sdb.player_id == AdminPlayerIDs[i]) {

				already_in_list = TRUE;

				break;

			}

		}

	   #if 1

		if (!already_in_list) {

			memcpy(&AdminPlayerIDs[1], &AdminPlayerIDs[0], sizeof(WORD32)*(MAX_REMEMBERED_ADMIN_PLAYERIDS-1));

			AdminPlayerIDs[0] =  ar->sdb.player_id;

			AdminPlayerIDIndex = 0; // set to latest

		}

	   #else

		// 20010116HK: changed as per discussions with MB.  Pulling up an old one should dupe it to the

		// front of the list... unles we ourselves have back-arrowed to it



	   #endif

	  #endif



	  	if (hTransferMoneyDlg) {	// we're transferring money... save elsewhere

			// Save as source or dest depending on what we wanted.

			// (if we're waiting for zero, accept the first to come along).

			if (ar->sdb.player_id==dwTransferMoneySourcePlayerID || !dwTransferMoneySourcePlayerID) {

				LatestXferSrcRecordFromServer = *ar;

				dwTransferMoneySourcePlayerID = ar->sdb.player_id;

			}

			if (ar->sdb.player_id==dwTransferMoneyDestPlayerID || !dwTransferMoneyDestPlayerID) {

				LatestXferDestRecordFromServer = *ar;

				dwTransferMoneyDestPlayerID = ar->sdb.player_id;

			}

			// Update the dialog box.

			PostMessage(hTransferMoneyDlg, WMP_NEW_ACCOUNT_RECORD, 0, 0);

	  	} else {

			LatestAccountRecordFromServer = *ar;

			if (hAdminEditAccountDlg) {

				PostMessage(hAdminEditAccountDlg, WMP_NEW_ACCOUNT_RECORD, 0, 0);

			}

			if (hViewTransactionsDLG) {

				ArToView = *ar;

				PostMessage(hViewTransactionsDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

			}

		  #if ADMIN_CLIENT

			if (hEditCreditableDLG) {

				PostMessage(hEditCreditableDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

			}

		  #endif

	  	}

	  #endif

	} else {	// it's for the client's current account

		LoggedInAccountRecord = *ar;
		
		// If the cashier screen is up... tell it to update the info.

		if (hCashierDlg) {

			PostMessage(hCashierDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		}

		if (hViewTransactionsDLG) {

			ArToView = *ar;

			PostMessage(hViewTransactionsDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

		}

	  #if ADMIN_CLIENT

		if (hEditCreditableDLG) {

			PostMessage(hEditCreditableDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

		}

	  #endif



		//20000207MB: If our email address is flagged as 'invalid' and this is a

		// real money account, make sure they get a notice message at least once.

		if ((LoggedInAccountRecord.sdb.flags & (SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES)) &&

			LoggedInPrivLevel >= ACCPRIV_REAL_MONEY &&

			!iDisplayedBadEmailNotice)

		{

			iDisplayedBadEmailNotice = TRUE;	// never display it twice a session

			PostMessage(hInitialWindow, WMP_DISPLAY_BAD_EMAIL_NOTICE, 0, 0);

		}

	}

	return ERR_NONE;

}

//*********************************************************

// 2000/02/01 - MB

//

// Mesage handler for the 'change email' dialog window

//

BOOL CALLBACK dlgFuncChangeEmail(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(GetDlgItem(hDlg, IDC_EMAIL), EM_LIMITTEXT, MAX_EMAIL_ADDRESS_LEN-1, 0L);

		SetWindowText(GetDlgItem(hDlg, IDC_USERID), LoginUserID);

		SetWindowText(GetDlgItem(hDlg, IDC_EMAIL), LoggedInAccountRecord.sdb.email_address);

		CheckDlgButton(hDlg, IDC_CHECK2, TRUE);	// always default to checked.

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_COMMAND:

		{

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				EndDialog(hDlg, LOWORD(wParam));

				return TRUE;	// We DID process this message

			case IDC_SUBMIT:

				{

					// retrieve the email they entered and make sure it looks

					// like an email address.

					char email[MAX_EMAIL_ADDRESS_LEN];

					zstruct(email);

					GetWindowText(GetDlgItem(hDlg, IDC_EMAIL), email, MAX_EMAIL_ADDRESS_LEN-1);



					if (ValidateEmail(hDlg, email)) {

						// Looks valid so far... submit it to the server...

						struct AccountRecord ar;

						zstruct(ar);

						ar.usage = ACCOUNTRECORDUSAGE_CHANGE_EMAIL;

						strnncpy(ar.sdb.email_address, email, MAX_EMAIL_ADDRESS_LEN);

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						EndDialog(hDlg, IDOK);

						MessageBox(hDlg, "Your new email address has been sent to the server.\n\n"

										 "Your validation code will be emailed to you and it\n"

										 "should arrive in a few minutes.", "Change email address...", MB_OK);

					}

				}

				return TRUE;	// We DID process this message

			case IDC_VALIDATE:

				{

					// retrieve the email they entered and make sure it's

					// validation code matches.

					char email[MAX_EMAIL_ADDRESS_LEN];

					zstruct(email);

					GetWindowText(GetDlgItem(hDlg, IDC_EMAIL), email, MAX_EMAIL_ADDRESS_LEN-1);

					char validation_str[MAX_COMMON_STRING_LEN];

					zstruct(validation_str);

					GetWindowText(GetDlgItem(hDlg, IDC_VALIDATION_CODE), validation_str, MAX_COMMON_STRING_LEN-1);

					int validation_code1 = atoi(validation_str);

					int validation_code2 = CalcEmailValidationCode(email);

					if (validation_code1 != validation_code2) {

						// didn't match...

						MessageBox(hDlg,"The validation code you entered does not match\n"

										"your email address.  Please make sure you enter\n"

										"the validation code that was emailed to you when\n"

										"you submitted your email address.\n\n"

										"If you continue to have difficulties, please email\n"

										"support@https://github.com/kriskoinfor assistance.",

										"Validation code does not match...", MB_OK);

					} else {

						// Looks valid so far... submit it to the server...

						struct AccountRecord ar;

						zstruct(ar);

						ar.usage = ACCOUNTRECORDUSAGE_SUBMIT_EMAIL_VALIDATION;

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						EndDialog(hDlg, IDOK);

						MessageBox(hDlg,"Your email address has been validated.",

										"Email validated", MB_OK);

					}

				}

				return TRUE;	// We DID process this message

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



//*********************************************************

// 1999/11/24 - MB

//

// Mesage handler for the 'change password' dialog window

//

BOOL CALLBACK dlgFuncChangePassword(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(GetDlgItem(hDlg, IDC_SECURE_PHRASE), EM_LIMITTEXT, MAX_PLAYER_SECURE_PHRASE_LEN-1, 0L); //Fonseca
		
		//SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), EM_LIMITTEXT, MAX_PLAYER_PASSWORD_LEN-1, 0L);

		SendMessage(GetDlgItem(hDlg, IDC_PASSWORD2), EM_LIMITTEXT, MAX_PLAYER_PASSWORD_LEN-1, 0L);

		SendMessage(GetDlgItem(hDlg, IDC_PASSWORD3), EM_LIMITTEXT, MAX_PLAYER_PASSWORD_LEN-1, 0L);

		SetWindowText(GetDlgItem(hDlg, IDC_USERID), LoginUserID);

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_COMMAND:

		{

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				EndDialog(hDlg, LOWORD(wParam));

				return TRUE;	// We DID process this message

			case IDOK:

				{

					// retrieve the passwords they entered and make sure they match

					//char pwd1[MAX_PLAYER_PASSWORD_LEN];

					char pwd2[MAX_PLAYER_PASSWORD_LEN];

					char pwd3[MAX_PLAYER_PASSWORD_LEN];

					char phrase[MAX_PLAYER_SECURE_PHRASE_LEN];
					
					//zstruct(pwd1);

					zstruct(pwd2);

					zstruct(pwd3);

					zstruct(phrase); //Fonseca

					//GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), pwd1, MAX_PLAYER_PASSWORD_LEN-1);

					GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD2), pwd2, MAX_PLAYER_PASSWORD_LEN-1);

					GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD3), pwd3, MAX_PLAYER_PASSWORD_LEN-1);

					GetWindowText(GetDlgItem(hDlg, IDC_SECURE_PHRASE), phrase, MAX_PLAYER_SECURE_PHRASE_LEN-1); //Fonseca

		
/*
					if (strcmp(phrase, LoggedInAccountRecord.sdb.secure_phrase)) {  //Fonseca

						MessageBox(hDlg, "Secure Phrase is incorrect.", "Can't change password", MB_OK|MB_ICONHAND|MB_APPLMODAL|MB_TOPMOST);

						return TRUE;

					}
*/					

					/*if (strcmp(pwd1, LoginPassword)) {

						MessageBox(hDlg, "Current password is incorrect.", "Can't change password", MB_OK|MB_ICONHAND|MB_APPLMODAL|MB_TOPMOST);

						return TRUE;

					}*/

					if (!ValidateString(pwd2, 5, MAX_PLAYER_PASSWORD_LEN, "new password")) {

						MessageBox(hDlg, "Your new password be at least 5 characters long.", "Can't change password", MB_OK|MB_ICONHAND|MB_APPLMODAL|MB_TOPMOST);
						return TRUE;

					}

					if (strcmp(pwd2, pwd3)) {

						MessageBox(hDlg, "Your new passwords do not match.", "Can't change password", MB_OK|MB_ICONHAND|MB_APPLMODAL|MB_TOPMOST);

						return TRUE;

					}

					// Submit to server...

					struct AccountRecord ar;

					zstruct(ar);

					ar.usage = ACCOUNTRECORDUSAGE_CHANGE_PASSWORD;

					strnncpy(ar.sdb.password, pwd2, MAX_PLAYER_PASSWORD_LEN);

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));



					strnncpy(LoginPassword, pwd2, MAX_PLAYER_PASSWORD_LEN);

					Defaults.password[0] = 0;	// forget what we had

					Defaults.changed_flag = TRUE;

					EndDialog(hDlg, LOWORD(wParam));

					MessageBox(hDlg, "Your new password has been sent to the server.\n\n"

									 "It should be working the next time you log in.", "Finished.", MB_OK);

				}

				return TRUE;	// We DID process this message

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



#if ADMIN_CLIENT

//****************************************************************

// 1999/08/05 - MB

//

// Mesage handler for 'Block Computer' window

//

BOOL CALLBACK dlgFuncBlockComputer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	//kp(("%s(%d) dlg $%08lx got message = %d\n",_FL, hDlg, message));

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		{

			char str[200];

			sprintf(str, "%d", dwComputerSerialNumToBlock);

			SetDlgItemTextIfNecessary(hDlg, IDC_SERIAL_NUM, str);

			// Set default radio button

			CheckDlgButton(hDlg, IDC_RADIO_UNBLOCK, FALSE);

			CheckDlgButton(hDlg, IDC_RADIO_BLOCK, TRUE);

		}

		return TRUE;	// TRUE = set keyboard focus appropriately

	case WM_COMMAND:

		//kp(("%s(%d) dialog $%08lx got WM_COMMAND\n", _FL, hDlg));

		{

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {

			case IDOK:

				struct MiscClientMessage mcm;

				zstruct(mcm);

				mcm.message_type = MISC_MESSAGE_SET_COMPUTER_SERIAL_NUM_BLOCK;

				mcm.misc_data_1 = GetDlgTextInt(hDlg, IDC_SERIAL_NUM);

				mcm.misc_data_2 = IsDlgButtonChecked(hDlg, IDC_RADIO_BLOCK);	// block/unblock flag

				if (mcm.misc_data_1) {	// only send if non-zero

					SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				}

				EndDialog(hDlg, 1);

			    return TRUE;	// We DID process this message.

			case IDCANCEL:

				//kp(("%s(%d) IDCANCEL received.\n",_FL));

				EndDialog(hDlg, 0);

			    return TRUE;	// We DID process this message.

			}

		}

		break;

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(hDlg);

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



static BYTE8 bCheckRunSelectFlags[MAX_PLAYERS_PER_CHECK_RUN];

static int iCheckRunButtonIDs[MAX_PLAYERS_PER_CHECK_RUN_PAGE] = {

	IDC_ISSUE_CHECK1,

	IDC_ISSUE_CHECK2,

	IDC_ISSUE_CHECK3,

	IDC_ISSUE_CHECK4,

	IDC_ISSUE_CHECK5,

	IDC_ISSUE_CHECK6,

	IDC_ISSUE_CHECK7,

	IDC_ISSUE_CHECK8,

	IDC_ISSUE_CHECK9,

	IDC_ISSUE_CHECK10,

	IDC_ISSUE_CHECK11,

	IDC_ISSUE_CHECK12,

	IDC_ISSUE_CHECK13,

	IDC_ISSUE_CHECK14,

	IDC_ISSUE_CHECK15,

	IDC_ISSUE_CHECK16,

	IDC_ISSUE_CHECK17,

	IDC_ISSUE_CHECK18,

	IDC_ISSUE_CHECK19,

	IDC_ISSUE_CHECK20,

	IDC_ISSUE_CHECK21,

	IDC_ISSUE_CHECK22,

	IDC_ISSUE_CHECK23,

	IDC_ISSUE_CHECK24,

	IDC_ISSUE_CHECK25,

	IDC_ISSUE_CHECK26,

	IDC_ISSUE_CHECK27,

	IDC_ISSUE_CHECK28,

	IDC_ISSUE_CHECK29,

	IDC_ISSUE_CHECK30,

	IDC_ISSUE_CHECK31,

	IDC_ISSUE_CHECK32,

	IDC_ISSUE_CHECK33,

	IDC_ISSUE_CHECK34,

	IDC_ISSUE_CHECK35,

	IDC_ISSUE_CHECK36,

	IDC_ISSUE_CHECK37,

	IDC_ISSUE_CHECK38,

	IDC_ISSUE_CHECK39,

	IDC_ISSUE_CHECK40,

	IDC_ISSUE_CHECK41,

	IDC_ISSUE_CHECK42,

	IDC_ISSUE_CHECK43,

	IDC_ISSUE_CHECK44,

	IDC_ISSUE_CHECK45,

	IDC_ISSUE_CHECK46,

	IDC_ISSUE_CHECK47,

	IDC_ISSUE_CHECK48,

	IDC_ISSUE_CHECK49,

	IDC_ISSUE_CHECK50,

	IDC_ISSUE_CHECK51,

	IDC_ISSUE_CHECK52,

	IDC_ISSUE_CHECK53,

	IDC_ISSUE_CHECK54,

	IDC_ISSUE_CHECK55,

	IDC_ISSUE_CHECK56,

	IDC_ISSUE_CHECK57,

	IDC_ISSUE_CHECK58,

	IDC_ISSUE_CHECK59,

	IDC_ISSUE_CHECK60,

	IDC_ISSUE_CHECK61,

	IDC_ISSUE_CHECK62,

	IDC_ISSUE_CHECK63,

	IDC_ISSUE_CHECK64,

	IDC_ISSUE_CHECK65,

	IDC_ISSUE_CHECK66,

	IDC_ISSUE_CHECK67,

	IDC_ISSUE_CHECK68,

	IDC_ISSUE_CHECK69,

	IDC_ISSUE_CHECK70,

	IDC_ISSUE_CHECK71,

	IDC_ISSUE_CHECK72,

	IDC_ISSUE_CHECK73,

	IDC_ISSUE_CHECK74,

	IDC_ISSUE_CHECK75,

	IDC_ISSUE_CHECK76,

	IDC_ISSUE_CHECK77,

	IDC_ISSUE_CHECK78,

	IDC_ISSUE_CHECK79,

	IDC_ISSUE_CHECK80,

	IDC_ISSUE_CHECK81,

	IDC_ISSUE_CHECK82,

	IDC_ISSUE_CHECK83,

	IDC_ISSUE_CHECK84,

	IDC_ISSUE_CHECK85,

	IDC_ISSUE_CHECK86,

	IDC_ISSUE_CHECK87,

	IDC_ISSUE_CHECK88,

	IDC_ISSUE_CHECK89,

	IDC_ISSUE_CHECK90,

};



//*********************************************************

// 1999/09/28 - MB

//

// Set the state on a dialog checkbox if necessary

//

static void CheckDlgButtonIfNecessary(HWND hwnd, int control_id, int new_checkbox_state)

{

	int old_state = IsDlgButtonChecked(hwnd, control_id);

	if ((new_checkbox_state && !old_state) || (!new_checkbox_state && old_state)) {

		// change it.

		CheckDlgButton(hwnd, control_id, new_checkbox_state);

	}

}



//*********************************************************

// 1999/12/28 - MB

//

// Fill in the text and show/hide the checkboxes for the

// check run dialog box

//

static void UpdateCheckRunDialog(HWND hDlg)

{

	char str[300];

	zstruct(str);

	char curr_string[MAX_CURRENCY_STRING_LEN];

	int total = 0;

	int check_count = 0;

	int index_offset = (CheckRunPageNumber-1)*90;

	for (int i=0 ; i<MAX_PLAYERS_PER_CHECK_RUN_PAGE ; i++) {

		if (AdminCheckRun.entries[i+index_offset].player_id) {

			CheckDlgButtonIfNecessary(hDlg, iCheckRunButtonIDs[i], bCheckRunSelectFlags[i+index_offset]);

			sprintf(str, "%9s  %s",

					CurrencyString(curr_string, AdminCheckRun.entries[i+index_offset].amount, CT_REAL, TRUE),

					AdminCheckRun.entries[i+index_offset].description);

			SetDlgItemTextIfNecessary(hDlg, iCheckRunButtonIDs[i], str);

			ShowWindowIfNecessary(GetDlgItem(hDlg, iCheckRunButtonIDs[i]), SW_SHOW);

			if (bCheckRunSelectFlags[i+index_offset]) {

				total += AdminCheckRun.entries[i+index_offset].amount;

				check_count++;

			}

		} else {

			ShowWindowIfNecessary(GetDlgItem(hDlg, iCheckRunButtonIDs[i]), SW_HIDE);

		}

	}

	sprintf(str, "Page %d:  %d checks, total %s", 

		CheckRunPageNumber, check_count, CurrencyString(curr_string, total, CT_REAL));

	SetDlgItemTextIfNecessary(hDlg, IDC_GRAND_TOTAL, str);

}



//*********************************************************

// 1999/12/07 - MB

//

// Mesage handler for the 'admin check run' dialog window

//

HWND hCheckRun;

BOOL CALLBACK dlgFuncAdminCheckRun(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			hCheckRun = hDlg;

			memset(bCheckRunSelectFlags, TRUE, sizeof(bCheckRunSelectFlags));	// default all checkboxes to true

			AddKeyboardTranslateHwnd(hDlg);

			WinPosWindowOnScreen(hDlg);

			CheckDlgButton(hDlg, IDC_RADIO_CHECKRUN_PAGE1,  TRUE);

			CheckRunPageNumber = 1;

			UpdateCheckRunDialog(hDlg);

			// set the font

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

				"Courier New" //  LPCTSTR lpszFace         // pointer to typeface name string

			);

			for (int index = 0 ; index < MAX_PLAYERS_PER_CHECK_RUN_PAGE ; index++) {

				HWND hCheckTxt = GetDlgItem(hDlg, iCheckRunButtonIDs[index]);

				if (hCheckTxt && font) {

					SendMessage(hCheckTxt, WM_SETFONT, (WPARAM)font, 0);

				}

			}

			ShowWindow(hDlg, SW_SHOW);

		}

		return FALSE;

	

	case WM_CONTEXTMENU: // pop up edit account info on a right-click

		{

			for (int index = 0 ; index < MAX_PLAYERS_PER_CHECK_RUN_PAGE ; index++) {

				if ((HWND)wParam == GetDlgItem(hDlg, iCheckRunButtonIDs[index])) {

					int index_offset = (CheckRunPageNumber-1)*90;

					AutoLookupPlayerID = AdminCheckRun.entries[index+index_offset].player_id;

					break;

				}

			}

			if (AutoLookupPlayerID) {

				PostMessage(hCardRoomDlg, WM_COMMAND, (WPARAM)IDM_EDIT_USER_ACCOUNTS, (LPARAM)0);

			}

		}

		break;



	case WM_COMMAND:

		{

			int button_id = LOWORD(wParam);

			for (int index = 0 ; index < MAX_PLAYERS_PER_CHECK_RUN_PAGE ; index++) {

				if (button_id == iCheckRunButtonIDs[index]) {

					int index_offset = (CheckRunPageNumber-1)*90;

					bCheckRunSelectFlags[index+index_offset] = (BYTE8)(IsDlgButtonChecked(hDlg, button_id) ? TRUE : FALSE);

					UpdateCheckRunDialog(hDlg);

					return TRUE;	// We DID process this message

				}

			}

			switch (LOWORD(wParam)) {



			case IDC_RADIO_CHECKRUN_PAGE1:

			case IDC_RADIO_CHECKRUN_PAGE2:

			case IDC_RADIO_CHECKRUN_PAGE3:

			case IDC_RADIO_CHECKRUN_PAGE4:

				CheckRunPageNumber = (LOWORD(wParam)-IDC_RADIO_CHECKRUN_PAGE1)+1;

				UpdateCheckRunDialog(hDlg);

				return TRUE;	// We DID process this message



			case IDOK:

				// Blank out the entries we don't want to issue and send the

				// resulting structure back to the server.

				if (MessageBox(hDlg,

						"Are you sure you want to issue these checks now?",

						"Last chance to bail out...",

						MB_TOPMOST|MB_YESNO)==IDYES)

				{

					for (int i=0 ; i<MAX_PLAYERS_PER_CHECK_RUN ; i++) {

						if (!bCheckRunSelectFlags[i]) {

							zstruct(AdminCheckRun.entries[i]);

						}

					}

					SendDataStructure(DATATYPE_ADMIN_CHECK_RUN, &AdminCheckRun, sizeof(AdminCheckRun));

					DestroyWindow(hDlg); // BAD WAVELAN

				}

				return TRUE;	// We DID process this message

			case IDCANCEL:

				DestroyWindow(hDlg);

				return TRUE;	// We DID process this message

			case IDC_SELECT_ALL:

				memset(bCheckRunSelectFlags, TRUE, sizeof(bCheckRunSelectFlags));	// default all checkboxes to true

				UpdateCheckRunDialog(hDlg);

				return TRUE;	// We DID process this message

			case IDC_UNSELECT_ALL:

				memset(bCheckRunSelectFlags, FALSE, sizeof(bCheckRunSelectFlags));	// default all checkboxes to true

				UpdateCheckRunDialog(hDlg);

				return TRUE;	// We DID process this message

			case IDC_INVERT_ALL:

				{

					for (int i=0 ; i<MAX_PLAYERS_PER_CHECK_RUN_PAGE ; i++) {

						bCheckRunSelectFlags[i] ^= TRUE;

					}

				}

				UpdateCheckRunDialog(hDlg);

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_DESTROY:

		hCheckRun = NULL;

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



//*********************************************************

// 2000/08/19 - MB

//

// Adjust a single dialog window item to essentially fill

// the entire client area of the parent window.

//

void AdjustDlgItemToFillWindow(HWND parent_hwnd, int dlg_item_id)

{

	RECT r;

	zstruct(r);

	GetClientRect(parent_hwnd, &r);

	int w = r.right  - r.left - 1;

	int h = r.bottom - r.top  - 0;

	SetWindowPos(GetDlgItem(parent_hwnd, dlg_item_id),

			0, 0, 0, w, h, 

			SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);

}



//*********************************************************

// 2000/09/05 - MB

//

// Convert a WCHAR string to a regular CHAR string.  Throw

// out any characters that don't make sense.

//

void WideStrToAsciiStr(WCHAR *src, char *dest, int max_dest_len)

{

	while (*src && max_dest_len > 1) {

		if (*src < 256) {

			*dest++ = (char)*src;

			max_dest_len--;

		}

		src++;

	}

	*dest = 0;

}



//*********************************************************

// 2000/09/05 - MB

//

// Guess the short time zone name (e.g. PDT) based on the long

// one we get back from GetTimeZoneInformation (e.g. Pacific Daylight Time)

// Dest string must be at least 4 characters.

//

void GuessShortTimeZoneName(char *src, char *dest)

{

	*dest++ = *src++;

	src = strchr(src, ' ');

	if (src) {

		src++;

		*dest++ = *src++;

		src = strchr(src, ' ');

		if (src) {

			src++;

			*dest++ = *src;

		}

	}

	*dest = 0;

}



//*********************************************************

// 1999/12/07 - MB

//

// Mesage handler for the 'admin stats' dialog window

//

BOOL CALLBACK dlgFuncAdminStats(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		hAdminStats = hDlg;

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(hDlg, WMP_UPDATE_YOURSELF, 0, 0);

		if (AutoShowServerStats==2) {	// restore to position 2?

			WinRestoreWindowPos(ProgramRegistryPrefix, "AdminStats2", hDlg, NULL, NULL, FALSE, FALSE);

		} else {

			WinRestoreWindowPos(ProgramRegistryPrefix, "AdminStats", hDlg, NULL, NULL, FALSE, FALSE);

		}

		WinPosWindowOnScreen(hDlg);

		// Resize the edit control to take up the whole window.

		AdjustDlgItemToFillWindow(hDlg, IDC_TEXT);



		ShowWindow(hDlg, SW_SHOW);

		SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_SETSEL, (WPARAM)-1, 0);	// remove selection

		SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));	// nobody should get keyboard focus

		return FALSE;

	case WM_COMMAND:

		{

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				DestroyWindow(hDlg);

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_DESTROY:

		if (AutoShowServerStats==2) {	// save to position 2?

			WinStoreWindowPos(ProgramRegistryPrefix, "AdminStats2", hDlg, NULL);

		} else {

			WinStoreWindowPos(ProgramRegistryPrefix, "AdminStats", hDlg, NULL);

		}

		hAdminStats = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		// Resize the edit control to take up the whole window.

		AdjustDlgItemToFillWindow(hDlg, IDC_TEXT);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WMP_UPDATE_YOURSELF:

		{

			// Save the current edit control selection

			DWORD saved_sel_start = 0;

			DWORD saved_sel_end = 0;

			SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_GETSEL, (WPARAM)&saved_sel_start, (LPARAM)&saved_sel_end);

			int first_line = SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_GETFIRSTVISIBLELINE, 0, 0);



			char str[3000];

			zstruct(str);

		  #if 0	//19991231MB

			//----------

			static int email_sent = FALSE;

			if (AdminStats.next_game_number >= 1000000 && !email_sent) {

				email_sent = TRUE;

				sprintf(str, "Now at game #%d!", AdminStats.next_game_number);

				Email("6047884130@message.bctm.com",

						"admin", "bob@avanira.com", str, "c:\\temp\\nomsg");

			}

			//----------

		  #endif

			if (AdminStats.time) {

				if (AdminStats.rake_per_hour < 0) {

					AdminStats.rake_per_hour = 0;

				}

				int gross_cc = 0;

				if (SavedAccountInfo.cc_purchase_fee_rate) {

					gross_cc = (int)(

							(double)(AdminStats.ecashfee_today) / (SavedAccountInfo.cc_purchase_fee_rate / 10000.0) + .5);

				}



				// !!!! TEMPORARY: log new data for making predictions.

				struct Pred_Data normal_pd, peak_pd;

				zstruct(normal_pd);

				zstruct(peak_pd);

				int peak_players_interval = 0;

				int peak_players = 0;	// estimated peak players (if predicted)

				int min_players_interval = 0;

				int seconds_into_interval = 0;

				int profile_rake_estimate = 0;

				int current_interval = 0;

				float min_players = 0.0;

				if (AdminStats.server_uptime >= 60*60) {

					Pred_LogNewStats(AdminStats.time,

									 AdminStats.active_player_connections,

									 AdminStats.rake_per_hour,

									 AdminStats.rake_today,

									 AdminStats.games_today,

									 gross_cc,

									 AdminStats.number_of_accounts_today,

									 &seconds_into_interval,

									 &current_interval);

					Pred_GetPredictionInfo( &normal_pd,

											&peak_pd,

											&peak_players_interval,

											&min_players,

											&min_players_interval,

											seconds_into_interval);

				}

				int seconds_into_day = current_interval * PREDICTION_DATA_INTERVAL;

				int seconds_left_today = 24*3600 - seconds_into_day;



				// !!! end prediction related stuff



				char curr1[MAX_CURRENCY_STRING_LEN];

				char curr2[MAX_CURRENCY_STRING_LEN];

				char curr3[MAX_CURRENCY_STRING_LEN];

				char curr6[MAX_CURRENCY_STRING_LEN];

				char curr7[MAX_CURRENCY_STRING_LEN];



				// Set title to include the time

				char title[100];

				sprintf(title, "Server stats   %s", TimeStr(AdminStats.time));

				SetWindowTextIfNecessary(hDlg, title);



				sprintf(str,

					"Players:    %4d low:%4d high:%4d\r\n"

					"Real tables:%4d low:%4d high:%4d\r\n"

					"Play tables:%4d low:%4d high:%4d\r\n"

					"Next game:%12s (%7s/hr)\r\n"

					,

					AdminStats.active_player_connections,

					AdminStats.player_count_low,

					AdminStats.player_count_high,

					AdminStats.real_tables,

					AdminStats.real_tables_low,

					AdminStats.real_tables_high,

					AdminStats.play_tables,

					AdminStats.play_tables_low,

					AdminStats.play_tables_high,

					IntegerWithCommas(curr6, AdminStats.next_game_number),

					IntegerWithCommas(curr7, AdminStats.games_per_hour)

				);



				int seated_players = AdminStats.active_player_connections -

									 AdminStats.unseated_players -

									 AdminStats.idle_players;

				seated_players = max(0,seated_players);

				sprintf(str+strlen(str),

					"Seated players:       %5d (%4.1f%%)\r\n"

					,

					seated_players,

					seated_players*100.0 / (float)max(1,AdminStats.active_player_connections)

				);



				sprintf(str+strlen(str),

					"Multi-seated players: %5d (%4.1f%%)\r\n"

					,

					AdminStats.multi_seated_players,

					AdminStats.multi_seated_players*100.0 / (float)max(1,seated_players)

				);



				sprintf(str+strlen(str),

					"Games/hour/seated player:%10.2f\r\n"

					,

					(float)((double)AdminStats.games_per_hour / (double)(max(1,seated_players)))

				);



			  #if 0	//20001106MB

				sprintf(str+strlen(str), "time left today = %ds\r\n", seconds_left_today);

			  #endif



			  #if 0	//20010207MB

				// 20000807MB: if within 50,000 games of another million,

				// display an ETA.

				#define GAMES_PER_DAY			250000	// wild guess (not normally used)

				#define MAX_GAMES_TO_GO			((GAMES_PER_DAY*2)/3)

				#define GAME_ESTIMATE_INTERVAL	500000

				int game_num = AdminStats.next_game_number + GAME_ESTIMATE_INTERVAL - 1;

				game_num -= game_num % GAME_ESTIMATE_INTERVAL;

				int games_left = game_num - AdminStats.next_game_number;

				char game_estimate_str[200];

				zstruct(game_estimate_str);

				if (games_left <= MAX_GAMES_TO_GO) {

					double games_per_hour = GAMES_PER_DAY/24;

					if (AdminStats.games_per_hour) {

						games_per_hour = AdminStats.games_per_hour;

					}

					int seconds_left = (int)((double)games_left / (games_per_hour / 3600.0));

					time_t finish_time = time(NULL) + seconds_left;

					sprintf(str+strlen(str),

							"Game #%10s:    %s\r\n",

							IntegerWithCommas(curr1, game_num),

							TimeStr(finish_time));



					sprintf(str+strlen(str),

							"      %10s to go.%3dh %2dm %2ds\r\n",

							IntegerWithCommas(curr1, games_left),

							seconds_left / 3600,

							(seconds_left / 60) % 60,

							seconds_left % 60);



					sprintf(game_estimate_str,

							"#%d@%-5.5s<br/>\n", game_num, TimeStr(finish_time)+6);

				}

			  #endif



			  #if 0	//20000904MB

				// 20000602HK: log players online and hands per hour

				static time_t last_log_upate_time;

				time_t time_now = time(NULL);

				if (difftime(time_now, last_log_upate_time) >= 4*60+40) {	// every n minutes

					last_log_upate_time = time_now;

					char log_line[100];

					zstruct(log_line);

					AddToLog("users.log",

						"Time\t"

						"Users\t"

						"Seated\t"

						"Seated\t"

						"Double-Seated\t"

						"Double-Seated\t"

						"Games/Hour\t"

						"Games/Hr/Seated\t"

						"Games/Hr/Seat\n",

						"%s\t"

						"%d\t"

						"%d\t"

						"%.1f%%\t"

						"%d\t"

						"%.1f%%\t"

						"%d\t"

						"%.2f\t"

						"%.2f\n",

						TimeStr(AdminStats.time), 

						AdminStats.active_player_connections,

						seated_players,

						seated_players*100.0 / (float)max(1,AdminStats.active_player_connections),

						AdminStats.multi_seated_players,

						AdminStats.multi_seated_players*100.0 / (float)max(1,seated_players),

						AdminStats.games_per_hour,

						(float)((double)AdminStats.games_per_hour / (double)(max(1,seated_players))),

						(float)((double)AdminStats.games_per_hour / (double)(max(1,seated_players+AdminStats.multi_seated_players)))

					);

				}

			  #endif

			  

			  #if 0	//20000526MB

				// Estimate when the next 100,000th game will be played.

				#define GAMES_PER_DAY	88000

				int game_num = AdminStats.next_game_number + 99999;

				game_num -= game_num % 100000;

				int games_left = game_num - AdminStats.next_game_number;

				double games_per_hour = GAMES_PER_DAY/24;

				if (AdminStats.games_per_hour) {

					games_per_hour = AdminStats.games_per_hour;

				}

				int seconds_left = (int)((double)games_left / (games_per_hour / 3600.0));

				time_t finish_time = time(NULL) + seconds_left;

				sprintf(str+strlen(str),

						"Game #%10s:    %s\r\n",

						IntegerWithCommas(curr1, game_num),

						TimeStr(finish_time));



				sprintf(str+strlen(str),

						"      %10s to go.%3dh %2dm %2ds\r\n",

						IntegerWithCommas(curr1, games_left),

						seconds_left / 3600,

						(seconds_left / 60) % 60,

						seconds_left % 60);



				// Estimate when the next 1,000,000th game will be played

				game_num = AdminStats.next_game_number + 999999;

				game_num -= game_num % 1000000;

				games_left = game_num - AdminStats.next_game_number;

				games_per_hour = GAMES_PER_DAY/24;

				seconds_left = (int)((double)games_left / (games_per_hour / 3600.0));

				finish_time = time(NULL) + seconds_left;

				sprintf(str+strlen(str),

						"Game #%10s:    %s\r\n",

						IntegerWithCommas(curr1, game_num),

						TimeStr(finish_time));

			  #endif



				// Do we get the privileged stats as well?

				if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

					sprintf(str+strlen(str),

							"Ecash today:%10s (%3d / %3ds)\r\n"

						,

						CurrencyString(curr3, AdminStats.ecash_today, CT_REAL, 0, 1),

						AdminStats.ecash_queue_len,

						AdminStats.ecash_post_time);

				} else {

					sprintf(str+strlen(str),

							"Ecash queue: %d items, %ds avg.\r\n"

						,

						AdminStats.ecash_queue_len,

						AdminStats.ecash_post_time);

				}



				int threads = AdminStats.ecash_threads ? AdminStats.ecash_threads : 4;

				int queue_minutes = ((AdminStats.ecash_queue_len / threads + 1) * AdminStats.ecash_post_time + 59) / 60;

				if (queue_minutes >= 4) {

					sprintf(str+strlen(str),

							"*** ECASH QUEUE TIME: %d minutes\r\n"

							,

							queue_minutes);

				}

				if (AdminStats.shotclock_flags & SCUF_CLOSE_CASHIER) {

					sprintf(str+strlen(str),

							"*** CASHIER IS CLOSED ***\r\n");

				}

				if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

					int gross_cc = 0;

					if (SavedAccountInfo.cc_purchase_fee_rate) {

						gross_cc = (int)(

								(double)(AdminStats.ecashfee_today) / (SavedAccountInfo.cc_purchase_fee_rate / 10000.0) + .5);

					}



				  #if 0	//20000905MB

					sprintf(str+strlen(str),

							"Gross ecash:%10s (today)\r\n"

						,

						CurrencyString(curr4, gross_cc, CT_REAL, 0, 1)

					);

				  #endif





					sprintf(str+strlen(str),

							"     ---- daily estimates ----\r\n");



				  #if 1	//20000819MB: prediction testing

					// Calculate our time zone information in seconds past UTC.

					TIME_ZONE_INFORMATION tzi;

					zstruct(tzi);

					DWORD tzresult = GetTimeZoneInformation(&tzi);

					// hack out the time zone name.  It's wide char, but I couldn't find

					// any easy conversion routines from wide to ascii (even something that

					// throws out unknown characters would be useful).

					pr(("%s(%d) tzi: %d, %d: %s, %d: %s\n",

							_FL,

							tzi.Bias,

							tzi.StandardBias, tzi.StandardName,

							tzi.DaylightBias, tzi.DaylightName));

					char szLocalTimeZoneName[32];

					zstruct(szLocalTimeZoneName);

					int iLocalTimeZoneSeconds;	// current # of seconds between UTC and our current time zone (DST adjusted)

					iLocalTimeZoneSeconds = tzi.Bias*60;

					if (tzresult==TIME_ZONE_ID_DAYLIGHT) {

						iLocalTimeZoneSeconds += tzi.DaylightBias*60;

						WideStrToAsciiStr(tzi.DaylightName, szLocalTimeZoneName, sizeof(szLocalTimeZoneName));

						GuessShortTimeZoneName(szLocalTimeZoneName, szLocalTimeZoneName);

					} else {

						WideStrToAsciiStr(tzi.StandardName, szLocalTimeZoneName, sizeof(szLocalTimeZoneName));

						GuessShortTimeZoneName(szLocalTimeZoneName, szLocalTimeZoneName);

					}



					// Calculate the number of seconds difference in time zones between

					// us (the client displaying the information) and the server.

					int local_seconds_offset = SERVER_TIMEZONE - iLocalTimeZoneSeconds;



					if (peak_players_interval > current_interval && normal_pd.players != 0.0) {

						peak_players = (int)((AdminStats.active_player_connections / normal_pd.players) * peak_pd.players);

						int peak_players_seconds = (peak_players_interval * PREDICTION_DATA_INTERVAL + local_seconds_offset) % (24*3600);

						sprintf(str+strlen(str),

								"Peak players @ %02d:%02d %3.3s:%10d\r\n"

							,

							(peak_players_seconds) / 3600,

							((peak_players_seconds) / 60) % 60,

							szLocalTimeZoneName,

							peak_players

						);

					}

					if (min_players_interval > current_interval && normal_pd.players != 0.0) {

						int est_min_players = (int)((AdminStats.active_player_connections / normal_pd.players) * min_players);

						pr(("%s(%d) current_interval = %d, min_players_interval = %d\n",

									_FL, current_interval, min_players_interval));

						int min_players_seconds = (min_players_interval * PREDICTION_DATA_INTERVAL + local_seconds_offset) % (24*3600);

						sprintf(str+strlen(str),

								"Min. players @ %02d:%02d %s:%10d\r\n"

							,

							min_players_seconds / 3600,

							(min_players_seconds / 60) % 60,

							szLocalTimeZoneName,

							est_min_players

						);

					}



					int est_gross_cc = 0;

					int est_games = 0;

					int est_rake = 0;

					if (normal_pd.rake_today != 0.0) {

					  #if 0	//20010227MB: now done in the Pred_GetPredictionInfo() function.

						// Calculate normal rake for however far into the current

						// interval we are.

						double normal_rake = normal_pd.rake_today +

								(normal_pd.rake_per_hour * seconds_into_interval) / 3600.0;

						pr(("%s %s(%d) normal: today=%5.0f at %4.0f/hr, %3d extra seconds, normal estimate = %5.0f (for right now)\n",

								TimeStr(), _FL,

								normal_pd.rake_today / 100.0,

								normal_pd.rake_per_hour / 100.0,

								seconds_into_interval,

								normal_rake / 100.0));

						est_rake = (int)((AdminStats.rake_today / normal_rake) * peak_pd.rake_today);



						pr(("%s %s(%d) peak for today: %5.0f, now = %5.0f (%.3f times normal), estimate is now %5.0f\n",

								TimeStr(), _FL,

								peak_pd.rake_today / 100.0,

								AdminStats.rake_today / 100.0,

								AdminStats.rake_today / normal_rake,

								est_rake / 100.0));

					  #else

						est_rake = (int)((AdminStats.rake_today / normal_pd.rake_today) * peak_pd.rake_today);

					  #endif



					  #if 0

						static int old_seconds;

						static double old_diff;

						int elapsed = SecondCounter - old_seconds;

						if (elapsed <= 0) {

							elapsed = 1;

						}

						// calculate how much rake is left to be made today

						double diff = (est_rake - AdminStats.rake_today ) / 100.0;

						kp(("%s %s(%d) ------ estimate - current = %.0f - %.0f = %.0f (%.0f/hr) ------\n",

								TimeStr(), _FL,

								est_rake / 100.0, AdminStats.rake_today / 100.0,

								diff,

								(old_diff - diff) * 3600.0 / (float)elapsed));

						old_diff = diff;

						old_seconds = SecondCounter;

					  #endif



						profile_rake_estimate = est_rake;

					}



					int round_factor = 0;	// default to no rounding.

					if (seconds_left_today <= 20*60) {	// less than n minutes left?

						round_factor = 0;	// no rounding.

					} else if (seconds_left_today <= 1*3600) {	// less than n hours left?

						round_factor = 10000;	// round to nearest $100

					} else if (seconds_left_today <= 3*3600) {	// less than n hours left?

						round_factor = 25000;	// round to nearest $250

					} else if (seconds_left_today <= 6*3600) {	// less than n hours left?

						round_factor = 50000;	// round to nearest $500

					} else {

						round_factor = 100000;	// round to nearest $1000

					}



					est_gross_cc = 0;

					if (normal_pd.gross_cc_today != 0.0) {

						est_gross_cc = (int)((gross_cc / normal_pd.gross_cc_today) * peak_pd.gross_cc_today);

					}

					int est_ecash = AdminStats.est_ecash_for_today;



					int rounded_rake = est_rake;

					if (round_factor) {

						rounded_rake += round_factor/2 - 1;

						rounded_rake = rounded_rake - (rounded_rake % round_factor);

						est_gross_cc += round_factor/2 - 1;

						est_gross_cc = est_gross_cc - (est_gross_cc % round_factor);

						est_ecash += round_factor/2 - 1;

						est_ecash = est_ecash - (est_ecash % round_factor);

					}



					static int old_rounded_rake;

					static char rake_direction = ' ';



					if (!old_rounded_rake) {				// first time?

						old_rounded_rake = rounded_rake;	// initialize it.

						rake_direction = ' ';

					}



					// If it's within a decent margin of the old rake, just use the old

					// number rather than the new one (to prevent flipping between two

					// numbers when it's right at a rounding margin).

					if (round_factor) {

						int margin = round_factor * 3 / 4;

						if (abs(est_rake - old_rounded_rake) <= margin) {

							// It's close enough to the last estimate.  Just use

							// the last estimate.

							rounded_rake = old_rounded_rake;

						}

					}

					if (rounded_rake != old_rounded_rake) {

						if (rounded_rake > old_rounded_rake) {

							rake_direction = '^';

						} else {

							rake_direction = 'v';	// use 'v', I think down arrows are unicode :(

						}

						old_rounded_rake = rounded_rake;

					}



					sprintf(str+strlen(str),

							"Rake:%8s/hr  %8s/%8s%c\r\n"

						,

						CurrencyString(curr1, AdminStats.rake_per_hour, CT_REAL, 0, 1),

						CurrencyString(curr2, AdminStats.rake_today, CT_REAL, 0, 1),

						CurrencyString(curr3, rounded_rake, CT_REAL, 0, 1),

						rake_direction

					);



					sprintf(str+strlen(str),

							"Net ecash:        %8s/%8s\r\n"

						,

						CurrencyString(curr1, AdminStats.ecash_today, CT_REAL, 0, 1),

						CurrencyString(curr2, est_ecash, CT_REAL, 0, 1)

					);



					sprintf(str+strlen(str),

							"Gross ecash:      %8s/%8s\r\n"

						,

						CurrencyString(curr1, gross_cc, CT_REAL, 0, 1),

						CurrencyString(curr2, est_gross_cc, CT_REAL, 0, 1)

					);



					if (normal_pd.games_today != 0.0) {

						est_games = (int)((AdminStats.games_today / normal_pd.games_today) * peak_pd.games_today);

						int games_round_factor = round_factor / 100;

						if (games_round_factor) {

							est_games += games_round_factor/2 - 1;

							est_games = est_games - (est_games % games_round_factor);

						}



						sprintf(str+strlen(str),

								"Games played:     %8s/%8s\r\n"

							,

							IntegerWithCommas(curr1, AdminStats.games_today),

							IntegerWithCommas(curr2, est_games)

						);

					}



					if (normal_pd.new_accounts_today != 0.0 && (int)AdminStats.number_of_accounts_today >= 0) {

						int est_new_accounts = (int)((AdminStats.number_of_accounts_today / normal_pd.new_accounts_today) * peak_pd.new_accounts_today);

						sprintf(str+strlen(str),

								"New accounts:     %8d/%8d\r\n"

							,

							AdminStats.number_of_accounts_today,

							est_new_accounts

						);

					}	

				  #endif



				  #if 0	//20000920MB: old method of rake estimate

					{

						// Round rake estimate to nearest $100.

						est_rake = AdminStats.est_rake_for_today;

						rounded_rake = est_rake;

						if (round_factor) {

							rounded_rake += round_factor/2 - 1;

							rounded_rake = rounded_rake - (rounded_rake % round_factor);

						}

						static int old_rounded_rake2;

						static char rake_direction2 = ' ';

						if (!old_rounded_rake2) {				// first time?

							old_rounded_rake2 = rounded_rake;	// initialize it.

						}

						if (rounded_rake != old_rounded_rake2) {

							if (rounded_rake > old_rounded_rake2) {

								rake_direction2 = '^';

							} else {

								rake_direction2 = 'v';	// use 'v', I think down arrows are unicode :(

							}

							old_rounded_rake2 = rounded_rake;

						}



						sprintf(str+strlen(str),

								"Rake (old method):        %9s%c\r\n"

							,

							CurrencyString(curr5, rounded_rake, CT_REAL, 0, 1),

							rake_direction2

						);

					}

				  #endif

				}



				if (AdminStats.gross_bets_today) {

					sprintf(str+strlen(str),

						"Gross bets today:%18s\r\n",

						CurrencyString(curr1, AdminStats.gross_bets_today, CT_REAL, 0, 1));

				}



				if (AdminStats.gross_tournament_buyins_today) {

					sprintf(str+strlen(str),

						"Gross tournament buy-ins:%10s\r\n",

						CurrencyString(curr1, AdminStats.gross_tournament_buyins_today, CT_REAL, 0, 1));

				}



				sprintf(str+strlen(str),

					"Tournaments started today:%9d\r\n",

					AdminStats.tournaments_today);



				int days    =  AdminStats.server_uptime / (24*60*60);

				int hours   = (AdminStats.server_uptime % (24*60*60)) / (60*60);

				int minutes = (AdminStats.server_uptime % (60*60))    / (60);

				sprintf(str+strlen(str),

					"Srv uptime:%3dd %2dh %2dm %5.1f%% idle\r\n",

					days, hours, minutes,

					AdminStats.recent_idle_percentage);



				days    =  AdminStats.os_up_time / (24*60*60);

				hours   = (AdminStats.os_up_time % (24*60*60)) / (60*60);

				minutes = (AdminStats.os_up_time % (60*60))    / (60);

				sprintf(str+strlen(str),

						"OS  uptime:%3dd %2dh %2dm\r\n"

						"Load average: %.2f   %.2f   %.2f\r\n"

					,

					days, hours, minutes,

					AdminStats.load_avg[0]/100.0,

					AdminStats.load_avg[1]/100.0,

					AdminStats.load_avg[2]/100.0 

				);



				sprintf(str+strlen(str),

					"Bad beats today: %2d  paid:%9s\r\n"

					,

					AdminStats.bad_beats_today,

					CurrencyString(curr6, AdminStats.bad_beats_payout, CT_REAL, 0, 1)

				);



				sprintf(str+strlen(str),

					"Money in play:            %9s\r\n"

					"Money logged in:          %9s\r\n"

					,

					CurrencyString(curr1, AdminStats.money_in_play, CT_REAL, 0, 1),

					CurrencyString(curr2, AdminStats.money_logged_in, CT_REAL, 0, 1)

				);



			  #if 1	//20000913MB

				sprintf(str+strlen(str),

						"Mem: %sK %sK %sK\r\n",

					IntegerWithCommas(curr1, AdminStats.mem_used>>10),

					IntegerWithCommas(curr2, AdminStats.mem_allocated>>10),

					IntegerWithCommas(curr3, AdminStats.mem_not_used>>10)

				);

			  #endif



				sprintf(str+strlen(str),

						"Avg Hand History queue:       %4.0fs\r\n",

					AdminStats.hand_history_queue_time);



			  #if 0	//20000913MB

				char curr4[MAX_CURRENCY_STRING_LEN];

				char curr5[MAX_CURRENCY_STRING_LEN];

				sprintf(str+strlen(str),

						"Bytes sent: %7s/s (%.2f Mbit/s)\r\n"

						"Bytes rcvd: %7s/s (%.2f Mbit/s)\r\n",

					IntegerWithCommas(curr4, AdminStats.bytes_sent_per_second),

					AdminStats.bytes_sent_per_second * 8.0 / 1000000.0,

					IntegerWithCommas(curr5, AdminStats.bytes_rcvd_per_second),

					AdminStats.bytes_rcvd_per_second * 8.0 / 1000000.0

				);

			  #endif



			  #if 1	//20000913MB

				int i;

				for (i=0 ; i<2 ; i++) {

					sprintf(str+strlen(str),

							"Output queue %d depth:        %6d\r\n",

						i,

						AdminStats.output_queue_lens[i]);

				}

			  #endif



			  #if 0	//20000113MB: display packet sent/received stats?

				int i = 0;

				sprintf(str+strlen(str), "Stats for packets sent:\r\n");

				for (i=0 ; i<DATATYPE_COUNT ; i++) {

					if (AdminStats.packet_stats[i].sent_count) {

						double compression_amount = 0.0;

						if (AdminStats.packet_stats[i].bytes_sent_uncompressed) {

							compression_amount = 1.0 - 

									(double)AdminStats.packet_stats[i].bytes_sent / 

									(double)AdminStats.packet_stats[i].bytes_sent_uncompressed;

							pr(("%s(%d) type %2d: send = %5d, sent_uncompressed = %5d, compression_amount = %.3f\n",

									_FL, i,

									AdminStats.packet_stats[i].bytes_sent,

									AdminStats.packet_stats[i].bytes_sent_uncompressed,

									compression_amount));

						}

						sprintf(str+strlen(str),

								"Pkt %2d:%12s %9sK %3.0f%%\r\n",

								i,

								IntegerWithCommas(curr1, AdminStats.packet_stats[i].sent_count),

								IntegerWithCommas(curr2, AdminStats.packet_stats[i].bytes_sent>>10),

								compression_amount * 100.0

						);

					}

				}

				sprintf(str+strlen(str), "Stats for packets received:\r\n");

				for (i=0 ; i<DATATYPE_COUNT ; i++) {

					if (AdminStats.packet_stats[i].rcvd_count) {

						double compression_amount = 0.0;

						if (AdminStats.packet_stats[i].bytes_rcvd_uncompressed) {

							compression_amount = 1.0 - 

									(double)AdminStats.packet_stats[i].bytes_rcvd / 

									(double)AdminStats.packet_stats[i].bytes_rcvd_uncompressed;

							pr(("%s(%d) type %2d: rcvd = %5d, rcvd_uncompressed = %5d, compression_amount = %.3f\n",

									_FL, i,

									AdminStats.packet_stats[i].bytes_rcvd,

									AdminStats.packet_stats[i].bytes_rcvd_uncompressed,

									compression_amount));

						}

						sprintf(str+strlen(str),

								"Pkt %2d:%12s %9sK %3.0f%%\r\n",

								i,

								IntegerWithCommas(curr1, AdminStats.packet_stats[i].rcvd_count),

								IntegerWithCommas(curr2, AdminStats.packet_stats[i].bytes_rcvd>>10),

								compression_amount * 100.0

						);

					}

				}

			  #endif



			  #if 1	//20000113MB: display packet pool stats?

				for (i=0 ; i<STATS_PKTPOOL_COUNT ; i++) {

					sprintf(str+strlen(str),

						"Pool %d:%4d pk=%4d cnt=%11s\r\n",

						i,

						AdminStats.pool_stats[i].packet_count,

						AdminStats.pool_stats[i].max_pool_size,

						IntegerWithCommas(curr1, AdminStats.pool_stats[i].alloc_count));

				}

				sprintf(str+strlen(str),

						"Pkts allocd:%5s/%5s living:%5s\r\n",

						IntegerWithCommas(curr1, AdminStats.packets_allocated),

						IntegerWithCommas(curr2, AdminStats.packets_constructed),

						IntegerWithCommas(curr3, AdminStats.living_packets));

			  #endif

			  #if 0	//20000807MB

				// Estimate the number of packets that have either gotten

				// leaked (forgotten about) or are still in the send queues...

				int packet_count_for_pool = 0;

				for (i=0 ; i<STATS_PKTPOOL_COUNT ; i++) {

					packet_count_for_pool += AdminStats.pool_stats[i].packet_count;

				}

				sprintf(str+strlen(str),

						"Packets Living - InPool - InQueues:\r\n");

				sprintf(str+strlen(str),

						"   %d - %d - %d - %d = %d\r\n",

						AdminStats.living_packets,

						packet_count_for_pool,

						AdminStats.output_queue_lens[0],

						AdminStats.output_queue_lens[1],

						AdminStats.living_packets - packet_count_for_pool - AdminStats.output_queue_lens[0] - AdminStats.output_queue_lens[1]);

			  #endif



				//20010129MB: display average player response times

				sprintf(str+strlen(str),

						"Avg response time (real):%8.0fms\r\n"

						"Avg response time (play):%8.0fms\r\n"

						,

						AdminStats.avg_response_time_real_money*1000.0,

						AdminStats.avg_response_time_play_money*1000.0);



				//20010207MB: display average processing times for some components

				sprintf(str+strlen(str),

						"Avg main loop time:      %8.1fms\r\n"

						"Avg input loop time:     %8.1fms\r\n"

						"Avg accept processing:   %8.1fms\r\n"

						"Avg table processing:    %8.2fms\r\n"

						,

						AdminStats.mainloop_avg_ms,

						AdminStats.inputloop_avg_ms,

						AdminStats.accept_avg_ms,

						AdminStats.table_avg_ms);



				//20000727MB: display # of clients with what version

				int total = AdminStats.client_count_newer +

							AdminStats.client_count_current +

							AdminStats.client_count_old;

				double percentage = 0.0;

				if (total) {

					percentage = (double)AdminStats.client_count_old * 100.0 / (double)total;

				}

				sprintf(str+strlen(str),

						"Newer clients:  %5d\r\n"

						"Current clients:%5d\r\n"

						"Older clients:  %5d (%.1f%%)\r\n",

						AdminStats.client_count_newer,

						AdminStats.client_count_current,

						AdminStats.client_count_old,

						percentage);



				//20000728MB: display # of real money table names used

				sprintf(str+strlen(str),

						"RM Table Names: %5d of %d\r\n",

						AdminStats.table_names_used,

						AdminStats.table_names_avail);



				//20000731MB: display # of accounts

				sprintf(str+strlen(str),

						"Accounts: %d (%d purged, %d new)\r\n",

						AdminStats.number_of_accounts,

						AdminStats.number_of_accounts_purged,

						AdminStats.number_of_accounts_today);



				//20000604MB: if the rake account balance went through

				// another million boundary, play the cash register sound.

				if (LoggedInPrivLevel >= ACCPRIV_SUPER_USER ||

						!stricmp(LoggedInAccountRecord.sdb.user_id, "masher")) {

					static WORD32 old_rake;

					if (old_rake) {

					  #if 1	//20000604MB

						#define RAKE_SOUND_INTERVAL	1000000	// interval (in dollars)

					  #else

						#define RAKE_SOUND_INTERVAL	10	// interval (in dollars)

					  #endif

						WORD32 old_million = old_rake / (RAKE_SOUND_INTERVAL*100);

						WORD32 new_million = AdminStats.rake_balance / (RAKE_SOUND_INTERVAL*100);

						if (new_million > old_million) {

							PlaySound("CashReg.wav", NULL, SND_ASYNC | SND_NODEFAULT);

						}

					}

					old_rake = AdminStats.rake_balance;



					#define ONE_MILLION	(100000000)

					int next_million = (AdminStats.rake_balance + ONE_MILLION - 1) / ONE_MILLION;

					next_million *= ONE_MILLION;

					int left_to_next_million = next_million - AdminStats.rake_balance;

					if (AdminStats.rake_per_hour && left_to_next_million < 50000*100) {

						// We're getting close to the next million.  Try to estimate the time.

						double seconds_left = ((double)left_to_next_million * 3600.0) / (double)AdminStats.rake_per_hour;

						time_t target_time = time(NULL) + (int)seconds_left;

						struct tm tm;

						struct tm *t;

						t = localtime(&target_time, &tm);



						sprintf(str+strlen(str),

							"%s raked: %02d:%02d\r\n",

							CurrencyString(curr1, next_million, CT_REAL),

							t->tm_hour, t->tm_min);

					}

				}



				// If it's exactly 8:45pm and this is Mike's computer, send

				// out an email summarizing where we are to date.

				#define TEST_STAT_EMAIL	0

			   #if HORATIO

				{

			   #else

				char *computer = getenv("COMPUTERNAME");

				if (computer && !stricmp(computer, "Mike21")) {

			   #endif

				 #if 0	//send 8:45pm summary email?

					char curr4[MAX_CURRENCY_STRING_LEN];

					static time_t last_summary_email;

					if (!last_summary_email) {

						last_summary_email = now-20*60;	// always init to something.

					}

					struct tm tm;

					struct tm *t;

					t = localtime(&now, &tm);

				  #if 0	//20000614MB

					kp(("%s(%d) t->tm_hour = %d, min = %d\n", _FL, t->tm_hour, t->tm_min));

					kp(("%s(%d) now - last_summary_email = %d\n", _FL, now - last_summary_email));

				  #endif

					if (now - last_summary_email >= 6*60 && (

					  #if TEST_STAT_EMAIL

						(t->tm_hour == 8 &&

						t->tm_min >= 16 && t->tm_min <= 60) ||

					  #endif

					  #if 0	// 10:58pm email

						(t->tm_hour == 22 &&

						t->tm_min >= 58 && t->tm_min <= 60) ||

					  #endif

						(t->tm_hour == 20 &&

						t->tm_min >= 45 && t->tm_min <= 50)

						)

					) {

						last_summary_email = now;

						kp(("%s %s(%d) Sending 8:45 summary email now...\n", TimeStr(), _FL));

						// Time to send it out...

						char subject[200];

						zstruct(subject);

						sprintf(subject,

							"%d/%d %s",

							AdminStats.active_player_connections,

							AdminStats.player_count_high,

							CurrencyString(curr4, profile_rake_estimate,CT_PLAY, 0, 1)

						);



						EmailStr(

						  #if TEST_STAT_EMAIL

							"license@e-mediasoftware.com,"

							//"xxxxxxxxxx@message.bctm.com,"

							"license@e-mediasoftware.com",	// To:

						  #else

							"license@e-mediasoftware.com,"

							// To:

						  #endif

							"Stats",				// From:

							"license@e-mediasoftware.com",	// From address

							subject,				// Subject

							NULL,					// Bcc

							""						// message

						);

					}

				 #endif	//end of 'send 8:45pm summary email'



					// [ ] new accounts, games played

					

					// Write out a file to red which can be fetched over the internet

					// at any time.

				  #if HORATIO

					// FILE *fd = fopen("d:/dev/poker/pokersrv/failcount.wml", "wt");

				  #else

					// FILE *fd = fopen("r:/mike/misc/html/failcount.wml", "wt");

				  #endif

				  /* 

					if (fd) {

						// generate locally what we need for the wml file

						days    =  AdminStats.server_uptime / (24*60*60);

						hours   = (AdminStats.server_uptime % (24*60*60)) / (60*60);

						minutes = (AdminStats.server_uptime % (60*60))    / (60);

						char cashier_str[25];

						zstruct(cashier_str);

						if (AdminStats.shotclock_flags & SCUF_CLOSE_CASHIER) {

							sprintf(cashier_str,":CLSD");	// cashier closed?

						} else {

							sprintf(cashier_str, "%d/%ds",

								AdminStats.ecash_queue_len,

								AdminStats.ecash_post_time);

						}

						if (!normal_pd.games_today) {	// avoid blowup

							normal_pd.games_today = 1;

						}

						int est_games = (int)((AdminStats.games_today / normal_pd.games_today) * peak_pd.games_today);

						int est_new_accounts = (int)((AdminStats.number_of_accounts_today / normal_pd.new_accounts_today) * peak_pd.new_accounts_today);

						fprintf(fd,

							"<?xml version=\"1.0\"?>\n"

							"<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">\n"

							"<wml>\n"

							"  <head>\n"

							"    <meta http-equiv=\"Cache-Control\" content=\"max-age=0\"/>\n"

							"    <meta http-equiv=\"Cache-Control\" content=\"must-revalidate\" forua=\"true\"/>\n"

							"    <meta http-equiv=\"Cache-Control\" content=\"no-cache\" forua=\"true\"/>\n"

							"  </head>\n"

							"  <card>\n"

							"    <p>\n"

							// content goes here

							"      %s<br/>\n"						// 1

							"      %d%c%d %.1f<br/>\n"				// 2

						  #if 0	//20010207MB

							"%s"									// 2.5

						  #endif

							"      %dd %dh %.1f%%<br/>\n"			// 3

							"      %.2f %.2f %.2f<br/>\n"			// 4

							"      T%d/%d C%s<br/>\n"				// 5

							"      Gp%d/%d<br/>\n"					// 6

							"      Ac%d %d/%d<br/>\n"				// 7

							"      Tr%d %s<br/>\n"					// 8

							//

							"    </p>\n"

							"  </card>\n"

							"</wml>\n",

							// line 1

							TimeStr(),

							// line 2

							AdminStats.active_player_connections,

							peak_players ? ':' : '/',

							peak_players ? peak_players : AdminStats.player_count_high,

							profile_rake_estimate / 100000.0,

						  #if 0	//20010207MB

							// line 2.5

							game_estimate_str,

						  #endif

							// line 3

							days, 

							hours,

							AdminStats.recent_idle_percentage,

							// line 4

							AdminStats.load_avg[0]/100.0,

							AdminStats.load_avg[1]/100.0,

							AdminStats.load_avg[2]/100.0,

							// line 5

							AdminStats.real_tables,

							AdminStats.play_tables,

							cashier_str,

							// line 6

							AdminStats.games_today,

							est_games,

							// line 7

							AdminStats.number_of_accounts,

							AdminStats.number_of_accounts_today,

							est_new_accounts,

							// line 8

							AdminStats.tournaments_today,

							/// !!! Line below needs proper test to work, then set to Op and Cl

							(ShotClockFlags & SCUF_NO_TOURNAMENT_SITDOWN ? " " : " ")

						);

						fclose(fd);

					}  */

				}

			}

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT), str);

			// Restore selection

			SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_SETSEL, (WPARAM)saved_sel_start, (LPARAM)saved_sel_end);

			SendMessage(GetDlgItem(hDlg, IDC_TEXT), EM_LINESCROLL, 0, (LPARAM)first_line);

		}

		return TRUE;	// We DID process this message

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/12/07 - MB

//

// Open the admin stats window (if it's not already open)



//

void OpenAdminStatsWindow(void)

{

	if (!hAdminStats) {

		CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADMIN_STATS), NULL, dlgFuncAdminStats);

		if (hAdminStats) {

			PostMessage(GetDlgItem(hAdminStats, IDC_TEXT), EM_SETSEL, (WPARAM)-1, 0);	// remove selection

			SetFocus(GetDlgItem(hAdminStats, IDC_BUTTON1));	// nobody should get keyboard focus

		}

	} else {	// exists, top it

		ShowWindow(hAdminStats, SW_SHOWNORMAL);

		ReallySetForegroundWindow(hAdminStats);

	}

}



//*********************************************************

// 1999/11/11 - MB

//

// Mesage handler for administrator transfer money dialog window

//

BOOL CALLBACK dlgFuncTransferMoney(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char user_id[MAX_PLAYER_USERID_LEN];	// used for local retrieval

	switch (message) {

	case WM_INITDIALOG:

		hTransferMoneyDlg = hDlg;

		if (iTransferMoneyRealMoneyFlag) {

			SetWindowText(hDlg, "Transfer REAL MONEY between accounts");

		} else {

			SetWindowText(hDlg, "Play money transfer");

		}

		AddKeyboardTranslateHwnd(hDlg);

		SendMessage(GetDlgItem(hDlg, IDC_TRANSFER_REASON), EM_LIMITTEXT, MAX_TRANSFER_REASON_LEN-1, 0L);

		zstruct(LatestXferSrcRecordFromServer);

		zstruct(LatestXferDestRecordFromServer);

		SendMessage(hDlg, WMP_NEW_ACCOUNT_RECORD, 0, 0);

		// Request the latest info from the server...

		{

			struct AccountRecord ar;

			zstruct(ar);

			ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

			ar.sdb.player_id = dwTransferMoneySourcePlayerID;

			SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

			if (dwTransferMoneySourcePlayerID != dwTransferMoneyDestPlayerID) {

				// Source and dest are different... now request the dest.

				ar.sdb.player_id = dwTransferMoneyDestPlayerID;

				SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

			}

		}

		CheckDlgButton(hDlg, IDC_RADIO_SRC_PENDING,  FALSE);

		CheckDlgButton(hDlg, IDC_RADIO_DEST_PENDING, FALSE);

		CheckDlgButton(hDlg, IDC_RADIO_SRC_AVAIL,    TRUE);

		CheckDlgButton(hDlg, IDC_RADIO_DEST_AVAIL,   TRUE);

		CheckDlgButton(hDlg, IDC_CHKBOX_NO_HISTORY_ENTRY, FALSE);

		// default transaction (ie, no customization)

		CheckDlgButton(hDlg, IDC_XFER_DEFAULT, TRUE);

		// the custom messasge is CT_SPARE_SPACE max characters

		SendMessage(GetDlgItem(hDlg, IDC_XFER_CUSTOM_MESSAGE), EM_LIMITTEXT, CT_SPARE_SPACE, 0L);

		// limit digits in SFC transaction (9 digits max)

		SendMessage(GetDlgItem(hDlg, IDC_TRANSACTION_NUMBER), EM_LIMITTEXT, 9, 0L);

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;

	case WM_COMMAND:

		{

			char *temp_id = 0;

			int direction = 0;

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {

			case IDCANCEL:

				EndDialog(hDlg, LOWORD(wParam));

				return TRUE;	// We DID process this message

			case IDC_RETRIEVE_USERID1:

				zstruct(user_id);

				GetWindowText(GetDlgItem(hDlg, IDC_USERID_1), user_id, MAX_PLAYER_USERID_LEN-1);

				temp_id = user_id;

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_ECASH:

				temp_id = "Ecash";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MARKETING:

				temp_id = "Marketing";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_PENDING:

				temp_id = "Pending";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MISC:

				temp_id = "Misc.";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_CHECKS:

				temp_id = "ChecksIssued";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_DRAFTS:

				temp_id = "DraftsIn";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_WIRES:

				temp_id = "WiresIn";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_CHARGEBACKS:

				temp_id = "ChargeBack";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_PRIZES:

				temp_id = "Prizes";

				dwTransferMoneySourcePlayerID = 0;	// unknown player id

				zstruct(LatestXferSrcRecordFromServer);

				goto send_retrieve_now;

			// other side

			case IDC_RETRIEVE_USERID2:

				zstruct(user_id);

				GetWindowText(GetDlgItem(hDlg, IDC_USERID_2), user_id, MAX_PLAYER_USERID_LEN-1);

				temp_id = user_id;

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_ECASH2:

				temp_id = "Ecash";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MARKETING2:

				temp_id = "Marketing";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_PENDING2:

				temp_id = "Pending";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MISC2:

				temp_id = "Misc.";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_CHECKS2:

				temp_id = "ChecksIssued";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_DRAFTS2:

				temp_id = "DraftsIn";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_WIRES2:

				temp_id = "WiresIn";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_CHARGEBACKS2:

				temp_id = "ChargeBack";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;

			case IDC_RETRIEVE_PRIZES2:

				temp_id = "Prizes";

				dwTransferMoneyDestPlayerID = 0;	// unknown player id

				zstruct(LatestXferDestRecordFromServer);

				goto send_retrieve_now;



send_retrieve_now:

				{

					struct AccountRecord ar;

					zstruct(ar);

					strnncpy(ar.sdb.user_id, temp_id, MAX_PLAYER_USERID_LEN);

					ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_USERID;

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

				}

				SendMessage(hDlg, WMP_NEW_ACCOUNT_RECORD, 0, 0);

				return TRUE;	// We DID process this message

			case IDC_INITIATE_TRANSFER1:

				direction = 1;

				goto do_transfer;

			case IDC_INITIATE_TRANSFER2:

				direction = -1;

do_transfer:

				// They've clicked one of the transfer buttons.  Use 'direction'

				// to determine the sign of the value.

				{

					

					int treat_as_cc = IsDlgButtonChecked(hDlg, IDC_XFER_TREATASPURCHASE);

					int transaction_number = GetDlgTextInt(hDlg, IDC_TRANSACTION_NUMBER);

					char confirmmessage[150];

					zstruct(confirmmessage);



					if (treat_as_cc && !transaction_number) {

						MessageBox(hDlg, "You must include a transaction number.",

							"Treat as Credit Card purchase...",

							MB_OK|MB_APPLMODAL|MB_TOPMOST|MB_ICONHAND);

						return TRUE;	// We DID process this message

					}

                    

					char from_id[50];

					char to_id[50];

					char amount_str[50];

                    GetDlgItemText(hDlg, IDC_USERID_1, from_id, 50);

					GetDlgItemText(hDlg, IDC_USERID_2, to_id, 50);

					GetDlgItemText(hDlg, IDC_AMOUNT, amount_str, 50);

                   

                    if  (direction == 1) {

					  sprintf(confirmmessage, "You are about to transfer %s US dollar from %s to %s."

						"\nSend transfer request to server now?", amount_str,from_id, to_id);

					}else{

					  sprintf(confirmmessage, "You are about to transfer %s US dollar from %s to %s."

						"\nSend transfer request to server now?", amount_str, to_id, from_id);

					

					}



					if (MessageBox(hDlg, confirmmessage,

							"Transfer Confirmation", MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST)==IDOK)

					{

						long amount = (long)floor(GetDlgTextFloat(hDlg, IDC_AMOUNT) * 100.0 + .5);

						//kp(("%s(%d) amount = %d\n", _FL, amount));

						// Form a request and send it to the server.

						// Always make the amount positive.

						if (amount < 0) {

							amount = -amount;

							direction = -direction;

						}

						struct TransferRequest tr;

						zstruct(tr);

						tr.amount = amount;

						if (iTransferMoneyRealMoneyFlag) {

							tr.chip_type = CT_REAL;

						} else {

							tr.chip_type = CT_PLAY;

						}

						tr.transaction_number = transaction_number;

						if (direction >= 0) {

							tr.from_id = dwTransferMoneySourcePlayerID;

							tr.to_id   = dwTransferMoneyDestPlayerID;

							//tr.from_account_field = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING_CHECK)) {

								tr.from_account_field = 2;

							}

							//tr.to_account_field   = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING_CHECK)) {

								tr.to_account_field = 2;

							}



							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING)) {

								tr.from_account_field = 3;

							}

							//tr.to_account_field   = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING)) {

								tr.to_account_field = 3;

							}

							//rgong 04/05/2002

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_CCFEE)) {

								tr.from_account_field = 1;

							}

							//tr.to_account_field   = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_CCFEE)) {

								tr.to_account_field = 1;

							}

							//end rgong

						} else {

							tr.to_id   = dwTransferMoneySourcePlayerID;

							tr.from_id = dwTransferMoneyDestPlayerID;

							//tr.to_account_field   = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING_CHECK)) {

								tr.to_account_field = 2;

							}

							//tr.from_account_field = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING_CHECK)) {

								tr.from_account_field = 2;

							}



							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_PENDING)) {

								tr.from_account_field = 3;

							}

							//tr.to_account_field = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING)) {

								tr.to_account_field = 3;

							}

							//rgong 04/05/2002

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_SRC_CCFEE)) {

								tr.from_account_field = 1;

							}

							//tr.to_account_field = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_PENDING) ? 1 : 0);

							if (IsDlgButtonChecked(hDlg, IDC_RADIO_DEST_CCFEE)) {

								tr.to_account_field = 1;

							}

							//end rgong

						}

						// set flags

						if (treat_as_cc) {

							tr.flags |= TRF_TREAT_AS_CC_PURCHASE;

						}

						if (IsDlgButtonChecked(hDlg, IDC_XFER_NONE)) {

							tr.flags |= TRF_NO_HISTORY_ENTRY;

						}

						if (IsDlgButtonChecked(hDlg, IDC_XFER_TOANDFROM)) {

							tr.flags |= TRF_PLAYER_TO_PLAYER;

						}

						if (IsDlgButtonChecked(hDlg, IDC_XFER_CUSTOM)) {

							tr.flags |= TRF_CUSTOM_MESSAGE;

						}

						GetWindowText(GetDlgItem(hDlg, IDC_TRANSFER_REASON), tr.reason, MAX_TRANSFER_REASON_LEN-1);

						// we can grab all CT_SPARE_SPACE (11) characters because we have space for 12 in the tr struct

						char tmp[TR_SPARE_SPACE];

						zstruct(tmp);

						GetWindowText(GetDlgItem(hDlg, IDC_XFER_CUSTOM_MESSAGE), tmp, TR_SPARE_SPACE);

						strnncpy(tr.str, tmp, TR_SPARE_SPACE);

						// Clear them out so we see when they get refreshed....

						zstruct(LatestXferSrcRecordFromServer);

						zstruct(LatestXferDestRecordFromServer);

						SendMessage(hDlg, WMP_NEW_ACCOUNT_RECORD, 0, 0);

						// Finally, send the request to the server.

						SendDataStructure(DATATYPE_TRANSFER_REQUEST, &tr, sizeof(tr));

						// Request the latest info from the server...

						{

							struct AccountRecord ar;

							zstruct(ar);

							ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

							ar.sdb.player_id = dwTransferMoneySourcePlayerID;

							SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

							if (dwTransferMoneySourcePlayerID != dwTransferMoneyDestPlayerID) {

								// Source and dest are different... now request the dest.

								ar.sdb.player_id = dwTransferMoneyDestPlayerID;

								SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

							}

						}

					  #if 0	// ready to go, enable it when the server is ready 20010310HK

						// if it was a real-money person->person transfer, pop up the request to send the email

						if (tr.chip_type == CT_REAL && tr.flags & TRF_PLAYER_TO_PLAYER && amount &&

							tr.to_id != tr.from_id) {

							// if there was one of these already, it's been ignored -- just kill it

							if (hReqXferEmailDlg) {

								DestroyWindow(hReqXferEmailDlg);

							}

							CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_REQ_XFER_EMAIL), NULL, dlgReqXferEmail, (LPARAM)&tr);

						}

					  #endif

					}

				}

				return TRUE;	// We DID process this message

			}	// bottom of WM_COMMAND switch statment

		}	// bottom of WM_COMMAND

		break;

	case WM_DESTROY:

		hTransferMoneyDlg = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WMP_NEW_ACCOUNT_RECORD:

		// A new account record (or two) has arrived from the server...

		// copy it to our dialog box.

		{

			// First, fill in the fields of the source account...

			char str[200];

			zstruct(str);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_USERID_1),

				LatestXferSrcRecordFromServer.sdb.user_id);

			sprintf(str, "Name: %s\nCity: %s\nEmail: %s",

					LatestXferSrcRecordFromServer.sdb.full_name,

					LatestXferSrcRecordFromServer.sdb.city,

					LatestXferSrcRecordFromServer.sdb.email_address);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SRC_ACCOUNT_NAME), str);

			char currency_str[MAX_CURRENCY_STRING_LEN];

			int total = 0;

			if (iTransferMoneyRealMoneyFlag) {

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.real_in_bank, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_INPLAY),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.real_in_play, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CREDIT),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.pending_paypal, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CHECK1),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.pending_check, CT_REAL, TRUE));

				//rgong 04/05/2002

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_CCFEE1),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.pending_fee_refund, CT_REAL, TRUE));	

				//end rgong

				total = LatestXferSrcRecordFromServer.sdb.real_in_bank +

						LatestXferSrcRecordFromServer.sdb.real_in_play +

						LatestXferSrcRecordFromServer.sdb.pending_paypal +

						//rgong 04/05/2002

						LatestXferSrcRecordFromServer.sdb.pending_fee_refund +

						//end rgong

						LatestXferSrcRecordFromServer.sdb.pending_check;

			} else {

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.fake_in_bank, CT_PLAY, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_INPLAY),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.fake_in_play, CT_PLAY, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CREDIT), "");

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CHECK1), "");

				total = LatestXferSrcRecordFromServer.sdb.fake_in_bank +

						LatestXferSrcRecordFromServer.sdb.fake_in_play;

			}

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_TOTAL),

				CurrencyString(currency_str, total, (iTransferMoneyRealMoneyFlag ? CT_REAL : CT_PLAY), TRUE));



			// Now fill in the fields of the dest account...

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_USERID_2),

				LatestXferDestRecordFromServer.sdb.user_id);

			sprintf(str, "Name: %s\nCity: %s\nEmail: %s",

					LatestXferDestRecordFromServer.sdb.full_name,

					LatestXferDestRecordFromServer.sdb.city,

					LatestXferDestRecordFromServer.sdb.email_address);

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_DEST_ACCOUNT_NAME), str);

			total = 0;

			if (iTransferMoneyRealMoneyFlag) {

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY2),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.real_in_bank, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_INPLAY4),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.real_in_play, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CREDIT2),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.pending_paypal, CT_REAL, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CHECK2),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.pending_check, CT_REAL, TRUE));

				//rgong 04/05/2002

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_CCFEE2),

						CurrencyString(currency_str,

						LatestXferSrcRecordFromServer.sdb.pending_fee_refund, CT_REAL, TRUE));	

				//end rgong

				total = LatestXferDestRecordFromServer.sdb.real_in_bank +

						LatestXferDestRecordFromServer.sdb.real_in_play +

						LatestXferDestRecordFromServer.sdb.pending_paypal +

						//rgong 04/05/2002

						LatestXferSrcRecordFromServer.sdb.pending_fee_refund +

						//end rgong

						LatestXferDestRecordFromServer.sdb.pending_check;

			} else {

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY2),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.fake_in_bank, CT_PLAY, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_INPLAY4),

						CurrencyString(currency_str,

						LatestXferDestRecordFromServer.sdb.fake_in_play, CT_PLAY, TRUE));

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CREDIT2), "");

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CHECK2), "");

				total = LatestXferDestRecordFromServer.sdb.fake_in_bank +

						LatestXferDestRecordFromServer.sdb.fake_in_play;

			}

			SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_TOTAL2),

				CurrencyString(currency_str, total, (iTransferMoneyRealMoneyFlag ? CT_REAL : CT_PLAY), TRUE));

		}

		return TRUE;	// We DID process this message



	}

    return FALSE;	// We did NOT process this message.

}





static short sOldPlayerIDSpinValue;	// the spinner only holds 16 bits - we need 32.

static WORD32 dwOldPlayerID;



static char *PrivNames[] =  {

	"Priv: none",

	"Priv: play money only",

	"Priv: play or real",

	"Priv: pit boss",

	"Priv: customer support",

	"Priv: ADMINISTRATOR",

	"Priv: SUPER_USER",

	NULL

};

static BYTE8 PrivLevels[] = {

	ACCPRIV_LOCKED_OUT,		// account is locked out

	ACCPRIV_PLAY_MONEY,		// play money only

	ACCPRIV_REAL_MONEY,		// normal user account status

	ACCPRIV_PIT_BOSS,		// possibly an end-user with extra privs

	ACCPRIV_CUSTOMER_SUPPORT,	// customer support personel

	ACCPRIV_ADMINISTRATOR,	// non-programmer admin

	ACCPRIV_SUPER_USER,		// user can do anything, even with bad consequences

	0,0,0,0,0,0	// extra padding for fun

};



//*********************************************************

// 1999/09/23 - MB

//

// Fill the editable fields of an account record with the data

// from the dialog box fields.

//

static void FillAccountRecordFromDialogFields(HWND hDlg, struct AccountRecord *ar)

{

	ar->sdb.valid_entry = TRUE;

	ar->sdb.player_id = GetDlgTextHex(hDlg, IDC_PLAYERID);

    strcpy(ar->sdb.user_id, LoginUserID);

	//GetDlgText(hDlg, IDC_USERID, ar->sdb.user_id, MAX_PLAYER_USERID_LEN);

//	GetDlgText(hDlg, LoginUserID, ar->sdb.user_id, MAX_PLAYER_USERID_LEN);

	
	GetDlgText(hDlg, IDC_PASSWORD, ar->sdb.password, MAX_PLAYER_PASSWORD_LEN);

  #if 0	//19991111MB

	GetDlgText(hDlg, IDC_RM_PASSWORD, ar->sdb.password_rm, MAX_PLAYER_PASSWORD_LEN);

  #endif

	GetDlgText(hDlg, IDC_EMAIL, ar->sdb.email_address, MAX_EMAIL_ADDRESS_LEN);

	GetDlgText(hDlg, IDC_FULLNAME, ar->sdb.full_name, MAX_PLAYER_FULLNAME_LEN);

	GetDlgText(hDlg, IDC_LASTNAME, ar->sdb.last_name, MAX_PLAYER_LASTNAME_LEN);

	GetDlgText(hDlg, IDC_CITY, ar->sdb.city, MAX_COMMON_STRING_LEN);

	GetDlgText(hDlg, IDC_ADDRESS_LINE1, ar->sdb.mailing_address1, MAX_PLAYER_ADDRESS_LEN);

	GetDlgText(hDlg, IDC_ADDRESS_LINE2, ar->sdb.mailing_address2, MAX_PLAYER_ADDRESS_LEN);

	GetDlgText(hDlg, IDC_EDIT_STATE, ar->sdb.mailing_address_state, MAX_COMMON_STRING_LEN);

	GetDlgText(hDlg, IDC_EDIT_COUNTRY, ar->sdb.mailing_address_country, MAX_COMMON_STRING_LEN);

	GetDlgText(hDlg, IDC_EDIT_POSTAL_CODE, ar->sdb.mailing_address_postal_code, MAX_COMMON_STRING_LEN);

	char _phone_number[PHONE_NUM_EXPANDED_LEN+2];

	zstruct(_phone_number);

	GetDlgText(hDlg, IDC_EDIT_PHONE_NUMBER, _phone_number, PHONE_NUM_EXPANDED_LEN+1);

	//EncodePhoneNumber(ar->sdb.phone_number, _phone_number);

	char uncomp_notes[MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED];

	zstruct(uncomp_notes);

	GetDlgText(hDlg, IDC_ADMIN_NOTES, uncomp_notes, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED);

	CompressString(ar->sdb.admin_notes, MAX_PLAYER_ADMIN_NOTES_LEN, uncomp_notes, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED);



	ar->sdb.dont_use_email1 =  (BYTE8)(IsDlgButtonChecked(hDlg, IDC_CHKBOX_NOJUNKMAIL));

	ar->sdb.dont_use_email2 = !(BYTE8)(IsDlgButtonChecked(hDlg, IDC_CHKBOX_EMAIL_WHEN_REAL));



	ar->sdb.flags &= ~SDBRECORD_FLAG_EMAIL_NOT_VALIDATED;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_EMAIL_INVALID)) {

		ar->sdb.flags |= SDBRECORD_FLAG_EMAIL_NOT_VALIDATED;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_EMAIL_BOUNCES;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_EMAIL_BOUNCES)) {

		ar->sdb.flags |= SDBRECORD_FLAG_EMAIL_NOT_VALIDATED|SDBRECORD_FLAG_EMAIL_BOUNCES;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_LOCKED_OUT;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_LOCKEDOUT)) {

		ar->sdb.flags |= SDBRECORD_FLAG_LOCKED_OUT;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_NO_ALLIN_RESET;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_ALLIN_RESET)) {

		ar->sdb.flags |= SDBRECORD_FLAG_NO_ALLIN_RESET;

	}

	

	//rgong

	ar->sdb.flags &= ~SDBRECORD_FLAG_NO_INI_BONUS;

	//ar->sdb.flags &= ~SDBRECORD_FLAG_NO_NEWS;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_NEWS)) {

		//rgong 04/05/2002

		//ar->sdb.flags |= SDBRECORD_FLAG_NO_NEWS; 

		ar->sdb.flags |= SDBRECORD_FLAG_NO_INI_BONUS; 

		//end rgong

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_REAL_PLAYER;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_REAL_PLAYER)) {

		ar->sdb.flags |= SDBRECORD_FLAG_REAL_PLAYER; 

	}

	//end rgong



	ar->sdb.flags &= ~SDBRECORD_FLAG_LOGIN_ALERT;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_LOGIN_ALERT)) {

		ar->sdb.flags |= SDBRECORD_FLAG_LOGIN_ALERT;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_AUTO_BLOCK;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_AUTOBLOCK)) {

		ar->sdb.flags |= SDBRECORD_FLAG_AUTO_BLOCK;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_SQUELCH_CHAT;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_SQUELCH_CHAT)) {

		ar->sdb.flags |= SDBRECORD_FLAG_SQUELCH_CHAT;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_SQUELCH_RB_CHAT;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_SQUELCH_RB_CHAT)) {

		ar->sdb.flags |= SDBRECORD_FLAG_SQUELCH_RB_CHAT;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_GOOD_ALLIN_ALERT;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_GOOD_ALLIN_ALERT)) {

		ar->sdb.flags |= SDBRECORD_FLAG_GOOD_ALLIN_ALERT;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_DUPES_OK;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_DUPES_OK)) {

		ar->sdb.flags |= SDBRECORD_FLAG_DUPES_OK;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_VIP;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_VIP)) {

		ar->sdb.flags |= SDBRECORD_FLAG_VIP;

	}



	ar->sdb.flags &= ~SDBRECORD_FLAG_NO_CASHIER;

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_CASHIER)) {

		ar->sdb.flags |= SDBRECORD_FLAG_NO_CASHIER;

	}



	

    //Tony	ar->sdb.flags &= ~SDBRECORD_FLAG_USED_FIREPAY;

	ar->sdb.flags &= ~SDBRECORD_FLAG_NO_CASHOUT;	//Tony, Dec 21, 2001

	if (IsDlgButtonChecked(hDlg, IDC_CHKBOX_USED_FIREPAY)) {

    //Tony		ar->sdb.flags |= SDBRECORD_FLAG_USED_FIREPAY;

		ar->sdb.flags |= SDBRECORD_FLAG_NO_CASHOUT;	//Tony, Dec 21, 2001

	}





	ar->sdb.gender = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_FEMALE) ? GENDER_FEMALE : GENDER_MALE);



	// Read privilege level from combo box control

    int index = SendDlgItemMessage(hDlg, IDC_COMBO_ACCOUNT_PRIV, CB_GETCURSEL, 0, 0);

	if (index >= 0) {

		ar->sdb.priv = PrivLevels[index];

	}

	//kp(("%s(%d) Resulting priv level = %d (index %d)\n", _FL, ar->sdb.priv, index));



	//rgong

	ar->sdb.fee_credit_points = GetDlgTextInt(hDlg, IDC_EPTS);

	//end rgong

}



//*********************************************************

// 2000/04/04 - MB

//

// Update the length info for the admin notes field

//

void UpdateAdminNotesLengthFields(HWND hDlg)

{

	char uncomp_notes[MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED];

	char comp_notes[MAX_PLAYER_ADMIN_NOTES_LEN];

	zstruct(uncomp_notes);

	zstruct(comp_notes);

	GetDlgText(hDlg, IDC_ADMIN_NOTES, uncomp_notes, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED);

	int bytes_chopped = CompressString(comp_notes, MAX_PLAYER_ADMIN_NOTES_LEN, uncomp_notes, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED);

	char str[50];

	zstruct(str);

	if (bytes_chopped) {

		sprintf(str, "%d *TOO LONG*", bytes_chopped);

	} else {

		sprintf(str, "%d chars", strlen(uncomp_notes)+1);

	}

	SetDlgItemTextIfNecessary(hDlg, IDC_ADMIN_NOTES_LEN, str);

}



/**********************************************************************************

 Function GetArgs(int maxArgc, char *argv[], char *string, char seperator)

 Date: HK1999/08/09

 Purpose: internal parser

***********************************************************************************/

int GetArgs(int maxArgc, char *argv[], char *string, char seperator)

{

	memset(argv, 0, sizeof(argv[0])*maxArgc);	// start by setting all ptrs to null

	int argc = 0;

	while (*string) {

		argv[argc++] = string;

		if (argc >= maxArgc)

			return -1;

		while (*string && *string != seperator)

			++string;

		if (!*string)

			break;

		*(string++) = '\0';

	}

	return argc;

}

/**********************************************************************************

 Function FillDupeAccButtons(HWND hDlg, char *uncomp_notes)

 Date: HK00/05/31

 Purpose: fill the shortcut to dupes buttons

 NOTE: the resource.h MUST HAVE DUPE BUTTONS #define numbered SEQUENTIALLY

***********************************************************************************/

void FillDupeAccButtons(HWND hDlg, char *uncomp_notes)

{

	#define MAX_DUPE_BUTTONS	8

	#define MAX_DUPE_ARGS		50

	char *argv[MAX_DUPE_ARGS];

	int button_count = 0;

	char ids[MAX_DUPE_BUTTONS][MAX_PLAYER_USERID_LEN];

	memset(ids, 0, MAX_DUPE_BUTTONS * MAX_PLAYER_USERID_LEN);

	char current_id[MAX_PLAYER_USERID_LEN];

	zstruct(current_id);

	GetDlgItemText(hDlg, IDC_USERID, current_id, MAX_PLAYER_USERID_LEN);

	// hide them all

	for (int i=0; i < MAX_DUPE_BUTTONS; i++) {

		ShowWindow(GetDlgItem(hDlg, IDC_DUPE1+i), FALSE);

	}

	// find the Dupes

	char *search = uncomp_notes;

	char *p = NULL;

	int second_loop_pass = FALSE;	// we want to run through it twice

	#define DUPE_SEARCH_STRING_LEN	20

	char search_string[DUPE_SEARCH_STRING_LEN];

	zstruct(search_string);

	forever {

		int offset = 0;	// skip past our string

		if (!second_loop_pass) {

			strnncpy(search_string,"Dupes:", DUPE_SEARCH_STRING_LEN);

		} else {

			strnncpy(search_string,"same cc as", DUPE_SEARCH_STRING_LEN);

		}

		p = strstr(search, search_string);

		if (p) {

			offset = strlen(search_string);

		}

		if (!p) {

			if (!second_loop_pass) {	// try for more

				second_loop_pass = TRUE;

				search = uncomp_notes;

				continue;

			} else {	// we're done

				break;

			}

		}

		p+=offset;	// skip past our search words

		zstruct(argv);

		int count = GetArgs(MAX_DUPE_ARGS, argv, p, ',');

		for (int i = 0; i < count; i++) {

			if (button_count >= MAX_DUPE_BUTTONS) {

				break;

			}

			char possible_id[MAX_PLAYER_USERID_LEN * 2];

			zstruct(possible_id);

			strnncpy(possible_id, argv[i], MAX_PLAYER_USERID_LEN * 2);

			TrimString(possible_id, MAX_PLAYER_USERID_LEN * 2);

			// get rid of newlines and other crap

			for (int k = 0; k < (int)(strlen(possible_id)); k++) {

				if (possible_id[k] < 32) {

					possible_id[k] = 0;

				}

			}

			if (strlen(possible_id) < MAX_PLAYER_USERID_LEN) {

				// looks valid, see if we already have it

				int found = FALSE;

				if (!strcmpi(possible_id, current_id)) {	// it's the same person

					found = TRUE;

				} else {

					for (int j=0; j < button_count; j++) {

						if (!stricmp(ids[j], possible_id)) {

							found = TRUE;

							break;

						}

					}

				}

				if (!found) {

					strnncpy(ids[button_count], possible_id, MAX_PLAYER_USERID_LEN);

					SetDlgItemTextIfNecessary(hDlg, IDC_DUPE1+button_count, possible_id);

					ShowWindow(GetDlgItem(hDlg, IDC_DUPE1+button_count), TRUE);

					button_count++;

				}

			}

		}

		search = p;	// move to next ocurrence

	}

}



/**********************************************************************************

 Function PasteTextFromClipboard(char *dest, int max_characters)

 Date: HK00/10/17

 Purpose: grab whatever text is in the clipboard and paste it into 'dest' (up to the max)

***********************************************************************************/

void PasteTextFromClipboard(char *dest, int max_characters)

{

	*dest = 0;	// assume nothing good there to paste		

	if (!IsClipboardFormatAvailable(CF_TEXT)) { // something valid to paste?

		return;

	}

	if (!OpenClipboard(NULL)) { // couldn't open the clipboard?

		return;

	}

	HANDLE hCPD = GetClipboardData(CF_TEXT);

    if (!hCPD) {	// got nothing useful

		return;

	}

	// got something, paste it		

	char *data = (char *)GlobalLock(hCPD);

	strnncpy(dest, data, max_characters);

	// we're done

	GlobalUnlock(hCPD);

	CloseClipboard();

}



/**********************************************************************************

 Function *FileLastModifiedStr()

 Date: HK00/10/15

 Purpose: return a string (internal buffer ptr) with last-modified info for filename

***********************************************************************************/

char *FileLastModifiedStr(char *filename)

{

	#define FLM_BUF_SIZE	100

	static char output[FLM_BUF_SIZE];

	zstruct(output);

	struct stat buf;

	zstruct(buf);

	// get data associated with 'fh'

	int rc = stat(filename, &buf);

	/* Check if statistics are valid: */

	if(rc) {	// zero is good

		sprintf(output, "Bad filename (%s) trying to read info", filename);

	} else {

		char last_update_str[50];

		zstruct(last_update_str);

		ConvertSecondsToString(time(NULL)-buf.st_mtime, last_update_str, FALSE, FALSE, FALSE);

		sprintf(output, "%s ago", last_update_str);

		sprintf(output+strlen(output), " -- %s", ctime(&buf.st_mtime));

	}

	return output;

}



struct EmailSearchResult {

	WORD32 player_id;

	WORD32 login_time;

	int hands_played;

	char user_id[MAX_PLAYER_USERID_LEN];

	char email_address[MAX_EMAIL_ADDRESS_LEN];

	char full_name[MAX_PLAYER_FULLNAME_LEN];

};



/* Tony

#define EMAIL_LOOKUP_FILENAME	"emailref.txt"*/

#define EMAIL_LOOKUP_FILENAME	"playerinfo.dat"	//Tony, Dec 18, 2001

#define MAX_EMAIL_SEARCH_RESULTS 1000

EmailSearchResult esr[MAX_EMAIL_SEARCH_RESULTS];

enum EmailListSearchType { ELST_EMAIL, ELST_NAME, ELST_USERID } ;

EmailListSearchType last_search_type = ELST_EMAIL;

int email_search_results = 0;	// how many we found in our last search

char email_search_string[MAX_EMAIL_ADDRESS_LEN];

int email_search_base_index = 0;			// where we are in our search list



/**********************************************************************************

 Function FillEmailSearchStruct()

 Date: HK00/10/14

 Purpose: search for matching emails and fill the structure

 Returns: number of matches found

***********************************************************************************/

int FillEmailSearchStruct(char *search_str, EmailListSearchType elst)

{

	FILE *in = fopen(EMAIL_LOOKUP_FILENAME, "rt");

	if (!in) {

		MessageBox(NULL, 

			"Unable to open reference file for searching.\n"

			"Hit the [Get Update] button to get the file.\n",

			"Can't search...",

			MB_OK|MB_ICONWARNING|MB_TOPMOST|MB_APPLMODAL);

		return 0;

	}

	// fix email search string if it's in " name <address@place.com> format "

	#define MAX_LINE_IN	200

	char line_in[MAX_LINE_IN];

	zstruct(line_in);

	strnncpy(line_in, search_str, MAX_LINE_IN);

	// fix email search string if it's in " name <address@place.com> format "

	char *p = strchr(line_in, '<');

	if (p) {	// it is, trim the end too

		p += 1;	// advance past it

		char *q = strchr(line_in, '>');

		if (q) {

			*q = 0;	// terminate there

		}

	} else {	// just normal

		p = line_in;

	}

	// now we're ready to deal with it

	last_search_type = elst;

	strnncpy(email_search_string, p, MAX_EMAIL_ADDRESS_LEN);

	pr(("%s(%d) Looking for %s\n", 	_FL, email_search_string));

	memset(esr, 0, sizeof(EmailSearchResult)*MAX_EMAIL_SEARCH_RESULTS);

	int index = 0;

	#define MAX_EMAIL_SEARCH_ARGS	10

	char *argv[MAX_EMAIL_SEARCH_ARGS];

	zstruct(*argv);

	while (!feof(in)) {

		fgets(line_in, MAX_LINE_IN, in);

		int arg_count = GetArgs(MAX_EMAIL_SEARCH_ARGS, argv, line_in, '\t');

		//Tony	if (arg_count != 6) {	// broken line?

		if (arg_count < 6) {	// Tony, Dec 18, 2001

			continue;

		}

		//    0           1           2         3      4       5

		// player_id login_time hands_played user_id email full_name

		// convert to upper case for the compare

		unsigned int i;

		char l_search[MAX_EMAIL_ADDRESS_LEN];

		char l_email[MAX_EMAIL_ADDRESS_LEN];

		zstruct(l_search);

		zstruct(l_email);

		strnncpy(l_search, email_search_string, MAX_EMAIL_ADDRESS_LEN);

		// figure out what we're searching and use it (email or full name or UserID)

		// int arg_to_search = (elst == ELST_EMAIL ? 4 : 5); // REMOVE

		int arg_to_search = ELST_EMAIL;

		switch (elst) {

		case ELST_EMAIL:

			arg_to_search = 4;

			break;

		case ELST_NAME:

			arg_to_search = 5;

			break;

		case ELST_USERID:

			arg_to_search = 3;

			break;

		}

		// int arg_to_search = (elst == ELST_EMAIL ? 4 : 5); // REMOVE

		strnncpy(l_email, argv[arg_to_search], MAX_EMAIL_ADDRESS_LEN);

		for (i = 0; i < strlen(l_email); i++) {

			l_email[i] = (char)toupper(l_email[i]);

		}

		for (i = 0; i < strlen(l_search); i++) {

			l_search[i] = (char)toupper(l_search[i]);

		}

		if (strstr(l_email, l_search)) {	// found a match

			sscanf(argv[0], "%x", &esr[index].player_id);

			esr[index].login_time = atol(argv[1]);

			esr[index].hands_played = atoi(argv[2]);

			strnncpy(esr[index].user_id, argv[3], MAX_PLAYER_USERID_LEN);

			strnncpy(esr[index].email_address, argv[4], MAX_EMAIL_ADDRESS_LEN);

			strnncpy(esr[index].full_name, argv[5], MAX_PLAYER_FULLNAME_LEN);

			TrimString(esr[index].full_name, MAX_PLAYER_FULLNAME_LEN);	// get rid of trailing \n

			index++;

			if (index == MAX_EMAIL_SEARCH_RESULTS) { // filled result struct... stop now

				break;

			}

		}			

	}

	fclose(in);

	email_search_results = index;

	return index;

}



#define EMAIL_SPIN_START	500	// range will be set to 0/1000 so start in the middle and go forever

int old_email_spinner = EMAIL_SPIN_START-1;	// used below to know which side of the spinner we hit

#define EMAIL_SEARCH_PLAYER_BUTTONS	5		// how many player buttons are on the control

/**********************************************************************************

 Function CALLBACK dlgEmailSearch(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK00/10/14

 Purpose: dlg function for search IDs from email adresses

***********************************************************************************/

BOOL CALLBACK dlgEmailSearch(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	#define MAX_TEMP_STRING_LEN	100

	char str[MAX_TEMP_STRING_LEN];

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		hEmailSearchDLG = hDlg;

		WinRestoreWindowPos(ProgramRegistryPrefix, "EmailSearch", hDlg, NULL, NULL, FALSE, TRUE);

		WinPosWindowOnScreen(hDlg);

		SendMessage(GetDlgItem(hDlg, IDC_SPIN_EMAILSEARCH), UDM_SETRANGE, (WPARAM)0, (LPARAM)1000);

		SendMessage(GetDlgItem(hDlg, IDC_SPIN_EMAILSEARCH), UDM_SETPOS, 0, (LPARAM)MAKELONG(EMAIL_SPIN_START, 0));

		ShowWindow(hDlg, SW_SHOW);

		PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		return TRUE;



	case WMP_UPDATE_PLAYER_INFO:	// redraw the dlg

		{

			SetDlgItemText(hDlg, IDC_EDIT_SEARCH_EMAIL, email_search_string);

			// hide some windows if there aren't enough matches

			ShowWindow(GetDlgItem(hDlg, IDC_SPIN_EMAILSEARCH), 

				email_search_results > EMAIL_SEARCH_PLAYER_BUTTONS ? TRUE : FALSE);

			ShowWindow(GetDlgItem(hDlg, IDC_TEXT_MATCHES), 

				email_search_results > EMAIL_SEARCH_PLAYER_BUTTONS ? TRUE : FALSE);

			char text_box[1200];

			zstruct(text_box);

			int blank_buttons = 0;	// used for cosmetics below

			for (int i=0; i < EMAIL_SEARCH_PLAYER_BUTTONS; i++) {

				struct EmailSearchResult *e = &esr[email_search_base_index+i];

				if (e->player_id) {

					char age[100];

					zstruct(age);

					time_t now = time(NULL);

					ConvertSecondsToString(now - e->login_time, age, FALSE, TRUE, 1);

					SetDlgItemText(hDlg, IDC_FOUND_PLAYER1+i, e->user_id);

					sprintf(text_box+strlen(text_box),

						"%d hands, last login: %11.11s (%s ago)\n"

						"%s - %s\n",

						e->hands_played, TimeStr(e->login_time), age,

						e->email_address, e->full_name);

					ShowWindow(GetDlgItem(hDlg, IDC_FOUND_PLAYER1+i), TRUE);

				} else {

					blank_buttons++;

					ShowWindow(GetDlgItem(hDlg, IDC_FOUND_PLAYER1+i), FALSE);

				}

			}

			SetDlgItemText(hDlg, IDC_TEXT_BLOCK, text_box);

			zstruct(str);

			sprintf(str,"%d - %d of %d matches",

				email_search_base_index+1, 

				email_search_base_index+EMAIL_SEARCH_PLAYER_BUTTONS-blank_buttons,

				email_search_results);

			SetDlgItemText(hDlg, IDC_TEXT_MATCHES, str);

			// set title and buttons

			char _desc[20];

			zstruct(_desc);

			switch (last_search_type) {

			case ELST_EMAIL:

				strcpy(_desc, "Email");

				CheckDlgButton(hDlg, IDC_RADIO_EMAIL, BST_CHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_NAME2, BST_UNCHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_USERID, BST_UNCHECKED);

				break;

			case ELST_NAME:

				strcpy(_desc, "Name");

				CheckDlgButton(hDlg, IDC_RADIO_EMAIL, BST_UNCHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_NAME2, BST_CHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_USERID, BST_UNCHECKED);

				break;

			case ELST_USERID:

				strcpy(_desc, "UserID");

				CheckDlgButton(hDlg, IDC_RADIO_EMAIL, BST_UNCHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_NAME2, BST_UNCHECKED);

				CheckDlgButton(hDlg, IDC_RADIO_USERID, BST_CHECKED);

				break;

			}

			zstruct(str);

			sprintf(str, "%s matches for ' %s '",  _desc, email_search_string);

			SetWindowText(hDlg, str);

			// update ref file info

			sprintf(str, "SearchDB current up to %s", FileLastModifiedStr(EMAIL_LOOKUP_FILENAME));

			SetDlgItemText(hDlg, IDC_FILE_INFO, str);

		}

		



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDC_FOUND_PLAYER1:

		case IDC_FOUND_PLAYER2:

		case IDC_FOUND_PLAYER3:

		case IDC_FOUND_PLAYER4:

		case IDC_FOUND_PLAYER5:

			AutoLookupPlayerID = esr[email_search_base_index+LOWORD(wParam)-IDC_FOUND_PLAYER1].player_id;

			OpenAdminEditAccountDialog();			

			return TRUE;			

		case IDC_BUTTON_UPDATE_FILE:

			_spawnl(_P_NOWAIT, "updtref.bat", "updtref.bat", NULL);

			return TRUE;			

		case IDC_BUTTON_PASTE:

			PasteTextFromClipboard(str, MAX_TEMP_STRING_LEN);

			SetDlgItemText(hDlg, IDC_EDIT_SEARCH_EMAIL, str);

			CheckDlgButton(hDlg, IDC_RADIO_EMAIL, BST_CHECKED);

			CheckDlgButton(hDlg, IDC_RADIO_NAME2, BST_UNCHECKED);

			// fall through for now...

		case IDC_BUTTON_SEARCH:

			{

				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_SEARCH_EMAIL), str, MAX_TEMP_STRING_LEN);

				EmailListSearchType elst = ELST_EMAIL;

				if (IsDlgButtonChecked(hDlg, IDC_RADIO_EMAIL)) {

					elst = ELST_EMAIL;

				} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_NAME2)) {

					elst = ELST_NAME;

				} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_USERID)) {

					elst = ELST_USERID;

				}

				FillEmailSearchStruct(str, elst);

				email_search_base_index = 0;	// make sure it's at the beginning

				PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

			}

			return TRUE;

		case IDCANCEL:

			DestroyWindow(hDlg);

			return TRUE;	// We DID process this message

		break;

		}



	case WM_HSCROLL:	// hit the [ < ] [ > ] buttons

		if ((HWND)lParam==GetDlgItem(hDlg, IDC_SPIN_EMAILSEARCH)) {

			int nScrollCode = (int)LOWORD(wParam);

			short nPos = (short)HIWORD(wParam);

			if (nScrollCode==SB_THUMBPOSITION) {

				pr(("%s(%d) nScrollCode = %d, new position = %d, old position = %d\n",

					_FL, nScrollCode, nPos, old_email_spinner));

				int scrolled_right = (nPos > old_email_spinner);

				pr(("%s(%d) Scrolled %s\n", _FL, scrolled_right ? "RIGHT" : "LEFT"));

				if (scrolled_right) {

					if (email_search_base_index+EMAIL_SEARCH_PLAYER_BUTTONS < email_search_results) {

						email_search_base_index += EMAIL_SEARCH_PLAYER_BUTTONS;

					}

				} else {	// scrolled left

					email_search_base_index -= EMAIL_SEARCH_PLAYER_BUTTONS;

					email_search_base_index = max(0, email_search_base_index);

				}

				old_email_spinner = nPos;

			}

			PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		}

		return TRUE;	// We DID process this message



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "EmailSearch", hDlg, NULL);

		hEmailSearchDLG = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		break;

	}	

	

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/09/23 - MB

//

// Fill in the fields of the edit account dialog box with the

// latest info from the server stored in LatestAccountRecordFromServer.

//

static void FillEditDialogFields(void)

{

	if (!hAdminEditAccountDlg) return;

	HWND hDlg = hAdminEditAccountDlg;

	struct AccountRecord *ar = &LatestAccountRecordFromServer;



	int valid = FALSE;



	// If the record from the server is blank, restore the player_id so

	// we can browse.

	if (!ar->sdb.player_id) {

		ar->sdb.player_id = dwOldPlayerID;

	} else {

		valid = TRUE;

	}



	if ( LoggedInPrivLevel >= 40  ) { 

		EnableWindow(GetDlgItem(hDlg, IDC_SAVE), valid);

    }

	dwOldPlayerID = ar->sdb.player_id;	// keep for next time.

	sOldPlayerIDSpinValue = (short)dwOldPlayerID;

	//kp(("%s(%d) Setting spin position to %d\n", _FL, sOldPlayerIDSpinValue));

	SendMessage(GetDlgItem(hDlg, IDC_PLAYERID_SPIN), UDM_SETPOS, 0, (LPARAM) MAKELONG(sOldPlayerIDSpinValue, 0));



	if (valid) {

		// Add this to the user id combo box

		// First, remove it if it's already there

		int index = SendMessage(GetDlgItem(hDlg, IDC_USERID1), CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)ar->sdb.user_id);

		if (index != CB_ERR) {

	        SendMessage(GetDlgItem(hDlg, IDC_USERID1), CB_DELETESTRING, index, (LPARAM)0);



		}

		// Now insert it at the top of the list.

        SendMessage(GetDlgItem(hDlg, IDC_USERID1), CB_INSERTSTRING, 0, (LPARAM)ar->sdb.user_id);

	}



	if (ar->sdb.user_id[0]) {

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_USERID1), ar->sdb.user_id);

	}

	SetFocus(GetDlgItem(hDlg, IDC_USERID1));

	SendMessage(GetDlgItem(hDlg, IDC_USERID1), EM_SETSEL, (WPARAM)0, (LPARAM)-1);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_USERID), ar->sdb.user_id);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_PASSWORD), ar->sdb.password);

  #if 0	//19991111MB

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_RM_PASSWORD), ar->sdb.password_rm);

  #endif

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EMAIL), ar->sdb.email_address);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_FULLNAME), ar->sdb.idAffiliate/*ar->sdb.full_name*/);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_CITY), ar->sdb.city);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_ADDRESS_LINE1), ar->sdb.mailing_address1);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_ADDRESS_LINE2), ar->sdb.mailing_address2);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EDIT_STATE), ar->sdb.mailing_address_state);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EDIT_COUNTRY), ar->sdb.mailing_address_country);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EDIT_POSTAL_CODE), ar->sdb.mailing_address_postal_code);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EDIT_CASHOUT), "");

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EDIT_PHONE_NUMBER), DecodePhoneNumber(ar->sdb.phone_number));

	char uncomp_notes[MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED];

	zstruct(uncomp_notes);

	UncompressString(ar->sdb.admin_notes, MAX_PLAYER_ADMIN_NOTES_LEN, uncomp_notes, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_ADMIN_NOTES), uncomp_notes);

	UpdateAdminNotesLengthFields(hDlg);

	//FillDupeAccButtons(hDlg, uncomp_notes);

	char str[2000];

	char str2[30];

	zstruct(str);

	zstruct(str2);

	sprintf(str, "0x%08lx", ar->sdb.player_id);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_PLAYERID), str);

	// Set the title

	sprintf(str, "Viewing account: ' %s '", ar->sdb.user_id);

	SetWindowText(hDlg, str);



	// Fill in the recent logins and IP addresses...

	int i;

	strcpy(str, "Recent logins (CST) and computer information:\r\n");

	for (i=0 ; i<LOGINS_TO_RECORD_PER_PLAYER ; i++) {

		if (!ar->sdb.last_login_times[i])

			break;

		char ip_str[20];

		IP_ConvertIPtoString(ar->sdb.last_login_ip[i], ip_str, 20);

		sprintf(str+strlen(str), " #%02d: %s from %s (%d)\r\n",

				i+1, TimeStr(ar->sdb.last_login_times[i], FALSE, TRUE, SERVER_TIMEZONE),

				ip_str, ar->sdb.last_login_computer_serial_nums[i]);

	}

	// Fill in the client's computer information...

	strcat(str, "\r\n");

	FillClientPlatformInfo(str+strlen(str), &ar->sdb.client_platform, ar->sdb.client_version);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_LOGINS), str);



	// Fill in the all-ins...

	str[0] = 0;

	int _local_count = 0;

	for (i=0 ; i<ALLINS_TO_RECORD_PER_PLAYER ; i++) {

		char *connection_state = "unknown";

		switch (ar->sdb.all_in_connection_state[i]) {

		case 0:

			connection_state = "good";

			break;

		case 1:

			connection_state = "poor";

			break;

		case 2:

			connection_state = "bad";

			break;

		case 3:

			connection_state = "lost";

			break;

		}

	  #if 0	//20001017MB

		sprintf(str+strlen(str), "#%d:", i+1);

	  #endif

		if (ar->sdb.all_in_times[i]) {

			sprintf(str+strlen(str), "%d> %11.11s (%s) #%d   ", ++_local_count,

					TimeStr(ar->sdb.all_in_times[i], FALSE, TRUE, SERVER_TIMEZONE),

					connection_state,

					ar->sdb.all_in_game_numbers[i]);

		}

		strcat(str, "\r\n");

	}

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_ALLINS), str);



	struct tm tm;

	localtime((time_t *)&ar->sdb.account_creation_time, &tm);

	sprintf(str, "Date Opened: %04d/%02d/%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_DATE_OPENED), str);



	sprintf(str, "%d hands, %d%% flops", ar->sdb.hands_seen, ar->sdb.hands_seen ? ar->sdb.flops_seen*100 / ar->sdb.hands_seen : 0);

//	sprintf(str, "%d hands, %d%% flops", ar->sdb.flops_seen, ar->sdb.flops_seen ? ar->sdb.flops_seen*100 / ar->sdb.hands_seen : 0);
	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_HANDS_SEEN), str);

	char curr_str[MAX_CURRENCY_STRING_LEN];

	// real money

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.real_in_bank, CT_REAL));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY), str);

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.real_in_play, CT_REAL));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_INPLAY), str);

	/*

	sprintf(str, "(%d pt) %s", 

		ar->sdb.fee_credit_points,

		CurrencyString(curr_str, ar->sdb.pending_fee_refund, CT_REAL));

	*/

	sprintf(str, "%s", 

		CurrencyString(curr_str, ar->sdb.pending_paypal, CT_REAL));

	

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CREDIT), str);

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.pending_check, CT_REAL));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_PENDING_CHECK), str);



	//rgong 04/05/2002

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.pending_fee_refund, CT_REAL));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_CCFEE), str);

	//end rgong



	INT32 total = ar->sdb.real_in_bank + ar->sdb.real_in_play + ar->sdb.pending_fee_refund + ar->sdb.pending_check + ar->sdb.pending_paypal;

	sprintf(str, "%s", CurrencyString(curr_str, total,CT_REAL));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_REALMONEY_TOTAL), str);



	// Calculate the gross and net exposure

	int exposure = 0;

	for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

		if (ar->sdb.transaction[i].transaction_type == CTT_PURCHASE) {

			exposure += ar->sdb.transaction[i].credit_left;

			// 20001013HK: take into account if it's disabled from crediting

			if (ar->sdb.transaction[i].credit_left > ar->sdb.transaction[i].transaction_amount) {

				exposure -= ar->sdb.transaction[i].transaction_amount;

			}

		}

	}

	int net_exposure = exposure - ar->sdb.real_in_bank - ar->sdb.real_in_play - ar->sdb.pending_check;

	ShowWindow(GetDlgItem(hDlg, IDC_NET_EXPOSURE), net_exposure == 0 ? FALSE : TRUE);

	ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_RED), FALSE);

	ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_GREEN), FALSE);

	if (net_exposure < -1000*100) {

		ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_RED), FALSE);

		ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_GREEN), TRUE);

		// green

	} else if (net_exposure > 1000*100) {

		ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_RED), TRUE);

		ShowWindow(GetDlgItem(hDlg, IDC_EXPOSURE_GREEN), FALSE);

		// red

	}

	sprintf(str, "Net %s:  %s", 

		net_exposure < 0 ? "surplus" : "at risk",

		// 20010202HK: added abs() so number is always positive in display

		CurrencyString(curr_str, abs(net_exposure), CT_REAL, FALSE, 1));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_NET_EXPOSURE), str);

	// play money

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.fake_in_bank, CT_PLAY));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_PLAYMONEY), str);

	//rgong

	ShowWindow(GetDlgItem(hDlg, IDC_RAKED_GAMES), TRUE);



	{

	int hands_to_play = 0;

	if (ar->sdb.pending_fee_refund > 0 )

		hands_to_play = 100 - ar->sdb.good_raked_games;

	sprintf(str, "%d", hands_to_play);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_RAKED_GAMES), str);

	}



	ShowWindow(GetDlgItem(hDlg, IDC_EPTS), TRUE);

	sprintf(str, "%d", ar->sdb.fee_credit_points);

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_EPTS), str);

	//end rgong

	sprintf(str, "%s", CurrencyString(curr_str, ar->sdb.fake_in_play, CT_PLAY));

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_PLAYMONEY_INPLAY), str);



	CheckDlgButton(hDlg, IDC_CHKBOX_NOJUNKMAIL, 	  ar->sdb.dont_use_email1 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_EMAIL_WHEN_REAL, !ar->sdb.dont_use_email2 ? BST_CHECKED : BST_UNCHECKED);

	// CheckDlgButton(hDlg, IDC_ATLEAST21, BST_CHECKED);

	CheckDlgButton(hDlg, IDC_MALE,   ar->sdb.gender == GENDER_MALE   ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_FEMALE, ar->sdb.gender == GENDER_FEMALE ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_EMAIL_INVALID,	ar->sdb.flags & SDBRECORD_FLAG_EMAIL_NOT_VALIDATED ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_EMAIL_BOUNCES,	ar->sdb.flags & SDBRECORD_FLAG_EMAIL_BOUNCES ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_LOCKEDOUT,		ar->sdb.flags & SDBRECORD_FLAG_LOCKED_OUT	 ? BST_CHECKED : BST_UNCHECKED);

	//rgong 04/05/2002

	CheckDlgButton(hDlg, IDC_CHKBOX_NO_NEWS,		ar->sdb.flags & SDBRECORD_FLAG_NO_INI_BONUS		 ? BST_CHECKED : BST_UNCHECKED);

	//CheckDlgButton(hDlg, IDC_CHKBOX_NO_NEWS,		ar->sdb.flags & SDBRECORD_FLAG_NO_NEWS		 ? BST_CHECKED : BST_UNCHECKED);

	//end rgong

	//rgong 04/09/2002

	CheckDlgButton(hDlg, IDC_CHKBOX_REAL_PLAYER,	ar->sdb.flags & SDBRECORD_FLAG_REAL_PLAYER		 ? BST_CHECKED : BST_UNCHECKED);

	//end rgong

	CheckDlgButton(hDlg, IDC_CHKBOX_LOGIN_ALERT,	ar->sdb.flags & SDBRECORD_FLAG_LOGIN_ALERT	 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_AUTOBLOCK,		ar->sdb.flags & SDBRECORD_FLAG_AUTO_BLOCK	 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_SQUELCH_CHAT,	ar->sdb.flags & SDBRECORD_FLAG_SQUELCH_CHAT	 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_SQUELCH_RB_CHAT,ar->sdb.flags & SDBRECORD_FLAG_SQUELCH_RB_CHAT? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_GOOD_ALLIN_ALERT,ar->sdb.flags & SDBRECORD_FLAG_GOOD_ALLIN_ALERT ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_DUPES_OK,		ar->sdb.flags & SDBRECORD_FLAG_DUPES_OK      ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_NO_ALLIN_RESET,	ar->sdb.flags & SDBRECORD_FLAG_NO_ALLIN_RESET? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_NO_CASHIER,		ar->sdb.flags & SDBRECORD_FLAG_NO_CASHIER	 ? BST_CHECKED : BST_UNCHECKED);

	//Tony	CheckDlgButton(hDlg, IDC_CHKBOX_USED_FIREPAY,	ar->sdb.flags & SDBRECORD_FLAG_USED_FIREPAY	 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hDlg, IDC_CHKBOX_USED_FIREPAY,	ar->sdb.flags & SDBRECORD_FLAG_NO_CASHOUT	 ? BST_CHECKED : BST_UNCHECKED);	// Tony, Dec 21, 2001

	EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_USED_FIREPAY), ar->sdb.flags & SDBRECORD_FLAG_NO_CASHIER ? FALSE : TRUE);	//Tony, Dec 21, 2001







	// 20000702HK: VIP checkbox and picture

	CheckDlgButton(hDlg, IDC_CHKBOX_VIP, ar->sdb.flags & SDBRECORD_FLAG_VIP ? BST_CHECKED : BST_UNCHECKED);

	if (ar->sdb.flags & SDBRECORD_FLAG_VIP) {

		ShowWindow(GetDlgItem(hDlg, IDC_PICTURE_VIP), TRUE);

	} else {

		ShowWindow(GetDlgItem(hDlg, IDC_PICTURE_VIP), FALSE);

	}

	sprintf(str, "%s", ar->flags & ACRF_LOGGED_IN ? "Logged in" : "Not logged in");

	// login status & buttons for pulling up the player's tables he's at

	ShowWindow(GetDlgItem(hDlg, IDC_SEATED_AT_1), FALSE);

	ShowWindow(GetDlgItem(hDlg, IDC_SEATED_AT_2), FALSE);

	// only supports two windows (as there are two buttons)

	for (i=0 ; i<MAX_GAMES_PER_PLAYER ; i++) {

		if (ar->seated_tables[i]) {

		  #if 0	// 20001217HK: better to know if he's connected or not

			sprintf(str,"Playing");

		  #endif

			sprintf(str2, "%s", TableNameFromSerialNumber(ar->seated_tables[i]));

			// if the top button is already being used, use the second one

			if (IsWindowVisible(GetDlgItem(hDlg, IDC_SEATED_AT_1))) {

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SEATED_AT_2), str2);

				ShowWindow(GetDlgItem(hDlg, IDC_SEATED_AT_2), TRUE);

				sprintf(str2, "%d", ar->seated_tables[i]);

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SERIAL_NUM_S2), str2);

			} else {	// first call, use first button

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SEATED_AT_1), str2);

				ShowWindow(GetDlgItem(hDlg, IDC_SEATED_AT_1), TRUE);

				sprintf(str2, "%d", ar->seated_tables[i]);

				SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_SERIAL_NUM_S1), str2);

			}

		}

	}

	SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_LOGGED_IN_STATUS), str);

	



	// Set privilege level combo box control

    int index = 0;

	while ((!index || PrivLevels[index]) && PrivLevels[index] < ar->sdb.priv) {

		index++;

	}

	if (!PrivLevels[index])

		index = 0;

    SendDlgItemMessage(hDlg, IDC_COMBO_ACCOUNT_PRIV, CB_SETCURSEL, (WPARAM)index, 0);

}



//*********************************************************

// Tooltips for the Admin Edit Accounts screen

static struct DlgToolTipText AdminEditAccountToolTipText[] = {

	//Tony	IDC_CHKBOX_USED_FIREPAY,	"This is checked if the client has ever used FirePay",

	IDC_CHKBOX_USED_FIREPAY,	"This is checked if player is not allowed to access cash out function",	//Tony, Dec 21, 2001

	0,0

};



//*********************************************************

// 1999/09/22 - MB

//

// Mesage handler for administrator edit account settings dialog window

//

BOOL CALLBACK dlgFuncAdminEditAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	static HWND tooltiphwnd;	// note: this static is a BAD idea if we ever support more than one edit account screen

	char temp_id[MAX_PLAYER_USERID_LEN];	// used for retrieve shortcuts

	switch (message) {

	case WM_INITDIALOG:

		hAdminEditAccountDlg = hDlg;

		WinRestoreWindowPos(ProgramRegistryPrefix, "AccountEdit", hDlg, NULL, NULL, FALSE, TRUE);

		AddKeyboardTranslateHwnd(hDlg);

		tooltiphwnd = OpenToolTipWindow(hDlg, AdminEditAccountToolTipText);

		// Add the system menu and our icon.  See Knowledge base

		// article Q179582 for more details.

		{

			HICON hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_CLIENT),

					IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),

                    GetSystemMetrics(SM_CYSMICON), 0);

			if(hIcon) {

				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			}



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



				mii.wID = (WORD)IDM_INSERT_BLANK_COMPUTER_NUM;

				mii.dwTypeData = "Allow user to create another new account...";

				InsertMenuItem(hm, insert_position++, MF_BYPOSITION, &mii);



				mii.wID = (WORD)IDM_MOVE_TO_TOP_WAITLIST;

				mii.dwTypeData = "Move player to top of all waiting lists...";

				InsertMenuItem(hm, insert_position++, MF_BYPOSITION, &mii);



				mii.fType = MFT_SEPARATOR;

				mii.wID = (WORD)0;

				mii.dwTypeData = 0;

				InsertMenuItem(hm, insert_position++, MF_BYPOSITION, &mii);

			}

		}

		{

			// Limit the # of characters in the various edit boxes

			// the struct holds 8 bytes (PHONE_NUM_LEN) encoded, meaning 16 actual characters

			static int max_edit_lengths[] =  {

			  #if 1	//19990819MB: specify lower limit so it fits on the player id boxes

				IDC_USERID, 13,

			  #else

				IDC_USERID, MAX_PLAYER_USERID_LEN,

			  #endif

				IDC_PASSWORD, MAX_PLAYER_PASSWORD_LEN,

				IDC_RM_PASSWORD, MAX_PLAYER_PASSWORD_LEN,

				IDC_EMAIL, MAX_EMAIL_ADDRESS_LEN,

				IDC_FULLNAME, MAX_PLAYER_FULLNAME_LEN,

				IDC_ADDRESS_LINE1, MAX_PLAYER_ADDRESS_LEN,

				IDC_ADDRESS_LINE2, MAX_PLAYER_ADDRESS_LEN,

				IDC_CITY, MAX_COMMON_STRING_LEN,

				IDC_EDIT_STATE, MAX_COMMON_STRING_LEN,

				IDC_EDIT_COUNTRY, MAX_COMMON_STRING_LEN,

				IDC_EDIT_POSTAL_CODE, MAX_COMMON_STRING_LEN,

				IDC_ADMIN_NOTES, MAX_PLAYER_ADMIN_NOTES_LEN_UNCOMPRESSED,

				IDC_EDIT_PHONE_NUMBER, PHONE_NUM_EXPANDED_LEN+1,

				0,0

			};

			int *p = max_edit_lengths;

			while (*p) {

				SendMessage(GetDlgItem(hDlg, p[0]), EM_LIMITTEXT, p[1]-1, 0L);

				p += 2;

			}

			SendMessage(GetDlgItem(hDlg, IDC_PLAYERID_SPIN), UDM_SETRANGE,

					0, (LPARAM)MAKELONG((short)-32768, (short)32767));

			SendMessage(GetDlgItem(hDlg, IDC_PLAYERID_SPIN), UDM_SETPOS, 0, 0);

			SendMessage(GetDlgItem(hDlg, IDC_HANDCOUNT_SPIN), UDM_SETRANGE,

					0, (LPARAM)MAKELONG((short)100, (short)1));

			SendMessage(GetDlgItem(hDlg, IDC_HANDCOUNT_SPIN), UDM_SETPOS,

					0, (LPARAM) MAKELONG((short)5, 0));



			// Fill in the fields for the privilege combo box

			HWND combo = GetDlgItem(hDlg, IDC_COMBO_ACCOUNT_PRIV);

		    SendMessage(combo, CB_RESETCONTENT, 0, 0); 

			char **s = PrivNames;

			while (*s) {

		        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)*s);

				s++;

			}

			SendMessage(combo, CB_SETCURSEL, 0, 0);

		}

		PostMessage(GetDlgItem(hDlg, IDC_HH_ADMIN_CHECK), BM_SETCHECK, BST_CHECKED, 0);

		// show admin buttons

		if (LoggedInPrivLevel >= ACCPRIV_ADMINISTRATOR) {

			ShowWindow(GetDlgItem(hDlg, IDC_RETRIEVE_ECASH), SW_SHOWNA);

			ShowWindow(GetDlgItem(hDlg, IDC_RETRIEVE_RAKE), SW_SHOWNA);

			ShowWindow(GetDlgItem(hDlg, IDC_RETRIEVE_ECASH_FEE), SW_SHOWNA);

			ShowWindow(GetDlgItem(hDlg, IDC_RETRIEVE_CHARGEBACK), SW_SHOWNA);

		}

		ShowWindow(hDlg, SW_SHOW);

		return TRUE;



	case WM_ACTIVATE:

		if (LOWORD(wParam)!=WA_INACTIVE) {

			// we got activated...

			SetFocus(GetDlgItem(hDlg, IDC_USERID1));
			SendMessage(GetDlgItem(hDlg, IDC_USERID1), EM_SETSEL, (WPARAM)0, (LPARAM)-1);

		}

		return TRUE;	// TRUE = we did process this message.



	case WM_HSCROLL:	// hit the [ < ] [ > ] buttons on UserID

		if ((HWND)lParam==GetDlgItem(hDlg, IDC_SPIN_USERID)) {

			int nScrollCode = (int)LOWORD(wParam);

			int scrolled_right = FALSE;

			if (nScrollCode==SB_THUMBPOSITION) {

				short nPos = (short)HIWORD(wParam);

				if (old_playerid_spinner == -1) {	// first call

					if (nPos == 100) {

						scrolled_right = TRUE;

					}

				} else {

					if (!nPos && old_playerid_spinner == 100) {

						scrolled_right = FALSE;

					} else if (!old_playerid_spinner && nPos == 100) {

						scrolled_right = TRUE;

					} else if (nPos < old_playerid_spinner) {

						scrolled_right = TRUE;

					}

				}

				pr(("%s(%d) nScrollCode = %d, new position = %d, old position = %d\n",

					_FL, nScrollCode, nPos, old_playerid_spinner));

				pr(("%s(%d) Scrolled %s\n", _FL, scrolled_right ? "RIGHT" : "LEFT"));

				old_playerid_spinner = nPos;

				WORD32 player_to_request = 0;

				if (scrolled_right) {	// means we decrement towards zero on the index

					if (AdminPlayerIDIndex > 0) {

						AdminPlayerIDIndex--;

						player_to_request = AdminPlayerIDs[AdminPlayerIDIndex];

					}	

				} else {	// left scroll -- something useful there?

					if (AdminPlayerIDIndex+1 < MAX_REMEMBERED_ADMIN_PLAYERIDS && AdminPlayerIDs[AdminPlayerIDIndex+1]) {

						AdminPlayerIDIndex++;

						player_to_request = AdminPlayerIDs[AdminPlayerIDIndex];

					}

				}

				// pull it up if we found something useful

				if (player_to_request) {

					AutoLookupPlayerID = player_to_request;

					PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_PLAYERID, 0);	// re-request it.

				}

			}

		}

		return TRUE;	// We DID process this message



	

	// need the Whistler SDK for these so we'll do it the old-fashioned way

	case 0x319:	// case WM_APPCOMMAND:

		{

			pr(("%s(%d) WM_APPCOMMAND: lParam = HI%d, LO:%d, wParam = HI:%d, LO:%d\n",

				_FL, HIWORD(lParam), LOWORD(lParam), HIWORD(wParam), LOWORD(wParam)));

			WORD32 player_to_request = 0;

			switch (HIWORD(lParam)) {

				// WinUser.h:#define FAPPCOMMAND_MOUSE 0x8000

				// WinUser.h:#define APPCOMMAND_BROWSER_BACKWARD       1

				// WinUser.h:#define APPCOMMAND_BROWSER_FORWARD        2

			case 0x8001:	// backward

				if (AdminPlayerIDIndex+1 < MAX_REMEMBERED_ADMIN_PLAYERIDS && AdminPlayerIDs[AdminPlayerIDIndex+1]) {

					AdminPlayerIDIndex++;

					player_to_request = AdminPlayerIDs[AdminPlayerIDIndex];

				}

				break;

			case 0x8002:	// forward

				if (AdminPlayerIDIndex > 0) {

					AdminPlayerIDIndex--;

					player_to_request = AdminPlayerIDs[AdminPlayerIDIndex];

				}

				break;

			}

			// pull it up if we found something useful

			if (player_to_request) {

				AutoLookupPlayerID = player_to_request;

				PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_PLAYERID, 0);	// request it.

			}

		}

		break;

	

	case WM_COMMAND:

		{

			// Process other buttons on the window...

			switch (LOWORD(wParam)) {	// switch on control identifier

			case IDCANCEL:

				//kp(("%s(%d) IDCANCEL received.  Cancelling upgrade.\n",_FL));

				DestroyWindow(hDlg);

				return TRUE;	// We DID process this message



			case IDC_ADMIN_NOTES:

				if (HIWORD(wParam)==EN_CHANGE) {

					// The admin notes section changed.

					UpdateAdminNotesLengthFields(hDlg);

				}

				return TRUE;	// We DID process this message

			case IDC_BLOCK_COMPUTER:

				dwComputerSerialNumToBlock = LatestAccountRecordFromServer.sdb.client_platform.computer_serial_num;	// default block number

				DialogBox(hInst, MAKEINTRESOURCE(IDD_BLOCK_COMPUTER), hDlg, dlgFuncBlockComputer);

				return TRUE;	// We DID process this message

			case IDC_BROADCAST_MESSAGE:	// send a broadcast to this player

				dwBroadcastDestPlayerID = LatestAccountRecordFromServer.sdb.player_id;

				DialogBox(hInst, MAKEINTRESOURCE(IDD_BROADCAST_MESSAGE), hDlg, dlgFuncEnterBroadcastMessage);

				return TRUE;	// We DID process this message

			case IDC_SET_CCLIMITS:	// set CC purchase limits for this player

				DialogBox(hInst, MAKEINTRESOURCE(IDD_CC_LIMITS), hDlg, dlgSetCCLimits);

				return TRUE;

			case IDC_KICK_OFF:	// kick this player off the system

				if (LatestAccountRecordFromServer.sdb.player_id) {

					WORD32 player_id = LatestAccountRecordFromServer.sdb.player_id;

					char str[300];

					zstruct(str);

					sprintf(str,"Are you sure you want to kick '%s' off\n"

								"the system right now?\n\n"

								"Note: if they're not locked out the client will\n"

								"automatically reconnect in a few seconds.",

								LatestAccountRecordFromServer.sdb.user_id);

					int result = MessageBox(hDlg, str, "Kick player off system",

							MB_OKCANCEL|MB_ICONHAND|MB_TOPMOST|MB_APPLMODAL);

					if (result==IDOK) {

						// Do it.

						struct AccountRecord ar;

						zstruct(ar);

						ar.sdb.player_id = player_id;

						ar.usage = ACCOUNTRECORDUSAGE_KICKOFF_PLAYERID;

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

					}

				}

				return TRUE;	// We DID process this message

			case IDC_EDIT_PHONE_NUMBER:

				if (HIWORD(wParam)==EN_CHANGE) {

					FixUpPhoneNumberEditControl(hDlg);

				}

				return TRUE;

			case IDC_GAIR_LETTER:

				{

					// we'll assume the most recent all-in is the one we want

					int hand_number = 0;

					for (int i=0 ; i < ALLINS_TO_RECORD_PER_PLAYER ; i++) {

						if (LatestAccountRecordFromServer.sdb.all_in_game_numbers[i]) {

							hand_number = LatestAccountRecordFromServer.sdb.all_in_game_numbers[i];

							break;

						}

					}

					// abort right here if we didn't find one

					if (!hand_number) {

						MessageBox(hDlg, "Couldn't find any recent all-in hands to ask about",

									"No can do...", MB_OK|MB_ICONSTOP);

						return TRUE;

					}

					// something there; fire off the request

					SendPlayerGAIRLetter(hDlg, 

						LatestAccountRecordFromServer.sdb.player_id,

						LatestAccountRecordFromServer.sdb.user_id,

						hand_number);

				}

				return TRUE;



			case IDC_REQINFO_ALLIN:

				RequestAdminInfo(MMRI_AIL, LatestAccountRecordFromServer.sdb.player_id, 0, 0, 0);

				return TRUE;



			case IDC_REQINFO_CHARGEBACK:

				RequestAdminInfo(MMRI_CHARGEBACK, LatestAccountRecordFromServer.sdb.player_id, 0, 0, 0);

				return TRUE;

			

			case IDC_REQINFO_PLAYERINFO:

				RequestAdminInfo(MMRI_PLAYERINFO, LatestAccountRecordFromServer.sdb.player_id, 0, 0, 0);

				return TRUE;



			case IDC_RESET_ALLINS:

				if (LatestAccountRecordFromServer.sdb.player_id) {

					WORD32 player_id = LatestAccountRecordFromServer.sdb.player_id;

					char str[300];

					zstruct(str);

					sprintf(str,"Send automatic confirmation letter to player\n"

								"%s after resetting their All-Ins?",

								LatestAccountRecordFromServer.sdb.user_id);

					int result = MessageBox(hDlg, str, "Reset player All-Ins",

							MB_YESNOCANCEL|MB_ICONQUESTION|MB_TOPMOST|MB_APPLMODAL);

					if (result==IDYES || result==IDNO) {

						// Do it.

						struct AccountRecord ar;

						zstruct(ar);

						ar.sdb.player_id = player_id;

						ar.usage = ACCOUNTRECORDUSAGE_RESET_ALLINS_FOR_PLAYERID;

						if (result==IDYES) {

							ar.flags |= ACRF_SEND_ALL_IN_RESET_EMAIL;

						}

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_USERID, 0);	// re-request it.

					}

				}

				return TRUE;	// We DID process this message

			case IDC_SEND_PASSWORD:

				if (LatestAccountRecordFromServer.sdb.player_id) {

					if (LatestAccountRecordFromServer.sdb.email_address[0]) {

						WORD32 player_id = LatestAccountRecordFromServer.sdb.player_id;

						char str[300];

						zstruct(str);

						sprintf(str,

							"This will send '%s' an email (to %s)\n"

							"with their password. It will also BCC answers with a copy of that email.\n"

							"\n"

							"Are you sure you want to do this?",

							LatestAccountRecordFromServer.sdb.user_id,

							LatestAccountRecordFromServer.sdb.email_address);

						int result = MessageBox(hDlg, str, "Send Password...",

								MB_YESNO|MB_ICONQUESTION|MB_TOPMOST|MB_APPLMODAL);

						if (result==IDYES) {

							// Do it.

							struct AccountRecord ar;

							zstruct(ar);

							ar.sdb.player_id = player_id;

							ar.usage = ACCOUNTRECORDUSAGE_SENDPASSWORD;

							SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						}

					} else {

						MessageBox(hDlg, "There's no email address to send the password!", "No email address..",

							MB_OK|MB_ICONWARNING|MB_TOPMOST|MB_APPLMODAL);

					}

				}

				return TRUE;	// We DID process this message

			case IDC_RETRIEVE_ECASH:

				strnncpy(temp_id,"Ecash",MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

			case IDC_RETRIEVE_RAKE:

				strnncpy(temp_id,"Rake",MAX_PLAYER_USERID_LEN);

				PlaySound("media\\CashReg.wav", NULL, SND_ASYNC | SND_NODEFAULT);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MARKETING:

				strnncpy(temp_id,"Marketing",MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

			case IDC_RETRIEVE_MISC:

				strnncpy(temp_id,"Misc.",MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

			case IDC_RETRIEVE_ECASH_FEE:

				strnncpy(temp_id,"EcashFee",MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

			case IDC_RETRIEVE_CHARGEBACK:

				strnncpy(temp_id,"ChargeBack",MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

			case IDC_DUPE1:

			case IDC_DUPE2:

			case IDC_DUPE3:

			case IDC_DUPE4:

			case IDC_DUPE5:

			case IDC_DUPE6:

			case IDC_DUPE7:

			case IDC_DUPE8:

				

				GetDlgItemText(hDlg, LOWORD(wParam), temp_id, MAX_PLAYER_USERID_LEN);

				goto send_retrieve_now;

send_retrieve_now:

				{

					struct AccountRecord ar;

					zstruct(ar);

					strnncpy(ar.sdb.user_id, temp_id, MAX_PLAYER_USERID_LEN);

					ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_USERID;

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

				}

				return TRUE;	// We DID process this message

			case IDC_RETRIEVE_PASTE:

				{

					#define MAX_TMP_SEARCH_STR 100	// needs room for <address> format

					char tmp_search[MAX_TMP_SEARCH_STR];

					PasteTextFromClipboard(tmp_search, MAX_TMP_SEARCH_STR);

					SetDlgItemText(hDlg, IDC_USERID1,tmp_search);

				}

				return TRUE;

			

			case IDC_RETRIEVE_USERID:	// Retrieve a record based on userid

				{

					struct AccountRecord ar;

					zstruct(ar);

					ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_USERID;

					GetWindowText(GetDlgItem(hDlg, IDC_USERID1), ar.sdb.user_id, MAX_PLAYER_USERID_LEN);

				  #if 0	//19991120MB: not implemented because we need to preserve spaces

					TrimCharsFromString(ar.sdb.user_id, " ", TRUE);

					TrimCharsFromString(ar.sdb.user_id, "'\t\"");

				  #endif



					//20000219MB: trim leading spaces and control characters...

					while (*ar.sdb.user_id > 0 && *ar.sdb.user_id <= ' ') {

						memmove(ar.sdb.user_id, ar.sdb.user_id+1, strlen(ar.sdb.user_id));	// shift left.

					}

					// trim trailing spaces...

					while (strlen(ar.sdb.user_id) && ar.sdb.user_id[strlen(ar.sdb.user_id)-1] == ' ') {

						ar.sdb.user_id[strlen(ar.sdb.user_id)-1] = 0;

					}

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

				}

				return TRUE;	// We DID process this message



			case IDC_RETRIEVE_PLAYERID:	// Retrieve a record based on player id (hex)

				{

					struct AccountRecord ar;

					zstruct(ar);

					ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

					if (AutoLookupPlayerID) {	// auto lookup request

						ar.sdb.player_id = AutoLookupPlayerID;

						AutoLookupPlayerID = 0;	// clear for next time

					} else {

						ar.sdb.player_id = GetDlgTextHex(hDlg, IDC_USERID1);

					}

					SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

				}

				return TRUE;	// We DID process this message



			case IDC_RETRIEVE_EMAIL:	// Retrieve a record based on email address

			case IDC_RETRIEVE_NAME:		// Retrieve a record based on name

				{

					char tmp_search[MAX_TMP_SEARCH_STR];

					zstruct(tmp_search);

					GetWindowText(GetDlgItem(hDlg, IDC_USERID1), tmp_search, MAX_TMP_SEARCH_STR);

					EmailListSearchType elst = ( (LOWORD(wParam) == IDC_RETRIEVE_EMAIL ? ELST_EMAIL : ELST_NAME));

					int count = FillEmailSearchStruct(tmp_search, elst);

					if (count == 1) {	// found just one, pop it up

						AutoLookupPlayerID = esr[0].player_id;

						PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_PLAYERID, 0);	// re-request it.

					} else {

						// pop up the box

						email_search_base_index = 0;	// make sure it's at the beginning

						if (!hEmailSearchDLG) {

							CreateDialog(hInst, MAKEINTRESOURCE(IDD_EMAIL_SEARCH), NULL, dlgEmailSearch);

						} else {

							SendMessage(hEmailSearchDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

							ReallySetForegroundWindow(hEmailSearchDLG);

						}

						// if it was zero, pop up a message

						if (!count) {

							char tmp[MAX_TMP_SEARCH_STR+40];

							zstruct(tmp);

							sprintf(tmp, "No matches found for ' %s  '", tmp_search);

							MessageBox(hDlg, tmp, "Nothing found...",

								MB_OK|MB_ICONWARNING|MB_TOPMOST|MB_APPLMODAL);

						}

					}

				}

				return TRUE;



			case IDC_REQUEST_HAND_HISTORY:

			case IDC_REQUEST_HAND_HISTORY2:

				// Request a hand history be sent to our current admin account

				{

					

					struct CardRoom_ReqHandHistory hhr;

					zstruct(hhr);

					hhr.hand_number = GetDlgTextInt(hDlg, IDC_HANDCOUNT);

					hhr.request_type = (BYTE8)(LOWORD(wParam)==IDC_REQUEST_HAND_HISTORY2 ? HHRT_LAST_N_ALLIN_HANDS : HHRT_LAST_N_HANDS);

					hhr.player_id = LatestAccountRecordFromServer.sdb.player_id;

					hhr.admin_flag = (BYTE8)(IsDlgButtonChecked(hDlg, IDC_HH_ADMIN_CHECK));

					SendDataStructure(DATATYPE_CARDROOM_REQ_HAND_HISTORY, &hhr, sizeof(hhr));

					

					MessageBox(hDlg,

							"You will receive an Email response to your request;\nThis may take a few minutes.",

							"Hand History Request Submitted...",

							MB_OK|MB_APPLMODAL|MB_TOPMOST);

					

				}

				return TRUE;	// We DID process this message

			case IDC_SAVE:

				// Save changes - send back to server.

				{

					struct AccountRecord ar;

					zstruct(ar);

					ar.usage = ACCOUNTRECORDUSAGE_MODIFY;

					FillCCLimitsForAccountRecord(&ar, LatestAccountRecordFromServer);

					FillAccountRecordFromDialogFields(hDlg, &ar);

					// make sure we didn't botch up the user id -- fix spaces, etc.

					// we will allow ourselves to create IDs 2 or bigger (or edit them)

					if (!ValidateString(ar.sdb.user_id,   2, MAX_PLAYER_USERID_LEN, "User ID")) {

						return TRUE;

					} else {

						SetDlgItemText(hDlg, IDC_USERID1, ar.sdb.user_id);

						SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

						PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_USERID, 0);	// re-request it.

					}

				  #if 0	//20000204MB

					MessageBox(hDlg,

							"Changes sent to server.  Hopefully they got there.",

							"Changes sent to server...",

							MB_OK|MB_APPLMODAL|MB_TOPMOST);

				  #endif

				}

				return TRUE;	// We DID process this message



			case IDC_CHKBOX_NO_CASHIER:	//Tony, Dec 21, 2001

				EnableWindow(GetDlgItem(hDlg, IDC_CHKBOX_USED_FIREPAY), !IsDlgButtonChecked(hDlg, IDC_CHKBOX_NO_CASHIER));	//Tony, Dec 21, 2001

				return TRUE;	//Tony, Dec 21, 2001



			case IDC_TRANSACT_HISTORY:

				ArToView = LatestAccountRecordFromServer;

				DialogBox(hInst, MAKEINTRESOURCE(IDD_VIEW_TRANSACTIONS), hDlg, dlgFuncViewTransactions);

				return TRUE;

			case IDC_CASHOUT_PLAYER_SPECIFIED:

			case IDC_CASHOUT_PLAYER_ENTIRE:

				{

					char str[300];

					char str2[50];

					int amount_to_cash_out = 0;

					if (LOWORD(wParam) == IDC_CASHOUT_PLAYER_SPECIFIED) {

						// we've been given an amount to cashout; read it from the edit control

						amount_to_cash_out = (int)floor(GetDlgTextFloat(hDlg, IDC_EDIT_CASHOUT) * 100.0 + .5);

					} else {

						amount_to_cash_out = LatestAccountRecordFromServer.sdb.real_in_bank;

					}

					sprintf(str,"Are you sure you want to cash out '%s' for %s?\n\n"

								"This amount will be applied towards the player's\n"

								"potential credit cards; the rest will be issued as a check.\n",

								LatestAccountRecordFromServer.sdb.user_id,

								CurrencyString(str2, amount_to_cash_out, CT_REAL));

					int result = MessageBox(hDlg, str, "Cash out player...",

							MB_OKCANCEL|MB_ICONHAND|MB_TOPMOST|MB_APPLMODAL);

					if (result==IDOK) {

						CCTransaction cct;

						zstruct(cct);

						cct.transaction_type = CCTRANSACTION_CASHOUT;

						sprintf(cct.amount, "%d", amount_to_cash_out);

						cct.player_id = LatestAccountRecordFromServer.sdb.player_id;

						// fluff: progress bar

						HWND hSubmitProgressDlg;

						hSubmitProgressDlg = NULL;

						hSubmitProgressDlg = CreateProgressWindow(hDlg,

							"Submitting Cash Out request to server...",

							60,	// start at 0%, go up to 60% while waiting

							2400);	// # of ms for indicator to get to that %

						if (hSubmitProgressDlg) {

							FinishProgressWindow(hSubmitProgressDlg, 200);

						}

						SendDataStructure(DATATYPE_CREDIT_CARD_TRANSACTION, &cct, sizeof(cct));

						MessageBox(hDlg, "Check monitor window and Transaction emails "

										 "to verify that the transaction went through.", 

										 "Cashed out player...",

										MB_OK|MB_TOPMOST|MB_APPLMODAL);

					}

				}

				return TRUE;

			case IDC_TRANSFER_MONEY:

				iTransferMoneyRealMoneyFlag = TRUE;

				dwTransferMoneySourcePlayerID = LatestAccountRecordFromServer.sdb.player_id;

				dwTransferMoneyDestPlayerID   = LatestAccountRecordFromServer.sdb.player_id;

				DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSFER_MONEY), hDlg, dlgFuncTransferMoney);

				PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_USERID, 0);	// re-request it.

				return TRUE;	// We DID process this message

			case IDC_TRANSFER_MONEY2:

				iTransferMoneyRealMoneyFlag = FALSE;

				dwTransferMoneySourcePlayerID = LatestAccountRecordFromServer.sdb.player_id;

				dwTransferMoneyDestPlayerID   = LatestAccountRecordFromServer.sdb.player_id;

				DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSFER_MONEY), hDlg, dlgFuncTransferMoney);

				PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_USERID, 0);	// re-request it.

				return TRUE;	// We DID process this message



			case IDC_SEATED_AT_1:

				GetDlgItemText(hDlg, IDC_SERIAL_NUM_S1, temp_id, 10);

				JoinTable(atoi(temp_id));

				return TRUE;

			

			case IDC_SEATED_AT_2:

				GetDlgItemText(hDlg, IDC_SERIAL_NUM_S2, temp_id, 10);

				JoinTable(atoi(temp_id));

				return TRUE;

			

			case IDC_USERID1:

				if (HIWORD(wParam)==CBN_SELCHANGE) {

					PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_USERID, 0);	// re-request it.

				}

				break;//return TRUE;



			}

		}

		break;



#if 0

	case WM_LBUTTONUP:

		// depending where we clicked, things may happen 

		kp(("CLICKED LEFT BUTTON\n"));

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

//			if (hDlg==GetActiveWindow() && PtInRect(&r, pt)) {

				HWND find_hwnd = ChildWindowFromPointEx(hDlg, pt, CWP_SKIPINVISIBLE);

				// what control did we click on?

				if (find_hwnd == GetDlgItem(hDlg, IDC_USERID1)) {

					kp(("CLICKED ON USERID1\n"));

				} else if (find_hwnd == GetDlgItem(hDlg, IDC_PLAYERID)) {

					kp(("CLICKED ON PLAYERID\n"));

				}

//			}

		}

		break;

#endif



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "AccountEdit", hDlg, NULL);

		CloseToolTipWindow(tooltiphwnd);	// Close our tooltip window

		tooltiphwnd = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		hAdminEditAccountDlg = NULL;

		return TRUE;	// We DID process this message

	case WM_SIZING:

		WinSnapWhileSizing(hDlg, message, wParam, lParam);

		break;

	case WM_MOVING:

		WinSnapWhileMoving(hDlg, message, wParam, lParam);

		break;

	case WM_SYSCOMMAND:	// system menu commands...

		{

			switch (wParam) {

			case IDM_INSERT_BLANK_COMPUTER_NUM:

				//kp(("%s(%d) IDM_INSERT_BLANK_COMPUTER_NUM selected.\n",_FL));

				if (MessageBox(hDlg,

						"This will insert a blank player id at the start\n"

						"of the computer serial number entry for their most\n"

						"recent computer, thereby allowing them to create\n"

						"one more new account.\n\n"

						"The user must create the new account BEFORE logging\n"

						"in to any of their other accounts.\n\n"

						"Do you wish to perform this action now?",

						"Allow user to create one more new account...",

						MB_OKCANCEL|MB_TOPMOST|MB_ICONQUESTION)==IDOK)

				{

					// Do it.

					struct MiscClientMessage mcm;

					zstruct(mcm);

					mcm.message_type = MISC_MESSAGE_INSERT_BLANK_PLAYERID;

					mcm.misc_data_1 = LatestAccountRecordFromServer.sdb.client_platform.computer_serial_num;

					SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				}

				return TRUE;	// We DID process this message



			case IDM_MOVE_TO_TOP_WAITLIST:

				//kp(("%s(%d) IDM_MOVE_TO_TOP_WAITLIST selected.\n",_FL));

				if (MessageBox(hDlg,

						"This will move the player to the top of any waiting\n"

						"lists they are already on.\n\n"

						"They MUST already be on the waiting list.\n\n"

						"Note that this might upset other players.\n\n"

						"Do you wish to perform this action now?",

						"Move player to top of waiting list...",

						MB_OKCANCEL|MB_TOPMOST|MB_ICONQUESTION)==IDOK)

				{

					// Do it.

					struct MiscClientMessage mcm;

					zstruct(mcm);

					mcm.message_type = MISC_MESSAGE_MOVE_TO_TOP_OF_WAITLISTS;

					mcm.misc_data_1 = LatestAccountRecordFromServer.sdb.player_id;

					SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				}

				return TRUE;	// We DID process this message

			}

		}

		break;

	case WM_VSCROLL:

	  #if 1 // below no longer used

		if ((HWND)lParam==GetDlgItem(hDlg, IDC_PLAYERID_SPIN)) {

			int nScrollCode = (int)LOWORD(wParam);

			short nPos = (short)HIWORD(wParam);

			pr(("%s(%d) nScrollCode = %d, new position = %d, old position = %d\n",

					_FL, nScrollCode, nPos, sOldPlayerIDSpinValue));

			if (nScrollCode==SB_THUMBPOSITION) {

				// Change the player id field and retrieve

				WORD32 player_id = GetDlgTextHex(hDlg, IDC_PLAYERID);

				// Determine if they scrolled up or down...

				player_id += (nPos - sOldPlayerIDSpinValue);

				sOldPlayerIDSpinValue = nPos;

				char str[20];

				sprintf(str, "0x%08lx", player_id);

				AutoLookupPlayerID = player_id;

				dwOldPlayerID = player_id;

				SetWindowText(GetDlgItem(hDlg, IDC_PLAYERID), str);

			}

			if (nScrollCode==SB_ENDSCROLL) {

				// Stopped moving... time to retrieve it.

				PostMessage(hDlg, WM_COMMAND, IDC_RETRIEVE_PLAYERID, 0);

			}

		}

	  #endif // above no longer used

		return TRUE;	// We DID process this message

	case WMP_NEW_ACCOUNT_RECORD:

		// A new account record arrived from the server... copy it to our

		// dialog box.

		FillEditDialogFields();

		return TRUE;	// We DID process this message

	}

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 1999/09/22 - MB

//

// Bring up the administrator's account editing dialog box.

//

void OpenAdminEditAccountDialog(void)

{

	if (!hAdminEditAccountDlg) {

		CreateDialog(hInst, MAKEINTRESOURCE(IDD_ACCOUNT_EDIT2), NULL, dlgFuncAdminEditAccount);

		if (AutoLookupPlayerID) {

			PostMessage(hAdminEditAccountDlg, WM_COMMAND, (WPARAM)IDC_RETRIEVE_PLAYERID, (LPARAM)0);

		}

	} else {

		if (AutoLookupPlayerID) {

			PostMessage(hAdminEditAccountDlg, WM_COMMAND, (WPARAM)IDC_RETRIEVE_PLAYERID, (LPARAM)0);

		}

	  #if 0	// 20000226HK

		ShowWindow(hAdminEditAccountDlg, SW_SHOWNORMAL); // only works for first call?

	  #else

		ShowWindow(hAdminEditAccountDlg, SW_RESTORE);

	  #endif

		ReallySetForegroundWindow(hAdminEditAccountDlg);

	}

}



WORD32 dlg_player_id;

int check_amount[TRANS_TO_RECORD_PER_PLAYER];

BYTE8 delivery_method[TRANS_TO_RECORD_PER_PLAYER];

#define MAX_TRACKINGNUMBER_LEN	20

char tracking_number[TRANS_TO_RECORD_PER_PLAYER][MAX_TRACKINGNUMBER_LEN]; // 20 = 8+11+null (see gamedata.h for ClientCheckTransaction)





/**********************************************************************************

 void SetDeliveryRadioButtons

 Date: 20000501HK

 Purpose: used by function below to select proper radio button

***********************************************************************************/

void SetDeliveryRadioButtons(HWND hDlg, BYTE8 delivery_method)

{

	// first we blank them all

	CheckDlgButton(hDlg, IDC_RADIO_NONE, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_DHL, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_TRANS_EXPRESS, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_FEDEX, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_CERTIFIED, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_REGISTERED, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_EXPRESS, FALSE);

	CheckDlgButton(hDlg, IDC_RADIO_PRIORITY, FALSE);

	// now find the right one to select

	switch (delivery_method) {

	case CDT_NONE:

		CheckDlgButton(hDlg, IDC_RADIO_NONE, TRUE);

		break;

	case CDT_DHL:

		CheckDlgButton(hDlg, IDC_RADIO_DHL, TRUE);

		break;

	case CDT_TRANS_EXPRESS:

		CheckDlgButton(hDlg, IDC_RADIO_TRANS_EXPRESS, TRUE);

		break;

	case CDT_FEDEX:

		CheckDlgButton(hDlg, IDC_RADIO_FEDEX, TRUE);

		break;

	case CDT_CERTIFIED:

		CheckDlgButton(hDlg, IDC_RADIO_CERTIFIED, TRUE);

		break;

	case CDT_REGISTERED:

		CheckDlgButton(hDlg, IDC_RADIO_REGISTERED, TRUE);

		break;

	case CDT_EXPRESS:

		CheckDlgButton(hDlg, IDC_RADIO_EXPRESS, TRUE);

		break;

	case CDT_PRIORITY:

		CheckDlgButton(hDlg, IDC_RADIO_PRIORITY, TRUE);

		break;

	}

}



/**********************************************************************************

 void SetDeliveryButtons

 Date: 20000501HK

 Purpose: used by function below to select proper radio button

***********************************************************************************/

void SetDeliveryButtons(HWND hDlg, BYTE8 delivery_method, int check_amount, char *tracking_number)

{

	// the applicable buttons depend on the current selection

	EnableWindow(GetDlgItem(hDlg, IDAPPLY_EMAIL), 

		check_amount && tracking_number[0] && delivery_method);

	EnableWindow(GetDlgItem(hDlg, IDAPPLY_NOEMAIL), TRUE);

	EnableWindow(GetDlgItem(hDlg, IDNOAPPLY_EMAIL), 

		check_amount && tracking_number[0] && delivery_method);

}



/**********************************************************************************

 BOOL CALLBACK dlgCheckTracking

 Date: 20000428HK

 Purpose: handler for dlg used for entering check tracking numbers

***********************************************************************************/

BOOL CALLBACK dlgCheckTracking(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	int i, index;

	ClientCheckTransaction *ct;

	char tmp[300];

	zstruct(tmp);

	HWND combo;

	switch (message) {

	case WM_INITDIALOG:

		for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

			check_amount[i] = 0;

			delivery_method[i] = 0;

			zstruct(tracking_number[i]);

		}

		zstruct(tmp);

		sprintf(tmp,"%s (%s)\n%s\n%s\n%s\n%s, %s, %s\n%s\n%s\n",

			LatestAccountRecordFromServer.sdb.user_id,

			LatestAccountRecordFromServer.sdb.email_address,

			LatestAccountRecordFromServer.sdb.full_name,

			LatestAccountRecordFromServer.sdb.mailing_address1,

			LatestAccountRecordFromServer.sdb.mailing_address2,

			LatestAccountRecordFromServer.sdb.city,

			LatestAccountRecordFromServer.sdb.mailing_address_state,

			LatestAccountRecordFromServer.sdb.mailing_address_country,

			LatestAccountRecordFromServer.sdb.mailing_address_postal_code,

			DecodePhoneNumber(LatestAccountRecordFromServer.sdb.phone_number));

		SetDlgItemText(hDlg, IDC_DELIVERY_DETAILS, tmp);

		AddKeyboardTranslateHwnd(hDlg);

		hCheckTrackingDLG = hDlg;

		CheckDlgButton(hDlg, IDC_RADIO_DHL, TRUE);

		WinRestoreWindowPos(ProgramRegistryPrefix, "CheckTrackingDialog", hDlg, NULL, NULL, FALSE, TRUE);

		WinPosWindowOnScreen(hDlg);

		ShowWindow(hDlg, SW_SHOW);

		PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		return TRUE;

	case WMP_UPDATE_PLAYER_INFO:

		{

			// Fill in the fields for the combo box

			combo = GetDlgItem(hDlg, IDC_COMBO_CHECKNUM);

			SendMessage(combo, CB_RESETCONTENT, 0, 0);

			index = 0;

			dlg_player_id = LatestAccountRecordFromServer.sdb.player_id;

			for (i=0; i <TRANS_TO_RECORD_PER_PLAYER; i++) {

				ct = (ClientCheckTransaction *)(&LatestAccountRecordFromServer.sdb.transaction[i]);

				if (ct->transaction_type == CTT_CHECK_ISSUED) {

					sprintf(tmp, "%d", ct->ecash_id);

					SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)tmp);

					zstruct(check_amount[index]);

					check_amount[index] = ct->transaction_amount;

					delivery_method[index] = ct->delivery_method;

					// !!! fix this when we upgrade the database records

					// for now, we're squeezing a string into parts using

					// what's left of the structure... 8 characters + 10 gives

					// us 18 chars to work with (no null!)

					zstruct(tracking_number[index]);	

					strncpy(tracking_number[index], ct->first_eight, 8);

					strncpy(tracking_number[index]+8, ct->last_ten, 10);

					tracking_number[index][MAX_TRACKINGNUMBER_LEN-1] = 0;

					index++;

				}

			}

			// add the "none" entry

			SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)"(none)");

			// set selection to top of list

			SendMessage(combo, CB_SETCURSEL, (WPARAM)0, 0);

			SetDlgItemText(hDlg, IDC_EDIT_DELIVERY_AMOUNT, CurrencyString(tmp, check_amount[0], CT_REAL, TRUE));

			SetDlgItemText(hDlg, IDC_EDIT_TRACKING_NUMBER, tracking_number[0]);

			SetDeliveryRadioButtons(hDlg, delivery_method[0]);

			SetDeliveryButtons(hDlg, delivery_method[0], check_amount[0], tracking_number[0]);

		}

		return TRUE;



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "CheckTrackingDialog", hDlg, NULL);

		hCheckTrackingDLG = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		

		case IDC_COMBO_CHECKNUM:

			if (HIWORD(wParam)==CBN_SELCHANGE) {

				// selection has changed...

				i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

				SetDlgItemText(hDlg, IDC_EDIT_DELIVERY_AMOUNT, CurrencyString(tmp, check_amount[i], CT_REAL, TRUE));

				SetDlgItemText(hDlg, IDC_EDIT_TRACKING_NUMBER, tracking_number[i]);

				SetDeliveryRadioButtons(hDlg, delivery_method[i]);

				SetDeliveryButtons(hDlg, delivery_method[i], check_amount[i], tracking_number[i]);



			}

			return TRUE;	// We DID process this message.

		case IDC_RADIO_NONE:

		case IDC_RADIO_TRANS_EXPRESS:

		case IDC_RADIO_DHL:

		case IDC_RADIO_FEDEX:

		case IDC_RADIO_CERTIFIED:

		case IDC_RADIO_REGISTERED:

		case IDC_RADIO_EXPRESS:

		case IDC_RADIO_PRIORITY:

			i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

			GetDlgItemText(hDlg, IDC_EDIT_TRACKING_NUMBER, tmp, 20);

			// we'll pull the tentative entry in the tracking number

			// to correctly set the buttons

			SetDeliveryButtons(hDlg, (BYTE8)(LOWORD(wParam) != IDC_RADIO_NONE),

				check_amount[i], tmp);

			break;



		case IDC_EDIT_TRACKING_NUMBER:

			GetDlgItemText(hDlg, IDC_EDIT_TRACKING_NUMBER, tmp, 20);

			// we'll pull the current stuff to correctly set the buttons

			SetDeliveryButtons(hDlg, (BYTE8)(!IsDlgButtonChecked(hDlg, IDC_RADIO_NONE)),

				TRUE, tmp);

			break;

			

		case IDAPPLY_EMAIL:

		case IDAPPLY_NOEMAIL:

		case IDNOAPPLY_EMAIL:

			combo = GetDlgItem(hDlg, IDC_COMBO_CHECKNUM);

			i = SendMessage(combo, CB_GETCURSEL, 0, 0);

			struct MiscClientMessage mcm;

			zstruct(mcm);

			mcm.message_type = MISC_MESSAGE_SET_CHECK_TRACKING_INFO;

			// we will use the table ID field for the player ID

			mcm.table_serial_number = dlg_player_id;

			// misc_data_1 is the check #

			SendMessage(combo, CB_GETLBTEXT, i, (LPARAM) (LPCSTR) tmp);

			mcm.misc_data_1 = atoi(tmp);

			// misc_data_2 is the check amount

			GetDlgItemText(hDlg, IDC_EDIT_DELIVERY_AMOUNT, tmp, 20);

			mcm.misc_data_2 = (WORD32)(StringToChips(tmp));

			// msg will hold the tracking number

			GetDlgItemText(hDlg, IDC_EDIT_TRACKING_NUMBER, tmp, 20);

			strnncpy(mcm.msg, tmp, MAX_TRACKINGNUMBER_LEN);

			// misc_data_3 holds which button was pushed

			// 0 = apply&email, 1 = apply/no email, 2 = only email

			switch (LOWORD(wParam)) {

			case IDAPPLY_EMAIL:

				mcm.misc_data_3 = 0;

				break;

			case IDAPPLY_NOEMAIL:

				mcm.misc_data_3 = 1;

				break;

			case IDNOAPPLY_EMAIL:

				mcm.misc_data_3 = 2;

				break;

			default:

				Error(ERR_ERROR, "%s(%d) Internal Error (impossible - see src)", _FL);

			}

			// display_flags will hold the courier type

			if (IsDlgButtonChecked(hDlg, IDC_RADIO_DHL)) {

				mcm.display_flags = CDT_DHL;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_TRANS_EXPRESS)) {

				mcm.display_flags = CDT_TRANS_EXPRESS;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_FEDEX)) {

				mcm.display_flags = CDT_FEDEX;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_CERTIFIED)) {

				mcm.display_flags = CDT_CERTIFIED;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_REGISTERED)) {

				mcm.display_flags = CDT_REGISTERED;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_EXPRESS)) {

				mcm.display_flags = CDT_EXPRESS;

			} else if (IsDlgButtonChecked(hDlg, IDC_RADIO_PRIORITY)) {

				mcm.display_flags = CDT_PRIORITY;

			} else {

				mcm.display_flags = CDT_NONE;

			}

			// structure is built; ship it

			SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

			// request update from server

			struct AccountRecord ar;

			zstruct(ar);

			ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

			ar.sdb.player_id = dlg_player_id;

			SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

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



int cred_amount[TRANS_TO_RECORD_PER_PLAYER];

int trans_amount[TRANS_TO_RECORD_PER_PLAYER];

int checkboxes[TRANS_TO_RECORD_PER_PLAYER];

int ecash_id[TRANS_TO_RECORD_PER_PLAYER];

WORD32 cc_num[TRANS_TO_RECORD_PER_PLAYER];

int last_index = 0;



/**********************************************************************************

 Function GetIndexFromCombo(int ecash_id)

 Date: HK00/10/13

 Purpose: used below to go from the scroll selector to an index (for arrays above)

***********************************************************************************/

int GetIndexFromCombo(HWND combo)

{



	ClientTransaction *ct;

	int i = SendMessage(combo, CB_GETCURSEL, 0, 0);

	char tmp[20];

	zstruct(tmp);

	SendMessage(combo, CB_GETLBTEXT, i, (LPARAM) (LPCSTR) tmp);

	WORD32 our_ecash_id = atoi(tmp);

	// now find it

	for (int index=0; index < TRANS_TO_RECORD_PER_PLAYER; index++) {

		ct = &LatestAccountRecordFromServer.sdb.transaction[index];

		if (ct->transaction_type == CTT_PURCHASE && ct->ecash_id == our_ecash_id) {

			return index;

		}

	}

	// didn't find it, just return 0

	kp(("%s(%d) We didn't find a match for %d\n", our_ecash_id));

	return 0;

}





/**********************************************************************************

 Function CALLBACK dlgEditCreditable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

 Date: HK00/02/07

 Purpose: handler for dlg used to set amounts creditable back on transactions

 NOTE: for disabling cashouts against a certain transaction, we add the amount of the

	   original purchase to the creditable amount... this lets the server know to ignore

	   it and lets us know how to display it

***********************************************************************************/

BOOL CALLBACK dlgEditCreditable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	int i, index;

	ClientTransaction *ct;

	struct AccountRecord ar;

	char tmp[20];

	HWND combo;

	WORD32 cc_num_to_match;

	int disabling_transactions;

	switch (message) {

	case WM_INITDIALOG:

		AddKeyboardTranslateHwnd(hDlg);

		hEditCreditableDLG = hDlg;

		WinRestoreWindowPos(ProgramRegistryPrefix, "CreditableDialog", hDlg, NULL, NULL, FALSE, TRUE);

		WinPosWindowOnScreen(hDlg);

		ShowWindow(hDlg, SW_SHOW);

		PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

		return TRUE;



	case WMP_UPDATE_PLAYER_INFO:

		// Fill in the fields for the combo box

		combo = GetDlgItem(hDlg, IDC_COMBO_TRANSACTIONS);

	    SendMessage(combo, CB_RESETCONTENT, 0, 0);

		//index = 0;

		dlg_player_id = LatestAccountRecordFromServer.sdb.player_id;

		zstruct(cred_amount);

		zstruct(trans_amount);

		zstruct(checkboxes);

		zstruct(cc_num);

		zstruct(ecash_id);

//		for (i=0; i <TRANS_TO_RECORD_PER_PLAYER; i++) {

		for (index=0; index < TRANS_TO_RECORD_PER_PLAYER; index++) {

			ct = &LatestAccountRecordFromServer.sdb.transaction[index];

			if (ct->transaction_type == CTT_PURCHASE) {

		        sprintf(tmp, "%d", ct->ecash_id);

				ecash_id[index] = ct->ecash_id;

				cred_amount[index] = ct->credit_left;

				trans_amount[index] = ct->transaction_amount;

				cc_num[index] = ct->partial_cc_number;

				if (cred_amount[index] > trans_amount[index]) {

					cred_amount[index] -= trans_amount[index];

					checkboxes[index] = TRUE;

				}

				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)tmp);

				//index++;

			}

		}

		// set selection to last referenced item

		ct = &LatestAccountRecordFromServer.sdb.transaction[last_index];

		SendMessage(combo, CB_SETCURSEL, (WPARAM)last_index, 0);

		index = GetIndexFromCombo(combo);

		SetDlgItemText(hDlg, IDC_CREDITABLE_EDIT, CurrencyString(tmp, cred_amount[index], CT_REAL, TRUE));

		CheckDlgButton(hDlg, IDC_CHECK_DISABLE, checkboxes[index]);

		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DISABLE), cred_amount[index]);

		// window positioning

		return TRUE;



	case WM_DESTROY:

		WinStoreWindowPos(ProgramRegistryPrefix, "CreditableDialog", hDlg, NULL);

		hEditCreditableDLG = NULL;

		RemoveKeyboardTranslateHwnd(hDlg);

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		

		case IDC_COMBO_TRANSACTIONS:

			if (HIWORD(wParam)==CBN_SELCHANGE) {

				// selection has changed...

				i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

				last_index = i;

				combo = GetDlgItem(hDlg, IDC_COMBO_TRANSACTIONS);

				index = GetIndexFromCombo(combo);

				int cred_amt;

				if (cred_amount[index] > trans_amount[index]) {

					cred_amt = cred_amount[index] - trans_amount[index];

				} else {

					cred_amt = cred_amount[index];

				}

				SetDlgItemText(hDlg, IDC_CREDITABLE_EDIT, CurrencyString(tmp, cred_amt, CT_REAL, TRUE));

				CheckDlgButton(hDlg, IDC_CHECK_DISABLE, checkboxes[index]);

				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DISABLE), cred_amount[index]);

			}

			return TRUE;	// We DID process this message.



		case IDC_CHECK_DISABLE_ALL:	// enable/disable all for one specific c/c number

		case IDC_CHECK_ENABLE_ALL:

			combo = GetDlgItem(hDlg, IDC_COMBO_TRANSACTIONS);

			index = GetIndexFromCombo(combo);

			disabling_transactions = (LOWORD(wParam) == IDC_CHECK_DISABLE_ALL);

			cc_num_to_match = cc_num[index];

			for (index=0; index < TRANS_TO_RECORD_PER_PLAYER; index++) {

				// check for a match, deal with as needed

				int server_needs_updating = FALSE;

				ct = &LatestAccountRecordFromServer.sdb.transaction[index];

				if (ct->transaction_type == CTT_PURCHASE && 

					ct->partial_cc_number == cc_num_to_match &&

					ct->credit_left)

				{

					if (disabling_transactions) {

						// check if it's enabled at this moment

						if (!checkboxes[index]) {

							// it's valid to credit -- disable it

							cred_amount[index] += trans_amount[index];

							checkboxes[index] = TRUE;

							server_needs_updating = TRUE;

						} 

					} else {	// we're enabling transactions

						// check if it's disabled at the moment

						if (checkboxes[index]) {

							// it's invalid to credit -- enable it

//							cred_amount[index] -= trans_amount[index];

							checkboxes[index] = FALSE;

							server_needs_updating = TRUE;

						}

					}

					// ship the change to the server if needed

					if (server_needs_updating) {

						struct MiscClientMessage mcm;

						zstruct(mcm);

						mcm.message_type = MISC_MESSAGE_SET_TRANS_CREDIT;

						mcm.table_serial_number = dlg_player_id;

						mcm.misc_data_1 = ecash_id[index];

						mcm.misc_data_2 = cred_amount[index];

						SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

					}

				}

			}

			// request update from server

			zstruct(ar);

			ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

			ar.sdb.player_id = dlg_player_id;

			SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

			return TRUE;	// We DID process this message.

			

		case IDC_CHECK_DISABLE:	// was just clicked, add/subtract as needed

		case IDOK:

			combo = GetDlgItem(hDlg, IDC_COMBO_TRANSACTIONS);

			i = SendMessage(combo, CB_GETCURSEL, 0, 0);

			index = GetIndexFromCombo(combo);

			struct MiscClientMessage mcm;

			zstruct(mcm);

			mcm.message_type = MISC_MESSAGE_SET_TRANS_CREDIT;

			// put the player ID where we put the table ID

			mcm.table_serial_number = dlg_player_id;

			SendMessage(combo, CB_GETLBTEXT, i, (LPARAM) (LPCSTR) tmp);

			mcm.misc_data_1 = atoi(tmp);

			GetDlgItemText(hDlg, IDC_CREDITABLE_EDIT, tmp, 20);

			//mcm.misc_data_2 = (WORD32)( (atof(tmp) * 100.0) );

			mcm.misc_data_2 = StringToChips(tmp);

			// it was the checkbox ticked... adjust creditable as needed

			checkboxes[index] = FALSE;

			if (mcm.misc_data_2 && IsDlgButtonChecked(hDlg, IDC_CHECK_DISABLE)) {

				// disable it by adding the transaction amount to creditable

				mcm.misc_data_2 += trans_amount[index];

				checkboxes[index] = TRUE;

			}

			SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

			// request update from server

			zstruct(ar);

			ar.usage = ACCOUNTRECORDUSAGE_LOOKUP_PLAYERID;

			ar.sdb.player_id = dlg_player_id;

			SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar, sizeof(ar));

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



/**********************************************************************************

 Function FillCCLimitsForAccountRecord(AccountRecord *ar, AccountRecord arIn)

 Date: HK00/03/31

 Purpose: fill an account record with CC Limit info

***********************************************************************************/

void FillCCLimitsForAccountRecord(AccountRecord *ar, AccountRecord arIn)

{

	ar->sdb.flags &= ~SDBRECORD_FLAG_HIGH_CC_LIMIT;

	if (arIn.sdb.flags & SDBRECORD_FLAG_HIGH_CC_LIMIT) {

		ar->sdb.flags |= SDBRECORD_FLAG_HIGH_CC_LIMIT;

	}

	ar->sdb.cc_override_limit1 = arIn.sdb.cc_override_limit1;

	ar->sdb.cc_override_limit2 = arIn.sdb.cc_override_limit2;

	ar->sdb.cc_override_limit3 = arIn.sdb.cc_override_limit3;

}



/**********************************************************************************

 Function NetPurchasedInLastNHours(WORD32 player_id, int hours, int *total_purchases)

 Date: HK00/07/13

 Purpose: figure out how much player has purchased in last n hours (used in CCLimits)

***********************************************************************************/

int NetPurchasedInLastNHours(AccountRecord *ar, int hours, int *total_purchases)

{

	if (!ar) {	// no account record?

		return 0;

	}

	time_t now = time(NULL);

	int total_charged = 0;

	int _total_purchases = 0;

	int valid_tr_index = -1, i;

	// first step -- just find the last relevant transaction

	for (i=0; i < TRANS_TO_RECORD_PER_PLAYER; i++) {

		ClientTransaction *ct = &ar->sdb.transaction[i];

		if (!ct->transaction_type) {

			continue;

		}

		if (difftime(now, ct->timestamp) < (int)(hours*3600)) {	// fits the time range

			valid_tr_index = i;	// set so we can count from here

		}

	}

	// now we do it for real, going backwards from this point towards the present

	for (i=valid_tr_index; i >= 0 ; i--) {

		ClientTransaction *ct = &ar->sdb.transaction[i];

		if (!ct->transaction_type) {

			continue;

		}

		if (ct->transaction_type == CTT_PURCHASE ) { // count it (+)

			total_charged += ct->transaction_amount;

			_total_purchases += ct->transaction_amount;

		}

		if (ct->transaction_type == CTT_FIREPAY_PURCHASE ) { // count it (+)

			total_charged += ct->transaction_amount;

			_total_purchases += ct->transaction_amount;

		}

		if (ct->transaction_type == CTT_CC_PURCHASE ) { // count it (+)

			total_charged += ct->transaction_amount;

			_total_purchases += ct->transaction_amount;

		}

		if (ct->transaction_type == CTT_CREDIT) {	// count it (-)

			total_charged -= ct->transaction_amount;

		}

		// don't ever go negative!

		total_charged = max(total_charged, 0);

	}

	if (total_purchases) {

		*total_purchases = _total_purchases;

	}

	return max(0,total_charged);

}



/**********************************************************************************

 Function RedrawCCLimitsDLG

 Date: HK00/03/23

 Purpose: used in dlgSetCCLimits

***********************************************************************************/

void RedrawCCLimitsDLG(HWND hDlg, AccountRecord *ar)

{

	#define LIMIT_EDIT_BOX 20

	char tmp[LIMIT_EDIT_BOX];

	int higher_limits;	// T/F

	int t_purchases = 0, t_limit = 0;

	higher_limits = ar->sdb.flags & SDBRECORD_FLAG_HIGH_CC_LIMIT;

	CheckDlgButton(hDlg, IDC_RADIO_LOWER_LIMITS, !higher_limits);

	CheckDlgButton(hDlg, IDC_RADIO_HIGHER_LIMITS, higher_limits);

	if (ar->sdb.cc_override_limit1 || IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT1)) {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT1, TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT1), TRUE);

		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT1), tmp, LIMIT_EDIT_BOX-1);

		if (ar->sdb.cc_override_limit1) {

			sprintf(tmp, "%d", ar->sdb.cc_override_limit1);

		}

	} else {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT1, FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT1), FALSE);

		sprintf(tmp, "%d",(higher_limits ? AdminStats.CCLimit1Amount2 : AdminStats.CCLimit1Amount));

	}

	SetDlgItemText(hDlg, IDC_EDIT_LIMIT1, tmp);

	t_limit = atoi(tmp);

	t_purchases = NetPurchasedInLastNHours(ar, 24, NULL) / 100;

	sprintf(tmp, "%d", t_purchases);

	SetDlgItemText(hDlg, IDC_PURCH_1, tmp);

	sprintf(tmp, "%d", t_limit-t_purchases);

	SetDlgItemText(hDlg, IDC_AVAIL_1, tmp);



	if (ar->sdb.cc_override_limit2 || IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT2)) {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT2, TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT2), TRUE);

		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT2), tmp, LIMIT_EDIT_BOX-1);

		if (ar->sdb.cc_override_limit2) {

			sprintf(tmp, "%d", ar->sdb.cc_override_limit2);

		}

	} else {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT2, FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT2), FALSE);

		sprintf(tmp, "%d",(higher_limits ? AdminStats.CCLimit2Amount2 : AdminStats.CCLimit2Amount));

	}

	SetDlgItemText(hDlg, IDC_EDIT_LIMIT2, tmp);

	t_limit = atoi(tmp);

	t_purchases = NetPurchasedInLastNHours(ar, 24*7, NULL) / 100;

	sprintf(tmp, "%d", t_purchases);

	SetDlgItemText(hDlg, IDC_PURCH_2, tmp);

	sprintf(tmp, "%d", t_limit-t_purchases);

	SetDlgItemText(hDlg, IDC_AVAIL_2, tmp);



	if (ar->sdb.cc_override_limit3 || IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT3)) {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT3, TRUE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT3), TRUE);

		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT3), tmp, LIMIT_EDIT_BOX-1);

		if (ar->sdb.cc_override_limit3) {

			sprintf(tmp, "%d", ar->sdb.cc_override_limit3);

		}

	} else {

		CheckDlgButton(hDlg, IDC_CHECK_LIMIT3, FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_LIMIT3), FALSE);

		sprintf(tmp, "%d",(higher_limits ? AdminStats.CCLimit3Amount2 : AdminStats.CCLimit3Amount));

	}

	SetDlgItemText(hDlg, IDC_EDIT_LIMIT3, tmp);

	t_limit = atoi(tmp);

	t_purchases = NetPurchasedInLastNHours(ar, 24*30, NULL) / 100;

	sprintf(tmp, "%d", t_purchases);

	SetDlgItemText(hDlg, IDC_PURCH_3, tmp);

	sprintf(tmp, "%d", t_limit-t_purchases);

	SetDlgItemText(hDlg, IDC_AVAIL_3, tmp);

}						

	

/**********************************************************************************

 Function dlgSetCCLimits

 Date: HK00/03/23

 Purpose: set credit card purchase limits for this player

***********************************************************************************/

BOOL CALLBACK dlgSetCCLimits(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	AccountRecord *ar;

	switch (message) {

	case WM_INITDIALOG: 

		{	

			char tmp[100];

			ar = &LatestAccountRecordFromServer;

			zstruct(tmp);

			sprintf(tmp, "Set C/C Limits for %s", ar->sdb.user_id);

			SetWindowText(hDlg, tmp);

			AddKeyboardTranslateHwnd(hDlg);

			RedrawCCLimitsDLG(hDlg, ar);

		}

	return FALSE;

	case WM_COMMAND:

		ar = &LatestAccountRecordFromServer;

		switch (LOWORD(wParam)) {

		case IDC_RADIO_LOWER_LIMITS:

			ar->sdb.flags &= ~SDBRECORD_FLAG_HIGH_CC_LIMIT;

			RedrawCCLimitsDLG(hDlg, ar);

			return TRUE;

		case IDC_RADIO_HIGHER_LIMITS:

			ar->sdb.flags |= SDBRECORD_FLAG_HIGH_CC_LIMIT;

			RedrawCCLimitsDLG(hDlg, ar);

			return TRUE;

		case IDC_CHECK_LIMIT1:

			if (!IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT1)) {

				ar->sdb.cc_override_limit1 = 0;

			}

			RedrawCCLimitsDLG(hDlg, ar);

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_LIMIT1), EM_SETSEL,(WPARAM)0,(LPARAM)-1);

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_LIMIT1));

			return TRUE;

		case IDC_CHECK_LIMIT2:

			if (!IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT2)) {

				ar->sdb.cc_override_limit2 = 0;

			}

			RedrawCCLimitsDLG(hDlg, ar);

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_LIMIT2), EM_SETSEL,(WPARAM)0,(LPARAM)-1);

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_LIMIT2));

			return TRUE;

		case IDC_CHECK_LIMIT3:

			if (!IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT3)) {

				ar->sdb.cc_override_limit3 = 0;

			}

			RedrawCCLimitsDLG(hDlg, ar);

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_LIMIT3), EM_SETSEL,(WPARAM)0,(LPARAM)-1);

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_LIMIT3));

			return TRUE;

		case IDOK:

			{

				// save it

				HWND hParent = GetParent(hDlg);

				struct AccountRecord ar2;

				zstruct(ar2);

				ar2.usage = ACCOUNTRECORDUSAGE_MODIFY;

				FillAccountRecordFromDialogFields(hParent, &ar2);

				// overrride our changes

				char tmp[20];

				zstruct(tmp);

				if (IsDlgButtonChecked(hDlg, IDC_RADIO_LOWER_LIMITS)) {

					ar2.sdb.flags &= ~SDBRECORD_FLAG_HIGH_CC_LIMIT;

				}

				if (IsDlgButtonChecked(hDlg, IDC_RADIO_HIGHER_LIMITS)) {

					ar2.sdb.flags |= SDBRECORD_FLAG_HIGH_CC_LIMIT;

				}

				ar2.sdb.cc_override_limit1 = 0;

				if (IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT1)) {

					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT1), tmp, 10);

					ar2.sdb.cc_override_limit1 = (WORD16)atoi(tmp);

				}

				ar2.sdb.cc_override_limit2 = 0;

				if (IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT2)) {

					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT2), tmp, 10);

					ar2.sdb.cc_override_limit2 = (WORD16)atoi(tmp);

				}

				ar2.sdb.cc_override_limit3 = 0;

				if (IsDlgButtonChecked(hDlg, IDC_CHECK_LIMIT3)) {

					GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LIMIT3), tmp, 10);

					ar2.sdb.cc_override_limit3 = (WORD16)atoi(tmp);

				}

				SendDataStructure(DATATYPE_ACCOUNT_RECORD, &ar2, sizeof(ar2));

				AutoLookupPlayerID = ar2.sdb.player_id;

				PostMessage(hParent, WM_COMMAND, IDC_RETRIEVE_PLAYERID, 0);	// re-request it.

				EndDialog(hDlg, IDOK);

			}

			return TRUE;

		case IDCANCEL:

			EndDialog(hDlg, IDCANCEL);

			return TRUE;

		}

		return TRUE;	// We DID process this message.



	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		return TRUE;	// We DID process this message

	}

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



//*********************************************************

// 2000/08/18 - MB

//

// Daily stat predictor: update a single float

// Try not to overwrite with bad data.

// Note: these functions are NOT thread-safe.  They must be called

// from exactly one thread.

//

#define PREVIOUS_PRED_DATA_WEIGHTING	(.8)

#define PRED_SAMPLES_PER_DAY			(24*3600 / PREDICTION_DATA_INTERVAL)

#define PRED_FILE_LEN (PRED_SAMPLES_PER_DAY * sizeof(struct Pred_Data))



struct Pred_Data *Pred_DataArray;



struct Pred_Data Pred_PreviousCurrentData;	// saved data from before current was updated.

struct Pred_Data Pred_PreviousCurrentData_Delta;	// saved delta (change) in data for one interval

int    Pred_PreviousCurrentData_Interval;	// interval # when above struct saved.



int Pred_DataArray_FileNumber;	// currently loaded file number (if anything is loaded)



static void Pred_UpdateEntry(float *dest, float new_data, int do_trivial_filtering)

{

	if (*dest <= 0.5) {

		// old data is essentially zero... completely replace with new data

		*dest = new_data;

		return;

	}

	if (do_trivial_filtering) {

		if (new_data < (*dest / 3.0)) {

			return;	// too low... throw out new data

		}

		if (new_data > (*dest * 1.7)) {

			return;	// too high... throw out new data

		}

	}

	*dest = (float)(*dest * PREVIOUS_PRED_DATA_WEIGHTING + new_data * (1.0 - PREVIOUS_PRED_DATA_WEIGHTING));

}



//*********************************************************

// 2000/08/18 - MB

//

// Daily stat predictor: log new data

// Note: these functions are NOT thread-safe.  They must be called

// from exactly one thread.

//

void Pred_LogNewStats(time_t src_data_time,

		int players,

		int rake_per_hour,

		int rake_today,

		int games_today,

		int gross_cc_today,

		int new_accounts_today,

		int *output_seconds_into_interval,

		int *output_current_interval)

{

	*output_seconds_into_interval = 0;	// initialize

	static time_t last_data_update;

	static int interval;				// current interval

	if (src_data_time - last_data_update < 60) {	// too soon since last?

		// estimate how far into the interval we are.  This may not be

		// accurate when first starting out, but it will settle down as soon

		// as the samples start being made at the start of each interval.

		*output_seconds_into_interval = src_data_time - last_data_update;

		*output_current_interval = interval;	// same as last time.

		return;	// ignore the new data.

	}



	static time_t last_data_interval;

	time_t server_time_less_gmt = src_data_time - SERVER_TIMEZONE;

	struct tm tm;

	gmtime(&server_time_less_gmt, &tm);

	int seconds_into_day = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

	interval = seconds_into_day / PREDICTION_DATA_INTERVAL;

	int start_of_interval = interval * PREDICTION_DATA_INTERVAL;

	*output_seconds_into_interval = seconds_into_day - start_of_interval;

	*output_current_interval = interval;

	//kp(("%s(%d) interval = %d\n", _FL, interval));



	if (interval==last_data_interval) {	// same interval as last time?

		return;	// ignore the new data.

	}



	// We want to consider updating the data.

	last_data_update = src_data_time;

	last_data_interval = interval;



	static int file_numbers[7] = {0,1,1,1,1,5,6};

	int file_number = file_numbers[tm.tm_wday];

	char fname[MAX_FNAME_LEN];

	zstruct(fname);

	sprintf(fname, "data-%d.bin", file_number);



	// If the file number has changed, free the old data array.

	if (Pred_DataArray && file_number != Pred_DataArray_FileNumber) {

		free(Pred_DataArray);

		zstruct(Pred_PreviousCurrentData);

		Pred_PreviousCurrentData_Interval = -1;

		Pred_DataArray = NULL;

	}

	if (!Pred_DataArray) {

		// Time to load up a new array

		Pred_DataArray = (struct Pred_Data *)malloc(PRED_FILE_LEN);

		if (!Pred_DataArray) {

			return;	// FAILED! Do nothing.

		}

		memset(Pred_DataArray, 0, PRED_FILE_LEN);

		long bytes_read = 0;

		ReadFile(fname, Pred_DataArray, PRED_FILE_LEN, &bytes_read);

		zstruct(Pred_PreviousCurrentData);

		Pred_PreviousCurrentData_Interval = -1;

	}

	Pred_DataArray_FileNumber = file_number;



	// Update the data in the array.  Try to keep bad data out of the file.

	struct Pred_Data *pd = Pred_DataArray + interval;

	if (interval==0) {

		// always reset some variables.  First interval should always start at zero.

		rake_today = 0;

		gross_cc_today = 0;

		games_today = 0;

		pd->rake_today = (float)0.0;

		pd->games_today = (float)0.0;

		pd->gross_cc_today = (float)0.0;

	}

	if (rake_today < 0) {		// someone xfer out of the rake account?

		rake_today += 1000000000;	// xfer's are done 10M at a time.

	}

	Pred_PreviousCurrentData = *pd;	// save the un-updated version

	Pred_PreviousCurrentData_Interval = interval;



	//kp(("%s %s(%d) GetFileAge(%s) = %d seconds\n", TimeStr(), _FL, fname, GetFileAge(fname)));

	if (SecondCounter >= PREDICTION_DATA_INTERVAL || GetFileAge(fname) >= PREDICTION_DATA_INTERVAL) {	// only update if we haven't just restarted

		Pred_UpdateEntry(&pd->players, 		 (float)players,		TRUE);

		Pred_UpdateEntry(&pd->rake_per_hour, (float)rake_per_hour,	TRUE);

		Pred_UpdateEntry(&pd->rake_today,	 (float)rake_today,		FALSE);

		Pred_UpdateEntry(&pd->games_today,	 (float)games_today,	FALSE);

		Pred_UpdateEntry(&pd->gross_cc_today,(float)gross_cc_today,	FALSE);

		if ((float)new_accounts_today >= 0) {

			Pred_UpdateEntry(&pd->new_accounts_today,(float)new_accounts_today,	FALSE);

		}



		// Now that we're done, write it out so the disk file is always up to date.

		WriteFile(fname, Pred_DataArray, PRED_FILE_LEN);

	}



	// Keep track of how much each variable typically changes during this interval.

	zstruct(Pred_PreviousCurrentData_Delta);

	if (interval > 0) {

		struct Pred_Data *pd2 = Pred_DataArray + interval - 1;

		Pred_PreviousCurrentData_Delta.players = pd->players - pd2->players;

		Pred_PreviousCurrentData_Delta.rake_today = (float)max(0.0, pd->rake_today - pd2->rake_today);

		Pred_PreviousCurrentData_Delta.games_today = (float)max(0.0, pd->games_today - pd2->games_today);

		Pred_PreviousCurrentData_Delta.gross_cc_today = (float)max(0.0, pd->gross_cc_today - pd2->gross_cc_today);

		Pred_PreviousCurrentData_Delta.new_accounts_today = (float)max(0.0, pd->new_accounts_today - pd2->new_accounts_today);

	}



}



//*********************************************************

// 2000/08/19 - MB

//

// Retrieve some prediction related information.

//

void Pred_GetPredictionInfo(struct Pred_Data *normal_output,

		struct Pred_Data *peak_output,

		int *output_peak_players_interval,

		float *output_min_players,

		int *output_min_players_interval,

		int seconds_into_interval)

{

	*normal_output = Pred_PreviousCurrentData;

	*output_peak_players_interval = 0;

	*output_min_players_interval = 0;

	float min_players = (float)0x7fffffff;	// start at max for a 32-bit int.

	*output_min_players = 0;

	// Search for the peak data...

	zstruct(*peak_output);

	if (Pred_DataArray) {

		// Loop through all data looking for the high...

		struct Pred_Data *p = Pred_DataArray;

		for (int i=0 ; i<PRED_SAMPLES_PER_DAY ; i++, p++) {

			if (p->players > peak_output->players) {

				peak_output->players = p->players;

				*output_peak_players_interval = i;

			}

			if (p->rake_per_hour > peak_output->rake_per_hour) {

				peak_output->rake_per_hour = p->rake_per_hour;

			}

			if (p->new_accounts_today < 0) {	// fix broken data?

				p->new_accounts_today = 0;

			}

			if (p->new_accounts_today > peak_output->new_accounts_today) {

				peak_output->new_accounts_today = p->new_accounts_today;

			}

			if (p->players < min_players) {

				min_players = p->players;

				*output_min_players_interval = i;

				*output_min_players = min_players;

			}

		}



		// Rake total always comes from the last non-zero entry, however that does

		// not account for the last 6 minutes, so therefore use the 6 minute rate

		// and add it in as well.

		// (It's 6 minutes because the summary email goes out at 00:01:00 (not 00:00:00)).

		struct Pred_Data old_peak_to_use = Pred_DataArray[PRED_SAMPLES_PER_DAY-1];



		// is the last interval being updated now?

		if (Pred_PreviousCurrentData_Interval == (PRED_SAMPLES_PER_DAY-1)) {

			// yes, use the previous data for the current interval.

			old_peak_to_use = Pred_PreviousCurrentData;

		}



		float six_min_rake = old_peak_to_use.rake_per_hour / (3600 / PREDICTION_DATA_INTERVAL + 60);

		peak_output->rake_today = old_peak_to_use.rake_today + six_min_rake;

		peak_output->games_today = old_peak_to_use.games_today;

		peak_output->gross_cc_today = old_peak_to_use.gross_cc_today;



		pr(("%s %s(%d) rake for full day = %.2f + %.2f = %.2f\n",

				TimeStr(), _FL,

				old_peak_to_use.rake_today / 100.0,

				five_min_rake / 100.0,

				peak_output->rake_today / 100.0));



		if (peak_output->rake_today < normal_output->rake_today) {

		  #if 0	//20010211MB

			kp(("%s %s(%d) *** Calculation problem: rake estimate for entire day (%.2f) is LESS than normal for now (%.2f)\n",

					TimeStr(), _FL,

					peak_output->rake_today / 100.0, normal_output->rake_today / 100.0));

		  #endif

			peak_output->rake_today = normal_output->rake_today;

		}



		// Scale the normal output for the time into the current interval

		float scale_factor = (float)seconds_into_interval / (float)PREDICTION_DATA_INTERVAL;

		normal_output->players            += Pred_PreviousCurrentData_Delta.players * scale_factor;

		normal_output->rake_today         += (float)((normal_output->rake_per_hour * seconds_into_interval) / 3600.0);

		normal_output->games_today		  += Pred_PreviousCurrentData_Delta.games_today * scale_factor;

		normal_output->gross_cc_today	  += Pred_PreviousCurrentData_Delta.gross_cc_today * scale_factor;

		normal_output->new_accounts_today += Pred_PreviousCurrentData_Delta.new_accounts_today * scale_factor;

	}

}



/**********************************************************************************

 Function RequestAdminPlayerInfo(void)

 Date: HK01/01/18

 Purpose: pop up the dlg reqest dlg if necessary and handle it

**********************************************************************************/

void RequestAdminPlayerInfo(void)

{

	if (BadTimeForIntensiveServerRequest("Not now...", 

		"Any time between h:55 and h:10 is a bad time to launch one of these!\n")) {	// returns TRUE if we should abort

		return;

	}



	if (!hReqPlayerInfoDLG) {

		CreateDialog(hInst, MAKEINTRESOURCE(IDD_REQ_PLAYER_INFO), NULL, dlgReqPlayerInfo);

	} else {

		SendMessage(hReqPlayerInfoDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

		ReallySetForegroundWindow(hReqPlayerInfoDLG);

	}

}



/**********************************************************************************

 Function RequestAdminInfo

 Date: HK01/01/02

 Purpose: request information from the server (as an admin client)

***********************************************************************************/

void RequestAdminInfo(MiscMessageRequestInfo info_type, WORD32 param1, WORD32 param2, WORD32 param3, WORD32 param4)

{

	// ignore request if called with a blank userid

	if (!param1) {

		return;

	}

	// handle MMRI_PLAYERINFO with its own function through the dlg

	if (info_type == MMRI_PLAYERINFO) {

		RequestAdminPlayerInfo();

		return;

	}

	struct MiscClientMessage mcm;

	zstruct(mcm);

	mcm.message_type = MISC_MESSAGE_REQ_INFO;

	mcm.display_flags = (WORD32)info_type;	// hide the info requested type here

	mcm.misc_data_1 = param1;

	mcm.misc_data_2 = param2;

	mcm.misc_data_3 = param3;

	mcm.misc_data_4 = param4;

	SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

}	



/**********************************************************************************

 Function SetPlayerReqClockDlgDate(HWND hDlg, time_t when)

 Date: HK01/01/18

 Purpose: used below to set initial date on playerinforeq dlg

 Note: based on MB's similar functions for the shot clock date picker

***********************************************************************************/

void SetGetPlayerReqClockDlgDate(HWND hDlg, time_t when)

{

	SYSTEMTIME systime;

	zstruct(systime);

	Convert_time_t_to_SYSTEMTIME(when, &systime);

	SYSTEMTIME localtime;

	SystemTimeToTzSpecificLocalTime(NULL, &systime, &localtime);

	DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_DATEPICKER), GDT_VALID, &localtime);

}



/**********************************************************************************

 Function time_t GetPlayerReqClockDlgDate(HWND hDlg)

 Date: HK01/01/18

 Purpose: used below to get the date set on the control

 Note: based on MB's similar functions for the shot clock date picker

***********************************************************************************/

time_t GetPlayerReqClockDlgDate(HWND hDlg)

{

	time_t result = 0;

	SYSTEMTIME systime_date;

	zstruct(systime_date);

	DateTime_GetSystemtime(GetDlgItem(hDlg, IDC_DATEPICKER), &systime_date);

	Convert_SYSTEMTIME_to_time_t(&systime_date, &result);

	return result;

}



/**********************************************************************************

 Function CALLBACK dlgReqPlayerInfo()

 Date: HK01/01/18

 Purpose: handler for player info request (supplies parameters to server for processing)

***********************************************************************************/

BOOL CALLBACK dlgReqPlayerInfo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)



{

	char str[500];

	SDBRecord *sdb;

	switch (message) {

	case WM_INITDIALOG:

		{

			hReqPlayerInfoDLG = hDlg;

			AddKeyboardTranslateHwnd(hDlg);

			// init the spinner control

			HWND hSpinner = GetDlgItem(hDlg, IDC_REQINFO_SPIN);

			SendMessage(hSpinner, UDM_SETBUDDY, (WPARAM)(GetDlgItem(hDlg,IDC_REQINFO_DAYS)),(LPARAM)0);

			SendMessage(hSpinner, UDM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELONG(30,1)));

			SendMessage(hSpinner, UDM_SETPOS, (WPARAM)0, (LPARAM)(MAKELONG(3,0)));

			// set default email destinations

			CheckDlgButton(hDlg, IDC_CHECK_EMAIL_ADMIN, TRUE);

			CheckDlgButton(hDlg, IDC_CHECK_EMAIL_SUPPORT, FALSE);

			PostMessage(hDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

			// initiaize the date picker

			SetGetPlayerReqClockDlgDate(hDlg, time(NULL));

			// Make ourselves a topmost window

			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		}

		return FALSE;



	case WMP_UPDATE_PLAYER_INFO:

		zstruct(str);

		sdb = &LatestAccountRecordFromServer.sdb;

		sprintf(str, "%s\n%s\n%08lx",

			sdb->full_name, sdb->user_id, sdb->player_id);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_TEXT_PLAYER_INFO), str);

		sprintf(str, "Email to %s", LoggedInAccountRecord.sdb.email_address);

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_CHECK_EMAIL_ADMIN), str);

		sprintf(str, "Email to support@e-mediasoftware.com");

		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_CHECK_EMAIL_SUPPORT), str);

	

	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		hReqPlayerInfoDLG = NULL;

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK:

			{

				sdb = &LatestAccountRecordFromServer.sdb;

				struct MiscClientMessage mcm;

				zstruct(mcm);

				mcm.message_type = MISC_MESSAGE_REQ_INFO;

				mcm.display_flags = (WORD32)MMRI_PLAYERINFO;	// hide the info requested type here

				mcm.misc_data_1 = sdb->player_id;

				mcm.misc_data_2 = GetPlayerReqClockDlgDate(hDlg);

				GetDlgItemText(hDlg, IDC_REQINFO_DAYS, str, 4);

				mcm.misc_data_3 = atoi(str);

				mcm.misc_data_4 = 0;

				mcm.misc_data_4 |= (IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_ADMIN) ? MMRI_PLAYERINFO_EMAIL_REQUESTER : 0);

				mcm.misc_data_4 |= (IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_SUPPORT) ? MMRI_PLAYERINFO_EMAIL_SUPPORT : 0);

				SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				DestroyWindow(hDlg);

				MessageBox(NULL,

					"You will receive an Email response to your request.\nThis may take a few minutes.",

					"Player info request submitted...",

					MB_OK|MB_TOPMOST);

			}

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

/**********************************************************************************

 Function CALLBACK dlgReqXferEmail()

 Date: HK01/01/18

 Purpose: request server to send out transfer emails to both parties

***********************************************************************************/

BOOL CALLBACK dlgReqXferEmail(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	char str[500];

	TransferRequest *tr;

	switch (message) {

	case WM_INITDIALOG:

		{

			tr = (TransferRequest *)(lParam);

			hReqXferEmailDlg = hDlg;

			zstruct(str);

			sprintf(str, "%d", tr->from_id);

			SetDlgItemTextIfNecessary(hDlg, IDC_STORE_FROM_ID, str);

			sprintf(str, "%d", tr->to_id);

			SetDlgItemTextIfNecessary(hDlg, IDC_STORE_TO_ID, str);

			sprintf(str, "%d", tr->amount);

			SetDlgItemTextIfNecessary(hDlg, IDC_STORE_AMOUNT, str);

			AddKeyboardTranslateHwnd(hDlg);

		  #if 0	// this info is no longer around and can't be used for this.. fix eventually

			if (LatestXferDestRecordFromServer.sdb.player_id == tr->from_id) {

				sprintf(str,"Send email to transferer (%s)", LatestXferDestRecordFromServer.sdb.user_id);

				SetDlgItemTextIfNecessary(hDlg, IDC_CHECK_EMAIL_FROM, str);

				sprintf(str,"Send email to transfere� (%s)", LatestXferSrcRecordFromServer.sdb.user_id);

				SetDlgItemTextIfNecessary(hDlg, IDC_CHECK_EMAIL_TO, str);

			} else { // backwards

				sprintf(str,"Send email to transferer (%s)", LatestXferSrcRecordFromServer.sdb.user_id);

				SetDlgItemTextIfNecessary(hDlg, IDC_CHECK_EMAIL_FROM, str);

				sprintf(str,"Send email to transfere� (%s)", LatestXferDestRecordFromServer.sdb.user_id);

				SetDlgItemTextIfNecessary(hDlg, IDC_CHECK_EMAIL_TO, str);

			}

		  #endif

			CheckDlgButton(hDlg, IDC_CHECK_EMAIL_FROM, BST_CHECKED);

			CheckDlgButton(hDlg, IDC_CHECK_EMAIL_TO, BST_CHECKED);

			// make ourselves a topmost window

			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		}

		return FALSE;



	case WM_DESTROY:

		RemoveKeyboardTranslateHwnd(hDlg);

		hReqXferEmailDlg = NULL;

		break;



	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDC_CHECK_EMAIL_FROM:

		case IDC_CHECK_EMAIL_TO:

			EnableWindow(GetDlgItem(hDlg, IDOK),

				IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_FROM) ||

				IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_TO));

			break;

		

		case IDOK:

			{

				int from_checked =  IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_FROM);

				int to_checked =  IsDlgButtonChecked(hDlg, IDC_CHECK_EMAIL_TO);

				struct MiscClientMessage mcm;

				zstruct(mcm);

				mcm.message_type = MISC_MESSAGE_SEND_XFER_EMAIL;

				GetDlgItemText(hDlg, IDC_STORE_FROM_ID, str, 20);

				mcm.misc_data_1 = atol(str);

				GetDlgItemText(hDlg, IDC_STORE_TO_ID, str, 20);

				mcm.misc_data_2 = atol(str);

				GetDlgItemText(hDlg, IDC_STORE_AMOUNT, str, 20);

				mcm.misc_data_3 = atol(str);

				mcm.misc_data_4 = 0;

				mcm.misc_data_4 |= (from_checked ? XFER_EMAIL_FROM : 0);

				mcm.misc_data_4 |= (to_checked ? XFER_EMAIL_TO : 0);

				SendDataStructure(DATATYPE_MISC_CLIENT_MESSAGE, &mcm, sizeof(mcm));

				DestroyWindow(hDlg);

				MessageBox(NULL,

					"The request has been submitted to the server",

					"Real-money transfer email request...", MB_OK|MB_TOPMOST);

			}

			return TRUE;	// message was processed

		case IDCANCEL:

			DestroyWindow(hDlg);

		    return TRUE;	// message was processed

		}

		break;

	}

	NOTUSED(wParam);

	NOTUSED(lParam);

    return FALSE;	// We did NOT process this message.

}



/**********************************************************************************

 Function (HWND hDlg, WORD32 player_id, char *user_id, int hand_number)

 Date: HK01/01/24

 Purpose: send a Good All-In Request letter to a player (generated on the server)

***********************************************************************************/

void SendPlayerGAIRLetter(HWND hDlg, WORD32 player_id, char *user_id, int hand_number)

{

	char str[100];

	zstruct(str);

	sprintf(str, "This will ask %s about the all-in on hand #%d\n", user_id, hand_number);

	int rc = MessageBox(hDlg, str, "Are you sure?", MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);

	if (rc == IDCANCEL) {

		return;

	}

	// ship it...

	struct CardRoom_ReqHandHistory crhh;

	zstruct(crhh);

	crhh.hand_number = hand_number;

	crhh.player_id = player_id;

	crhh.request_type = HHRT_ADMIN_GAIR_LETTER;

	SendDataStructure(DATATYPE_CARDROOM_REQ_HAND_HISTORY, &crhh, sizeof(crhh));

	sprintf(str, 

		"The GAIR letter will be sent directly to %s.\nCheck 'answers' to see a copy of it.\n",

		user_id);

	MessageBox(hDlg,

			str,

			"Letter Request Submitted...",

			MB_OK|MB_APPLMODAL|MB_TOPMOST);

}



#endif	 // ADMIN_CLIENT



