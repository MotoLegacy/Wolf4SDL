// WL_ACT1.C

#include "wl_def.h"

/*
=============================================================================
                                                        STATICS
=============================================================================
*/


statobj_t       statobjlist[MAXSTATS];
statobj_t       *laststatobj;


typedef struct
{
    short      picnum;
    wl_stat_t  type;
    unsigned int   specialFlags;    // they are ORed to the statobj_t flags
} statinfo_t;

statinfo_t statinfo[] =
{
    {SPR_STAT_0},                           // puddle          spr1v
    {SPR_STAT_1,block},                     // Green Barrel    "
    {SPR_STAT_2,block},                     // Table/chairs    "
    {SPR_STAT_3,block,FL_FULLBRIGHT},       // Floor lamp      "
    {SPR_STAT_4,none,FL_FULLBRIGHT},        // Chandelier      "
    {SPR_STAT_5,block},                     // Hanged man      "
    {SPR_STAT_6,bo_alpo},                   // Bad food        "
    {SPR_STAT_7,block},                     // Red pillar      "
    //
    // NEW PAGE
    //
    {SPR_STAT_8,block},                     // Tree            spr2v
    {SPR_STAT_9},                           // Skeleton flat   "
    {SPR_STAT_10,block},                    // Sink            " (SOD:gibs)
    {SPR_STAT_11,block},                    // Potted plant    "
    {SPR_STAT_12,block},                    // Urn             "
    {SPR_STAT_13,block},                    // Bare table      "
    {SPR_STAT_14,none,FL_FULLBRIGHT},       // Ceiling light   "
    #ifndef SPEAR
    {SPR_STAT_15},                          // Kitchen stuff   " (SOD: {SPR_STAT_15,block},{Gibs!} Bloody Cage)
    #else
    {SPR_STAT_15,block},                    // Gibs!
    #endif
    //
    // NEW PAGE
    //
    {SPR_STAT_16,block},                    // suit of armor   spr3v
    {SPR_STAT_17,block},                    // Hanging cage    "
    {SPR_STAT_18,block},                    // SkeletoninCage  "
    {SPR_STAT_19},                          //  Pile of Bones  "
    {SPR_STAT_20,bo_key1},                  // Key 1           "
    {SPR_STAT_21,bo_key2},                  // Key 2           "
    {SPR_STAT_22,block},                    // bed            (SOD: Bloody Cage w/Skulls)
    {SPR_STAT_23},                          // Water bowl
    //
    // NEW PAGE
    //
    {SPR_STAT_24,bo_food},                  // Chicken Dinner       spr4v
    {SPR_STAT_25,bo_firstaid},              // First aid       "
    {SPR_STAT_26,bo_clip},                  // Clip            "
    {SPR_STAT_27,bo_machinegun},            // Machine gun     "
    {SPR_STAT_28,bo_chaingun},              // Gatling gun     "
    {SPR_STAT_29,bo_cross},                 // Cross           "
    {SPR_STAT_30,bo_chalice},               // Chalice         "
    {SPR_STAT_31,bo_bible},                 // Treasure Chest  "
    //
    // NEW PAGE
    //
    {SPR_STAT_32,bo_crown},                 // crown           spr5v
    {SPR_STAT_33,bo_fullheal,FL_FULLBRIGHT},// one up          "
    {SPR_STAT_34,bo_gibs},                  // Bloody Bones    "
    {SPR_STAT_35,block},                    // barrel          "
    {SPR_STAT_36,block},                    // well            "
    {SPR_STAT_37,block},                    // Empty well      "
    {SPR_STAT_38,bo_gibs},                  // Blood Puddle    "
    {SPR_STAT_39,block},                    // flag            "
    //
    // NEW PAGE
    //
    #ifndef SPEAR
    {SPR_STAT_40,block},                    // Call Apogee         (SOD: {SPR_STAT_40},Red Light)
    #else
    {SPR_STAT_40},                          // Red light
    #endif
    //
    // NEW PAGE
    //
    {SPR_STAT_41},                          // junk            "
    {SPR_STAT_42},                          // junk            "
    {SPR_STAT_43},                          // junk            "
    #ifndef SPEAR
    {SPR_STAT_44},                          // pots            " (SOD: {SPR_STAT_44,block},{Gibs!} Bloody Mass/Wooden Cross/Goat Skull)
    #else
    {SPR_STAT_44,block},                    // Gibs!
    #endif
    {SPR_STAT_45,block},                    // stove           " (SOD: {gibs} Bloody Well)
    {SPR_STAT_46,block},                    // spears          " (SOD: {gibs} Demon Statue)
    {SPR_STAT_47},                          // vines           "
    //
    // NEW PAGE
    //
    #ifdef SPEAR
    {SPR_STAT_48,block},                    // marble pillar
    {SPR_STAT_49,bo_25clip},                // bonus 25 clip
    {SPR_STAT_50,block},                    // truck
    {SPR_STAT_51,bo_spear},                 // SPEAR OF DESTINY!
    #endif

    {SPR_STAT_26,bo_clip2},                 // Clip            "
#ifndef SEGA_SATURN
#ifdef USE_DIR3DSPR
    // These are just two examples showing the new way of using dir 3d sprites.
    // You can find the allowed values in the objflag_t enum in wl_def.h.
    {SPR_STAT_47,none,FL_DIR_VERT_MID},
    {SPR_STAT_47,block,FL_DIR_HORIZ_MID},
#endif
#endif
    {-1}                                    // terminator
};

