//#define _MT



#include "stdafx.h"

#include "string.h"

#ifdef HORATIO

  #define DISP 0

#else

  #define DISP 0

#endif



#if INCL_SSL_SUPPORT

  #include <openssl/ssl.h>

  #include <openssl/crypto.h>    

#endif



#define USE_SELECT_FOR_LOOP_CONTROL	1



extern int valid_limitation;

// Version information about a program (local or remote).

struct VersionInfo ServerVersionInfo;

struct VersionNumber ClientVersionNumber;



ClientSocket *ClientSock;

int ClientSockConnected;	// set after initial communications with server has succeeded.

Packet *TableSummaryListPackets[MAX_CLIENT_DISPLAY_TABS];

char szOurIPString[100];	// either "123.456.2.223" or "123.456.2.223 / 192.168.1.1"

IPADDRESS OurLocalIP;		// our local ip address (local subnet)

long TimeDiffWithServer;	// an offset (in seconds) between the server's time and our time (not affected by time zones)



struct CardRoom_TableInfo *TableInfo;

struct AccountInfo SavedAccountInfo;	// most recent struct AccountInfo sent from server



int TableInfo_table_count;	// # of entries in the TableInfo structure.

int iDisableServerCommunications;	// set to disable all communications with the server.

int iSeatedAtATournamentTableFlag;	// set if we are currently seated at a tournament table

WORD32 dwLastPacketReceivedSeconds;	// SecondCounter when last packet was received.



#if ADMIN_CLIENT

WORD32 dwEarliestPacketSendTime;	// debug only: used for delaying all I/O for testing

struct AdminCheckRun AdminCheckRun;	// admin only: saved check run info from server.

int iPlayedConnectionUpSound = TRUE;	// admin only: set when we played the 'connection up' sound.

int iPlayedConnectionDownSound = TRUE;	// admin only: set when we played the 'connection down' sound



struct SpoofedClientSocketInfo {

	ClientSocket *sock;

	int sent_version_info;				// set once the initial version packet has been sent

	int packets_received_since_sending;	// # of packets received since we sent something back.

	int printed_connected_msg;			// set once we print the 'we've connected' msg.

	int time_of_last_send;				// SecondCounter when we last sent something

} *SpoofedClientSockets;	// ptr to an array of these.

#endif

WORD32 dwLastPingSentTime;			// SecondCounter when last ping sent from us

char szConnectionTypeString[120];	// string to indicate the type of connection (for the about box)

struct GameCommonData SavedGameCommonData;	// any received with no open table go here

static struct CardRoom_SeatAvail previous_seat_avail;



#if INCL_SSL_SUPPORT

SSL_CTX *MainSSL_CTX;

#endif

extern BOOL CALLBACK dlgFuncWesternUnion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK dlgFuncTransactionLarge(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern BOOL CALLBACK dlgDownLoadClient(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



char  pstr[200];

char  samount[30];

extern HWND hCCPurchaseDLG;

bool upgrade_yes =false;
int cont=0;

//*********************************************************

//
//

// Validate the game_rules BYTE8 received from the server for

// various packets.  This is necessary for a new client to

// connect to an old server which doesn't yet supply the

// seperated display tab index and game rules.

//

static void ValidateGameRules(BYTE8 client_display_tab_index, BYTE8 *game_rules)

{

	if (*game_rules < GAME_RULES_START || *game_rules >= GAME_RULES_START+MAX_GAME_RULES) {

		static BYTE8 DefaultGameRules[MAX_CLIENT_DISPLAY_TABS] = {

			GAME_RULES_HOLDEM,

			GAME_RULES_OMAHA_HI,

			GAME_RULES_STUD7,

			GAME_RULES_HOLDEM,

			GAME_RULES_OMAHA_HI_LO,

			GAME_RULES_STUD7_HI_LO,

			GAME_RULES_HOLDEM,		// tournament: should never be used

		};

		*game_rules = DefaultGameRules[client_display_tab_index];

	}

}



//****************************************************************

// May 3, 1999 - MB

//

// Take a new CardRoom_TableInfo structure from the server and

// store it in our array of those.  If we run out of room,

// re-alloc the array with more space and copy our existing

// data into it.

// Update TableInfo and TableInfo_table_count accordingly.

//

void UpdateTableInfoEntry(struct CardRoom_TableInfo *ti)

{

	pr(("%s(%d) Updating TableInfo entry for table %d\n", _FL, ti->table_serial_number));

	EnterCriticalSection(&CardRoomVarsCritSec);

	if (!TableInfo) {

		// First time... initialize.

		TableInfo_table_count = 8;	// a reasonable start.

		int len = sizeof(struct CardRoom_TableInfo) * TableInfo_table_count;

		TableInfo = (struct CardRoom_TableInfo *)malloc(len);

		if (!TableInfo) {

			Error(ERR_ERROR, "%s(%d) Error allocating %d bytes for TableInfo.",_FL,len);

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return;

		}

		memset(TableInfo, 0, len);	// always clear to zero.

	}



	// Update the info in the array.

	// First, search the array to see if it's already there.

	int first_empty = -1;

	for (int i=0 ; i<TableInfo_table_count ; i++) {

		if (first_empty < 0 && !TableInfo[i].table_serial_number) {

			first_empty = i;	// keep track of the first empty slot.

		}

		if (TableInfo[i].table_serial_number == ti->table_serial_number) {

			// Found it.  Just update it.

			TableInfo[i] = *ti;

			// Notify the cardroom dialog box.

			if (hCardRoomDlg) {

				PostMessage(hCardRoomDlg, WMP_UPDATE_TABLE_INFO, 0, (LPARAM)ti->table_serial_number);

			}

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return;

		}

	}



	// We didn't find it.  Add it.

	pr(("%s(%d) We didn't find it... first_empty = %d\n", _FL, first_empty));

	if (first_empty < 0) {

		// There's no space... allocate more.

		int old_len = sizeof(struct CardRoom_TableInfo) * TableInfo_table_count;

		first_empty = TableInfo_table_count;	// new stuff can go in first new entry

		TableInfo_table_count = (TableInfo_table_count * 3) / 2;	// make somewhat larger.

		int new_len = sizeof(struct CardRoom_TableInfo) * TableInfo_table_count;

		pr(("%s(%d) old_len = %d, new_len = %d\n", _FL, old_len, new_len));

		struct CardRoom_TableInfo *new_TableInfo = (struct CardRoom_TableInfo *)malloc(new_len);

		if (!new_TableInfo) {

			Error(ERR_ERROR, "%s(%d) Error allocating %d bytes for new_TableInfo.",_FL,new_len);

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return;

		}

		memset(new_TableInfo, 0, new_len);	// always clear to zero.

		memmove(new_TableInfo, TableInfo, old_len);	// copy what we knew

		free(TableInfo);			// free old copy.

		TableInfo = new_TableInfo;	// start using new array

	}



	TableInfo[first_empty] = *ti;

	// Notify the cardroom dialog box.

	if (hCardRoomDlg) {

		PostMessage(hCardRoomDlg, WMP_UPDATE_TABLE_INFO, 0, (LPARAM)ti->table_serial_number);

	}

	LeaveCriticalSection(&CardRoomVarsCritSec);

}



//*********************************************************

//
//

// Send the ClientStateInfo structure for a table, provided it

// has changed since it was last sent.

//

ErrorType SendClientStateInfo(struct TableInfo *t)

{

	t->ClientState.table_serial_number = t->table_serial_number;	// always keep up to date

	// If no in-turn action is specified, clear out the fields which

	// change depending on the in-turn action chosen.
	
	if (t->ClientState.in_turn_action==ACT_NO_ACTION) {

		t->ClientState.in_turn_action_game_state = 0;

		t->ClientState.in_turn_action_last_input_request_serial_number = 0;

		t->ClientState.in_turn_action_amount = 0;

	}



	//t->ClientState.sitting_out_flag = (BYTE8)IsDlgButtonChecked(hDlg, IDC_SIT_OUT);



	if (!memcmp(&t->ClientState, &t->PrevClientState, sizeof(t->ClientState))) {

		// They're identical... our work here is done.

		return ERR_NONE;

	}



	// send the low 32-bits of cpu's time stamp counter as additional

	// entropy for rng on the server.

	t->ClientState.random_bits = dwRandomEntropy;

	if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

		t->ClientState.random_bits += (WORD32)rdtsc();

	}



	t->PrevClientState = t->ClientState;



	pr(("%s(%d) Sending ClientStateInfo for table %d. sitting_out_flag = %d\n",

			_FL, t->table_serial_number, t->ClientState.sitting_out_flag));

	return SendDataStructure(DATATYPE_CLIENT_STATE_INFO, &(t->ClientState), sizeof(t->ClientState));

}



//****************************************************************

//
//

// Send an arbitrary data structure to the server (arbitrary socket)

//

ErrorType SendDataStructure(ClientSocket *sock, int data_type, void *data_structure_ptr, int data_structure_len)

{

	if (!sock) {

		//kp(("%s(%d) No ClientSock exists... cannot send data structure\n", _FL));

		return ERR_NONE;	// we can't send it if there's no connection.

	}

	if (data_structure_ptr==NULL && data_structure_len) {

		Error(ERR_INTERNAL_ERROR, "%s(%d) NULL pointer passed to SendDataStructure()",_FL);

		return ERR_INTERNAL_ERROR;

	}

	if (data_type < 0 || data_structure_len < 0 || (data_structure_ptr && data_structure_len==0)) {

		Error(ERR_INTERNAL_ERROR, "%s(%d) Illegal data_type (%d) or data_structure_len (%d)",_FL, data_type, data_structure_len);

		return ERR_INTERNAL_ERROR;

	}



	EnterCriticalSection(&ClientSockCritSec);

	// Allocate a new packet... grab on old one from the pool if possible.

	Packet *p = PktPool_Alloc(sizeof(struct DataPacketHeader) + data_structure_len);

	if (p==NULL) {

		Error(ERR_ERROR, "%s(%d) new Packet allocation failed",_FL);

		LeaveCriticalSection(&ClientSockCritSec);

		return ERR_ERROR;

	}



  #if 0//

	p->packing_disabled = TRUE;

	kp1(("%s(%d) Warning: disabling packing on all packets.\n",_FL));

  #endif



	// Write the header into the packet.

	struct DataPacketHeader hdr;

	zstruct(hdr);

	hdr.data_packet_type = (WORD16)data_type;

	hdr.data_packet_length = (WORD16)data_structure_len;

	ErrorType err = p->WriteData((char *)&hdr, sizeof(hdr), 0);

	if (err) {

		delete p;

		LeaveCriticalSection(&ClientSockCritSec);

		return err;

	}



	// Write the data into the packet.

	err = p->WriteData((char *)data_structure_ptr, data_structure_len, sizeof(hdr));

	if (err) {

		delete p;

		LeaveCriticalSection(&ClientSockCritSec);

		return err;

	}



  #if 0   //para que corra en Win98

	kp(("%s(%d) Here's the packet we're about to send:\n",_FL));

	khexdump(p->user_data_ptr, p->user_data_length, 16, 1);

  #endif



	//kp(("%s(%d) Calling SendPacket()... packet #%d, p->user_data_length = %d, p->length = %d, data_structure_len = %d\n", _FL, p->packet_serial_number, p->user_data_length, p->length, data_structure_len));



	// We're all done building the packet... send it off.

	err = sock->SendPacket(&p, NULL);

	if (p) {

		// It didn't get cleared by SendPacket(), therefore there must have been

		// some sort of error and we need to delete it ourselves.

		delete p;

	}

	sock->ProcessSendQueue();	//20000614MB: try to send asap to help reduce latency

	LeaveCriticalSection(&ClientSockCritSec);

	return err;

}



ErrorType SendDataStructure(int data_type, void *data_structure_ptr, int data_structure_len)

{

	if (!ClientSockConnected) {

		return ERR_NONE;	// we're not ready to communicate... don't send.

	}

	EnterCriticalSection(&ClientSockCritSec);

	ErrorType err = SendDataStructure(ClientSock, data_type, data_structure_ptr, data_structure_len);

	LeaveCriticalSection(&ClientSockCritSec);

	return err;

}



//*********************************************************

//
//

// Return a flag to indicate whether our app should be considered

// active or idle (true = active, false = idle).

//

int IsOurAppActive(void){
	if (iOurAppIsActive && (SecondCounter - dwLastUserActionMessageTime <= 180)) {
		return TRUE;
	}
	return FALSE;
}



//*********************************************************


//

// Send a KeepAlive structure to our currently connected socket.

//

ErrorType SendKeepAlive(void)

{

	struct KeepAlive ka;

	zstruct(ka);

	if (IsOurAppActive()) {

		ka.status_bits |= 0x01;	// set flag to indicate we're the active app

	}



	// send the low 32-bits of cpu's time stamp counter as additional

	// entropy for rng on the server.

	ka.random_bits = dwRandomEntropy;

	if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

		ka.random_bits += (WORD32)rdtsc();

	}


   // PPlaySound(SOUND_WAKE_UP);	// 'small' sound
	return SendDataStructure(DATATYPE_KEEP_ALIVE2, &ka, sizeof(ka));

}



//*********************************************************

//
//

// Send a ping to the server.

//

void SendPing(void)

{

	//kp(("%s(%d) SendPing() has been called.\n",_FL));

	// Never send pings too often (at most every 2s)

	if (SecondCounter - dwLastPingSentTime < 2)

		return;	// don't send it.



	struct Ping ping;

	zstruct(ping);

	ping.local_ms_timer = GetTickCount();



	// send the low 32-bits of cpu's time stamp counter as additional

	// entropy for rng on the server.

	ping.random_bits = dwRandomEntropy;

	if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

		ping.random_bits += (WORD32)rdtsc();

	}

    //#if 1
		//produce un sonido cada vez que envia un ping al 
		// server
	//	PPlaySound(SOUND_WAKE_UP);	// 'small' sound
	//#endif

	SendDataStructure(DATATYPE_PING, &ping, sizeof(ping));
	
	dwLastPingSentTime = SecondCounter;

}



//****************************************************************

//
//

// Send a GamePlayerInputResult structure to our currently

// connected socket.

//

ErrorType SendGamePlayerInputResult(struct GamePlayerInputResult *gpir)

{

	// send the low 32-bits of cpu's time stamp counter as additional

	// entropy for rng on the server.

	gpir->random_bits = dwRandomEntropy;

	if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

		gpir->random_bits += (WORD32)rdtsc();

	}

	ErrorType err = SendDataStructure(DATATYPE_PLAYER_INPUT_RESULT, gpir, sizeof(*gpir));

	// Always follow it up with another packet to catch dropped packets

	// sooner.

	SendKeepAlive();

	return err;

}



//****************************************************************

//  April 14, 1999 - MB

//

//	Verify a few of the fields in an incoming packet to make

//	sure that it looks at least remotely valid.

//	In particular, check the length to make sure it matches up

//	properly with what is expected.

//

ErrorType VerifyIncomingPacketFormat(Packet *p, int expected_size)

{

	struct DataPacketHeader *hdr = (struct DataPacketHeader *)p->user_data_ptr;

	if (hdr->data_packet_length != expected_size) {

		Error(ERR_ERROR, "%s(%d) Received data packet (type %d) which was wrong size (%d vs %d). Tossing it.",

							_FL, hdr->data_packet_length, expected_size);

		return ERR_ERROR;

	}

	return ERR_NONE;

}



//****************************************************************

//
//

// Process a received GameCommonData structure.

//

ErrorType ProcessGameCommonData(struct GameCommonData*gcd, int input_structure_len)

