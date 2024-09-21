//****************************************************************
//

// Cards.cpp : Card drawing related routines for the client tables

//

//****************************************************************



#define DISP 0



#include "stdafx.h"

#include "resource.h"



#define DRAW_EVERYTHING  0		// for testing: everything gets filled up.

#define DRAW_ALL_CARDS	 0	// for testing: draw every card



// Order of cards in CardBitmaps array:

//		52	regular 52 cards (index = suit*CARD_RANKS + rank)

//		13	blue diamonds (index = rank)

//		13	green clubs (index = rank)

//		52+13+13 repeat of above, but with grey background

//		1	full size back of card

//		1	small size back of card



#define SUIT_INDEX_BLUE_DIAMONDS	4

#define SUIT_INDEX_GREEN_CLUBS		5

#define SUITS_TO_LOAD				6	// pretend we have 6 suits to load.



#define TOTAL_CARD_BITMAPS	(CARD_RANKS*SUITS_TO_LOAD*2+2)



HBITMAP CardBitmaps[TOTAL_CARD_BITMAPS];



#define CARD_BACK_INDEX			(CARD_RANKS*SUITS_TO_LOAD*2)	// index into CardBitmaps for the 'back' card

#define CARD_SMALL_BACK_INDEX	(CARD_RANKS*SUITS_TO_LOAD*2+1)	// index into CardBitmaps for the small 'back' card



static int PlayerCardsControlIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_CARDS_PLR1,

			IDC_CARDS_PLR2,

			IDC_CARDS_PLR3,

			IDC_CARDS_PLR4,

			IDC_CARDS_PLR5,

			IDC_CARDS_PLR6,

			IDC_CARDS_PLR7,

			IDC_CARDS_PLR8,

			IDC_CARDS_PLR9,

			IDC_CARDS_PLR10,

};



static int PlayerSmallCardsControlIDs[MAX_PLAYERS_PER_GAME] = {

			IDC_SMALL_CARD_PLR1,

			IDC_SMALL_CARD_PLR2,

			IDC_SMALL_CARD_PLR3,

			IDC_SMALL_CARD_PLR4,

			IDC_SMALL_CARD_PLR5,

			IDC_SMALL_CARD_PLR6,

			IDC_SMALL_CARD_PLR7,

			IDC_SMALL_CARD_PLR8,

			IDC_SMALL_CARD_PLR9,

			IDC_SMALL_CARD_PLR10,

};



//*********************************************************

//
//

// Load the card bitmaps into memory.

//

void LoadCardBitmaps(HPALETTE hpal)

