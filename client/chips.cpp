//****************************************************************


//

// Chips.cpp : Chips related routines for the client tables

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"



#define DRAW_EVERYTHING			0 // for testing: everything gets filled up.
#define DRAW_ALL_CHIP_STACKS	0 // for testing: draw one of each stack.



#define TOTAL_CHIP_BITMAPS	35

struct ChipBitmapEntry {

	HBITMAP hbitmap;

	int preferred_only;	// only use when drawing preferred stack sizes?

	int amount;

};

struct ChipBitmapEntry PlayChipBitmaps[TOTAL_CHIP_BITMAPS] =  {

	0, 0, 25,

	0, 0, 50,

	0, 0, 75,

	0, 0, 100,

	0, 1, 150,

	0, 0, 200,

	0, 1, 250,

	0, 0, 300,

	0, 0, 400,

	0, 0, 500,

	0, 1, 600,

	0, 1, 800,

	0, 0, 1000,

	0, 1, 1200,

	0, 0, 1500,

	0, 1, 1600,

	0, 0, 2000,

	0, 0, 2500,

	0, 1, 3000,

	0, 1, 0,

	0, 0, 5000,

	0, 1, 0,

	0, 0, 7500,

	0, 1, 0,

	0, 0, 10000,

	0, 0, 20000,

	0, 0, 30000,

	0, 0, 40000,

	0, 0, 50000,

	0, 1, 0,

	0, 1, 0,

	0, 0, 100000,

	0, 1, 0,

	0, 0, 150000,

	0, 0, 200000,

};



struct ChipBitmapEntry RealChipBitmaps[TOTAL_CHIP_BITMAPS] =  {

	0, 0, 25,

	0, 0, 50,

	0, 0, 75,

	0, 0, 100,

	0, 0, 150,

	0, 0, 200,

	0, 0, 250,

	0, 0, 300,

	0, 0, 400,

	0, 0, 500,

	0, 1, 600,

	0, 1, 800,

	0, 0, 1000,

	0, 1, 1200,

	0, 0, 1500,

	0, 1, 1600,

	0, 0, 2000,

	0, 0, 2500,

	0, 1, 3000,

	0, 1, 4000,

	0, 0, 5000,

	0, 1, 6000,

	0, 0, 7500,

	0, 1, 8000,

	0, 0, 10000,

	0, 0, 20000,

	0, 0, 30000,

	0, 0, 40000,

	0, 0, 50000,

	0, 1, 0,

	0, 1, 0,

	0, 0, 100000,

	0, 1, 0,

	0, 0, 150000,

	0, 0, 200000,

};



struct ChipBitmapEntry TournChipBitmaps[TOTAL_CHIP_BITMAPS] =  {

	0, 0, 25,

	0, 0, 50,

	0, 0, 75,

	0, 0, 100,

	0, 0, 150,

	0, 0, 200,

	0, 0, 250,

	0, 0, 300,

	0, 0, 400,

	0, 0, 500,

	0, 1, 600,

	0, 1, 800,

	0, 0, 1000,

	0, 1, 1200,

	0, 0, 1500,

	0, 1, 1600,

	0, 0, 2000,

	0, 0, 2500,

	0, 1, 3000,

	0, 1, 4000,

	0, 0, 5000,

	0, 1, 6000,

	0, 0, 7500,

	0, 1, 8000,

	0, 0, 10000,

	0, 0, 20000,

	0, 0, 30000,

	0, 0, 40000,

	0, 0, 50000,

	0, 1, 60000,

	0, 1, 80000,

	0, 0, 100000,

	0, 1, 120000,

	0, 0, 150000,

	0, 0, 200000,

};



HBITMAP ButtonBitmap;

HBITMAP ReservedButtonBitmap;



static int PlayerChipsControlIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_CHIPS_PLR1,

			IDC_CHIPS_PLR2,

			IDC_CHIPS_PLR3,

			IDC_CHIPS_PLR4,

			IDC_CHIPS_PLR5,

			IDC_CHIPS_PLR6,

			IDC_CHIPS_PLR7,

			IDC_CHIPS_PLR8,

			IDC_CHIPS_PLR9,

			IDC_CHIPS_PLR10,
			
};



int PlayerButtonIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_PLAYERPOS_1,

			IDC_PLAYERPOS_2,

			IDC_PLAYERPOS_3,

			IDC_PLAYERPOS_4,

			IDC_PLAYERPOS_5,

			IDC_PLAYERPOS_6,

			IDC_PLAYERPOS_7,

			IDC_PLAYERPOS_8,

			IDC_PLAYERPOS_9,

			IDC_PLAYERPOS_10

};



#define DISPLAY_POT_ID_COUNT	3

int PotIDs[DISPLAY_POT_ID_COUNT] = {

			IDC_POT1,

			IDC_POT2,

			IDC_POT3,

};




//*********************************************************

// 1999/06/25 - MB

//

// Load the chip bitmaps into memory.

// Modified by Allen 2001-09-24

// To change the Name convention of the chip bitmaps

//  

void LoadChipBitmaps(HPALETTE hpal, ChipType chip_type)

{

	struct ChipBitmapEntry *cbe = PlayChipBitmaps;

	char *ct_name = "";

	switch (chip_type) {

	case CT_PLAY:

		cbe = PlayChipBitmaps;

		ct_name = "";

		break;

	case CT_REAL:

		cbe = RealChipBitmaps;

		// Modifiec by Alle 9-24-2001

		// ct_name = "rm-";

		ct_name = "rm";

		break;

	case CT_TOURNAMENT:

		cbe = TournChipBitmaps;

		// Modifiec by Alle 9-24-2001

		// ct_name = "t-";

		ct_name = "t";

		break;

	}

	for (int i=0 ; i<TOTAL_CHIP_BITMAPS ; i++, cbe++) {

		if (!cbe->hbitmap && cbe->amount) {

			// This one needs loading.

			char fname[MAX_FNAME_LEN];

			if (cbe->amount % 100) {	// not divisible by a dollar?  less than a dollar (cents)

			

				// Modifiec by Alle 9-24-2001

				// sprintf(fname, "chips-%s%dc.bmp", ct_name, cbe->amount);

				sprintf(fname, "media\\ch_%s%dc.bmp", ct_name, cbe->amount);

			} else {	

				// Modifiec by Alle 9-24-2001
				sprintf(fname, "media\\ch_%s%d.bmp", ct_name, cbe->amount/100);

			}

			//kp(("%s(%d) fname = '%s', FindFile() returns '%s'\n", _FL, fname, FindFile(fname)));



			// Modifiec by Alle 9-24-2001

			//cbe->hbitmap = LoadBMPasBitmap(FindFile(fname), hpal);

			cbe->hbitmap = LoadBMPasBitmap(FindMediaFile(fname), hpal);

			if (!cbe->hbitmap) {

				Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, fname);

				iBitmapLoadingProblem = TRUE;

			}

		}

	}

	if (!ButtonBitmap) {

		

		// Modifiec by Alle 9-24-2001

		// ButtonBitmap = LoadBMPasBitmap(FindFile("button.bmp"), hpal);

		

		ButtonBitmap = LoadBMPasBitmap(FindMediaFile("button.bmp"), hpal);

		

		if (!ButtonBitmap) {

			iBitmapLoadingProblem = TRUE;

		}

	}

	if (!ReservedButtonBitmap) {

		

		// Modifiec by Alle 9-24-2001

		// ReservedButtonBitmap = LoadBMPasBitmap(FindFile("reserved.bmp"), hpal);



		ReservedButtonBitmap = LoadBMPasBitmap(FindMediaFile("reserved.bmp"), hpal);



		if (!ReservedButtonBitmap) {

			iBitmapLoadingProblem = TRUE;

		}

	}

}



//*********************************************************

// 1999/06/25 - MB

//

// Draw stacks of chips at a specific coordinate.

//

void DrawChips(BlitQueue *q, LPPOINT dest_pt, int preferred_stack_size, int amount, int draw_priority, struct TableInfo *table, int right_to_left_flag)

