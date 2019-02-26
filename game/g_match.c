/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of wolfX source code.

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 22.10/12
Updated: 18.04/13
===========================================================================
*/
#include "g_local.h"

// OSP
char *aTeams[TEAM_NUM_TEAMS] = { "FFA", "^1Axis^7", "^4Allies^7", "Spectators" };
team_info teamInfo[TEAM_NUM_TEAMS];
/*
=================
Countdown

Causes some troubles on client side so done it here.
=================
*/
void CountDown(qboolean restart){
	gentity_t *other;
	char *index="";

	if (level.CNyes == qfalse) {
		return;
	}
		// Countdown...
	if (level.CNstart == 0) { //index = "prepare.wav";
		if (level.clients->pers.connected == CON_CONNECTED)
            AAPS("sound/match/prepare.wav");
		if (!restart) AP( "cp \"Prepare to fight^1!\n\"2" );}
	if (level.CNstart == 1) { //index = "cn_5.wav";
		if (!restart) AP( "cp \"Match resumes in: ^15\n\"2" ); 	 }
	if (level.CNstart == 2) { //index = "cn_4.wav";
		if (!restart) AP( "cp \"Match resumes in: ^14\n\"2" );     }
	if (level.CNstart == 3) {// index = "cn_3.wav";
		if (!restart) AP( "cp \"Match resumes in: ^13\n\"2" );    }
	if (level.CNstart == 4) { //index = "cn_2.wav";
		if (!restart) AP( "cp \"Match resumes in: ^12\n\"2" );     }
	if (level.CNstart == 5) { //index = "cn_1.wav";
		if (!restart) AP( "cp \"Match resumes in: ^11\n\"2" );	 }
	// Pushes forward. Could be done in 5 but then there's a sound bug ..
	if (level.CNstart == 6 ) { level.HAprintnum++;	 }

	// Prepare to fight takes 2 seconds..
	if(level.CNstart == 0){
		level.CNpush = level.time+2000;
	// Just enough to fix the bug and skip to action..
	} else if (level.CNstart == 6) {
		level.CNpush = level.time+200;
	// Otherwise, 1 second.
	} else {
		level.CNpush = level.time+1000;
	}

	// We're done.. restart the game
	if (level.CNstart == 7) {
		if (restart) {
			level.warmupTime += 10000;
			trap_Cvar_Set( "g_restarted", "1" );
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
		} else {
			// Resume the match..
			resetPause();
			AAPS("sound/match/fight.wav");
			AP("cp \"^1FIGHT\n\"2");
		}
	return;
	}

	other = g_entities;
//	if (level.clients->pers.connected == CON_CONNECTED)
//		doSound(other, EV_ANNOUNCER_SOUND, "sound/scenaric/", va("%s", index));
//	if (level.clients->pers.connected == CON_CONNECTED)
	//	AAPS(va("sound/match/%s", index));
level.CNstart++;  // push forward each frame.. :)
}

/*
=================
Pause countdown
=================
*/
void PauseHandle( void ) {

	if (level.paused == !PAUSE_NONE) {
		// TODO: Add auto timeout..
		if (level.paused != PAUSE_UNPAUSING) {
			if (!g_duelAutoPause.integer)
				AP( va("cp \"Call a vote to resume the match.\n Timeouts remaining: ^1A^7(%i)/^4A^7(%i)\n\"",
					g_pauseLimit.integer - level.axisTimeouts, g_pauseLimit.integer - level.alliedTimeouts));
			else
				AP("cp \"Match will resume once teams are even!\n\"");
		} else {
			level.paused = PAUSE_UNPAUSING;
			AP( "print \"Prepare to fight!\n\"" );
			APS("sound/scenaric/prepare.wav");
		}
	}

	if (level.paused == PAUSE_UNPAUSING) {
		CountDown(qfalse);
	}
}

// So it can be called from elsewhere..
void resetPause( void ) {
	trap_SetConfigstring( CS_SCREENFADE, va( "0 %i 150", level.time + 250 ) );
	trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.startTime + level.timeDelta ) );
	trap_SetConfigstring( CS_PAUSED, va( "%i", PAUSE_NONE ));
	level.paused = PAUSE_NONE;
}
/*
=================
Weapon limiting

See if player can spawn with weapon...
TODO: Add first in line...
=================
*/
////////////
// Sort correct limit
int sortWeaponLimit(int weap) {

	if (weap == 6) {
		if (g_maxTeamSniper.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamSniper.integer;
	}

	if (weap == 8) {
		if (g_maxTeamPF.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamPF.integer;
	}

	if (weap == 9) {
		if (g_maxTeamVenom.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamVenom.integer;
	}

	if (weap == 10) {
		if (g_maxTeamFlamer.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamFlamer.integer;
	}

return g_maxclients.integer;
}
////////////
// See if weapon can be used..
int isWeaponLimited( gclient_t *client, int weap ) {
	int count=0;

	// Limit
	if (( weap == 6 ) && ( client->pers.restrictedWeapon != WP_MAUSER ) )
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisSniper : level.alliedSniper;
	else if (( weap == 8 ) && ( client->pers.restrictedWeapon != WP_PANZERFAUST ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisPF : level.alliedPF;
	else if (( weap == 9 )  && ( client->pers.restrictedWeapon != WP_VENOM ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisVenom : level.alliedVenom;
	else if (( weap == 10 ) && ( client->pers.restrictedWeapon != WP_FLAMETHROWER ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer : level.alliedFlamer;

	if (count >= sortWeaponLimit(weap))
		return 1;
	else
		return 0;

return 0;
}
/*
================
Default weapon

Accounts for "selected weapon" as well.
================
*/
///////////
// Deals only with soldier for weapon restrictions (To avoid breaking anything..).
void setDefWeap(gclient_t *client, int clips) {
	if (client->sess.sessionTeam == TEAM_RED)
	{
		COM_BitSet(client->ps.weapons, WP_MP40);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * clips);
		client->ps.weapon = WP_MP40;
	} else {
		COM_BitSet(client->ps.weapons, WP_THOMPSON);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * clips);
		client->ps.weapon = WP_THOMPSON;
	}
}
///////////
// Deals with weapons
//
// NOTE: Selected weapons only works for eng and med..sold and lt can pick their weapons already..
//       so setting it can potentialy overlap with client spawn scripts..
void setDefaultWeapon(gclient_t *client, qboolean isSold) {


}
