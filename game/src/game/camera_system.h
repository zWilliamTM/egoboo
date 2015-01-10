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

/// @file game/camera_system.h

#pragma once

#include "game/egoboo_typedef.h"

#include "game/camera.h"
#include "game/input.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define MAX_CAMERAS MAX_LOCAL_PLAYERS

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

// Forward declaration.
struct ego_mesh_t;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

/// an opaque struct holding all of the extended camera information
struct ext_camera_t;

/// an opaque struct used to house the list of cameras
struct ext_camera_list_t;

/// an opaque struct used to iterate over cameras
struct ext_camera_iterator_t;

/// a callback for the camera system to render the game data
typedef void ( *renderer_ptr_t )( const camera_t *, const int render_list_index, const int dolist_index );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// Function prototypes

bool    camera_system_is_started();

egolib_rv camera_system_begin( int camera_count );
egolib_rv camera_system_end();
egolib_rv camera_system_init( int camera_count );

egolib_rv camera_system_reset( struct ego_mesh_t * pmesh );
egolib_rv camera_system_move( struct ego_mesh_t * pmesh );
egolib_rv camera_system_reset_targets( struct ego_mesh_t * pmesh );

camera_t          * camera_system_get_main();
ext_camera_list_t * camera_system_get_list();

egolib_rv camera_system_render_all( renderer_ptr_t );

egolib_rv camera_system_begin_camera( int index );
egolib_rv camera_system_end_camera( int index );

ext_camera_iterator_t * camera_list_iterator_begin( ext_camera_list_t * ptr );
ext_camera_iterator_t * camera_list_iterator_next( ext_camera_iterator_t * );
ext_camera_iterator_t * camera_list_iterator_end( ext_camera_iterator_t * );
camera_t *              camera_list_iterator_get_camera( ext_camera_iterator_t * );

camera_t *              camera_list_find_target( ext_camera_list_t * plst, const CHR_REF itarget );
camera_t *              camera_list_get_camera_index( ext_camera_list_t * plst, int index );
ext_camera_t *          camera_list_get_ext_camera_index( ext_camera_list_t * plst, int index );
int                     camera_list_find_target_index( ext_camera_list_t * plst, const CHR_REF itarget );

bool ext_camera_set_screen( ext_camera_t * ptr, float xmin, float ymin, float xmax, float ymax );
bool ext_camera_get_screen( ext_camera_t * pext, ego_frect_t * prect );
