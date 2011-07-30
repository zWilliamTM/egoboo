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

/// @file file_formats/controls_file.h
/// @details routines for reading and writing the file controls.txt and "scancode.txt"

#include "../typedef.h"
#include "../vfs.h"
#include "../input_device.h"

#if defined(__cplusplus)
extern "C"
{
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    struct s_control;
    struct s_input_device;
    struct s_device_list;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#   define CURRENT_CONTROLS_FILE_VERSION 2

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    bool_t input_settings_load_vfs( const char *szFilename, int version );
    bool_t input_settings_save_vfs( const char* szFilename, int version );

    void export_control( vfs_FILE * filewrite, const char * text, int device, struct s_control * pcontrol );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define _controls_file_h