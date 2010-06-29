#pragma once

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
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file char.h
/// @note You will routinely include "char.h" only in headers (*.h) files where you need to declare an
///       object of team_t or chr_t. In *.inl files or *.c/*.cpp files you will routinely include "char.inl", instead.

#include "egoboo_object.h"

#include "file_formats/cap_file.h"
#include "graphic_mad.h"

#include "sound.h"
#include "script.h"
#include "md2.h"
#include "graphic.h"
#include "physics.h"
#include "bsp.h"

#include "egoboo.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

struct s_mad;
struct s_eve;
struct s_pip;
struct s_object_profile;
struct s_billboard_data_t;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/// Attack directions
#define ATK_FRONT  0x0000
#define ATK_RIGHT  0x4000
#define ATK_BEHIND 0x8000
#define ATK_LEFT   0xC000

#define MAP_TURN_OFFSET 0x8000

#define MAXXP           200000                        ///< Maximum experience
#define MAXMONEY        9999                        ///< Maximum money
#define SHOP_IDENTIFY   200                         ///< Maximum value for identifying shop items

#define MAX_CAP    MAX_PROFILE

#define INFINITE_WEIGHT          (( Uint32 )0xFFFFFFFF)
#define MAX_WEIGHT               (( Uint32 )0xFFFFFFFE)

#define GRABSIZE            90.0f                      ///< Grab tolerance
#define NOHIDE              127                        ///< Don't hide
#define SEEINVISIBLE        128                        ///< Cutoff for invisible characters
#define RESPAWN_ANYTIME     0xFF          ///< Code for respawnvalid...

#define RAISE               12                  ///< Helps correct z level

/// The possible methods for characters to determine what direction they are facing
typedef enum e_turn_modes
{
    TURNMODE_VELOCITY = 0,                       ///< Character gets rotation from velocity (normal)
    TURNMODE_WATCH,                              ///< For watch towers, look towards waypoint
    TURNMODE_SPIN,                               ///< For spinning objects
    TURNMODE_WATCHTARGET,                        ///< For combat intensive AI
    TURNMODE_COUNT
} TURN_MODE;

#define MANARETURNSHIFT     22                      ///< ChrList.lst[ichr].manareturn/MANARETURNSHIFT = mana regen per second

#define TURNSPD             0.01f                 ///< Cutoff for turning or same direction
#define SPINRATE            200                   ///< How fast spinners spin
#define WATCHMIN            0.01f                 ///< Tolerance for TURNMODE_WATCH

/// The vertex offsets for the various grips
enum e_grip_offset
{
    GRIP_ORIGIN    =               0,                ///< Spawn attachments at the center
    GRIP_LAST      =               1,                ///< Spawn particles at the last vertex
    GRIP_LEFT      = ( 1 * GRIP_VERTS ),             ///< Left weapon grip starts  4 from last
    GRIP_RIGHT     = ( 2 * GRIP_VERTS ),             ///< Right weapon grip starts 8 from last

    // aliases
    GRIP_INVENTORY =               GRIP_ORIGIN,
    GRIP_ONLY      =               GRIP_LEFT
};
typedef enum e_grip_offset grip_offset_t;

grip_offset_t slot_to_grip_offset( slot_t slot );
slot_t        grip_offset_to_slot( grip_offset_t grip );

#define PITDEPTH            -60                     ///< Depth to kill character

#define NOSKINOVERRIDE      -1          ///< For import

#define HURTDAMAGE           256                     ///< Minimum damage for hurt animation


//Dismounting
#define DISMOUNTZVEL        16
#define DISMOUNTZVELFLY     4
#define PHYS_DISMOUNT_TIME  (TICKS_PER_SEC*0.5f)          ///< time delay for full object-object interaction (approximately 0.5 second)

//Knockbacks
#define REEL                7600.0f     ///< Dampen for melee knock back
#define REELBASE            0.35f

//Water
#define RIPPLETOLERANCE     60          ///< For deep water
#define SPLASHTOLERANCE     10
#define RIPPLEAND           15          ///< How often ripples spawn

/// Stats
#define LOWSTAT             256                     ///< Worst...
#define PERFECTSTAT         (60*256)                ///< Maximum stat without magic effects
#define PERFECTBIG          (100*256)               ///< Perfect life or mana...
#define HIGHSTAT            (100*256)               ///< Absolute max adding enchantments as well