{

	HBITMAP first_card_found = 0;



	// Load the individual cards...

	for (int style=0 ; style<2 ; style++) {

		for (int suit=0 ; suit<SUITS_TO_LOAD ; suit++) {

			char *color_char = "";

			if (suit==SUIT_INDEX_BLUE_DIAMONDS) {

				color_char = "b";

			} else if (suit==SUIT_INDEX_GREEN_CLUBS) {

				color_char = "n";

			}

			for (int rank=0 ; rank<CARD_RANKS ; rank++) {

				int i = style*CARD_RANKS*SUITS_TO_LOAD + suit*CARD_RANKS + rank;

				if (!CardBitmaps[i]) {

					// This one needs loading.

					char fname[MAX_FNAME_LEN];

					char *suits = "cdhsdc";	// note: 6 suits, not 4.

					/*

					sprintf(fname, "%c%c%s%s.bmp", cRanks[rank], suits[suit],

							color_char, style ? "g" : "");

					*/



					sprintf(fname, "media\\pc%c%c%s%s.bmp", cRanks[rank], suits[suit],

							color_char, style ? "g" : "");

					



					//kp(("%s(%d) %d/%d/%2d index = %3d, fname = %s\n", _FL, style, suit, rank, i, fname));

					CardBitmaps[i] = LoadBMPasBitmap(FindMediaFile(fname), hpal);

					if (!CardBitmaps[i]) {

						Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, fname);

						iBitmapLoadingProblem = TRUE;

					} else if (!first_card_found) {

						first_card_found = CardBitmaps[i];

					}

				}

			}

		}

	}



	// Load the overturned (back) cards

	if (!CardBitmaps[CARD_BACK_INDEX]) {

		/*

		CardBitmaps[CARD_BACK_INDEX] = LoadBMPasBitmap(FindFile("back.bmp"), hpal);

		if (!CardBitmaps[CARD_BACK_INDEX]) {

			Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, "back.bmp");

			iBitmapLoadingProblem = TRUE;

		}

	    */



        CardBitmaps[CARD_BACK_INDEX] = LoadBMPasBitmap(FindMediaFile("media\\crd_back1.bmp"), hpal);

		if (!CardBitmaps[CARD_BACK_INDEX]) {

			Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, "back.bmp");

			iBitmapLoadingProblem = TRUE;

		}

	}



    /* 

	if (!CardBitmaps[CARD_SMALL_BACK_INDEX]) {

		CardBitmaps[CARD_SMALL_BACK_INDEX] = LoadBMPasBitmap(FindFile("sback.bmp"), hpal);

		if (!CardBitmaps[CARD_SMALL_BACK_INDEX]) {

			Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, "sback.bmp");

			iBitmapLoadingProblem = TRUE;

		}



    */



    if (!CardBitmaps[CARD_SMALL_BACK_INDEX]) {

		CardBitmaps[CARD_SMALL_BACK_INDEX] = LoadBMPasBitmap(FindMediaFile("media\\crd_back2.bmp"), hpal);

		if (!CardBitmaps[CARD_SMALL_BACK_INDEX]) {

			Error(ERR_ERROR, "%s(%d) Could not load '%s'", _FL, "sback.bmp");

			iBitmapLoadingProblem = TRUE;

		}







	}



	// For anything that was not loaded, use the first card we found.

	for (int i=0 ; i<TOTAL_CARD_BITMAPS ; i++) {

		if (!CardBitmaps[i]) {

			CardBitmaps[i] = first_card_found;

			iBitmapLoadingProblem = TRUE;

		}

	}

}



//*********************************************************

//
//

// Draw a hand of cards at a specific coordinate.

// card_array points to the cards.  card_count tells how many cards

// are in that array.  style=0 produces seperated cards (for the flop).

// style=1 produces overlapped cards, style=2 is for small overlapped cards

//

void DrawCards(BlitQueue *q, LPPOINT dest_pt, Card *card_array, int card_count, int style, int draw_priority, GameRules game_rules, int our_cards_flag)

{

	#define CARD_WIDTH				50

	#define CARD_FLOP_X_SPACING		(CARD_WIDTH+4)

	#define CARD_OVERLAP_X_SPACING	16

	#define CARD_OVERLAP_Y_SPACING	2

	#define CARD_OVERLAP_X_SPACING_SMALL 5

	#define CARD_OVERLAP_Y_SPACING_SMALL 2

	#define CARD_SHIFT_Y_SPACING	10	// amount to shift visible cards in 7CS



	if (!q) {

		return;	// nowhere to draw.

	}

	POINT pt = *dest_pt;

	int cards_drawn = 0;

	while (card_count > 0) {

		if (*card_array != CARD_NO_CARD) {

			int card_index;

			if (*card_array == CARD_HIDDEN) {

				card_index = style==2 ? CARD_SMALL_BACK_INDEX : CARD_BACK_INDEX;

			} else {

				card_index = SUIT(*card_array)*CARD_RANKS + RANK(*card_array);

				if (Defaults.i4ColorDeckEnabled) {

					if (SUIT(*card_array)==Diamonds) {

						card_index = SUIT_INDEX_BLUE_DIAMONDS*CARD_RANKS + RANK(*card_array);

					} else if (SUIT(*card_array)==Clubs) {

						card_index = SUIT_INDEX_GREEN_CLUBS*CARD_RANKS + RANK(*card_array);

					}

				}

			}

			// If this is 7CS, use grey cards when appropriate.

			// Use grey version for cards 0,1, and 6

			if (game_rules==GAME_RULES_STUD7 || game_rules==GAME_RULES_STUD7_HI_LO) {

				if (card_index <=SUITS_TO_LOAD*CARD_RANKS && style==1 && 

					(cards_drawn==0 || cards_drawn==1 || cards_drawn==6)) {

					card_index += SUITS_TO_LOAD*CARD_RANKS;

				}

				if (our_cards_flag && cards_drawn==2) {

					// Shift up...

					pt.y -= CARD_SHIFT_Y_SPACING;

				}

				if (our_cards_flag && cards_drawn==6) {

					// Shift back down...

					pt.y += CARD_SHIFT_Y_SPACING;

				}

			}

			q->AddBitmap(CardBitmaps[card_index], &pt, draw_priority);

			cards_drawn++;

		}

		if (style==0) {	// flop style

			pt.x += CARD_FLOP_X_SPACING;

		} else if (style==1) {	// large overlapped hand style

			pt.x += CARD_OVERLAP_X_SPACING;

			pt.y += CARD_OVERLAP_Y_SPACING;

		} else {				// small overlapped hand style

			pt.x += CARD_OVERLAP_X_SPACING_SMALL;

			pt.y += CARD_OVERLAP_Y_SPACING_SMALL;

		}

		card_array++;

		card_count--;

	}

}	

