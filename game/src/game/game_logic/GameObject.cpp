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

/// @file game/game_logic/GameObject.hpp
/// @details An object representing instances of in-game egoboo objects (GameObject)
/// @author Johan Jansen

#include "game/game_logic/GameObject.hpp"
#include "game/module/ObjectHandler.hpp"
#include "game/profiles/ProfileSystem.hpp"
#include "game/game.h"
#include "game/player.h"
#include "game/char.h" //ZF> TODO: remove

//Declare class static constants
const size_t GameObject::MAXNUMINPACK;


GameObject::GameObject(const PRO_REF profile, const CHR_REF id) : 
    terminateRequested(false),
    bsp_leaf(),
    spawn_data(),
    ai(),
    latch(),
    Name(),
    gender(GENDER_MALE),
    life_color(0),
    life(0),
    life_max(0),
    life_return(0),
    mana_color(0),
    mana(0),
    mana_max(0),
    mana_return(0),
    mana_flow(0),
    strength(0),
    wisdom(0),
    intelligence(0),
    dexterity(0),
    experience(0),
    experiencelevel(0),
    money(0),
    ammomax(0),
    ammo(0),
    holdingwhich(),
    equipment(),
    inventory(),
    team(TEAM_NULL),
    team_base(TEAM_NULL),
    firstenchant(INVALID_ENC_REF),
    undoenchant(INVALID_ENC_REF), 
    fat_stt(0.0f),
    fat(0.0f),
    fat_goto(0.0f),
    fat_goto_time(0),
    jump_power(0.0f),
    jump_timer(JUMPDELAY),
    jumpnumber(0),
    jumpnumberreset(0),
    jumpready(0),
    attachedto(INVALID_CHR_REF),
    inwhich_slot(SLOT_LEFT),
    inwhich_inventory(INVALID_CHR_REF),
    platform(false),
    canuseplatforms(false),
    holdingweight(0),
    targetplatform_level(0.0f),
    targetplatform_ref(INVALID_CHR_REF),
    onwhichplatform_ref(INVALID_CHR_REF),
    onwhichplatform_update(0),
    damagetarget_damagetype(0),
    reaffirm_damagetype(0),
    damage_modifier(),
    damage_resistance(),
    defense(0),
    damage_boost(0),
    damage_threshold(0),
    missiletreatment(MISSILE_NORMAL),
    missilecost(0),
    missilehandler(INVALID_CHR_REF),
    is_hidden(false),
    alive(true),
    waskilled(false),
    is_which_player(INVALID_PLAYER_REF),
    islocalplayer(false),
    invictus(false),
    iskursed(false),
    nameknown(false),
    ammoknown(false),
    hitready(true),
    isequipped(false),
    isitem(false),
    cangrabmoney(false),
    openstuff(false),
    stickybutt(false),
    isshopitem(false),
    canbecrushed(false),
    canchannel(false),
    grog_timer(0),
    daze_timer(0),
    bore_timer(BORETIME),
    careful_timer(CAREFULTIME),
    reload_timer(0),
    damage_timer(0),
    flashand(0),
    transferblend(false),
    draw_icon(false),
    sparkle(NOSPARKLE),
    show_stats(false),
    uoffvel(0),
    voffvel(0),
    shadow_size_stt(0.0f),
    shadow_size(0),
    shadow_size_save(0),
    ibillboard(INVALID_BILLBOARD_REF),
    is_overlay(false),
    skin(0),
    profile_ref(profile),
    basemodel_ref(profile),
    inst(),
    darkvision_level(0),
    see_kurse_level(0),
    see_invisible_level(0),
    skills(),

    bump_stt(),
    bump(),
    bump_save(),
    bump_1(),      
    chr_max_cv(),  
    chr_min_cv(),  
    slot_cv(),

    stoppedby(0),
    pos_stt(0, 0, 0),
    vel(0, 0, 0),

    ori(),
    pos_old(0, 0, 0), 
    vel_old(0, 0, 0),
    ori_old(),
    onwhichgrid(0),
    onwhichblock(0),
    bumplist_next(INVALID_CHR_REF),

    waterwalk(false),
    turnmode(TURNMODE_VELOCITY),
    movement_bits(( unsigned )(~0)),    // all movements valid
    anim_speed_sneak(0.0f),
    anim_speed_walk(0.0f),
    anim_speed_run(0.0f),
    maxaccel(0.0f),
    maxaccel_reset(0.0f),

    flyheight(0),
    phys(),
    enviro(),
    dismount_timer(0),  /// @note ZF@> If this is != 0 then scorpion claws and riders are dropped at spawn (non-item objects)
    dismount_object(INVALID_CHR_REF),
    safe_valid(false),
    safe_pos(0, 0, 0),
    safe_time(0),
    safe_grid(0),
    crumbs(),

    _characterID(id),
    _profile(_profileSystem.getProfile(profile)),
    _position(0.0f, 0.0f, 0.0f)
{
    // Construct the BSP node for this entity.
    bsp_leaf.ctor(&bsp_leaf, BSP_LEAF_CHR, _characterID);

    // Grip info
    holdingwhich.fill(INVALID_CHR_REF);

    //Damage resistance
    damage_modifier.fill(0);
    damage_resistance.fill(0);

    // pack/inventory info
    equipment.fill(INVALID_CHR_REF);
    inventory.fill(INVALID_CHR_REF);

    // Set up position
    ori.map_twist_facing_y = MAP_TURN_OFFSET;  // These two mean on level surface
    ori.map_twist_facing_x = MAP_TURN_OFFSET;

    //---- call the constructors of the "has a" classes

    // set the insance values to safe values
    chr_instance_ctor( &inst );

    // intialize the ai_state
    ai_state_ctor( &ai );

    // initialize the physics
    phys_data_ctor( &phys );
}