{

	pr(("%s(%d) We received a struct GameCommonData\n",_FL));

	//khexdump(gcd, sizeof(*gcd), 24, 1);



	if (sizeof(*gcd) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) GameCommonData structure was wrong length (%d vs %d)",_FL,sizeof(*gcd),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	pr(("%s(%d) GameCommonData received: GCDF_USE_REAL_MONEY_CHIPS = %d\n",

			_FL, gcd->flags & GCDF_USE_REAL_MONEY_CHIPS));

    ValidateGameRules(gcd->client_display_tab_index, &gcd->game_rules); // make sure game_rules is valid

	int i = TableIndexFromSerialNumber(gcd->table_serial_number);

	if (i<0) {

		//Error(ERR_ERROR, "%s(%d) Received data for a table we don't belong to (%d)", _FL, gcd->table_serial_number);

		SavedGameCommonData = *gcd;	// any with no home go here.

		return ERR_ERROR;	// do not process.

	}

	struct TableInfo *t = &Table[i];


	pr(("%s %s(%d) GCD for game %d arrived %dms after last GCD.\n",

				TimeStr(), _FL,

				gcd->game_serial_number,

				GetTickCount() - t->ticks_when_gcd_arrived));

	t->NewestGameCommonData = *gcd;

	t->ticks_when_gcd_arrived = GetTickCount();



	// If we were asked to enter 'sit out mode' due to being timed out or something,

	// set those flags right here and now (asap).

	if (!gcd->sit_out_request_serial_num) {	// not being asked to sit out?

		// make sure our table's version is initialized to the same value.

		t->sit_out_request_serial_num = 0;

	}

	if (gcd->sit_out_request_serial_num != t->sit_out_request_serial_num) {

		// We've just been forced into sitting out mode.

		pr(("%s(%d) We've just been forced into sit-out mode by the server (s/n %d vs %d).\n",

				_FL, gcd->sit_out_request_serial_num, t->sit_out_request_serial_num));

		t->sit_out_request_serial_num = gcd->sit_out_request_serial_num;

		t->ClientState.sitting_out_flag = TRUE;

		t->forced_to_sit_out_flag = TRUE;	// indicate the server forced us out.

		CheckDlgButton(t->hwnd, IDC_SIT_OUT, t->ClientState.sitting_out_flag);

		InvalidateRect(GetDlgItem(t->hwnd, IDC_SIT_OUT), NULL, FALSE);

		SendClientStateInfo(t);	// re-send to server

	}



  #if 0	//20000913MB: GamePlayerData should always follow a GameCommonData

	FlagRedrawTable(i);

  #endif



  #if 0

	kp(("%s(%d) Current bar snacks for game %d:\n",_FL,gcd->game_serial_number));

	for (int j=0 ; j<MAX_PLAYERS_PER_GAME ; j++) {

		if (gcd->bar_snack[j]) {

			kp(("%s(%d)   plr %d: bar snack = %d\n", _FL, j, gcd->bar_snack[j]));

		}

	}

  #endif


  return ERR_NONE;

}



//****************************************************************

//
//

// Process a received GamePlayerData structure.

//

ErrorType ProcessGamePlayerData(struct GamePlayerData *gpd, int input_structure_len)

{

	pr(("%s(%d) We received a struct GamePlayerData\n",_FL));



	if (sizeof(*gpd) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) GamePlayerData structure was wrong length (%d vs %d)",_FL,sizeof(*gpd),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	int i = TableIndexFromSerialNumber(gpd->table_serial_number);

	if (i<0) {

		//Error(ERR_ERROR, "%s(%d) Received data for a table we don't belong to (%d)", _FL, gpd->table_serial_number);

		return ERR_ERROR;	// do not process.

	}

	struct TableInfo *t = Table + i;



  #if 0	//

	static DWORD old_ticks;

	DWORD ticks = GetTickCount();

	int different = memcmp(gpd, &t->GamePlayerData, sizeof(*gpd));

	kp(("%s(%d) Received GamePlayerData %5dms after the last one (%5dms after gcd)%s\n",

			_FL,ticks - old_ticks,

			ticks - t->ticks_when_gcd_arrived,

			different ? "" : " ** Exactly the same data as last time **"));

	old_ticks = ticks;

  #endif

  #if 0	//20000914MB

	kp(("            pot[0]=%3d   pot[1]=%3d   pot[2]=%3d\n", gpd->pot[0], gpd->pot[1], gpd->pot[2]));

	for (int j=0 ; j<MAX_PLAYERS_PER_GAME ; j++) {

		kp(("            plr %d chips_bet_this_round = %3d\n", j, gpd->chips_bet_this_round[j]));

	}

  #endif



	t->GamePlayerData = *gpd;

	t->GameCommonData = t->NewestGameCommonData;	// copy matching gcd when gpd arrives.

	t->ticks_when_gpd_arrived = GetTickCount();



  #if 0	//20000911MB: no longer used

	// If we were asked to enter 'sit out mode' due to being timed out or something,

	// set those flags right here and now (asap).

	if ((gpd->flags & GPDF_FORCED_TO_SIT_OUT) && !t->ClientState.sitting_out_flag) {

		// We've just been forced into sitting out mode.

		kp(("%s(%d) We've just been forced into sit-out mode by the server.\n", _FL));

		t->ClientState.sitting_out_flag = TRUE;

		t->forced_to_sit_out_flag = TRUE;	// indicate the server forced us out.

		CheckDlgButton(t->hwnd, IDC_SIT_OUT, t->ClientState.sitting_out_flag);

		InvalidateRect(GetDlgItem(t->hwnd, IDC_SIT_OUT), NULL, FALSE);

		SendClientStateInfo(t);

	}

  #endif



   

	// Keep the animation disable count reasonable (it only needed to be large when

	// we weren't sure how long it would be until we received some more recent

	// GamePlayerData structures).

	Table[i].animation_disable_count = min(Table[i].animation_disable_count, 2);

	//kp(("%s(%d) table->animation_disable_count = %d\n", _FL, Table[i].animation_disable_count));



	FlagRedrawTable(i);

	return ERR_NONE;

}



//*********************************************************

//
//

// If the server has asked for a particular action and we have

// that action in our action_mask, return the first match to the

// input request directly to the server.

// Returns: FALSE for no match, TRUE for match found and result sent out.

//

int CheckAutoRespondToInput(struct TableInfo *t, struct GamePlayerInputRequest *gpir, int action_mask)

{

	int matched_bits = gpir->action_mask & action_mask;

	if (matched_bits) {

		// We have a match...  Find first bit.

		int action = 0;

		while (!(matched_bits & (1<<action)) && action < 32) {

			action++;

		}

		pr(("%s(%d) action_mask1 = $%04x, action_mask2 = $%04x, (1&2=%04x), action result = %d\n", _FL, gpir->action_mask, action_mask, matched_bits, action));

		if (t){	// handle normally...

			t->GamePlayerInputRequest = *gpir;	// save it so ProcessGPIResult() can use it.

			ProcessGPIResult(t, action);

		} else{

			// No table defined... must be a latent request after we left

			// the table.  Just respond directly and forget about it.

			struct GamePlayerInputResult result;

			zstruct(result);

			result.game_serial_number			= gpir->game_serial_number;

			result.table_serial_number			= gpir->table_serial_number;

			result.input_request_serial_number	= gpir->input_request_serial_number;

			result.seating_position				= gpir->seating_position;

			result.action = (BYTE8)action;

			result.ready_to_process = TRUE;

			SendGamePlayerInputResult(&result);	// send it to server

		}

		return TRUE;	// match found and processed.

	}

	return FALSE;	// no match found.

}



//****************************************************************

//
//

// Process a received GamePlayerInputRequest structure.

//

ErrorType ProcessGamePlayerInputRequest(struct GamePlayerInputRequest *gpir, int input_structure_len)

{

	pr(("%s(%d) We received a struct GamePlayerInputRequest\n",_FL));



	if (sizeof(*gpir) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) GamePlayerInputRequest structure was wrong length (%d vs %d)",_FL,sizeof(*gpir),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	EnterCriticalSection(&CardRoomVarsCritSec);

	int i = TableIndexFromSerialNumber(gpir->table_serial_number);

	if (i<0) {

		//Error(ERR_ERROR, "%s(%d) Received data for a table we don't belong to (%d)", _FL, gpir->table_serial_number);

		kp(("%s(%d) Received input request for a table we're no longer joined to. Trying to fold.\n",_FL));

		if (CheckAutoRespondToInput(NULL, gpir, 1<<ACT_FOLD)) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}



		if (CheckAutoRespondToInput(NULL, gpir,

						(1<<ACT_SIT_OUT_SB) |

						(1<<ACT_SIT_OUT_BB) |

						(1<<ACT_SIT_OUT_POST) |

						(1<<ACT_SIT_OUT_BOTH) |

						(1<<ACT_SIT_OUT_ANTE))

		) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}



		if (CheckAutoRespondToInput(NULL, gpir,

						(1<<ACT_BRING_IN) |

						(1<<ACT_BRING_IN_ALL_IN) |

						(1<<ACT_SHOW_HAND) |

						(1<<ACT_TOSS_HAND) |

						(1<<ACT_MUCK_HAND))

		) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}



		LeaveCriticalSection(&CardRoomVarsCritSec);

		Error(ERR_ERROR, "%s(%d) Received input request for a table we don't belong to (%d) and couldn't fold (mask=$%08lx)", _FL, gpir->table_serial_number, gpir->action_mask);

		return ERR_ERROR;

	}

	struct TableInfo *t = &Table[i];

	t->last_input_request_serial_number = gpir->input_request_serial_number;	// always copy

	// If we've already answered this request, it must have gotten lost (or

	// latency was involved) so send it out again to be safe.

	if (t->GamePlayerInputResult.table_serial_number == gpir->table_serial_number &&

		t->GamePlayerInputResult.game_serial_number == gpir->game_serial_number &&

		t->GamePlayerInputResult.input_request_serial_number == gpir->input_request_serial_number) {

		// Double check that we've got an answer...

		if ((gpir->action_mask & (1<<t->GamePlayerInputResult.action))) {

			pr(("%s(%d) Got duplicate input request... sending old answer again (table %d, game %d, input_request %d).\n",

						_FL, t->GamePlayerInputResult.table_serial_number,

						t->GamePlayerInputResult.game_serial_number,

						t->GamePlayerInputResult.input_request_serial_number));

			LeaveCriticalSection(&CardRoomVarsCritSec);

			SendGamePlayerInputResult(&(t->GamePlayerInputResult));	// send it to server

			return ERR_NONE;	// no more processing to do.

		}

	}



	// If the user has clicked 'fold in turn', 'auto post', or 'sit out',

	// we may be able to answer this input request right here.

	// See also table.cpp where these checkboxes are clicked; there's

	// similar code that may need updating in parallel with this code.

	if (t->ClientState.sitting_out_flag && CheckAutoRespondToInput(t, gpir,

					(1<<ACT_SIT_OUT_SB) |

					(1<<ACT_SIT_OUT_BB) |

					(1<<ACT_SIT_OUT_POST) |

					(1<<ACT_SIT_OUT_BOTH) |

					(1<<ACT_SIT_OUT_ANTE))

	) {

		LeaveCriticalSection(&CardRoomVarsCritSec);

		return ERR_NONE;	// no more processing to do.

	}



	//  we want to handle all situations for POSTing, keeping in mind the wait_for_bb

	if (t->wait_for_bb && CheckAutoRespondToInput(t, gpir,

					(1<<ACT_SIT_OUT_POST) |

					(1<<ACT_SIT_OUT_BOTH))

 	) {

		LeaveCriticalSection(&CardRoomVarsCritSec);

		return ERR_NONE;	// no more processing to do.

	}

	

	if (t->ClientState.fold_in_turn && CheckAutoRespondToInput(t, gpir, 1<<ACT_FOLD)) {

		LeaveCriticalSection(&CardRoomVarsCritSec);

		return ERR_NONE;	// no more processing to do.

	}



	// most autoposts get taken care of here -- BB is seperate (down below)

	if (!t->wait_for_bb  &&

		t->ClientState.post_in_turn && 

		CheckAutoRespondToInput(t, gpir,

					(1<<ACT_POST) |

					(1<<ACT_POST_SB) |

					(1<<ACT_POST_BB) |

					(1<<ACT_POST_BOTH) |

					(1<<ACT_POST_ANTE))

	) {

		LeaveCriticalSection(&CardRoomVarsCritSec);

		return ERR_NONE;	// no more processing to do.

	}



	// autopost if we're waiting for the BB and it's available to post

	if (t->wait_for_bb  &&

		t->ClientState.post_in_turn && 

		CheckAutoRespondToInput(t, gpir, (1<<ACT_POST_BB))) {

			// clear the wait for_for_bb now

			t->wait_for_bb = FALSE;

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

	}





  #if 0	//19990927MB

	//kp(("%s(%d) t->ClientState.muck_losing_hands = %d\n", _FL, t->ClientState.muck_losing_hands));

	if (t->ClientState.muck_losing_hands && CheckAutoRespondToInput(t, gpir, 1<<ACT_MUCK_HAND)) {

		LeaveCriticalSection(&CardRoomVarsCritSec);

		return ERR_NONE;	// no more processing to do.

	}

  #endif



	// If they've selected an 'in turn' action... auto-responsd to it now.

  #if 0	//19990930MB

	kp1(("%s(%d) WARNING: IN TURN ACTIONS ARE CURRENTLY IGNORED ON THE CLIENT END (FOR TESTING PURPOSES ONLY)!\n",_FL));

  #else

	// If they've selected an 'in turn' action... auto-responsd to it now.

	int saved_in_turn_action = t->ClientState.in_turn_action;

	int saved_action_amount  = t->ClientState.in_turn_action_amount;

	pr(("%s(%d) Testing in_turn_action of %d to see if we can auto-respond (action_amount = %d)\n",_FL,saved_in_turn_action, saved_action_amount));

	ClearInTurnActions(t, FALSE);

	switch (saved_in_turn_action) {

	case ACT_FOLD:

		if (saved_action_amount != -1) {	// is checking allowed?

			// First, try to check... if that fails, fold.

			if (CheckAutoRespondToInput(t, gpir, 1<<ACT_CHECK)) {

				LeaveCriticalSection(&CardRoomVarsCritSec);

				return ERR_NONE;	// no more processing to do.

			}

		}

		if (CheckAutoRespondToInput(t, gpir, 1<<ACT_FOLD)) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}

		break;

	case ACT_RAISE:

		if (saved_action_amount != -1 && saved_action_amount != gpir->raise_amount)	{	
		// the raise amount changed and they did not select 'any'... disallow.
			break;
		}

		if (CheckAutoRespondToInput(t, gpir,

				(1<<ACT_BET) |

				(1<<ACT_RAISE) |

				(1<<ACT_BET_ALL_IN) |

				(1<<ACT_RAISE_ALL_IN))

		) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}

		// That didn't work... try to check/call (if pot was capped)...

		// fall through to the ACT_CALL handler.

	case ACT_CALL:

		if (saved_action_amount != -1 &&

			saved_action_amount != gpir->call_amount)

		{	// the call amount changed and they did not select 'any'... disallow.

			break;

		}

		if (CheckAutoRespondToInput(t, gpir,

				(1<<ACT_CHECK) |

				(1<<ACT_CALL) |

				(1<<ACT_CALL_ALL_IN))

		) {

			LeaveCriticalSection(&CardRoomVarsCritSec);

			return ERR_NONE;	// no more processing to do.

		}

		break;

	}

  #endif



	pr(("%s(%d) Did not auto-respond to input request\n",_FL));

	// Determine if this is a reminder notice or if it's the first time

	// we've seen this request.

	int reminder = FALSE;

	if (t->GamePlayerInputRequest.table_serial_number == gpir->table_serial_number &&

		t->GamePlayerInputRequest.game_serial_number == gpir->game_serial_number &&

		t->GamePlayerInputRequest.input_request_serial_number == gpir->input_request_serial_number) {

		reminder = TRUE;

	}



	t->GamePlayerInputRequest = *gpir;

	LeaveCriticalSection(&CardRoomVarsCritSec);

	// If this table is in computer play mode, determine the result now.

	int skip_popup = FALSE;	// override for preventing window from popping up

  #if ADMIN_CLIENT

	#if MIN_HAND_RANK_POPUP

	  // if there's a setting here and we don't qualify, no popup

	  pr(("%s(%d) Testing... puohr=%d, chr = %d, puphrn = %d, TrHand %s\n", _FL, t->pop_up_on_hand_rank, t->current_hand_rank, t->pop_up_on_hand_rank_number,t->tournament_game_number_str));

	  if (t->pop_up_on_hand_rank && 

		  t->current_hand_rank > t->pop_up_on_hand_rank_number) {	// skip it

			skip_popup = TRUE;

	  }

	#endif

	if (t->computer_play) {

		int wait_time = 0;

		if (t->computer_play_seconds)

			wait_time = random(t->computer_play_seconds*2);

		// impose a minimum delay of avg_time/3

		wait_time = max(t->computer_play_seconds/3, wait_time);

		t->computer_play_answer_delay = wait_time;

		t->computer_play_answer_time = SecondCounter + t->computer_play_answer_delay;

	} else

  #endif

	{

		// Normal play

		pr(("%s(%d) Calling FlagRedrawTable()\n",_FL));

		FlagRedrawTable(i);



		//: If they've already got a window up with action buttons on it,

		// don't top this one yet.  It will get done automatically once the previous

		// action buttons are dealt with.

		//kp(("%s(%d) %4d: # of tables with action buttons showing = %d\n", _FL, SecondCounter, CountTablesWithActionButtonsShowing()));

		if (CountTablesWithActionButtonsShowing() <= 1 && !dwCheckForInputButtons_Ticks) {

			// Top it and restore it so that the user knows they are being waited on.

		  #if 1	//

			// Find the table this relates to.

			for (int i=0 ; i<MAX_TABLES ; i++) {

				if (Table[i].table_serial_number==gpir->table_serial_number) {

					if (!skip_popup) {

						ShowTableWindowIfPlaying(&Table[i]);

					}

					break;

				}

			}

		  #else

			PostMessage(hInitialWindow, WMP_SHOW_TABLE_WINDOW, FALSE, gpir->table_serial_number);

		  #endif

			// If this is a reminder notice, play a 'bigger' sound.

			if (reminder) {

				PPlaySound(SOUND_WAKE_UP);		// 'bigger' sound

			} else {

				PPlaySound(SOUND_YOUR_TURN);	// 'small' sound

			}

		}

	}

	pr(("%s(%d) Done processing GamePlayerInputRequest\n",_FL));

	return ERR_NONE;

}