{

	if (!q) {

		return;	// nowhere to draw.

	}

	// Convert stack size and amount to dollars (from pennies)

  #if 0	//19991118MB

	preferred_stack_size /= 100;

	amount /= 100;

  #endif



	//kp(("%s(%d) ------- DrawChips ------- preferred = %d, amount = %d\n", _FL, preferred_stack_size, amount));

	#define STACK_X_SPACING	24

	#define MAX_PREFERRED_STACKS	4

	POINT pt = *dest_pt;

	// First, try to draw using the preferred stack size.

	// Only do this if we have the preferred stack size and we can do it

	// in 4 or fewer stacks.

	int x_adjust = STACK_X_SPACING;

	if (right_to_left_flag) {

		pt.x += (MAX_PREFERRED_STACKS - 1) * STACK_X_SPACING;

		x_adjust = -STACK_X_SPACING;

	}



	struct ChipBitmapEntry *base_cbe = NULL;

	switch (table->chip_type) {

	case CT_REAL:

		base_cbe = RealChipBitmaps;

		break;

	case CT_PLAY:

		base_cbe = PlayChipBitmaps;

		break;

	case CT_TOURNAMENT:

		base_cbe = TournChipBitmaps;

		if (table->GameCommonData.flags & GCDF_USE_REAL_MONEY_CHIPS) {

			// Override with real chips temporarily

			//kp(("%s(%d) Overriding with real chips.\n", _FL));

			base_cbe = RealChipBitmaps;

		}

		break;

	case CT_NONE:

		Error(ERR_INTERNAL_ERROR,"%s(%d) called with CT_NONE", _FL);
		break;

	default:

		Error(ERR_INTERNAL_ERROR,"%s(%d) called with unknown chip_type", _FL);

		break;

	}



	// If it doesn't fit using our regular preferred stack, try doubling

	// our preferred stack (except heads up, which has lots of room).

	if (amount > preferred_stack_size*MAX_PREFERRED_STACKS &&

		amount <= preferred_stack_size*MAX_PREFERRED_STACKS*2 &&

		!(table->GameCommonData.flags & GCDF_ONE_ON_ONE))

	{

		preferred_stack_size *= 2;

	}

	if (amount <= preferred_stack_size*MAX_PREFERRED_STACKS || (table->GameCommonData.flags & GCDF_ONE_ON_ONE)) {

		// We're not over.... try to find something that matches.

		struct ChipBitmapEntry *cbe = base_cbe;

		for (int i=0 ; i<TOTAL_CHIP_BITMAPS ; i++, cbe++) {

			if (cbe->hbitmap && cbe->amount==preferred_stack_size) {

				// Found our preferred stack size.  Draw as many as we can.

				//kp(("%s(%d) Found a preferred stack of %d to draw %d chips.\n",_FL, cbe->amount, amount));

				while (amount >= cbe->amount) {

					//kp(("%s(%d) Drawing preferred stack: amount = %d, ChipBitmaps[%d].amount = %d\n", _FL, amount, i, cbe->amount));

					// Draw this stack.

					q->AddBitmap(cbe->hbitmap, &pt, draw_priority);

					// Update vars so we know what's left to draw.

					amount -= cbe->amount;

					pt.x += x_adjust;

				}

				break;

			}

		}

	}



	// Now draw whatever is left...

	//kp(("%s(%d) Done attempting preferred stacks.  Left over amount = %d\n", _FL, amount));

	while (amount > 0) {

		//kp(("%s(%d) Searching for a stack to draw %d\n", _FL, amount));

		// Draw as much as we can in this stack.

		// Start by finding the biggest stack available which is less than

		// or equal to the number of chips we need to display.

		int i = TOTAL_CHIP_BITMAPS-1;

		int old_amount = amount;

		struct ChipBitmapEntry *cbe = base_cbe;

		while (i > 0 && (cbe[i].amount > amount

				|| !cbe[i].hbitmap || cbe[i].preferred_only)) {

			i--;

		}

		//kp(("%s(%d) Drawing stack index %d (%d chips).  Amount is %d, soon will be only %d\n", _FL, i, cbe->amount, amount, amount - cbe->amount));

		// Draw this stack.

		q->AddBitmap(cbe[i].hbitmap, &pt, draw_priority);



		// Update vars so we know what's left to draw.

		amount -= cbe[i].amount;

		pt.x += x_adjust;
         

		if (amount==old_amount) {

			kp1(("%s(%d) ChipDraw error: could not draw %d chips.\n", _FL, amount));

			break;

		}

	}

	//kp(("%s(%d) ------ done drawing stack -------\n",_FL));
}	



// Same as above, but right_to_left_flag defaults to FALSE.

void DrawChips(BlitQueue *q, LPPOINT dest_pt, int preferred_stack_size, int amount, int draw_priority, struct TableInfo *table)

