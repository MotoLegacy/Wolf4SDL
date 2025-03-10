/* WL_AGENT.C */

#include "wl_def.h"

/*
=============================================================================
                                LOCAL CONSTANTS
=============================================================================
*/

#define MAXMOUSETURN    10


#define MOVESCALE       150l
#define BACKMOVESCALE   100l
#define ANGLESCALE      20

/*
=============================================================================
                                GLOBAL VARIABLES
=============================================================================
*/



/*
** player state info
*/
int        thrustspeed;
#ifdef SWITCH
int JOYSTICK_DEAD_ZONE = 3000;
int JOYSTICK_MAX_ZONE = 30000;
HidAnalogStickState pos_left, pos_right;
#endif
unsigned short            plux, pluy;          /* player coordinates scaled to unsigned */

short           anglefrac;

objtype* LastAttacker;

/*
=============================================================================
                                                 LOCAL VARIABLES
=============================================================================
*/
#if !defined(EMBEDDED) && !defined(SEGA_SATURN)
void    T_Player(objtype* ob);
void    T_Attack(objtype* ob);

statetype   s_player = { false,0,0,(statefunc)T_Player,NULL,NULL };
statetype   s_attack = { false,0,0,(statefunc)T_Attack,NULL,NULL };
#endif

static struct atkinf
{
    char    tics, attack, frame;   /* Attack is for: 1 Gun, 2 Knife, 3 for Machine Gun, 4 Chain Gun */
} attackinfo[4][4] =
{
    { {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,3,3},{6,-1,4} },
    { {6,0,1},{6,1,2},{6,4,3},{6,-1,4} },
};

/* =========================================================================== */


boolean TryMove(objtype* ob);
void T_Player(objtype* ob);

void ClipMove(objtype* ob, int xmove, int ymove);

/*
=============================================================================
                                CONTROL STUFF
=============================================================================
*/

/*
======================
=
= CheckWeaponChange
=
= Keys 1-4 change weapons
=
======================
*/

void CheckWeaponChange(void)
{
    int i, newWeapon = -1;
#ifdef _arch_dreamcast 
    int joyx, joyy;
#endif   
    if (!gamestate.ammo)            /* must use knife with no ammo */
        return;

#ifdef _arch_dreamcast
    IN_GetJoyFineDelta(&joyx, &joyy);
    if (joyx < -64)
        buttonstate[bt_prevweapon] = true;
    else if (joyx > 64)
        buttonstate[bt_nextweapon] = true;
#endif

    if (buttonstate[bt_nextweapon] && !buttonheld[bt_nextweapon])
    {
        newWeapon = gamestate.weapon + 1;
        if (newWeapon > gamestate.bestweapon) newWeapon = 0;
    }
    else if (buttonstate[bt_prevweapon] && !buttonheld[bt_prevweapon])
    {
        newWeapon = gamestate.weapon - 1;
        if (newWeapon < 0) newWeapon = gamestate.bestweapon;
    }
    else
    {
        for (i = wp_knife; i <= gamestate.bestweapon; i++)
        {
            if (buttonstate[bt_readyknife + i - wp_knife])
            {
                newWeapon = i;
                break;
            }
        }
    }

    if (newWeapon != -1)
    {
        gamestate.weapon = gamestate.chosenweapon = (weapontype)newWeapon;
        DrawWeapon();
    }
}


/*
=======================
=
= ControlMovement
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
=======================
*/

void ControlMovement(objtype* ob)
{
    int     angle;
    int     angleunits;
    fixed speed;
#ifdef SWITCH
    float strafespeed = 0;
#endif
    thrustspeed = 0;

#ifdef SWITCH
    /* Read the joysticks' position */
    pos_left = padGetStickPos(&pad, 0);
    pos_right = padGetStickPos(&pad, 1);

    if( pos_left.x < -JOYSTICK_DEAD_ZONE)
    {
        strafespeed = floor((float)((float)-pos_left.x/(float)32767)*(float)35);
    }
    if( pos_left.x > JOYSTICK_DEAD_ZONE)
    {
        strafespeed = floor((float)((float)pos_left.x/(float)32767)*(float)35);
    }
    if( pos_left.x < -JOYSTICK_MAX_ZONE || pos_left.x > JOYSTICK_MAX_ZONE)
    {
        strafespeed = 35;
    }
#endif

    if (buttonstate[bt_strafeleft])
    {
        angle = ob->angle + ANGLES / 4;
        if (angle >= ANGLES)
            angle -= ANGLES;
        if (buttonstate[bt_run])
#ifdef SWITCH
			Thrust(angle, (strafespeed*2) * MOVESCALE * (int)tics);
#else
#ifdef CSGO_STRAFE
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics * 0.6f);
#else
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics);
#endif
#endif
        else
#ifdef SWITCH
			Thrust(angle, strafespeed * MOVESCALE * (int)tics);
#else
#ifdef CSGO_STRAFE
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics * 0.6f);
#else
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics);
#endif
#endif
    }

    if (buttonstate[bt_straferight])
    {
        angle = ob->angle - ANGLES / 4;
        if (angle < 0)
            angle += ANGLES;
        if (buttonstate[bt_run])
#ifdef SWITCH	
            Thrust(angle, (strafespeed * 2) * MOVESCALE * (int)tics);
#else		
#ifdef CSGO_STRAFE
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics * 0.6f);
#else
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics);
#endif
#endif
		else
#ifdef SWITCH
			Thrust(angle, strafespeed * MOVESCALE * (int)tics);