//****************************************************************
//

// Process a received GamePlayerInputRequestCancel structure.

//

ErrorType ProcessGamePlayerInputRequestCancel(struct GamePlayerInputRequestCancel *gpir, int input_structure_len)

{

	pr(("%s(%d) We received a struct GamePlayerInputRequestCancel\n",_FL));



	if (sizeof(*gpir) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) GamePlayerInputRequestCancel structure was wrong length (%d vs %d)",_FL,sizeof(*gpir),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	int i = TableIndexFromSerialNumber(gpir->table_serial_number);

	if (i<0) {

		//Error(ERR_ERROR, "%s(%d) Received data for a table we don't belong to (%d)", _FL, gpir->table_serial_number);

		return ERR_ERROR;	// do not process.

	}



	struct TableInfo *t = &Table[i];

	t->last_input_request_serial_number = gpir->input_request_serial_number;	// always copy

	pr(("%s(%d) changing t->ClientState.in_turn_action from %d to zero.\n",_FL,t->ClientState.in_turn_action));

	ClearInTurnActions(t, FALSE);

	// if we're told this player is now sitting out, click his checkbox

	if (gpir->now_sitting_out) {

		t->ClientState.sitting_out_flag = TRUE;

		t->forced_to_sit_out_flag = TRUE;

		CheckDlgButton(t->hwnd, IDC_SIT_OUT, t->ClientState.sitting_out_flag);

		InvalidateRect(GetDlgItem(t->hwnd, IDC_SIT_OUT), NULL, FALSE);

	}

	// Now make sure the server knows the new state of our sit out checkbox and other checkboxes

	SendClientStateInfo(t);

	zstruct(Table[i].GamePlayerInputRequest);

	FlagRedrawTable(i);

	return ERR_NONE;

}




//****************************************************************


//

// Process the response to a GamePlayerInputRequest.  This

// includes filling in and sending out the GamePlayerInputResponse

// structure.

//

void ProcessGPIResult(struct TableInfo *t, int action_result)

{

//	char vv[5];
//	sprintf(vv,"%d", t->GamePlayerInputRequest.raise_amount);
//	MessageBox(t->hwnd,vv , "Debug", MB_OK);

	EnterCriticalSection(&CardRoomVarsCritSec);

	zstruct(t->GamePlayerInputResult);

	if (t->GamePlayerInputRequest.game_serial_number) {

		if (!(t->GamePlayerInputRequest.action_mask & (1<<action_result))) {

			kp((ANSI_ERROR"%s(%d) Error: ProcessGPIResult() is sending back an action that was not in the allowable mask (mask=$%08lx, result=%d)\n",_FL,t->GamePlayerInputRequest.action_mask,action_result));

		}

		t->GamePlayerInputResult.game_serial_number				= t->GamePlayerInputRequest.game_serial_number;

		t->GamePlayerInputResult.table_serial_number			= t->GamePlayerInputRequest.table_serial_number;

		t->GamePlayerInputResult.input_request_serial_number	= t->GamePlayerInputRequest.input_request_serial_number;

		t->GamePlayerInputResult.seating_position				= t->GamePlayerInputRequest.seating_position;

		t->GamePlayerInputResult.action = (BYTE8)action_result;

//		t->GamePlayerInputResult.raise_amount					= t->GamePlayerInputRequest.raise_amount;  //J Fonseca 01/02/2004

		t->GamePlayerInputResult.ready_to_process = TRUE;

		zstruct(t->GamePlayerInputRequest);	// clear request immediately after processing an input



		// clear action buttons after pressing


		for (int i=0 ; i<MAX_BUTTONS_PER_TABLE ; i++) {

			t->button_actions[i] = 0;

		}



		LeaveCriticalSection(&CardRoomVarsCritSec);



		// If we folded, we're not allowed to have any more 'in turn' actions,

		// so keep track of which game we folded.

		if (t->GamePlayerInputResult.action==ACT_FOLD) {

			t->fold_game_serial_number = t->GamePlayerInputResult.game_serial_number;

		}

		t->last_input_result_seconds = SecondCounter;	// keep track of when it was last sent

		// default to not expecting another turn this round until we get

		// some new information from the server which indicates otherwise.

		t->GamePlayerData.expected_turn_this_round = 0;

		SendGamePlayerInputResult(&(t->GamePlayerInputResult));	// send it to server
      
		FlagRedrawTable(t->table_index);		// redraw our window.
		

	} else {

		LeaveCriticalSection(&CardRoomVarsCritSec);

	}

}



/**********************************************************************************

 Function NameIsInMsg(char *msg, char *name)


 Purpose: T/F is name in msg? (case insensitive)

 Notes:   used below in chat function

***********************************************************************************/

int NameIsInMsg(char *msg, char *name)

{

	if ( !strlen(msg) || !strlen(name) ) {	// blanks

		return FALSE;

	}

	// a chat msg is never more than the defined length + who said it + padding (minimal)

	#define MAX_CHAT_TEST_MSG	MAX_CHAT_MSG_LEN+MAX_PLAYER_USERID_LEN+10

	char t_msg[MAX_CHAT_TEST_MSG];

	char t_name[MAX_PLAYER_USERID_LEN];

	// local copies

	strnncpy(t_msg, msg, MAX_CHAT_TEST_MSG);

	strnncpy(t_name, name, MAX_PLAYER_USERID_LEN);

	// normalize case

	unsigned int i;

	for (i = 0; i < strlen(t_msg); i++) {

		t_msg[i] = (char)toupper(t_msg[i]);

	}

	for (i = 0; i < strlen(t_name); i++) {

		t_name[i] = (char)toupper(t_name[i]);

	}

	// now do the compare

	if (strstr(t_msg, t_name)) {

		return TRUE;

	}

	return FALSE;

}



//****************************************************************


//

// Process a received GameChatMessage structure.

//

ErrorType ProcessGameChatMessage(struct GameChatMessage *gcm, int input_structure_len)

{

	pr(("%s(%d) We received a struct GameChatMessage\n",_FL));



	if (sizeof(*gcm) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) GameChatMessage structure was wrong length (%d vs %d)",_FL,sizeof(*gcm),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	//  hack to figure out if these messages came from tournament tables

	if (gcm->text_type == CHATTEXT_MONITOR_PLAY && !strstr(gcm->message,"Play money ")) {

		// it's actually a tournament table -- change the message type

		gcm->text_type = CHATTEXT_MONITOR_TOURN;

	}

	// -- modified/externalized processing of chat messages

	// -- added admin chat monitoring

	if (gcm->text_type == CHATTEXT_MONITOR_PLAY || 

		gcm->text_type == CHATTEXT_MONITOR_REAL ||

		gcm->text_type == CHATTEXT_MONITOR_TOURN ||

		(gcm->text_type >= ALERT_1 && gcm->text_type <= ALERT_10))

	{

      #if ADMIN_CLIENT

		if (iAdminClientFlag) {

			if (UpdateAdminChatBuffer(gcm)) {	// returns TRUE if we need to refresh

				PostMessage(hCardRoomDlg, WMP_REFRESH_MONITOR_CHAT, 0, 0);

			}

		}

	  #endif

	} else {	// regular chat

		int table_index = TableIndexFromSerialNumber(gcm->table_serial_number);

		if (table_index < 0) {

			return ERR_ERROR;

		}

		if (UpdateChatBuffer(gcm, table_index)) {	// returns TRUE if we need to refresh

			PostMessage(Table[table_index].hwnd, WMP_REFRESH_CHAT, 0, 0);

		}

	}



  #if ADMIN_CLIENT && 0	// This code has been removed because it waits for the message thread - potential hang!

	if (iAdminClientFlag) {

		struct TableInfo *t = &Table[table_index];

		int check_state = IsDlgButtonChecked(t->hwnd, IDC_CHAT_POPUP);

		if (check_state == BST_CHECKED || (check_state == BST_INDETERMINATE && NameIsInMsg(gcm->message, LoginUserID)) ) {

			// activate it if box is checked, or box is half checked and name is in it

			if (stricmp(gcm->name, "Dealer")) {	// ignore dealer messages

				ShowTableWindow(t->table_serial_number, FALSE);

				// Make the focus on that table the chat edit window

				PostMessage(t->hwnd, WMP_SET_CHAT_FOCUS, 0, 0);

				PPlaySound(SOUND_WAKE_UP);		// 'bigger' sound

			}

		}

	}

  #endif

	return ERR_NONE;

}



//****************************************************************

//  - MB

//

// Process a received struct CardRoom_TableSummarySerialNums.

//

ErrorType ProcessCardRoomSerialNums(struct CardRoom_TableSummarySerialNums *p, int input_structure_len)

{

	if (sizeof(*p) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) CardRoom_TableSummarySerialNums structure was wrong length (%d vs %d)",_FL,sizeof(*p),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	pr(("%s(%d) Summary serial nums = %3d,%3d,%3d,%3d,%3d,%3d,%3d\n", _FL,

			p->serial_num[0],

			p->serial_num[1],

			p->serial_num[2],

			p->serial_num[3],

			p->serial_num[4],

			p->serial_num[5],

			p->serial_num[6]));



	// Keep track of most recent serial numbers from server.

	Server_TableSerialNums = *p;



	// If we are watching one of the game types that changed,

	// request the new info from the server now.

	if (hCardRoomDlg && !iCardRoomDlgMinimizedFlag) {

		CheckForNewGameList(GameListTabIndex);

	}



	return ERR_NONE;

}



//****************************************************************

// - MB

//

// Process a received struct CardRoom_TableSummaryList.

//

ErrorType ProcessCardRoomTableList(Packet **pkt)

{

	struct DataPacketHeader *hdr = (struct DataPacketHeader *)(*pkt)->user_data_ptr;

	struct CardRoom_TableSummaryList *p = (struct CardRoom_TableSummaryList *)

				((char *)(*pkt)->user_data_ptr+sizeof(struct DataPacketHeader));



	int expected_length = sizeof(struct CardRoom_TableSummaryList) +

				  sizeof(struct CardRoom_TableSummaryInfo) * (p->table_count-1);

	if (hdr->data_packet_length != expected_length) {

		Error(ERR_ERROR, "%s(%d) CardRoom_TableSummaryList structure was wrong length (%d vs %d)",_FL,expected_length,hdr->data_packet_length);

		return ERR_ERROR;	// do not process.

	}

	if (p->client_display_tab_index >= MAX_CLIENT_DISPLAY_TABS) {

		Error(ERR_ERROR, "%s(%d) Illegal client_display_tab_index (%d) in CardRoom_TableSummaryList",_FL, p->client_display_tab_index);

		return ERR_ERROR;	// do not process.

	}



    // Validate the game rules for each CardRoom_TableSummaryInfo structure.

	for (int i=0 ; i<(int)p->table_count ; i++) {

        ValidateGameRules(p->tables[i].client_display_tab_index, &p->tables[i].game_rules);

    }

  #if DISP

	kp(("%s(%d) TableSummaryList received.  Gametype = %d, %d tables\n",_FL,p->game_type,p->table_count));

	for (int i=0 ; i<(int)p->table_count ; i++) {

		kp(("%s(%d) table '%-10.10s': $%2d/$%2d  %d players  avg pot=$%d  hands/hr=%d  plrs/flop=%d%%\n",_FL,

				p->tables[i].table_name,

				p->tables[i].small_blind_amount,

				p->tables[i].big_blind_amount,

				p->tables[i].player_count,

				p->tables[i].avg_pot_size,

				p->tables[i].hands_per_hour,

				p->tables[i].players_per_flop));

	}

  #endif

	EnterCriticalSection(&CardRoomVarsCritSec);

	if (TableSummaryListPackets[p->client_display_tab_index]) {

		delete TableSummaryListPackets[p->client_display_tab_index];

		TableSummaryListPackets[p->client_display_tab_index] = NULL;

	}

	TableSummaryListPackets[p->client_display_tab_index] = *pkt;



	// Copy a few of the most recent fields to every game type.  This

	// prevents displaying the old out of date fields when the user

	// first clicks on a different game type.

	struct CardRoom_TableSummaryList *ts = (struct CardRoom_TableSummaryList *)

				((char *)(TableSummaryListPackets[p->client_display_tab_index]->user_data_ptr)+sizeof(struct DataPacketHeader));



	pr(("%s %s(%d) TableSummaryList received: active_tables = %3d, real_active_tables = %3d (diff=%3d)\n",

			TimeStr(), _FL, ts->active_tables, ts->active_real_tables, ts->active_tables - ts->active_real_tables));

	// only copy it if it looks somewhat valid.

	if (ts->total_players && strlen(ts->shotclock_msg1)) {

		for (int j=0 ; j<MAX_CLIENT_DISPLAY_TABS ; j++) {

			if (TableSummaryListPackets[j]) {

				struct CardRoom_TableSummaryList *ts2 = (struct CardRoom_TableSummaryList *)

					((char *)(TableSummaryListPackets[j]->user_data_ptr)+sizeof(struct DataPacketHeader));

				ts2->total_players = ts->total_players;

				ts2->active_tables = ts->active_tables;

				ts2->active_real_tables = ts->active_real_tables;

				ts2->unseated_players = ts->unseated_players;

				ts2->number_of_accounts = ts->number_of_accounts;

				ts2->idle_players = ts->idle_players;

				ts2->money_in_play = ts->money_in_play;

				ts2->money_logged_in = ts->money_logged_in;

			}

		}

		dwMoneyInPlay   = ts->money_in_play;

		dwMoneyLoggedIn = ts->money_logged_in;

	}



	*pkt = NULL;	// caller is no longer responsible for this packet.

	LeaveCriticalSection(&CardRoomVarsCritSec);



	// Post a message for the list view control to update itself

	if (hCardRoomDlg) {

		// If the user has not selected a suitable default table,

		// select one for him now.

		if (!dwTableInfoSerialNum) {

			dwTableInfoSerialNum = SelectDefaultTable(GameListTabIndex);

			if (dwTableInfoSerialNum) {

				// If we found one, request and subscribe to the table's info

				RequestTableInfo(dwTableInfoSerialNum, TRUE);

			}

		}



		PostMessage(hCardRoomDlg, WMP_UPDATE_GAME_LIST, 0, (LPARAM)p->client_display_tab_index);

	}



  #if ADMIN_CLIENT

	// Debug: auto-join a table if we aren't joined to any

	if (AutoJoinDefaultTable) {

		int table_count = 0;

		for (int i=0 ; i<MAX_TABLES ; i++) {

			if (Table[i].table_serial_number) {

				table_count++;	// we're joined to this one.

			}

		}

		if (!table_count) {

			// We're not joined to anything.

			WORD32 autojointable = SelectDefaultTable(Defaults.preferred_display_tab_index, TRUE);

			if (autojointable) {

				//kp(("%s(%d) We're not joined to any tables... choosing %d and asking to sit down.\n", _FL, autojointable));

				// We found a suitable default table.

				// Send a DATATYPE_CARDROOM_JOIN_TABLE packet.

				struct CardRoom_JoinTable jt;

				zstruct(jt);

				jt.table_serial_number = autojointable;

				jt.status = 1;	// request to sit down at this table.

				jt.buy_in_chips = (unsigned)1000000; 	// buy in with lots

				jt.seating_position = (BYTE8)-1;		// sit anywhere.
               
				jt.flags |= JOINTABLE_FLAG_AUTOJOIN;	// indicate this is an automatic request and not worth logging errors

				SendDataStructure(DATATYPE_CARDROOM_JOIN_TABLE, &jt, sizeof(jt));

			}

		}

	}

  #endif //ADMIN_CLIENT



	return ERR_NONE;

}