/*
===============
=
= InitStaticList
=
===============
*/

void InitStaticList (void)
{
    laststatobj = &statobjlist[0];
}



/*
===============
=
= SpawnStatic
=
===============
*/

void SpawnStatic (int tilex, int tiley, int type)
{
    laststatobj->shapenum = statinfo[type].picnum;
    laststatobj->tilex = tilex;
    laststatobj->tiley = tiley;
    laststatobj->visspot = &spotvis[tilex][tiley];
    laststatobj->itemnumber = statinfo[type].type;

    switch (statinfo[type].type)
    {
        case block:
#ifdef EMBEDDED		
            //			actorat[tilex][tiley] = 1;	// consider it a blocking tile
            set_wall_at(tilex, tiley, 1);	// consider it a blocking tile
#else		
            actorat[tilex][tiley] = (objtype*)BIT_WALL;          // consider it a blocking tile
#endif

        case none:
            laststatobj->flags = 0;
            break;

        case    bo_cross:
        case    bo_chalice:
        case    bo_bible:
        case    bo_crown:
        case    bo_fullheal:
#ifndef SEGA_SATURN
            if (!loadedgame)
#endif
                gamestate.treasuretotal++;

        case    bo_firstaid:
        case    bo_key1:
        case    bo_key2:
        case    bo_key3:
        case    bo_key4:
        case    bo_clip:
        case    bo_25clip:
        case    bo_machinegun:
        case    bo_chaingun:
        case    bo_food:
        case    bo_alpo:
        case    bo_gibs:
        case    bo_spear:
            laststatobj->flags = FL_BONUS;
            break;
    }
#ifdef PUSHOBJECT // Pushable Static Object Item
    if (MAPSPOT(tilex, tiley, 0) == PUSHITEMMARKER)
    {
        laststatobj->pushable = 1;     // Make Item Pushable or True
        ResetFloorCode(tilex, tiley);  // Reset the Floor code to a valid Floor code value
    }
#endif

    laststatobj->flags |= statinfo[type].specialFlags;

    laststatobj++;

    if (laststatobj == &statobjlist[MAXSTATS])
        Quit ("Too many static objects!\n");
}

#ifdef PUSHOBJECT // Objects that can be pushed
/* ============ Reset Floor Code ================ */
// Replace Any Map Modifier Code with a valid Floor Code Value
void ResetFloorCode(int tilex, int tiley)
{
    int x, y;
    for (x = di_north; x <= di_west; x++)
    {
        y = MAPSPOT(tilex + dx4dir[x], tiley + dy4dir[x], 0);
        if (y >= AREATILE && y <= (AREATILE + NUMAREAS - 1))
        {
            MAPSPOT(tilex, tiley, 0) = y; return;
        }
    }
}
#endif