#else
#ifdef CSGO_STRAFE
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics * 0.6f);
#else
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics);
#endif
#endif
    }
#ifdef EXTRACONTROLS
    if (buttonstate[bt_moveforward])
    {
        angle = ob->angle - ANGLES;
        if (angle < 0)
            angle += ANGLES;

        if (controlstrafe)
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics);
        else
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics);
    }

    if (buttonstate[bt_movebackward])
    {
        angle = ob->angle - ANGLES / 2;
        if (angle < 0)
            angle += ANGLES;

        if (controlstrafe)
            Thrust(angle, RUNMOVE * MOVESCALE * (int)tics);
        else
            Thrust(angle, BASEMOVE * MOVESCALE * (int)tics);
    }
#endif

    /*
    ** side to side move
    */	
    if (buttonstate[bt_strafe] || mouselookenabled)
	{
        /*
        ** strafing
        */
        if (controlx > 0)
        {
            angle = ob->angle - ANGLES / 4;
            if (angle < 0)
                angle += ANGLES;
            Thrust(angle, controlx * MOVESCALE);      /* move to left */
        }
        else if (controlx < 0)
        {
            angle = ob->angle + ANGLES / 4;
            if (angle >= ANGLES)
                angle -= ANGLES;
            Thrust(angle, -controlx * MOVESCALE);     /* move to right */
        }
    }
    else
    {
        /*
        ** not strafing and mouselook disabled when activated
        */
        anglefrac += (short)controlx;
        angleunits = anglefrac / ANGLESCALE;
        anglefrac -= angleunits * ANGLESCALE;
        ob->angle -= angleunits;

        if (ob->angle >= ANGLES)
            ob->angle -= ANGLES;
        if (ob->angle < 0)
            ob->angle += ANGLES;

    }

#if SDL_MAJOR_VERSION == 2
    if (gamecontrolstrafe < 0)
    {
        angle = ob->angle + ANGLES / 4;
        if (angle >= ANGLES)
            angle -= ANGLES;

        speed = -gamecontrolstrafe * MOVESCALE;
        if (controly != 0)
            speed = (speed * 70) / 100; /* correct faster diagonal movement */
        Thrust(angle, (int)speed);           /* move to left */
    }
    else if (gamecontrolstrafe > 0)
    {
        angle = ob->angle - ANGLES / 4;
        if (angle < 0)
            angle += ANGLES;

        speed = (fixed)gamecontrolstrafe * MOVESCALE;
        if (controly != 0)
            speed = (speed * 70) / 100; /* correct faster diagonal movement */
        Thrust(angle, (int)speed);           /* move to right */
    }
#endif

    if (mouselookenabled) {
        /*
        ** mouselook enabled
        */
        anglefrac += mousecontrolx;
        angleunits = anglefrac/ANGLESCALE;
        anglefrac -= angleunits*ANGLESCALE;
        ob->angle -= angleunits;

        if (ob->angle >= ANGLES)
            ob->angle -= ANGLES;
        if (ob->angle < 0)
            ob->angle += ANGLES;
    }

    /*
    ** forward/backwards move
    */
    if (controly < 0)
    {
        speed = -controly * MOVESCALE;
#if SDL_MAJOR_VERSION == 2        
        if (gamecontrolstrafe != 0)
            speed = (speed * 70) / 100; /* correct faster diagonal movement */
#endif   
        Thrust(ob->angle, (int)speed);       /* move forwards */
    }
    else if (controly > 0)
    {
        speed = (fixed)controly * BACKMOVESCALE;
        angle = ob->angle + ANGLES / 2;
        if (angle >= ANGLES)
            angle -= ANGLES;
#if SDL_MAJOR_VERSION == 2        
        if (gamecontrolstrafe != 0)
#endif
            speed = (speed * 70) / 100; /* correct faster diagonal movement */
        
        Thrust(angle, (int)speed);           /* move backwards */
    }

    if (gamestate.victoryflag)              /* watching the BJ actor */
        return;
}

/*
=============================================================================
                            STATUS WINDOW STUFF
=============================================================================
*/


/*
==================
=
= StatusDrawPic
=
==================
*/

#ifdef SEGA_SATURN
void LatchDrawPicScaledCoordIndirect(unsigned scx, unsigned scy, unsigned picnum);
#endif

void StatusDrawPic(unsigned x, unsigned y, unsigned picnum)
{
#ifdef SEGA_SATURN
#if SATURN_WIDTH == 352
    LatchDrawPicScaledCoord(2 + (screenWidth - scaleFactor * SATURN_WIDTH) / 16 + scaleFactor * x,
        screenHeight - scaleFactor * (STATUSLINES - y), picnum);
#else
    LatchDrawPicScaledCoord((screenWidth - scaleFactor * SATURN_WIDTH) / 16 + scaleFactor * x,
        screenHeight - scaleFactor * (STATUSLINES - y), picnum);
#endif
#else
    VWB_DrawPicScaledCoord(((screenWidth - scaleFactor * 320) / 16 + scaleFactor * x) * 8,
        screenHeight - scaleFactor * (STATUSLINES - y), picnum);
#endif
}

#ifdef SEGA_SATURN
wlinline void StatusDrawPicIndirect(unsigned x, unsigned y, unsigned picnum)
{
#if SATURN_WIDTH == 352	
    LatchDrawPicScaledCoordIndirect(2 + (screenWidth - scaleFactor * SATURN_WIDTH) / 16 + scaleFactor * x,
        screenHeight - scaleFactor * (STATUSLINES - y), picnum);
#else	
    LatchDrawPicScaledCoordIndirect((screenWidth - scaleFactor * SATURN_WIDTH) / 16 + scaleFactor * x,
        screenHeight - scaleFactor * (STATUSLINES - y), picnum);
#endif		
}
#endif