//****************************************************************


//

// Process a received struct CardRoom_TableInfo.

//

ErrorType ProcessCardRoomTableInfo(Packet **pkt)

{

	pr(("%s(%d) ProcessCardRoomTableInfo() called.\n",_FL));

	struct DataPacketHeader *hdr = (struct DataPacketHeader *)(*pkt)->user_data_ptr;

	struct CardRoom_TableInfo *p = (struct CardRoom_TableInfo *)((char *)(*pkt)->user_data_ptr+sizeof(struct DataPacketHeader));



	if (sizeof(*p) != hdr->data_packet_length) {

		Error(ERR_ERROR, "%s(%d) CardRoom_TableInfo structure was wrong length (%d vs %d)",_FL,sizeof(*p),hdr->data_packet_length);

		return ERR_ERROR;	// do not process.

	}

	EnterCriticalSection(&CardRoomVarsCritSec);

	UpdateTableInfoEntry(p);

	delete *pkt;	// we're done with it.

	*pkt = NULL;

	LeaveCriticalSection(&CardRoomVarsCritSec);

	NOTUSED(p);

	return ERR_NONE;

}



//****************************************************************

//

// Process a received struct ShutDownMsg structure

// DATATYPE_CLOSING_CONNECTION

//

ErrorType ProcessConnectionClosing(struct ConnectionClosing *cc, int input_structure_len)

{

	if (sizeof(*cc) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) ConnectionClosing was wrong length (%d vs %d)",_FL,sizeof(*cc),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	// Close our connection.

	if (ClientSock) {

		ClientSock->CloseSocket();

	}



	if (cc->reason==1) {

		// Someone else logged into our account.

		AutoLoginFlag = FALSE;

		char str[20], msg[200];

		IP_ConvertIPtoString(cc->new_ip_address, str, 20);

		sprintf(msg,"Your connection here has been logged off\n"

					"another machine has logged into the same\n"

					"account. From: %s\n\n"

					"You may only connect from one computer at\n"

					"a time.", str);

		MessageBox(hCardRoomDlg, msg, "e-Media Poker Logoff Notice", MB_OK|MB_ICONSTOP|MB_TOPMOST);

	} else if (cc->reason==2) {

		// Blocked (or similar error code)... don't try to reconnect.

		iDisableServerCommunications = TRUE;

		EnterCriticalSection(&ClientSockCritSec);

		if (ClientSock) {

			ClientSock->CloseSocket();

		}

		LeaveCriticalSection(&ClientSockCritSec);

		

		if (cc->error_code == 615) {

            cc->error_code=17;

		}

		char msg[200];

		sprintf(msg,"Cannot connect to server. Error code %d.", cc->error_code);

		MessageBox(hCardRoomDlg, msg, "Connection problem", MB_OK|MB_ICONSTOP|MB_TOPMOST);

		 

		//MessageBox(hCardRoomDlg, "Unable to connect to server", "Connection problem", MB_OK|MB_ICONSTOP|MB_TOPMOST);

	}

	return ERR_NONE;

}



//****************************************************************

//

// Process a received struct CardRoom_JoinTable structure

// DATATYPE_CARDROOM_JOIN_TABLE

//

ErrorType ProcessJoinTableResult(struct CardRoom_JoinTable *jt, int input_structure_len)

{

  #if 0

	kp(("%s(%d) Critical sections owned by this thread at ProcessJoinTableResult():\n",_FL));

	DisplayOwnedCriticalSections();

  #endif



	if (sizeof(*jt) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) JoinTable result was wrong length (%d vs %d)",_FL,sizeof(*jt),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	// Verify some parms...

	if (jt->status > 2) {

		Error(ERR_ERROR, "%s(%d) JoinTable result status was illegal (%d)", _FL, jt->status);

		return ERR_ERROR;	// do not process.

	}



    ValidateGameRules(jt->client_display_tab_index, &jt->game_rules); // make sure game_rules is valid

	// jt->status values: 0=unjoin, 1=join, 2=watch

  #if 0

	char *action = "unjoined from";

	if (jt->status==1)

		action = "joined to";

	else if (jt->status==2)

		action = "watching";

	kp(("%s %s(%d) Server tells us we're %s table %d\n", TimeStr(), _FL, action, jt->table_serial_number));

  #endif



	CloseGoingToTableDialog();	// no matter what the result, we're not on our way anymore



	iSeatedAtATournamentTableFlag = (jt->flags & JOINTABLE_FLAG_SEATED_AT_TOURNAMENT) ? TRUE : FALSE;



	// Add this to the pending queue and tell the message thread it is now there.

	struct CardRoom_JoinTable *jt2 = PendingJoinTableResults;

	for (int i=0 ; i<MAX_TABLES ; i++, jt2++) {

		if (!jt2->table_serial_number) {	// found a blank spot for it...

			*jt2 = *jt;

			break;	// all done.

		}

	}

	PostMessage(hInitialWindow, WMP_OPEN_TABLE_WINDOW, 0, 0);



	if (jt->status) {

		// Now this is kind of a hack... block the comm thread for up to 3s so

		// that the window can get opened, otherwise we get the GameCommonData

		// for this table before we have anywhere to put it.

		WORD32 abort_ticks = GetTickCount() + 3000;

		while (GetTickCount() < abort_ticks) {

			int i = TableIndexFromSerialNumber(jt->table_serial_number);

			if (i >= 0) {

				Table[i].ClientState.leave_table = FALSE;	//20000921MB: always clear when opening.

				SendClientStateInfo(&Table[i]);		// Always send our ClientState asap

				break;	// done.

			}

			Sleep(100);

		}

	}

	return ERR_NONE;

}



//****************************************************************

//

// Process a received struct CardRoom_JoinedTables structure

// DATATYPE_CARDROOM_JOINED_TABLES

//

ErrorType ProcessJoinedTables(struct CardRoom_JoinedTables *jt, int input_structure_len)

{

	if (sizeof(*jt) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) struct JoinedTables was wrong length (%d vs %d)",_FL,sizeof(*jt),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	// Make sure we're joined to all these tables.  Close any which are

	// not on this list.

	int i;

  #if 0	

	kp(("%s(%d) Server says we're joined to these tables: ", _FL));

	for (i=0 ; i<MAX_GAMES_PER_PLAYER ; i++) {

		if (jt->table_serial_numbers[i]) {

			kp((" %d", jt->table_serial_numbers[i]));

		}

	}

	kp(("\n"));

  #endif



	int confirmed[MAX_GAMES_PER_PLAYER];

	memset(confirmed, 0, sizeof(confirmed[0])*MAX_GAMES_PER_PLAYER);

	for (i=0 ; i<MAX_GAMES_PER_PLAYER ; i++) {

		if (jt->table_serial_numbers[i]) {

		int index = TableIndexFromSerialNumber(jt->table_serial_numbers[i]);

			if (index >= 0) {

				confirmed[index] = TRUE;

			} else {

				//kp(("%s(%d) Warning... server thinks we should be joined to table %d\n", _FL, jt->table_serial_numbers[i]));

			}

		}

	}



	// For any that weren't confirmed, close them now.

	int closed_one = FALSE;

	for (i=0 ; i<MAX_GAMES_PER_PLAYER ; i++) {

		if (Table[i].hwnd && !confirmed[i]) {

			//kp(("%s(%d) Server says we're not joined to table %d. Closing window.\n", _FL, Table[i].table_serial_number));

			PostMessage(Table[i].hwnd, WMP_CLOSE_YOURSELF, 0, 0);

			closed_one = TRUE;

		}

	}

	CloseGoingToTableDialog();

	if (hCardRoomDlg && closed_one) {

		PostMessage(hCardRoomDlg, WMP_SHOW_YOURSELF, 0, 0);

	}



	return ERR_NONE;

}



//****************************************************************


//

// Process a received struct MiscClientMessage structure

// DATATYPE_MISC_CLIENT_MESSAGE

//

ErrorType ProcessMiscClientMsg(struct MiscClientMessage *mcm, int input_structure_len)

