/* Egoboo - game.c
*/

/*
This file is part of Egoboo.

Egoboo is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Egoboo is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
*/


//#define MENU_DEMO   // Uncomment this to build just the menu demo
#define DECLARE_GLOBALS
#include "egoboo.h"
#include "Ui.h"
#include "Font.h"
#include "Clock.h"
#include "Log.h"
#include "Client.h"
#include "Server.h"
#include "System.h"
#include "id_md2.h"
#include "Menu.h"
#include "egoboo_strutil.h"

#include <SDL_endian.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define INITGUID
#define NAME "Boo"
#define TITLE "Boo"

#define RELEASE(x) if (x) {x->Release(); x=NULL;}

#define PROFILE_DECLARE(XX) ClockState * clkstate_##XX = NULL; double clkcount_##XX = 0.0; double clktime_##XX = 0.0;
#define PROFILE_INIT(XX)    { clkstate_##XX  = clock_create_state(); clock_init(clkstate_##XX); }
#define PROFILE_FREE(XX)    { clock_free_state(clkstate_##XX); clkstate_##XX = NULL; }
#define PROFILE_QUERY(XX)   ( (double)clktime_##XX / (double)clkcount_##XX )

#define PROFILE_BEGIN(XX) clock_frameStep(clkstate_##XX); {
#define PROFILE_END(XX)   clock_frameStep(clkstate_##XX);  clkcount_##XX = clkcount_##XX*0.9 + 0.1*1.0; clktime_##XX = clktime_##XX*0.9 + 0.1*clock_getFrameDuration(clkstate_##XX); }

ClockState * g_clk_state = NULL;
vect3        lightspekdir = {0, 0, 0};
vect3        lightspekcol = {1, 1, 1};
vect3        lightambicol = {1, 1, 1};

PROFILE_DECLARE( resize_characters );
PROFILE_DECLARE( keep_weapons_with_holders );
PROFILE_DECLARE( let_ai_think );
PROFILE_DECLARE( do_weather_spawn );
PROFILE_DECLARE( do_enchant_spawn );
PROFILE_DECLARE( cl_unbufferLatches );
PROFILE_DECLARE( sv_unbufferLatches );
PROFILE_DECLARE( check_respawn );
PROFILE_DECLARE( move_characters );
PROFILE_DECLARE( move_particles );
PROFILE_DECLARE( make_character_matrices );
PROFILE_DECLARE( attach_particles );
PROFILE_DECLARE( make_onwhichfan );
PROFILE_DECLARE( do_bumping );
PROFILE_DECLARE( stat_return );
PROFILE_DECLARE( pit_kill );
PROFILE_DECLARE( animate_tiles );
PROFILE_DECLARE( move_water );
PROFILE_DECLARE( figure_out_what_to_draw );
PROFILE_DECLARE( draw_main );

static void update_looped_sounds();

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
ACTION what_action( char cTmp )
{
  // ZZ> This function changes a letter into an action code
  ACTION action = ACTION_DA;

  switch ( toupper( cTmp ) )
  {
    case 'U':  action = ACTION_UA; break;
    case 'T':  action = ACTION_TA; break;
    case 'S':  action = ACTION_SA; break;
    case 'C':  action = ACTION_CA; break;
    case 'B':  action = ACTION_BA; break;
    case 'L':  action = ACTION_LA; break;
    case 'X':  action = ACTION_XA; break;
    case 'F':  action = ACTION_FA; break;
    case 'P':  action = ACTION_PA; break;
    case 'Z':  action = ACTION_ZA; break;
  };

  return action;
}

//------------------------------------------------------------------------------
bool_t memory_cleanUp()
{
  //ZF> This function releases all loaded things in memory and cleans up everything properly
  bool_t result = bfalse;

  close_session();       //Turn off networking
  if ( moduleActive ) release_module(); //Remove memory loaded by a module
  if ( mixeron ) Mix_CloseAudio();    //Close audio systems
  ui_shutdown();       //Shut down support systems
  net_shutDown();
  clock_shutdown( g_clk_state );
  sys_shutdown();
  SDL_Quit();        //Quit SDL

  result = btrue;
  return result;  //Todo: Need to do a check here if the functions above worked
}

//------------------------------------------------------------------------------
//Random Things-----------------------------------------------------------------
//------------------------------------------------------------------------------
void make_newloadname( char *modname, char *appendname, char *newloadname )
{
  // ZZ> This function takes some names and puts 'em together
  int cnt, tnc;
  char ctmp;

  cnt = 0;
  ctmp = modname[cnt];
  while ( ctmp != 0 )
  {
    newloadname[cnt] = ctmp;
    cnt++;
    ctmp = modname[cnt];
  }
  tnc = 0;
  ctmp = appendname[tnc];
  while ( ctmp != 0 )
  {
    newloadname[cnt] = ctmp;
    cnt++;
    tnc++;
    ctmp = appendname[tnc];
  }
  newloadname[cnt] = 0;
}

//--------------------------------------------------------------------------------------------
void load_global_waves( char *modname )
{
  // ZZ> This function loads the global waves
  STRING tmploadname, newloadname;
  int cnt;

  if ( CData.soundvalid )
  {
    // load in the sounds local to this module
    snprintf( tmploadname, sizeof( tmploadname ), "%s%s/", modname, CData.gamedat_dir );
    for ( cnt = 0; cnt < MAXWAVE; cnt++ )
    {
      snprintf( newloadname, sizeof( newloadname ), "%ssound%d.wav", tmploadname, cnt );
      globalwave[cnt] = Mix_LoadWAV( newloadname );
    };

    //These sounds are always standard, but DO NOT override sounds that were loaded local to this module
    if ( NULL == globalwave[GSOUND_COINGET] )
    {
      snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s/%s", CData.basicdat_dir, CData.globalparticles_dir, CData.coinget_sound );
      globalwave[GSOUND_COINGET] = Mix_LoadWAV( CStringTmp1 );
    };

    if ( NULL == globalwave[GSOUND_DEFEND] )
    {
      snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s/%s", CData.basicdat_dir, CData.globalparticles_dir, CData.defend_sound );
      globalwave[GSOUND_DEFEND] = Mix_LoadWAV( CStringTmp1 );
    }

    if ( NULL == globalwave[GSOUND_COINFALL] )
    {
      snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s/%s", CData.basicdat_dir, CData.globalparticles_dir, CData.coinfall_sound );
      globalwave[GSOUND_COINFALL] = Mix_LoadWAV( CStringTmp1 );
    };
  }

  /*  The Global Sounds
  * 0 - Pick up coin
  * 1 - Defend clank
  * 2 - Weather Effect
  * 3 - Hit Water tile (Splash)
  * 4 - Coin falls on ground

  //These new values todo should determine sound and particle effects (examples below)
  Weather Type: DROPS, RAIN, SNOW, LAVABUBBLE (Which weather effect to spawn)
  Water Type: LAVA, WATER, DARK (To determine sound and particle effects)

  //We shold also add standard particles that can be used everywhere (Located and
  //loaded in globalparticles folder) such as these below.
  Particle Effect: REDBLOOD, SMOKE, HEALCLOUD
  */
}