void StatusDrawFace(unsigned int picnum)
{
    StatusDrawPic(17, 4, picnum);

#ifdef _arch_dreamcast
    StatusDrawLCD(picnum);
#endif
}


/*
==================
=
= DrawFace
=
==================
*/

void DrawFace(void)
{
    if (viewsize == 21 && ingame) return;
    if (SD_SoundPlaying() == GETGATLINGSND)
        StatusDrawFace(GOTGATLINGPIC);
    else if (gamestate.health)
    {
#ifdef SPEAR
        if (godmode)
            StatusDrawFace(GODMODEFACE1PIC + gamestate.faceframe);
        else
#endif
            StatusDrawFace(FACE1APIC + 3 * ((100 - gamestate.health) / 16) + gamestate.faceframe);
    }
    else
    {
#ifndef SPEAR
        if (LastAttacker && LastAttacker->obclass == needleobj)
            StatusDrawFace(MUTANTBJPIC);
        else
#endif
            StatusDrawFace(FACE8APIC);
    }
}

/*
===============
=
= UpdateFace
=
= Calls draw face if time to change
=
===============
*/

int facecount = 0;
int facetimes = 0;

void UpdateFace(void)
{
    /* don't make demo depend on sound playback */
    if (demoplayback || demorecord)
    {
        if (facetimes > 0)
        {
            facetimes--;
            return;
        }
    }
    else if (SD_SoundPlaying() == GETGATLINGSND)
        return;

    facecount += (int)tics;
    if (facecount > US_RndT())
    {
        gamestate.faceframe = (US_RndT() >> 6);
        if (gamestate.faceframe == 3)
            gamestate.faceframe = 1;

        facecount = 0;
        DrawFace();
    }
}

/*
===============
=
= LatchNumber
=
= right justifies and pads with blanks
=
===============
*/

static void LatchNumber(int x, int y, size_t width, int number)
{
    size_t length, c;
    char    str[20];
    wlltoa(number, str, 10);
    length = strlen(str);

    while (length < width)
    {
        StatusDrawPic(x, y, N_BLANKPIC);
        x++;
        width--;
    }

    c = length <= width ? 0 : length - width;

    while (c < length)
    {
        StatusDrawPic(x, y, str[c] - '0' + N_0PIC);
        x++;
        c++;
    }
}


/*
===============
=
= DrawHealth
=
===============
*/

void DrawHealth(void)
{
    if (viewsize == 21 && ingame) return;
    LatchNumber(21, 16, 3, gamestate.health);
}


/*
===============
=
= TakeDamage
=
===============
*/

void TakeDamage(int points, objtype* attacker)
{
    LastAttacker = attacker;

    if (gamestate.victoryflag)
        return;
    if (gamestate.difficulty == gd_baby)
        points >>= 2;

    if (!godmode)
        gamestate.health -= points;

    if (gamestate.health <= 0) 
    {
#ifdef MAPCONTROLLEDLTIME
        if (tilemap[63][4]) {
            gamestate.health = 0;
            playstate = ex_completed;
        }
        else 
        {
#endif
            gamestate.health = 0;
            playstate = ex_died;
            killerobj = attacker;
#ifdef MAPCONTROLLEDLTIME
        }
#endif
    }

    if (godmode != 2)
        StartDamageFlash(points);

    DrawHealth();
    DrawFace();

    /*
    ** MAKE BJ'S EYES BUG IF MAJOR DAMAGE!
    */
#ifdef SPEAR
    if (points > 30 && gamestate.health != 0 && !godmode && viewsize != 21)
    {
        StatusDrawFace(BJOUCHPIC);
        facecount = 0;
    }
#endif
}

/*
===============
=
= HealSelf
=
===============
*/

void HealSelf(int points)
{
    gamestate.health += points;
    if (gamestate.health > 100)
        gamestate.health = 100;

    DrawHealth();
    DrawFace();
}


/* =========================================================================== */


/*
===============
=
= DrawLevel
=
===============
*/

void DrawLevel(void)
{
    if (viewsize == 21 && ingame) return;
#ifdef SPEAR
    if (gamestate.mapon == 20)
        LatchNumber(2, 16, 2, 18);
    else
#endif
        LatchNumber(2, 16, 2, gamestate.mapon + 1);
}

/* =========================================================================== */


/*
===============
=
= DrawLives
=
===============
*/

void DrawLives(void)
{
    if (viewsize == 21 && ingame) return;
    LatchNumber(14, 16, 1, gamestate.lives);
}


/*
===============
=
= GiveExtraMan
=
===============
*/

void GiveExtraMan(void)
{
    if (gamestate.lives < 9)
        gamestate.lives++;
    DrawLives();
    SD_PlaySound(BONUS1UPSND);
}

/* =========================================================================== */

/*
===============
=
= DrawScore
=
===============
*/

void DrawScore(void)
{
    if (viewsize == 21 && ingame) return;
    LatchNumber(6, 16, 6, gamestate.score);
}

/*
===============
=
= GivePoints
=
===============
*/

void GivePoints(int points)
{
    gamestate.score += points;
    while (gamestate.score >= gamestate.nextextra)
    {
        gamestate.nextextra += EXTRAPOINTS;
        GiveExtraMan();
    }
    DrawScore();
}

/* =========================================================================== */

/*
==================
=
= DrawWeapon
=
==================
*/