/// Throwing
#define THROWFIX            30.0f                    ///< To correct thrown velocities
#define MINTHROWVELOCITY    15.0f
#define MAXTHROWVELOCITY    75.0f

/// Inventory
#define MAXNUMINPACK        6                       ///< Max number of items to carry in pack
#define PACKDELAY           25                      ///< Time before inventory rotate again
#define GRABDELAY           25                      ///< Time before grab again

/// Z velocity
#define FLYDAMPEN           0.001f                    ///< Levelling rate for flyers
#define JUMPDELAY           20                      ///< Time between jumps
#define WATERJUMP           25						///< How good we jump in water
#define JUMPINFINITE        255                     ///< Flying character
#define SLIDETOLERANCE      10                      ///< Stick to ground better
#define PLATADD             -10                     ///< Height add...
#define PLATASCEND          0.10f                     ///< Ascension rate
#define PLATKEEP            0.90f                     ///< Retention rate
#define MOUNTTOLERANCE      (2 * PLATTOLERANCE)
#define STOPBOUNCING        0.1f // 1.0f                ///< To make objects stop bouncing
#define DROPZVEL            7
#define DROPXYVEL           8

//Timer resets
#define DAMAGETILETIME      32                            ///< Invincibility time
#define DAMAGETIME          16                            ///< Invincibility time
#define DEFENDTIME          16                            ///< Invincibility time
#define BORETIME            generate_randmask( 120, 255 ) ///< IfBored timer
#define CAREFULTIME         50                            ///< Friendly fire timer
#define SIZETIME            50                            ///< Time it takes to resize a character

/// Bits used to control options for the chr_get_name() function
enum e_chr_name_bits
{
    CHRNAME_NONE     = 0,               ///< no options
    CHRNAME_ARTICLE  = ( 1 << 0 ),      ///< use an article (a, an, the)
    CHRNAME_DEFINITE = ( 1 << 1 ),      ///< if set, choose "the" else "a" or "an"
    CHRNAME_CAPITAL  = ( 1 << 2 )       ///< capitalize the name
};

//------------------------------------
/// Team variables
//------------------------------------
enum e_team_types
{
    TEAM_EVIL            = ( 'E' - 'A' ),        ///< Evil team
    TEAM_GOOD            = ( 'G' - 'A' ),        ///< Good team
    TEAM_NULL            = ( 'N' - 'A' ),        ///< Null or Neutral team
    TEAM_ZIPPY           = ( 'Z' - 'A' ),        ///< Zippy Team?
    TEAM_DAMAGE,                                 ///< For damage tiles
    TEAM_MAX
};

#define NOLEADER            0xFFFF                   ///< If the team has no leader...

//--------------------------------------------------------------------------------------------
/// The description of a single team
struct s_team
{
    bool_t   hatesteam[TEAM_MAX];    ///< Don't damage allies...
    Uint16   morale;                 ///< Number of characters on team
    CHR_REF  leader;                 ///< The leader of the team
    CHR_REF  sissy;                  ///< Whoever called for help last
};
typedef struct s_team team_t;

//--------------------------------------------------------------------------------------------
/// Everything that is necessary to compute the character's interaction with the environment
struct s_chr_environment
{
    // floor stuff
    Uint8  twist;

    float  floor_level;           ///< Height of tile
    float  level;                 ///< Height of a tile or a platform
    float  fly_level;             ///< Height of tile, platform, or water, whever is highest.

    float  zlerp;
    bool_t grounded;              ///< standing on something?

    // friction stuff
    bool_t is_slipping;
    bool_t is_slippy,    is_watery;
    float  air_friction, ice_friction;
    float  fluid_friction_hrz, fluid_friction_vrt;
    float  traction, friction_hrz;

    // misc states
    bool_t   inwater;
    float    new_vx, new_vy;
    fvec3_t   acc;
};
typedef struct s_chr_environment chr_environment_t;

//--------------------------------------------------------------------------------------------
struct s_pack
{
    bool_t         is_packed;    ///< Is it in the inventory?
    bool_t         was_packed;   ///< Temporary thing...
    CHR_REF        next;        ///< Link to the next item
    int            count;       ///< How many
};
typedef struct s_pack pack_t;