GameObject::~GameObject()
{
    /// Free all allocated memory

    // deallocate
    BillboardList_free_one(ibillboard);

    chr_instance_dtor( &inst );
    ai_state_dtor( &ai );

    EGOBOO_ASSERT( nullptr == inst.vrt_lst );
}

bool GameObject::isOverWater() const
{
	//Make sure water in the current module is actually water (could be lava, acid, etc.)
	if(!water.is_water) {
		return false;
	}

	//Check if we are on a valid tile
    if (!ego_mesh_grid_is_valid(PMesh, onwhichgrid)) {
    	return false;
    }

    return 0 != ego_mesh_test_fx(PMesh, onwhichgrid, MAPFX_WATER);
}


bool GameObject::setPosition(const fvec3_t& position)
{
    LOG_NAN_FVEC3(position);

    //Has our position changed?
    if(position != _position)
    {
        _position = position;

        onwhichgrid   = ego_mesh_get_grid(PMesh, _position.x, _position.y);
        onwhichblock  = ego_mesh_get_block(PMesh, _position.x, _position.y);

        // update whether the current character position is safe
        chr_update_safe( this, false );

        // update the breadcrumb list
        chr_update_breadcrumb( this, false );

        return true;
    }

    return false;
}


void GameObject::movePosition(const float x, const float y, const float z)
{
    _position.x += x;
    _position.y += y;
    _position.z += z;
}

void GameObject::setAlpha(const int alpha)
{
    inst.alpha = Math::constrain(alpha, 0, 0xFF);
    chr_instance_update_ref(&inst, enviro.grid_level, false);
}

void GameObject::setLight(const int light)
{
    inst.light = Math::constrain(light, 0, 0xFF);

    //This prevents players from becoming completely invisible
    if (VALID_PLA(is_which_player))  
    {
        inst.light = std::max<uint8_t>(128, inst.light);
    }

    chr_instance_update_ref(&inst, enviro.grid_level, false);
}

void GameObject::setSheen(const int sheen)
{
    inst.sheen = Math::constrain(sheen, 0, 0xFF);
    chr_instance_update_ref(&inst, enviro.grid_level, false);
}