//---------------------------------------------------------------------------------------------
void export_one_character( CHR_REF character, Uint16 owner, int number )
{
  // ZZ> This function exports a character
  int tnc, profile;
  char fromdir[128];
  char todir[128];
  char fromfile[128];
  char tofile[128];
  char todirname[16];
  char todirfullname[64];

  // Don't export enchants
  disenchant_character( character );

  profile = chrmodel[character];
  if (( capcancarrytonextmodule[profile] || !capisitem[profile] ) && exportvalid )
  {
    STRING tmpname;
    // TWINK_BO.OBJ
    snprintf( todirname, sizeof( todirname ), "%s/", CData.players_dir );

    convert_spaces( todirname, sizeof( tmpname ), chrname[owner] );
    strncat( todirname, ".obj", sizeof( tmpname ) );

    // Is it a character or an item?
    if ( owner != character )
    {
      // Item is a subdirectory of the owner directory...
      snprintf( todirfullname, sizeof( todirfullname ), "%s/%d.obj", todirname, number );
    }
    else
    {
      // Character directory
      strncpy( todirfullname, todirname, sizeof( todirfullname ) );
    }


    // players/twink.obj or players/twink.obj/sword.obj
    snprintf( todir, sizeof( todir ), "%s/%s", CData.players_dir, todirfullname );

    // modules/advent.mod/objects/advent.obj
    strncpy( fromdir, madname[profile], sizeof( fromdir ) );

    // Delete all the old items
    if ( owner == character )
    {
      tnc = 0;
      while ( tnc < 8 )
      {
        snprintf( tofile, sizeof( tofile ), "%s/%d.obj", todir, tnc );   /*.OBJ*/
        fs_removeDirectoryAndContents( tofile );
        tnc++;
      }
    }


    // Make the directory
    fs_createDirectory( todir );


    // Build the DATA.TXT file
    snprintf( tofile, sizeof( tofile ), "%s/%s", todir, CData.data_file );    /*DATA.TXT*/
    export_one_character_profile( tofile, character );


    // Build the SKIN.TXT file
    snprintf( tofile, sizeof( tofile ), "%s/%s", todir, CData.skin_file );    /*SKIN.TXT*/
    export_one_character_skin( tofile, character );


    // Build the NAMING.TXT file
    snprintf( tofile, sizeof( tofile ), "%s/%s", todir, CData.naming_file );    /*NAMING.TXT*/
    export_one_character_name( tofile, character );


    // Copy all of the misc. data files
    snprintf( fromfile, sizeof( fromfile ), "%s/%s", fromdir, CData.message_file );   /*MESSAGE.TXT*/
    snprintf( tofile, sizeof( tofile ), "%s/%s", todir, CData.message_file );   /*MESSAGE.TXT*/
    fs_copyFile( fromfile, tofile );

    snprintf( fromfile, sizeof( fromfile ), "%s/tris.md2", fromdir );    /*TRIS.MD2*/
    snprintf( tofile,   sizeof( tofile ), "%s/tris.md2", todir );    /*TRIS.MD2*/
    fs_copyFile( fromfile, tofile );

    snprintf( fromfile, sizeof( fromfile ), "%s/%s", fromdir, CData.copy_file );    /*COPY.TXT*/
    snprintf( tofile,   sizeof( tofile ), "%s/%s", todir, CData.copy_file );    /*COPY.TXT*/
    fs_copyFile( fromfile, tofile );

    snprintf( fromfile, sizeof( fromfile ), "%s/%s", fromdir, CData.script_file );
    snprintf( tofile,   sizeof( tofile ), "%s/%s", todir, CData.script_file );
    fs_copyFile( fromfile, tofile );

    snprintf( fromfile, sizeof( fromfile ), "%s/%s", fromdir, CData.enchant_file );
    snprintf( tofile,   sizeof( tofile ), "%s/%s", todir, CData.enchant_file );
    fs_copyFile( fromfile, tofile );

    snprintf( fromfile, sizeof( fromfile ), "%s/%s", fromdir, CData.credits_file );
    snprintf( tofile,   sizeof( tofile ), "%s/%s", todir, CData.credits_file );
    fs_copyFile( fromfile, tofile );


    // Copy all of the particle files
    tnc = 0;
    while ( tnc < PRTPIP_PEROBJECT_COUNT )
    {
      snprintf( fromfile, sizeof( fromfile ), "%s/part%d.txt", fromdir, tnc );
      snprintf( tofile,   sizeof( tofile ), "%s/part%d.txt", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      tnc++;
    }


    // Copy all of the sound files

    for ( tnc = 0; tnc < MAXWAVE; tnc++ )
    {
      snprintf( fromfile, sizeof( fromfile ), "%s/sound%d.wav", fromdir, tnc );
      snprintf( tofile,   sizeof( tofile ), "%s/sound%d.wav", todir,   tnc );
      fs_copyFile( fromfile, tofile );
    }


    // Copy all of the image files
    tnc = 0;
    while ( tnc < 4 )
    {
      snprintf( fromfile, sizeof( fromfile ), "%s/tris%d.bmp", fromdir, tnc );
      snprintf( tofile,   sizeof( tofile ), "%s/tris%d.bmp", todir,   tnc );
      fs_copyFile( fromfile, tofile );

      snprintf( fromfile, sizeof( fromfile ), "%s/icon%d.bmp", fromdir, tnc );
      snprintf( tofile,   sizeof( tofile ), "%s/icon%d.bmp", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      tnc++;
    }
  }
}

//---------------------------------------------------------------------------------------------
void export_all_local_players( void )
{
  // ZZ> This function saves all the local players in the
  //     PLAYERS directory
  int number;
  CHR_REF character, item;
  PLA_REF ipla;

  // Check each player
  if ( !exportvalid ) return;

  for ( ipla = 0; ipla < MAXPLAYER; ipla++ )
  {
    if ( !VALID_PLA( ipla ) || INBITS_NONE == pladevice[ipla] ) continue;

    // Is it alive?
    character = pla_get_character( ipla );
    if ( !VALID_CHR( character ) || !chralive[character] ) continue;

    // Export the character
    export_one_character( character, character, 0 );

    // Export all held items
    for ( _slot = SLOT_BEGIN; _slot < SLOT_COUNT; _slot = ( SLOT )( _slot + 1 ) )
    {
      item = chr_get_holdingwhich( character, _slot );
      if ( VALID_CHR( item ) && chrisitem[item] )
      {
        SLOT loc_slot = (_slot == SLOT_SADDLE ? _slot : SLOT_LEFT);
        export_one_character( item, character, loc_slot );
      };
    }

    // Export the inventory
    number = 3;
    item  = chr_get_nextinpack( character );
    while ( VALID_CHR( item ) )
    {
      if ( chrisitem[item] ) export_one_character( item, character, number );
      item  = chr_get_nextinpack( item );
      number++;
    }

  }

}

//---------------------------------------------------------------------------------------------
void quit_module( void )
{
  // ZZ> This function forces a return to the menu
  moduleActive = bfalse;
  hostactive = bfalse;
  export_all_local_players();
  gamepaused = bfalse;
  if ( CData.soundvalid ) Mix_FadeOutChannel( -1, 500 );    //Stop all sounds that are playing
}

//--------------------------------------------------------------------------------------------
void quit_game( void )
{
  // ZZ> This function exits the game entirely

  log_info( "Exiting Egoboo %s the good way...\n", VERSION );

  if ( gameActive )
  {
    gameActive = bfalse;
  }

  if ( moduleActive )
  {
    quit_module();
  }

  free_mesh_memory();
}

//--------------------------------------------------------------------------------------------
void fgoto_colon( FILE* fileread )
{
  // ZZ> This function moves a file read pointer to the next colon
  //    char cTmp;
  Uint32 ch = fgetc( fileread );

  //    fscanf(fileread, "%c", &cTmp);
  while ( ch != ':' )
  {
    if ( ch == EOF )
    {
      // not enough colons in file!
      log_error( "There are not enough colons in file! (%s)\n", globalname );
    }

    ch = fgetc( fileread );
  }
}

//--------------------------------------------------------------------------------------------
bool_t fgoto_colon_yesno( FILE* fileread )
{
  // ZZ> This function moves a file read pointer to the next colon, or it returns
  //     bfalse if there are no more
  char cTmp;
  bool_t bfound = bfalse;

  while ( !feof( fileread ) )
  {
    cTmp = fgetc( fileread );
    if ( ':' == cTmp )
    {
      bfound = btrue;
      break;
    };
  }

  return bfound;
}



//--------------------------------------------------------------------------------------------
//Tag Reading---------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void reset_tags()
{
  //This function resets the tags
  numscantag = 0;
}

//--------------------------------------------------------------------------------------------
int read_tag( FILE *fileread )
{
  // ZZ> This function finds the next tag, returning btrue if it found one
  if ( numscantag < MAXTAG )
  {
    if ( fget_next_string( fileread, tagname[numscantag], sizeof( tagname[numscantag] ) ) )
    {
      tagvalue[numscantag] = fget_int( fileread );
      numscantag++;
      return btrue;
    }
  }
  return bfalse;
}

//--------------------------------------------------------------------------------------------
void read_all_tags( char *szFilename )
{
  // ZZ> This function reads the scancode.txt file
  FILE* fileread;

  reset_tags();
  fileread = fs_fileOpen( PRI_WARN, NULL, szFilename, "r" );
  if ( NULL == fileread )
  {
    log_warning("Could not read input codes (%s)\n", szFilename);
    return;
  };

  while ( read_tag( fileread ) );
  fs_fileClose( fileread );
}

//--------------------------------------------------------------------------------------------
int tag_value( char *string )
{
  // ZZ> This function matches the string with its tag, and returns the value...
  //     It will return 255 if there are no matches.
  int cnt;

  cnt = 0;
  while ( cnt < numscantag )
  {
    if ( strcmp( string, tagname[cnt] ) == 0 )
    {
      // They match
      return tagvalue[cnt];
    }
    cnt++;
  }
  // No matches
  return 255;
}

//--------------------------------------------------------------------------------------------
void read_controls( char *szFilename )
{
  // ZZ> This function reads the controls.txt file
  FILE* fileread;
  char currenttag[TAGSIZE];
  int tnc;
  INPUT_TYPE   input;
  CONTROL_LIST control;

  log_info( "read_controls() - reading control settings from the %s file.\n", szFilename );

  fileread = fs_fileOpen( PRI_WARN, NULL, szFilename, "r" );
  if ( NULL != fileread )
  {
    input = INPUT_KEYB;
    for ( control = KEY_FIRST, tnc = 0; control < KEY_LAST; control = ( CONTROL_LIST )( control + 1 ), tnc++ )
    {
      fget_next_string( fileread, currenttag, sizeof( currenttag ) );
      controlvalue[input][tnc] = tag_value( currenttag );
      controliskey[input][tnc] = ( currenttag[0] == 'K' );
    }

    input = INPUT_MOUS;
    for ( control = MOS_FIRST, tnc = 0; control < MOS_LAST; control = ( CONTROL_LIST )( control + 1 ), tnc++ )
    {
      fget_next_string( fileread, currenttag, sizeof( currenttag ) );
      controlvalue[input][tnc] = tag_value( currenttag );
      controliskey[input][tnc] = ( currenttag[0] == 'K' );
    }

    input = INPUT_JOYA;
    for ( control = JOA_FIRST, tnc = 0; control < JOA_LAST; control = ( CONTROL_LIST )( control + 1 ), tnc++ )
    {
      fget_next_string( fileread, currenttag, sizeof( currenttag ) );
      controlvalue[input][tnc] = tag_value( currenttag );
      controliskey[input][tnc] = ( currenttag[0] == 'K' );
    }

    input = INPUT_JOYB;
    for ( control = JOB_FIRST, tnc = 0; control < JOB_LAST; control = ( CONTROL_LIST )( control + 1 ), tnc++ )
    {
      fget_next_string( fileread, currenttag, sizeof( currenttag ) );
      controlvalue[input][tnc] = tag_value( currenttag );
      controliskey[input][tnc] = ( currenttag[0] == 'K' );
    }

    fs_fileClose( fileread );
  }
  else log_warning( "Could not load input settings (%s)\n", szFilename );
}

//--------------------------------------------------------------------------------------------
bool_t key_is_pressed( int keycode )
{
  // ZZ> This function returns btrue if the given control is pressed...
  if ( netmessagemode )  return bfalse;

  if ( sdlkeybuffer )
    return SDLKEYDOWN( keycode );
  else
    return bfalse;
}

//--------------------------------------------------------------------------------------------
bool_t control_key_is_pressed( CONTROL control )
{
  // ZZ> This function returns btrue if the given control is pressed...

  if ( controliskey[INPUT_KEYB][control] )
    return key_is_pressed( controlvalue[INPUT_KEYB][control] );
  else
    return bfalse;
}

//--------------------------------------------------------------------------------------------
bool_t control_mouse_is_pressed( CONTROL control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  bool_t retval = bfalse;

  if ( controliskey[INPUT_MOUS][control] )
  {
    retval = key_is_pressed( controlvalue[INPUT_MOUS][control] );
  }
  else
  {
    retval = ( msb == controlvalue[INPUT_MOUS][control] );
  }

  return retval;
}

//--------------------------------------------------------------------------------------------
bool_t control_joya_is_pressed( CONTROL control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  bool_t retval = bfalse;

  if ( controliskey[INPUT_JOYA][control] )
  {
    retval = key_is_pressed( controlvalue[INPUT_JOYA][control] );
  }
  else
  {
    retval = ( jab == controlvalue[INPUT_JOYA][control] );
  }

  return retval;
}

//--------------------------------------------------------------------------------------------
bool_t control_joyb_is_pressed( CONTROL control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  bool_t retval = bfalse;

  if ( controliskey[INPUT_JOYB][control] )
  {
    retval = key_is_pressed( controlvalue[INPUT_JOYB][control] );
  }
  else
  {
    retval = ( jbb == controlvalue[INPUT_JOYB][control] );
  }

  return retval;
}



//--------------------------------------------------------------------------------------------
int get_free_message( void )
{
  // This function finds the best message to use
  int cnt, tnc = msgstart, found = msgstart, mintime = 0;

  for ( cnt = 0; cnt < CData.maxmessage; cnt++ )
  {
    if ( 0 == msgtime[tnc] )
    {
      found = tnc;
      break;
    }

    if ( msgtime[tnc] > 0 && ( mintime == 0 || mintime > msgtime[tnc] ) )
    {
      found = tnc;
      mintime = msgtime[tnc];
    }

    tnc = ( tnc + 1 ) % CData.maxmessage;
  };

  msgstart = ( found + 1 ) % CData.maxmessage;

  return found;
}

//--------------------------------------------------------------------------------------------
void display_message( int message, CHR_REF character )
{
  // ZZ> This function sticks a message in the display queue and sets its timer
  int slot, read, write, cnt;
  char *eread;
  STRING szTmp;
  char cTmp, lTmp;

  CHR_REF target = chr_get_aitarget( character );
  CHR_REF owner = chr_get_aiowner( character );
  if ( message < msgtotal )
  {
    slot = get_free_message();
    msgtime[slot] = DELAY_MESSAGE;
    // Copy the message
    read = msgindex[message];
    cnt = 0;
    write = 0;
    cTmp = msgtext[read];  read++;
    while ( cTmp != 0 )
    {
      if ( cTmp == '%' )
      {
        // Escape sequence
        eread = szTmp;
        szTmp[0] = 0;
        cTmp = msgtext[read];  read++;
        if ( cTmp == 'n' ) // Name
        {
          if ( chrnameknown[character] )
            strncpy( szTmp, chrname[character], sizeof( STRING ) );
          else
          {
            lTmp = capclassname[chrmodel[character]][0];
            if ( lTmp == 'A' || lTmp == 'E' || lTmp == 'I' || lTmp == 'O' || lTmp == 'U' )
              snprintf( szTmp, sizeof( szTmp ), "an %s", capclassname[chrmodel[character]] );
            else
              snprintf( szTmp, sizeof( szTmp ), "a %s", capclassname[chrmodel[character]] );
          }
          if ( cnt == 0 && szTmp[0] == 'a' )  szTmp[0] = 'A';
        }
        if ( cTmp == 'c' ) // Class name
        {
          eread = capclassname[chrmodel[character]];
        }
        if ( cTmp == 't' ) // Target name
        {
          if ( chrnameknown[target] )
            strncpy( szTmp, chrname[target], sizeof( STRING ) );
          else
          {
            lTmp = capclassname[chrmodel[target]][0];
            if ( lTmp == 'A' || lTmp == 'E' || lTmp == 'I' || lTmp == 'O' || lTmp == 'U' )
              snprintf( szTmp, sizeof( szTmp ), "an %s", capclassname[chrmodel[target]] );
            else
              snprintf( szTmp, sizeof( szTmp ), "a %s", capclassname[chrmodel[target]] );
          }
          if ( cnt == 0 && szTmp[0] == 'a' )  szTmp[0] = 'A';
        }
        if ( cTmp == 'o' ) // Owner name
        {
          if ( chrnameknown[owner] )
            strncpy( szTmp, chrname[owner], sizeof( STRING ) );
          else
          {
            lTmp = capclassname[chrmodel[owner]][0];
            if ( lTmp == 'A' || lTmp == 'E' || lTmp == 'I' || lTmp == 'O' || lTmp == 'U' )
              snprintf( szTmp, sizeof( szTmp ), "an %s", capclassname[chrmodel[owner]] );
            else
              snprintf( szTmp, sizeof( szTmp ), "a %s", capclassname[chrmodel[owner]] );
          }
          if ( cnt == 0 && szTmp[0] == 'a' )  szTmp[0] = 'A';
        }
        if ( cTmp == 's' ) // Target class name
        {
          eread = capclassname[chrmodel[target]];
        }
        if ( cTmp >= '0' && cTmp <= '0' + ( MAXSKIN - 1 ) )  // Target's skin name
        {
          eread = capskinname[chrmodel[target]][cTmp-'0'];
        }
        if ( cTmp == 'd' ) // tmpdistance value
        {
          snprintf( szTmp, sizeof( szTmp ), "%d", scr_globals.tmpdistance );
        }
        if ( cTmp == 'x' ) // tmpx value
        {
          snprintf( szTmp, sizeof( szTmp ), "%d", scr_globals.tmpx );
        }
        if ( cTmp == 'y' ) // tmpy value
        {
          snprintf( szTmp, sizeof( szTmp ), "%d", scr_globals.tmpy );
        }
        if ( cTmp == 'D' ) // tmpdistance value
        {
          snprintf( szTmp, sizeof( szTmp ), "%2d", scr_globals.tmpdistance );
        }
        if ( cTmp == 'X' ) // tmpx value
        {
          snprintf( szTmp, sizeof( szTmp ), "%2d", scr_globals.tmpx );
        }
        if ( cTmp == 'Y' ) // tmpy value
        {
          snprintf( szTmp, sizeof( szTmp ), "%2d", scr_globals.tmpy );
        }
        if ( cTmp == 'a' ) // Character's ammo
        {
          if ( chrammoknown[character] )
            snprintf( szTmp, sizeof( szTmp ), "%d", chrammo[character] );
          else
            snprintf( szTmp, sizeof( szTmp ), "?" );
        }
        if ( cTmp == 'k' ) // Kurse state
        {
          if ( chriskursed[character] )
            snprintf( szTmp, sizeof( szTmp ), "kursed" );
          else
            snprintf( szTmp, sizeof( szTmp ), "unkursed" );
        }
        if ( cTmp == 'p' ) // Character's possessive
        {
          if ( chrgender[character] == GEN_FEMALE )
          {
            snprintf( szTmp, sizeof( szTmp ), "her" );
          }
          else
          {
            if ( chrgender[character] == GEN_MALE )
            {
              snprintf( szTmp, sizeof( szTmp ), "his" );
            }
            else
            {
              snprintf( szTmp, sizeof( szTmp ), "its" );
            }
          }
        }
        if ( cTmp == 'm' ) // Character's gender
        {
          if ( chrgender[character] == GEN_FEMALE )
          {
            snprintf( szTmp, sizeof( szTmp ), "female " );
          }
          else
          {
            if ( chrgender[character] == GEN_MALE )
            {
              snprintf( szTmp, sizeof( szTmp ), "male " );
            }
            else
            {
              snprintf( szTmp, sizeof( szTmp ), " " );
            }
          }
        }
        if ( cTmp == 'g' ) // Target's possessive
        {
          if ( chrgender[target] == GEN_FEMALE )
          {
            snprintf( szTmp, sizeof( szTmp ), "her" );
          }
          else
          {
            if ( chrgender[target] == GEN_MALE )
            {
              snprintf( szTmp, sizeof( szTmp ), "his" );
            }
            else
            {
              snprintf( szTmp, sizeof( szTmp ), "its" );
            }
          }
        }
        cTmp = *eread;  eread++;
        while ( cTmp != 0 && write < MESSAGESIZE - 1 )
        {
          msgtextdisplay[slot][write] = cTmp;
          cTmp = *eread;  eread++;
          write++;
        }
      }
      else
      {
        // Copy the letter
        if ( write < MESSAGESIZE - 1 )
        {
          msgtextdisplay[slot][write] = cTmp;
          write++;
        }
      }
      cTmp = msgtext[read];  read++;
      cnt++;
    }
    msgtextdisplay[slot][write] = 0;
  }
}

//--------------------------------------------------------------------------------------------
void remove_enchant( Uint16 enchantindex )
{
  // ZZ> This function removes a specific enchantment and adds it to the unused list
  CHR_REF character, overlay;
  Uint16 eve;
  Uint16 lastenchant, currentenchant;
  int add, cnt;

  if ( enchantindex >= MAXENCHANT || !encon[enchantindex] ) return;

  // grab the profile
  eve = enceve[enchantindex];

  // Unsparkle the spellbook
  character = encspawner[enchantindex];
  if ( VALID_CHR( character ) )
  {
    chrsparkle[character] = NOSPARKLE;
    // Make the spawner unable to undo the enchantment
    if ( chrundoenchant[character] == enchantindex )
    {
      chrundoenchant[character] = MAXENCHANT;
    }
  }


  // Play the end sound
  character = enctarget[enchantindex];
  if ( INVALID_SOUND != eveendsound[eve] )
  {
    play_sound( 1.0f, chrpos_old[character], capwavelist[eve][eveendsound[eve]], 0, character, eveendsound[eve] );
  };



  // Unset enchant values, doing morph last (opposite order to spawn_enchant)
  unset_enchant_value( enchantindex, SETCHANNEL );
  for ( cnt = SETCOSTFOREACHMISSILE; cnt >= SETCOSTFOREACHMISSILE; cnt-- )
  {
    unset_enchant_value( enchantindex, cnt );
  }
  unset_enchant_value( enchantindex, SETMORPH );


  // Remove all of the cumulative values
  add = 0;
  while ( add < EVE_ADD_COUNT )
  {
    remove_enchant_value( enchantindex, add );
    add++;
  }


  // Unlink it
  if ( chrfirstenchant[character] == enchantindex )
  {
    // It was the first in the list
    chrfirstenchant[character] = encnextenchant[enchantindex];
  }
  else
  {
    // Search until we find it
    lastenchant    = MAXENCHANT;
    currentenchant = chrfirstenchant[character];
    while ( currentenchant != enchantindex )
    {
      lastenchant = currentenchant;
      currentenchant = encnextenchant[currentenchant];
    }

    // Relink the last enchantment
    encnextenchant[lastenchant] = encnextenchant[enchantindex];
  }



  // See if we spit out an end message
  if ( eveendmessage[eve] >= 0 )
  {
    display_message( madmsgstart[eve] + eveendmessage[eve], enctarget[enchantindex] );
  }
  // Check to see if we spawn a poof
  if ( evepoofonend[eve] )
  {
    spawn_poof( enctarget[enchantindex], eve );
  }
  // Check to see if the character dies
  if ( evekillonend[eve] )
  {
    if ( chrinvictus[character] )  teammorale[chrbaseteam[character]]++;
    chrinvictus[character] = bfalse;
    kill_character( character, MAXCHR );
  }
  // Kill overlay too...
  overlay = encoverlay[enchantindex];
  if ( overlay < MAXCHR )
  {
    if ( chrinvictus[overlay] )  teammorale[chrbaseteam[overlay]]++;
    chrinvictus[overlay] = bfalse;
    kill_character( overlay, MAXCHR );
  }





  // Now get rid of it
  encon[enchantindex] = bfalse;
  freeenchant[numfreeenchant] = enchantindex;
  numfreeenchant++;

  // Now fix dem weapons
  for ( _slot = SLOT_LEFT; _slot <= SLOT_RIGHT; _slot = ( SLOT )( _slot + 1 ) )
  {
    reset_character_alpha( chr_get_holdingwhich( character, _slot ) );
  }

  // And remove see kurse enchantment
  if(evecanseekurse[enchantindex] && !capcanseekurse[character]) 
  {
    chrcanseekurse[character] = bfalse;
  }


}

//--------------------------------------------------------------------------------------------
Uint16 enchant_value_filled( Uint16 enchantindex, Uint8 valueindex )
{
  // ZZ> This function returns MAXENCHANT if the enchantment's target has no conflicting
  //     set values in its other enchantments.  Otherwise it returns the enchantindex
  //     of the conflicting enchantment
  CHR_REF character;
  Uint16 currenchant;

  character = enctarget[enchantindex];
  currenchant = chrfirstenchant[character];
  while ( currenchant != MAXENCHANT )
  {
    if ( encsetyesno[currenchant][valueindex] )
    {
      return currenchant;
    }
    currenchant = encnextenchant[currenchant];
  }
  return MAXENCHANT;
}

//--------------------------------------------------------------------------------------------
void set_enchant_value( Uint16 enchantindex, Uint8 valueindex,
                        Uint16 enchanttype )
{
  // ZZ> This function sets and saves one of the character's stats
  Uint16 conflict, character;


  encsetyesno[enchantindex][valueindex] = bfalse;
  if ( evesetyesno[enchanttype][valueindex] )
  {
    conflict = enchant_value_filled( enchantindex, valueindex );
    if ( conflict == MAXENCHANT || eveoverride[enchanttype] )
    {
      // Check for multiple enchantments
      if ( conflict < MAXENCHANT )
      {
        // Multiple enchantments aren't allowed for sets
        if ( everemoveoverridden[enchanttype] )
        {
          // Kill the old enchantment
          remove_enchant( conflict );
        }
        else
        {
          // Just unset the old enchantment's value
          unset_enchant_value( conflict, valueindex );
        }
      }
      // Set the value, and save the character's real stat
      character = enctarget[enchantindex];
      encsetyesno[enchantindex][valueindex] = btrue;
      switch ( valueindex )
      {
        case SETDAMAGETYPE:
          encsetsave[enchantindex][valueindex] = chrdamagetargettype[character];
          chrdamagetargettype[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETNUMBEROFJUMPS:
          encsetsave[enchantindex][valueindex] = chrjumpnumberreset[character];
          chrjumpnumberreset[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETLIFEBARCOLOR:
          encsetsave[enchantindex][valueindex] = chrlifecolor[character];
          chrlifecolor[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETMANABARCOLOR:
          encsetsave[enchantindex][valueindex] = chrmanacolor[character];
          chrmanacolor[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETSLASHMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_SLASH];
          chrdamagemodifier_fp8[character][DAMAGE_SLASH] = evesetvalue[enchanttype][valueindex];
          break;

        case SETCRUSHMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_CRUSH];
          chrdamagemodifier_fp8[character][DAMAGE_CRUSH] = evesetvalue[enchanttype][valueindex];
          break;

        case SETPOKEMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_POKE];
          chrdamagemodifier_fp8[character][DAMAGE_POKE] = evesetvalue[enchanttype][valueindex];
          break;

        case SETHOLYMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_HOLY];
          chrdamagemodifier_fp8[character][DAMAGE_HOLY] = evesetvalue[enchanttype][valueindex];
          break;

        case SETEVILMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_EVIL];
          chrdamagemodifier_fp8[character][DAMAGE_EVIL] = evesetvalue[enchanttype][valueindex];
          break;

        case SETFIREMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_FIRE];
          chrdamagemodifier_fp8[character][DAMAGE_FIRE] = evesetvalue[enchanttype][valueindex];
          break;

        case SETICEMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_ICE];
          chrdamagemodifier_fp8[character][DAMAGE_ICE] = evesetvalue[enchanttype][valueindex];
          break;

        case SETZAPMODIFIER:
          encsetsave[enchantindex][valueindex] = chrdamagemodifier_fp8[character][DAMAGE_ZAP];
          chrdamagemodifier_fp8[character][DAMAGE_ZAP] = evesetvalue[enchanttype][valueindex];
          break;

        case SETFLASHINGAND:
          encsetsave[enchantindex][valueindex] = chrflashand[character];
          chrflashand[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETLIGHTBLEND:
          encsetsave[enchantindex][valueindex] = chrlight_fp8[character];
          chrlight_fp8[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETALPHABLEND:
          encsetsave[enchantindex][valueindex] = chralpha_fp8[character];
          chralpha_fp8[character] = evesetvalue[enchanttype][valueindex];
          chrbumpstrength[character] = capbumpstrength[chrmodel[character]] * FP8_TO_FLOAT( chralpha_fp8[character] );
          break;

        case SETSHEEN:
          encsetsave[enchantindex][valueindex] = chrsheen_fp8[character];
          chrsheen_fp8[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETFLYTOHEIGHT:
          encsetsave[enchantindex][valueindex] = chrflyheight[character];
          if ( chrflyheight[character] == 0 && chrpos[character].z > -2 )
          {
            chrflyheight[character] = evesetvalue[enchanttype][valueindex];
          }
          break;

        case SETWALKONWATER:
          encsetsave[enchantindex][valueindex] = chrwaterwalk[character];
          if ( !chrwaterwalk[character] )
          {
            chrwaterwalk[character] = evesetvalue[enchanttype][valueindex];
          }
          break;

        case SETCANSEEINVISIBLE:
          encsetsave[enchantindex][valueindex] = chrcanseeinvisible[character];
          chrcanseeinvisible[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETMISSILETREATMENT:
          encsetsave[enchantindex][valueindex] = chrmissiletreatment[character];
          chrmissiletreatment[character] = evesetvalue[enchanttype][valueindex];
          break;

        case SETCOSTFOREACHMISSILE:
          encsetsave[enchantindex][valueindex] = chrmissilecost[character];
          chrmissilecost[character] = evesetvalue[enchanttype][valueindex];
          chrmissilehandler[character] = encowner[enchantindex];
          break;

        case SETMORPH:
          encsetsave[enchantindex][valueindex] = ( chrtexture[character] - madskinstart[chrmodel[character]] ) % MAXSKIN;
          // Special handler for morph
          change_character( character, enchanttype, 0, LEAVE_ALL );
          chraimorphed[character] = btrue;
          break;

        case SETCHANNEL:
          encsetsave[enchantindex][valueindex] = chrcanchannel[character];
          chrcanchannel[character] = evesetvalue[enchanttype][valueindex];
          break;

      }
    }
  }
}

//--------------------------------------------------------------------------------------------
void getadd( int MIN, int value, int MAX, int* valuetoadd )
{
  // ZZ> This function figures out what value to add should be in order
  //     to not overflow the MIN and MAX bounds
  int newvalue;

  newvalue = value + ( *valuetoadd );
  if ( newvalue < MIN )
  {
    // Increase valuetoadd to fit
    *valuetoadd = MIN - value;
    if ( *valuetoadd > 0 )  *valuetoadd = 0;
    return;
  }


  if ( newvalue > MAX )
  {
    // Decrease valuetoadd to fit
    *valuetoadd = MAX - value;
    if ( *valuetoadd < 0 )  *valuetoadd = 0;
  }
}

//--------------------------------------------------------------------------------------------
void fgetadd( float MIN, float value, float MAX, float* valuetoadd )
{
  // ZZ> This function figures out what value to add should be in order
  //     to not overflow the MIN and MAX bounds
  float newvalue;


  newvalue = value + ( *valuetoadd );
  if ( newvalue < MIN )
  {
    // Increase valuetoadd to fit
    *valuetoadd = MIN - value;
    if ( *valuetoadd > 0 )  *valuetoadd = 0;
    return;
  }


  if ( newvalue > MAX )
  {
    // Decrease valuetoadd to fit
    *valuetoadd = MAX - value;
    if ( *valuetoadd < 0 )  *valuetoadd = 0;
  }
}

//--------------------------------------------------------------------------------------------
void add_enchant_value( Uint16 enchantindex, Uint8 valueindex,
                        Uint16 enchanttype )
{
  // ZZ> This function does cumulative modification to character stats
  int valuetoadd, newvalue;
  float fvaluetoadd, fnewvalue;
  CHR_REF character;


  character = enctarget[enchantindex];
  valuetoadd = 0;
  switch ( valueindex )
  {
    case ADDJUMPPOWER:
      fnewvalue = chrjump[character];
      fvaluetoadd = eveaddvalue[enchanttype][valueindex] / 16.0;
      fgetadd( 0, fnewvalue, 30.0, &fvaluetoadd );
      valuetoadd = fvaluetoadd * 16.0; // Get save value
      fvaluetoadd = valuetoadd / 16.0;
      chrjump[character] += fvaluetoadd;

      break;

    case ADDBUMPDAMPEN:
      fnewvalue = chrbumpdampen[character];
      fvaluetoadd = eveaddvalue[enchanttype][valueindex] / 128.0;
      fgetadd( 0, fnewvalue, 1.0, &fvaluetoadd );
      valuetoadd = fvaluetoadd * 128.0; // Get save value
      fvaluetoadd = valuetoadd / 128.0;
      chrbumpdampen[character] += fvaluetoadd;
      break;

    case ADDBOUNCINESS:
      fnewvalue = chrdampen[character];
      fvaluetoadd = eveaddvalue[enchanttype][valueindex] / 128.0;
      fgetadd( 0, fnewvalue, 0.95, &fvaluetoadd );
      valuetoadd = fvaluetoadd * 128.0; // Get save value
      fvaluetoadd = valuetoadd / 128.0;
      chrdampen[character] += fvaluetoadd;
      break;

    case ADDDAMAGE:
      newvalue = chrdamageboost[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, 4096, &valuetoadd );
      chrdamageboost[character] += valuetoadd;
      break;

    case ADDSIZE:
      fnewvalue = chrsizegoto[character];
      fvaluetoadd = eveaddvalue[enchanttype][valueindex] / 128.0;
      fgetadd( 0.5, fnewvalue, 2.0, &fvaluetoadd );
      valuetoadd = fvaluetoadd * 128.0; // Get save value
      fvaluetoadd = valuetoadd / 128.0;
      chrsizegoto[character] += fvaluetoadd;
      chrsizegototime[character] = DELAY_RESIZE;
      break;

    case ADDACCEL:
      fnewvalue = chrmaxaccel[character];
      fvaluetoadd = eveaddvalue[enchanttype][valueindex] / 25.0;
      fgetadd( 0, fnewvalue, 1.5, &fvaluetoadd );
      valuetoadd = fvaluetoadd * 1000.0; // Get save value
      fvaluetoadd = valuetoadd / 1000.0;
      chrmaxaccel[character] += fvaluetoadd;
      break;

    case ADDRED:
      newvalue = chrredshift[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex];
      getadd( 0, newvalue, 6, &valuetoadd );
      chrredshift[character] += valuetoadd;
      break;

    case ADDGRN:
      newvalue = chrgrnshift[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex];
      getadd( 0, newvalue, 6, &valuetoadd );
      chrgrnshift[character] += valuetoadd;
      break;

    case ADDBLU:
      newvalue = chrblushift[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex];
      getadd( 0, newvalue, 6, &valuetoadd );
      chrblushift[character] += valuetoadd;
      break;

    case ADDDEFENSE:
      newvalue = chrdefense_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex];
      getadd( 55, newvalue, 255, &valuetoadd );   // Don't fix again!
      chrdefense_fp8[character] += valuetoadd;
      break;

    case ADDMANA:
      newvalue = chrmanamax_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, HIGHSTAT, &valuetoadd );
      chrmanamax_fp8[character] += valuetoadd;
      chrmana_fp8[character] += valuetoadd;
      if ( chrmana_fp8[character] < 0 )  chrmana_fp8[character] = 0;
      break;

    case ADDLIFE:
      newvalue = chrlifemax_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( LOWSTAT, newvalue, HIGHSTAT, &valuetoadd );
      chrlifemax_fp8[character] += valuetoadd;
      chrlife_fp8[character] += valuetoadd;
      if ( chrlife_fp8[character] < 1 )  chrlife_fp8[character] = 1;
      break;

    case ADDSTRENGTH:
      newvalue = chrstrength_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, PERFECTSTAT, &valuetoadd );
      chrstrength_fp8[character] += valuetoadd;
      break;

    case ADDWISDOM:
      newvalue = chrwisdom_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, PERFECTSTAT, &valuetoadd );
      chrwisdom_fp8[character] += valuetoadd;
      break;

    case ADDINTELLIGENCE:
      newvalue = chrintelligence_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, PERFECTSTAT, &valuetoadd );
      chrintelligence_fp8[character] += valuetoadd;
      break;

    case ADDDEXTERITY:
      newvalue = chrdexterity_fp8[character];
      valuetoadd = eveaddvalue[enchanttype][valueindex] << 6;
      getadd( 0, newvalue, PERFECTSTAT, &valuetoadd );
      chrdexterity_fp8[character] += valuetoadd;
      break;
  }

  encaddsave[enchantindex][valueindex] = valuetoadd;  // Save the value for undo
}


