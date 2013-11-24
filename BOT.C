#include "wl_def.h"
#include "bot.h"
#include "queue.h"

#define ENEMY_VISIBLE_RANGE 15

word path_prev[MAPSIZE*MAPSIZE];

objtype* EnemyVisible(int *angle, int *distance)
{
	objtype* ob;
	int tx = (int)player->tilex, ty = (int)player->tiley;
	int i, j, k;
	static objtype* oldob;
	
	int distmin = 0x7fff;
	fixed dxmin, dymin;
	objtype* obmin = NULL;
		
	for(ob = player->next; ob; ob = ob->next)
	{
		k = abs(ob->tilex - tx);
		j = abs(ob->tiley - ty);
		i = k > j ? k : j;
		
		if(i > ENEMY_VISIBLE_RANGE)
			continue;
		if(!CheckLine(ob))
			continue;
		if(!(ob->flags & FL_VISABLE))
			continue;
		if((abs(*distance - i) >= 2 && ob != oldob) || ob == oldob || (oldob && !(oldob->flags & FL_SHOOTABLE)))
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
		*angle = (short)(180.f/PI_F*atan2f((float)obmin->y - player->y, (float)obmin->x - player->x));
		*distance = distmin;
	}
	
	return obmin;
}

boolean ObjectOfInterest(word n)
{
	word tx = TILE_DECODE_X(n);
	word ty = TILE_DECODE_Y(n);
	objtype* check = actorat[tx][ty];
	statobj_t	*statptr;
	
	for(statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if(!(statptr->flags & FL_BONUS))
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
				if(!(gamestate.keys & (1<<(itemnum - bo_key1))))
					return true;
				break;
			case    bo_cross:
			case    bo_chalice:
			case    bo_bible:
			case    bo_crown:
				return true;
			case    bo_machinegun:
				if(gamestate.bestweapon < wp_machinegun || gamestate.ammo <= 80)
					return true;
				break;
			case    bo_chaingun:
				if(gamestate.bestweapon < wp_chaingun || gamestate.ammo <= 80)
					return true;
				break;
			case    bo_fullheal:
				return true;
			case    bo_clip:
				if(gamestate.ammo < 80)
					return true;
				break;
			case    bo_clip2:
				if (gamestate.ammo < 80)
					return true;
				break;
#ifdef SPEAR
			case    bo_25clip:
				if (gamestate.ammo <= 74)
					return true;
				break;
			case    bo_spear:
				if(searchstage >= SSSecretLift || moodBox() & MoodBox::MOOD_TAKEFIRSTEXIT)
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

	if(check >= objlist && (check->flags & FL_SHOOTABLE) && gamestate.health > 75 && gamestate.ammo > 50)
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
		lock = doorobjlist[door].lock;
		if(lock >= dr_lock1 && lock <= dr_lock4)
		{
			if(doorobjlist[door].action != dr_open && doorobjlist[door].action != dr_opening && !(gamestate.keys & (1 << (lock - dr_lock1))))
				return false;
		}
	}
	return true;
}

word FindShortestPath()
{
#define PROCEED(n) if(!path_prev(n) && TilePassable(n)){path_prev[n] = m; QUEUE_PUSH(n)}
	word n = TILE_ENCODE(player->tilex, player->tiley);
	word m;
	byte i;
	memset(path_prev,0,sizeof(path_prev));
	QUEUE_PUSH(n)
	path_prev[n] = WORD_MAX;
	while(!QUEUE_EMPTY)
	{
		m = QueuePop();
		if(ObjectOfInterest(m))
			return m;
		// add neighbours. don't worry about bounds checking, maps should be guarded anyway
		n = m + 1;
		PROCEED(n)
		n = m - 1;
		PROCEED(n)
		n = m + MAPSIZE;
		PROCEED(n)
		n = m - MAPSIZE;
		PROCEED(n)
	}
#undef PROCEED
}

void DoCombatAI(int angle, int distance)
{
	QUEUE_RESET
	QueuePush(
}

void BotCommand()
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