void DrawCards(BlitQueue *q, LPPOINT dest_pt, Card *card_array, int card_count, int style, int draw_priority, GameRules game_rules)

{

	DrawCards(q, dest_pt, card_array, card_count, style, draw_priority, game_rules, FALSE);

}



//*********************************************************

//
//

// Draw all the cards at a table.  This includes all players and

// the flop.  This function might be called when the cards update

// OR when processing WM_PAINT.

//

void DrawAllCards(struct TableInfo *t)

{

#if DRAW_ALL_CARDS	// draw all cards for testing purposes

	{

		POINT pt = {100, 50};

		int j = 0;

		POINT pt2 = pt;

		int pri = 500;

		for (int i=0 ; i<TOTAL_CARD_BITMAPS ; i++, pri++) {

			t->blit_queue.AddBitmap(CardBitmaps[i], &pt2, pri);

			pt2.x += 22;

			j++;

			if (j==26) {	// end of row reached

				pt.y += 35;	// move down

				pt2 = pt;	// start new row

				j = 0;

			}//if j==26

		}// for

	}//draw all

	return;

  #endif

	// Retrieve location of the deck (where the cards come from/go to)

	POINT deck_pt;


	GetPictureCoordsOnTable(t, IDC_DECK_LOCATION, &deck_pt);



	// Don't allow cards to disappear too soon after finished dealing.

	int ms_since_last_dealt_card = GetTickCount() - t->dwLastDealtCardTicks;

	int freeze_cards = FALSE;	// set to prevent cards from folding/mucking.

	if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

		if (ms_since_last_dealt_card < 2000) {

			freeze_cards = TRUE;

		}

	} else {

		if (ms_since_last_dealt_card < 600) {

			freeze_cards = TRUE;

		}

	}



	// --- Draw the flop ---



	// Watch of the game over flag goes from true to false.  That signals

	// the start of a new game.

	int new_game_flag = FALSE;

	if (t->card_previous_game_over_flag && !t->GamePlayerData.s_gameover) {

		new_game_flag = TRUE;

	}

	t->card_previous_game_over_flag = t->GamePlayerData.s_gameover;



	// If it's the start of a new game, initialize our display cards.

	if (new_game_flag) {

		memset(t->display_common_cards, CARD_NO_CARD, MAX_PUBLIC_CARDS);

		for (int i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			memset(t->display_cards[i], CARD_NO_CARD, MAX_PRIVATE_CARDS);

			memset(t->dealt_cards[i], CARD_NO_CARD, MAX_PRIVATE_CARDS);

		}

		t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

	}



	// Check if any new cards have been dealt...

	int i, j;

	for (j=0 ; j<MAX_PUBLIC_CARDS ; j++) {

		if (t->display_common_cards[j]        == CARD_NO_CARD &&

			t->GamePlayerData.common_cards[j] != CARD_NO_CARD)

		{

			t->new_cards_dealt_flag = TRUE;

		}

	}

	//20000215MB: Make a pass through the new cards from the server

	// and update our 'dealt cards' array.

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		for (j=0 ; j<MAX_PRIVATE_CARDS ; j++) {

			if (t->GamePlayerData.cards[i][j] != CARD_NO_CARD) {

				// always copy real cards.... they must get displayed at

				// least once

				if (t->dealt_cards[i][j] != t->GamePlayerData.cards[i][j]) {

					pr(("%s(%d) dealing card $%02x to slot [%d][%d] (slot used to have $%02x)\n",

							_FL, t->GamePlayerData.cards[i][j], i, j, t->dealt_cards[i][j]));

					t->dealt_cards[i][j] = t->GamePlayerData.cards[i][j];

				}

			} else {

				// Server says no card in this slot... only clear from dealt_cards

				// if it has been displayed.

				if (t->dealt_cards[i][j] != CARD_NO_CARD &&

					t->display_cards[i][j] != CARD_NO_CARD)

				{

					// It's ok to clear it... it made it to the display array.

					if (!freeze_cards) {

						pr(("%s(%d) %dms after dealing... folding card $%02x from slot [%d][%d]\n",

								_FL, ms_since_last_dealt_card, t->dealt_cards[i][j], i, j));

						t->dealt_cards[i][j] = t->GamePlayerData.cards[i][j];

					} else {

						pr(("%s(%d) %dms after dealing... can't fold yet $%02x from [%d][%d].\n",

								_FL, ms_since_last_dealt_card, t->dealt_cards[i][j], i, j));

					}

				}

			}

		}

	}



	// Now check if we've still got cards in the dealt cards array

	// that need to be displayed.

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		for (j=0 ; j<MAX_PRIVATE_CARDS ; j++) {

			if (t->display_cards[i][j] == CARD_NO_CARD &&

				t->dealt_cards[i][j]   != CARD_NO_CARD)

			{

				t->new_cards_dealt_flag = TRUE;

			}

		}

	}



	// If animation is disabled, just copy the real structures to the display

	// structures so that no animation will occur.

	if (t->animation_disable_flag || t->animation_disable_count) {

		memcpy(t->display_common_cards, t->GamePlayerData.common_cards, sizeof(Card)*MAX_PUBLIC_CARDS);

		for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			memcpy(t->display_cards[i], t->GamePlayerData.cards[i], MAX_PRIVATE_CARDS);

			memcpy(t->dealt_cards[i], 	t->GamePlayerData.cards[i], MAX_PRIVATE_CARDS);

		}

	}



	// Draw any cards we've already finished animating (underneath the

	// new cards).

	POINT pt;

	GetPictureCoordsOnTable(t, IDC_CARDS_FLOP, &pt);

	DrawCards(&t->blit_queue, &pt, t->display_common_cards, MAX_PUBLIC_CARDS, 0, 100, t->game_rules);



	if (memcmp(t->display_common_cards, t->GamePlayerData.common_cards, sizeof(Card)*MAX_PUBLIC_CARDS)) {

		//kp(("%s(%d) Common cards have changed.  Updating anim state (current = %d)\n",_FL,t->flop_animation_state));

		t->animation_flag = TRUE;	// we're still animating.

		// Determine the first card which needs updating.

		int card_index = 0;

		while (card_index < MAX_PUBLIC_CARDS && 

				t->display_common_cards[card_index] ==

				t->GamePlayerData.common_cards[card_index]) {

			card_index++;

		}

		//kp(("%s(%d) card_index = %d\n", _FL, card_index));

		// Offset the dest coord by the offset to the first

		// card that is different.

		pt.x += CARD_FLOP_X_SPACING*card_index;



		if (t->flop_animation_state==FLOP_ANIM_INIT) {

			// start animating from the dealer down to the flop area.

			t->AnimCommonCards.Init(&deck_pt, &pt, 450);

			if (t->active_table_window && !Defaults.iSoundsDisabled && t->animation_disable_count <= 1) {

				PPlaySound(SOUND_FLOP_DEALING);

			}

			t->flop_animation_state = FLOP_ANIM_SLIDING_DOWN;

		}

		if (t->flop_animation_state==FLOP_ANIM_SLIDING_DOWN) {

			// Cards are currently sliding down from the dealer to the

			// flop area.  Move them and draw them.

			t->AnimCommonCards.Update();

			Card c = CARD_HIDDEN;

			DrawCards(&t->blit_queue, &t->AnimCommonCards.current_point, &c, 1, 0, 500, t->game_rules);

			if (t->AnimCommonCards.state==ANIM_STATE_COMPLETE) {

				// Finished moving.  Move on to the next stage.

				t->flop_animation_state = FLOP_ANIM_FLIPPING_OVER;

			}

		}

		if (t->flop_animation_state==FLOP_ANIM_FLIPPING_OVER) {

			// Flipping over is easy for now... just do nothing :)

			t->flop_animation_state = FLOP_ANIM_SLIDING_RIGHT_INIT;	// next state.

		}

		if (t->flop_animation_state==FLOP_ANIM_SLIDING_RIGHT_INIT) {

			// Initialize animation for sliding the next card to the right.

			POINT pt2 = pt;

			pt2.x += CARD_FLOP_X_SPACING;

			t->AnimCommonCards.Init(&pt, &pt2, 350);

			t->flop_animation_state = FLOP_ANIM_SLIDING_RIGHT;	// next state.

			// Copy this card_index's card so that it gets displayed under

			// the sliding card.

			t->display_common_cards[card_index] = t->GamePlayerData.common_cards[card_index];

			//kp(("%s(%d) Processed FLOP_ANIM_SLIDING_RIGHT_INIT for card index %d\n", _FL, card_index));

		}

		if (t->flop_animation_state==FLOP_ANIM_SLIDING_RIGHT) {

			t->AnimCommonCards.Update();

			// Display the last new card.

			Card c;

			int last_card = card_index;

			while (last_card<MAX_PUBLIC_CARDS-1 && 

					t->GamePlayerData.common_cards[last_card+1]!=CARD_NO_CARD) {

				last_card++;

			}

			//kp(("%s(%d) card_index = %d, last_card index = %d\n", _FL, card_index, last_card));

			c = t->GamePlayerData.common_cards[last_card];

			DrawCards(&t->blit_queue, &t->AnimCommonCards.current_point, &c, 1, 0, 500, t->game_rules);

			if (t->AnimCommonCards.state==ANIM_STATE_COMPLETE) {

				// Finished moving.  Move on to the next stage.

				if (card_index < last_card) {	// more cards to do?

					t->flop_animation_state = FLOP_ANIM_SLIDING_RIGHT_INIT;

				} else {	// all done.

					t->flop_animation_state = FLOP_ANIM_INIT;

					// Done animating.  Move all cards to regular array to be displayed using the old method.

				  #if 1	//20000708MB

					memcpy(t->display_common_cards, t->GamePlayerData.common_cards, sizeof(Card)*MAX_PUBLIC_CARDS);

				  #else

					kp1(("%s(%d) *** WARNING: FLOP ANIMATION NEVER ENDS ***\n", _FL));

					memset(t->display_common_cards, CARD_NO_CARD, MAX_PUBLIC_CARDS);

				  #endif

				}

			}

		}

	} else {

		t->flop_animation_state = FLOP_ANIM_INIT;	// reset animation state

	}



	// --- Determine if we're dealing or not ---

	// Make a pass through all the players and see if any have been dealt

	// new cards.  If so, we're in 'dealing' mode and must be careful

	// which cards we display for all the other players.

	if (t->animation_disable_flag || t->animation_disable_count) {

		// When minimized, force cards into everyone's hands so

		// animations don't need to occur.

		for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

			memcpy(t->display_cards[i], t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

			memcpy(t->dealt_cards[i],	t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

		}

		t->dealing_flag = FALSE;

	}



	int dealing_flag = FALSE;

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		if (t->GamePlayerData.player_status[i]==PLAYER_STATUS_PLAYING) {

			// If cards have just appeared in a player's hand, we're dealing.

			for (int j=0 ; j < MAX_PRIVATE_CARDS ; j++) {

				if (t->dealt_cards[i][j]!=CARD_NO_CARD &&

					t->display_cards[i][j]==CARD_NO_CARD) {

					// Player has just received cards.  We're dealing.

					dealing_flag = TRUE;

					//kp(("%s(%d) We're still dealing.  Now at player %d, card %d\n",_FL, t->card_deal_player, t->card_deal_index));

					break;

				}

			}

		}

	}

	if (t->dealing_flag && !dealing_flag) {	// dealing just stop?

		// Make sure we do one final redraw of the player id boxes.

		t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

	}

	//kp(("%s(%d) changing t->dealing_flag from %d to %d\n", _FL, t->dealing_flag, dealing_flag));

	t->dealing_flag = dealing_flag;



	// --- Draw the cards for each player ---

	// Draw the cards already in each player's hand

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		// Determine which style to display the cards in (large or small)

		int card_style;

		Card *c = dealing_flag ? t->display_cards[i] : t->dealt_cards[i];

		if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

			// Always use big cards

			card_style = 1;

		} else {

			// Hold'em shortcut: if first card is visible, use large cards, otherwise use small

			if (*c!=CARD_HIDDEN && *c!=CARD_NO_CARD) {

				card_style = 1;

			} else {

				card_style = 2;

			}//if

		}//if

		POINT card_pt;

		if (card_style==1) {

			GetPictureCoordsOnTable(t, PlayerCardsControlIDs[i], &card_pt);

		} else {

			GetPictureCoordsOnTable(t, PlayerSmallCardsControlIDs[i], &card_pt);

		}//if



		if (t->dealt_cards[i][0]   == CARD_NO_CARD &&

		    t->display_cards[i][0] != CARD_NO_CARD)

		{

			//kp(("%s(%d) Player %d has just mucked cards (or folded).\n", _FL, i));



			if (t->AnimCards[i].state==ANIM_STATE_COMPLETE) {

				// Previous animation is complete (if there was any)

				// Start the new one.

				//kp(("%s(%d) Starting to muck cards for player %d\n", _FL, i));

				t->AnimCards[i].Init(&card_pt, &deck_pt, 250);

				if (t->active_table_window && !Defaults.iSoundsDisabled && t->animation_disable_count <= 1) {

					PPlaySound(SOUND_CARDS_FOLDING);

				}//if

			}//if

			t->AnimCards[i].Update();

			// If we're finished animating these cards, stop drawing them.

			if (t->AnimCards[i].state==ANIM_STATE_COMPLETE) {

				//kp(("%s(%d) Finished mucking cards for player %d\n", _FL, i));

				memcpy(t->display_cards[i], t->dealt_cards[i], sizeof(t->dealt_cards[i][0])*MAX_PRIVATE_CARDS);

				t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

			}//if

			t->animation_flag = TRUE;	// we're still animating.

			c = t->display_cards[i];	// show old cards until anim is over.

		} else {

			// No animation necessary... just keep display cards up to date.

			//kp(("%s(%d) Player %d's cards don't need animating.\n", _FL, i));

			if (!dealing_flag) {	// don't copy them all if still dealing.

				if (t->display_cards[i][0] != t->dealt_cards[i][0] &&

					t->dealt_cards[i][0]   != CARD_HIDDEN &&

					t->display_cards[i][0] != CARD_NO_CARD)

				{

					// card probably got flipped over (shown)

					if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

						t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

						//kp(("%s(%d) cards got flipped in stud... flagging player id box redraw.\n",_FL));

					}//if 

					memcpy(t->display_cards[i], t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

					memcpy(t->dealt_cards[i],   t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

				}//if

				if (!freeze_cards) {

					memcpy(t->display_cards[i], t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

					memcpy(t->dealt_cards[i],   t->GamePlayerData.cards[i], sizeof(t->GamePlayerData.cards[i][0])*MAX_PRIVATE_CARDS);

					pr(("%s(%d)\n",_FL));

				}//if ok

			}//if

			t->AnimCards[i].SetPoint(&card_pt);

		}



		int draw_priority = 100;	// laying flat

		if (t->AnimCards[i].state!=ANIM_STATE_COMPLETE) {

			draw_priority = 500;	// animated cards

		}

		if (card_style==1) {

			draw_priority = 600;	// large private cards

		}

		//kp(("%s(%d) We're in seat %d.  Drawing cards for player %d\n", _FL, t->GamePlayerData.seating_position, i));

		// Finally, draw them where they should be and in the right style.

		int our_cards_flag = FALSE;

		if (!t->watching_flag && i==t->GamePlayerData.seating_position) {

			our_cards_flag = TRUE;

		}

		DrawCards(&t->blit_queue, &t->AnimCards[i].current_point, c, MAX_PRIVATE_CARDS,

				card_style, draw_priority, t->game_rules, our_cards_flag);

	}



	// --- Draw the cards currently being dealt (if any) ---

	int first_player = (t->GameCommonData.p_button+1) % MAX_PLAYERS_PER_GAME;	// first player to get cards.

	if (dealing_flag) {

		// We're dealing.  Animate cards from the dealer to the player.

		// This must be done one at a time (obviously).

		// If no animation is in progress, find the next player

		// that needs a card dealt to him.

		if (t->AnimDealtCard.state==ANIM_STATE_COMPLETE) {

			// No animation is in progress.

			// Find next player that needs a card dealt.

			int found_card_to_deal = FALSE;

			forever {

				do  {

					if (

					  #if 0	//20000215MB

						(t->GamePlayerData.player_status[t->card_deal_player]==PLAYER_STATUS_PLAYING ||

						  // 20000129HK: the cause of the all-in card animation problem -- resolved

						 t->GamePlayerData.player_status[t->card_deal_player]==PLAYER_STATUS_ALL_IN) &&

					  #endif

						 t->dealt_cards[t->card_deal_player][t->card_deal_index] != 

						 t->display_cards[t->card_deal_player][t->card_deal_index])

					{

						//kp(("%s(%d) Time to deal to player %d, card #%d\n", _FL, t->card_deal_player, t->card_deal_index));

						found_card_to_deal = TRUE;

						break;

					}

					t->card_deal_player = (t->card_deal_player+1) % MAX_PLAYERS_PER_GAME;	// next player

				} while (t->card_deal_player != first_player);

				if (found_card_to_deal) {

					if (t->active_table_window && !Defaults.iSoundsDisabled && t->animation_disable_count <= 1) {

						PPlaySound(SOUND_CARDS_DEALING); // 990715HK

					}

					break;	// don't need to search any further.

				}

				// We didn't find a card to deal.  Go around again.

				t->card_deal_player = first_player;

				t->card_deal_index++;

				if (t->card_deal_index >= MAX_PRIVATE_CARDS) {

					//kp(("%s(%d) No more cards to deal.  We're done dealing.\n", _FL));

					t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

					t->dwLastDealtCardTicks = GetTickCount();

					goto donedealing;

				}

			}

			//kp(("%s(%d) Starting to deal card #%d to player %d\n", _FL, t->card_deal_index, t->card_deal_player));

			POINT card_pt;

			if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

				GetPictureCoordsOnTable(t, PlayerCardsControlIDs[t->card_deal_player], &card_pt);

				// Adjust the end point depending on how many cards have been dealt already.

				card_pt.x += t->card_deal_index * CARD_OVERLAP_X_SPACING;

				card_pt.y += t->card_deal_index * CARD_OVERLAP_Y_SPACING;

			} else {

				GetPictureCoordsOnTable(t, PlayerSmallCardsControlIDs[t->card_deal_player], &card_pt);

				// Adjust the end point depending on how many cards have been dealt already.

				card_pt.x += t->card_deal_index * CARD_OVERLAP_X_SPACING_SMALL;

				card_pt.y += t->card_deal_index * CARD_OVERLAP_Y_SPACING_SMALL;

			}

			// Determine how many milliseconds to spend animating this card.

			// If the last card was dealt very recently, try to space them

			// perfectly evenly, rather than slowing down as more and more cards

			// need to be drawn.

		  #if 1	//20000912MB

			#define DESIRED_CARD_DEALING_TICKS			90

		  #else

			#define DESIRED_CARD_DEALING_TICKS			150

		  #endif

			int ms_to_deal_this_one = DESIRED_CARD_DEALING_TICKS;	// default

			int ms_since_last = GetTickCount() - t->dwLastDealtCardTicks;

			t->dwLastDealtCardTicks = GetTickCount();

			if (ms_since_last < DESIRED_CARD_DEALING_TICKS*5) {	// last card dealt recently?

				ms_to_deal_this_one = DESIRED_CARD_DEALING_TICKS*2 - ms_since_last;

				ms_to_deal_this_one = max(ms_to_deal_this_one, DESIRED_CARD_DEALING_TICKS/4);

				ms_to_deal_this_one = min(ms_to_deal_this_one, DESIRED_CARD_DEALING_TICKS);

			}

			//kp(("%s(%d) card animation time = %4dms (ms_since_last = %4dms)\n", _FL, ms_to_deal_this_one, ms_since_last));

			t->AnimDealtCard.Init(&deck_pt, &card_pt, ms_to_deal_this_one);

		}

		t->AnimDealtCard.Update();

		// If we're finished animating this card, add it to the player's hand

		if (t->AnimDealtCard.state==ANIM_STATE_COMPLETE) {

			t->display_cards[t->card_deal_player][t->card_deal_index] =

					t->dealt_cards[t->card_deal_player][t->card_deal_index];

			//kp(("%s(%d) Done animating card #%d for player %d\n", _FL, t->card_deal_index, t->card_deal_player));

			// next player is now eligible for a card.

			t->card_deal_player = (t->card_deal_player+1) % MAX_PLAYERS_PER_GAME;	// next player

			t->invalidate_player_id_boxes = TRUE;	// force redraw so card remnants disappear.

		}

		// Draw the animated card (small and face down)

		Card hidden_card = CARD_HIDDEN;

		//kp(("%s(%d) Drawing card #%d for player %d\n", _FL, t->card_deal_index, t->card_deal_player));

		DrawCards(&t->blit_queue, &t->AnimDealtCard.current_point, &hidden_card, 1, 2, 500, t->game_rules);

		t->animation_flag = TRUE;	// we're still animating.

	} else {

donedealing:

		// We're not dealing.  Reset dealing state.

		t->card_deal_player = first_player;

		t->card_deal_index = 0;			

		t->AnimDealtCard.SetPoint(&deck_pt);

	}



  #if DRAW_EVERYTHING	// Used only for positioning things in the dialog box

	// Draw small cards for everyone

	for (i=0 ; i<MAX_PLAYERS_PER_GAME ; i++) {

		if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

			GetPictureCoordsOnTable(t, PlayerCardsControlIDs[i], &pt);

			Card faceup_hand[2] =  {0,1};

			DrawCards(&t->blit_queue, &pt, faceup_hand, 2, 1, 900, GAME_RULES_STUD7, TRUE);

		} else {

		  #if 0 //19990813MB: draw 4 face down

			GetPictureCoordsOnTable(t, PlayerSmallCardsControlIDs[i], &pt);
			Card facedown_cards[4] =  {CARD_HIDDEN, CARD_HIDDEN, CARD_HIDDEN, CARD_HIDDEN};
			DrawCards(&t->blit_queue, &pt, facedown_cards, 5, 2, 900, t->game_rules);

		  #else	// draw 4 large faceup
			GetPictureCoordsOnTable(t, PlayerCardsControlIDs[i], &pt);
		//	Card faceup_cards[2] =  {0,1};
			Card faceup_cards[5] =  {0,1,2,3,4};
        //  DrawCards(&t->blit_queue, &pt, faceup_cards, 2, 1, 900, t->game_rules); 
			DrawCards(&t->blit_queue, &pt, faceup_cards, 5, 1, 900, t->game_rules);

		  #endif

		}

	}

	if (t->game_rules==GAME_RULES_STUD7 || t->game_rules==GAME_RULES_STUD7_HI_LO) {

		Card full_flop[5] = {0};

		GetPictureCoordsOnTable(t, IDC_CARDS_FLOP, &pt);

		DrawCards(&t->blit_queue, &pt, full_flop, 1, 0, 900, t->game_rules);

	} else {

		Card full_flop[5] = {0,1,2,3,4};

		GetPictureCoordsOnTable(t, IDC_CARDS_FLOP, &pt);

		DrawCards(&t->blit_queue, &pt, full_flop, 5, 0, 900, t->game_rules);

	}

  #endif

}