//--------------------------------------------------------------------------------------------
Uint16 spawn_enchant( Uint16 owner, Uint16 target,
                      Uint16 spawner, Uint16 enchantindex, Uint16 modeloptional )
{
  // ZZ> This function enchants a target, returning the enchantment index or MAXENCHANT
  //     if failed
  Uint16 enchanttype, overlay;
  int add, cnt;


  if ( modeloptional < MAXMODEL )
  {
    // The enchantment type is given explicitly
    enchanttype = modeloptional;
  }
  else
  {
    // The enchantment type is given by the spawner
    enchanttype = chrmodel[spawner];
  }


  // Target and owner must both be alive and on and valid
  if ( target < MAXCHR )
  {
    if ( !VALID_CHR( target ) || !chralive[target] )
      return MAXENCHANT;
  }
  else
  {
    // Invalid target
    return MAXENCHANT;
  }
  if ( owner < MAXCHR )
  {
    if ( !VALID_CHR( owner ) || !chralive[owner] )
      return MAXENCHANT;
  }
  else
  {
    // Invalid target
    return MAXENCHANT;
  }


  if ( evevalid[enchanttype] )
  {
    if ( enchantindex == MAXENCHANT )
    {
      // Should it choose an inhand item?
      if ( everetarget[enchanttype] )
      {
        bool_t bfound = bfalse;
        SLOT best_slot = SLOT_BEGIN;

        // Is at least one valid?
        for ( _slot = SLOT_LEFT; _slot <= SLOT_RIGHT; _slot = ( SLOT )( _slot + 1 ) )
        {
          if ( chr_using_slot( target, _slot ) )
          {
            bfound = btrue;
            if ( _slot > best_slot ) best_slot = _slot;  // choose SLOT_RIGHT above SLOT_LEFT
            break;
          }
        };

        if ( !bfound ) return MAXENCHANT;

        target = chr_get_holdingwhich( target, best_slot );
      }


      // Make sure it's valid
      if ( evedontdamagetype[enchanttype] != DAMAGE_NULL )
      {
        if (( chrdamagemodifier_fp8[target][evedontdamagetype[enchanttype]]&7 ) >= 3 )   // Invert | Shift = 7
        {
          return MAXENCHANT;
        }
      }
      if ( eveonlydamagetype[enchanttype] != DAMAGE_NULL )
      {
        if ( chrdamagetargettype[target] != eveonlydamagetype[enchanttype] )
        {
          return MAXENCHANT;
        }
      }


      // Find one to use
      enchantindex = get_free_enchant();
    }
    else
    {
      numfreeenchant--;  // To keep it in order
    }
    if ( enchantindex < MAXENCHANT )
    {
      // Make a new one
      encon[enchantindex] = btrue;
      enctarget[enchantindex] = target;
      encowner[enchantindex] = owner;
      encspawner[enchantindex] = spawner;
      if ( spawner < MAXCHR )
      {
        chrundoenchant[spawner] = enchantindex;
      }
      enceve[enchantindex] = enchanttype;
      enctime[enchantindex] = evetime[enchanttype];
      encspawntime[enchantindex] = 1;
      encownermana[enchantindex] = eveownermana[enchanttype];
      encownerlife[enchantindex] = eveownerlife[enchanttype];
      enctargetmana[enchantindex] = evetargetmana[enchanttype];
      enctargetlife[enchantindex] = evetargetlife[enchanttype];



      // Add it as first in the list
      encnextenchant[enchantindex] = chrfirstenchant[target];
      chrfirstenchant[target] = enchantindex;


      // Now set all of the specific values, morph first
      set_enchant_value( enchantindex, SETMORPH, enchanttype );
      for ( cnt = SETDAMAGETYPE; cnt <= SETCOSTFOREACHMISSILE; cnt++ )
      {
        set_enchant_value( enchantindex, cnt, enchanttype );
      }
      set_enchant_value( enchantindex, SETCHANNEL, enchanttype );


      // Now do all of the stat adds
      add = 0;
      while ( add < EVE_ADD_COUNT )
      {
        add_enchant_value( enchantindex, add, enchanttype );
        add++;
      }

	    //Enchant to allow see kurses?
	    chrcanseekurse[target] = chrcanseekurse[target] || evecanseekurse[enchantindex];


      // Create an overlay character?
      encoverlay[enchantindex] = MAXCHR;
      if ( eveoverlay[enchanttype] )
      {
        overlay = spawn_one_character( chrpos[target], enchanttype, chrteam[target], 0, chrturn_lr[target], NULL, MAXCHR );
        if ( overlay < MAXCHR )
        {
          encoverlay[enchantindex] = overlay;  // Kill this character on end...
          chraitarget[overlay] = target;
          chraistate[overlay] = eveoverlay[enchanttype];
          chroverlay[overlay] = btrue;


          // Start out with ActionMJ...  Object activated
          if ( madactionvalid[chrmodel[overlay]][ACTION_MJ] )
          {
            chraction[overlay] = ACTION_MJ;
            chrlip_fp8[overlay] = 0;
            chrflip[overlay] = 0.0f;
            chrframe[overlay] = madactionstart[chrmodel[overlay]][ACTION_MJ];
            chrframelast[overlay] = chrframe[overlay];
            chractionready[overlay] = bfalse;
          }
          chrlight_fp8[overlay] = 254;  // Assume it's transparent...
        }
      }
    }
    return enchantindex;
  }
  return MAXENCHANT;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void load_action_names( char* loadname )
{
  // ZZ> This function loads all of the 2 letter action names
  FILE* fileread;
  ACTION cnt;
  char first, second;

  log_info( "load_action_names() - loading all 2 letter action names from %s.\n", loadname );

  fileread = fs_fileOpen( PRI_WARN, NULL, loadname, "r" );
  if ( NULL == fileread )
  {
    log_warning( "Problems loading model action codes (%s)\n", loadname );
    return;
  }

  for ( cnt = ACTION_ST; cnt < MAXACTION; cnt = ( ACTION )( cnt + 1 ) )
  {
    fgoto_colon( fileread );
    fscanf( fileread, "%c%c", &first, &second );
    cActionName[cnt][0] = first;
    cActionName[cnt][1] = second;
  }
  fs_fileClose( fileread );

}




//--------------------------------------------------------------------------------------------
void read_setup( char* filename )
{
  // ZZ> This function loads the setup file

  ConfigFilePtr lConfigSetup;
  char lCurSectionName[64];
  bool_t lTempBool;
  Sint32 lTmpInt;
  char lTmpStr[24];


  lConfigSetup = OpenConfigFile( filename );
  if ( lConfigSetup == NULL )
  {
    //Major Error
    log_error( "Could not find setup file (%s).\n", filename );
    return;
  }


  globalname = filename; // heu!?

  /*********************************************

  GRAPHIC Section

  *********************************************/

  strcpy( lCurSectionName, "GRAPHIC" );

  //Draw z reflection?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "Z_REFLECTION", &CData.zreflect ) == 0 )
  {
    CData.zreflect = CData_default.zreflect;
  }

  //Max number of vertices (Should always be 100!)
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_NUMBER_VERTICES", &lTmpInt ) == 0 )
  {
    CData.maxtotalmeshvertices = CData_default.maxtotalmeshvertices;
  }
  else
  {
    CData.maxtotalmeshvertices = lTmpInt * 1024;
  }

  //Do CData.fullscreen?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FULLSCREEN", &CData.fullscreen ) == 0 )
  {
    CData.fullscreen = CData_default.fullscreen;
  }

  //Screen Size
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SCREENSIZE_X", &lTmpInt ) == 0 )
  {
    CData.scrx = CData_default.scrx;
  }
  else
  {
    CData.scrx = lTmpInt;
  };

  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SCREENSIZE_Y", &lTmpInt ) == 0 )
  {
    CData.scry = CData_default.scry;
  }
  else
  {
    CData.scry = lTmpInt;
  };

  //Color depth
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "COLOR_DEPTH", &lTmpInt ) == 0 )
  {
    CData.scrd = CData_default.scrd;
  }
  else
  {
    CData.scrd = lTmpInt;
  };

  //The z depth
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "Z_DEPTH", &lTmpInt ) == 0 )
  {
    CData.scrz = CData_default.scrz;
  }
  else
  {
    CData.scrz = lTmpInt;
  };

  //Max number of messages displayed
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_TEXT_MESSAGE", &lTmpInt ) == 0 )
  {
    CData.messageon  = CData_default.messageon;
    CData.maxmessage = CData_default.maxmessage;
  }
  else
  {
    CData.messageon = btrue;
    CData.maxmessage = lTmpInt;
  };

  if ( CData.maxmessage < 1 )  { CData.maxmessage = 1;  CData.messageon = bfalse; }
  if ( CData.maxmessage > MAXMESSAGE )  { CData.maxmessage = MAXMESSAGE; }

  //Show status bars? (Life, mana, character icons, etc.)
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "STATUS_BAR", &CData.staton ) == 0 )
  {
    CData.staton = CData_default.staton;
  }

  CData.wraptolerance = 32;

  if ( CData.staton )
  {
    CData.wraptolerance = 90;
  }

  //Perspective correction
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "PERSPECTIVE_CORRECT", &lTempBool ) == 0 )
  {
    CData.perspective = CData_default.perspective;
  }
  else
  {
    CData.perspective = lTempBool ? GL_NICEST : GL_FASTEST;
  };

  //Enable dithering?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DITHERING", &CData.dither ) == 0 )
  {
    CData.dither = CData_default.dither;
  }

  //Reflection fadeout
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FLOOR_REFLECTION_FADEOUT", &lTempBool ) == 0 )
  {
    CData.reffadeor = CData_default.reffadeor;
  }
  else
  {
    CData.reffadeor = (lTempBool ? 0 : 255);
  };  


  //Draw Reflection?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "REFLECTION", &CData.refon ) == 0 )
  {
    CData.refon = CData_default.refon;
  }

  //Draw shadows?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SHADOWS", &CData.shaon ) == 0 )
  {
    CData.shaon = CData_default.shaon;
  }

  //Draw good shadows
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SHADOW_AS_SPRITE", &CData.shasprite ) == 0 )
  {
    CData.shasprite = CData_default.shasprite;
  }

  //Draw phong mapping?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "PHONG", &CData.phongon ) == 0 )
  {
    CData.phongon = CData_default.phongon;
  }

  //Draw water with more layers?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "MULTI_LAYER_WATER", &CData.twolayerwateron ) == 0 )
  {
    CData.twolayerwateron = CData_default.twolayerwateron;
  }

  //TODO: This is not implemented
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "OVERLAY", &CData.overlayvalid ) == 0 )
  {
    CData.overlayvalid = CData_default.overlayvalid;
  }

  //Allow backgrounds?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "BACKGROUND", &CData.backgroundvalid ) == 0 )
  {
    CData.backgroundvalid = CData_default.backgroundvalid;
  }

  //Enable fog?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FOG", &CData.fogallowed ) == 0 )
  {
    CData.fogallowed = CData_default.fogallowed;
  }

  //Do gourad CData.shading?
  CData.shading = CData_default.shading;
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "GOURAUD_SHADING", &lTempBool ) != 0 )
  {
    CData.shading = lTempBool ? GL_SMOOTH : GL_FLAT;
  }

  //Enable CData.antialiasing?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "ANTIALIASING", &CData.antialiasing ) == 0 )
  {
    CData.antialiasing = CData_default.antialiasing;
  }

  //Do we do texture filtering?

  if ( GetConfigValue( lConfigSetup, lCurSectionName, "TEXTURE_FILTERING", lTmpStr, 24 ) == 0 )
  {
    CData.texturefilter = 1;
  }
  else if ( isdigit( lTmpStr[0] ) )
  {
    sscanf( lTmpStr, "%d", &CData.texturefilter );
    if ( CData.texturefilter >= TX_ANISOTROPIC )
    {
      int tmplevel = CData.texturefilter - TX_ANISOTROPIC + 1;
      userAnisotropy = 1 << tmplevel;
    }
  }
  else if ( toupper(lTmpStr[0]) == 'L')  CData.texturefilter = TX_LINEAR;
  else if ( toupper(lTmpStr[0]) == 'B')  CData.texturefilter = TX_BILINEAR;
  else if ( toupper(lTmpStr[0]) == 'T')  CData.texturefilter = TX_TRILINEAR_2;
  else if ( toupper(lTmpStr[0]) == 'A')  CData.texturefilter = TX_ANISOTROPIC + log2Anisotropy;

  if ( GetConfigValue( lConfigSetup, lCurSectionName, "PARTICLE_EFFECTS", lTmpStr, 24 ) == 0 )
  {
    CData.particletype = PART_NORMAL; //Default
  }
  else if ( toupper(lTmpStr[0]) == 'N' )  CData.particletype = PART_NORMAL;
  else if ( toupper(lTmpStr[0]) == 'S' )  CData.particletype = PART_SMOOTH;
  else if ( toupper(lTmpStr[0]) == 'F' )  CData.particletype = PART_FAST;

  //Do vertical sync?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "VERTICAL_SYNC", &CData.backgroundvalid ) == 0 )
  {
    CData.vsync = CData_default.vsync;
  }

  //Force openGL hardware acceleration
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "GFX_ACCELERATION", &CData.backgroundvalid ) == 0 )
  {
    CData.gfxacceleration = CData_default.gfxacceleration;
  }

  /*********************************************

  SOUND Section

  *********************************************/

  strcpy( lCurSectionName, "SOUND" );

  //Enable sound
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SOUND", &CData.soundvalid ) == 0 )
  {
    CData.soundvalid = CData_default.soundvalid;
  }

  //Enable music
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "MUSIC", &CData.musicvalid ) == 0 )
  {
    CData.musicvalid = CData_default.musicvalid;
  }

  //Music volume
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MUSIC_VOLUME", &CData.musicvolume ) == 0 )
  {
    CData.musicvolume = CData_default.musicvolume;
  }

  //Sound volume
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SOUND_VOLUME", &CData.soundvolume ) == 0 )
  {
    CData.soundvolume = CData_default.soundvolume;
  }

  //Max number of sound channels playing at the same time
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_SOUND_CHANNEL", &CData.maxsoundchannel ) == 0 )
  {
    CData.maxsoundchannel = CData_default.maxsoundchannel;
  }
  if ( CData.maxsoundchannel < 8 ) CData.maxsoundchannel = 8;
  if ( CData.maxsoundchannel > 32 ) CData.maxsoundchannel = 32;

  //The output buffer size
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "OUPUT_BUFFER_SIZE", &CData.buffersize ) == 0 )
  {
    CData.buffersize = CData_default.buffersize;
  }
  if ( CData.buffersize < 512 ) CData.buffersize = 512;
  if ( CData.buffersize > 8196 ) CData.buffersize = 8196;


  /*********************************************

  CONTROL Section

  *********************************************/

  strcpy( lCurSectionName, "CONTROL" );

  //Camera control mode
  if ( GetConfigValue( lConfigSetup, lCurSectionName, "AUTOTURN_CAMERA", lTmpStr, 24 ) == 0 )
  {
    CData.autoturncamera = CData_default.autoturncamera;
  }
  else
  {
    switch( toupper(lTmpStr[0]) )
    {
      case 'G': CData.autoturncamera = 255;    break;
      case 'T': CData.autoturncamera = btrue;  break;

      default:
      case 'F': CData.autoturncamera = bfalse; break;
    }
  }


  /*********************************************

  NETWORK Section

  *********************************************/

  strcpy( lCurSectionName, "NETWORK" );

  //Enable networking systems?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "NETWORK_ON", &CData.networkon ) == 0 )
  {
    CData.networkon = CData_default.networkon;
  }

  //Max CData.lag
  if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "LAG_TOLERANCE", &lTmpInt ) == 0 )
  {
    CData.lag = CData_default.lag;
  }
  else
  {
    CData.lag = lTmpInt;
  };

  //Name or IP of the host or the target to join
  if ( GetConfigValue( lConfigSetup, lCurSectionName, "HOST_NAME", CData.nethostname, 64 ) == 0 )
  {
    strcpy( CData.nethostname, CData_default.nethostname );
  }

  //Multiplayer name
  if ( GetConfigValue( lConfigSetup, lCurSectionName, "MULTIPLAYER_NAME", CData.netmessagename, 64 ) == 0 )
  {
    strcpy( CData.netmessagename, CData_default.netmessagename );
  }


  /*********************************************

  DEBUG Section

  *********************************************/

  strcpy( lCurSectionName, "DEBUG" );

  //Show the FPS counter?
  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DISPLAY_FPS", &lTempBool ) == 0 )
  {
    CData.fpson = CData_default.fpson;
  }
  else
  {
    CData.fpson = lTempBool;
  };

  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "HIDE_MOUSE", &CData.HideMouse ) == 0 )
  {
    CData.HideMouse = CData_default.HideMouse;
  }

  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "GRAB_MOUSE", &lTempBool ) == 0 )
  {
    CData.GrabMouse = CData_default.GrabMouse;
  }
  else
  {
    CData.GrabMouse = lTempBool ? SDL_GRAB_ON : SDL_GRAB_OFF;
  };

  if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DEVELOPER_MODE", &CData.DevMode ) == 0 )
  {
    CData.DevMode = CData_default.DevMode;
  }

  CloseConfigFile( lConfigSetup );

}
//--------------------------------------------------------------------------------------------
void log_madused( char *savename )
{
  // ZZ> This is a debug function for checking model loads
  FILE* hFileWrite;
  int cnt;

  hFileWrite = fs_fileOpen( PRI_NONE, NULL, savename, "w" );
  if ( hFileWrite )
  {
    fprintf( hFileWrite, "Slot usage for objects in last module loaded...\n" );
    fprintf( hFileWrite, "%d of %d frames used...\n", madloadframe, MAXFRAME );
    cnt = 0;
    while ( cnt < MAXMODEL )
    {
      fprintf( hFileWrite, "%3d %32s %s\n", cnt, capclassname[cnt], madname[cnt] );
      cnt++;
    }
    fs_fileClose( hFileWrite );
  }
}

//---------------------------------------------------------------------------------------------
void make_lightdirectionlookup()
{
  // ZZ> This function builds the lighting direction table
  //     The table is used to find which direction the light is coming
  //     from, based on the four corner vertices of a mesh tile.
  Uint32 cnt;
  Uint16 tl, tr, br, bl;
  int x, y;

  for ( cnt = 0; cnt < UINT16_SIZE; cnt++ )
  {
    tl = ( cnt & 0xf000 ) >> 12;
    tr = ( cnt & 0x0f00 ) >> 8;
    br = ( cnt & 0x00f0 ) >> 4;
    bl = ( cnt & 0x000f );
    x = br + tr - bl - tl;
    y = br + bl - tl - tr;
    lightdirectionlookup[cnt] = ( atan2( -y, x ) + PI ) * RAD_TO_BYTE;
  }
}


//---------------------------------------------------------------------------------------------
int vertexconnected( MD2_Model * m, int vertex )
{
  // ZZ> This function returns 1 if the model vertex is connected, 0 otherwise

  MD2_GLCommand * g;
  int commands, entry;

  if(NULL == m) return 0;

  g = md2_get_Commands(m);
  if(NULL == g) return 0;

  for( /*nothing*/; NULL != g; g = g->next)
  {
    commands = g->command_count;
    for(entry = 0; entry<commands; entry++)
    {
      if(g->data[entry].index == vertex)
      {
        return 1;
      }
    }
  }

  // The vertex is not used
  return 0;
}

//---------------------------------------------------------------------------------------------
int count_madtransvertices( MD2_Model * m )
{
  // ZZ> This function gets the number of vertices to transform for a model...
  //     That means every one except the grip ( unconnected ) vertices
  int cnt, vrtcount, trans = 0;

  if(NULL == m) return 0;

  vrtcount = md2_get_numVertices(m);

  for ( cnt = 0; cnt < vrtcount; cnt++ )
    trans += vertexconnected( m, cnt );

  return trans;
}

//---------------------------------------------------------------------------------------------
int rip_md2_header( void )
{
  // ZZ> This function makes sure an md2 is really an md2
  int iTmp;
  int* ipIntPointer;

  // Check the file type
  ipIntPointer = ( int* ) cLoadBuffer;
  iTmp = ipIntPointer[0];

#if SDL_BYTEORDER != SDL_LIL_ENDIAN
  iTmp = SDL_Swap32( iTmp );
#endif

  if ( iTmp != MD2START ) return bfalse;

  return btrue;
}