bool GameObject::canMount(const std::shared_ptr<GameObject> mount) const
{
    //Cannot mount ourselves!
    if(this == mount.get()) {
        return false;
    }

    //Make sure they are a mount and alive
    if(!mount->isMount() || !mount->alive) {
        return false;
    }

    //We must be alive and not an item to become a rider
    if(!alive || isitem || _gameObjects.exists(attachedto)) {
        return false;
    }

    //Cannot mount while flying
    if(flyheight != 0) {
        return false;
    }

    //Make sure they aren't mounted already
    if(!mount->getProfile()->isSlotValid(SLOT_LEFT) || _gameObjects.exists(mount->holdingwhich[SLOT_LEFT])) {
        return false;
    }

    //We need a riding animation to be able to mount stuff
    int action_mi = mad_get_action_ref( chr_get_imad( _characterID ), ACTION_MI );
    bool has_ride_anim = ( ACTION_COUNT != action_mi && !ACTION_IS_TYPE( action_mi, D ) );

    return has_ride_anim;
}


//--------------------------------------------------------------------------------------------
int GameObject::damage(const FACING_T direction, const IPair  damage, const DamageType damagetype, const TEAM_REF team,
                      const std::shared_ptr<GameObject> &attacker, const BIT_FIELD effects, const bool ignore_invictus)
{
    int     action;
    bool do_feedback = ( EGO_FEEDBACK_TYPE_OFF != cfg.feedback );

    //Simply ignore damaging invincible targets
    if(invictus && !ignore_invictus) {
        return 0;
    }

    //Don't continue if there is no damage or the character isn't alive
    int max_damage = std::abs( damage.base ) + std::abs( damage.rand );
    if ( !alive || 0 == max_damage ) return 0;

    // make a special exception for DAMAGE_NONE
    uint8_t damageModifier = ( damagetype >= DAMAGE_COUNT ) ? 0 : damage_modifier[damagetype];

    // determine some optional behavior
    bool friendly_fire = false;
    if ( !attacker )
    {
        do_feedback = false;
    }
    else
    {
        // do not show feedback for damaging yourself
        if ( attacker.get() == this )
        {
            do_feedback = false;
        }

        // identify friendly fire for color selection :)
        if ( getTeam() == attacker->getTeam() )
        {
            friendly_fire = true;
        }

        // don't show feedback from random objects hitting each other
        if ( !attacker->show_stats )
        {
            do_feedback = false;
        }

        // don't show damage to players since they get feedback from the status bars
        if ( show_stats || VALID_PLA( is_which_player ) )
        {
            do_feedback = false;
        }
    }

    // Lessen actual_damage for resistance, resistance is done in percentages where 0.70f means 30% damage reduction from that damage type
    // This can also be used to lessen effectiveness of healing
    int actual_damage = generate_irand_pair( damage );
    int base_damage   = actual_damage;
    actual_damage *= std::max( 0.00f, ( damagetype >= DAMAGE_COUNT ) ? 1.00f : 1.00f - damage_resistance[damagetype] );

    // Increase electric damage when in water
    if ( damagetype == DAMAGE_ZAP && isOverWater() )
    {
        // Only if actually in the water
        if ( getPosZ() <= water.surface_level )
            actual_damage *= 2.0f;     /// @note ZF> Is double damage too much?
    }

    // Allow actual_damage to be dealt to mana (mana shield spell)
    if (HAS_SOME_BITS(damageModifier, DAMAGEMANA))
    {
        int manadamage;
        manadamage = std::max( mana - actual_damage, 0 );
        mana = manadamage;
        actual_damage -= manadamage;
        updateLastAttacker(attacker, false);
    }

    // Allow charging (Invert actual_damage to mana)
    if (HAS_SOME_BITS(damageModifier, DAMAGECHARGE))
    {
        mana += actual_damage;
        if ( mana > mana_max )
        {
            mana = mana_max;
        }
        return 0;
    }

    // Invert actual_damage to heal
    if (HAS_SOME_BITS(damageModifier, DAMAGEINVERT))
    {
        actual_damage = -actual_damage;        
    }

    // Remember the actual_damage type
    ai.damagetypelast = damagetype;
    ai.directionlast  = direction;

    // Check for characters who are immune to this damage, no need to continue if they have
    bool immune_to_damage = (actual_damage > 0 && actual_damage <= damage_threshold) || HAS_SOME_BITS(damageModifier, DAMAGEINVICTUS);
    if ( immune_to_damage && !ignore_invictus )
    {
        actual_damage = 0;

        //Tell that the character is simply immune to the damage
        //but don't do message and ping for mounts, it's just irritating
        if ( !isMount() )
        {
            //Dark green text
            const float lifetime = 3;
            SDL_Color text_color = {0xFF, 0xFF, 0xFF, 0xFF};
            GLXvector4f tint  = { 0.0f, 0.5f, 0.00f, 1.00f };

            spawn_defense_ping( this, attacker->getCharacterID() );
            chr_make_text_billboard(_characterID, "Immune!", text_color, tint, lifetime, bb_opt_all);
        }
    }

    // Do it already
    if ( actual_damage > 0 )
    {
        // Only actual_damage if not invincible
        if ( 0 == damage_timer || ignore_invictus )
        {
            // Normal mode reduces damage dealt by monsters with 30%!
            if (cfg.difficulty == GAME_NORMAL && VALID_PLA(is_which_player))
            {
                actual_damage *= 0.70f;
            }

            // Easy mode deals 25% extra actual damage by players and 50% less to players
            if ( cfg.difficulty <= GAME_EASY )
            {
                if ( VALID_PLA( attacker->is_which_player )  && !VALID_PLA(is_which_player) ) actual_damage *= 1.25f;
                if ( !VALID_PLA( attacker->is_which_player ) &&  VALID_PLA(is_which_player) ) actual_damage *= 0.5f;
            }

            if ( 0 != actual_damage )
            {
                //Does armor apply?
                if ( HAS_NO_BITS( DAMFX_ARMO, effects ) )
                {
                    //Armor can reduce up to 50% of the damage (at 255)
                    actual_damage *= 0.5f + (256.0f - defense)/256.0f;
                }

                life -= actual_damage;

                // Spawn blud particles
                if ( _profile->getBludType() )
                {
                    if ( _profile->getBludType() == ULTRABLUDY || ( base_damage > HURTDAMAGE && DAMAGE_IS_PHYSICAL( damagetype ) ) )
                    {
                        spawnOneParticle( getPosition(), ori.facing_z + direction, _profile->getSlotNumber(), _profile->getBludParticleProfile(),
                                            INVALID_CHR_REF, GRIP_LAST, team, _characterID);
                    }
                }

                // Set attack alert if it wasn't an accident
                if ( base_damage > HURTDAMAGE )
                {
                    if ( team == TEAM_DAMAGE )
                    {
                        ai.attacklast = INVALID_CHR_REF;
                    }
                    else
                    {
                        updateLastAttacker(attacker, false );
                    }
                }

                //Did we survive?
                if (life <= 0)
                {
                    kill_character( _characterID, attacker->getCharacterID(), ignore_invictus );
                }
                else
                {
                    //Yes, but play the hurt animation
                    action = ACTION_HA;
                    if ( base_damage > HURTDAMAGE )
                    {
                        action += generate_randmask(0, 3);
                        chr_play_action(this, action, false);

                        // Make the character invincible for a limited time only
                        if ( HAS_NO_BITS( effects, DAMFX_TIME ) )
                        {
                            damage_timer = DAMAGETIME;
                        }
                    }
                }
            }

            /// @test spawn a fly-away damage indicator?
            if ( do_feedback )
            {
                const char * tmpstr;
                int rank;

                //tmpstr = describe_wounds( pchr->life_max, pchr->life );

                tmpstr = describe_value( actual_damage, UINT_TO_UFP8( 10 ), &rank );
                if ( rank < 4 )
                {
                    tmpstr = describe_value( actual_damage, max_damage, &rank );
                    if ( rank < 0 )
                    {
                        tmpstr = "Fumble!";
                    }
                    else
                    {
                        tmpstr = describe_damage( actual_damage, life_max, &rank );
                        if ( rank >= -1 && rank <= 1 )
                        {
                            tmpstr = describe_wounds( life_max, life );
                        }
                    }
                }

                if ( NULL != tmpstr )
                {
                    const int lifetime = 3;
                    STRING text_buffer = EMPTY_CSTR;

                    // "white" text
                    SDL_Color text_color = {0xFF, 0xFF, 0xFF, 0xFF};

                    // friendly fire damage = "purple"
                    GLXvector4f tint_friend = { 0.88f, 0.75f, 1.00f, 1.00f };

                    // enemy damage = "red"
                    GLXvector4f tint_enemy  = { 1.00f, 0.75f, 0.75f, 1.00f };

                    // write the string into the buffer
                    snprintf( text_buffer, SDL_arraysize( text_buffer ), "%s", tmpstr );

                    chr_make_text_billboard(_characterID, text_buffer, text_color, friendly_fire ? tint_friend : tint_enemy, lifetime, bb_opt_all );
                }
            }
        }
    }

    // Heal 'em instead
    else if ( actual_damage < 0 )
    {
        heal(attacker, -actual_damage, ignore_invictus);

        // Isssue an alert
        if ( team == TEAM_DAMAGE )
        {
            ai.attacklast = INVALID_CHR_REF;
        }

        /// @test spawn a fly-away heal indicator?
        if ( do_feedback )
        {
            const float lifetime = 3;
            STRING text_buffer = EMPTY_CSTR;

            // "white" text
            SDL_Color text_color = {0xFF, 0xFF, 0xFF, 0xFF};

            // heal == yellow, right ;)
            GLXvector4f tint = { 1.00f, 1.00f, 0.75f, 1.00f };

            // write the string into the buffer
            snprintf( text_buffer, SDL_arraysize( text_buffer ), "%s", describe_value( -actual_damage, damage.base + damage.rand, NULL ) );

            chr_make_text_billboard(_characterID, text_buffer, text_color, tint, lifetime, bb_opt_all );
        }
    }

    return actual_damage;
}