void DrawWeapon(void)
{
    if (viewsize == 21 && ingame) return;
#ifdef SEGA_SATURN
    if (gamestate.weapon == -1) return; /* vbt ajout */
    StatusDrawPicIndirect(32, 8, KNIFEPIC + gamestate.weapon);
#else
    StatusDrawPic(32, 8, KNIFEPIC + gamestate.weapon);
#endif
}


/*
==================
=
= DrawKeys
=
==================
*/

void DrawKeys(void)
{
    if (viewsize == 21 && ingame) return;
    if (gamestate.keys & 1)
        StatusDrawPic(30, 4, GOLDKEYPIC);
    else
        StatusDrawPic(30, 4, NOKEYPIC);

    if (gamestate.keys & 2)
        StatusDrawPic(30, 20, SILVERKEYPIC);
    else
        StatusDrawPic(30, 20, NOKEYPIC);
}

/*
==================
=
= GiveWeapon
=
==================
*/

void GiveWeapon(int weapon)
{
    GiveAmmo(6);

    if (gamestate.bestweapon < weapon)
        gamestate.bestweapon = gamestate.weapon
        = gamestate.chosenweapon = (weapontype)weapon;

    DrawWeapon();
}

/* =========================================================================== */

/*
===============
=
= DrawAmmo
=
===============
*/

void DrawAmmo(void)
{
    if (viewsize == 21 && ingame) return;
    LatchNumber(27, 16, 2, gamestate.ammo);
}

/*
===============
=
= GiveAmmo
=
===============
*/

void GiveAmmo(int ammo)
{
    if (!gamestate.ammo)                            /* knife was out */
    {
        if (!gamestate.attackframe)
        {
            gamestate.weapon = gamestate.chosenweapon;
            DrawWeapon();
        }
    }
    gamestate.ammo += ammo;
    if (gamestate.ammo > 99)
        gamestate.ammo = 99;
    DrawAmmo();
}

/* =========================================================================== */

/*
==================
=
= GiveKey
=
==================
*/

void GiveKey(int key)
{
    gamestate.keys |= (1 << key);
    DrawKeys();
}



/*
=============================================================================
                                MOVEMENT
=============================================================================
*/


/*
===================
=
= GetBonus
=
===================
*/
void GetBonus(statobj_t* check)
{
    if (playstate == ex_died)   /* ADDEDFIX 31 - Chris */
        return;

    switch (check->itemnumber)
    {
    case    bo_firstaid:
        if (gamestate.health == 100)
            return;

        SD_PlaySound(HEALTH2SND);
        HealSelf(25);
        break;

    case    bo_key1:
    case    bo_key2:
    case    bo_key3:
    case    bo_key4:
        GiveKey(check->itemnumber - bo_key1);
        SD_PlaySound(GETKEYSND);
        break;

    case    bo_cross:
        SD_PlaySound(BONUS1SND);
        GivePoints(100);
        gamestate.treasurecount++;
        break;
    case    bo_chalice:
        SD_PlaySound(BONUS2SND);
        GivePoints(500);
        gamestate.treasurecount++;
        break;
    case    bo_bible:
        SD_PlaySound(BONUS3SND);
        GivePoints(1000);
        gamestate.treasurecount++;
        break;
    case    bo_crown:
        SD_PlaySound(BONUS4SND);
        GivePoints(5000);
        gamestate.treasurecount++;
        break;

    case    bo_clip:
        if (gamestate.ammo == 99)
            return;

        SD_PlaySound(GETAMMOSND);
        GiveAmmo(8);
        break;
    case    bo_clip2:
        if (gamestate.ammo == 99)
            return;

        SD_PlaySound(GETAMMOSND);
        GiveAmmo(4);
        break;

#ifdef SPEAR
    case    bo_25clip:
        if (gamestate.ammo == 99)
            return;

        SD_PlaySound(GETAMMOBOXSND);
        GiveAmmo(25);
        break;
#endif

    case    bo_machinegun:
        SD_PlaySound(GETMACHINESND);
        GiveWeapon(wp_machinegun);
        break;
    case    bo_chaingun:
        SD_PlaySound(GETGATLINGSND);
        facetimes = 38;
        GiveWeapon(wp_chaingun);

        if (viewsize != 21)
            StatusDrawFace(GOTGATLINGPIC);
        facecount = 0;
        break;

    case    bo_fullheal:
        SD_PlaySound(BONUS1UPSND);
        HealSelf(99);
        GiveAmmo(25);
        GiveExtraMan();
        gamestate.treasurecount++;
        break;

    case    bo_food:
        if (gamestate.health == 100)
            return;

        SD_PlaySound(HEALTH1SND);
        HealSelf(10);
        break;

    case    bo_alpo:
        if (gamestate.health == 100)
            return;

        SD_PlaySound(HEALTH1SND);
        HealSelf(4);
        break;

    case    bo_gibs:
        if (gamestate.health > 10)
            return;

        SD_PlaySound(SLURPIESND);
        HealSelf(1);
        break;

#ifdef SPEAR
    case    bo_spear:
        spearflag = true;
        spearx = player->x;
        speary = player->y;
        spearangle = player->angle;
        playstate = ex_completed;
#endif
    }

    StartBonusFlash();
    check->shapenum = -1;                   /* remove from list */
}

/*
===================
=
= TryMove
=
= returns true if move ok
= debug: use pointers to optimize
===================
*/