////---------------------------------------------------------------------------------------------
//void fix_md2_normals( Uint16 modelindex )
//{
//  // ZZ> This function helps light not flicker so much
//  int cnt, tnc;
//  Uint8 indexofcurrent, indexofnext, indexofnextnext, indexofnextnextnext;
//  Uint8 indexofnextnextnextnext;
//  Uint32 frame;
//
//  frame = madframestart[modelindex];
//  cnt = 0;
//  while ( cnt < madvertices[modelindex] )
//  {
//    tnc = 0;
//    while ( tnc < madframes[modelindex] )
//    {
//      indexofcurrent = madvrta[frame][cnt];
//      indexofnext = madvrta[frame+1][cnt];
//      indexofnextnext = madvrta[frame+2][cnt];
//      indexofnextnextnext = madvrta[frame+3][cnt];
//      indexofnextnextnextnext = madvrta[frame+4][cnt];
//      if ( indexofcurrent == indexofnextnext && indexofnext != indexofcurrent )
//      {
//        madvrta[frame+1][cnt] = indexofcurrent;
//      }
//      if ( indexofcurrent == indexofnextnextnext )
//      {
//        if ( indexofnext != indexofcurrent )
//        {
//          madvrta[frame+1][cnt] = indexofcurrent;
//        }
//        if ( indexofnextnext != indexofcurrent )
//        {
//          madvrta[frame+2][cnt] = indexofcurrent;
//        }
//      }
//      if ( indexofcurrent == indexofnextnextnextnext )
//      {
//        if ( indexofnext != indexofcurrent )
//        {
//          madvrta[frame+1][cnt] = indexofcurrent;
//        }
//        if ( indexofnextnext != indexofcurrent )
//        {
//          madvrta[frame+2][cnt] = indexofcurrent;
//        }
//        if ( indexofnextnextnext != indexofcurrent )
//        {
//          madvrta[frame+3][cnt] = indexofcurrent;
//        }
//      }
//      tnc++;
//    }
//    cnt++;
//  }
//}
//
//---------------------------------------------------------------------------------------------
//void rip_md2_commands( Uint16 modelindex )
//{
//  // ZZ> This function converts an md2's GL commands into our little command list thing
//  int iTmp;
//  float fTmpu, fTmpv;
//  int iNumVertices;
//  int tnc;
//
//  char* cpCharPointer = ( char* ) cLoadBuffer;
//  int* ipIntPointer = ( int* ) cLoadBuffer;
//  float* fpFloatPointer = ( float* ) cLoadBuffer;
//
//  // Number of GL commands in the MD2
//  int iNumCommands = ipIntPointer[9];
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//  iNumCommands = SDL_Swap32( iNumCommands );
//#endif
//
//  // Offset (in DWORDS) from the start of the file to the gl command list.
//  int iCommandOffset = ipIntPointer[15] >> 2;
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//  iCommandOffset = SDL_Swap32( iCommandOffset );
//#endif
//
//  // Read in each command
//  // iNumCommands isn't the number of commands, rather the number of dwords in
//  // the command list...  Use iCommandCount to figure out how many we use
//  int iCommandCount = 0;
//  int entry = 0;
//
//  int cnt = 0;
//  while ( cnt < iNumCommands )
//  {
//    iNumVertices = ipIntPointer[iCommandOffset];
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//    iNumVertices = SDL_Swap32( iNumVertices );
//#endif
//
//    iCommandOffset++;
//    cnt++;
//
//    if ( iNumVertices != 0 )
//    {
//      if ( iNumVertices < 0 )
//      {
//        // Fans start with a negative
//        iNumVertices = -iNumVertices;
//        // PORT: madcommandtype[modelindex][iCommandCount] = (Uint8) D3DPT_TRIANGLEFAN;
//        madcommandtype[modelindex][iCommandCount] = GL_TRIANGLE_FAN;
//        madcommandsize[modelindex][iCommandCount] = ( Uint8 ) iNumVertices;
//      }
//      else
//      {
//        // Strips start with a positive
//        madcommandtype[modelindex][iCommandCount] = GL_TRIANGLE_STRIP;
//        madcommandsize[modelindex][iCommandCount] = ( Uint8 ) iNumVertices;
//      }
//
//      // Read in vertices for each command
//      tnc = 0;
//      while ( tnc < iNumVertices )
//      {
//        fTmpu = fpFloatPointer[iCommandOffset];  iCommandOffset++;  cnt++;
//        fTmpv = fpFloatPointer[iCommandOffset];  iCommandOffset++;  cnt++;
//        iTmp = ipIntPointer[iCommandOffset];  iCommandOffset++;  cnt++;
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//        fTmpu = LoadFloatByteswapped( &fTmpu );
//        fTmpv = LoadFloatByteswapped( &fTmpv );
//        iTmp = SDL_Swap32( iTmp );
//#endif
//        madcommandu[modelindex][entry] = fTmpu - ( .5 / 64 ); // GL doesn't align correctly
//        madcommandv[modelindex][entry] = fTmpv - ( .5 / 64 ); // with D3D
//        madcommandvrt[modelindex][entry] = ( Uint16 ) iTmp;
//        entry++;
//        tnc++;
//      }
//      iCommandCount++;
//    }
//  }
//  madcommands[modelindex] = iCommandCount;
//}

//---------------------------------------------------------------------------------------------
//char * rip_md2_frame_name( MD2_Model * m, int frame )
//{
//  // ZZ> This function gets frame names from the load buffer, it returns
//  //     btrue if the name in cFrameName[] is valid
//  int iFrameOffset;
//  int iNumVertices;
//  int iNumFrames;
//  int cnt;
//  MD2_Frame * pFrame;
//  char      * pFrameName;
//  bool_t foundname;
//
//
//  if(NULL == m) return bfalse;
//
//  // Jump to the Frames section of the md2 data
//  
//
//  ipNamePointer = ( int* ) pFrame->name;
//
//
//  iNumVertices = ipIntPointer[6];
//  iNumFrames = ipIntPointer[10];
//  iFrameOffset = ipIntPointer[14] >> 2;
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//  iNumVertices = SDL_Swap32( iNumVertices );
//  iNumFrames = SDL_Swap32( iNumFrames );
//  iFrameOffset = SDL_Swap32( iFrameOffset );
//#endif
//
//
//  // Chug through each frame
//  foundname = bfalse;
//  
//  for ( cnt = 0; cnt < iNumFrames && !foundname; cnt++ )
//  {
//    pFrame     = md2_get_Frame(m , frame);
//    pFrameName = pFrame->name;
//
//    iFrameOffset += 6;
//    if ( cnt == frame )
//    {
//      ipNamePointer[0] = ipIntPointer[iFrameOffset]; iFrameOffset++;
//      ipNamePointer[1] = ipIntPointer[iFrameOffset]; iFrameOffset++;
//      ipNamePointer[2] = ipIntPointer[iFrameOffset]; iFrameOffset++;
//      ipNamePointer[3] = ipIntPointer[iFrameOffset]; iFrameOffset++;
//      foundname = btrue;
//    }
//    else
//    {
//      iFrameOffset += 4;
//    }
//    iFrameOffset += iNumVertices;
//    cnt++;
//  }
//  cFrameName[15] = 0;  // Make sure it's null terminated
//  return foundname;
//}

//---------------------------------------------------------------------------------------------
//void rip_md2_frames( MD2_Model * m )
//{
//  // ZZ> This function gets frames from the load buffer and adds them to
//  //     the indexed model
//  Uint8 cTmpx, cTmpy, cTmpz;
//  Uint8 cTmpNormalIndex;
//  float fRealx, fRealy, fRealz;
//  float fScalex, fScaley, fScalez;
//  float fTranslatex, fTranslatey, fTranslatez;
//  int iFrameOffset;
//  int iNumVertices;
//  int iNumFrames;
//  int cnt, tnc;
//  char* cpCharPointer;
//  int* ipIntPointer;
//  float* fpFloatPointer;
//
//  if(NULL == m) return;
//
//
//  // Jump to the Frames section of the md2 data
//  cpCharPointer = ( char* ) cLoadBuffer;
//  ipIntPointer = ( int* ) cLoadBuffer;
//  fpFloatPointer = ( float* ) cLoadBuffer;
//
//
//  iNumVertices = md2_get_numVertices(m);
//  iNumFrames   = md2_get_numFrames(m);
//
//  
//  for( cnt = 0; cnt < iNumFrames; cnt++ )
//  {
//    MD2_Frame * = MD2_Frame(m, cnt);
//
//    fScalex = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//    fScaley = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//    fScalez = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//    fTranslatex = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//    fTranslatey = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//    fTranslatez = fpFloatPointer[iFrameOffset]; iFrameOffset++;
//
//#if SDL_BYTEORDER != SDL_LIL_ENDIAN
//    fScalex = LoadFloatByteswapped( &fScalex );
//    fScaley = LoadFloatByteswapped( &fScaley );
//    fScalez = LoadFloatByteswapped( &fScalez );
//
//    fTranslatex = LoadFloatByteswapped( &fTranslatex );
//    fTranslatey = LoadFloatByteswapped( &fTranslatey );
//    fTranslatez = LoadFloatByteswapped( &fTranslatez );
//#endif
//
//    iFrameOffset += 4;
//    tnc = 0;
//    while ( tnc < iNumVertices )
//    {
//      // This should work because it's reading a single character
//      cTmpx = cpCharPointer[( iFrameOffset<<2 )];
//      cTmpy = cpCharPointer[( iFrameOffset<<2 ) +1];
//      cTmpz = cpCharPointer[( iFrameOffset<<2 ) +2];
//      cTmpNormalIndex = cpCharPointer[( iFrameOffset<<2 ) +3];
//      fRealx = ( cTmpx * fScalex ) + fTranslatex;
//      fRealy = ( cTmpy * fScaley ) + fTranslatey;
//      fRealz = ( cTmpz * fScalez ) + fTranslatez;
//      madvrtx[madloadframe][tnc] = -fRealx * 3.5;
//      madvrty[madloadframe][tnc] = fRealy * 3.5;
//      madvrtz[madloadframe][tnc] = fRealz * 3.5;
//      madvrta[madloadframe][tnc] = cTmpNormalIndex;
//      iFrameOffset++;
//      tnc++;
//    }
//    madloadframe++;
//    cnt++;
//  }
//}

//---------------------------------------------------------------------------------------------
int load_one_md2( char * szLoadname, Uint16 imdl )
{
  // ZZ> This function loads an id md2 file, storing the converted data in the indexed model
  //    int iFileHandleRead;

  size_t iBytesRead = 0;
  int iFrames;

  // make sure this model is empty
  if(NULL != mad_md2[imdl])
  {
    free_one_md2(imdl);
  }

  // load the actual md2 data
  mad_md2[imdl] = md2_load( szLoadname, NULL );
  if(NULL == mad_md2[imdl]) return bfalse;

  // Figure out how many vertices to transform
  madvertices[imdl]      = md2_get_numVertices( mad_md2[imdl] );
  madtransvertices[imdl] = count_madtransvertices( mad_md2[imdl] );

  iFrames = md2_get_numFrames(mad_md2[imdl]);

  madframelip[imdl] = calloc(sizeof(Uint8),  iFrames);
  madframefx[imdl]  = calloc(sizeof(Uint16), iFrames);
  return btrue;
}

//---------------------------------------------------------------------------------------------
void free_one_md2( Uint16 imdl )
{
  // ZZ> This function loads an id md2 file, storing the converted data in the indexed model
  //    int iFileHandleRead;

  if(imdl > MAXMODEL) return;

  if(NULL != mad_md2[imdl])
  {
    md2_delete(mad_md2[imdl]);
    mad_md2[imdl] = NULL;
  }

  if(NULL != madframelip[imdl])
  {
    free(madframelip[imdl]);
    madframelip[imdl] = NULL;
  };

  if(NULL != madframefx[imdl])
  {
    free(madframefx[imdl]);
    madframefx[imdl] = NULL;
  };

  
}

//--------------------------------------------------------------------------------------------
void make_enviro( void )
{
  // ZZ> This function sets up the environment mapping table
  int cnt;
  float z;
  float x, y;

  // Find the environment map positions
  for ( cnt = 0; cnt < MD2LIGHTINDICES; cnt++ )
  {
    x = -kMd2Normals[cnt][0];
    y = kMd2Normals[cnt][1];
    x = 1.0f + atan2( y, x ) / PI;
    x--;

    if ( x < 0 )
      x--;

    indextoenvirox[cnt] = x;
  }

  for ( cnt = 0; cnt < 256; cnt++ )
  {
    z = FP8_TO_FLOAT( cnt );   // Z is between 0 and 1
    lighttoenviroy[cnt] = z;
  }
}

//--------------------------------------------------------------------------------------------
void show_stat( Uint16 statindex )
{
  // ZZ> This function shows the more specific stats for a character
  CHR_REF character;
  char gender[8];

  if ( statdelay == 0 )
  {
    if ( statindex < numstat )
    {
      character = statlist[statindex];


      // Name
      debug_message( 1, "=%s=", chrname[character] );

      // Level and gender and class
      gender[0] = '\0';
      if ( chralive[character] )
      {
        switch ( chrgender[character] )
        {
          case GEN_MALE: snprintf( gender, sizeof( gender ), "Male" ); break;
          case GEN_FEMALE: snprintf( gender, sizeof( gender ), "Female" ); break;
        };

        debug_message( 1, " %s %s", gender, capclassname[chrmodel[character]] );
      }
      else
      {
        debug_message( 1, " Dead %s", capclassname[chrmodel[character]] );
      }

      // Stats
      debug_message( 1, " STR:%2.1f ~WIS:%2.1f ~INT:%2.1f", FP8_TO_FLOAT( chrstrength_fp8[character] ), FP8_TO_FLOAT( chrwisdom_fp8[character] ), FP8_TO_FLOAT( chrintelligence_fp8[character] ) );
      debug_message( 1, " DEX:%2.1f ~LVL:%4.1f ~DEF:%2.1f", FP8_TO_FLOAT( chrdexterity_fp8[character] ), calc_chr_level( character ), FP8_TO_FLOAT( chrdefense_fp8[character] ) );

      statdelay = 10;
    }
  }
}


//--------------------------------------------------------------------------------------------
void check_stats()
{
  // ZZ> This function lets the players check character stats
  if ( !netmessagemode )
  {
    if ( SDLKEYDOWN( SDLK_1 ) )  show_stat( 0 );
    if ( SDLKEYDOWN( SDLK_2 ) )  show_stat( 1 );
    if ( SDLKEYDOWN( SDLK_3 ) )  show_stat( 2 );
    if ( SDLKEYDOWN( SDLK_4 ) )  show_stat( 3 );
    if ( SDLKEYDOWN( SDLK_5 ) )  show_stat( 4 );
    if ( SDLKEYDOWN( SDLK_6 ) )  show_stat( 5 );
    if ( SDLKEYDOWN( SDLK_7 ) )  show_stat( 6 );
    if ( SDLKEYDOWN( SDLK_8 ) )  show_stat( 7 );

    // !!!BAD!!!  CHEAT
    if ( CData.DevMode && SDLKEYDOWN( SDLK_x ) )
    {
      if ( SDLKEYDOWN( SDLK_1 ) && VALID_CHR( plachr[0] ) )  give_experience( plachr[0], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_2 ) && VALID_CHR( plachr[1] ) )  give_experience( plachr[1], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_3 ) && VALID_CHR( plachr[2] ) )  give_experience( plachr[2], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_4 ) && VALID_CHR( plachr[3] ) )  give_experience( plachr[3], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_5 ) && VALID_CHR( plachr[4] ) )  give_experience( plachr[4], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_6 ) && VALID_CHR( plachr[5] ) )  give_experience( plachr[5], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_7 ) && VALID_CHR( plachr[6] ) )  give_experience( plachr[6], 25, XP_DIRECT );
      if ( SDLKEYDOWN( SDLK_8 ) && VALID_CHR( plachr[7] ) )  give_experience( plachr[7], 25, XP_DIRECT );
      statdelay = 0;
    }
  }
}

void check_screenshot()
{
  //This function checks if we want to take a screenshot
  if ( SDLKEYDOWN( SDLK_F11 ) )
  {
    if ( !dump_screenshot() )               //Take the shot, returns bfalse if failed
    {
      debug_message( 1, "Error writing screenshot" );
      log_warning( "Error writing screenshot\n" );    //Log the error in log.txt
    }
  }
}

bool_t dump_screenshot()
{
  // dumps the current screen (GL context) to a new bitmap file
  // right now it dumps it to whatever the current directory is

  // returns btrue if successful, bfalse otherwise

  SDL_Surface *screen, *temp;
  Uint8 *pixels;
  STRING buff;
  int i;
  FILE *test;

  screen = SDL_GetVideoSurface();
  temp = SDL_CreateRGBSurface( SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                               0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
                               0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
                             );


  if ( temp == NULL )
    return bfalse;

  pixels = malloc( 3 * screen->w * screen->h );
  if ( pixels == NULL )
  {
    SDL_FreeSurface( temp );
    return bfalse;
  }

  glReadPixels( 0, 0, screen->w, screen->h, GL_RGB, GL_UNSIGNED_BYTE, pixels );

  for ( i = 0; i < screen->h; i++ )
    memcpy((( char * ) temp->pixels ) + temp->pitch * i, pixels + 3 * screen->w * ( screen->h - i - 1 ), screen->w * 3 );
  free( pixels );

  // find the next EGO??.BMP file for writing
  i = 0;
  test = NULL;

  do
  {
    if ( test != NULL )
      fs_fileClose( test );

    snprintf( buff, sizeof( buff ), "ego%02d.bmp", i );

    // lame way of checking if the file already exists...
    test = fs_fileOpen( PRI_NONE, NULL, buff, "rb" );
    i++;

  }
  while (( test != NULL ) && ( i < 100 ) );

  SDL_SaveBMP( temp, buff );
  SDL_FreeSurface( temp );

  debug_message( 1, "Saved to %s", buff );

  return btrue;
}

//--------------------------------------------------------------------------------------------
void add_stat( CHR_REF character )
{
  // ZZ> This function adds a status display to the do list
  if ( numstat < MAXSTAT )
  {
    statlist[numstat] = character;
    chrstaton[character] = btrue;
    numstat++;
  }
}

//--------------------------------------------------------------------------------------------
void move_to_top( CHR_REF character )
{
  // ZZ> This function puts the character on top of the statlist
  int cnt, oldloc;


  // Find where it is
  oldloc = numstat;

  for ( cnt = 0; cnt < numstat; cnt++ )
    if ( statlist[cnt] == character )
    {
      oldloc = cnt;
      cnt = numstat;
    }

  // Change position
  if ( oldloc < numstat )
  {
    // Move all the lower ones up
    while ( oldloc > 0 )
    {
      oldloc--;
      statlist[oldloc+1] = statlist[oldloc];
    }
    // Put the character in the top slot
    statlist[0] = character;
  }
}

//--------------------------------------------------------------------------------------------
void sort_stat()
{
  // ZZ> This function puts all of the local players on top of the statlist
  int cnt;

  for ( cnt = 0; cnt < numpla; cnt++ )
  {
    if ( !VALID_PLA( cnt ) || INBITS_NONE == pladevice[cnt] ) continue;

    move_to_top( plachr[cnt] );
  }
}

//--------------------------------------------------------------------------------------------
void move_water( float dUpdate )
{
  // ZZ> This function animates the water overlays
  int layer;

  for ( layer = 0; layer < MAXWATERLAYER; layer++ )
  {
    waterlayeru[layer] += waterlayeruadd[layer] * dUpdate;
    waterlayerv[layer] += waterlayervadd[layer] * dUpdate;
    if ( waterlayeru[layer] > 1.0 )  waterlayeru[layer] -= 1.0;
    if ( waterlayerv[layer] > 1.0 )  waterlayerv[layer] -= 1.0;
    if ( waterlayeru[layer] < -1.0 )  waterlayeru[layer] += 1.0;
    if ( waterlayerv[layer] < -1.0 )  waterlayerv[layer] += 1.0;
    waterlayerframe[layer] = (( int )( waterlayerframe[layer] + waterlayerframeadd[layer] * dUpdate ) ) & WATERFRAMEAND;
  }
}

//--------------------------------------------------------------------------------------------
void play_action( CHR_REF character, ACTION action, bool_t actionready )
{
  // ZZ> This function starts a generic action for a character
  if ( madactionvalid[chrmodel[character]][action] )
  {
    chrnextaction[character] = ACTION_DA;
    chraction[character] = action;
    chrlip_fp8[character] = 0;
    chrflip[character] = 0.0f;
    chrframelast[character] = chrframe[character];
    chrframe[character] = madactionstart[chrmodel[character]][chraction[character]];
    chractionready[character] = actionready;
  }
}

//--------------------------------------------------------------------------------------------
void set_frame( CHR_REF character, Uint16 frame, Uint8 lip )
{
  // ZZ> This function sets the frame for a character explicitly...  This is used to
  //     rotate Tank turrets
  chrnextaction[character] = ACTION_DA;
  chraction[character] = ACTION_DA;
  chrlip_fp8[character] = ( lip << 6 );
  chrflip[character] = lip * 0.25;
  chrframelast[character] = madactionstart[chrmodel[character]][ACTION_DA] + frame;
  chrframe[character] = madactionstart[chrmodel[character]][ACTION_DA] + frame + 1;
  chractionready[character] = btrue;
}