#define PACK_BEGIN_LOOP(IT,INIT) IT = INIT; while( MAX_CHR != IT ) { CHR_REF IT##_internal = ChrList.lst[IT].pack.next;
#define PACK_END_LOOP(IT) IT = IT##_internal; }

//--------------------------------------------------------------------------------------------
/// the data used to define the spawning of a character
struct s_chr_spawn_data
{
    fvec3_t     pos;
    PRO_REF     profile;
    TEAM_REF    team;
    Uint8       skin;
    FACING_T    facing;
    STRING      name;
    CHR_REF     override;
};

typedef struct s_chr_spawn_data chr_spawn_data_t;

//--------------------------------------------------------------------------------------------
/// The definition of the character object
/// This "inherits" for ego_object_base_t
struct s_chr
{
    ego_object_base_t obj_base;

    chr_spawn_data_t  spawn_data;

    // character state
    ai_state_t     ai;              ///< ai data
    latch_t        latch;

    // character stats
    STRING         Name;            ///< My name
    Uint8          gender;          ///< Gender

    Uint8          lifecolor;       ///< Bar color
    SFP8_T         life;            ///< Basic character stats
    SFP8_T         lifemax;         ///< 8.8 fixed point
    UFP8_T         life_heal;       ///< 8.8 fixed point
    SFP8_T         life_return;     ///< Regeneration/poison - 8.8 fixed point

    Uint8          manacolor;       ///< Bar color
    SFP8_T         mana;            ///< Mana stuff
    SFP8_T         manamax;         ///< 8.8 fixed point
    SFP8_T         manaflow;        ///< 8.8 fixed point
    SFP8_T         manareturn;      ///< 8.8 fixed point

    SFP8_T         strength;        ///< Strength     - 8.8 fixed point
    SFP8_T         wisdom;          ///< Wisdom       - 8.8 fixed point
    SFP8_T         intelligence;    ///< Intelligence - 8.8 fixed point
    SFP8_T         dexterity;       ///< Dexterity    - 8.8 fixed point

    Uint32         experience;                    ///< Experience
    Uint8          experiencelevel;               ///< Experience Level

    pack_t         pack;             ///< what the character is holding

    Sint16         money;            ///< Money
    Uint8          ammomax;          ///< Ammo stuff
    Uint16         ammo;
    CHR_REF        holdingwhich[SLOT_COUNT]; ///< !=MAX_CHR if character is holding something
    CHR_REF        inventory[INVEN_COUNT];   ///< !=MAX_CHR if character is storing something

    // team stuff
    TEAM_REF       team;            ///< Character's team
    TEAM_REF       baseteam;        ///< Character's starting team

    // enchant data
    ENC_REF        firstenchant;                  ///< Linked list for enchants
    ENC_REF        undoenchant;                   ///< Last enchantment spawned

    float          fat;                           ///< Character's size
    float          fat_goto;                      ///< Character's size goto
    Sint16         fat_goto_time;                  ///< Time left in size change

    // jump stuff
    float          jump_power;                    ///< Jump power
    Uint8          jumptime;                      ///< Delay until next jump
    Uint8          jumpnumber;                    ///< Number of jumps remaining
    Uint8          jumpnumberreset;               ///< Number of jumps total, 255=Flying
    Uint8          jumpready;                     ///< For standing on a platform character

    // attachments
    CHR_REF        attachedto;                    ///< !=MAX_CHR if character is a held weapon
    slot_t         inwhich_slot;                  ///< SLOT_LEFT or SLOT_RIGHT

    // platform stuff
    bool_t         platform;                      ///< Can it be stood on
    bool_t         canuseplatforms;               ///< Can use platforms?
    int            holdingweight;                 ///< For weighted buttons
    CHR_REF        onwhichplatform;               ///< Am I on a platform?

    // combat stuff
    Uint8          damagetargettype;              ///< Type of damage for AI DamageTarget
    Uint8          reaffirmdamagetype;            ///< For relighting torches
    Uint8          damagemodifier[DAMAGE_COUNT];  ///< Resistances and inversion
    Uint8          defense;                       ///< Base defense rating
    SFP8_T         damageboost;                   ///< Add to swipe damage
    SFP8_T         damagethreshold;               ///< Damage below this number is ignored