boolean TryMove(objtype* ob)
{
    int         xl, yl, xh, yh, x, y;
    objtype* check;
    int     deltax, deltay;

    xl = (ob->x - PLAYERSIZE) >> TILESHIFT;
    yl = (ob->y - PLAYERSIZE) >> TILESHIFT;

    xh = (ob->x + PLAYERSIZE) >> TILESHIFT;
    yh = (ob->y + PLAYERSIZE) >> TILESHIFT;

#define PUSHWALLMINDIST PLAYERSIZE

    /*
    ** check for solid walls
    */

#if defined(EMBEDDED) && defined(SEGA_SATURN)
    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
        {
            if (solid_actor_at(x, y))
                return false;
        }
#else
    for (y = yl; y <= yh; y++)
    {
        for (x = xl; x <= xh; x++)
        {
            check = actorat[x][y];
            if (check && !ISPOINTER(check))
            {
                if (tilemap[x][y] == BIT_WALL && x == pwallx && y == pwally)   /* back of moving pushwall? */
                {
                    switch (pwalldir)
                    {
                    case di_north:
                        if (ob->y - PUSHWALLMINDIST <= (pwally << TILESHIFT) + ((63 - pwallpos) << 10))
                            return false;
                        break;
                    case di_west:
                        if (ob->x - PUSHWALLMINDIST <= (pwallx << TILESHIFT) + ((63 - pwallpos) << 10))
                            return false;
                        break;
                    case di_east:
                        if (ob->x + PUSHWALLMINDIST >= (pwallx << TILESHIFT) + (pwallpos << 10))
                            return false;
                        break;
                    case di_south:
                        if (ob->y + PUSHWALLMINDIST >= (pwally << TILESHIFT) + (pwallpos << 10))
                            return false;
                        break;
                    }
                }
                else return false;
            }
        }
    }
#endif
    /*
    ** check for actors
    */
    if (yl > 0)
        yl--;
    if (yh < MAPSIZE - 1)
        yh++;
    if (xl > 0)
        xl--;
    if (xh < MAPSIZE - 1)
        xh++;

    for (y = yl; y <= yh; y++)
    {
        for (x = xl; x <= xh; x++)
        {
#if defined(EMBEDDED) && defined(SEGA_SATURN)

            check = &objlist[get_actor_at(x, y)];
            if (check->flags & FL_SHOOTABLE)
#else
            check = actorat[x][y];
#endif 
            if (ISPOINTER(check) && check != player && (check->flags & FL_SHOOTABLE))
            {
                deltax = ob->x - check->x;
                if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
                    continue;
                deltay = ob->y - check->y;
                if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
                    continue;

                return false;
            }
        }
    }

    return true;
}


/*
===================
=
= ClipMove
=
===================
*/

void ClipMove(objtype* ob, int xmove, int ymove)
{
    int    basex, basey;

    basex = ob->x;
    basey = ob->y;

    ob->x = basex + xmove;
    ob->y = basey + ymove;
    if (TryMove(ob))
        return;

#if !defined(REMDEBUG) || !defined(SEGA_SATURN)
    if (noclip && ob->x > 2 * TILEGLOBAL && ob->y > 2 * TILEGLOBAL
        && ob->x < (((int)(mapwidth - 1)) << TILESHIFT)
        && ob->y < (((int)(mapheight - 1)) << TILESHIFT))
        return;         /* walk through walls */
#endif

    if (!SD_SoundPlaying())
        SD_PlaySound(HITWALLSND);

    ob->x = basex + xmove;
    ob->y = basey;
    if (TryMove(ob))
        return;

    ob->x = basex;
    ob->y = basey + ymove;
    if (TryMove(ob))
        return;

    ob->x = basex;
    ob->y = basey;
}

/* ========================================================================== */

/*
===================
=
= VictoryTile
=
===================
*/

void VictoryTile(void)
{
#ifndef SPEAR
    SpawnBJVictory();
#endif

    gamestate.victoryflag = true;
}

/*
===================
=
= Thrust
=
===================
*/

/* For player movement in demos exactly as in the original Wolf3D v1.4 source code */
static fixed FixedByFracOrig(fixed a, fixed b)
{
    int sign = 0;
    fixed res;

    if (b == 65536) b = 65535;
    else if (b == -65536) b = 65535, sign = 1;
    else if (b < 0) b = (-b), sign = 1;

    if (a < 0)
    {
        a = -a;
        sign = !sign;
    }

    res = (fixed)(((int64_t)a * b) >> 16);
/*   res = (fixed)fixedpt_mul(a, b) >> 16;      */
	if (sign)
        res = -res;
    return res;
}

void Thrust(int angle, int speed)
{
    int xmove, ymove;

    /*
    ** ZERO FUNNY COUNTER IF MOVED!
    */
#ifdef SPEAR
    if (speed)
        funnyticount = 0;
#endif

    thrustspeed += (int)speed;
    /*
    ** moving bounds speed
    */
    if (speed >= MINDIST * 2)
        speed = MINDIST * 2 - 1;

#ifdef SEGA_SATURN
    xmove = FixedMul(speed, costable[angle]);
    ymove = -FixedMul(speed, sintable[angle]);
#else
    xmove = (int)DEMOCHOOSE_ORIG_SDL(
        FixedByFracOrig(speed, costable[angle]),
        FixedMul(speed, costable[angle]));
    ymove = (int)DEMOCHOOSE_ORIG_SDL(
        -FixedByFracOrig(speed, sintable[angle]),
        -FixedMul(speed, sintable[angle]));
#endif
    ClipMove(player, xmove, ymove);

    player->tilex = (player->x >> TILESHIFT);                /* scale to tile values */
    player->tiley = (player->y >> TILESHIFT);

    player->areanumber = MAPSPOT(player->tilex, player->tiley, 0) - AREATILE;

    if (MAPSPOT(player->tilex, player->tiley, 1) == EXITTILE)
        VictoryTile();
}