//--------------------------------------------------------------------------------------------
void reset_character_alpha( CHR_REF character )
{
  // ZZ> This function fixes an item's transparency
  Uint16 enchant, mount;

  if ( !VALID_CHR( character ) ) return;

  mount = chr_get_attachedto( character );
  if ( VALID_CHR( mount ) && chrisitem[character] && chrtransferblend[mount] )
  {
    // Okay, reset transparency
    enchant = chrfirstenchant[character];
    while ( enchant < MAXENCHANT )
    {
      unset_enchant_value( enchant, SETALPHABLEND );
      unset_enchant_value( enchant, SETLIGHTBLEND );
      enchant = encnextenchant[enchant];
    }
    chralpha_fp8[character] = capalpha_fp8[chrmodel[character]];
    chrbumpstrength[character] = capbumpstrength[chrmodel[character]] * FP8_TO_FLOAT( chralpha_fp8[character] );
    chrlight_fp8[character] = caplight_fp8[chrmodel[character]];
    enchant = chrfirstenchant[character];
    while ( enchant < MAXENCHANT )
    {
      set_enchant_value( enchant, SETALPHABLEND, enceve[enchant] );
      set_enchant_value( enchant, SETLIGHTBLEND, enceve[enchant] );
      enchant = encnextenchant[enchant];
    }
  }

}

//--------------------------------------------------------------------------------------------
Uint32 generate_unsigned( PAIR * ppair )
{
  // ZZ> This function generates a random number

  Uint32 itmp = 0;

  if ( NULL != ppair )
  {
    itmp = ppair->ibase;

    if ( ppair->irand > 1 )
    {
      itmp += rand() % ppair->irand;
    }
  }
  else
  {
    itmp = rand();
  }

  return itmp;
}

//--------------------------------------------------------------------------------------------
Sint32 generate_signed( PAIR * ppair )
{
  // ZZ> This function generates a random number

  Sint32 itmp = 0;

  if ( NULL != ppair )
  {
    itmp = ppair->ibase;

    if ( ppair->irand > 1 )
    {
      itmp += rand() % ppair->irand;
      itmp -= ppair->irand >> 1;
    }
  }
  else
  {
    itmp = rand();
  }

  return itmp;
}


//--------------------------------------------------------------------------------------------
Sint32 generate_dither( PAIR * ppair, Uint16 strength_fp8 )
{
  // ZZ> This function generates a random number

  Sint32 itmp = 0;

  if ( NULL != ppair && ppair->irand > 1 )
  {
    itmp = rand();
    itmp %= ppair->irand;
    itmp -= ppair->irand >> 1;

    if ( strength_fp8 != INT_TO_FP8( 1 ) )
    {
      itmp *= strength_fp8;
      itmp = FP8_TO_FLOAT( itmp );
    };

  };


  return itmp;

}

//--------------------------------------------------------------------------------------------
void drop_money( CHR_REF character, Uint16 money )
{
  // ZZ> This function drops some of a character's money
  Uint16 huns, tfives, fives, ones, cnt;

  if ( money > chrmoney[character] )  money = chrmoney[character];

  if ( money > 0 && chrpos[character].z > -2 )
  {
    chrmoney[character] -= money;
    huns   = money / 100;  money -= huns   * 100;
    tfives = money /  25;  money -= tfives *  25;
    fives  = money /   5;  money -= fives  *   5;
    ones   = money;

    for ( cnt = 0; cnt < ones; cnt++ )
      spawn_one_particle( 1.0f, chrpos[character], 0, MAXMODEL, PRTPIP_COIN_001, MAXCHR, GRIP_LAST, TEAM_NULL, MAXCHR, cnt, MAXCHR );

    for ( cnt = 0; cnt < fives; cnt++ )
      spawn_one_particle( 1.0f, chrpos[character], 0, MAXMODEL, PRTPIP_COIN_005, MAXCHR, GRIP_LAST, TEAM_NULL, MAXCHR, cnt, MAXCHR );

    for ( cnt = 0; cnt < tfives; cnt++ )
      spawn_one_particle( 1.0f, chrpos[character], 0, MAXMODEL, PRTPIP_COIN_025, MAXCHR, GRIP_LAST, TEAM_NULL, MAXCHR, cnt, MAXCHR );

    for ( cnt = 0; cnt < huns; cnt++ )
      spawn_one_particle( 1.0f, chrpos[character], 0, MAXMODEL, PRTPIP_COIN_100, MAXCHR, GRIP_LAST, TEAM_NULL, MAXCHR, cnt, MAXCHR );

    chrdamagetime[character] = DELAY_DAMAGE;  // So it doesn't grab it again
  }
}

//--------------------------------------------------------------------------------------------
CHR_REF search_best_leader( TEAM team, CHR_REF exclude )
{
  // BB > find the best (most experienced) character other than the sissy to be a team leader
  CHR_REF cnt;
  Uint16  best_leader = MAXCHR;
  int     best_experience = 0;
  bool_t  bfound = bfalse;
  Uint16  exclude_sissy = team_get_sissy( team );

  for ( cnt = 0; cnt < MAXCHR; cnt++ )
  {
    if ( !VALID_CHR( cnt ) || cnt == exclude || cnt == exclude_sissy || chrteam[cnt] != team ) continue;

    if ( !bfound || chrexperience[cnt] > best_experience )
    {
      best_leader     = cnt;
      best_experience = chrexperience[cnt];
      bfound = btrue;
    }
  }

  return bfound ? best_leader : MAXCHR;
}

//--------------------------------------------------------------------------------------------
void call_for_help( CHR_REF character )
{
  // ZZ> This function issues a call for help to all allies
  TEAM team;
  Uint16 cnt;


  team = chrteam[character];


  // make the character in who needs help the sissy
  teamleader[team] = search_best_leader( team, character );
  teamsissy[team]  = character;


  // send the help message
  for ( cnt = 0; cnt < MAXCHR; cnt++ )
  {
    if ( !VALID_CHR( cnt ) || cnt == character ) continue;
    if ( !teamhatesteam[chrbaseteam[cnt]][team] )
    {
      chralert[cnt] |= ALERT_CALLEDFORHELP;
    };
  }

  // TODO: make a yelping sound

}

//--------------------------------------------------------------------------------------------
void give_experience( CHR_REF character, int amount, EXPERIENCE xptype )
{
  // ZZ> This function gives a character experience, and pawns off level gains to
  //     another function
  int newamount;
  int curlevel, nextexperience;
  int number;
  int profile;


  // Figure out how much experience to give
  profile = chrmodel[character];
  newamount = amount;
  if ( xptype < XP_COUNT )
  {
    newamount = amount * capexperiencerate[profile][xptype];
  }
  newamount += chrexperience[character];
  if ( newamount > MAXXP )  newamount = MAXXP;
  chrexperience[character] = newamount;


  // Do level ups and stat changes
  curlevel       = chrexperiencelevel[character];
  nextexperience = capexperiencecoeff[profile] * ( curlevel + 1 ) * ( curlevel + 1 ) + capexperienceconst[profile];
  while ( chrexperience[character] >= nextexperience )
  {
    // The character is ready to advance...
    if ( chrisplayer[character] )
    {
      debug_message( 1, "%s gained a level!!!", chrname[character] );

      snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.lvlup_sound );
      wave = Mix_LoadWAV( CStringTmp1 );

      Mix_PlayChannel( -1, wave, 0 );
    }
    chrexperiencelevel[character]++;

    // Size
    if (( chrsizegoto[character] + capsizeperlevel[profile] ) < 1 + ( capsizeperlevel[profile]*10 ) ) chrsizegoto[character] += capsizeperlevel[profile];
    chrsizegototime[character] += DELAY_RESIZE * 100;

    // Strength
    number = generate_unsigned( &capstrengthperlevel_fp8[profile] );
    number += chrstrength_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrstrength_fp8[character] = number;

    // Wisdom
    number = generate_unsigned( &capwisdomperlevel_fp8[profile] );
    number += chrwisdom_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrwisdom_fp8[character] = number;

    // Intelligence
    number = generate_unsigned( &capintelligenceperlevel_fp8[profile] );
    number += chrintelligence_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrintelligence_fp8[character] = number;

    // Dexterity
    number = generate_unsigned( &capdexterityperlevel_fp8[profile] );
    number += chrdexterity_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrdexterity_fp8[character] = number;

    // Life
    number = generate_unsigned( &caplifeperlevel_fp8[profile] );
    number += chrlifemax_fp8[character];
    if ( number > PERFECTBIG ) number = PERFECTBIG;
    chrlife_fp8[character] += ( number - chrlifemax_fp8[character] );
    chrlifemax_fp8[character] = number;

    // Mana
    number = generate_unsigned( &capmanaperlevel_fp8[profile] );
    number += chrmanamax_fp8[character];
    if ( number > PERFECTBIG ) number = PERFECTBIG;
    chrmana_fp8[character] += ( number - chrmanamax_fp8[character] );
    chrmanamax_fp8[character] = number;

    // Mana Return
    number = generate_unsigned( &capmanareturnperlevel_fp8[profile] );
    number += chrmanareturn_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrmanareturn_fp8[character] = number;

    // Mana Flow
    number = generate_unsigned( &capmanaflowperlevel_fp8[profile] );
    number += chrmanaflow_fp8[character];
    if ( number > PERFECTSTAT ) number = PERFECTSTAT;
    chrmanaflow_fp8[character] = number;

    curlevel       = chrexperiencelevel[character];
    nextexperience = capexperiencecoeff[profile] * ( curlevel + 1 ) * ( curlevel + 1 ) + capexperienceconst[profile];
  }
}


//--------------------------------------------------------------------------------------------
void give_team_experience( TEAM team, int amount, EXPERIENCE xptype )
{
  // ZZ> This function gives a character experience, and pawns off level gains to
  //     another function
  int cnt;

  for ( cnt = 0; cnt < MAXCHR; cnt++ )
    if ( chrteam[cnt] == team && chron[cnt] )
      give_experience( cnt, amount, xptype );
}


//--------------------------------------------------------------------------------------------
void setup_alliances( char *modname )
{
  // ZZ> This function reads the alliance file
  STRING newloadname, szTemp;
  TEAM teama, teamb;
  FILE *fileread;


  // Load the file
  snprintf( newloadname, sizeof( CStringTmp1 ), "%s%s/%s", modname, CData.gamedat_dir, CData.alliance_file );
  fileread = fs_fileOpen( PRI_NONE, NULL, newloadname, "r" );
  if ( NULL != fileread )
  {
    while ( fget_next_string( fileread, szTemp, sizeof( szTemp ) ) )
    {
      teama = ( szTemp[0] - 'A' ) % TEAM_COUNT;

      fget_string( fileread, szTemp, sizeof( szTemp ) );
      teamb = ( szTemp[0] - 'A' ) % TEAM_COUNT;

      teamhatesteam[teama][teamb] = bfalse;
    }
    fs_fileClose( fileread );
  }
}

//grfx.c

//--------------------------------------------------------------------------------------------
void check_respawn()
{
  int cnt;

  for ( cnt = 0; cnt < MAXCHR; cnt++ )
  {
    // Let players respawn
    if ( respawnvalid && !chralive[cnt] && HAS_SOME_BITS( chrlatchbutton[cnt], LATCHBUTTON_RESPAWN ) )
    {
      respawn_character( cnt );
      teamleader[chrteam[cnt]] = cnt;
      chralert[cnt] |= ALERT_CLEANEDUP;

      // Cost some experience for doing this...
      chrexperience[cnt] *= EXPKEEP;
    }
    chrlatchbutton[cnt] &= ~LATCHBUTTON_RESPAWN;
  }

};



//--------------------------------------------------------------------------------------------
void begin_integration()
{
  int cnt;

  for( cnt=0; cnt<MAXCHR; cnt++)
  {
    if( !chron[cnt] ) continue;
    
    VectorClear( chraccum_acc[cnt].v );
    VectorClear( chraccum_vel[cnt].v );
    VectorClear( chraccum_pos[cnt].v );
  };

  for( cnt=0; cnt<MAXPRT; cnt++)
  {
    if( !prton[cnt] ) continue;
    
    VectorClear( prtaccum_acc[cnt].v );
    VectorClear( prtaccum_vel[cnt].v );
    VectorClear( prtaccum_pos[cnt].v );
    prt_calculate_bumpers(cnt);
  };

};

//--------------------------------------------------------------------------------------------
bool_t chr_collide_mesh(CHR_REF ichr)
{
  float meshlevel;
  vect3 norm;
  bool_t hitmesh = bfalse;

  if( !VALID_CHR(ichr) ) return hitmesh;

  if ( 0 != __chrhitawall( ichr, &norm ) )
  {
    float dotprod = DotProduct(norm, chrvel[ichr]);

    if(dotprod < 0.0f)
    {
      // do the reflection
      chraccum_vel[ichr].x += -(1.0f + chrdampen[ichr]) * dotprod * norm.x - chrvel[ichr].x;
      chraccum_vel[ichr].y += -(1.0f + chrdampen[ichr]) * dotprod * norm.y - chrvel[ichr].y;
      chraccum_vel[ichr].z += -(1.0f + chrdampen[ichr]) * dotprod * norm.z - chrvel[ichr].z;

      // if there is reflection, go back to the last valid position
      if( 0.0f != norm.x ) chraccum_pos[ichr].x += chrpos_old[ichr].x - chrpos[ichr].x;
      if( 0.0f != norm.y ) chraccum_pos[ichr].y += chrpos_old[ichr].y - chrpos[ichr].y;
      if( 0.0f != norm.z ) chraccum_pos[ichr].z += chrpos_old[ichr].z - chrpos[ichr].z;

      hitmesh = btrue;
    };
  }

  meshlevel = mesh_get_level( chronwhichfan[ichr], chrpos[ichr].x, chrpos[ichr].y, capwaterwalk[chrmodel[ichr]] );
  if( chrpos[ichr].z < meshlevel )
  {
    hitmesh = btrue;

    chraccum_pos[ichr].z += meshlevel + 0.001f - chrpos[ichr].z;

    if ( chrvel[ichr].z < -STOPBOUNCING )
    {
      chraccum_vel[ichr].z += -chrvel[ichr].z * ( 1.0f + chrdampen[ichr] );
    }
    else if ( chrvel[ichr].z < STOPBOUNCING )
    {
      chraccum_vel[ichr].z += -chrvel[ichr].z;
    }

    if ( chrhitready[ichr] )
    {
      chralert[ichr] |= ALERT_HITGROUND;
      chrhitready[ichr] = bfalse;
    };
  }

  return hitmesh;
};

//--------------------------------------------------------------------------------------------
bool_t prt_collide_mesh(PRT_REF iprt)
{
  float meshlevel, dampen;
  CHR_REF attached;
  vect3 norm;
  bool_t hitmesh = bfalse;
  Uint16 pip;

  if( !VALID_PRT(iprt) ) return hitmesh;

  
  attached = prt_get_attachedtochr(iprt);
  pip      = prtpip[iprt];
  dampen   = pipdampen[pip];

  if ( 0 != __prthitawall( iprt, &norm ) )
  {
    float dotprod = DotProduct(norm, prtvel[iprt]);

    if(dotprod < 0.0f)
    {
      play_particle_sound( prtbumpstrength[iprt]*MIN( 1.0f, prtvel[iprt].x / 10.0f ), iprt, pipsoundwall[pip] );

      if ( pipendwall[pip] )
      {
        prtgopoof[iprt] = btrue;
      }
      else if( !piprotatetoface[pip] && !VALID_CHR(attached) )  // "rotate to face" gets it's own treatment
      {
        vect3 old_vel;
        float dotprodN;
        float dampen = pipdampen[prtpip[iprt]];

        old_vel = prtvel[iprt];

        // do the reflection
        dotprodN = DotProduct(norm, prtvel[iprt]);
        prtaccum_vel[iprt].x += -(1.0f + dampen) * dotprodN * norm.x - prtvel[iprt].x;
        prtaccum_vel[iprt].y += -(1.0f + dampen) * dotprodN * norm.y - prtvel[iprt].y;
        prtaccum_vel[iprt].z += -(1.0f + dampen) * dotprodN * norm.z - prtvel[iprt].z;

        // Change facing
        // determine how much the billboarded particle should rotate on reflection from
        // the rotation of the velocity vector about the vector to the camera
        {
          vect3 vec_out, vec_up, vec_right, wld_up;
          float old_vx, old_vy, new_vx, new_vy;
          
          Uint16 new_turn, old_turn;

          vec_out.x = prtpos[iprt].x - campos.x;
          vec_out.y = prtpos[iprt].y - campos.y;
          vec_out.z = prtpos[iprt].z - campos.z;

          wld_up.x = 0;
          wld_up.y = 0;
          wld_up.z = 1;

          vec_right = Normalize( CrossProduct( wld_up, vec_out ) );
          vec_up    = Normalize( CrossProduct( vec_right, vec_out ) );

          old_vx = DotProduct(old_vel, vec_right);
          old_vy = DotProduct(old_vel, vec_up);
          old_turn = vec_to_turn(old_vx, old_vy);

          new_vx = DotProduct(prtvel[iprt], vec_right);
          new_vy = DotProduct(prtvel[iprt], vec_up);
          new_turn = vec_to_turn(new_vx, new_vy);

          prtfacing[iprt] += new_turn - old_turn;
        }
      }

      // if there is reflection, go back to the last valid position
      if( 0.0f != norm.x ) prtaccum_pos[iprt].x += prtpos_old[iprt].x - prtpos[iprt].x;
      if( 0.0f != norm.y ) prtaccum_pos[iprt].y += prtpos_old[iprt].y - prtpos[iprt].y;
      if( 0.0f != norm.z ) prtaccum_pos[iprt].z += prtpos_old[iprt].z - prtpos[iprt].z;

      hitmesh = btrue;
    };
  }

  meshlevel = mesh_get_level( prtonwhichfan[iprt], prtpos[iprt].x, prtpos[iprt].y, capwaterwalk[prtmodel[iprt]] );
  if( prtpos[iprt].z < meshlevel )
  {
    hitmesh = btrue;

    if(prtvel[iprt].z < 0.0f)
    {
      play_particle_sound( MIN( 1.0f, -prtvel[iprt].z / 10.0f ), iprt, pipsoundfloor[pip] );
    };

    if( pipendground[pip] )
    {
      prtgopoof[iprt] = btrue;
    }
    else if( !VALID_CHR( attached ) )
    {
      prtaccum_pos[iprt].z += meshlevel + 0.001f - prtpos[iprt].z;

      if ( prtvel[iprt].z < -STOPBOUNCING )
      {
        prtaccum_vel[iprt].z -= prtvel[iprt].z * ( 1.0f + dampen );
      }
      else if ( prtvel[iprt].z < STOPBOUNCING )
      {
        prtaccum_vel[iprt].z -= prtvel[iprt].z;
      }
    };

  }

  return hitmesh;
};



//--------------------------------------------------------------------------------------------
void do_integration(float dFrame)
{
  int cnt, tnc;

  for( cnt=0; cnt<MAXCHR; cnt++)
  {
    if( !chron[cnt] ) continue;

    chrpos_old[cnt] = chrpos[cnt];

    chrpos[cnt].x += chrvel[cnt].x * dFrame + chraccum_pos[cnt].x;
    chrpos[cnt].y += chrvel[cnt].y * dFrame + chraccum_pos[cnt].y;
    chrpos[cnt].z += chrvel[cnt].z * dFrame + chraccum_pos[cnt].z;

    chrvel[cnt].x += chraccum_acc[cnt].x * dFrame + chraccum_vel[cnt].x;
    chrvel[cnt].y += chraccum_acc[cnt].y * dFrame + chraccum_vel[cnt].y;
    chrvel[cnt].z += chraccum_acc[cnt].z * dFrame + chraccum_vel[cnt].z;

    VectorClear( chraccum_acc[cnt].v );
    VectorClear( chraccum_vel[cnt].v );
    VectorClear( chraccum_pos[cnt].v );

    // iterate through the integration routine until you force the new position to be valid
    // should only ever go through the loop twice
    tnc = 0;
    while( tnc < 20 && chr_collide_mesh(cnt) )
    {
      chrpos[cnt].x += chraccum_pos[cnt].x;
      chrpos[cnt].y += chraccum_pos[cnt].y;
      chrpos[cnt].z += chraccum_pos[cnt].z;

      chrvel[cnt].x += chraccum_vel[cnt].x;
      chrvel[cnt].y += chraccum_vel[cnt].y;
      chrvel[cnt].z += chraccum_vel[cnt].z;

      VectorClear( chraccum_acc[cnt].v );
      VectorClear( chraccum_vel[cnt].v );
      VectorClear( chraccum_pos[cnt].v );

      tnc++;
    };
  };

  for( cnt=0; cnt<MAXPRT; cnt++)
  {
    if( !prton[cnt] ) continue;

    prtpos_old[cnt] = prtpos[cnt];

    prtpos[cnt].x += prtvel[cnt].x * dFrame + prtaccum_pos[cnt].x;
    prtpos[cnt].y += prtvel[cnt].y * dFrame + prtaccum_pos[cnt].y;
    prtpos[cnt].z += prtvel[cnt].z * dFrame + prtaccum_pos[cnt].z;

    prtvel[cnt].x += prtaccum_acc[cnt].x * dFrame + prtaccum_vel[cnt].x;
    prtvel[cnt].y += prtaccum_acc[cnt].y * dFrame + prtaccum_vel[cnt].y;
    prtvel[cnt].z += prtaccum_acc[cnt].z * dFrame + prtaccum_vel[cnt].z;

    // iterate through the integration routine until you force the new position to be valid
    // should only ever go through the loop twice
    tnc = 0;
    while( tnc < 20 && prt_collide_mesh(cnt) )
    {
      prtpos[cnt].x += prtaccum_pos[cnt].x;
      prtpos[cnt].y += prtaccum_pos[cnt].y;
      prtpos[cnt].z += prtaccum_pos[cnt].z;

      prtvel[cnt].x += prtaccum_vel[cnt].x;
      prtvel[cnt].y += prtaccum_vel[cnt].y;
      prtvel[cnt].z += prtaccum_vel[cnt].z;

      VectorClear( prtaccum_acc[cnt].v );
      VectorClear( prtaccum_vel[cnt].v );
      VectorClear( prtaccum_pos[cnt].v );

      tnc++;
    }
  };

};