{

	DrawChips(q, dest_pt, preferred_stack_size, amount, draw_priority, table, FALSE);	
		// --- Draw the rake --- (no animation)
	

}





//*********************************************************

// 1999/06/25 - MB

//

// Get the coordinates for a picture control on a table.

//

void GetPictureCoordsOnTable(struct TableInfo *table, int control_id, LPPOINT output_pt)

{

	RECT r;

	zstruct(r);

	GetWindowRect(GetDlgItem(table->hwnd, control_id), &r);

	output_pt->x = r.left;

	output_pt->y = r.top;

	ScreenToClient(table->hwnd, output_pt);

}



//*********************************************************

// 1999/06/25 - MB

//

// Draw all the chips at a table.  This includes all players,

// the pot(s), and the rake.  This function might be called when

// the chips update OR when processing WM_PAINT.

//

void DrawAllChips(struct TableInfo *table)

{

	int preferred_stack_size = table->GamePlayerData.standard_bet_for_round;

	POINT pt;

	int draw_dealer_button = TRUE;


	if (table->game_rules == GAME_RULES_STUD7 || table->game_rules == GAME_RULES_STUD7_HI_LO) {

		draw_dealer_button = FALSE;

	}


	if (table->chip_type==CT_TOURNAMENT && table->GameCommonData.table_tourn_state==TTS_WAITING) {

		// no dealer button until all seats are filled.

		draw_dealer_button = FALSE;

	}

	if (draw_dealer_button) {

		// --- Draw the button ---

		// Has it moved since last time (or is this the first time through

		// this function for this table (check x coord for zero))?

		if (table->GameCommonData.p_button != table->previous_button_position ||

			table->AnimButton.current_point.x==0 || table->animation_disable_count) {

			// The button has moved... start animating it from its previous

			// location to the new location.

			pr(("%s(%d) Initializing button for slide to new loc.  current_point.x = %d, anim_disable=%d\n",_FL,

						table->AnimButton.current_point.x, table->animation_disable_count));

			POINT new_point;

			GetPictureCoordsOnTable(table, PlayerButtonIDs[table->GameCommonData.p_button], &new_point);

			if (table->animation_disable_count || table->AnimButton.current_point.x==0) {	// animation disabled?

				table->AnimButton.SetPoint(&new_point);

				//kp(("%s(%d) Button is jumping to position %d\n", _FL, table->GameCommonData.p_button));

				table->previous_button_position = table->GameCommonData.p_button;

			} else {

				POINT old_point;

				//kp(("%s(%d) Button is starting slide to position %d\n", _FL, table->GameCommonData.p_button));

				GetPictureCoordsOnTable(table, PlayerButtonIDs[table->previous_button_position], &old_point);

				table->AnimButton.Init(&old_point, &new_point, 500);

				table->previous_button_position = table->GameCommonData.p_button;

				if (table->active_table_window && !Defaults.iSoundsDisabled && table->animation_disable_count <= 1) {

					PPlaySound(SOUND_MOVE_BUTTON);

				}

			}

		}

		// Update the animation position and determine if we've completed our move.

		table->AnimButton.Update();

		if (table->AnimButton.state != ANIM_STATE_COMPLETE) {

			table->animation_flag = TRUE;	// we're still animating.

		}

		// Draw the dealer button bitmap at the current animation coordinates.

		table->blit_queue.AddBitmap(ButtonBitmap, &table->AnimButton.current_point,

				table->AnimButton.state == ANIM_STATE_COMPLETE ? 200 : 400);

	}



	// Draw any "reserved" buttons if a table has any reserved seats.

	if (table->GameCommonData.flags & GCDF_WAIT_LIST_REQUIRED) {

			
		for (int i=0 ; i<table->max_players_per_table ; i++) {

			// Heads up uses only two particular seating positions.  If this

			// isn't one of then, skip to the next position.

			if ((table->GameCommonData.flags & GCDF_ONE_ON_ONE) &&

				i != table->heads_up_seat_1 &&

				i != table->heads_up_seat_2)

			{

				continue;

			}

			if (!table->GameCommonData.player_id[i] && !table->GameCommonData.name[i][0]) {

				// This seat is empty.

				POINT pt;

				GetPictureCoordsOnTable(table, PlayerButtonIDs[i], &pt);

				table->blit_queue.AddBitmap(ReservedButtonBitmap, &pt, 0);

			}

		}

	}



	// --- Draw the rake --- (no animation)
	//ccv 9-22-2003
	if (table->chip_type !=CT_PLAY){
		GetPictureCoordsOnTable(table, IDC_CHIPS_RAKE, &pt);
		DrawChips(&table->blit_queue, &pt,
		  #if 1	//20000616MB: set preferred size to the rake size.
			table->GamePlayerData.rake_total,
		  #else
			preferred_stack_size,
		  #endif
			table->GamePlayerData.rake_total, 30, table);		
	};//if(table->chip_type !=CT_PLAY)
	//ccv 9-22-2003


	// --- Draw the pots and the chips in front of each player ---

	// These two items need to be combined because we may need to subtract

	// the amounts we're animating if they're moving to/from the pot.



	int keep_old_pots = FALSE;	// default to using the new pot sizes.



	// Determine where we are in a game.  There are three possibilities

	// that we care about:

	//  0 = Start of game

	//  1 = Middle of game

	//  2 = End of game

	int game_state = 0;	// for now, assume start of game

	if (table->GamePlayerData.s_gameover != GAMEOVER_FALSE) {

		game_state = 2;			// game over.

		keep_old_pots = TRUE;	// preserve our adjusted pot sizes at end game

	} else {

		// If there are any chips on the table, it's mid-game, otherwise

		// we should remove all chips from our display pots to prevent

		// the previous game's chips from getting animated.

		if (table->GamePlayerData.pot[0]) {

			// Something is in the main pot... we must be mid-game.

			game_state = 1;

		} else {

			// Main pot is empty... check if there are any chips

			// in front of the players

			for (int i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

				if (table->GamePlayerData.chips_in_front_of_player[i]) {

					// Something is in front of this player...

					// we must be mid-game.
					game_state = 1;

				}

			}

		}

	}



	// If this is the start of a game, initialize our display pots.

	if (game_state==0 || table->animation_disable_count) {

		for (int j=0 ; j<MAX_PLAYERS_PER_GAME ; j++) {

			table->player_display_chips[j] = table->GamePlayerData.chips_in_front_of_player[j];

		}

		memset(table->display_pots, 0, sizeof(table->display_pots[0])*MAX_PLAYERS_PER_GAME);

		table->previous_preferred_stack_size = preferred_stack_size;

		table->chips_game_over_completed = FALSE;

	}



  #if 0	//19990713MB

	if (table->display_pots[0] != table->GamePlayerData.pot[0]) {

		kp(("%s(%d) Pot 0 has changed from %3d to %3d\n",_FL, table->display_pots[0], table->GamePlayerData.pot[0]));

	}

  #endif



	// We need to start animating (or keep animating) if the number of

	// chips in front of a player suddenly goes down and the total chips

	// in the pot goes up.

	int pot_total = 0;

	int display_pot_total = 0;

	int i;

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		pot_total += table->GamePlayerData.pot[i];

		display_pot_total += table->display_pots[i];

	}


	pr(("%s(%d) Latest from server: chips_in_front_of_player[3] = %4d, player[6] = %4d\n", _FL,

			table->GamePlayerData.chips_in_front_of_player[3],

			table->GamePlayerData.chips_in_front_of_player[6]));



	int old_chips_game_over_completed = table->chips_game_over_completed;

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		pr(("%s(%d) Chips in front of player %d = %d, chips_won = %d\n",

					_FL, i, table->GamePlayerData.chips_in_front_of_player[i],

					table->GamePlayerData.chips_won[i]));

	  #if 0	//19990723MB

		if (table->GamePlayerData.chips_in_front_of_player[i] < 0) {

			kp(("%s(%d) Warning: chips_in_front_of_player[%d] = %d\n", _FL, i, table->GamePlayerData.chips_in_front_of_player[i]));

			table->GamePlayerData.chips_in_front_of_player[i] = -table->GamePlayerData.chips_in_front_of_player[i];

		}

	  #endif

	  #if 0	//19990713MB

		if (table->player_display_chips[i] != table->GamePlayerData.chips_in_front_of_player[i]) {

			kp(("%s(%d) Player %d: chips_in_front_of_player has changed from %3d to %3d (anim state = %d)\n",_FL,i, table->player_display_chips[i], table->GamePlayerData.chips_in_front_of_player[i],table->AnimChips[i].state));

			kp(("%s(%d)            player_display_chips = %d, Anim.state = %d\n", _FL, table->player_display_chips[i], table->AnimChips[i].state));

			kp(("%s(%d)            game_state = %d, display_pot_total = %d, pot_total = %d\n", _FL, game_state, display_pot_total, pot_total));

		}

	  #endif

		//19990910MB: If we're not animating, we can use the newest

		// chips_in_front_of_player value.  If we ARE animating, stick with

		// the old one until we're done.

		if (table->AnimChips[i].state==ANIM_STATE_COMPLETE || table->AnimChips[i].state==ANIM_STATE_INIT) {

			if (table->old_chips_in_front_of_player[i] != table->GamePlayerData.chips_in_front_of_player[i]) {

			  #if 0	//19990910MB

				kp(("%s(%d) Changing old_chips_in_front_of_player[%d] from %d to %d\n",

					_FL, i,

					table->old_chips_in_front_of_player[i],

					table->GamePlayerData.chips_in_front_of_player[i]));

			  #endif

			}

			table->old_chips_in_front_of_player[i] = table->GamePlayerData.chips_in_front_of_player[i];	

		}



		if (game_state && display_pot_total < pot_total &&

			table->GamePlayerData.chips_in_front_of_player[i] < table->player_display_chips[i]

		) {

			//kp(("%s(%d) Player %d's chips need animating.  current Anim.state = %d\n", _FL, i, table->AnimChips[i].state));

			// This player's chips need animating between the player and the pot

			// Start the animation if necessary.

			
			
			if (table->AnimChips[i].state==ANIM_STATE_COMPLETE) {

				// Previous animation is complete... start a new one.

				//kp(("%s(%d) Beginning chip animation for player %d\n", _FL, i));
				POINT old_point, new_point;

				// Determine which pot to drag it to.

				int pot_index = DISPLAY_POT_ID_COUNT-1;	// start with last display pot
				
				
				while (pot_index > 0 && !table->display_pots[pot_index]) {

					pot_index--;	// find last pot with something in it.

				}

			
				
				GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &old_point);

				GetPictureCoordsOnTable(table, PotIDs[pot_index], &new_point);

				table->AnimChips[i].Init(&old_point, &new_point, 700);			

				if (table->active_table_window && !Defaults.iSoundsDisabled && table->animation_disable_count <= 1) {

					PPlaySound(SOUND_CHIPS_SLIDING);

				}

			}

			table->AnimChips[i].Update();	// Update the animation


			// If the animation is finished, deal with that
			if (table->AnimChips[i].state==ANIM_STATE_COMPLETE) {

				//kp(("%s(%d) Player %d's chips are finished moving.\n",_FL,i));

				// This animation is finished.  Move the chips

				// into the appropriate display pot (don't go over).

				//!!! note: this code must be modified to adjust the appropriate pot

				table->display_pots[0] += table->player_display_chips[i];

				table->display_pots[0] = min(table->display_pots[0], table->GamePlayerData.pot[0]);

				table->player_display_chips[i] = table->GamePlayerData.chips_in_front_of_player[i];

				if (game_state != 2) {

					POINT pt;

					GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &pt);

					table->AnimChips[i].SetPoint(&pt);

				}

			} else {

				// The animation is not yet finished.  Keep displaying

				// our old pots.

				keep_old_pots = TRUE;

				table->animation_flag = TRUE;	// we're still animating.

			}   

		} else if (game_state==2 && table->GamePlayerData.chips_won[i] > 0) {

			// Game over and this player is receiving chips.  Animate them

			// from the pot to his location.

			// Start the animation if necessary.

			if (table->AnimChips[i].state==ANIM_STATE_COMPLETE && !old_chips_game_over_completed) {

				// Previous animation is complete... start a new one.

				//kp(("%s(%d) Beginning chip animation for player %d at end game. chips_won = %d\n", _FL, i, table->GamePlayerData.chips_won[i]));

				POINT old_point, new_point;

				GetPictureCoordsOnTable(table, IDC_POT1, &old_point);

				GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &new_point);
	

				if (table->animation_disable_count) {

					table->chips_game_over_completed = TRUE;

					table->AnimChips[i].SetPoint(&new_point);

				} else {

					table->AnimChips[i].Init(&old_point, &new_point, 900);

					if (table->active_table_window && !Defaults.iSoundsDisabled && table->animation_disable_count <= 1) {

						PPlaySound(SOUND_CHIPS_DRAGGING);

					}

				}

				// Adjust the values we use to display the chips...

				int chips_from_pot = table->GamePlayerData.chips_won[i];

				table->player_display_chips[i] = chips_from_pot;

				pr(("%s(%d) Changing chips_in_fron_of_player[%d] from %d to %d\n",

						_FL, i, table->player_display_chips[i], chips_from_pot));

				table->GamePlayerData.chips_in_front_of_player[i] = chips_from_pot;

			  #if 0	//20000726MB

					// this code is commented out because it interfered with the

					// bad beat jackpot payout code.  We couldn't figure out why

					// it was here the in the first place, so it has simply been

					// replaced by something much simpler.

				int j = 0;

				while (chips_from_pot && j<MAX_PLAYERS_PER_GAME) {

					if (table->display_pots[j]) {	// this pot has chips.

						int x = min(table->display_pots[j], chips_from_pot);

						table->display_pots[j] -= x;

						chips_from_pot -= x;

					}

					j++;

				}

			  #else

				// Any time there's a payout to any player, we want to make

				// sure there's nothing left in the middle of the table,

				// otherwise players may wonder if the house is keeping it

				// (which it never does).

				for (int j=0 ; j<MAX_PLAYERS_PER_GAME ; j++) {

					table->display_pots[j] = 0;

				}

			  #endif

			}

			table->AnimChips[i].Update();	// Update the animation

			// If the animation is finished, deal with that

			if (table->AnimChips[i].state==ANIM_STATE_COMPLETE) {

				//kp(("%s(%d) Player %d's chips are finished moving.\n",_FL,i));

				table->chips_game_over_completed = TRUE;

				//hack: table->GamePlayerData.chips_won[i] = 0;	// stop animation from occuring again.

			} else {

				// The animation is not yet finished.  Keep displaying

				// our old pots.

				table->animation_flag = TRUE;	// we're still animating.

			}

		} else {	// These chips do not need animating.

			if (game_state != 2) {	// not game over?

				table->player_display_chips[i] = table->GamePlayerData.chips_in_front_of_player[i];

				// 990720HK: if the number of chips in front of him has changed, play a sound

				if (table->player_display_chips[i] != table->player_display_chips_last_sound[i]) {

					table->player_display_chips_last_sound[i] = table->player_display_chips[i];

					// ... but only play a sound if he's got chips in front of him.  if it went to

					// zero, the sound will be taken care of elsewhere

					//kp(("%s(%d) table->animation_disable_count = %d\n", _FL, table->animation_disable_count));

					if (table->active_table_window && table->player_display_chips[i] && !Defaults.iSoundsDisabled && table->animation_disable_count <= 1) {

						PPlaySound(SOUND_CHIPS_BETTING);

					}

				}

			}

			POINT pt;

			GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &pt);

			table->AnimChips[i].SetPoint(&pt);

		}

	}

	if (!keep_old_pots) {

		// Update our display pots with the current pots

		int old_display_total = 0;

		int new_total = 0;

		for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			old_display_total += table->display_pots[i];

			new_total += table->GamePlayerData.pot[i];

			table->display_pots[i] = table->GamePlayerData.pot[i];

		}

		//kp(("%s(%d) Copying main pots (total=%d) to our display pots (total was %d).\n", _FL, new_total, old_display_total));

		table->previous_preferred_stack_size = preferred_stack_size;

	}



	// --- Draw all the pots we have room for ---

	for (i=0 ; i<DISPLAY_POT_ID_COUNT-1 ; i++) {

		GetPictureCoordsOnTable(table, PotIDs[i], &pt);

		DrawChips(&table->blit_queue, &pt, 0, table->display_pots[i], 300, table);//

	}



	// Total up how much is in all remaining pots and display it all in the

	// last display pot.

	int total = 0;

	for ( ; i<MAX_PLAYERS_PER_GAME ; i++) {

		total += table->display_pots[i];

	}

	GetPictureCoordsOnTable(table, PotIDs[DISPLAY_POT_ID_COUNT-1], &pt);

	DrawChips(&table->blit_queue, &pt, 0, total, 300, table); //



	// --- Draw the chips for each player ---

	int stack_size = table->previous_preferred_stack_size;

	// If end of game and chips are sliding TO a player, don't use the

	// preferred stack size.

	// If this is a tournament game, all pre-tournament stuff should be done

	// using the stack size, regardless of game_state.

	if (table->chip_type==CT_TOURNAMENT && table->GameCommonData.table_tourn_state <= TTS_DEAL_HIGH_CARD) {

		// This is a tournament table and we're starting

		stack_size = table->GameCommonData.big_blind_amount;

	} else {

		if (game_state==2) {

			stack_size = 0;

		}

	}

	static int holdem_draw_direction[MAX_PLAYERS_PER_GAME] =  {	0,1,1,1,1,0,0,0,0,1 };

	static int stud7_draw_direction[MAX_PLAYERS_PER_GAME]  =  { 0,1,1,1,0,0,0,1 };

	int *draw_direction = holdem_draw_direction;

	if (table->game_rules==GAME_RULES_STUD7 || table->game_rules==GAME_RULES_STUD7_HI_LO) {

		draw_direction = stud7_draw_direction;

	}

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		// If there's a mismatch between the display chips for this player

		// and the current chips in front of this player, we're probably

		// animating his chips and may need to draw two stacks.

		int diff = table->player_display_chips[i] - table->old_chips_in_front_of_player[i];

		if (diff > 0 && table->old_chips_in_front_of_player[i]) {

			// There's still something sitting in front of him.

			pr(("%s(%d) Displaying two stacks for player %d: $%d in motion and and $%d in front of him.\n",

						_FL, i, diff, table->old_chips_in_front_of_player[i]));

			DrawChips(&table->blit_queue,

					&table->AnimChips[i].current_point, stack_size, diff,

					table->AnimChips[i].state==ANIM_STATE_COMPLETE ? 300 : 400,

					table,

					draw_direction[i]);



			// Display the old chips_in_front_of_player amount still sitting

			// in front of the player (it gets cleared when the animation is

			// completed).

			GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &pt);

			DrawChips(&table->blit_queue, &pt, stack_size,

					table->old_chips_in_front_of_player[i],

					300, table,

					draw_direction[i]); 

		} else {

			DrawChips(&table->blit_queue, &table->AnimChips[i].current_point,

					stack_size, table->player_display_chips[i],

					table->AnimChips[i].state==ANIM_STATE_COMPLETE ? 300 : 400,

					table,

					draw_direction[i]);

		}

	}



  #if DRAW_EVERYTHING	// Used only for positioning things in the dialog box

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		GetPictureCoordsOnTable(table, PlayerChipsControlIDs[i], &pt);

	//	DrawChips(&table->blit_queue, &pt, 400, 1500, 902, table->game_rules , draw_direction[i],table);
        		DrawChips(&table->blit_queue,&pt,400,1000,900,table,1);

        //Fonseca 