/*
===============
=
= PlaceItemType
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
===============
*/

void PlaceItemType (int itemtype, int tilex, int tiley)
{
    int type;
    statobj_t *spot;

    //
    // find the item number
    //
    for (type=0; ; type++)
    {
        if (statinfo[type].picnum == -1)                    // end of list
            Quit ("PlaceItemType: couldn't find type!");
        if (statinfo[type].type == itemtype)
            break;
    }

    //
    // find a spot in statobjlist to put it in
    //
    for (spot=&statobjlist[0]; ; spot++)
    {
        if (spot==laststatobj)
        {
            if (spot == &statobjlist[MAXSTATS])
                return;                                     // no free spots
            laststatobj++;                                  // space at end
            break;
        }

        if (spot->shapenum == -1)                           // -1 is a free spot
            break;
    }
    //
    // place it
    //
    spot->shapenum = statinfo[type].picnum;
    spot->tilex = tilex;
    spot->tiley = tiley;
    spot->visspot = &spotvis[tilex][tiley];
    spot->flags = FL_BONUS | statinfo[type].specialFlags;
    spot->itemnumber = statinfo[type].type;
}



/*
=============================================================================
                                  DOORS
doorobjlist[] holds most of the information for the doors
doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
        this is directly accessed by AsmRefresh during rendering
The number of doors is limited to 64 because a spot in tilemap holds the
        door number in the low 6 bits, with the high bit meaning a door center
        and bit 6 meaning a door side tile
Open doors connect two areas, so sounds will travel between them and sight
        will be checked when the player is in a connected area.
Areaconnect is incremented/decremented by each door. If >0 they connect
Every time a door opens or closes the areabyplayer matrix gets recalculated.
        An area is true if it connects with the player's current spot.
=============================================================================
*/

#define DOORWIDTH       0x7800
#define OPENTICS        300

doorobj_t       doorobjlist[MAXDOORS],*lastdoorobj;
short           doornum;

#ifdef BLAKEDOORS
unsigned short      ldoorposition[MAXDOORS], rdoorposition[MAXDOORS];   // leading edge of door 0=closed
#else
unsigned short            doorposition[MAXDOORS];             // leading edge of door 0=closed
#endif

                                                    // 0xffff = fully open

unsigned char            areaconnect[NUMAREAS][NUMAREAS];

boolean         areabyplayer[NUMAREAS];



/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void RecursiveConnect (int areanumber)
{
    int i;

    for (i=0;i<NUMAREAS;i++)
    {
        if (areaconnect[areanumber][i] && !areabyplayer[i])
        {
            areabyplayer[i] = true;
            RecursiveConnect (i);
        }
    }
}


void ConnectAreas (void)
{
    memset (areabyplayer,0,sizeof(areabyplayer));
    areabyplayer[player->areanumber] = true;
    RecursiveConnect (player->areanumber);
}


void InitAreas (void)
{
    memset (areabyplayer,0,sizeof(areabyplayer));
    if (player->areanumber < NUMAREAS)
        areabyplayer[player->areanumber] = true;
}



/*
===============
=
= InitDoorList
=
===============
*/

void InitDoorList (void)
{
    memset (areabyplayer,0,sizeof(areabyplayer));
    memset (areaconnect,0,sizeof(areaconnect));

    lastdoorobj = &doorobjlist[0];
    doornum = 0;
}