    // sound stuff
    Sint8          sound_index[SOUND_COUNT];       ///< a map for soundX.wav to sound types
    int            loopedsound_channel;           ///< Which sound channel it is looping on, -1 is none.

    // missle handling
    Uint8          missiletreatment;              ///< For deflection, etc.
    Uint8          missilecost;                   ///< Mana cost for each one
    CHR_REF        missilehandler;                ///< Who pays the bill for each one...

    // "variable" properties
    bool_t         is_hidden;
    bool_t         alive;                         ///< Is it alive?
    bool_t         waskilled;                     ///< Fix for network
    bool_t         isplayer;                      ///< btrue = player
    bool_t         islocalplayer;                 ///< btrue = local player
    bool_t         invictus;                      ///< Totally invincible?
    bool_t         iskursed;                      ///< Can't be dropped?
    bool_t         nameknown;                     ///< Is the name known?
    bool_t         ammoknown;                     ///< Is the ammo known?
    bool_t         hitready;                      ///< Was it just dropped?
    bool_t         isequipped;                    ///< For boots and rings and stuff

    // "constant" properties
    bool_t         isitem;                        ///< Is it grabbable?
    bool_t         cangrabmoney;                  ///< Picks up coins?
    bool_t         openstuff;                     ///< Can it open chests/doors?
    bool_t         stickybutt;                    ///< Rests on floor
    bool_t         isshopitem;                    ///< Spawned in a shop?
    bool_t         ismount;                       ///< Can you ride it?
    bool_t         canbecrushed;                  ///< Crush in a door?
    bool_t         canchannel;                    ///< Can it convert life to mana?
    Sint16         manacost;                      ///< Mana cost to use

    // misc timers
    Sint16         grogtime;                      ///< Grog timer
    Sint16         dazetime;                      ///< Daze timer
    Sint16         boretime;                      ///< Boredom timer
    Uint8          carefultime;                   ///< "You hurt me!" timer
    Uint16         reloadtime;                    ///< Time before another shot
    Uint8          damagetime;                    ///< Invincibility timer

    // graphica info
    Uint8          flashand;        ///< 1,3,7,15,31 = Flash, 255 = Don't
    bool_t         transferblend;   ///< Give transparency to weapons?
    bool_t         icon;            ///< Show the icon?
    Uint8          sparkle;         ///< Sparkle color or 0 for off
    bool_t         StatusList_on;   ///< Display stats?
    SFP8_T         uoffvel;         ///< Moving texture speed
    SFP8_T         voffvel;
    Uint32         shadow_size;      ///< Size of shadow
    Uint32         shadow_size_save; ///< Without size modifiers
    BBOARD_REF     ibillboard;       ///< The attached billboard

    // model info
    bool_t         is_overlay;                    ///< Is this an overlay? Track aitarget...
    Uint16         skin;                          ///< Character's skin
    PRO_REF        iprofile;                      ///< Character's profile
    PRO_REF        basemodel;                     ///< The true form
    Uint8          alpha_base;
    Uint8          light_base;
    chr_instance_t inst;                          ///< the render data

    // Skills
    int           shieldproficiency;            ///< Can it use shields?
    int           canjoust;
    int           canuseadvancedweapons;
    int           see_invisible_level;
    int           canseekurse;
    int           canusedivine;
    int           canusearcane;
    int           canusetech;
    int           candisarm;
    int           canbackstab;
    int           canusepoison;
    int           canread;
    int           hascodeofconduct;
    int           darkvision_level;
    int           darkvision_level_base;

    /// collision info

    /// @note - to make it easier for things to "hit" one another (like a damage particle from
    ///        a torch hitting a grub bug), Aaron sometimes made the bumper size much different
    ///        than the shape of the actual object.
    ///        The old bumper data that is read from the data.txt file will be kept in
    ///        the struct "bump". A new bumper that actually matches the size of the object will
    ///        be kept in the struct "collision"
    bumper_t     bump;
    bumper_t     bump_save;