//--------------------------------------------------------------------------------------------
void update_game( float dUpdate )
{
  // ZZ> This function does several iterations of character movements and such
  //     to keep the game in sync.
  int    cnt, numdead;

  // Check for all local players being dead
  alllocalpladead = bfalse;
  somelocalpladead = bfalse;
  localseeinvisible = bfalse;
  localseekurse = bfalse;
  numdead = 0;
  for ( cnt = 0; cnt < MAXPLAYER; cnt++ )
  {
    CHR_REF ichr;
    if ( !VALID_PLA( cnt ) || INBITS_NONE == pladevice[cnt] ) continue;

    ichr = pla_get_character( cnt );
    if ( !VALID_CHR( ichr ) ) continue;

    if ( !chralive[ichr] && chrislocalplayer[ichr] )
    {
      numdead++;
    };

    if ( chrcanseeinvisible[ichr] )
    {
      localseeinvisible = btrue;
    }

    if ( chrcanseekurse[ichr] )
    {
      localseekurse = btrue;
    }

  };

  somelocalpladead = ( numdead > 0 );
  alllocalpladead  = ( numdead >= numlocalpla );

  // This is the main game loop
  msgtimechange = 0;

  // [claforte Jan 6th 2001]
  // TODO: Put that back in place once networking is functional.
  // Important stuff to keep in sync
  while (( wldclock < allclock ) && ( AClientState.numplatimes > 0 ) )
  {
    srand( randsave );

    PROFILE_BEGIN( resize_characters );
    resize_characters( dUpdate );
    PROFILE_END( resize_characters );

    PROFILE_BEGIN( keep_weapons_with_holders );
    keep_weapons_with_holders();
    PROFILE_END( keep_weapons_with_holders );

    PROFILE_BEGIN( let_ai_think );
    let_ai_think( dUpdate );
    PROFILE_END( let_ai_think );

    PROFILE_BEGIN( do_weather_spawn );
    do_weather_spawn( dUpdate );
    PROFILE_END( do_weather_spawn );

    PROFILE_BEGIN( do_enchant_spawn );
    do_enchant_spawn( dUpdate );
    PROFILE_END( do_enchant_spawn );

    // unbuffer the updated latches after let_ai_think() and before move_characters()
    PROFILE_BEGIN( cl_unbufferLatches );
    cl_unbufferLatches( &AClientState );
    PROFILE_END( cl_unbufferLatches );

    PROFILE_BEGIN( sv_unbufferLatches );
    sv_unbufferLatches( &AServerState );
    PROFILE_END( sv_unbufferLatches );


    PROFILE_BEGIN( check_respawn );
    check_respawn();
    PROFILE_END( check_respawn );

    despawn_characters();
    despawn_particles();

    PROFILE_BEGIN( make_onwhichfan );
    make_onwhichfan();
    PROFILE_END( make_onwhichfan );
    
    begin_integration();
    {
      PROFILE_BEGIN( move_characters );
      move_characters( dUpdate );
      PROFILE_END( move_characters );

      PROFILE_BEGIN( move_particles );
      move_particles( dUpdate );
      PROFILE_END( move_particles );

      PROFILE_BEGIN( attach_particles );
      attach_particles();
      PROFILE_END( attach_particles );

      PROFILE_BEGIN( do_bumping );
      do_bumping( dUpdate );
      PROFILE_END( do_bumping );

    }
    do_integration(dUpdate);

    PROFILE_BEGIN( make_character_matrices );
    make_character_matrices();
    PROFILE_END( make_character_matrices );


    PROFILE_BEGIN( stat_return );
    stat_return( dUpdate );
    PROFILE_END( stat_return );

    PROFILE_BEGIN( pit_kill );
    pit_kill( dUpdate );
    PROFILE_END( pit_kill );

    // Generate the new seed
    randsave += * (( Uint32* ) & kMd2Normals[wldframe&127][0] );
    randsave += * (( Uint32* ) & kMd2Normals[randsave&127][1] );

    // Stuff for which sync doesn't matter
    PROFILE_BEGIN( animate_tiles );
    animate_tiles( dUpdate );
    PROFILE_END( animate_tiles );

    PROFILE_BEGIN( move_water );
    move_water( dUpdate );
    PROFILE_END( move_water );

    // Timers
    wldclock += UPDATESKIP;
    wldframe++;
    msgtimechange++;
    if ( statdelay > 0 )  statdelay--;
    statclock++;
  }

  {
    if ( AClientState.numplatimes == 0 )
    {
      // The remote ran out of messages, and is now twiddling its thumbs...
      // Make it go slower so it doesn't happen again
      wldclock += UPDATESKIP / 4.0f;
    }
    else if ( !hostactive && AClientState.numplatimes > 3 )
    {
      // The host has too many messages, and is probably experiencing control
      // CData.lag...  Speed it up so it gets closer to sync
      wldclock -= UPDATESKIP / 4.0f;
    }
  }
}

//--------------------------------------------------------------------------------------------
void update_timers()
{
  // ZZ> This function updates the game timers
  lstclock = allclock;

  if(game_single_frame)
  {
    if(game_do_frame) allclock = wldclock + UPDATESKIP;
  }
  else
  {
    allclock = SDL_GetTicks() - sttclock;
  }

  ups_clock += allclock - lstclock;
  fps_clock += allclock - lstclock;

  if ( ups_loops > 0 && ups_clock > 0)
  {
    stabilized_ups_sum        = stabilized_ups_sum * 0.99 + 0.01 * ( float ) ups_loops / (( float ) ups_clock / TICKS_PER_SEC );
    stabilized_ups_weight = stabilized_ups_weight * 0.99 + 0.01;
  };

  if ( fps_loops > 0 && fps_clock > 0 )
  {
    stabilized_fps_sum        = stabilized_fps_sum * 0.99 + 0.01 * ( float ) fps_loops / (( float ) fps_clock / TICKS_PER_SEC );
    stabilized_fps_weight = stabilized_fps_weight * 0.99 + 0.01;
  };

  if ( ups_clock >= TICKS_PER_SEC )
  {
    ups_clock = 0;
    ups_loops = 0;
    assert(stabilized_ups_weight>0);
    stabilized_ups = stabilized_ups_sum / stabilized_ups_weight;
  }

  if ( fps_clock >= TICKS_PER_SEC )
  {
    fps_clock = 0;
    fps_loops = 0;
    assert(stabilized_ups_weight>0);
    stabilized_fps = stabilized_fps_sum / stabilized_fps_weight;
  }

}




//--------------------------------------------------------------------------------------------
void reset_teams()
{
  // ZZ> This function makes everyone hate everyone else
  int teama, teamb;


  teama = 0;
  while ( teama < TEAM_COUNT )
  {
    // Make the team hate everyone
    teamb = 0;
    while ( teamb < TEAM_COUNT )
    {
      teamhatesteam[teama][teamb] = btrue;
      teamb++;
    }
    // Make the team like itself
    teamhatesteam[teama][teama] = bfalse;

    // Set defaults
    teamleader[teama] = MAXCHR;
    teamsissy[teama] = 0;
    teammorale[teama] = 0;
    teama++;
  }


  // Keep the null team neutral
  teama = 0;
  while ( teama < TEAM_COUNT )
  {
    teamhatesteam[teama][TEAM_NULL] = bfalse;
    teamhatesteam[TEAM_NULL][teama] = bfalse;
    teama++;
  }
}

//--------------------------------------------------------------------------------------------
void reset_messages()
{
  // ZZ> This makes messages safe to use
  int cnt;

  msgtotal = 0;
  msgtotalindex = 0;
  msgtimechange = 0;
  msgstart = 0;
  cnt = 0;
  while ( cnt < MAXMESSAGE )
  {
    msgtime[cnt] = 0;
    cnt++;
  }
  cnt = 0;
  while ( cnt < MAXTOTALMESSAGE )
  {
    msgindex[cnt] = 0;
    cnt++;
  }
  msgtext[0] = 0;
}

//--------------------------------------------------------------------------------------------
void make_randie()
{
  // ZZ> This function makes the random number table
  int tnc, cnt;


  // Fill in the basic values
  cnt = 0;
  while ( cnt < MAXRAND )
  {
    randie[cnt] = rand() << 1;
    cnt++;
  }


  // Keep adjusting those values
  tnc = 0;
  while ( tnc < 20 )
  {
    cnt = 0;
    while ( cnt < MAXRAND )
    {
      randie[cnt] += ( Uint16 ) rand();
      cnt++;
    }
    tnc++;
  }

  // All done
  randindex = 0;
}

//--------------------------------------------------------------------------------------------
void reset_timers()
{
  // ZZ> This function resets the timers...
  sttclock = SDL_GetTicks();
  allclock = 0;
  lstclock = 0;
  wldclock = 0;
  statclock = 0;
  pitclock = 0;  pitskill = bfalse;
  wldframe = 0;
  allframe = 0;
  outofsync = bfalse;

  ups_loops = 0;
  ups_clock = 0;
  fps_loops = 0;
  fps_clock = 0;
}

extern int mnu_Run( float deltaTime );
extern int initMenus();

//--------------------------------------------------------------------------------------------
void set_default_config_data(CONFIG_DATA * pcon)
{
  if(NULL==pcon) return;

  strncpy( pcon->basicdat_dir, "basicdat", sizeof( STRING ) );
  strncpy( pcon->gamedat_dir, "gamedat" , sizeof( STRING ) );
  strncpy( pcon->mnu_dir, "menu" , sizeof( STRING ) );
  strncpy( pcon->globalparticles_dir, "globalparticles" , sizeof( STRING ) );
  strncpy( pcon->modules_dir, "modules" , sizeof( STRING ) );
  strncpy( pcon->music_dir, "music" , sizeof( STRING ) );
  strncpy( pcon->objects_dir, "objects" , sizeof( STRING ) );
  strncpy( pcon->import_dir, "import" , sizeof( STRING ) );
  strncpy( pcon->players_dir, "players", sizeof( STRING ) );

  strncpy( pcon->nullicon_bitmap, "nullicon.bmp" , sizeof( STRING ) );
  strncpy( pcon->keybicon_bitmap, "keybicon.bmp" , sizeof( STRING ) );
  strncpy( pcon->mousicon_bitmap, "mousicon.bmp" , sizeof( STRING ) );
  strncpy( pcon->joyaicon_bitmap, "joyaicon.bmp" , sizeof( STRING ) );
  strncpy( pcon->joybicon_bitmap, "joybicon.bmp" , sizeof( STRING ) );

  strncpy( pcon->tile0_bitmap, "tile0.bmp" , sizeof( STRING ) );
  strncpy( pcon->tile1_bitmap, "tile1.bmp" , sizeof( STRING ) );
  strncpy( pcon->tile2_bitmap, "tile2.bmp" , sizeof( STRING ) );
  strncpy( pcon->tile3_bitmap, "tile3.bmp" , sizeof( STRING ) );
  strncpy( pcon->watertop_bitmap, "watertop.bmp" , sizeof( STRING ) );
  strncpy( pcon->waterlow_bitmap, "waterlow.bmp" , sizeof( STRING ) );
  strncpy( pcon->phong_bitmap, "phong.bmp" , sizeof( STRING ) );
  strncpy( pcon->plan_bitmap, "plan.bmp" , sizeof( STRING ) );
  strncpy( pcon->blip_bitmap, "blip9.png" , sizeof( STRING ) );
  strncpy( pcon->font_bitmap, "font.png" , sizeof( STRING ) );
  strncpy( pcon->icon_bitmap, "icon.bmp" , sizeof( STRING ) );
  strncpy( pcon->bars_bitmap, "bars.png" , sizeof( STRING ) );
  strncpy( pcon->particle_bitmap, "particle_normal.png" , sizeof( STRING ) );
  strncpy( pcon->title_bitmap, "title.bmp" , sizeof( STRING ) );

  strncpy( pcon->menu_main_bitmap, "menu_main.png" , sizeof( STRING ) );
  strncpy( pcon->menu_advent_bitmap, "menu_advent.png" , sizeof( STRING ) );
  strncpy( pcon->menu_sleepy_bitmap, "menu_sleepy.png" , sizeof( STRING ) );
  strncpy( pcon->menu_gnome_bitmap, "menu_gnome.png" , sizeof( STRING ) );


  strncpy( pcon->slotused_file, "slotused.txt" , sizeof( STRING ) );
  strncpy( pcon->passage_file, "passage.txt" , sizeof( STRING ) );
  strncpy( pcon->aicodes_file, "aicodes.txt" , sizeof( STRING ) );
  strncpy( pcon->actions_file, "actions.txt" , sizeof( STRING ) );
  strncpy( pcon->alliance_file, "alliance.txt" , sizeof( STRING ) );
  strncpy( pcon->fans_file, "fans.txt" , sizeof( STRING ) );
  strncpy( pcon->fontdef_file, "font.txt" , sizeof( STRING ) );
  strncpy( pcon->mnu_file, "menu.txt" , sizeof( STRING ) );
  strncpy( pcon->money1_file, "1money.txt" , sizeof( STRING ) );
  strncpy( pcon->money5_file, "5money.txt" , sizeof( STRING ) );
  strncpy( pcon->money25_file, "25money.txt" , sizeof( STRING ) );
  strncpy( pcon->money100_file, "100money.txt" , sizeof( STRING ) );
  strncpy( pcon->weather4_file, "weather4.txt" , sizeof( STRING ) );
  strncpy( pcon->weather5_file, "weather5.txt" , sizeof( STRING ) );
  strncpy( pcon->script_file, "script.txt" , sizeof( STRING ) );
  strncpy( pcon->ripple_file, "ripple.txt" , sizeof( STRING ) );
  strncpy( pcon->scancode_file, "scancode.txt" , sizeof( STRING ) );
  strncpy( pcon->playlist_file, "playlist.txt" , sizeof( STRING ) );
  strncpy( pcon->spawn_file, "spawn.txt" , sizeof( STRING ) );
  strncpy( pcon->wawalite_file, "wawalite.txt" , sizeof( STRING ) );
  strncpy( pcon->defend_file, "defend.txt" , sizeof( STRING ) );
  strncpy( pcon->splash_file, "splash.txt" , sizeof( STRING ) );
  strncpy( pcon->mesh_file, "level.mpd" , sizeof( STRING ) );
  strncpy( pcon->setup_file, "setup.txt" , sizeof( STRING ) );
  strncpy( pcon->log_file, "log.txt", sizeof( STRING ) );
  strncpy( pcon->controls_file, "controls.txt", sizeof( STRING ) );
  strncpy( pcon->data_file, "data.txt", sizeof( STRING ) );
  strncpy( pcon->copy_file, "copy.txt", sizeof( STRING ) );
  strncpy( pcon->enchant_file, "enchant.txt", sizeof( STRING ) );
  strncpy( pcon->message_file, "message.txt", sizeof( STRING ) );
  strncpy( pcon->naming_file, "naming.txt", sizeof( STRING ) );
  strncpy( pcon->modules_file, "modules.txt", sizeof( STRING ) );
  strncpy( pcon->setup_file, "setup.txt", sizeof( STRING ) );
  strncpy( pcon->skin_file, "skin.txt", sizeof( STRING ) );
  strncpy( pcon->credits_file, "credits.txt", sizeof( STRING ) );
  strncpy( pcon->quest_file, "quest.txt", sizeof( STRING ) );


  pcon->uifont_points  = 20;
  pcon->uifont_points2 = 18;
  strncpy( pcon->uifont_ttf, "Negatori.ttf" , sizeof( STRING ) );

  strncpy( pcon->coinget_sound, "coinget.wav" , sizeof( STRING ) );
  strncpy( pcon->defend_sound, "defend.wav" , sizeof( STRING ) );
  strncpy( pcon->coinfall_sound, "coinfall.wav" , sizeof( STRING ) );
  strncpy( pcon->lvlup_sound, "lvlup.wav" , sizeof( STRING ) );

  pcon->fullscreen = bfalse;
  pcon->zreflect = bfalse;
  pcon->maxtotalmeshvertices = 256 * 256 * 6;
  pcon->scrd = 8;                   // Screen bit depth
  pcon->scrx = 320;                 // Screen X size
  pcon->scry = 200;                 // Screen Y size
  pcon->scrz = 16;                  // Screen z-buffer depth ( 8 unsupported )
  pcon->maxmessage = MAXMESSAGE;    //
  pcon->messageon  = btrue;           // Messages?
  pcon->wraptolerance = 80;     // Status bar
  pcon->staton = btrue;                 // Draw the status bars?
  pcon->render_overlay = bfalse;
  pcon->render_background = bfalse;
  pcon->perspective = GL_FASTEST;
  pcon->dither = bfalse;
  pcon->shading = GL_FLAT;
  pcon->antialiasing = bfalse;
  pcon->refon = bfalse;
  pcon->shaon = bfalse;
  pcon->texturefilter = TX_LINEAR;
  pcon->wateron = btrue;
  pcon->shasprite = bfalse;
  pcon->phongon = btrue;                // Do phong overlay? OUTDATED?
  pcon->zreflect = bfalse;
  pcon->reffadeor = 255;              // 255 = Don't fade reflections
  pcon->twolayerwateron = bfalse;        // Two layer water?
  pcon->overlayvalid = bfalse;               // Allow large overlay?
  pcon->backgroundvalid = bfalse;            // Allow large background?
  pcon->fogallowed = btrue;          //
  pcon->particletype = PART_NORMAL;
  pcon->vsync = bfalse;
  pcon->gfxacceleration = btrue;
  pcon->soundvalid = bfalse;     //Allow playing of sound?
  pcon->musicvalid = bfalse;     // Allow music and loops?
  pcon->networkon  = btrue;              // Try to connect?
  pcon->lag        = 3;                                // Lag tolerance
  strncpy( pcon->nethostname, "no host", sizeof( pcon->nethostname ) );                        // Name for hosting session
  strncpy( pcon->netmessagename, "little Raoul", sizeof( pcon->netmessagename ) );             // Name for messages
  pcon->fpson = btrue;               // FPS displayed?
  // Debug option
  pcon->GrabMouse = SDL_GRAB_ON;
  pcon->HideMouse = bfalse;
  pcon->DevMode  = btrue;
  // Debug option
};

//--------------------------------------------------------------------------------------------
#undef DEBUG_UPDATE_CLAMP

typedef enum proc_states_e
{
  PROC_Begin,
  PROC_Entering,
  PROC_Running,
  PROC_Leaving,
  PROC_Finish,
} ProcessStates;

int proc_program( int argc, char **argv );
int proc_gameLoop( double frameDuration, bool_t cleanup );
int proc_menuLoop( double frameDuration, bool_t cleanup );