/*
===============
=
= SpawnDoor
=
===============
*/
#if defined(EMBEDDED) && defined(SEGA_SATURN)
void SpawnDoor (int tilex, int tiley, boolean vertical, int lock)
{
    word *map;

    if (doornum==MAXDOORS)
        Quit ("64+ doors on level!");

    doorposition[doornum] = 0;              // doors start out fully closed
    lastdoorobj->tilex = tilex;
    lastdoorobj->tiley = tiley;
    lastdoorobj->vertical = vertical;
    lastdoorobj->lock = lock;
    lastdoorobj->action = dr_closed;

#ifdef SEGA_SATURN
    set_door_actor(tilex, tiley, doornum);	// consider it a solid wall
#else
    actorat[tilex][tiley] = (objtype *)(uintptr_t)(doornum | BIT_DOOR);   // consider it a solid wall
#endif
    //
    // make the door tile a special tile, and mark the adjacent tiles
    // for door sides
    //
    tilemap[tilex][tiley] = doornum | BIT_DOOR;
    map = &MAPSPOT(tilex,tiley,0);
    if (vertical)
    {
        *map = *(map-1);                        // set area number
        tilemap[tilex][tiley-1] |= BIT_WALL;
        tilemap[tilex][tiley+1] |= BIT_WALL;
    }
    else
    {
        *map = *(map-mapwidth);                                 // set area number
        tilemap[tilex-1][tiley] |= BIT_WALL;
        tilemap[tilex+1][tiley] |= BIT_WALL;
    }

    doornum++;
    lastdoorobj++;
}

/*
=====================
=
= CloseDoor
=
=====================
*/
void CloseDoor(int door)
{
    int     tilex, tiley, area;
    objtype* check;

    //
    // don't close on anything solid
    //
    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if (actorat[tilex][tiley])
        return;

    if (player->tilex == tilex && player->tiley == tiley)
        return;

    if (doorobjlist[door].vertical)
    {
        if (player->tiley == tiley)
        {
            if (((player->x + MINDIST) >> TILESHIFT) == tilex)
                return;
            if (((player->x - MINDIST) >> TILESHIFT) == tilex)
                return;
        }
        check = actorat[tilex - 1][tiley];
        if (ISPOINTER(check) && ((check->x + MINDIST) >> TILESHIFT) == tilex)
            return;
        check = actorat[tilex + 1][tiley];
        if (ISPOINTER(check) && ((check->x - MINDIST) >> TILESHIFT) == tilex)
            return;
    }
    else
    {
        if (player->tilex == tilex)
        {
            if (((player->y + MINDIST) >> TILESHIFT) == tiley)
                return;
            if (((player->y - MINDIST) >> TILESHIFT) == tiley)
                return;
        }
        check = actorat[tilex][tiley - 1];
        if (ISPOINTER(check) && ((check->y + MINDIST) >> TILESHIFT) == tiley)
            return;
        check = actorat[tilex][tiley + 1];
        if (ISPOINTER(check) && ((check->y - MINDIST) >> TILESHIFT) == tiley)
            return;
    }


    //
    // play door sound if in a connected area
    //
    area = MAPSPOT(tilex, tiley, 0) - AREATILE;

    if (areabyplayer[area])
    {
        PlaySoundLocTile(CLOSEDOORSND, doorobjlist[door].tilex, doorobjlist[door].tiley); // JAB
    }

    doorobjlist[door].action = dr_closing;
    //
    // make the door space solid
    //
    set_door_actor(tilex, tiley, door);
}

