#include "wl_def.h"
#pragma hdrstop
#include "bot.h"
#include "queue.h"
#include "util.h"

#define ENEMY_VISIBLE_RANGE 15
#define ENEMY_VISIBLE_CHANGE_DISTANCE 2

#define AMMO_NEED_AMOUNT 80
#define HUNT_NAZIS_HEALTH 75
#define HUNT_NAZIS_AMMO 50
#define ANGLE_TURN_SLOWDOWN 15

#define DISTANCE_SHOOT 10
#define DISTANCE_CHARGE 6
#define ANGLE_FOV 45

#define SOUND_HEAR_TIME_SECONDS 3

static const int RELATIVE_DIR_X[4] = {1, 0, -1,  0};
static const int RELATIVE_DIR_Y[4] = {0, 1,  0, -1};

word far path_prev[MAPSIZE*MAPSIZE];
word destination;
word pushtargetx, pushtargety;

void BotMapInit(void)
{
	destination = 0;
}

objtype* EnemyVisible(int *angle, int *distance)
{
	objtype* ob;
	int tx = (int)player->tilex, ty = (int)player->tiley;
	int i, j, k;
	static objtype* oldob;

	int distmin = INT_MAX;
	fixed dxmin, dymin;
	objtype* obmin = NULL;

	for(ob = player->next; ob; ob = ob->next)
	{
		k = abs(ob->tilex - tx);
		j = abs(ob->tiley - ty);
		i = k > j ? k : j;

		if(i > ENEMY_VISIBLE_RANGE)
			continue;
		if(!(ob->flags & FL_VISABLE)
		&& time(NULL) - ob->lastSoundTime > SOUND_HEAR_TIME_SECONDS)
			continue;
		if(!(ob->flags & FL_SHOOTABLE))
			continue;
		if(ob->hitpoints <= 0)
			continue;
		if(!CheckLine(ob))
			continue;
		if((abs(*distance - i) >= ENEMY_VISIBLE_CHANGE_DISTANCE &&
		    ob != oldob)
		   || ob == oldob
		   || (oldob && !(oldob->flags & FL_SHOOTABLE)))
		{
			if(i <= distmin)
			{
				distmin = i;
				obmin = ob;
			}
		}
	}

	if(obmin)
	{
		oldob = obmin;
		*angle = (short)(180.f / PI_F * atan2(-((float)obmin->y -
							      player->y),
							(float)obmin->x -
							      player->x));
		*distance = distmin;
	}

	return obmin;
}

boolean ObjectOfInterest(word n)
{
	word tx = TILE_DECODE_X(n);
	word ty = TILE_DECODE_Y(n);
	word tx2, ty2;
	objtype* check = actorat[tx][ty];
	statobj_t	*statptr;
	byte i;

	for(statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if(!(statptr->flags & FL_BONUS))
			continue;
		if(statptr->shapenum == -1)
			continue;
		if(statptr->tilex == tx && statptr->tiley == ty)
		{
			switch(statptr->itemnumber)
			{
			case    bo_firstaid:
				if (gamestate.health <= 75)
					return true;
				break;
			case    bo_food:
				if (gamestate.health <= 90)
					return true;
				break;
			case    bo_alpo:
				if (gamestate.health <= 96)
					return true;
				break;
			case    bo_key1:
			case    bo_key2:
			case    bo_key3:
			case    bo_key4:
				if(!(gamestate.keys &
				(1 << (statptr->itemnumber - bo_key1))))
					return true;
				break;
			case    bo_cross:
			case    bo_chalice:
			case    bo_bible:
			case    bo_crown:
				return true;
			case    bo_machinegun:
				if(gamestate.bestweapon < wp_machinegun
				|| gamestate.ammo <= 80)
					return true;
				break;
			case    bo_chaingun:
				if(gamestate.bestweapon < wp_chaingun
				|| gamestate.ammo <= 80)
					return true;
				break;
			case    bo_fullheal:
				return true;
			case    bo_clip:
				if(gamestate.ammo < AMMO_NEED_AMOUNT)
					return true;
				break;
			case    bo_clip2:
				if (gamestate.ammo < AMMO_NEED_AMOUNT)
					return true;
				break;
#ifdef SPEAR
			case    bo_25clip:
				if (gamestate.ammo <= 74)
					return true;
				break;
			case    bo_spear:
				return true;
				break;
#endif
			case    bo_gibs:
				if (gamestate.health <= 10)
					return true;
				break;
			}
		}
	}

	if(check >= objlist && (check->flags & FL_SHOOTABLE)
	&& gamestate.health > HUNT_NAZIS_HEALTH
	&& gamestate.ammo > HUNT_NAZIS_AMMO && check->hitpoints > 0)
		return true;

	// Check for secret
	for(i = 0; i < 4; ++i)
	{
		tx2 = tx + RELATIVE_DIR_X[i];
		ty2 = ty + RELATIVE_DIR_Y[i];
		if(actorat[tx2][ty2] < objlist
		   && !actorat[tx + 2 * RELATIVE_DIR_X[i]]
			      [ty + 2 * RELATIVE_DIR_Y[i]]
		   && *(mapsegs[1]+farmapylookup[ty2] + tx2) == PUSHABLETILE)
		{
			pushtargetx = tx2;
			pushtargety = ty2;
			return true;
		}
	}

	// Check for exit
	for(i = 0; i < 2; ++i)
	{
		tx2 = tx + (i << 1) - 1;
		if(tilemap[tx2][ty] == ELEVATORTILE)
		{
			pushtargetx = tx2;
			pushtargety = ty;
			return true;
		}
	}

	return false;
}