int proc_program( int argc, char **argv )
{
  // ZZ> This is where the program starts and all the high level stuff happens

  int retval = 0;
  static ProcessStates procState = PROC_Begin;
  static double frameDuration;
  static bool_t menuActive = btrue;
  static bool_t menuTerminated = bfalse;
  static bool_t gameTerminated = bfalse;
  static float  updateTimer = 0;


  switch ( procState )
  {
    case PROC_Begin:
      {

        // use CData_default gives the default values for CData
        // when we read the CData values from a file, the CData_default will be used
        // for any non-existant tags
        set_default_config_data(&CData_default);
        memcpy(&CData, &CData_default, sizeof(CONFIG_DATA));

        // Initialize logging first, so that we can use it everywhere.
        log_init();
        log_setLoggingLevel( 2 );

        // start initializing the various subsystems
        log_message( "Starting Egoboo %s...\n", VERSION );

        // initialize system dependent services
        sys_initialize();

        // initialize the clock
        g_clk_state = clock_create_state();
        clock_init( g_clk_state );
        fs_init();

        read_setup( CData.setup_file );

        snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.scancode_file );
        read_all_tags( CStringTmp1 );

        read_controls( CData.controls_file );
        reset_ai_script();

        snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.aicodes_file );
        load_ai_codes( CStringTmp1 );

        snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.actions_file );
        load_action_names( CStringTmp1 );

        // initialize the SDL and OpenGL
        sdlinit( argc, argv );
        glinit( argc, argv );

        // initialize the network
        // Network's temporarily disabled
        net_initialize();

        // initialize the sound system
        mixeron = sdlmixer_initialize();
        load_all_music_sounds();

        // allocate the maximum amount of mesh memory
        if ( !get_mesh_memory() )
        {
          log_error( "Unable to initialize Mesh Memory - Reduce the maximum number of vertices (See SETUP.TXT)\n" );
          return bfalse;
        }
        load_mesh_fans();

        // set up the MD2 models
        init_all_models();

        // Make lookup tables
        make_textureoffset();
        make_lightdirectionlookup();
        make_turntosin();
        make_enviro();

        //DEBUG
        CData.scrd = 32;    //We need to force 32 bit, since SDL_image crashes without it
        //DEBUG END

        gameActive     = bfalse;
        gameTerminated = btrue;

        menuActive     = btrue;
        menuTerminated = btrue;
      }
      procState = PROC_Entering;
      break;

    case PROC_Entering:
      {
        PROFILE_INIT( resize_characters );
        PROFILE_INIT( keep_weapons_with_holders );
        PROFILE_INIT( let_ai_think );
        PROFILE_INIT( do_weather_spawn );
        PROFILE_INIT( do_enchant_spawn );
        PROFILE_INIT( cl_unbufferLatches );
        PROFILE_INIT( sv_unbufferLatches );
        PROFILE_INIT( check_respawn );
        PROFILE_INIT( move_characters );
        PROFILE_INIT( move_particles );
        PROFILE_INIT( make_character_matrices );
        PROFILE_INIT( attach_particles );
        PROFILE_INIT( make_onwhichfan );
        PROFILE_INIT( do_bumping );
        PROFILE_INIT( stat_return );
        PROFILE_INIT( pit_kill );
        PROFILE_INIT( animate_tiles );
        PROFILE_INIT( move_water );
        PROFILE_INIT( figure_out_what_to_draw );
        PROFILE_INIT( draw_main );

        clock_frameStep( g_clk_state );
        frameDuration = clock_getFrameDuration( g_clk_state );

        if( menuActive )
        {
          mnu_enterMenuMode();
          play_music( 0, 500, -1 ); //Play the menu music
        };
      }
      procState = PROC_Running;
      break;

    case PROC_Running:
      if ( !gameActive && !menuActive )
      {
        procState = PROC_Leaving;
      }
      else
      {
        // Clock updates each frame
        clock_frameStep( g_clk_state );
        frameDuration = clock_getFrameDuration( g_clk_state );

        updateTimer += frameDuration;

        // read the input
        read_input();

        // blank the screen, if required
        do_clear();

        // Do the game, if it is active
        if( updateTimer * TICKS_PER_SEC > UPDATESKIP )
        {
          // all these procedures must use the same timer so that their 
          // frames line up

          // update the game, if active
          if ( gameActive )
          {
            int proc_value = proc_gameLoop( updateTimer, bfalse );
            gameTerminated = bfalse;

            switch ( proc_value )
            {
              case - 1:
                gameActive = bfalse; 
                gameTerminated = btrue; 

                // restart the menu
                menuActive = btrue;
                mnu_enterMenuMode();
                play_music( 0, 500, -1 ); //Play the menu music
                break;
            };
          }

          // Do the in-game menu, if it is active
          if ( ingameMenuActive )
          {
            ui_beginFrame( updateTimer );
            {
              int menuResult = mnu_RunIngame(( float ) updateTimer );
              switch ( menuResult )
              {
                case 1: /* nothing */
                  break;

                case - 1:
                  // The user selected "Quit"
                  ingameMenuActive = bfalse;
                  mnu_exitMenuMode();
                  break;
              }

            }
            ui_endFrame();

            request_pageflip();
          }

          // Do the menu, if it is active
          if ( menuActive )
          {
            int proc_value = proc_menuLoop( updateTimer, bfalse );
            menuTerminated = bfalse;

            switch ( proc_value )
            {
              case  1:
                gameActive = btrue;
                menuActive = bfalse;
                mnu_exitMenuMode();
                break;

              case - 1:
                menuActive = bfalse;
                menuTerminated = btrue;
                mnu_exitMenuMode();
                break;
            }
          }

          updateTimer = 0;
        }

        // do pageflip, if required
        do_pageflip();

        //Pressed panic button
        if ( SDLKEYDOWN( SDLK_q ) && SDLKEYDOWN( SDLK_LCTRL ) )
        {
          log_info( "User pressed escape button (LCTRL+Q)... Quitting game gracefully.\n" );
          procState = PROC_Leaving;
        }


        // let the OS breathe
        {
          int ms_leftover = MAX(0, UPDATESKIP - frameDuration * TICKS_PER_SEC);
          if(ms_leftover / 4 > 0)
          {
            SDL_Delay( ms_leftover / 4 );
          }
        }
      }

      break;

    case PROC_Leaving:
      {
        // request that the menu loop to clean itself.
        if ( !menuTerminated )
        {
          int proc_value = proc_menuLoop( frameDuration, btrue );
          switch ( proc_value )
          {
            case - 1: menuActive = bfalse; menuTerminated = btrue; break;
          };
        };

        // force the main loop to clean itself
        if ( !gameTerminated )
        {
          int proc_value = proc_gameLoop( frameDuration, btrue );
          switch ( proc_value )
          {
            case - 1: gameActive = bfalse; gameTerminated = btrue; break;
          };
        };
      }

      if ( gameTerminated && menuTerminated ) procState = PROC_Finish;
      break;

    case PROC_Finish:
      {
        log_info( "============ PROFILE =================\n" );
        log_info( "\tresize_characters - %lf\n", 1e6*PROFILE_QUERY( resize_characters ) );
        log_info( "\tkeep_weapons_with_holders - %lf\n", 1e6*PROFILE_QUERY( keep_weapons_with_holders ) );
        log_info( "\tlet_ai_think - %lf\n", 1e6*PROFILE_QUERY( let_ai_think ) );
        log_info( "\tdo_weather_spawn - %lf\n", 1e6*PROFILE_QUERY( do_weather_spawn ) );
        log_info( "\tdo_enchant_spawn - %lf\n", 1e6*PROFILE_QUERY( do_enchant_spawn ) );
        log_info( "\tcl_unbufferLatches - %lf\n", 1e6*PROFILE_QUERY( cl_unbufferLatches ) );
        log_info( "\tsv_unbufferLatches - %lf\n", 1e6*PROFILE_QUERY( sv_unbufferLatches ) );
        log_info( "\tcheck_respawn - %lf\n", 1e6*PROFILE_QUERY( check_respawn ) );
        log_info( "\tmove_characters - %lf\n", 1e6*PROFILE_QUERY( move_characters ) );
        log_info( "\tmove_particles - %lf\n", 1e6*PROFILE_QUERY( move_particles ) );
        log_info( "\tmake_character_matrices - %lf\n", 1e6*PROFILE_QUERY( make_character_matrices ) );
        log_info( "\tattach_particles - %lf\n", 1e6*PROFILE_QUERY( attach_particles ) );
        log_info( "\tmake_onwhichfan - %lf\n", 1e6*PROFILE_QUERY( make_onwhichfan ) );
        log_info( "\tbump_characters - %lf\n", 1e6*PROFILE_QUERY( do_bumping ) );
        log_info( "\tstat_return - %lf\n", 1e6*PROFILE_QUERY( stat_return ) );
        log_info( "\tpit_kill - %lf\n", 1e6*PROFILE_QUERY( pit_kill ) );
        log_info( "\tanimate_tiles - %lf\n", 1e6*PROFILE_QUERY( animate_tiles ) );
        log_info( "\tmove_water - %lf\n", 1e6*PROFILE_QUERY( move_water ) );
        log_info( "======================================\n" );
        log_info( "\tfigure_out_what_to_draw - %lf\n", 1e6*PROFILE_QUERY( figure_out_what_to_draw ) );
        log_info( "\tdraw_main - %lf\n", 1e6*PROFILE_QUERY( draw_main ) );
        log_info( "======================================\n\n" );

        PROFILE_FREE( resize_characters );
        PROFILE_FREE( keep_weapons_with_holders );
        PROFILE_FREE( let_ai_think );
        PROFILE_FREE( do_weather_spawn );
        PROFILE_FREE( do_enchant_spawn );
        PROFILE_FREE( cl_unbufferLatches );
        PROFILE_FREE( sv_unbufferLatches );
        PROFILE_FREE( check_respawn );
        PROFILE_FREE( move_characters );
        PROFILE_FREE( move_particles );
        PROFILE_FREE( make_character_matrices );
        PROFILE_FREE( attach_particles );
        PROFILE_FREE( make_onwhichfan );
        PROFILE_FREE( do_bumping );
        PROFILE_FREE( stat_return );
        PROFILE_FREE( pit_kill );
        PROFILE_FREE( animate_tiles );
        PROFILE_FREE( move_water );

        quit_game();
        if ( !memory_cleanUp() ) log_message( "WARNING! COULD NOT CLEAN MEMORY AND SHUTDOWN SUPPORT SYSTEMS!!\n" );
      }
      retval = -1;
      procState = PROC_Begin;
      break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
int proc_gameLoop( double frameDuration, bool_t cleanup )
{
  int retval = 0;
  static ProcessStates procState = PROC_Begin;
  static Uint32 time_current, update_last, frame_last;
  static float dFrame, dUpdate;
  static Uint32 dTimeUpdate, dTimeFrame;

  if ( cleanup ) procState = PROC_Leaving;

  switch ( procState )
  {
    case PROC_Begin:
      {
        // Start a new module
        srand(( Uint32 ) - 1 );

        load_module( pickedmodule );   // :TODO: Seems to be the next part to fix

        pressed = bfalse;
        make_onwhichfan();
        reset_camera();
        reset_timers();
        figure_out_what_to_draw();
        make_character_matrices();
        attach_particles();

        if ( CData.networkon )
        {
          log_info( "SDL_main: Loading module %s...\n", pickedmodule );
          netmessagemode = bfalse;
          netmessagedelay = 20;
          net_sayHello();
        }
      }
      procState = PROC_Entering;
      break;

    case PROC_Entering:
      {
        // Let the game go
        moduleActive = btrue;
        randsave = 0;
        srand( 0 );
        time_current = SDL_GetTicks();
        update_last = time_current - UPDATESKIP;
        frame_last  = time_current - FRAMESKIP;
        dFrame = dUpdate = 0.0f;
      }
      procState = PROC_Running;
      break;

    case PROC_Running:
      {
        bool_t outdoors = lightspek > 0;

        time_current = SDL_GetTicks();

        dTimeFrame  = time_current - frame_last;
        dTimeUpdate = time_current - update_last;

        if(game_single_frame)
        {
          dFrame = 1.0f;
        }
        else if ( dTimeFrame > FRAMESKIP )
        {
          dFrame  += ( float )( dTimeFrame ) / ( float ) FRAMESKIP;
          frame_last = time_current;
        };

        if(game_single_frame)
        {
          dUpdate = 1.0f;
        }
        else if ( dTimeUpdate > UPDATESKIP )
        {
          dUpdate += ( float )( dTimeUpdate ) / ( float ) UPDATESKIP;
          update_last = time_current;
        };

        update_timers();

#if defined(DEBUG_UPDATE_CLAMP) && defined(_DEBUG)
        if(CData.DevMode)
        {
          log_info( "wldframe == %d\t dframe == %2.4f\n", wldframe, dUpdate );
        };
#endif

        // This is the control loop
        check_screenshot();

        if( game_do_frame )
        {
          game_do_frame = bfalse;
        }
        else if( SDLKEYDOWN( SDLK_F10 ) )
        {
          sdlkeybuffer[SDLK_F10] = 0;
          game_do_frame = btrue;
        }
        

        //Do game pause if needed
        //Check for pause key   //TODO: What to do in network games?
        if ( !SDLKEYDOWN( SDLK_F8 ) ) startpause = btrue;
        if ( SDLKEYDOWN( SDLK_F8 ) && keyon && startpause )
        {
          if ( gamepaused ) gamepaused = bfalse;
          else gamepaused = btrue;
          startpause = bfalse;
        }

        // animate the light source
        //if( outdoors && dUpdate >= 1.0 )
        //{
        //  // BB > simulate sunlight

        //  float sval, cval, xval, yval, ftmp;

        //  sval = sin( dUpdate/50.0f * TWO_PI / 30.0f);
        //  cval = cos( dUpdate/50.0f * TWO_PI / 30.0f);

        //  xval = lightspekdir.y;
        //  yval = lightspekdir.z;

        //  lightspekdir.y = xval*cval + yval*sval;
        //  lightspekdir.z =-xval*sval + yval*cval;

        //  if ( lightspekdir.z > 0 )
        //  {
        //    // do the filtering
        //    lightspekcol.r = exp(( 1.0f -  1.0f  / lightspekdir.z ) * 0.1f );
        //    lightspekcol.g = exp(( 5.0f -  5.0f  / lightspekdir.z ) * 0.1f );
        //    lightspekcol.b = exp(( 16.0f - 16.0f / lightspekdir.z ) * 0.1f );

        //    lightambicol.r = ( 1.0f - lightspekcol.r );
        //    lightambicol.g = ( 1.0f - lightspekcol.g );
        //    lightambicol.b = ( 1.0f - lightspekcol.b );

        //    // do the intensity
        //    lightspekcol.r *= ABS( lightspekdir.z ) * lightspek;
        //    lightspekcol.g *= ABS( lightspekdir.z ) * lightspek;
        //    lightspekcol.b *= ABS( lightspekdir.z ) * lightspek;

        //    if(lightambicol.r + lightambicol.g + lightambicol.b > 0)
        //    {
        //      ftmp = ( lightspekcol.r + lightspekcol.g + lightspekcol.b ) / (lightambicol.r + lightambicol.g + lightambicol.b);
        //      lightambicol.r = 0.025f + (1.0f-0.025f) * ftmp * lightambicol.r + ABS( lightspekdir.z ) * lightambi;
        //      lightambicol.g = 0.044f + (1.0f-0.044f) * ftmp * lightambicol.g + ABS( lightspekdir.z ) * lightambi;
        //      lightambicol.b = 0.100f + (0.9f-0.100f) * ftmp * lightambicol.b + ABS( lightspekdir.z ) * lightambi;
        //    };
        //  }
        //  else
        //  {
        //    lightspekcol.r = 0.0f;
        //    lightspekcol.g = 0.0f;
        //    lightspekcol.b = 0.0f;

        //    lightambicol.r = 0.025 + lightambi;
        //    lightambicol.g = 0.044 + lightambi;
        //    lightambicol.b = 0.100 + lightambi;
        //  };

        //  make_spektable( lightspekdir );
        //};


        while ( dUpdate >= 1.0 )
        {
          // Do important things
          if ( CData.networkon && waitingforplayers )
          {
            wldclock = allclock;
          }
          else if ( gamepaused || (game_single_frame && !game_do_frame) )
          {
            wldclock = allclock;
          }
          else
          {
            check_stats();
            set_local_latches();
            update_timers();
            check_passage_music();
            update_looped_sounds();

            // NETWORK PORT
            {
              // buffer the existing latches
              input_net_message();
              cl_bufferLatches( &AClientState );
              sv_bufferLatches( &AServerState );

              // upload the information
              cl_talkToHost( &AClientState );
              sv_talkToRemotes( &AServerState );

              // download/handle any queued packets
              listen_for_packets();
            }

            update_game( EMULATEUPS / TARGETUPS );
          }

          dUpdate -= 1.0f;
          ups_loops++;

#if defined(DEBUG_UPDATE_CLAMP) && defined(_DEBUG)
          if(CData.DevMode)
          {
            log_info( "\t\twldframe == %d\t dframe == %2.4f\n", wldframe, dUpdate );
          }
#endif

          move_camera( UPDATESCALE );
        };

        if ( dFrame >= 1.0 )
        {

          // Do the display stuff
          PROFILE_BEGIN( figure_out_what_to_draw );
          figure_out_what_to_draw();
          PROFILE_END( figure_out_what_to_draw );

          PROFILE_BEGIN( draw_main );
          draw_main(( float ) frameDuration );
          PROFILE_END( draw_main );

          dFrame = 0.0f;
        }

        // Check for quitters
        // :TODO: nolocalplayers is not set correctly
        if ( SDLKEYDOWN( SDLK_ESCAPE ) /*|| nolocalplayers*/ )
        {
          procState = PROC_Leaving;
        }
      }
      break;

    case PROC_Leaving:
      {
      }
      procState = PROC_Finish;
      break;

    case PROC_Finish:
      {
        quit_module();

        release_module();
        close_session();

        // Let the normal OS mouse cursor work
        SDL_WM_GrabInput( CData.GrabMouse );
        //SDL_ShowCursor(SDL_ENABLE);
      }
      retval = -1;
      procState = PROC_Begin;
      break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
int proc_menuLoop( double frameDuration, bool_t cleanup )
{
  int retval = 0;
  static ProcessStates procState = PROC_Begin;
  static int menuResult;
  static Uint32 time_current, update_last, frame_last;
  static float dFrame, dUpdate;
  static Uint32 dTimeUpdate, dTimeFrame;

  if ( cleanup ) procState = PROC_Leaving;

  switch ( procState )
  {
    case PROC_Begin:
      {
        snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.uifont_ttf );
        ui_initialize( CStringTmp1, CData.uifont_points );

        //Load stuff into memory
        prime_icons();
        prime_titleimage();
        make_textureoffset();    // THIS SHOULD WORK
        //load_all_menu_images();
        initMenus();             //Start the game menu

        // initialize the bitmap font so we can use the cursor
        snprintf( CStringTmp1, sizeof( CStringTmp1 ), "%s/%s", CData.basicdat_dir, CData.font_bitmap );
        snprintf( CStringTmp2, sizeof( CStringTmp2 ), "%s/%s", CData.basicdat_dir, CData.fontdef_file );
        if ( !load_font( CStringTmp1, CStringTmp2 ) )
        {
          log_warning( "UI unable to use load bitmap font for cursor. Files missing from %s directory\n", CData.basicdat_dir );
        };


        // Let the normal OS mouse cursor work
        SDL_WM_GrabInput( SDL_GRAB_OFF );
        SDL_ShowCursor( SDL_DISABLE );
        mouseon = bfalse;
      }
      procState = PROC_Entering;
      break;

    case PROC_Entering:
      {
      }
      procState = PROC_Running;
      break;

    case PROC_Running:
      {
        time_current = SDL_GetTicks();

        dTimeFrame  = time_current - frame_last;
        dTimeUpdate = time_current - update_last;

        if ( dTimeFrame > FRAMESKIP )
        {
          dFrame  += ( float )( dTimeFrame ) / ( float ) FRAMESKIP;
          frame_last = time_current;
        };

        if ( dTimeUpdate > UPDATESKIP )
        {
          dUpdate += ( float )( dTimeUpdate ) / ( float ) UPDATESKIP;
          update_last = time_current;
        };

        // do menus
        ui_beginFrame( frameDuration );

        menuResult = mnu_Run(( float ) frameDuration );
        retval = menuResult;
        switch ( menuResult )
        {
          case 1:
            // Go ahead and start the game
            hostactive = CData.networkon;
            break;

          case - 1:
            // The user selected "Quit"
            procState = PROC_Leaving;
            break;
        }
        ui_endFrame();

        request_pageflip();
      };
      break;

    case PROC_Leaving:
      {
      }
      procState = PROC_Finish;
      break;

    case PROC_Finish:
      {
        quit_module();

        release_module();
        close_session();

        // Let the normal OS mouse cursor work
        SDL_WM_GrabInput( CData.GrabMouse );
        //SDL_ShowCursor(SDL_ENABLE);
      }
      retval = -1;
      procState = PROC_Begin;
      break;
  };

  return retval;
};


//--------------------------------------------------------------------------------------------
int SDL_main( int argc, char **argv )
{
  int program_state = 0;

  do
  {
    program_state = proc_program( argc, argv );
  }
  while ( -1 != program_state );


  return btrue;
}

//--------------------------------------------------------------------------------------------
SLOT grip_to_slot( GRIP g )
{
  SLOT s = SLOT_NONE;

  switch ( g )
  {
    case GRIP_ORIGIN: s = SLOT_NONE; break;
    case GRIP_LAST:   s = SLOT_NONE; break;
    case GRIP_RIGHT:  s = SLOT_RIGHT; break;

      // TODO : differentiate between GRIP_SADDLE and GRIP_LEFT
    case GRIP_LEFT:   s = SLOT_LEFT;  break;
      // case GRIP_SADDLE: s = SLOT_SADDLE; break;

    case GRIP_NONE:   s = SLOT_NONE; break;

    default:
      //try to do this mathematically

      if ( 0 == ( g % GRIP_SIZE ) )
      {
        s = ( g / GRIP_SIZE ) - GRIP_SIZE;
        if ( s < 0 ) s = SLOT_NONE;
        if ( s >= SLOT_COUNT ) s = SLOT_NONE;
      }
      else
        s = SLOT_NONE;

      break;
  };

  return s;
};


//--------------------------------------------------------------------------------------------
GRIP slot_to_grip( SLOT s )
{
  GRIP g = SLOT_NONE;

  switch ( s )
  {
    case SLOT_LEFT:      g = GRIP_LEFT;   break;
    case SLOT_RIGHT:     g = GRIP_RIGHT;  break;
    case SLOT_SADDLE:    g = GRIP_SADDLE; break;

    case SLOT_NONE:      g = GRIP_ORIGIN; break;
    case SLOT_INVENTORY: g = GRIP_INVENTORY; break;

    default:
      //try to do this mathematically

      g = s * GRIP_SIZE + GRIP_SIZE;
      if ( g > GRIP_RIGHT ) g = GRIP_ORIGIN;
  }

  return g;
};

//--------------------------------------------------------------------------------------------
Uint16 slot_to_offset( SLOT s )
{
  Uint16 o = 1;

  switch ( s )
  {
    case SLOT_LEFT:      o = GRIP_LEFT;   break;
    case SLOT_RIGHT:     o = GRIP_RIGHT;  break;
    case SLOT_SADDLE:    o = GRIP_SADDLE; break;

    case SLOT_NONE:      o = GRIP_LAST; break;
    case SLOT_INVENTORY: o = GRIP_LAST; break;

    default:
      //try to do this mathematically
      o = s * GRIP_SIZE + GRIP_SIZE;
      if ( o > GRIP_RIGHT ) o = GRIP_LAST;
  }

  return o;
};

//--------------------------------------------------------------------------------------------
Uint16 slot_to_latch( Uint16 object, SLOT s )
{
  Uint16 latch = LATCHBUTTON_NONE;
  bool_t in_hand = bfalse;

  if ( VALID_CHR( object ) )
    in_hand = chr_using_slot( object, s );

  switch ( s )
  {
    case SLOT_LEFT:  latch = in_hand ? LATCHBUTTON_LEFT  : LATCHBUTTON_ALTLEFT;  break;
    case SLOT_RIGHT: latch = in_hand ? LATCHBUTTON_RIGHT : LATCHBUTTON_ALTRIGHT; break;
  };

  return latch;
};

//--------------------------------------------------------------------------------------------
DAMAGE fget_damage( FILE *fileread )
{
  char cTmp;
  DAMAGE retval = DAMAGE_NULL;
  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'S': retval = DAMAGE_SLASH; break;
    case 'C': retval = DAMAGE_CRUSH; break;
    case 'P': retval = DAMAGE_POKE;  break;
    case 'H': retval = DAMAGE_HOLY;  break;
    case 'E': retval = DAMAGE_EVIL;  break;
    case 'F': retval = DAMAGE_FIRE;  break;
    case 'I': retval = DAMAGE_ICE;   break;
    case 'Z': retval = DAMAGE_ZAP;   break;
    case 'N': retval = DAMAGE_NULL;  break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
DAMAGE fget_next_damage( FILE *fileread )
{
  DAMAGE retval = DAMAGE_NULL;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_damage( fileread );
  }

  return retval;
};

//--------------------------------------------------------------------------------------------
BLUD_LEVEL fget_blud( FILE *fileread )
{
  char cTmp;
  BLUD_LEVEL retval = BLUD_NORMAL;
  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'T': retval = BLUD_NORMAL; break;
    case 'F': retval = BLUD_NONE;   break;
    case 'U': retval = BLUD_ULTRA;  break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
BLUD_LEVEL fget_next_blud( FILE *fileread )
{
  BLUD_LEVEL retval = BLUD_NORMAL;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_blud( fileread );
  }

  return retval;
};

//--------------------------------------------------------------------------------------------
RESPAWN_MODE fget_respawn( FILE *fileread )
{
  char cTmp;
  RESPAWN_MODE retval = RESPAWN_NORMAL;
  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'T': retval = RESPAWN_NORMAL;   break;
    case 'F': retval = RESPAWN_NONE;   break;
    case 'A': retval = RESPAWN_ANYTIME;  break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
RESPAWN_MODE fget_next_respawn( FILE *fileread )
{
  RESPAWN_MODE retval = RESPAWN_NORMAL;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_respawn( fileread );
  }

  return retval;
};

//--------------------------------------------------------------------------------------------
DYNA_MODE fget_dynamode( FILE *fileread )
{
  char cTmp;
  DYNA_MODE retval = DYNA_OFF;
  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'T': retval = DYNA_ON;   break;
    case 'F': retval = DYNA_OFF;   break;
    case 'L': retval = DYNA_LOCAL; break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
DYNA_MODE fget_next_dynamode( FILE *fileread )
{
  DYNA_MODE retval = DYNA_OFF;
  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_dynamode( fileread );
  }

  return retval;
};


//--------------------------------------------------------------------------------------------
char * undo_idsz( IDSZ idsz )
{
  // ZZ> This function takes an integer and makes an text IDSZ out of it.
  //     It will set valueidsz to "NONE" if the idsz is 0
  static char value_string[5] = {"NONE"};

  if ( idsz == IDSZ_NONE )
  {
    snprintf( value_string, sizeof( value_string ), "NONE" );
  }
  else
  {
    value_string[0] = (( idsz >> 15 ) & 31 ) + 'A';
    value_string[1] = (( idsz >> 10 ) & 31 ) + 'A';
    value_string[2] = (( idsz >> 5 ) & 31 ) + 'A';
    value_string[3] = (( idsz ) & 31 ) + 'A';
    value_string[4] = 0;
  }

  return value_string;
}

//--------------------------------------------------------------------------------------------
IDSZ fget_idsz( FILE* fileread )
{
  // ZZ> This function reads and returns an IDSZ tag, or IDSZ_NONE if there wasn't one
  char sTemp[4], cTmp;
  IDSZ idsz = IDSZ_NONE;

  if ( feof( fileread ) ) return idsz;

  cTmp = fget_first_letter( fileread );
  if ( cTmp == '[' )
  {
    int cnt;
    for ( cnt = 0; cnt < 4; cnt++ ) sTemp[cnt] = fgetc( fileread );
    assert( ']' == fgetc( fileread ) );
    idsz = MAKE_IDSZ( sTemp );
  }

  return idsz;
}

//--------------------------------------------------------------------------------------------
IDSZ fget_next_idsz( FILE* fileread )
{
  // ZZ> This function reads and returns an IDSZ tag, or IDSZ_NONE if there wasn't one

  IDSZ idsz = IDSZ_NONE;

  if ( fgoto_colon_yesno( fileread ) )
  {
    idsz = fget_idsz( fileread );
  };

  return idsz;
}

//--------------------------------------------------------------------------------------------
bool_t fget_pair_fp8( FILE* fileread, PAIR * ppair )
{
  // ZZ> This function reads a damage/stat pair ( eg. 5-9 )

  char   cTmp;
  int    iBase, iRand;
  float  fBase, fRand;

  if ( feof( fileread ) ) return bfalse;

  fscanf( fileread, "%f", &fBase );   // The first number
  iBase = FLOAT_TO_FP8( fBase );

  cTmp = fget_first_letter( fileread );   // The hyphen
  if ( cTmp != '-' )
  {
    // second number is non-existant
    if ( NULL != ppair )
    {
      ppair->ibase = iBase;
      ppair->irand = 1;
    };
  }
  else
  {
    fscanf( fileread, "%f", &fRand );   // The second number
    iRand = FLOAT_TO_FP8( fRand );

    if ( NULL != ppair )
    {
      ppair->ibase = ppair->ibase = MIN( iBase, iRand );
      ppair->irand = ppair->irand = ( MAX( iBase, iRand ) - pairbase ) + 1;
    };
  };

  return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t fget_next_pair_fp8( FILE* fileread, PAIR * ppair )
{
  // ZZ> This function reads the next damage/stat pair ( eg. 5-9 )
  bool_t retval = bfalse;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_pair_fp8( fileread, ppair );
  };

  return retval;

}

//--------------------------------------------------------------------------------------------
bool_t fget_pair( FILE* fileread, PAIR * ppair )
{
  // ZZ> This function reads a damage/stat pair ( eg. 5-9 )

  char   cTmp;
  int    iBase, iRand;
  float  fBase, fRand;

  if ( feof( fileread ) ) return bfalse;

  fscanf( fileread, "%f", &fBase );   // The first number
  iBase = fBase;

  cTmp = fget_first_letter( fileread );   // The hyphen
  if ( cTmp != '-' )
  {
    // second number is non-existant
    if ( NULL != ppair )
    {
      ppair->ibase = iBase;
      ppair->irand = 1;
    };
  }
  else
  {
    fscanf( fileread, "%f", &fRand );   // The second number
    iRand = fRand;

    if ( NULL != ppair )
    {
      ppair->ibase = ppair->ibase = MIN( iBase, iRand );
      ppair->irand = ppair->irand = ( MAX( iBase, iRand ) - pairbase ) + 1;
    };
  };

  return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t fget_next_pair( FILE* fileread, PAIR * ppair )
{
  // ZZ> This function reads the next damage/stat pair ( eg. 5-9 )
  bool_t retval = bfalse;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_pair( fileread, ppair );
  };

  return retval;

}

//--------------------------------------------------------------------------------------------
bool_t undo_pair_fp8( PAIR * ppair, RANGE * prange )
{
  // ZZ> This function generates a damage/stat pair ( eg. 3-6.5 )
  //     from the base and random values.  It set prange->ffrom and
  //     prange->fto

  bool_t bfound = bfalse;
  float fFrom = 0.0f, fTo = 0.0f;

  if ( NULL != ppair )
  {
    fFrom = FP8_TO_FLOAT( ppair->ibase );
    fTo   = FP8_TO_FLOAT( ppair->ibase + ppair->irand - 1 );
    if ( fFrom < 0.0 )  fFrom = 0.0;
    if ( fTo   < 0.0 )  fTo   = 0.0;
    bfound = btrue;
  };

  if ( NULL != prange )
  {
    prange->ffrom = fFrom;
    prange->fto   = fTo;
  }

  return bfound;
}

//--------------------------------------------------------------------------------------------
bool_t undo_pair( PAIR * ppair, RANGE * prange )
{
  // ZZ> This function generates a damage/stat pair ( eg. 3-6.5 )
  //     from the base and random values.  It set prange->ffrom and
  //     prange->fto

  bool_t bfound = bfalse;
  float fFrom = 0.0f, fTo = 0.0f;

  if ( NULL != ppair )
  {
    fFrom = ppair->ibase;
    fTo   = ppair->ibase + ppair->irand - 1;
    if ( fFrom < 0.0 )  fFrom = 0.0;
    if ( fTo   < 0.0 )  fTo   = 0.0;
    bfound = btrue;
  };

  if ( NULL != prange )
  {
    prange->ffrom = fFrom;
    prange->fto   = fTo;
  }

  return bfound;
}

//--------------------------------------------------------------------------------------------
void ftruthf( FILE* filewrite, char* text, Uint8 truth )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     btrue bfalse statements

  fprintf( filewrite, text );
  if ( truth )
  {
    fprintf( filewrite, "TRUE\n" );
  }
  else
  {
    fprintf( filewrite, "FALSE\n" );
  }
}