#else
void SpawnDoor(int tilex, int tiley, boolean vertical, int lock)
{
    unsigned short* map;

    if (doornum == MAXDOORS)
        Quit("64+ doors on level!");
#ifndef BLAKEDOORS
    doorposition[doornum] = 0;              // doors start out fully closed
#endif

#ifdef BLAKEDOORS
    switch (MAPSPOT(tilex, tiley, 0))
    {
    case 90:
    case 91:
        lastdoorobj->doubledoor = true;
        break;
    default:
        lastdoorobj->doubledoor = false;
    }
    if (lastdoorobj->doubledoor)
    {
        ldoorposition[doornum] = 0x7fff;      // doors start out fully closed
        rdoorposition[doornum] = 0x8000;
    }
    else
    {
        ldoorposition[doornum] = 0;      // doors start out fully closed
        rdoorposition[doornum] = 0; // this will function like the original doorposition[]
    }
#endif

    lastdoorobj->tilex = tilex;
    lastdoorobj->tiley = tiley;
    lastdoorobj->vertical = vertical;
    lastdoorobj->lock = lock;
    lastdoorobj->action = dr_closed;

#ifdef SEGA_SATURN
    set_door_actor(tilex, tiley, doornum);	// consider it a solid wall
#else
    actorat[tilex][tiley] = (objtype*)(uintptr_t)(doornum | BIT_DOOR);   // consider it a solid wall
#endif
    //
    // make the door tile a special tile, and mark the adjacent tiles
    // for door sides
    //
    tilemap[tilex][tiley] = doornum | BIT_DOOR;
    map = &MAPSPOT(tilex, tiley, 0);
    if (vertical)
    {
        *map = *(map - 1);                        // set area number
        tilemap[tilex][tiley - 1] |= BIT_WALL;
        tilemap[tilex][tiley + 1] |= BIT_WALL;
    }
    else
    {
        *map = *(map - mapwidth);                                 // set area number
        tilemap[tilex - 1][tiley] |= BIT_WALL;
        tilemap[tilex + 1][tiley] |= BIT_WALL;
    }

//	made this change so more than 64 walls can exist
#ifdef WIP
    if (vertical)
        *map = *(map - 1);						// set area number
    else
        *map = *(map - mapwidth);				// set area number
#endif
    doornum++;
    lastdoorobj++;
}

/*
=====================
=
= CloseDoor
=
=====================
*/

void CloseDoor(int door)
{
    int     tilex, tiley, area;
    objtype* check;

    //
    // don't close on anything solid
    //
    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if (actorat[tilex][tiley])
        return;

    if (player->tilex == tilex && player->tiley == tiley)
        return;

    if (doorobjlist[door].vertical)
    {
        if (player->tiley == tiley)
        {
            if (((player->x + MINDIST) >> TILESHIFT) == tilex)
                return;
            if (((player->x - MINDIST) >> TILESHIFT) == tilex)
                return;
        }
        check = actorat[tilex - 1][tiley];
        if (ISPOINTER(check) && ((check->x + MINDIST) >> TILESHIFT) == tilex)
            return;
        check = actorat[tilex + 1][tiley];
        if (ISPOINTER(check) && ((check->x - MINDIST) >> TILESHIFT) == tilex)
            return;
    }
    else
    {
        if (player->tilex == tilex)
        {
            if (((player->y + MINDIST) >> TILESHIFT) == tiley)
                return;
            if (((player->y - MINDIST) >> TILESHIFT) == tiley)
                return;
        }
        check = actorat[tilex][tiley - 1];
        if (ISPOINTER(check) && ((check->y + MINDIST) >> TILESHIFT) == tiley)
            return;
        check = actorat[tilex][tiley + 1];
        if (ISPOINTER(check) && ((check->y - MINDIST) >> TILESHIFT) == tiley)
            return;
    }


    //
    // play door sound if in a connected area
    //
    area = MAPSPOT(tilex, tiley, 0) - AREATILE;

    if (areabyplayer[area])
    {
        PlaySoundLocTile(CLOSEDOORSND, doorobjlist[door].tilex, doorobjlist[door].tiley); // JAB
    }

    doorobjlist[door].action = dr_closing;
    //
    // make the door space solid
    //
    actorat[tilex][tiley] = (objtype*)(uintptr_t)(door | BIT_DOOR);
}

#endif
//===========================================================================

/*
=====================
=
= OpenDoor
=
=====================
*/

void OpenDoor(int door)
{
    if (doorobjlist[door].action == dr_open)
        doorobjlist[door].ticcount = 0;         // reset open time
    else
        doorobjlist[door].action = dr_opening;  // start it opening
}

/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
=
=====================
*/

void OperateDoor (int door)
{
    int lock;

    lock = doorobjlist[door].lock;
    if (lock >= dr_lock1 && lock <= dr_lock4)
    {
        if ( ! (gamestate.keys & (1 << (lock-dr_lock1) ) ) )
        {
#ifdef BLAKEDOORS         
            if (ldoorposition[door] == 0)
                SD_PlaySound(NOWAYSND);
            if(rdoorposition[door] == 0)
                SD_PlaySound(NOWAYSND);
#else
            if(doorposition[door]==0)
                SD_PlaySound(NOWAYSND);  // ADDEDFIX 9       // locked
#endif
            return;
        }
    }

    switch (doorobjlist[door].action)
    {
        case dr_closed:
        case dr_closing:
            OpenDoor (door);
            break;
        case dr_open:
        case dr_opening:
            CloseDoor (door);
            break;
    }
}


