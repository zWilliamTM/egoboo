#pragma once

#include "egoboo_typedef.h"

#include "id_md2.h"
#include "Md2.h"

#include <SDL_opengl.h>

//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http:// www.gnu.org/licenses/>.
//*
//********************************************************************************************

/* Egoboo - mad.h
 *
 */

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

// Model tags
enum
{
    MADFX_INVICTUS       = ( 1 <<  0 ),                    // I  Make the character invincible
    MADFX_ACTLEFT        = ( 1 <<  1 ),                    // AL Activate left item
    MADFX_ACTRIGHT       = ( 1 <<  2 ),                    // AR Activate right item
    MADFX_GRABLEFT       = ( 1 <<  3 ),                    // GL GO Grab left/Grab only item
    MADFX_GRABRIGHT      = ( 1 <<  4 ),                    // GR Grab right item
    MADFX_DROPLEFT       = ( 1 <<  5 ),                    // DL Drop the item in the left/only grip
    MADFX_DROPRIGHT      = ( 1 <<  6 ),                    // DR Drop the item in the right grip
    MADFX_STOP           = ( 1 <<  7 ),                    // S  Stop movement
    MADFX_FOOTFALL       = ( 1 <<  8 ),                    // F  Play a footfall sound
    MADFX_CHARLEFT       = ( 1 <<  9 ),                    // CL Grab a character with the left/only grip
    MADFX_CHARRIGHT      = ( 1 << 10 ),                    // CR Grab a character with the right grip
    MADFX_POOF           = ( 1 << 11 )                     // P  Poof the character
};

// Animation walking
#define LIPDA               0                       // For smooth transitions 'tween
#define LIPWA               1                       //   walking rates
#define LIPWB               2
#define LIPWC               3

#define MAX_PIP_PER_PROFILE               13                                      // Max part*.txt per object

// This stuff is for actions
#define NOACTION            0xffff                     // Action not valid for this character
enum e_action
{
    ACTION_DA = 0,         // DA - Dance ( Typical standing )
    ACTION_DB,             // DB - Dance ( Bored )
    ACTION_DC,             // DC - Dance ( Bored )
    ACTION_DD,             // DD - Dance ( Bored )
    ACTION_UA,             // UA - Unarmed Attack ( Left )
    ACTION_UB,             // UB - Unarmed Attack ( Left )
    ACTION_UC,             // UC - Unarmed Attack ( Right )
    ACTION_UD,             // UD - Unarmed Attack ( Right )
    ACTION_TA,             // TA - Thrust Attack ( Left )
    ACTION_TB,             // TB - Thrust Attack ( Left )
    ACTION_TC,             // TC - Thrust Attack ( Right )
    ACTION_TD,             // TD - Thrust Attack ( Right )
    ACTION_CA,             // CA - Chop Attack ( Left )
    ACTION_CB,             // CB - Chop Attack ( Left )
    ACTION_CC,             // CC - Chop Attack ( Right )
    ACTION_CD,             // CD - Chop Attack ( Right )
    ACTION_SA,             // SA - Slice Attack ( Left )
    ACTION_SB,             // SB - Slice Attack ( Left )
    ACTION_SC,             // SC - Slice Attack ( Right )
    ACTION_SD,             // SD - Slice Attack ( Right )
    ACTION_BA,             // BA - Bash Attack ( Left )
    ACTION_BB,             // BB - Bash Attack ( Left )
    ACTION_BC,             // BC - Bash Attack ( Right )
    ACTION_BD,             // BD - Bash Attack ( Right )
    ACTION_LA,             // LA - Longbow Attack ( Left )
    ACTION_LB,             // LB - Longbow Attack ( Left )
    ACTION_LC,             // LC - Longbow Attack ( Right )
    ACTION_LD,             // LD - Longbow Attack ( Right )
    ACTION_XA,             // XA - Crossbow Attack ( Left )
    ACTION_XB,             // XB - Crossbow Attack ( Left )
    ACTION_XC,             // XC - Crossbow Attack ( Right )
    ACTION_XD,             // XD - Crossbow Attack ( Right )
    ACTION_FA,             // FA - Flinged Attack ( Left )
    ACTION_FB,             // FB - Flinged Attack ( Left )
    ACTION_FC,             // FC - Flinged Attack ( Right )
    ACTION_FD,             // FD - Flinged Attack ( Right )
    ACTION_PA,             // PA - Parry or Block ( Left )
    ACTION_PB,             // PB - Parry or Block ( Left )
    ACTION_PC,             // PC - Parry or Block ( Right )
    ACTION_PD,             // PD - Parry or Block ( Right )
    ACTION_EA,             // EA - Evade
    ACTION_EB,             // EB - Evade
    ACTION_RA,             // RA - Roll
    ACTION_ZA,             // ZA - Zap Magic ( Left )
    ACTION_ZB,             // ZB - Zap Magic ( Left )
    ACTION_ZC,             // ZC - Zap Magic ( Right )
    ACTION_ZD,             // ZD - Zap Magic ( Right )
    ACTION_WA,             // WA - Sneak
    ACTION_WB,             // WB - Walk
    ACTION_WC,             // WC - Run
    ACTION_WD,             // WD - Push
    ACTION_JA,             // JA - Jump
    ACTION_JB,             // JB - Falling ( End of Jump ) ( Dropped Item left )
    ACTION_JC,             // JC - Falling [ Dropped item right ]
    ACTION_HA,             // HA - Hit
    ACTION_HB,             // HB - Hit
    ACTION_HC,             // HC - Hit
    ACTION_HD,             // HD - Hit
    ACTION_KA,             // KA - Killed
    ACTION_KB,             // KB - Killed
    ACTION_KC,             // KC - Killed
    ACTION_KD,             // KD - Killed
    ACTION_MA,             // MA - Misc ( Drop Left Item )
    ACTION_MB,             // MB - Misc ( Drop Right Item )
    ACTION_MC,             // MC - Misc ( Cheer/Slam Left )
    ACTION_MD,             // MD - Misc ( Show Off/Slam Right/Rise from ground )
    ACTION_ME,             // ME - Misc ( Grab Item Left )
    ACTION_MF,             // MF - Misc ( Grab Item Right )
    ACTION_MG,             // MG - Misc ( Open Chest )
    ACTION_MH,             // MH - Misc ( Sit )
    ACTION_MI,             // MI - Misc ( Ride )
    ACTION_MJ,             // MJ - Misc ( Object Activated )
    ACTION_MK,             // MK - Misc ( Snoozing )
    ACTION_ML,             // ML - Misc ( Unlock )
    ACTION_MM,             // MM - Misc ( Held Left )
    ACTION_MN,             // MN - Misc ( Held Right )
    ACTION_COUNT
};

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
struct s_mad
{
    bool_t  used;                          // Model slot
    STRING  name;                          // Model name