/*
=============================================================================
                                ACTIONS
=============================================================================
*/


/*
===============
=
= Cmd_Fire
=
===============
*/

void Cmd_Fire(void)
{
    buttonheld[bt_attack] = true;

    gamestate.weaponframe = 0;

#ifdef EMBEDDED
    player->state = s_attack;
#else
    player->state = &s_attack;
#endif

    gamestate.attackframe = 0;
    gamestate.attackcount =
        attackinfo[gamestate.weapon][gamestate.attackframe].tics;
    gamestate.weaponframe =
        attackinfo[gamestate.weapon][gamestate.attackframe].frame;

}

/* =========================================================================== */

/*
===============
=
= Cmd_Use
=
===============
*/

void Cmd_Use(void)
{
    int     checkx, checky, doornum, dir;
    boolean elevatorok;
#if defined(PUSHOBJECT) || defined(LOGFILE) 
    statobj_t* statptr;
#endif
    /*
    ** find which cardinal direction the player is facing
    */
    if (player->angle < ANGLES / 8 || player->angle > 7 * ANGLES / 8)
    {
        checkx = player->tilex + 1;
        checky = player->tiley;
        dir = di_east;
        elevatorok = true;
    }
    else if (player->angle < 3 * ANGLES / 8)
    {
        checkx = player->tilex;
        checky = player->tiley - 1;
        dir = di_north;
        elevatorok = false;
    }
    else if (player->angle < 5 * ANGLES / 8)
    {
        checkx = player->tilex - 1;
        checky = player->tiley;
        dir = di_west;
        elevatorok = true;
    }
    else
    {
        checkx = player->tilex;
        checky = player->tiley + 1;
        dir = di_south;
        elevatorok = false;
    }
#ifdef LOGFILE
    /* Added by Havoc for interactive objects */
    for (statptr = &statobjlist[0]; statptr != laststatobj; statptr++)
    {
        if (statptr->tilex == checkx && statptr->tiley == checky &&
            (statptr->shapenum == SPR_STAT_4 || statptr->shapenum == SPR_STAT_9 || statptr->shapenum == SPR_STAT_19) &&
            !buttonheld[bt_use])
        {
            buttonheld[bt_use] = true;

            ClearMemory();

            VL_FadeOut(0, 255, 0, 0, 0, 30);

            switch (statptr->shapenum)
            {
            case SPR_STAT_4:
                LogDiscScreens("1");
                break;

            case SPR_STAT_9:
                LogDiscScreens("2");
                break;

            case SPR_STAT_19:
                LogDiscScreens("3");
                break;
            }

            ClearMemory();

            IN_ClearKeysDown();

            DrawPlayScreen();
        }
    }
#endif
    doornum = tilemap[checkx][checky];
#if defined(EMBEDDED) && defined(SEGA_SATURN)
    if (*(mapsegs[1] + farmapylookup[checky] + checkx) == PUSHABLETILE)
#else
    if (MAPSPOT(checkx, checky, 1) == PUSHABLETILE)
#endif
    {
        /*
        ** pushable wall
        */

        PushWall(checkx, checky, dir);
        return;
    }
    if (!buttonheld[bt_use] && doornum == ELEVATORTILE && elevatorok)
    {
        /*
        ** use elevator
        */
        buttonheld[bt_use] = true;

        tilemap[checkx][checky]++;              /* flip switch */
#if defined(SEGA_SATURN) && defined(EMBEDDED)
        if (*(mapsegs[0] + farmapylookup[player->tiley] + player->tilex) == ALTELEVATORTILE)
#else
        if (MAPSPOT(player->tilex, player->tiley, 0) == ALTELEVATORTILE)
#endif
            playstate = ex_secretlevel;
        else
            playstate = ex_completed;
        SD_PlaySound(LEVELDONESND);
        SD_WaitSoundDone();
    }
    else if (!buttonheld[bt_use] && doornum & BIT_DOOR)
    {
        buttonheld[bt_use] = true;
        OperateDoor(doornum & ~BIT_DOOR);
    }
    else
        SD_PlaySound(DONOTHINGSND);
#ifdef PUSHOBJECT
    /* Static Object/Items manipulation routines */
    for (statptr = statobjlist; statptr != laststatobj; statptr++)
    {
        if (statptr->tilex == checkx && statptr->tiley == checky && !buttonheld[bt_use])
        {
            buttonheld[bt_use] = true;        /* that must react to spacebar */

#ifdef PUSHOBJECT /* Pushable Items or Objects that can be pushed */

            if (statptr->pushable) /* Is the item Pushable */
            {
                if (actorat[checkx][checky])
                {
                    if (actorat[checkx + dx4dir[dir]][checky + dy4dir[dir]] > 0) { return; } /* Is Tile to move to Free? */
                    SD_PlaySound(TAKEDAMAGESND);                                 /* Make a Ugh pushing sound */
                    statptr->tilex = statptr->tilex + dx4dir[dir];                /* Free to move object */
                    statptr->tiley = statptr->tiley + dy4dir[dir];                /* to it's new location */
                    statptr->visspot = &spotvis[statptr->tilex][statptr->tiley];  /* Make it visible */
                    actorat[checkx][checky] = (objtype*)(uintptr_t)0;
                    actorat[statptr->tilex][statptr->tiley] = (objtype*)(uintptr_t)64;
                }
            }
#endif
            /* Other routines for static objects can be inserted here */

            /* End of static object check loop */
        }
    }
#endif
#ifdef PUSHOBJECT /* Pushable Static Object Item */
    if (MAPSPOT(tilex, tiley, 0) == PUSHITEMMARKER)
    {
        laststatobj->pushable = 1;     /* Make Item Pushable or True */
        ResetFloorCode(tilex, tiley);  /* Reset the Floor code to a valid Floor code value */
    }
#endif
}