boolean CanGoThruLockedDoor(byte door)
{
	byte lock = doorobjlist[door].lock;
	if(lock >= dr_lock1 && lock <= dr_lock4)
	{
		if(doorobjlist[door].action != dr_open
		&& doorobjlist[door].action != dr_opening
		&& !(gamestate.keys & (1 << (lock - dr_lock1))))
			return false;
	}
	return true;
}

boolean TilePassable(word n)
{
	word tx = TILE_DECODE_X(n);
	word ty = TILE_DECODE_Y(n);
	objtype* check = actorat[tx][ty];
	byte door = tilemap[tx][ty];
	byte lock;

	if(check && check < objlist && !(door & 0x80))
		return false;
	else if(door & 0x80)
	{
		door = door & ~0x80;
		if(!CanGoThruLockedDoor(door))
			return false;
	}
	return true;
}

word FindShortestPath(void)
{
#define PROCEED(n) if(!path_prev[n] && TilePassable(n)){path_prev[n] = m; QUEUE_PUSH(n)}
	word n = TILE_ENCODE(player->tilex, player->tiley);
	word m;
	byte i;
	_fmemset(path_prev,0,sizeof(path_prev));
	QUEUE_RESET
	QUEUE_PUSH(n)
	path_prev[n] = WORD_MAX;
	while(!QUEUE_EMPTY)
	{
		m = QueuePop();
		if(ObjectOfInterest(m))
		{
			return m;
		}
		// add neighbours. don't worry about bounds checking, maps
		// should be guarded anyway
		n = m + 1;
		PROCEED(n)
		n = m - 1;
		PROCEED(n)
		n = m + MAPSIZE;
		PROCEED(n)
		n = m - MAPSIZE;
		PROCEED(n)
	}
	return 0;
#undef PROCEED
}

void TurnToAngle(int dangle)
{
	buttonstate[bt_strafe] = false;

	if(dangle > ANGLE_TURN_SLOWDOWN)
		controlx = -RUNMOVE * tics;
	else if(dangle > 0)
		controlx = -BASEMOVE * tics;
	else if(dangle < -ANGLE_TURN_SLOWDOWN)
		controlx = +RUNMOVE * tics;
	else if(dangle < 0)
		controlx = +BASEMOVE * tics;
}

void DoCombatAI(int angle, int distance)
{
	int dangle = CentreAngle(angle, player->angle);
	static int inc;
	objtype* check;
	WRITELOG((logfile, "DoCombatAI: angle=%d player->angle=%d dangle=%d\n", angle, player->angle, dangle))
	TurnToAngle(dangle);
	check = GunSightTarget();
	++inc;
	if(check)
	{
		if(distance <= DISTANCE_SHOOT)
			if(gamestate.weapon <= wp_pistol)
				buttonstate[bt_attack] = (boolean)(inc % 2);
			else
				buttonstate[bt_attack] = true;
		if(distance > DISTANCE_CHARGE || gamestate.weapon == wp_knife)
			controly = -RUNMOVE * tics;
	}
}

void DoNonCombatAI(void)
{
	static boolean waitpwall;
	word nowon, nexton, mypos, nx, ny;
	fixed nxd, nyd;
	byte tile;
	int tangle, dangle;
	boolean tryuse;
	objtype* check;
	static int inc;
	inc++;

	if(!destination)
		destination = FindShortestPath();
	if(!destination)
		return;
	if(pwallstate)
		waitpwall = true;
	mypos = TILE_ENCODE(player->tilex, player->tiley);
	nowon = nexton = WORD_MAX;
	for(nowon = destination; nowon != WORD_MAX;
	    nexton = nowon, nowon = path_prev[nowon])
	{
		if(nowon == mypos)
			break;
	}
	if(nowon == WORD_MAX || (!pwallstate && waitpwall))
	{
		waitpwall = false;
		destination = 0;
		return;
	}
	if(nexton == WORD_MAX)
	{
//		prevon = path_prev[nowon];
		destination = 0;
		nx = pushtargetx;
		ny = pushtargety;
	}
	else
	{
		nx = TILE_DECODE_X(nexton);
		ny = TILE_DECODE_Y(nexton);
		tile = tilemap[nx][ny];
		if(tile & 0x80)
		{
			tile &= ~0x80;
			if(!CanGoThruLockedDoor(tile))
			{
				destination = 0;
				return;
			}
			tryuse = (doorobjlist[tile].action == dr_closed
			       || doorobjlist[tile].action == dr_closing);
		}
	}
	tangle = DirAngle(player->x, player->y, ((fixed)nx << TILESHIFT)
	    + TILEGLOBAL/2, ((fixed)ny << TILESHIFT) + TILEGLOBAL / 2);
	dangle = CentreAngle(tangle, player->angle);
	if(dangle > -ANGLE_FOV && dangle < ANGLE_FOV)
	{
		controly = -RUNMOVE * tics;
		if(tryuse)
		{
			check = actorat[nx][ny];
			if(check && check < objlist && inc % 2)
				buttonstate[bt_use] = true;
		}
	}
	TurnToAngle(dangle);
}

void BotCommand(void)
{
	int angle = -1, distance = -2;
	objtype* check;

	check = EnemyVisible(&angle, &distance);
	if(check)
	{
		DoCombatAI(angle, distance);
	}
	else
	{
		DoNonCombatAI();
	}
}