    bumper_t     bump_1;       ///< the loosest collision volume that mimics the current bump
    oct_bb_t     chr_prt_cv;   ///< a looser collision volume for chr-prt interactions
    oct_bb_t     chr_chr_cv;   ///< the tightest collision volume for chr-chr interactions

    Uint8        stoppedby;                     ///< Collision mask

    // character location data
    fvec3_t        pos_stt;                       ///< Starting position
    fvec3_t        pos;                           ///< Character's position
    fvec3_t        vel;                           ///< Character's velocity
    orientation_t  ori;                           ///< Character's orientation

    fvec3_t        pos_old;                       ///< Character's last position
    fvec3_t        vel_old;                       ///< Character's last velocity
    orientation_t  ori_old;                       ///< Character's last orientation

    Uint32         onwhichgrid;                   ///< Where the char is
    Uint32         onwhichblock;                  ///< The character's collision block
    CHR_REF        bumplist_next;                 ///< Next character on fanblock

    // movement properties
    bool_t         waterwalk;                     ///< Always above watersurfacelevel?
    TURN_MODE      turnmode;                      ///< Turning mode
    Uint8          sneakspd;                      ///< Sneaking if above this speed
    Uint8          walkspd;                       ///< Walking if above this speed
    Uint8          runspd;                        ///< Running if above this speed
    float          maxaccel;                      ///< Maximum acceleration
    Uint8          flyheight;                     ///< Height to stabilize at

    // data for doing the physics in bump_all_objects()
    phys_data_t       phys;
    chr_environment_t enviro;
    BSP_leaf_t        bsp_leaf;

    int               dismount_timer;                ///< a timer BB added in to make mounts and dismounts not so unpredictable
    CHR_REF           dismount_object;               ///< the object that you were dismounting from

	bool_t         safe_valid;                    ///< is the last "safe" position valid?
	fvec3_t        safe_pos;                      ///< the last "safe" position
	Uint32         safe_time;                     ///< the last "safe" time
	Uint32         safe_grid;                     ///< the last "safe" grid

	breadcrumb_list_t crumbs;                     ///< a list of previous valid positions that the object has passed through

#if defined(__cplusplus)
    s_chr();
    ~s_chr();
#endif
};

typedef struct s_chr chr_t;

//--------------------------------------------------------------------------------------------
// list definitions
//--------------------------------------------------------------------------------------------

DECLARE_STACK_EXTERN( team_t, TeamStack, TEAM_MAX );

#define VALID_TEAM_RANGE( ITEAM ) ( ((ITEAM) >= 0) && ((ITEAM) < TEAM_MAX) )

DECLARE_STACK_EXTERN( cap_t,  CapStack,  MAX_PROFILE );

#define VALID_CAP_RANGE( ICAP ) ( ((ICAP) >= 0) && ((ICAP) < MAX_CAP) )
#define LOADED_CAP( ICAP )       ( VALID_CAP_RANGE( ICAP ) && CapStack.lst[ICAP].loaded )

// counters for debugging wall collisions
extern int chr_stoppedby_tests;
extern int chr_pressure_tests;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/// Function prototypes

void character_system_begin();
void character_system_end();

chr_t * chr_ctor( chr_t * pchr );
chr_t * chr_dtor( chr_t * pchr );

void drop_money( const CHR_REF by_reference character, int money );
void call_for_help( const CHR_REF by_reference character );
void give_experience( const CHR_REF by_reference character, int amount, xp_type xptype, bool_t override_invictus );
void give_team_experience( const TEAM_REF by_reference team, int amount, Uint8 xptype );
int  damage_character( const CHR_REF by_reference character, FACING_T direction,
                       IPair damage, Uint8 damagetype, TEAM_REF team,
                       CHR_REF attacker, Uint16 effects, bool_t ignore_invictus );
void kill_character( const CHR_REF by_reference character, const CHR_REF by_reference killer, bool_t ignore_invictus );
bool_t heal_character( const CHR_REF by_reference character, const CHR_REF by_reference healer, int amount, bool_t ignore_invictus );
void spawn_poof( const CHR_REF by_reference character, const PRO_REF by_reference profile );
void spawn_defense_ping( chr_t *pchr, const CHR_REF by_reference attacker );