/*
=============================================================================
                                PLAYER CONTROL
=============================================================================
*/



/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer(int tilex, int tiley, int dir)
{
    player->obclass = playerobj;
    player->active = ac_yes;
    player->tilex = (unsigned short)tilex;
    player->tiley = (unsigned short)tiley;
#if defined(EMBEDDED) && defined(SEGA_SATURN)
    player->areanumber = *(mapsegs[0] + farmapylookup[player->tiley] + player->tilex);
#else
    player->areanumber = MAPSPOT(tilex, tiley, 0) - AREATILE;
#endif
    player->x = (tilex << TILESHIFT) + TILEGLOBAL / 2;
    player->y = (tiley << TILESHIFT) + TILEGLOBAL / 2;
#if defined(EMBEDDED) && defined(SEGA_SATURN)
    player->state = s_player;
#else
    player->state = &s_player;
#endif
    player->angle = (1 - dir) * 90;
    if (player->angle < 0)
        player->angle += ANGLES;
    player->flags = FL_NEVERMARK;
    Thrust(0, 0);                           /* set some variables */

    InitAreas();
}


/* =========================================================================== */

/*
===============
=
= T_KnifeAttack
=
= Update player hands, and try to do damage when the proper frame is reached
=
===============
*/

void    KnifeAttack(objtype* ob)
{
    objtype* check, * closest;
    fixed  dist;

    SD_PlaySound(ATKKNIFESND);
    /* actually fire */
    dist = 0x7fffffff;
    closest = NULL;
    for (check = ob->next; check; check = check->next)
    {
        if ((check->flags & FL_SHOOTABLE) && (check->flags & FL_VISIBLE)
            && abs((int)check->viewx - centerx) < shootdelta)
        {
            if (check->transx < dist)
            {
                dist = check->transx;
                closest = check;
            }
        }
    }

    if (!closest || dist > 0x18000l)
    {
        /* missed */
        return;
    }

    /* hit something */
    DamageActor(closest, US_RndT() >> 4);
}

#ifdef BULLET_CALC
#define COLSIZE 0x4000l
#define STEPDIST 0x800l

void    GunAttack(objtype* ob)
{
    short           damage, accuracy, numshots;

    objtype* check;
    int i;
    switch (gamestate.weapon)
    {
    case wp_pistol:
        SD_PlaySound(ATKPISTOLSND);
        break;
    case wp_machinegun:
        SD_PlaySound(ATKMACHINEGUNSND);
        break;
    case wp_chaingun:
        SD_PlaySound(ATKGATLINGSND);
        break;
    }

    madenoise = true;



    /*
    ** AlumiuN's new weaponry code - trace bullets
    */
    switch (gamestate.weapon)
    {
    case wp_pistol: accuracy = 4; numshots = 1; break;
    case wp_machinegun: accuracy = 5; numshots = 1; break;
    case wp_chaingun: accuracy = 9; numshots = 1; break;
    }

    for (i = 0; i < numshots; i++)
    {
        int bulletx = player->x;
        int bullety = player->y;
        short bangle = player->angle;
		boolean         hit;
        
		if (accuracy)
        {
            if (US_RndT() > 127)
                bangle -= US_RndT() % (accuracy + 1);
            else
                bangle += US_RndT() % (accuracy + 1);
        }

        hit = false;    /* Assume the worst! Oh, and reset the variable. :) */

        while (1)
        {
            bulletx += FixedMul(STEPDIST, costable[bangle]);
            bullety -= FixedMul(STEPDIST, sintable[bangle]);

            /*
            ** check for solid walls
            */
            check = actorat[bulletx >> TILESHIFT][bullety >> TILESHIFT];
            if (check && !ISPOINTER(check) && (uintptr_t)check != 64)
            {
                if ((uintptr_t)check < 128)      /* Hit a wall */
                {
                    /* Add any effects that you want to happen when the bullet hits a wall */
                }
                else if (doorobjlist[(uintptr_t)check - 128].action < 0xdfff / 2)   /* Cheap hack - improve later? */
                {
                    /* Add any effects that you want to happen when the bullet hits a door */
                }
                hit = true;
            }

            if (hit)
                break;

            check = objlist;
            while (check)
            {
                if (check->flags & FL_SHOOTABLE)
                {
                    if (labs(bulletx - check->x) < COLSIZE && labs(bullety - check->y) < COLSIZE)
                    {
                        hit = true;
                        switch (gamestate.weapon)
                        {
                        case wp_pistol: damage = 10 + US_RndT() % 25; break;
                        case wp_machinegun: damage = 8 + US_RndT() % 22; break;
                        case wp_chaingun: damage = 6 + US_RndT() % 20; break;
                        }
                        DamageActor(check, damage);
                    }
                }
                check = check->next;
            }

            if (hit)
                break;
        }
    }
}
#else
void    GunAttack(objtype* ob)
{
    objtype* check, * closest;
    int      damage;
    int      dx, dy, dist;
    fixed  viewdist;

    switch (gamestate.weapon)
    {
    case wp_pistol:
        SD_PlaySound(ATKPISTOLSND);
        break;
    case wp_machinegun:
        SD_PlaySound(ATKMACHINEGUNSND);
        break;
    case wp_chaingun:
        SD_PlaySound(ATKGATLINGSND);
        break;
    default:
        break;   
    }

    madenoise = true;

    /*
    ** find potential targets
    */
    viewdist = 0x7fffffffl;
    closest = NULL;

    while (1)
    {
        objtype *oldclosest = closest;

        for (check = ob->next; check; check = check->next)
        {
            if ((check->flags & FL_SHOOTABLE) && (check->flags & FL_VISIBLE)
                && abs((int)check->viewx - centerx) < shootdelta)
            {
                if (check->transx < viewdist)
                {
                    viewdist = check->transx;
                    closest = check;
                }
            }
        }

        if (closest == oldclosest)
            return;                                         /* no more targets, all missed */

        /*
        ** trace a line from player to enemy
        */
        if (CheckLine(closest))
            break;
    }

    /*
    ** hit something
    */
    dx = abs((int)closest->tilex - player->tilex);
    dy = abs((int)closest->tiley - player->tiley);
    dist = dx > dy ? dx : dy;
    if (dist < 2)
        damage = US_RndT() / 4;
    else if (dist < 4)
        damage = US_RndT() / 6;
    else
    {
        if ((US_RndT() / 12) < dist)           /* missed */
            return;
        damage = US_RndT() / 6;
    }
    DamageActor(closest, damage);
}
#endif
/* =========================================================================== */