//===========================================================================

/*
===============
=
= DoorOpen
=
= Close the door after three seconds
=
===============
*/

void DoorOpen (int door)
{
    if ( (doorobjlist[door].ticcount += (short) tics) >= OPENTICS)
        CloseDoor (door);
}



/*
===============
=
= DoorOpening
=
===============
*/

void DoorOpening (int door)
{
    unsigned area1,area2;
    unsigned short *map;
    int position;

#ifdef BLAKEDOORS
    if (doorobjlist[door].doubledoor)
        position = 0x7fff - ldoorposition[door];
    else
        position = rdoorposition[door];
#else
    position = doorposition[door];
#endif
    if (!position)
    {
        //
        // door is just starting to open, so connect the areas
        //
        map = &MAPSPOT(doorobjlist[door].tilex,doorobjlist[door].tiley,0);

        if (doorobjlist[door].vertical)
        {
            area1 = *(map+1);
            area2 = *(map-1);
        }
        else
        {
            area1 = *(map-mapwidth);
            area2 = *(map+mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]++;
            areaconnect[area2][area1]++;

            if (player->areanumber < NUMAREAS)
                ConnectAreas ();

            if (areabyplayer[area1])
                PlaySoundLocTile(OPENDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);  // JAB
        }
    }

    //
    // slide the door by an adaptive amount
    //
#ifdef BLAKEDOORS
    if (doorobjlist[door].doubledoor)
    {
        position += (tics << 10) / 2;
        if (position >= 0x7fff)
        {
            //
            // door is all the way open
            //
            position = 0x7fff;
            doorobjlist[door].ticcount = 0;
            doorobjlist[door].action = dr_open;
            actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
        }
        ldoorposition[door] = 0x7fff - position;
        rdoorposition[door] = 0x8000 + position;
    }
    else
    {
#endif
    position += tics<<10;
    if (position >= 0xffff)
    {
        //
        // door is all the way open
        //
        position = 0xffff;
        doorobjlist[door].ticcount = 0;
        doorobjlist[door].action = dr_open;
#ifdef SEGA_SATURN
        clear_actor(doorobjlist[door].tilex, doorobjlist[door].tiley);
#else
        actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
#endif
    }

#ifndef BLAKEDOORS
    doorposition[door] = (unsigned short)position;
#else
    ldoorposition[door] = 0;
    rdoorposition[door] = position;
    }
#endif
}


/*
===============
=
= DoorClosing
=
===============
*/

void DoorClosing (int door)
{
    unsigned area1,area2;
    unsigned short *map;
    int position;
    int tilex,tiley;

    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

#ifdef SEGA_SATURN
    if (obj_actor_at(tilex, tiley)
        || (player->tilex == tilex && player->tiley == tiley) )
#else
    if ( ((int)(uintptr_t)actorat[tilex][tiley] != (door | BIT_DOOR))
        || (player->tilex == tilex && player->tiley == tiley) )
#endif
    {                       // something got inside the door
        OpenDoor (door);
        return;
    };

#ifndef BLAKEDOORS
    position = doorposition[door];

    //
    // slide the door by an adaptive amount
    //
    position -= tics << 10;
#else
    if (doorobjlist[door].doubledoor)
    {
        position = 0x7fff - ldoorposition[door];
        // slide the door by an adaptive amount
        position -= (tics << 10) / 2;
    }
    else
    {
        position = rdoorposition[door];
        // slide the door by an adaptive amount
        position -= (tics << 10);
    }
#endif
    if (position <= 0)
    {
        //
        // door is closed all the way, so disconnect the areas
        //
        position = 0;

        doorobjlist[door].action = dr_closed;

        map = &MAPSPOT(tilex,tiley,0);

        if (doorobjlist[door].vertical)
        {
            area1 = *(map+1);
            area2 = *(map-1);
        }
        else
        {
            area1 = *(map-mapwidth);
            area2 = *(map+mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]--;
            areaconnect[area2][area1]--;

            if (player->areanumber < NUMAREAS)
                ConnectAreas ();
        }
    }

#ifndef BLAKEDOORS
    doorposition[door] = (unsigned short)position;
#else
    if (doorobjlist[door].doubledoor)
    {
        ldoorposition[door] = 0x7fff - position;
        rdoorposition[door] = 0x8000 + position;
    }
    else
    {
        ldoorposition[door] = 0;
        rdoorposition[door] = position;
    }
#endif
}