//--------------------------------------------------------------------------------------------
void fdamagf( FILE* filewrite, char* text, DAMAGE damagetype )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     SLASH CRUSH POKE HOLY EVIL FIRE ICE ZAP statements
  fprintf( filewrite, text );

  switch ( damagetype )
  {
    case DAMAGE_SLASH: fprintf( filewrite, "SLASH\n" ); break;
    case DAMAGE_CRUSH: fprintf( filewrite, "CRUSH\n" ); break;
    case DAMAGE_POKE:  fprintf( filewrite, "POKE\n" );  break;
    case DAMAGE_HOLY:  fprintf( filewrite, "HOLY\n" );  break;
    case DAMAGE_EVIL:  fprintf( filewrite, "EVIL\n" );  break;
    case DAMAGE_FIRE:  fprintf( filewrite, "FIRE\n" );  break;
    case DAMAGE_ICE:   fprintf( filewrite, "ICE\n" );   break;
    case DAMAGE_ZAP:   fprintf( filewrite, "ZAP\n" );   break;
    default:           fprintf( filewrite, "NONE\n" );  break;
  };
}

//--------------------------------------------------------------------------------------------
void factiof( FILE* filewrite, char* text, ACTION action )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     SLASH CRUSH POKE HOLY EVIL FIRE ICE ZAP statements

  fprintf( filewrite, text );

  if ( action >= ACTION_DA && action <= ACTION_DD )
    fprintf( filewrite, "WALK\n" );
  else if ( action >= ACTION_UA && action <= ACTION_UD )
    fprintf( filewrite, "UNARMED\n" );
  else if ( action >= ACTION_TA && action <= ACTION_TD )
    fprintf( filewrite, "THRUST\n" );
  else if ( action >= ACTION_SA && action <= ACTION_SD )
    fprintf( filewrite, "SLASH\n" );
  else if ( action >= ACTION_CA && action <= ACTION_CD )
    fprintf( filewrite, "CHOP\n" );
  else if ( action >= ACTION_BA && action <= ACTION_BD )
    fprintf( filewrite, "BASH\n" );
  else if ( action >= ACTION_LA && action <= ACTION_LD )
    fprintf( filewrite, "LONGBOW\n" );
  else if ( action >= ACTION_XA && action <= ACTION_XD )
    fprintf( filewrite, "XBOW\n" );
  else if ( action >= ACTION_FA && action <= ACTION_FD )
    fprintf( filewrite, "FLING\n" );
  else if ( action >= ACTION_PA && action <= ACTION_PD )
    fprintf( filewrite, "PARRY\n" );
  else if ( action >= ACTION_ZA && action <= ACTION_ZD )
    fprintf( filewrite, "ZAP\n" );
}

//--------------------------------------------------------------------------------------------
void fgendef( FILE* filewrite, char* text, GENDER gender )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     MALE FEMALE OTHER statements

  fprintf( filewrite, text );

  switch ( gender )
  {
    case GEN_MALE:   fprintf( filewrite, "MALE" ); break;
    case GEN_FEMALE: fprintf( filewrite, "FEMALE" ); break;
    default:
    case GEN_OTHER:  fprintf( filewrite, "OTHER" ); break;
  };

  fprintf( filewrite, "\n" );
}

//--------------------------------------------------------------------------------------------
void fpairof( FILE* filewrite, char* text, PAIR * ppair )
{
  // ZZ> This function mimics fprintf in spitting out
  //     damage/stat pairs
  RANGE rng;

  fprintf( filewrite, text );
  if ( undo_pair_fp8( ppair, &rng ) )
  {
    fprintf( filewrite, "%4.2f-%4.2f\n", rng.ffrom, rng.fto );
  }
  else
  {
    fprintf( filewrite, "0\n" );
  }

}

//--------------------------------------------------------------------------------------------
void funderf( FILE* filewrite, char* text, char* usename )
{
  // ZZ> This function mimics fprintf in spitting out
  //     a name with underscore spaces
  STRING tmpstr;

  convert_spaces( tmpstr, sizeof( tmpstr ), usename );
  fprintf( filewrite, "%s%s\n", text, tmpstr );
}

//--------------------------------------------------------------------------------------------
bool_t fget_message( FILE* fileread )
{
  // ZZ> This function loads a string into the message buffer, making sure it
  //     is null terminated.

  bool_t retval = bfalse;
  int cnt;
  char cTmp;
  STRING szTmp;

  if ( feof( fileread ) ) return retval;

  if ( msgtotal >= MAXTOTALMESSAGE ) return retval;

  if ( msgtotalindex >= MESSAGEBUFFERSIZE )
  {
    msgtotalindex = MESSAGEBUFFERSIZE - 1;
  }

  // a zero return value from fscanf() means that no fields were filled
  if ( 0 != fscanf( fileread, "%s", szTmp ) )
  {
    msgindex[msgtotal] = msgtotalindex;
    szTmp[255] = 0;
    cTmp = szTmp[0];
    cnt = 1;
    while ( cTmp != 0 && msgtotalindex < MESSAGEBUFFERSIZE - 1 )
    {
      if ( cTmp == '_' )  cTmp = ' ';
      msgtext[msgtotalindex] = cTmp;
      msgtotalindex++;
      cTmp = szTmp[cnt];
      cnt++;
    }
    msgtext[msgtotalindex] = 0;  msgtotalindex++;
    msgtotal++;

    retval = btrue;
  };

  return retval;
}

//--------------------------------------------------------------------------------------------
bool_t fget_next_message( FILE* fileread )
{
  // ZZ> This function loads a string into the message buffer, making sure it
  //     is null terminated.

  bool_t retval = bfalse;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_message( fileread );
  };

  return retval;
}

//--------------------------------------------------------------------------------------------
void load_all_messages( char *loadname, Uint16 object )
{
  // ZZ> This function loads all of an objects messages
  FILE *fileread;


  madmsgstart[object] = 0;
  fileread = fs_fileOpen( PRI_NONE, NULL, loadname, "r" );
  if ( NULL != fileread )
  {
    madmsgstart[object] = msgtotal;
    while ( fget_next_message( fileread ) ) {};

    fs_fileClose( fileread );
  }
}
//--------------------------------------------------------------------------------------------
char fget_first_letter( FILE* fileread )
{
  // ZZ> This function returns the next non-whitespace character
  char cTmp = '\0';
  bool_t bfound = bfalse;

  while ( !feof( fileread ) && !bfound )
  {
    cTmp = fgetc( fileread );
    bfound = isprint( cTmp ) && !isspace( cTmp );
  };

  return cTmp;
}

//--------------------------------------------------------------------------------------------
bool_t fget_name( FILE* fileread, char *szName, size_t lnName )
{
  // ZZ> This function loads a string of up to MAXCAPNAMESIZE characters, parsing
  //     it for underscores.  The szName argument is rewritten with the null terminated
  //     string

  bool_t retval = bfalse;
  STRING szTmp;

  if ( feof( fileread ) ) return retval;

  // a zero return value from fscanf() means that no fields were filled
  if ( fget_string( fileread, szTmp, sizeof( szTmp ) ) )
  {
    convert_underscores( szName, lnName, szTmp );
    retval = btrue;
  };

  return retval;
}

//--------------------------------------------------------------------------------------------
bool_t fget_next_name( FILE* fileread, char *szName, size_t lnName )
{
  bool_t retval = bfalse;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_name( fileread, szName, lnName );
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
int fget_int( FILE* fileread )
{
  int iTmp = 0;

  if ( feof( fileread ) ) return iTmp;

  fscanf( fileread, "%d", &iTmp );

  return iTmp;
};

//--------------------------------------------------------------------------------------------
int fget_next_int( FILE* fileread )
{
  int iTmp = 0;

  if ( fgoto_colon_yesno( fileread ) )
  {
    iTmp = fget_int( fileread );
  };

  return iTmp;
};

//--------------------------------------------------------------------------------------------
float fget_float( FILE* fileread )
{
  float fTmp = 0;

  if ( feof( fileread ) ) return fTmp;

  fscanf( fileread, "%f", &fTmp );

  return fTmp;
};

//--------------------------------------------------------------------------------------------
float fget_next_float( FILE* fileread )
{
  float fTmp = 0;

  if ( fgoto_colon_yesno( fileread ) )
  {
    fTmp = fget_float( fileread );
  };

  return fTmp;
};

//--------------------------------------------------------------------------------------------
Uint16 fget_fixed( FILE* fileread )
{
  float fTmp = 0;

  if ( feof( fileread ) ) return fTmp;

  fscanf( fileread, "%f", &fTmp );

  return FLOAT_TO_FP8( fTmp );
};

//--------------------------------------------------------------------------------------------
Uint16 fget_next_fixed( FILE* fileread )
{
  Uint16 iTmp = 0;

  if ( fgoto_colon_yesno( fileread ) )
  {
    iTmp = fget_fixed( fileread );
  };

  return iTmp;
};

//--------------------------------------------------------------------------------------------
bool_t fget_bool( FILE* fileread )
{
  bool_t bTmp = bfalse;
  char cTmp;

  if ( feof( fileread ) ) return bTmp;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'T': bTmp = btrue; break;
    case 'F': bTmp = bfalse; break;
  };

  return bTmp;
};

//--------------------------------------------------------------------------------------------
bool_t fget_next_bool( FILE* fileread )
{
  bool_t bTmp = 0;

  if ( fgoto_colon_yesno( fileread ) )
  {
    bTmp = fget_bool( fileread );
  };

  return bTmp;
};

//--------------------------------------------------------------------------------------------
GENDER fget_gender( FILE* fileread )
{
  char cTmp;
  GENDER retval = GEN_RANDOM;
  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'F': retval = GEN_FEMALE; break;
    case 'M': retval = GEN_MALE;   break;
    case 'O': retval = GEN_OTHER;  break;
    case 'R': retval = GEN_RANDOM; break;
  }

  return retval;
};

//--------------------------------------------------------------------------------------------
GENDER fget_next_gender( FILE* fileread )
{
  GENDER retval = GEN_RANDOM;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_gender( fileread );
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
ACTION fget_action( FILE* fileread )
{
  char cTmp;
  ACTION retval = ACTION_DA;
  if ( feof( fileread ) ) return retval;


  cTmp = fget_first_letter( fileread );
  return what_action( cTmp );
}

//--------------------------------------------------------------------------------------------
ACTION fget_next_action( FILE* fileread )
{
  ACTION retval = ACTION_DA;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_action( fileread );
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
PRTTYPE fget_prttype( FILE * fileread )
{
  char cTmp;
  PRTTYPE retval = PRTTYPE_SOLID;

  if ( feof( fileread ) ) return retval;

  cTmp = fget_first_letter( fileread );
  switch ( toupper( cTmp ) )
  {
    case 'L': retval = PRTTYPE_LIGHT; break;
    case 'S': retval = PRTTYPE_SOLID; break;
    case 'T': retval = PRTTYPE_ALPHA; break;
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
PRTTYPE fget_next_prttype( FILE * fileread )
{
  PRTTYPE retval = PRTTYPE_SOLID;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_prttype( fileread );
  };

  return retval;
};

//--------------------------------------------------------------------------------------------
bool_t fget_string( FILE* fileread, char *szLine, size_t lnLine )
{
  // BB > this is a size-sensitive replacement for fscanf(fileread, "%s", &szLine)

  size_t len;
  char cTmp, *pout = szLine, *plast = szLine + lnLine;

  if ( feof( fileread ) ) return bfalse;
  if ( NULL == szLine || lnLine <= 1 ) return bfalse;

  // read past any initial spaces
  cTmp = '\0';
  while ( !feof( fileread ) )
  {
    cTmp = fgetc( fileread );
    if ( !isspace( cTmp ) ) break;
  }

  // stop at the next space or control character
  len = 0;
  while ( !feof( fileread ) && pout < plast )
  {
    if ( isspace( cTmp ) || !isprint( cTmp ) )
    {
      ungetc( cTmp, fileread );
      break;
    };
    *pout = cTmp;
    pout++;
    len++;
    cTmp = fgetc( fileread );
  };

  if ( pout < plast ) *pout = '\0';

  return len != 0;
}

//--------------------------------------------------------------------------------------------
bool_t fget_next_string( FILE* fileread, char *szLine, size_t lnLine )
{
  bool_t retval = bfalse;

  if ( fgoto_colon_yesno( fileread ) )
  {
    retval = fget_string( fileread, szLine, lnLine );
  };

  return retval;
}

//--------------------------------------------------------------------------------------------
void update_looped_sounds()
{
  CHR_REF ichr;

  for(ichr=0; ichr<MAXCHR; ichr++)
  {
    if( !chron[ichr] || INVALID_CHANNEL == chrloopingchannel[ichr] ) continue;

    sound_apply_mods( chrloopingchannel[ichr], chrloopingvolume[ichr], chrpos[ichr], camtrackpos, camturn_lr);
  };

}