/*
===============
=
= VictorySpin
=
===============
*/

void VictorySpin(void)
{
    int    desty;

    if (player->angle > 270)
    {
        player->angle -= (short)(tics * 3);
        if (player->angle < 270)
            player->angle = 270;
    }
    else if (player->angle < 270)
    {
        player->angle += (short)(tics * 3);
        if (player->angle > 270)
            player->angle = 270;
    }

    desty = (((int)player->tiley - 5) << TILESHIFT) - 0x3000;

    if (player->y > desty)
    {
        player->y -= (int)tics * 4096;
        if (player->y < desty)
            player->y = desty;
    }
}


/* =========================================================================== */

/*
===============
=
= T_Attack
=
===============
*/

void    T_Attack(objtype* ob)
{
    struct  atkinf* cur;

    UpdateFace();

    if (gamestate.victoryflag)              /* watching the BJ actor */
    {
        VictorySpin();
        return;
    }

    if (buttonstate[bt_use] && !buttonheld[bt_use])
        buttonstate[bt_use] = false;

    if (buttonstate[bt_attack] && !buttonheld[bt_attack])
        buttonstate[bt_attack] = false;

    ControlMovement(ob);
    if (gamestate.victoryflag)              /* watching the BJ actor */
        return;

    plux = (unsigned short)(player->x >> UNSIGNEDSHIFT);                     /* scale to fit in unsigned */
    pluy = (unsigned short)(player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                /* scale to tile values */
    player->tiley = (short)(player->y >> TILESHIFT);

    /*
    ** change frame and fire
    */
    gamestate.attackcount -= (short)tics;
    while (gamestate.attackcount <= 0)
    {
        cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
        switch (cur->attack)
        {
        case -1:
#if defined(EMBEDDED) && defined(SEGA_SATURN)
            ob->state = s_player;
#else
            ob->state = &s_player;
#endif		
            if (!gamestate.ammo)
            {
                gamestate.weapon = wp_knife;
                DrawWeapon();
            }
            else
            {
                if (gamestate.weapon != gamestate.chosenweapon)
                {
                    gamestate.weapon = gamestate.chosenweapon;
                    DrawWeapon();
                }
            }
            gamestate.attackframe = gamestate.weaponframe = 0;
            return;

        case 4:
            if (!gamestate.ammo)
                break;
            if (buttonstate[bt_attack])
                gamestate.attackframe -= 2;
            /* fall through */               
        case 1:
            if (!gamestate.ammo)
            {       /* can only happen with chain gun */
                gamestate.attackframe++;
                break;
            }
            GunAttack(ob);
#ifndef SEGA_SATURN
            if (!ammocheat)
#endif
                gamestate.ammo--;
            DrawAmmo();
            break;

        case 2:
            KnifeAttack(ob);
            break;

        case 3:
            if (gamestate.ammo && buttonstate[bt_attack])
                gamestate.attackframe -= 2;
            break;
        }

        gamestate.attackcount += cur->tics;
        gamestate.attackframe++;
        gamestate.weaponframe =
            attackinfo[gamestate.weapon][gamestate.attackframe].frame;
    }
}



/* =========================================================================== */

/*
===============
=
= T_Player
=
===============
*/

void    T_Player(objtype* ob)
{
    if (gamestate.victoryflag)              /* watching the BJ actor */
    {
        VictorySpin();
        return;
    }

    UpdateFace();
#ifndef SEGA_SATURN
    CheckWeaponChange();
#endif
    if (buttonstate[bt_use])
        Cmd_Use();

    if (buttonstate[bt_attack] && !buttonheld[bt_attack])
        Cmd_Fire();

    ControlMovement(ob);
    if (gamestate.victoryflag)              /* watching the BJ actor */
        return;

    plux = (unsigned short)(player->x >> UNSIGNEDSHIFT);                     /* scale to fit in unsigned */
    pluy = (unsigned short)(player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                /* scale to tile values */
    player->tiley = (short)(player->y >> TILESHIFT);
}
/* WL_AGENT_C */