void reset_character_alpha( const CHR_REF by_reference character );
void reset_character_accel( const CHR_REF by_reference character );
bool_t detach_character_from_mount( const CHR_REF by_reference character, Uint8 ignorekurse, Uint8 doshop );

void flash_character_height( const CHR_REF by_reference character, Uint8 valuelow, Sint16 low, Uint8 valuehigh, Sint16 high );
void flash_character( const CHR_REF by_reference character, Uint8 value );

void free_one_character_in_game( const CHR_REF by_reference character );
//void make_one_weapon_matrix( const CHR_REF by_reference iweap, const CHR_REF by_reference iholder, bool_t do_phys  );
void make_all_character_matrices( bool_t do_physics );
void free_inventory_in_game( const CHR_REF by_reference character );

void keep_weapons_with_holders();
void make_one_character_matrix( const CHR_REF by_reference cnt );

void update_all_characters( void );
void move_all_characters( void );
void cleanup_all_characters( void );

void bump_all_characters_update_counters( void );

void do_level_up( const CHR_REF by_reference character );
bool_t setup_xp_table( const CHR_REF by_reference character );

void free_all_chraracters();

Uint32 chr_hit_wall( chr_t * pchr, float test_pos[], float nrm[], float * pressure );
bool_t chr_test_wall( chr_t * pchr, float test_pos[] );

int chr_count_free();

CHR_REF spawn_one_character( fvec3_t   pos, const PRO_REF by_reference profile, const TEAM_REF by_reference team, Uint8 skin, FACING_T facing, const char *name, const CHR_REF by_reference override );
void    respawn_character( const CHR_REF by_reference character );
int     change_armor( const CHR_REF by_reference character, int skin );
void    change_character( const CHR_REF by_reference cnt, const PRO_REF by_reference profile, Uint8 skin, Uint8 leavewhich );
void    change_character_full( const CHR_REF by_reference ichr, const PRO_REF by_reference profile, Uint8 skin, Uint8 leavewhich );
bool_t  cost_mana( const CHR_REF by_reference character, int amount, const CHR_REF by_reference killer );
void    switch_team( const CHR_REF by_reference character, const TEAM_REF by_reference team );
void    issue_clean( const CHR_REF by_reference character );
int     restock_ammo( const CHR_REF by_reference character, IDSZ idsz );
void    attach_character_to_mount( const CHR_REF by_reference character, const CHR_REF by_reference mount, grip_offset_t grip_off );
bool_t  inventory_add_item( const CHR_REF by_reference item, const CHR_REF by_reference character );
CHR_REF inventory_get_item( const CHR_REF by_reference character, grip_offset_t grip_off, bool_t ignorekurse );
void    drop_keys( const CHR_REF by_reference character );
bool_t  drop_all_items( const CHR_REF by_reference character );
bool_t  character_grab_stuff( const CHR_REF by_reference chara, grip_offset_t grip, bool_t people );

bool_t  export_one_character_name_vfs( const char *szSaveName, const CHR_REF by_reference character );
bool_t  export_one_character_profile_vfs( const char *szSaveName, const CHR_REF by_reference character );
bool_t  export_one_character_skin_vfs( const char *szSaveName, const CHR_REF by_reference character );
CAP_REF load_one_character_profile_vfs( const char *szLoadName, int slot_override, bool_t required );

void character_swipe( const CHR_REF by_reference cnt, slot_t slot );

int check_skills( const CHR_REF by_reference who, IDSZ whichskill );

bool_t is_invictus_direction( FACING_T direction, const CHR_REF by_reference character, Uint16 effects );

void   init_slot_idsz();

bool_t ai_add_order( ai_state_t * pai, Uint32 value, Uint16 counter );

struct s_billboard_data * chr_make_text_billboard( const CHR_REF by_reference ichr, const char * txt, SDL_Color color, int lifetime_secs );
const char * chr_get_name( const CHR_REF by_reference ichr, Uint32 bits );
const char * chr_get_dir_name( const CHR_REF by_reference ichr );

//--------------------------------------------------------------------------------------------
/// helper functions

void init_all_cap();
void release_all_cap();
bool_t release_one_cap( const CAP_REF by_reference icap );

const char * describe_value( float value, float maxval, int * rank_ptr );
const char* describe_damage( float value, float maxval, int * rank_ptr );

