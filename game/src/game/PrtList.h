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

/// @file game/PrtList.h
/// @brief Routines for particle list management

#pragma once

#include "game/egoboo_typedef.h"
#include "game/egoboo_object.h"
#include "game/LockableList.hpp"
#include "game/particle.h"

//--------------------------------------------------------------------------------------------
// looping macros
//--------------------------------------------------------------------------------------------

// Macros automate looping through the PrtList. This hides code which defers the creation and deletion of
// objects until the loop terminates, so tha the length of the list will not change during the loop.

#define PRT_BEGIN_LOOP_ACTIVE(IT, PRT_BDL) \
    { \
        int IT##_internal; \
        int prt_loop_start_depth = PrtList.getLockCount(); \
        PrtList.lock(); \
        for(IT##_internal=0;IT##_internal<PrtList.getUsedCount();IT##_internal++) \
        { \
            PRT_REF IT; \
            prt_bundle_t PRT_BDL; \
            IT = (PRT_REF)PrtList.used_ref[IT##_internal]; \
            if(!ACTIVE_PRT(IT)) continue; \
            prt_bundle_t::set(&PRT_BDL, PrtList.get_ptr( IT ));

#define PRT_BEGIN_LOOP_DISPLAY(IT, PRT_BDL) \
    { \
        int IT##_internal; \
        int prt_loop_start_depth = PrtList.getLockCount(); \
        PrtList.lock(); \
        for(IT##_internal=0;IT##_internal<PrtList.getUsedCount();IT##_internal++) \
        { \
            PRT_REF IT; \
            prt_bundle_t PRT_BDL; \
            IT = (PRT_REF)PrtList.used_ref[IT##_internal]; \
            if(!DISPLAY_PRT(IT)) continue; \
            prt_bundle_t::set(&PRT_BDL, PrtList.get_ptr(IT));

#define PRT_END_LOOP() \
        } \
        PrtList.unlock(); \
        EGOBOO_ASSERT(prt_loop_start_depth == PrtList.getLockCount()); \
        PrtList.maybeRunDeferred(); \
    }

//--------------------------------------------------------------------------------------------
// external variables
//--------------------------------------------------------------------------------------------

struct ParticleManager : public _LockableList < prt_t, PRT_REF, INVALID_PRT_REF, MAX_PRT, BSP_LEAF_PRT>
{
    ParticleManager() :
        _LockableList(),
        _displayLimit(512)
    {
    }

    void update_used();

    /**
     * @brief
     *	Run all deferred updates if the particle list is not locked.
     */
    void maybeRunDeferred();

    /**
     * @brief
     *  Get an unused particle.
     *   If all particles are in use and @a force is @a true, get the first unimportant one.
     * @return
     *  the particle index on success, INVALID_PRT_REF on failure
     */
    PRT_REF allocate(const bool force);

    void reset_all();

public:
    bool free_one(const PRT_REF iprt);
    bool push_free(const PRT_REF);
    void prune_used_list();
    void prune_free_list();

protected:
    /**
     * @brief
     *  An display limit smaller than @a MAX_PRT is an upper-bound for the number of particles rendered.
     */
    size_t _displayLimit;
public:
    /**
     * @brief
     *  Get the display limit for particles.
     * @return
     *  the display limit for particles
     */
    size_t getDisplayLimit() const
    {
        return _displayLimit;
    }
    /**
     * @brief
     *  Set the display limit for particles.
     * @param displayLimit
     *  the display limit for particles
     */
    void setDisplayLimit(size_t displayLimit)
    {
        displayLimit = Math::constrain<uint16_t>(displayLimit, 256, MAX_PRT);
        if (_displayLimit != displayLimit)
        {
            _displayLimit = displayLimit;
        }
    }
};

extern ParticleManager PrtList;

//--------------------------------------------------------------------------------------------
// testing functions
//--------------------------------------------------------------------------------------------

bool VALID_PRT_RANGE(const PRT_REF IPRT);
bool DEFINED_PRT(const PRT_REF IPRT);
bool ALLOCATED_PRT(const PRT_REF IPRT);
bool ACTIVE_PRT(const PRT_REF IPRT);
bool WAITING_PRT(const PRT_REF IPRT);
bool TERMINATED_PRT(const PRT_REF IPRT);
PRT_REF GET_REF_PPRT(const prt_t *PPRT);
bool DEFINED_PPRT(const prt_t *PPRT);
bool VALID_PRT_PTR(const prt_t *PPRT);
bool ALLOCATED_PPRT(const prt_t *PPRT);
bool ACTIVE_PPRT(const prt_t *PPRT);
bool WAITING_PPRT(const prt_t *PPRT);
bool TERMINATED_PPRT(const prt_t *PPRT);
bool INGAME_PRT_BASE(const PRT_REF IPRT);
bool INGAME_PPRT_BASE(const prt_t *PPRT);
bool INGAME_PRT(const PRT_REF IPRT);
bool INGAME_PPRT(const prt_t *PPRT);
bool DISPLAY_PRT(const PRT_REF IPRT);
bool DISPLAY_PPRT(const prt_t *PPRT);