//		if (table->game_rules != GAME_RULES_STUD7 && table->game_rules != GAME_RULES_STUD7_HI_LO) {

			GetPictureCoordsOnTable(table, PlayerButtonIDs[i], &pt);

			table->blit_queue.AddBitmap(ButtonBitmap, &pt, 901);
			
//		}

	}

   #if 0	//19990809MB

	for (i=0 ; i<1 ; i++)

   #else

	for (i=0 ; i<DISPLAY_POT_ID_COUNT ; i++)

   #endif

	{

		GetPictureCoordsOnTable(table, PotIDs[i], &pt);

		DrawChips(&table->blit_queue, &pt, 2000, 248400, 901, table);

	}

	GetPictureCoordsOnTable(table, IDC_CHIPS_RAKE, &pt);

	DrawChips(&table->blit_queue, &pt, 500, 100, 901, table);

  #endif

  #if DRAW_ALL_CHIP_STACKS	// used to test new chip stack artwork

	int index = 0;

	for (i=0 ; i<3 ; i++) {
	
		GetPictureCoordsOnTable(table, PotIDs[i], &pt);

		pt.y += i*8;	// space them out a little bit more than usual.

		for (int j=0 ; j<=TOTAL_CHIP_BITMAPS/3 && index < TOTAL_CHIP_BITMAPS ; j++) {

			DrawChips(&table->blit_queue, &pt,

					PlayChipBitmaps[index].amount,	// preferred stack size

					PlayChipBitmaps[index].amount,	// # of chips to draw

					901, table);

			pt.x += STACK_X_SPACING;

			index++;

		}

	}

  #endif	// DRAW_ALL_CHIP_STACKS

}	

