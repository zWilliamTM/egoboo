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

#pragma once
#if !defined(EGOLIB_PROFILES_PRIVATE) || EGOLIB_PROFILES_PRIVATE != 1
#error(do not include directly, include `egolib/profiles/_Include.hpp` instead)
#endif

#include "egolib/Profiles/_AbstractProfileSystem.hpp"
#include "egolib/Profiles/ParticleProfile.hpp"
#include "egolib/Profiles/ParticleProfileReader.hpp"

extern _AbstractProfileSystem<pip_t, PIP_REF, INVALID_PIP_REF, MAX_PIP, ParticleProfileReader> PipStack;

#define VALID_PIP_RANGE(ref) (PipStack.isValidRange(ref))
#define LOADED_PIP(ref) (PipStack.isLoaded(ref))