void reset_teams();

egoboo_rv chr_update_matrix( chr_t * pchr, bool_t update_size );
bool_t chr_teleport( const CHR_REF by_reference ichr, float x, float y, float z, FACING_T facing_z );

bool_t chr_request_terminate( const CHR_REF by_reference ichr );

chr_t * chr_update_hide( chr_t * pchr );

bool_t ai_state_set_changed( ai_state_t * pai );

bool_t chr_matrix_valid( chr_t * pchr );

egoboo_rv chr_update_collision_size( chr_t * pchr, bool_t update_matrix );

CHR_REF chr_has_inventory_idsz( const CHR_REF by_reference ichr, IDSZ idsz, bool_t equipped, CHR_REF * pack_last );
CHR_REF chr_holding_idsz( const CHR_REF by_reference ichr, IDSZ idsz );
CHR_REF chr_has_item_idsz( const CHR_REF by_reference ichr, IDSZ idsz, bool_t equipped, CHR_REF * pack_last );

bool_t apply_reflection_matrix( chr_instance_t * pinst, float floor_level );

bool_t chr_can_see_object( const CHR_REF by_reference ichr, const CHR_REF by_reference iobj );
int    chr_get_price( const CHR_REF by_reference ichr );

void chr_set_floor_level( chr_t * pchr, float level );
void chr_set_redshift( chr_t * pchr, int rs );
void chr_set_grnshift( chr_t * pchr, int gs );
void chr_set_blushift( chr_t * pchr, int bs );
void chr_set_sheen( chr_t * pchr, int sheen );
void chr_set_alpha( chr_t * pchr, int alpha );
void chr_set_light( chr_t * pchr, int light );

void chr_instance_get_tint( chr_instance_t * pinst, GLfloat * tint, Uint32 bits );

CHR_REF chr_get_lowest_attachment( const CHR_REF by_reference ichr, bool_t non_item );

bool_t chr_get_mass_pair( chr_t * pchr_a, chr_t * pchr_b, float * wta, float * wtb );

bool_t chr_can_mount( const CHR_REF by_reference ichr_a, const CHR_REF by_reference ichr_b );

Uint32 chr_get_framefx( chr_t * pchr );

void      chr_set_frame( const CHR_REF by_reference character, int action, int frame, int lip );

egoboo_rv chr_set_action( chr_t * pchr, int action, bool_t action_ready, bool_t override_action );
egoboo_rv chr_start_anim( chr_t * pchr, int action, bool_t action_ready, bool_t override_action );
egoboo_rv chr_set_anim( chr_t * pchr, int action, int frame, bool_t action_ready, bool_t override_action );
egoboo_rv chr_increment_action( chr_t * pchr );
egoboo_rv chr_increment_frame( chr_t * pchr );
egoboo_rv chr_play_action( chr_t * pchr, int action, bool_t action_ready );

void character_system_begin();
void character_system_end();

// these accessor functions are to complex to be inlined
MAD_REF        chr_get_imad( const CHR_REF by_reference ichr );
struct s_mad * chr_get_pmad( const CHR_REF by_reference ichr );
TX_REF         chr_get_icon_ref( const CHR_REF by_reference item );

chr_t * chr_run_config( chr_t * pchr );

chr_t * chr_config_construct( chr_t * pprt, int max_iterations );
chr_t * chr_config_initialize( chr_t * pprt, int max_iterations );
chr_t * chr_config_activate( chr_t * pprt, int max_iterations );
chr_t * chr_config_deinitialize( chr_t * pprt, int max_iterations );
chr_t * chr_config_deconstruct( chr_t * pprt, int max_iterations );

bool_t chr_update_safe( chr_t * pchr, bool_t force );

bool_t chr_update_breadcrumb_raw( chr_t * pchr );
bool_t chr_update_breadcrumb( chr_t * pchr, bool_t force );
bool_t chr_update_safe_raw( chr_t * pchr );
bool_t chr_update_safe( chr_t * pchr, bool_t force );

fvec3_t chr_get_pos( chr_t * pchr );
bool_t  chr_set_pos( chr_t * pchr, fvec3_base_t pos );

float * chr_get_pos_v( chr_t * pchr );