    // templates
    Uint16  ai;                            // AI for each model
    Uint16  prtpip[MAX_PIP_PER_PROFILE];    // Local particles

    Uint16  skins;                         // Number of skins
    Uint16  skinstart;                     // Starting skin of model

    Uint16  msgstart;                      // The first message
    Uint16  transvertices;                 // Number to transform

    ego_md2_t md2;           // the OpenGL commands

    Uint16  frameliptowalkframe[4][16];    // For walk animations

    Uint8   actionvalid[ACTION_COUNT];        // bfalse if not valid
    Uint16  actionstart[ACTION_COUNT];        // First frame of anim
    Uint16  actionend[ACTION_COUNT];          // One past last frame

    // TEMPORARY: Global list of Md2Models.  It's declared in egoboo.h, which
    // is why I have to include it here at the moment.
    Md2Model * md2_ptr;
};
typedef struct s_mad mad_t;

extern mad_t   MadList[MAX_PROFILE];

#define VALID_MAD_RANGE( IMAD ) ( ((IMAD) >= 0) && ((IMAD) < MAX_PROFILE) )
#define VALID_MAD( IMAD )       ( VALID_MAD_RANGE( IMAD ) && MadList[IMAD].loaded )
#define INVALID_MAD( IMAD )     ( !VALID_MAD_RANGE( IMAD ) || !MadList[IMAD].loaded )

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

Uint16 test_frame_name( char letter );

Uint16 action_number();
Uint16 action_frame();
void   action_check_copy( const char* loadname, Uint16 object );
void   action_copy_correct( Uint16 object, Uint16 actiona, Uint16 actionb );
int    action_which( char cTmp );
void   load_action_names( const char* loadname );

void   mad_get_framefx( int frame );
void   mad_get_walk_frame( Uint16 object, int lip, int action );
void   mad_make_equally_lit( int model );
void   mad_make_framelip( Uint16 object, int action );
void   mad_rip_actions( Uint16 object );

void   log_objused( const char *savename );

int load_model_data( const char * filename, int imad, int skin_stt  );

int load_one_model_profile( const char* tmploadname, Uint16 object, int skin );