{

	if (sizeof(*mcm) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) MiscClientMsg was wrong length (%d vs %d)",_FL,sizeof(*mcm),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	//kp(("%s(%d) Received MiscClientMessage structure.  Message_type = %d, misc_data_1 = %d\n",_FL, mcm->message_type, mcm->misc_data_1));



	// If it was for a particular table, notify that table, otherwise

	// just let the cardroom handle it.

	

	struct TableInfo *t = NULL;

	if (mcm->table_serial_number) {

		t = TablePtrFromSerialNumber(mcm->table_serial_number);

	}



	// do any special handling for this type of message

	switch (mcm->message_type) {

	case MISC_MESSAGE_SET_COMPUTER_SERIAL_NUM:

		// The server is assigning us a computer serial number.

		// Keep it somewhere safe and obscure.

		kp(("%s(%d) Server has assigned us computer serial number %d\n", _FL, mcm->misc_data_1));

		SaveComputerSerialNumber(mcm->misc_data_1);

		return ERR_NONE;



	case MISC_MESSAGE_ECASH:

		//int tmpretval;

		//int rc;

		char str[300];

		char vplayerID[9];



		LPTSTR p;

        p=strstr( mcm->msg,"APPROVED");

		if (p!=NULL) {

		EndDialog(hCCPurchaseDLG, 1);

		p=p+ 8;

        

		for (int i=0;i<8;i++)

		{   

		   vplayerID[i]=mcm->msg[8 + i];

		}

		

		vplayerID[8]=0;



		    /*

        	sprintf(str, "Player ID: %s\n\nEmail: %s\n\nMailing Address:\n\n"

					"   %s\n"

					"   %s\n"

					"%s%s%s"

					"   %s, %s, %s\n",

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

					LoggedInAccountRecord.sdb.mailing_address_postal_code

			);

			*/



		FILE *stream;

		//FILE *stream2;

	    //int numclosed;

        

		/* Open for read (will fail if file "data" does not exist) */

		stream  = fopen( "pay.html", "w" );

        

        fputs( "<FORM NAME=\"form1\" METHOD=\"POST\" ACTION=\"https://www.paypal.com/cgi-bin/webscr\">\n", stream );

        fputs( "<INPUT TYPE=hidden NAME=cmd VALUE=\"_ext-enter\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=redirect_cmd VALUE=\"_xclick\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=business VALUE=\"paypal@e-mediasoftware.com\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=item_name VALUE=\"e-Media Poker Chip Purchase\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=no_shipping VALUE=\"1\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=no_note VALUE=\"1\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=return VALUE=\"\">\n", stream );

		fputs( "<INPUT TYPE=hidden NAME=cancel_return VALUE=\"http://www.e-mediasoftware.com\">\n", stream );

		//sprintf(str, "<INPUT TYPE=hidden NAME=amount VALUE=\" %s\">\n",

		//			samount);

		

		sprintf(str, "<INPUT TYPE=hidden NAME=amount VALUE=\" %s\">\n",

					samount);

		fputs( str, stream );

		//fputs( "<INPUT TYPE=hidden NAME=amount VALUE=\"50\">\n", stream );

        sprintf(str, "<INPUT TYPE=hidden NAME=custom VALUE=\"%s\">\n",

				 vplayerID);

		fputs( str, stream );

 

        sprintf(str, "<INPUT TYPE=hidden NAME=invoice VALUE=\"%s\">\n",

				p);

		

		//fputs( "<INPUT TYPE=hidden NAME=invoice VALUE=\"pp-121e07-18831350\">\n", stream );

		fputs( str, stream );



		sprintf(str, "<INPUT TYPE=hidden NAME=first_name VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.full_name);

		//fputs( "<INPUT TYPE=hidden NAME=first_name VALUE=\"alfredkou\">\n", stream );

		fputs( str, stream );

		//fputs( "<INPUT TYPE=hidden NAME=last_name VALUE=\"\">\n", stream );

		sprintf(str, "<INPUT TYPE=hidden NAME=address1 VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address1);

		fputs( str, stream );

		sprintf(str, "<INPUT TYPE=hidden NAME=address2 VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address2);

		fputs( str, stream );

		

		sprintf(str, "<INPUT TYPE=hidden NAME=city VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.city);

					

		fputs( str, stream );

					

		

		sprintf(str, "<INPUT TYPE=hidden NAME=state VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address_state);

					

		fputs( str, stream );



		sprintf(str, "<INPUT TYPE=hidden NAME=country VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address_country);

					

		fputs( str, stream );

		

		sprintf(str, "<INPUT TYPE=hidden NAME=zip VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address_postal_code);

					

		fputs( str, stream );



		

		

		//fputs( "<INPUT TYPE=hidden NAME=address1 VALUE=\"128,wyndhamstr st\">\n", stream );

		//fputs( "<INPUT TYPE=hidden NAME=address2 VALUE=\"\">\n", stream );

		//fputs( "<INPUT TYPE=hidden NAME=city VALUE=\"montreal\">\n", stream );

		//fputs( "<INPUT TYPE=hidden NAME=state VALUE=\"ON\">\n", stream );



		//fputs( "<INPUT TYPE=hidden NAME=zip VALUE=\"M6K 1R7\">\n", stream );

		sprintf(str, "<INPUT TYPE=hidden NAME=zip VALUE=\" %s\">\n",

					LoggedInAccountRecord.sdb.mailing_address_postal_code);

					

		fputs( str, stream );

		

		fputs( "<SCRIPT LANGUAGE=\"javascript\">\n", stream );

		fputs( "document.form1.submit();\n", stream );

		fputs( "</SCRIPT>\n", stream );

		fputs( "</FORM>\n", stream );





	    /* Close stream */

        fclose( stream );



		LaunchInternetBrowser("pay.html");



		}

		else

		{   LPTSTR p;

        	p=strstr( mcm->msg,"online notification");

			if (p!=NULL)

			{

			  // DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_PROCESS), NULL,dlgFuncWesternUnion );

			}

            

			p=strstr( mcm->msg,"Minimum");

			

			if (p!=NULL)

			{

				p=strstr(mcm->msg,"Please note: Minimum cashout is");

				if (p!=NULL)

                   ; //DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_DECLINE_SMALL1), NULL,dlgFuncWesternUnion );

				else

			       ; // DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_DECLINE_SMALL), NULL,dlgFuncWesternUnion );

			}



			p=strstr( mcm->msg,"maximum");

			strcpy(pstr,mcm->msg);

			if (p!=NULL)

			{

			  //DialogBox(hInst, MAKEINTRESOURCE(IDD_TRANSACTION_DECLINE_LARGE), NULL,dlgFuncTransactionLarge );

			}

			p=strstr(mcm->msg,"PayPal");

			if (p!=NULL)

			{ 

			  p = strstr(mcm->msg,"returned");

			  if (p!= NULL) {

				  MessageBox(NULL, mcm->msg, "Refund Transaction Completed.", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);

			  } else {

				

				  MessageBox(NULL, mcm->msg, "PayPal Transaction Completed.", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);



			  }

			} else {	// 

			  p=strstr(mcm->msg, "Approved");

			  if (p!=NULL) {

				  memcpy(p,"APPROVED",8);

				  EndDialog(hCCPurchaseDLG,1);

			  }

			  p=strstr(mcm->msg, "received");

			  if (p!=NULL){

				  EndDialog(hCCPurchaseDLG,1);

			  }

			  p=strstr(mcm->msg, "most recent");

			  if (p!=NULL) {

				MessageBox(NULL , mcm->msg, "Cash-Out Request", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);

			  } else {

				MessageBox(NULL , mcm->msg, "Credit Card Transaction", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);	// Tony, Dec 21, 2001

			  }	

			}



		}



		break;



		/*

		MessageBox(NULL, "Please enter an amount greater than 50\nThe minimum amount of deposit is 50 US dollars.", mcm->msg, 

		MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);  

      	break;

		

				}

		char * tmpchar;

        tmpchar= strstr(mcm->msg, "DECLINED");

		if (*tmpchar=='D')

		{

		

		MessageBox(NULL, mcm->msg, " ",

		MB_OKCANCEL|MB_APPLMODAL|MB_TOPMOST);  

      	break;

		

		}

        */	

		pr(("%s(%d) ecash message arrived -- do extra handling here if needed\n", _FL));

		break;



	case MISC_MESSAGE_CREATEACCOUNT_RESULT:

		HandleCreateAccountResult(mcm);

		return ERR_NONE;



	case MISC_MESSAGE_GHOST:

		// remove the "going to table" dlg if it exists...

		if (hGoingToTableDlg) {

			PostMessage(hGoingToTableDlg, WMP_CLOSE_YOURSELF, 0, 0);

		}

		// since we get this when we've failed going to a table, restore the cardroom

		ShowWindow(hCardRoomDlg, SW_SHOWNORMAL);

		ReallySetForegroundWindow(hCardRoomDlg);

		break;



	case MISC_MESSAGE_REQ_TOURN_SUMMARY_EMAIL:

		if (t && t->hwnd) {	// if he closed the window, don't send it

			t->misc_client_msg = *mcm;

			PostMessage(t->hwnd, WMP_REQUEST_TOURNAMENT_EMAIL, 0, 0);

		}

		return ERR_NONE;



	case MISC_MESSAGE_INSUFFICIENT_CHIPS:

		if (t) {

			BuyInDLGInfo *bi = (struct BuyInDLGInfo *)(malloc(sizeof(BuyInDLGInfo)));

			zstruct(*bi);

			bi->seating_position = mcm->misc_data_1;

			bi->minimum_allowed = mcm->misc_data_2;

			t->minimum_buy_in = bi->minimum_allowed;

			bi->chips = ((t->chip_type == CT_REAL) ? RealChipsInBank : FakeChipsInBank);

			bi->ti = t;

			int update_only = mcm->misc_data_3;	// updating numbers or actually buying?

			if (!update_only) {

				PostMessage(t->hwnd, WMP_BUY_MORE_CHIPS, 0, (LPARAM)bi);

			} else {

				pr(("%s(%d) received t->minimum_buy_in update of %d\n", _FL, t->minimum_buy_in));

				return ERR_NONE;	// nothing to pop up

			}

		}

		break;



	case MISC_MESSAGE_SERVER_FULL:

		iDisableServerCommunications = TRUE;

		EnterCriticalSection(&ClientSockCritSec);

		if (ClientSock) {

			ClientSock->CloseSocket();

		}

		LeaveCriticalSection(&ClientSockCritSec);

		break;

	case MISC_MESSAGE_CC_BUYIN_LIMITS:

		MiscMsgCCBuyinLimits = *mcm;	// just save it

		return ERR_NONE;

	case MISC_MESSAGE_TABLE_SITDOWN:

		if (t) {

			//kp(("%s(%d) Saving table sit-down message for table %d\n", _FL, t->table_serial_number));

			t->MiscMsgSitdown = *mcm;	// just save it.

		} else {

			kp(("%s(%d) Warning: got table sitdown message but had nowhere to save it!\n",_FL));

		}

		return ERR_NONE;

	case MISC_MESSAGE_CREATE_ACCOUNT_WARNING:

		MiscMsgCreateAccountWarning = *mcm;	// just save it

		return ERR_NONE;

	}



	 

     

	

	if ((mcm->message_type == MISC_MESSAGE_CL_REQ_BROADCAST_MESSAGE) ||

		(mcm->message_type == MISC_MESSAGE_UNSPECIFIED) )

	{	// for the cashier?

	if (t && t->hwnd) {	// give it to a table.

		t->misc_client_msg = *mcm;

		PostMessage(t->hwnd, WMP_MISC_CLIENT_MSG, 0, 0);

	} else {	// give it to the cardroom.

		CardRoom_misc_client_msg = *mcm;

		PostMessage(hCardRoomDlg ? hCardRoomDlg : hInitialWindow, WMP_MISC_CLIENT_MSG, 0, 0);

	}

	}

	return ERR_NONE;

}



//*********************************************************

// 

//

// Process a received struct Ping structure

// DATATYPE_PING

//

ErrorType ProcessPing(struct Ping *ping, int input_structure_len)

{

	if (sizeof(*ping) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) Ping was wrong length (%d vs %d)",_FL,sizeof(*ping),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	// If it came from the server, send it back.

	if (ping->flags & 0x01) {	// client to server and back?

		// It was one of ours...

		kp(("%s(%d) Elapsed time for ping from client to server and back: %dms\n", _FL, GetTickCount() - ping->local_ms_timer));

	} else {

		// It originated on the server... send it back.

		//kp(("%s(%d) Sending ping back to server.\n",_FL));

		SendDataStructure(DATATYPE_PING, ping, sizeof(*ping));

	}

	return ERR_NONE;

}



//****************************************************************

// 

//

// Process a received struct CardRoom_SeatAvail structure

// DATATYPE_CARDROOM_SEAT_AVAIL

//

ErrorType ProcessSeatAvail(struct CardRoom_SeatAvail *sa, int input_structure_len)

{

	if (sizeof(*sa) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) SeatAvail was wrong length (%d vs %d)",_FL,sizeof(*sa),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	pr(("%s(%d) Received SeatAvail: table=%d, timeout=%ds, players=%d\n",

				_FL, sa->table_serial_number, sa->timeout,

				sa->number_of_players));



	// Must match current SeatAvail, otherwise we'll ignore it.

	// (because we only handle one at a time).

	if (!SeatAvail.table_serial_number ||

		!sa->table_serial_number ||			//: allow 'cancel any' from server

		 SeatAvail.table_serial_number==sa->table_serial_number) {

		// If it's a perfect match with the old one, don't save it.

		if (memcmp(sa, &previous_seat_avail, sizeof(*sa))) {

			// Doesn't match... normal processing.

			previous_seat_avail = *sa;

			SeatAvail = *sa;

			SeatAvailSecondCounter = SecondCounter;

			PostMessage(hCardRoomDlg, WMP_SEAT_AVAIL, 0, 0);

		}

	} else {

		Error(ERR_WARNING, "%s(%d) Ignoring additional SeatAvail notifications for other tables (new table=%d, oldtable=%d)", _FL, sa->table_serial_number, SeatAvail.table_serial_number);

	}

	return ERR_NONE;

}



//*********************************************************


//

// Process a received struct VersionInfo structure from the Server.

// It describes the current server version and the requirements

// for the client version, as well as info about any new client

// versions.

//

ErrorType ProcessServerVersionInfo(struct VersionInfo *vi, int input_structure_len)

{
	if (sizeof(*vi) != input_structure_len) {
		Error(ERR_ERROR, "%s(%d) received VersionInfo structure is wrong size (%d vs %d). Ignoring.",

				_FL, input_structure_len, sizeof(struct VersionInfo));
		return ERR_ERROR;	// do not process.
	};//if

	// Save Server's VersionInfo

	ServerVersionInfo = *vi;
	// Create a 'ip string' which indicates where we're connected from.
	// Use two IPs if the ip address the server sent does not match our own.
	IP_ConvertIPtoString(vi->source_ip, szOurIPString, 20);
	WORD32 our_local_ip = vi->source_ip;
	if (ClientSock) {
		SOCKADDR_IN sa;
		zstruct(sa);
		int bytes = sizeof(sa);

		if (!getsockname(ClientSock->sock, (SOCKADDR *)&sa, &bytes)) {
			// no error... use it.
			our_local_ip = sa.sin_addr.s_addr;
			OurLocalIP = our_local_ip;

		}

	}

	if (vi->source_ip != our_local_ip) {

		strcat(szOurIPString, " / ");
		IP_ConvertIPtoString(our_local_ip, szOurIPString+strlen(szOurIPString), 20);

	}

	TimeDiffWithServer = 0;
	if (vi->server_time) {
		TimeDiffWithServer = vi->server_time - time(NULL);
	}



  #if 0	//19990522MB

  	{

		kp(("%s(%d) Server is '%s' version %d.%02d (build %d/$%08lx)\n",_FL,

				ServerVersionInfo.server_version_string,

				ServerVersionInfo.server_version.major,

				ServerVersionInfo.server_version.minor,

				ServerVersionInfo.server_version.build & 0x0000FFFF,

				ServerVersionInfo.server_version.build));

		kp(("%s(%d) Min client required is %d.%02d (build %d/$%08lx).\n",_FL,

				ServerVersionInfo.min_client_version.major,

				ServerVersionInfo.min_client_version.minor,

				ServerVersionInfo.min_client_version.build & 0x0000FFFF,

				ServerVersionInfo.min_client_version.build));

		struct tm *t = localtime((time_t *)&ServerVersionInfo.new_version_release_date);

		kp(("%s(%d) Newest client is %d.%02d (build %d/$%08lx) ('%s') released %d/%02d/%02d@%d:%02d:%02d\n", _FL,

				ServerVersionInfo.new_client_version.major,

				ServerVersionInfo.new_client_version.minor,

				ServerVersionInfo.new_client_version.build & 0x0000FFFF,

				ServerVersionInfo.new_client_version.build,

				ServerVersionInfo.new_version_string,

				t->tm_year, t->tm_mon+1, t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec));

		kp(("%s(%d) For more info go to: %s\n",_FL,

				ServerVersionInfo.new_ver_user_url));

		kp(("%s(%d) Use %s for auto-upgrading.\n",_FL,

				ServerVersionInfo.new_ver_auto_url));

		kp(("%s(%d) iRunningLiveFlag on server is%sset.\n", _FL,

				ServerVersionInfo.server_version.flags & VERSIONFLAG_RUNNINGLIVE ? " " : " not "));

	}

  #endif





	// If we're not connected to the correct server, switch now.

	if (ServerVersionInfo.alternate_server_ip) {

		kp(("%s(%d) Switching to server at $%08lx\n", _FL, ServerVersionInfo.alternate_server_ip));
		Defaults.last_server_ip_address = ServerVersionInfo.alternate_server_ip;
		EnterCriticalSection(&ClientSockCritSec);

		if (ClientSock) {
			ClientSock->CloseSocket();	// force a reconnect
		}

		LeaveCriticalSection(&ClientSockCritSec);
		return ERR_NONE;	// don't perform the version minimum check

	}





  #if 1
	PostMessage(hInitialWindow, WMP_PROCESS_VERSION_INFO, 0, 0);
  #else

  	// If there is a new version of the client or if our version

	// is too old, notify the user and ask how to proceed.

	if (ClientVersionNumber.build < ServerVersionInfo.min_client_version.build ||
		ClientVersionNumber.build < ServerVersionInfo.new_client_version.build) {
	  #if ADMIN_CLIENT

		if (!RunningManyFlag)

	  #endif

		{
			pr(("%s(%d) A new client version is available.\n",_FL));
			if (!AutoUpgradeDisable) {
				PostMessage(hInitialWindow, WMP_PROCESS_VERSION_INFO, 0, 0);
			}
		}
	}

  #endif

	
	//Robert 02/28/2002
	

	if (
	/*	(ClientVersionNumber.major  & 0x0000FFFF )< (ServerVersionInfo.min_client_version.major  & 0x0000FFFF) &&

		(ClientVersionNumber.minor  & 0x0000FFFF )>= (ServerVersionInfo.min_client_version.minor  & 0x0000FFFF) {
		
      */0){ 

		//Robert 03/22/2002
		 iDisableServerCommunications = FALSE;
		 EnterCriticalSection(&ClientSockCritSec);
		 if (ClientSock) {
			 ClientSock->CloseSocket();
		 };//if
		 LeaveCriticalSection(&ClientSockCritSec);
		
		 EnterCriticalSection(&CardRoomVarsCritSec);
		 int res; 
		
		 if (upgrade_yes == false){
			res=MessageBox(hCardRoomDlg, "New e-media Poker Version Found \n Do you want to process the upgrader ?", "e-Media Poker Upgrader",MB_YESNO);
		 }
 		 LeaveCriticalSection(&CardRoomVarsCritSec);

		 if(res==IDYES){ //if to process the upgrader
			EnterCriticalSection(&CardRoomVarsCritSec);
		     upgrade_yes = true;
			EnterCriticalSection(&CardRoomVarsCritSec);
			 CheckForUpgrade(FALSE);
		 }		
		 

			//CheckForUpgrade(TRUE);

/*		
		HWND hUpdateDlg;

		hUpdateDlg=CreateDialog(hInst, MAKEINTRESOURCE(IDD_DOWNLOAD_INQUIRY), NULL, (DLGPROC)dlgDownLoadClient);

		ShowWindow(hUpdateDlg, SW_SHOW);

		ErrorType err = ERR_NONE;

		HINTERNET inet_hndl = InternetOpen(szAppName,INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

		if (!inet_hndl) {
			MessageBox(NULL, "Internet connection error with the website.", "Client Software Update", MB_OK);
			PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		}

		err = WriteFileFromUrl(inet_hndl, "http://www.e-mediasoftware.com/client_downloads/update.exe", "wiseupdt.exe");
		//err = WriteFileFromUrlUsingSockets("http://www.e-mediasoftware.com/client_downloads/update.exe", "wiseupdt.exe", WriteFileFromURLCallback, 80);
		if (err==ERR_NONE) {

			//ShowWindow(hUpgradeStatusDlg, SW_SHOW);
			//if (hUpgradeStatusDlg) {
			WinExec("wiseupdt.exe", SW_SHOW);								
			PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
				//}

		} else {

			MessageBox(NULL, "Client Software Update download error!", "Client Software Update", MB_OK);
			PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		}
		
		//PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		//End Robert Gong
		*/
	}

	return ERR_NONE;

	
}



//****************************************************************


//

// Process a received struct ShutDownMsg structure

// DATATYPE_SHUTDOWN_MSG

//

ErrorType ProcessShutDownMsg(struct ShutDownMsg *sdm, int input_structure_len)

{

	if (sizeof(*sdm) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) ShutDownMsg was wrong length (%d vs %d)",_FL,sizeof(*sdm),input_structure_len);

		return ERR_ERROR;	// do not process.

	}



	// Close our connection.

	if (ClientSock) {

		ClientSock->CloseSocket();

	}



  #if ADMIN_CLIENT

	if (sdm->shutdown_client_flag) {	// Debug: should we shut down as well?

		// For now, just shut down when we get this message.  This

		// is obviously not good enough for the final client.

		if (hCardRoomDlg) {

			PostMessage(hCardRoomDlg, WMP_CLOSE_YOURSELF, TRUE, 0);

		} else {

			PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, TRUE, 0);

		}

	} else

  #endif

	{

		MessageBox(hCardRoomDlg, sdm->shutdown_reason, "e-Media Poker Server shutdown notice", MB_OK|MB_ICONSTOP|MB_TOPMOST);

	}



	// Clear the table list

	EnterCriticalSection(&CardRoomVarsCritSec);

	for (int i=0 ; i<MAX_CLIENT_DISPLAY_TABS ; i++) {

		if (TableSummaryListPackets[i]) {

			delete TableSummaryListPackets[i];

			TableSummaryListPackets[i] = NULL;

		}

		PostMessage(hCardRoomDlg, WMP_UPDATE_GAME_LIST, 0, (LPARAM)i);

	}

	LeaveCriticalSection(&CardRoomVarsCritSec);

	return ERR_NONE;

}



/**********************************************************************************

 Function ProcessAccountInfo(struct AccountInfo *ai, int input_structure_len)

 Purpose: process an account info packet (only chip counts for now)

***********************************************************************************/

ErrorType ProcessAccountInfo(struct AccountInfo *ai, int input_structure_len)

{

	if (sizeof(*ai) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) AccountInfo was wrong length (%d vs %d)",_FL,sizeof(*ai),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	//kp(("%s(%d) Received AccountInfo structure.\n",_FL));

	// for now, set some globals

	RealChipsInBank = ai->real_in_bank;

	RealChipsInPlay = ai->real_in_play;

	// play money chips are called "fake" chips

	FakeChipsInBank = ai->fake_in_bank;

	FakeChipsInPlay = ai->fake_in_play;

	PendingCredit = ai->pending_credit;

	PendingCheck = ai->pending_check;

	PendingPaypal = ai->pending_paypal;

	CreditFeePoints = ai->credit_fee_points;

	//?????????

	GoodRakedGames = ai->good_raked_games;

	//end 

	LoggedIn = ai->login_status;

	LoggedInPrivLevel = ai->login_priv;

	

	//kp(("%s(%d) LoggedInPrivLevel = %d\n", _FL, LoggedInPrivLevel));

	SavedAccountInfo = *ai;



	//kp(("%s(%d) Received and set login_status of %d\n", _FL, LoggedIn));

	if (strlen(ai->user_id)) {	// grab his "real" name -- proper caps, etc.

		strnncpy(LoginUserID, ai->user_id, MAX_PLAYER_USERID_LEN);

	}

	// if the buy-in dlg happens to be up (in the process of buying in), update his

	// chip totals

	if (hBuyInDLG) {	// dlg exists

		PostMessage(hBuyInDLG, WMP_UPDATE_REALBANK_CHIPS, 0, (LPARAM)RealChipsInBank);

		PostMessage(hBuyInDLG, WMP_UPDATE_FAKEBANK_CHIPS, 0, (LPARAM)FakeChipsInBank);

	}

	if (hCashierDlg) {	// dlg exists

		PostMessage(hCashierDlg, WMP_UPDATE_REALBANK_CHIPS, 0, (LPARAM)RealChipsInBank);

		PostMessage(hCashierDlg, WMP_UPDATE_FAKEBANK_CHIPS, 0, (LPARAM)FakeChipsInBank);

		PostMessage(hCashierDlg, WMP_UPDATE_PLAYER_INFO, 0, 0);

	}

	if (hViewTransactionsDLG) {	// dlg exists

		PostMessage(hViewTransactionsDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

	}

  #if ADMIN_CLIENT

	if (hEditCreditableDLG) {	// dlg exists

		PostMessage(hEditCreditableDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

	}

	if (hCheckTrackingDLG) {	// dlg exists

		PostMessage(hCheckTrackingDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

	}

	if (hReqPlayerInfoDLG) {	// dlg exists

		PostMessage(hReqPlayerInfoDLG, WMP_UPDATE_PLAYER_INFO, 0, 0);

	}

  #endif

	

	// every time we get one of these, check if the login status changed and act if needed

	DealWithLoginStatus(ai->login_status);

	PostMessage(hInitialWindow, WMP_UPDATE_CONNECT_STATUS, 0, 0);

	//kp(("%s(%d) Server says our all-in count is %d\n",_FL,ai->all_in_count));



	return ERR_NONE;

}



#if ADMIN_CLIENT

//*********************************************************


//

// Process a received struct AdminCheckRun structure

// DATATYPE_ADMIN_CHECK_RUN

//

ErrorType ProcessAdminCheckRun(struct AdminCheckRun *acr, int input_structure_len)

{

	if (sizeof(*acr) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) AccountCheckRun was wrong length (%d vs %d)",_FL,sizeof(*acr),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	// Save the admin stats and flag the window for redrawing if it's open

	AdminCheckRun = *acr;

	PostMessage(hInitialWindow, WMP_OPEN_CHECK_RUN, 0, 0);

	return ERR_NONE;

}



//*********************************************************


//

// Process a received struct AdminStats structure

// DATATYPE_ADMIN_STATS

//

ErrorType ProcessAdminStats(struct AdminStats *as, int input_structure_len)

{

	if (sizeof(*as) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) AdminStats was wrong length (%d vs %d)",_FL,sizeof(*as),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	// Save the admin stats and flag the window for redrawing if it's open

	AdminStats = *as;

	if (hAdminStats) {

		PostMessage(hAdminStats, WMP_UPDATE_YOURSELF, 0, 0);

	}

  #if ADMIN_CLIENT

	// Save any shot clock messages we got sent

	SaveShotClockMessages(as->shotclock_message1, as->shotclock_message2, as->shotclock_message1_expired, as->shotclock_message2_expired);

  #endif



	return ERR_NONE;

}



/**********************************************************************************

 Function ProcessAdminInfoBlock(struct AdminInfoBlock *aib, int input_structure_len)


 Purpose: process an admin info block

***********************************************************************************/

ErrorType ProcessAdminInfoBlock(struct AdminInfoBlock *aib, int input_structure_len)

{

	if (sizeof(*aib) != input_structure_len) {

		Error(ERR_ERROR, "%s(%d) AdminInfoBlock was wrong length (%d vs %d)",_FL,sizeof(*aib),input_structure_len);

		return ERR_ERROR;	// do not process.

	}

	AdminInfo = *aib;

	PostMessage(hCardRoomDlg, WMP_INFO_BLOCK_ARRIVED, 0, 0);

	return ERR_NONE;

}



#endif



//****************************************************************

//

// Parse any incoming data structures we've received

//

void ParseIncomingData(ClientSocket *cs)

{

	forever {	// Loop until we run out of incoming data

	  #if ADMIN_CLIENT

		if (SecondCounter < dwEarliestPacketSendTime) {

			// debug: It's too early to do any I/O.

			return;

		}	  

	  #endif

		// Handle any pending I/O (these functions are non-blocking)

		EnterCriticalSection(&ClientSockCritSec);

		cs->ProcessSendQueue();

		cs->ReadDataToQueue();	

		Packet *p;

		p = NULL;

		cs->ReadPacket(&p);		// Pop next complete packet from queue

		LeaveCriticalSection(&ClientSockCritSec);

		if (!p) {

			break;	// we're all caught up... nothing more to read.

		}

		dwLastPacketReceivedSeconds = SecondCounter;



		// Something was ready...

		// Calculate pointers to the two data regions...

		struct DataPacketHeader *hdr = (struct DataPacketHeader *)p->user_data_ptr;



		// Verify the length of the data.

		if ((int)(hdr->data_packet_length + sizeof(*hdr)) != p->user_data_length) {

			Error(ERR_ERROR, "%s(%d) Packet data length fields don't match (%d vs %d).  Tossing packet.",

					_FL, hdr->data_packet_length + sizeof(*hdr), p->user_data_length);

		  #if 1

			kp(("%s(%d) Here's the packet we're going to ignore:\n",_FL));

			khexdump(p->user_data_ptr, p->user_data_length, 16, 1);

		  #endif

			delete p;

			p = NULL;

			continue;

		}



	  #if 0	

		kp1(("%s(%d) Playing a sound every time we receive a packet\n",_FL));

		PPlaySound(SOUND_WAKE_UP);	// 'small' sound

	  #endif



		void *data_ptr = (void *)((char *)p->user_data_ptr+sizeof(*hdr));



	  #if DISP || 1

		kp(("%s(%d) Received a packet: type=%d, len=%d\n",_FL,hdr->data_packet_type, hdr->data_packet_length));

		khexdump(data_ptr, hdr->data_packet_length, 16, 1);

	  #endif

		//kp(("%s(%d) Processing packet type %d...\n",_FL,hdr->data_packet_type));



	  #if 0

		kp1(("%s(%d) Warning: comm thread grabbing and holding critsecs to look for hang bug\n",_FL));

		//EnterCriticalSection(&MessageThreadCritSec);

		EnterCriticalSection(&CardRoomVarsCritSec);

		EnterCriticalSection(&ClientSockCritSec);

		int packet_type = hdr->data_packet_type;

		kp(("%s(%d) Comm thread: about to process packet type %d...\n",_FL,packet_type));

		Sleep(100);

	  #endif



		switch (hdr->data_packet_type) {

		case DATATYPE_SERVER_VERSION_INFO:			
                    
			ProcessServerVersionInfo((struct VersionInfo *)data_ptr, hdr->data_packet_length);
			break;

		case DATATYPE_GAME_COMMON_DATA:

			ProcessGameCommonData((struct GameCommonData*)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_GAME_PLAYER_DATA:

			ProcessGamePlayerData((struct GamePlayerData*)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_PLAYER_INPUT_REQUEST:

			ProcessGamePlayerInputRequest((struct GamePlayerInputRequest *)data_ptr, hdr->data_packet_length);

			SendKeepAlive();	// Always send out a keep-alive ASAP after receiving an input request.

			break;

		case DATATYPE_PLAYER_INPUT_REQUEST_CANCEL:	// cancel previous input request?

			ProcessGamePlayerInputRequestCancel((struct GamePlayerInputRequestCancel *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_KEEP_ALIVE:

			// No processing to do.

			pr(("%s(%d) Received KEEP_ALIVE message.\n",_FL));

			break;

		case DATATYPE_CHAT_MSG:

			ProcessGameChatMessage((struct GameChatMessage *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_CARDROOM_SERIAL_NUMS:	// struct CardRoom_TableSummarySerialNums

			ProcessCardRoomSerialNums((struct CardRoom_TableSummarySerialNums *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_CARDROOM_TABLE_LIST:	// struct CardRoom_TableSummaryList

			ProcessCardRoomTableList(&p);

			break;

		case DATATYPE_CARDROOM_TABLE_INFO:	// struct CardRoom_TableInfo

			ProcessCardRoomTableInfo(&p);

			break;

		case DATATYPE_CARDROOM_JOIN_TABLE:	// struct CardRoom_JoinTable

			ProcessJoinTableResult((struct CardRoom_JoinTable *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_CARDROOM_JOINED_TABLES:	// struct CardRoom_JoinedTables

			ProcessJoinedTables((struct CardRoom_JoinedTables *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_CARDROOM_SEAT_AVAIL:	// struct CardRoom_SeatAvail

			ProcessSeatAvail((struct CardRoom_SeatAvail *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_ACCOUNT_INFO:			// struct AccountInfo

			ProcessAccountInfo((struct AccountInfo *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_ADMIN_CHECK_RUN:

		  #if ADMIN_CLIENT

			ProcessAdminCheckRun((struct AdminCheckRun *)data_ptr, hdr->data_packet_length);

		  #endif

			break;

		case DATATYPE_ADMIN_INFO_BLOCK:

		  #if ADMIN_CLIENT

			ProcessAdminInfoBlock((struct AdminInfoBlock *)data_ptr, hdr->data_packet_length);

		  #endif

			break;

		case DATATYPE_ADMIN_STATS:

		  #if ADMIN_CLIENT

			ProcessAdminStats((struct AdminStats *)data_ptr, hdr->data_packet_length);

		  #endif

			break;

		case DATATYPE_ACCOUNT_RECORD:			// struct AccountRecord

		  #if ADMIN_CLIENT

			ProcessAccountRecord((struct AccountRecord *)data_ptr, hdr->data_packet_length, hdr->data_packet_type);

		  #endif

			break;

		case DATATYPE_ACCOUNT_RECORD2:			// struct AccountRecord

			ProcessAccountRecord((struct AccountRecord *)data_ptr, hdr->data_packet_length, hdr->data_packet_type);

			break;

		case DATATYPE_MISC_CLIENT_MESSAGE:

			ProcessMiscClientMsg((struct MiscClientMessage *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_PING:

			ProcessPing((struct Ping *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_SHUTDOWN_MSG:

			ProcessShutDownMsg((struct ShutDownMsg *)data_ptr, hdr->data_packet_length);

			break;

		case DATATYPE_CLOSING_CONNECTION:

			ProcessConnectionClosing((struct ConnectionClosing *)data_ptr, hdr->data_packet_length);

			break;

		default:

			Error(ERR_INTERNAL_ERROR, "%s(%d) Unknown or unhandled data packet type (%d).  Tossing packet.",

					_FL, hdr->data_packet_type);

			break;

		}

		//kp(("%s(%d) Done processing packet.\n",_FL));



		// If we still have a pointer to the packet then we are responsible

		// for deleting it.

		if (p) {

		  #if 1	//

			delete p;

		  #else

			PktPool_ReturnToPool(p);

		  #endif

			p = NULL;

		}

	  #if 0

		kp(("%s(%d) Comm thread: done processing packet type  %d.\n",_FL,packet_type));

		kp1(("%s(%d) Warning: comm thread grabbing and holding critsecs to look for hang bug\n",_FL));

		Sleep(50);

		LeaveCriticalSection(&ClientSockCritSec);

		LeaveCriticalSection(&CardRoomVarsCritSec);

		//LeaveCriticalSection(&MessageThreadCritSec);

	  #endif

	}

}



//****************************************************************

//

// Request a new game list summary if necessary

//

void CheckForNewGameList(ClientDisplayTabIndex client_display_tab_index)

{

	if (client_display_tab_index >= 6 && ServerVersionInfo.server_version.build < 0x010a0001) {

		return;	// don't request something the server does not support.

	}

	if (Our_TableSerialNums.serial_num[client_display_tab_index] != Server_TableSerialNums.serial_num[client_display_tab_index]) {

		pr(("%s(%d) Tables for game type %d have changed.  Requesting update.\n",_FL,client_display_tab_index));

		struct CardRoom_RequestTableSummaryList r;

		zstruct(r);

		r.client_display_tab_index = (BYTE8)client_display_tab_index;



		// send the low 32-bits of cpu's time stamp counter as additional

		// entropy for rng on the server.

		r.random_bits = dwRandomEntropy;

		if (iRDTSCAvailFlag) {	// is the rdtsc instruction available?

			r.random_bits += (WORD32)rdtsc();

		}



		SendDataStructure(DATATYPE_CARDROOM_REQUEST_TABLE_LIST, &r, sizeof(r));

	}

}



//****************************************************************


//

// Request an updated table info structure

//

void RequestTableInfo(WORD32 table_serial_number, int subscribe_flag)

{

	pr(("%s(%d) Requesting TableInfo for table %d (subscribe_flag = %d)\n", _FL, table_serial_number, subscribe_flag));

	struct CardRoom_RequestTableInfo r;

	zstruct(r);

	r.subscribe_flag = (BYTE8)subscribe_flag;

	r.table_serial_number = table_serial_number;

	SendDataStructure(DATATYPE_CARDROOM_REQUEST_TABLE_INFO, &r, sizeof(r));

}



/**********************************************************************************

 Function SendLoginInfo(char *name, char *password)


 Purpose: send a login-attempt packet to the server

***********************************************************************************/

void SendLoginInfo(char *name, char *password, int request_priority_login_flag)

{

	struct PlayerLoginRequest plr;

	zstruct(plr);

	strnncpy(plr.user_id, name, MAX_PLAYER_USERID_LEN);

	strnncpy(plr.password, password, MAX_PLAYER_PASSWORD_LEN);

  #if ADMIN_CLIENT

	if (iAdminClientFlag) {	// admin clients always request priority login status

	  #if 1

		plr.priority_flag = TRUE;

	  #else

		kp1(("%s(%d) Warning: admin clients should always set priority login flag\n",_FL));

	  #endif

	}

  #endif

	if (request_priority_login_flag) {

		plr.priority_flag = TRUE;

	}
   
  //   PPlaySound(SOUND_WAKE_UP);	// 'small' sound
	SendDataStructure(DATATYPE_PLAYER_LOGIN_REQUEST, &plr, sizeof(PlayerLoginRequest));


	pr(("Sending login %s / %s\n", name, password));

}



//*********************************************************

//

// Request all table lists.  This is typically done once at startup

// and once again when we log in, thereafter we just grab

// new ones when the tab on the game list view control changes.

//

void RequestAllTableLists(void)

{

	for (int i=0 ; i<ACTUAL_CLIENT_DISPLAY_TABS ; i++) {

		Our_TableSerialNums.serial_num[i] = (BYTE8)(Server_TableSerialNums.serial_num[i]-1);	// make different

	  #if 1

		// Do not request game types the server does not understand.

		if (i < 6 || ServerVersionInfo.server_version.build >= 0x010a0001)

	  #endif

		{

			struct CardRoom_RequestTableSummaryList r;

			zstruct(r);

			r.client_display_tab_index = (BYTE8)i;

			SendDataStructure(DATATYPE_CARDROOM_REQUEST_TABLE_LIST, &r, sizeof(r));

		}

	}

}	



//*********************************************************


//

// This function get called by the Error() function when an error

// occurs.  If we've got a good connection to the server, pass

// the error on to the server.

//

void ClientCommErrorCallback(char *error_string)

{

  #if ADMIN_CLIENT

	if (iAdminClientFlag) {

		return;	// do not send to server in admin mode.

	}

  #endif

	static volatile int reentered = FALSE;

	if (++reentered == 1) {

		static int sent_errors = 0;

		if (sent_errors++ < 10) {	// don't send more than n

			if (ClientSockConnected) {

				struct ClientErrorString ces;

				zstruct(ces);

				strnncpy(ces.error_string, error_string, CLIENT_ERROR_STRING_LEN);

				SendDataStructure(DATATYPE_ERROR_STRING, &ces, sizeof(ces));

			}

		}

	}

	reentered--;

}



//****************************************************************


//

// Connect to the server and process any I/O while connected.

// Does not return until connection to server has been terminated or lost.

//

// Hosts to try (in order of preference)



void ConnectToServer(void)

{

	int host_index = 0;

	int retry_count = 0;



startover:

  #if ADMIN_CLIENT

	if (iAdminClientFlag && !iPlayedConnectionDownSound && !iDisableServerSwitching) {

		iFlashTitleBars = 20;			// start flashing the title bars

		iPlayedConnectionDownSound = TRUE;

		PPlaySound(SOUND_CONNECTION_DOWN);

		iPlayedConnectionUpSound = FALSE;

	}

  #endif



	EnterCriticalSection(&ClientSockCritSec);

	if (ClientSock) {

		delete ClientSock;

		ClientSock = NULL;

	}

	LeaveCriticalSection(&ClientSockCritSec);



	LoggedIn = LOGIN_NO_LOGIN;

	UpdateSplashStatus(CONNSTAT_DISCONNECTED);



	IPADDRESS server_ip = 0;

	char *server_name = NULL;

	IPADDRESS hardcoded_server_ip = 0;



  #if 0//
	if (iDisableServerSwitching) {
		if (host_index > 1) {
			host_index = 0;
		}
	}
  #endif



	if (host_index > 6) {

		host_index = 0;

	}


#if 1
	switch (host_index) {
    case 0:
		server_name = NULL;
		hardcoded_server_ip = Defaults.last_server_ip_address;
		//hardcoded_server_ip = IP_ConvertHostNameToIP("162.168.0.50");
        //MessageBox(NULL, "Case 6", "SERVER RESOLV !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
	break;

	case 1:
		server_name = "gameserver.e-mediasoftware.com";
		 hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
       // MessageBox(NULL,"CASE 0", "SERVER RESOLV case 0 !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		break;

	case 2:
		server_name = ServerName;
		 hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
		//hardcoded_server_ip = 0;
      //  MessageBox(NULL, "Case 1", server_name, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		break;

	case 3:
		server_name = "gameserver1.e-mediasoftware.com";
		hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
	//	hardcoded_server_ip = IP_ConvertHostNameToIP("200.9.59.60");
        //MessageBox(NULL, "Case 2", server_name, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		break;

	case 4:
		server_name = "gameserver2.e-mediasoftware.com";
	    hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
       // MessageBox(NULL, "Case 3", "SERVER RESOLV !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		break;

	case 5:
		server_name = "gameserver3.e-mediasoftware.com";
		hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
	//	hardcoded_server_ip = IP_ConvertHostNameToIP("200.9.59.60");
       // MessageBox(NULL, "Case 4", "SERVER RESOLV !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
		break;

	case 6:
		server_name = "gameserver4.e-mediasoftware.com";
		hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
		//hardcoded_server_ip = IP_ConvertHostNameToIP("200.9.59.60");
        //MessageBox(NULL, "Case 5", "SERVER RESOLV !!", MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST); 
		break;
   
	}
#endif 
//	server_name = "gameserver.e-mediasoftware.com";
//	hardcoded_server_ip = IP_ConvertHostNameToIP("200.9.59.60"); 
//	hardcoded_server_ip = IP_ConvertHostNameToIP(server_name);
//    hardcoded_server_ip = IP_ConvertHostNameToIP("192.168.0.50");

	if (server_name && server_name[0]) {
		//kp(("%s(%d) Resolving '%s'...\n", _FL, server_name));
		UpdateSplashStatus(CONNSTAT_RESOLVING);
		server_ip = IP_ConvertHostNameToIP(server_name);
		//kp(("%s(%d) Resolved '%s' to $%08lx\n", _FL, server_name, server_ip));
		if (!server_ip) {
			Error(ERR_ERROR, "%s(%d) Unable to resolve '%s'", _FL, server_name);
			if (ExitNowFlag) {
				return;
			}
			if (retry_count >= 9
			  #if ADMIN_CLIENT
		  //#if 0
				&& !iAdminClientFlag
			  #endif
			) {
				char msg[200];
				sprintf(msg, "Unable to resolve '%s' into an IP address.\n\nPlease check your internet connection.", server_name);
				int result = MessageBox(NULL, msg, "e-Media Poker.com", MB_RETRYCANCEL|MB_ICONEXCLAMATION);
				if (result==IDRETRY) {
					goto startover;
				}
				exit(0);
			}
			// server_ip was not found... try hard coded values...
			server_ip = hardcoded_server_ip;
		}
	} else {
		//kp(("%s(%d) no server name.  Using $%08lx instead.\n", _FL, hardcoded_server_ip));
		server_ip = hardcoded_server_ip;
	}

	if (!server_ip) {	// nothing to try?
		host_index++;	// try another host
		retry_count++;
		goto startover;	// try a few times before giving up.
	}

	if (ExitNowFlag) {
		return;
	}

	UpdateSplashStatus(CONNSTAT_CONNECTING);
	EnterCriticalSection(&ClientSockCritSec);
	ClientSock = new ClientSocket;
	ClientSockConnected = FALSE;
	//kp(("%s(%d) *** FORCING IP ***\n", _FL)); server_ip=0x0100007f;
	//kp(("%s(%d) Attempting to connect to socket at IP address $%08lx\n", _FL, server_ip));
  #if INCL_SSL_SUPPORT
	
//kp(("%s(%d) Calling SSL_*() from thread %d\n", _FL, GetCurrentThreadId()));
	ErrorType err = ClientSock->ConnectSocket(server_ip,
			MainSSL_CTX ? (short)PortNumber_SSL : (short)PortNumber, MainSSL_CTX);
  #else
	ErrorType err = ClientSock->ConnectSocket(server_ip, (short)PortNumber);
  #endif

	if (err)

		DIE("ConnectSocket() failed.");

	ClientSock->InitialConnectionTimeout = InitialConnectionTimeout;	// copy .INI file setting to ClientSocket



  #if 1

	{

		char str[100];

		IP_ConvertIPtoString(server_ip, str, 100);

		kp(("%s %s(%d) Connection to %s is in progress...\n",TimeStr(),_FL, str));

	}

  #endif

	LeaveCriticalSection(&ClientSockCritSec);



   #if ADMIN_CLIENT

	//: open extra simulated client sockets if requested.

	// (for simulating I/O load on the server)

	static already_opened_sockets = FALSE;	// only do this once

	if (ExtraSocketsToOpen && !already_opened_sockets) {

		kp(("%s(%d) attempting to connect %d extra sockets\n", _FL, ExtraSocketsToOpen));

		// Allocate an array to keep all the sockets in...

		SpoofedClientSockets = (struct SpoofedClientSocketInfo *)malloc(sizeof(struct SpoofedClientSocketInfo)*ExtraSocketsToOpen);

		if (SpoofedClientSockets) {

			memset(SpoofedClientSockets, 0, sizeof(struct SpoofedClientSocketInfo)*ExtraSocketsToOpen);

		}

	}

   #endif



	UpdateSplashStatus(CONNSTAT_SETTINGUP);

	// Wait here until the connection finishes completing.

	while (!ExitNowFlag) {

		Sleep(100);

		EnterCriticalSection(&ClientSockCritSec);

		// Handle any pending I/O (these functions are non-blocking)

		ClientSock->ProcessSendQueue();

		ClientSock->ReadDataToQueue();	

		LeaveCriticalSection(&ClientSockCritSec);

		if (ClientSock->disconnected) {

			UpdateSplashStatus(CONNSTAT_DISCONNECTED);

			break;	// we got disconnected, leave our little loop.

		}

		if (ClientSock->connected_flag) {

			dwLastPacketReceivedSeconds = SecondCounter;

		  #if ADMIN_CLIENT && 0	//20000718MB: moved elsewhere

			if (iAdminClientFlag && flash_on_connect && !iDisableServerSwitching) {

				iFlashTitleBars = 20;	// start flashing the title bars

				//flash_on_connect = FALSE;

				PPlaySound(SOUND_CONNECTION_UP);

			}

			flash_on_disconnect = TRUE;

		  #endif

			break;	// we're connected now, leave our little loop.

		}

	}

	if (!ClientSock->connected_flag) {

		host_index++;	// try another host

		retry_count++;

		goto startover;	// try a few times before giving up.

	}

	// Keep track of the last server ip address we tried to connect to.

	Defaults.last_server_ip_address = server_ip;

	Defaults.changed_flag = TRUE;



	// Send our version packet...

	EnterCriticalSection(&ClientSockCritSec);

	if (ClientSock->connected_flag) {

		ClientSockConnected = TRUE;

		SOCKADDR_IN sa;

		zstruct(sa);

		int bytes = sizeof(sa);

		if (!getsockname(ClientSock->sock, (SOCKADDR *)&sa, &bytes)) {

			// no error... use it.

			OurLocalIP = sa.sin_addr.s_addr;

		}



	  #if INCL_SSL_SUPPORT

		if (ClientSock->ssl) {

			int bits = 0;

			//kp(("%s(%d) Calling SSL_*() from thread %d\n", _FL, GetCurrentThreadId()));

			int result = SSL_get_cipher_bits((SSL *)ClientSock->ssl, &bits);

			if (result > 128) {

				//kp1(("%s(%d) Limiting cipher strength display to 128 bits (result was %d)\n", _FL, result));

				result = 128;

			}

			pr(("%s(%d) Connected using %s (%s, %d/%d-bits)\n",

					_FL,

					SSL_get_cipher_version((SSL *)ClientSock->ssl),

					SSL_get_cipher_name((SSL *)ClientSock->ssl),

					bits, result));

			sprintf(szConnectionTypeString, "Connected to the server using encryption.\nAlgorithm: %s (%s, %d-bits)",

					SSL_get_cipher_version((SSL *)ClientSock->ssl),

					SSL_get_cipher_name((SSL *)ClientSock->ssl),

					result);

		} else {

			strcpy(szConnectionTypeString, "Connected to server. Encryption is off.");

		}

	  #else

		strcpy(szConnectionTypeString, "Connected to server.");

	  #endif



		UpdateSplashStatus(CONNSTAT_EXCHANGING);

	  #if 0

		kp1(("%s(%d) **** WARNING **** Running in simulated client mode!\n", _FL));

		ClientVersionNumber.flags |= VERSIONFLAG_SIMULATED;

	  #endif

		SendDataStructure(DATATYPE_VERSION_NUMBER, &ClientVersionNumber, sizeof(ClientVersionNumber));



		SendClientPlatformInfo();	// send screen resolution, etc. (before logging in)



		// Log in (as anonymous if necessary)

		if (AutoLoginFlag && (!LoginUserID[0] || !LoginPassword[0])) {

			kp(("%s(%d) Warning: could not login automatically because name or password was blank.\n", _FL));

		}

		if (AutoLoginFlag && LoginUserID[0] && LoginPassword[0]) {

			SendLoginInfo(LoginUserID, LoginPassword, TRUE);

		} else {

			SendLoginInfo(SZ_ANON_LOGIN, SZ_ANON_LOGIN, iRequestPriorityLogin);

		}



		// Request each game list once at startup, thereafter we just grab

		// new ones when the tab on the game list view control changes.

		RequestAllTableLists();

		// Zero out the old copies of the client state info so that we re-send them

		// to the server immediately when requested.

		for (int i=0 ; i<MAX_TABLES ; i++) {

			zstruct(Table[i].PrevClientState);

		}

		UpdateSplashStatus(CONNSTAT_CONNECTED);

	}

	LeaveCriticalSection(&ClientSockCritSec);



	// Parse any incoming data...

	//kp(("%s(%d)\n",_FL));

  #if USE_SELECT_FOR_LOOP_CONTROL

	fd_set readset;

	zstruct(readset);

	FD_ZERO(&readset);

	#pragma warning(disable : 4127)

	FD_SET(ClientSock->sock, &readset);

	#pragma warning(default : 4127)

  #endif // USE_SELECT_FOR_LOOP_CONTROL

	while (!ExitNowFlag) {

		ParseIncomingData(ClientSock);	// loops internally until receive queue is empty

		UpdateSplashStatus(CONNSTAT_CONNECTED);

		if (ClientSock->disconnected) {

			//kp(("%s(%d)\n",_FL));

			UpdateSplashStatus(CONNSTAT_DISCONNECTED);

			if (!ExitNowFlag) {

				if (ClientSock->connected_flag) {

					kp(("%s(%d) We got disconnected. Last packet received %ds ago.\n",_FL, SecondCounter - dwLastPacketReceivedSeconds));

				} else {

					kp(("%s(%d) %s Unable to establish connection to %s:%u\n",_FL,TimeStr(),ServerName,PortNumber));

				}

			}

			break;	// we got disconnected, leave our little loop.

		}



		// If our app has gone inactive, send a keep alive indicating the new status.

		static int old_active_flag;

		int active_flag = IsOurAppActive();

		if (!active_flag && old_active_flag) {

			//kp(("%s(%d)\n",_FL));

			SendKeepAlive();

			//kp(("%s(%d)\n",_FL));

		}

		old_active_flag = active_flag;



		// Send out keep alive's if it has been too long since we've sent something.

		if (!ClientSock->disconnected &&

				(SecondCounter - ClientSock->time_of_last_sent_packet) >= 30) {

			//kp(("%s(%d)\n",_FL));

			SendKeepAlive();

			//kp(("%s(%d)\n",_FL));

		}



	  #if ADMIN_CLIENT

		// Handle spoofed connections for load testing...

		if (SpoofedClientSockets) {

			int i;

			// Count how many sockets we've currently got open.

			int open_sockets = 0;		// # of sockets which are currently open

			int pending_connects = 0;	// # of sockets just opened and waiting to connect properly

			int connected_sockets = 0;	// # of sockets which are actually connected

			static int iDisableNewSpoofedSockets;	// flag to indicate if we should give up opening new sockets.

			for (i=0 ; i<ExtraSocketsToOpen ; i++) {

				if (SpoofedClientSockets[i].sock) {

					if (SpoofedClientSockets[i].sock->sock != INVALID_SOCKET) {

						open_sockets++;

						if (!SpoofedClientSockets[i].printed_connected_msg) {

							pending_connects++;

						}

					}

					if (SpoofedClientSockets[i].sock->connected_flag &&

						SpoofedClientSockets[i].printed_connected_msg)

					{

						connected_sockets++;

					}

				}

			}

			for (i=0 ; i<ExtraSocketsToOpen ; i++) {

				// If a socket has become disconnected, delete it and reconnect.

				if (SpoofedClientSockets[i].sock && 

					SpoofedClientSockets[i].sock->disconnected)

				{

					kp(("%s(%d) socket #%4d is disconnected (%4d open, %4d pending, %4d connected)\n",

							_FL, i, open_sockets, pending_connects, connected_sockets));

					delete SpoofedClientSockets[i].sock;

					SpoofedClientSockets[i].sock = NULL;

					connected_sockets--;

					open_sockets--;

				}



				// See if we should open a new socket

				if (pending_connects < 20 && !SpoofedClientSockets[i].sock && !iDisableNewSpoofedSockets) {

					//kp(("%s(%d) Trying to create new ClientSocket  #%d\n", _FL, i));

					static WORD32 dwLastConnectAttempt;

					WORD32 now = GetTickCount();

				  #if 0	//Limit how quickly we pound the server with connection requests.

					if (now - dwLastConnectAttempt >= 5)

				  #endif

					{

						dwLastConnectAttempt = now;

						zstruct(SpoofedClientSockets[i]);

						SpoofedClientSockets[i].sock = new ClientSocket;

						//kp(("%s(%d) Finished creating new ClientSocket #%d, ptr = $%08lx\n", _FL, i, SpoofedClientSockets[i].sock));

						if (!SpoofedClientSockets[i].sock) {

							kp(("%s(%d) Warning: extra client socket #%d failed to create!\n", _FL, i));

						} else {

						  #if INCL_SSL_SUPPORT

							SpoofedClientSockets[i].sock->ConnectSocket(server_ip, MainSSL_CTX ? (short)PortNumber_SSL : (short)PortNumber, MainSSL_CTX);

						  #else

							SpoofedClientSockets[i].sock->ConnectSocket(server_ip, (short)PortNumber);

						  #endif

						}

						if (SpoofedClientSockets[i].sock->sock != INVALID_SOCKET) {

							open_sockets++;

							pending_connects++;

						} else {

							// We couldn't open a socket for some reason.  Out of file handles?

							kp(("%s %s(%d) ConnectSocket() failed.  Out of file handles?  Giving up.\n",

										TimeStr(), _FL));

							iDisableNewSpoofedSockets = TRUE;

						}

					}

				}



				if (SpoofedClientSockets[i].sock) {

					// Handle any pending I/O (these functions are non-blocking)

					SpoofedClientSockets[i].sock->ProcessSendQueue();

					SpoofedClientSockets[i].sock->ReadDataToQueue();	

					if (SpoofedClientSockets[i].sock->connected_flag) {

						// We're connected right now.

						if (!SpoofedClientSockets[i].sent_version_info) {

							// We haven't sent version info yet... do it now.

							SpoofedClientSockets[i].sent_version_info = TRUE;

							struct VersionNumber cvn = ClientVersionNumber;

							cvn.flags |= VERSIONFLAG_SIMULATED;

							//kp(("%s %s(%d) Sending version info for spoofed client %d\n",  TimeStr(), _FL, i));

							SendDataStructure(SpoofedClientSockets[i].sock, DATATYPE_VERSION_NUMBER, &cvn, sizeof(cvn));

						}

					}



					// Handle anything coming in...

					Packet *p;

					int got_a_packet = FALSE;

					do {

						p = NULL;

						SpoofedClientSockets[i].sock->ReadPacket(&p);		// Pop next complete packet from queue

						if (p) {	// we got something.

							got_a_packet = TRUE;

							if (SpoofedClientSockets[i].sock->connected_flag &&

								!SpoofedClientSockets[i].printed_connected_msg)

							{

								connected_sockets++;

								SpoofedClientSockets[i].printed_connected_msg = TRUE;

								kp(("%s(%d) socket #%4d has connected successfully (%4d open, %4d pending, %4d connected)\n",

									_FL, i, open_sockets, pending_connects, connected_sockets));

							}



							SpoofedClientSockets[i].packets_received_since_sending++;

							struct DataPacketHeader *hdr = (struct DataPacketHeader *)p->user_data_ptr;

							if (hdr->data_packet_type==DATATYPE_PING && hdr->data_packet_length==sizeof(struct Ping)) {

							  #if 0	//

								if () {	// unfinished !!!!!!

								}

								ProcessPing((struct Ping *)data_ptr, hdr->data_packet_length);

								if (sizeof(*ping) != input_structure_len) {

									Error(ERR_ERROR, "%s(%d) Ping was wrong length (%d vs %d)",_FL,sizeof(*ping),input_structure_len);

									return ERR_ERROR;	// do not process.

								}

								// If it came from the server, send it back.

								if (ping->flags & 0x01) {	// client to server and back?

									// It was one of ours...

									kp(("%s(%d) Elapsed time for ping from client to server and back: %dms\n", _FL, GetTickCount() - ping->local_ms_timer));

								} else {

									// It originated on the server... send it back.

									//kp(("%s(%d) Sending ping back to server.\n",_FL));

									SendDataStructure(DATATYPE_PING, ping, sizeof(*ping));

								}



							  #endif

							}

						  #if 0	//

							kp(("%s(%d) Extra socket #%4d has received a packet (type %2d, len %3d)\n",

									_FL, i, hdr->data_packet_type, hdr->data_packet_length));

						  #endif

							delete p;

						}

					} while(p);



					if (SpoofedClientSockets[i].sent_version_info &&

						(SpoofedClientSockets[i].packets_received_since_sending >= 2 ||

						 SecondCounter - SpoofedClientSockets[i].time_of_last_send >= 10))

					{

						//kp(("%s %s(%d) Sending PING on spoofed client socked %d\n", TimeStr(), _FL, i));

						// Time to send something.

						SpoofedClientSockets[i].packets_received_since_sending = 0;

						SpoofedClientSockets[i].time_of_last_send = SecondCounter;

						// Send something back.

						struct Ping ping;

						zstruct(ping);

						ping.local_ms_timer = GetTickCount();

						ping.flags = 0x01;	// client to server and back

						ping.random_bits += (WORD32)rdtsc();;

						SendDataStructure(SpoofedClientSockets[i].sock, DATATYPE_PING, &ping, sizeof(ping));

					}

				  #if 1	// Randomly disconnect every now and then.

					kp1(("%s(%d) Randomly disconnecting spoofed client sockets.\n", _FL));

					if (got_a_packet && !random(ExtraSocketsToOpen*5+10)) {

						kp(("%s(%d) Extra socket #%4d is being disconnected.\n",  _FL, i));

						SpoofedClientSockets[i].sock->CloseSocket();

					}

				  #endif

				}

			}

		}

	  #endif



	  #if USE_SELECT_FOR_LOOP_CONTROL

		//kp1(("%s(%d) USE_SELECT_FOR_LOOP_CONTROL is enabled.\n", _FL));

		if (WinSockVersionInstalled >= 0x200) {	// version 2.0 or later supports select()

			// Sleep for up to 100ms or until new data arrives.

			//kp1(("%s(%d) Using select() for I/O thread delay\n", _FL));

		  	struct timeval tv;

			zstruct(tv);

			fd_set tempset = readset;

			tv.tv_usec = 100000;	// 100ms timeout

			select(0, &tempset, NULL, NULL, &tv);

		} else {

			// select() is not supported... just sleep briefly.

			kp1(("%s(%d) Using Sleep(20) for I/O thread delay\n", _FL));

			Sleep(20);

		}

	  #else

		Sleep(20);

	  #endif

	}

	//kp(("%s(%d)\n",_FL));

	LoggedIn = LOGIN_NO_LOGIN;



	EnterCriticalSection(&ClientSockCritSec);

	delete ClientSock;

	ClientSock = NULL;

	LeaveCriticalSection(&ClientSockCritSec);

	UpdateSplashStatus(CONNSTAT_DISCONNECTED);



	// As soon as our connection is lost, hide all input request buttons and clear

	// any in-turn actions we might eventually act upon.

	for (int i=0 ; i<MAX_TABLES ; i++) {

		if (Table[i].hwnd) {

			struct TableInfo *t = Table + i;

			ClearInTurnActions(t);

			// Also manually set a bunch of things to zero to make extra

			// certain that nothing will get auto-responded to.

			t->ClientState.fold_in_turn = FALSE;

			t->ClientState.leave_table = FALSE;

			t->ClientState.in_turn_action_game_serial_number = 0;

			t->ClientState.in_turn_action_last_input_request_serial_number = 0;

			t->ClientState.in_turn_action = 0;

			t->ClientState.in_turn_action_amount = 0;

			t->ClientState.post_in_turn = FALSE;

			t->ClientState.post_in_turn_game_serial_number = 0;

			t->wait_for_bb = FALSE;

			t->sit_out_request_serial_num = 0;

			t->button_1_alternate_meaning = 0;	// 0=none, 1=Join Waiting list, 2=Unjoin Waiting List

			zstruct(t->GamePlayerInputRequest);

			zstruct(t->PrevClientState);	// force resending when we get reconnected.

			FlagRedrawTable(i);

		}

	}



	// Hide any seat available dialog boxes.

	SeatAvail.timeout = 0;

	SeatAvailSecondCounter = SecondCounter;

	zstruct(previous_seat_avail);

	if (SeatAvail_hwnd) {

		PostMessage(hCardRoomDlg, WMP_SEAT_AVAIL, 0, 0);

	}

	dwTableInfoSerialNum = 0;	// we no longer have a default table.

}



//****************************************************************


//
// Entry point for the communications thread.
//
// #ifndef DEBUG_CONNECT_SERVER
void _cdecl CommThreadEntryPoint(void *args)
//#else
//void CommThreadEntryPoint(void)
//#endif

{

  #if INCL_STACK_CRAWL

	volatile int top_of_stack_signature = TOP_OF_STACK_SIGNATURE;	// for stack crawl

  #endif

	pr(("%s(%d) CommThread has now started.\n",_FL));



	ErrorCallbackFunction = ClientCommErrorCallback;



#if INCL_SSL_SUPPORT && 1

	// Enable the use of encryption by this client...

	//kp(("%s(%d) Calling SSL_*() from thread %d\n", _FL, GetCurrentThreadId()));

	CRYPTO_malloc_init();

	SSLeay_add_ssl_algorithms();

    MainSSL_CTX = SSL_CTX_new(SSLv23_client_method());

    if (!MainSSL_CTX) {

        DIE("Could not create SSL_CTX");

    }



 #if 1

   #if ADMIN_CLIENT && 1

	if (iAdminClientFlag && !ExtraSocketsToOpen) {

	  #if 0	//

		SSL_CTX_set_cipher_list(MainSSL_CTX,

				"ALL"

		);

	  #endif

	} else

   #endif

	{

		#define ADHERE_TO_EXPORT_RESTRICTIONS	0

	  #if ADHERE_TO_EXPORT_RESTRICTIONS

		kp(("%s(%d) Restricting available ciphers (export restrictions)\n",_FL));

	  #else

		kp(("%s(%d) Restricting available ciphers for patent reasons\n",_FL));

	  #endif

		// Performance note: "ALL" uses EDH-RSA which seems to make SSL_accept() on

		// the server end take quite a long time to return.  In fact, it seems about

		// 60 times slower than when just EDH is used.  DO NOT enable different

		// ciphers without doing some performance testing!

		SSL_CTX_set_cipher_list(MainSSL_CTX,

			#if ADHERE_TO_EXPORT_RESTRICTIONS	//20000528MB

				// 56 bits or less...

			  #if 0	//: use RC4-MD5 because it's faster then DES.

					//: note, this change was done on the server instead.

				"EXP:!RSA:!EDH:!DES"	// use 40-bit ciphers and disallow RSA and EDH-RSA.

			  #else

				"EXP:!RSA:!EDH"	// use 40-bit ciphers and disallow RSA and EDH-RSA.

			  #endif

			#else	// full strength (128 bit or higher)

				// Allow large ciphers, but avoid patented ones (RSA and RSA-EDH)

				"ALL:!RSA:!EDH"

				//"ALL:!RSA:!EDH:!DES:!3DES"

			#endif

		);

	}

 #endif

	//kp(("%s(%d) Client SSL compression methods = $%08lx\n", _FL, MainSSL_CTX->comp_methods));



#else

	kp1((ANSI_WHITE_ON_GREEN"%s(%d) Warning: encryption is disabled in this client.\n",_FL));

#endif

	while (IP_Open()) {

	  #if 1	//

	  	// There is similar wording in splash.cpp.  Search for IDC_I_USE_AOL

		if (MessageBox(hCardRoomDlg,

			"Your computer does not appear to be connected to the internet.\n"

			"\n"

			"If you are using AOL:\n"

			"    1) Start up AOL and connect to the internet.\n"

			"    2) On AOL, go to the \"Internet\" menu and select \"Go to Web\".\n"

			"    3) Minimize (don't close) AOL.\n"

			"    4) Press \"Retry\" on this dialog box.\n"

			,

			"e-mediasoftware.com", MB_RETRYCANCEL|MB_ICONSTOP|MB_TOPMOST) == IDCANCEL)

		{

			exit(0);

		}

	  #else

		DIE("IP_Open() failed.");

	  #endif

	}



	forever {

		strcpy(szConnectionTypeString, "Not connected to server.");

		//kp(("%s(%d)\n",_FL));

		if (!iDisableServerCommunications) {

			ConnectToServer();

		}

		//kp(("%s(%d)\n",_FL));

		if (ExitNowFlag) {

			break;

		}

		Sleep(1000);

	}

	strcpy(szConnectionTypeString, "Not connected to server.");



	pr(("%s(%d) Calling IP_Close()\n",_FL));

	IP_Close();



  #if INCL_SSL_SUPPORT

    // Close up openssl if open.

	if (MainSSL_CTX != NULL) {

		//kp(("%s(%d) Calling SSL_*() from thread %d\n", _FL, GetCurrentThreadId()));

    	SSL_CTX_free(MainSSL_CTX);

        MainSSL_CTX = NULL;

	}

  #endif



	ErrorCallbackFunction = NULL;

	pr(("%s(%d) Communications thread is now exiting.\n",_FL));
//#ifndef	DEBUG_CONNECT_SERVER
	NOTUSED(args);
//#endif

  #if INCL_STACK_CRAWL

	NOTUSED(top_of_stack_signature);

  #endif

}



//****************************************************************


//

// Start up the communications thread

//

//

void StartCommThread(void)

{
//#ifndef DEBUG_CONNECT_SERVER
	_beginthread(CommThreadEntryPoint, 0, 0);
//#else
//   CommThreadEntryPoint();
//#endif


}









BOOL CALLBACK dlgFuncTransactionLarge(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

{

	switch (message) {

	case WM_INITDIALOG:

		{

			/*

			AddKeyboardTranslateHwnd(hDlg);

			char *file = (char *)LoadFile("RealMoneyPlayAgreement.txt", NULL);

			if (!file) {

				EndDialog(hDlg, IDCANCEL);	// could not load the agreement

			}

			SetWindowText(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), file);

			free(file);

			SendMessage(GetDlgItem(hDlg, IDC_MESSAGE_TEXT), EM_SETSEL, (WPARAM)-1, 0);

			*/

            //SetDlgItemTextIfNecessary(hDlg, IDC_STATIC_DATETIME, str);

			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SHOW_LARGE),pstr);

			//ShowWindowText(GetDlgItem(hDlg, IDC_STATIC),pstr);

			//SendMessage(GetDlgItem(hDlg, IDC_EXT), EM_SETSEL, (WPARAM)-1, 0);





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