/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/

void MoveDoors (void)
{
    int door;

    if (gamestate.victoryflag)              // don't move door during victory sequence
        return;

    for (door = 0; door < doornum; door++)
    {
        switch (doorobjlist[door].action)
        {
            case dr_open:
                DoorOpen (door);
                break;

            case dr_opening:
                DoorOpening(door);
                break;

            case dr_closing:
                DoorClosing(door);
                break;
        }
    }
}


/*
=============================================================================
                                PUSHABLE WALLS
=============================================================================
*/

unsigned short pwallstate;
unsigned short pwallpos;                  // amount a pushable wall has been moved (0-63)
unsigned short pwallx,pwally;
unsigned char pwalldir;
tiletype pwalltile;
int dirs[4][2]={{0,-1},{1,0},{0,1},{-1,0}};

/*
===============
=
= PushWall
=
===============
*/
#if defined(EMBEDDED) && defined(SEGA_SATURN)
void PushWall(int checkx, int checky, int dir)
{
    int oldtile, dx, dy;

    if (pwallstate)
        return;

    oldtile = tilemap[checkx][checky];
    if (!oldtile)
        return;

    dx = dirs[dir][0];
    dy = dirs[dir][1];

    if (any_actor_at(checkx + dx, checky + dy))
    {
        SD_PlaySound(NOWAYSND);
        return;
    }
    actorat[checkx + dx][checky + dy] = (objtype*)(uintptr_t)(tilemap[checkx + dx][checky + dy] = oldtile);

    gamestate.secretcount++;
    pwallx = checkx;
    pwally = checky;
    pwalldir = dir;
    pwallstate = 1;
    pwallpos = 0;
    pwalltile = tilemap[pwallx][pwally];
    tilemap[pwallx][pwally] = BIT_WALL;
    tilemap[pwallx + dx][pwally + dy] = BIT_WALL;
    MAPSPOT(pwallx, pwally, 1) = 0;   // remove P tile info
    MAPSPOT(pwallx, pwally, 0) = MAPSPOT(player->tilex, player->tiley, 0); // set correct floorcode (BrotherTank's fix) TODO: use a better method...

    SD_PlaySound(PUSHWALLSND);
}