void GameObject::updateLastAttacker(const std::shared_ptr<GameObject> &attacker, bool healing)
{
    // Don't let characters chase themselves...  That would be silly
    if ( this == attacker.get() ) return;

    // Don't alert the character too much if under constant fire
    if (0 != careful_timer) return;

    CHR_REF actual_attacker = INVALID_CHR_REF;

    // Figure out who is the real attacker, in case we are a held item or a controlled mount
    if(attacker)
    {
        actual_attacker = attacker->getCharacterID();

        //Do not alert items damaging (or healing) their holders, healing potions for example
        if ( attacker->attachedto ==ai.index ) return;

        //If we are held, the holder is the real attacker... unless the holder is a mount
        if ( _gameObjects.exists( attacker->attachedto ) && !_gameObjects.get(attacker->attachedto)->isMount() )
        {
            actual_attacker = attacker->attachedto;
        }

        //If the attacker is a mount, try to blame the rider
        else if ( attacker->isMount() && _gameObjects.exists( attacker->holdingwhich[SLOT_LEFT] ) )
        {
            actual_attacker = attacker->holdingwhich[SLOT_LEFT];
        }
    }

    //Update alerts and timers
    ai.attacklast = actual_attacker;
    SET_BIT(ai.alert, healing ? ALERTIF_HEALED : ALERTIF_ATTACKED);
    careful_timer = CAREFULTIME;
}

bool GameObject::heal(const std::shared_ptr<GameObject> &healer, const UFP8_T amount, const bool ignoreInvincibility)
{
    //Don't heal dead and invincible stuff
    if (!alive || (invictus && !ignoreInvincibility)) return false;

    //This actually heals the character
    life = CLIP(static_cast<UFP8_T>(life), life + amount, life_max);

    // Set alerts, but don't alert that we healed ourselves
    if (healer && this != healer.get() && healer->attachedto != _characterID && amount > HURTDAMAGE)
    {
        updateLastAttacker(healer, true);
    }

    return true;
}