/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls(void)
{
    int oldblock, oldtile;

    if (!pwallstate)
        return;

    oldblock = pwallstate / 128;

    pwallstate += (word)tics;

    if (pwallstate / 128 != oldblock)
    {
        // block crossed into a new block
        oldtile = pwalltile;

        //
        // the tile can now be walked into
        //
        tilemap[pwallx][pwally] = 0;
        actorat[pwallx][pwally] = 0;
        MAPSPOT(pwallx, pwally, 0) = player->areanumber + AREATILE;    // TODO: this is unnecessary, and makes a mess of mapsegs

        int dx = dirs[pwalldir][0], dy = dirs[pwalldir][1];
        //
        // see if it should be pushed farther
        //
        if (pwallstate >= 256)            // only move two tiles fix
        {
            //
            // the block has been pushed two tiles
            //
            pwallstate = 0;
            tilemap[pwallx + dx][pwally + dy] = oldtile;
            return;
        }
        else
        {
            int xl, yl, xh, yh;
            xl = (player->x - PLAYERSIZE) >> TILESHIFT;
            yl = (player->y - PLAYERSIZE) >> TILESHIFT;
            xh = (player->x + PLAYERSIZE) >> TILESHIFT;
            yh = (player->y + PLAYERSIZE) >> TILESHIFT;

            pwallx += dx;
            pwally += dy;

            if (actorat[pwallx + dx][pwally + dy]
                || xl <= pwallx + dx && pwallx + dx <= xh && yl <= pwally + dy && pwally + dy <= yh)
            {
                pwallstate = 0;
                tilemap[pwallx][pwally] = oldtile;
                return;
            }
            actorat[pwallx + dx][pwally + dy] = (objtype*)(uintptr_t)(tilemap[pwallx + dx][pwally + dy] = oldtile);
            tilemap[pwallx + dx][pwally + dy] = BIT_WALL;
        }
    }

    pwallpos = (pwallstate / 2) & 63;
}
#else
void PushWall (int checkx, int checky, int dir)
{
    int oldtile, dx, dy;

    if (pwallstate)
        return;

    oldtile = tilemap[checkx][checky];
    if (!oldtile)
        return;

    dx = dirs[dir][0];
    dy = dirs[dir][1];

    if (actorat[checkx+dx][checky+dy])
    {
        SD_PlaySound (NOWAYSND);
        return;
    }
    actorat[checkx+dx][checky+dy] = (objtype *)(uintptr_t) (tilemap[checkx+dx][checky+dy] = oldtile);

    gamestate.secretcount++;
    pwallx = checkx;
    pwally = checky;
    pwalldir = dir;
    pwallstate = 1;
    pwallpos = 0;
    pwalltile = tilemap[pwallx][pwally];
    tilemap[pwallx][pwally] = BIT_WALL;
    tilemap[pwallx+dx][pwally+dy] = BIT_WALL;
    MAPSPOT(pwallx,pwally,1) = 0;   // remove P tile info
    MAPSPOT(pwallx,pwally,0) = MAPSPOT(player->tilex,player->tiley,0); // set correct floorcode (BrotherTank's fix) TODO: use a better method...

    SD_PlaySound (PUSHWALLSND);
}



/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls (void)
{
    int oldblock,oldtile;

    if (!pwallstate)
        return;

    oldblock = pwallstate/128;

    pwallstate += (unsigned short)tics;

    if (pwallstate/128 != oldblock)
    {
		int dx, dy;
        // block crossed into a new block
        oldtile = pwalltile;

        //
        // the tile can now be walked into
        //
        tilemap[pwallx][pwally] = 0;
        actorat[pwallx][pwally] = 0;
        MAPSPOT(pwallx,pwally,0) = player->areanumber+AREATILE;    // TODO: this is unnecessary, and makes a mess of mapsegs

        dx=dirs[pwalldir][0];
		dy=dirs[pwalldir][1];

        //
        // see if it should be pushed farther
        //
        if (pwallstate>=256)            // only move two tiles fix
        {
            //
            // the block has been pushed two tiles
            //
            pwallstate = 0;
            tilemap[pwallx+dx][pwally+dy] = oldtile;
            return;
        }
        else
        {
            int xl,yl,xh,yh;
            xl = (player->x-PLAYERSIZE) >> TILESHIFT;
            yl = (player->y-PLAYERSIZE) >> TILESHIFT;
            xh = (player->x+PLAYERSIZE) >> TILESHIFT;
            yh = (player->y+PLAYERSIZE) >> TILESHIFT;

            pwallx += dx;
            pwally += dy;

            if (actorat[pwallx+dx][pwally+dy]
                || xl<=pwallx+dx && pwallx+dx<=xh && yl<=pwally+dy && pwally+dy<=yh)
            {
                pwallstate = 0;
                tilemap[pwallx][pwally] = oldtile;
                return;
            }
            actorat[pwallx+dx][pwally+dy] = (objtype *)(uintptr_t) (tilemap[pwallx+dx][pwally+dy] = oldtile);
            tilemap[pwallx+dx][pwally+dy] = BIT_WALL;
        }
    }

    pwallpos = (pwallstate/2)&63;
}
#endif