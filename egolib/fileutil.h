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

/// @file fileutil.h
/// Read/write values from/to files

#include "../egolib/typedef.h"

#include "../egolib/vfs.h"
#include "../egolib/file_common.h"

#if defined(__cplusplus)
extern "C"
{
#endif

//--------------------------------------------------------------------------------------------
// TYPEDEFS
//--------------------------------------------------------------------------------------------

    struct s_oglx_texture;

//--------------------------------------------------------------------------------------------
// MACROS
//--------------------------------------------------------------------------------------------

// ASCII terminal/printer control codes
#    define ASCII_NUL_CHAR  '\x00'            /**< null */
#    define ASCII_SOH_CHAR  '\x01'            /**< start of heading */
#    define ASCII_STX_CHAR  '\x02'            /**< start of text */
#    define ASCII_ETX_CHAR  '\x03'            /**< end of text */
#    define ASCII_EOT_CHAR  '\x04'            /**< end of transmission */
#    define ASCII_ENQ_CHAR  '\x05'            /**< enquiry */
#    define ASCII_ACK_CHAR  '\x06'            /**< acknowledge */
#    define ASCII_BEL_CHAR  '\x07'            /**< bell */
#    define ASCII_BS_CHAR   '\x08'            /**< backspace */
#    define ASCII_HT_CHAR   '\x09'            /**< horizontal tab */
#    define ASCII_NL_CHAR   '\x0A'            /**< new line (or LF, line feed) */
#    define ASCII_VT_CHAR   '\x0B'            /**< vertical tab */
#    define ASCII_NP_CHAR   '\x0C'            /**< new page (or FF, form feed) */
#    define ASCII_CR_CHAR   '\x0D'            /**< carriage return */
#    define ASCII_SO_CHAR   '\x0E'            /**< shift out */
#    define ASCII_SI_CHAR   '\x0F'            /**< shift in */
#    define ASCII_DLE_CHAR  '\x10'            /**< data link escape */
#    define ASCII_DC1_CHAR  '\x11'            /**< device control 1 */
#    define ASCII_DC2_CHAR  '\x12'            /**< device control 2 */
#    define ASCII_DC3_CHAR  '\x13'            /**< device control 3 */
#    define ASCII_DC4_CHAR  '\x14'            /**< device control 4 */
#    define ASCII_NAK_CHAR  '\x15'            /**< negative acknowledge */
#    define ASCII_SYN_CHAR  '\x16'            /**< synchronous idle */
#    define ASCII_ETB_CHAR  '\x17'            /**< end of transmission block */
#    define ASCII_CAN_CHAR  '\x18'            /**< cancel */
#    define ASCII_EM_CHAR   '\x19'            /**< end of medium */
#    define ASCII_SUB_CHAR  '\x1A'            /**< substitute */
#    define ASCII_ESC_CHAR  '\x1B'            /**< escape */
#    define ASCII_FS_CHAR   '\x1C'            /**< file separator */
#    define ASCII_GS_CHAR   '\x1D'            /**< group separator */
#    define ASCII_RS_CHAR   '\x1E'            /**< record separator */
#    define ASCII_US_CHAR   '\x1F'            /**< unit separator */
#    define ASCII_SP_CHAR   '\x20'            /**< space  */

/// @note win32 systems (and some others) handle newlines by using a combinarion
///       of linefeed and carriage return characters. So...
///       the numerical values of '\n' and '\r' may vary from system to system and
///       may be different for input and output (i.e. '\n' writing '\x0D\x0A" on win32 systems)

#    define ASCII_LINEFEED_CHAR    ASCII_NL_CHAR

/// @note the following escape codes are translated by the compiler
///       to whatever encoding is necessary
#    define C_BELL_CHAR            '\a'
#    define C_BACKSPACE_CHAR       '\b'
#    define C_FORMFEED_CHAR        '\f'
#    define C_NEW_LINE_CHAR        '\n'
#    define C_CARRIAGE_RETURN_CHAR '\r'
#    define C_TAB_CHAR             '\t'
#    define C_VERTICAL TAB_CHAR    '\v'
#    define C_SINGLE_QUOTE_CHAR    '\''
#    define C_DOUBLE_QUOTE_CHAR    '\"'

#define TRANSCOLOR                      0

//--------------------------------------------------------------------------------------------
// EXTERNAL VARIABLES
//--------------------------------------------------------------------------------------------

    extern const char *parse_filename;          ///< For debuggin' goto_colon_vfs

    extern  STRING     TxFormatSupported[20]; ///< OpenGL icon surfaces
    extern  Uint8      maxformattypes;

//--------------------------------------------------------------------------------------------
// GLOBAL FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------

    void   make_newloadname( const char *modname, const char *appendname, char *newloadname );

    C_BOOLEAN goto_delimiter_vfs( char * buffer, vfs_FILE* fileread, char delim, C_BOOLEAN optional );
    char   goto_delimiter_list_vfs( char * buffer, vfs_FILE* fileread, const char * delim_list, C_BOOLEAN optional );
    C_BOOLEAN goto_colon_vfs( char * buffer, vfs_FILE* fileread, C_BOOLEAN optional );
    char * goto_colon_mem( char * buffer, char * pmem, char * pmem_end, C_BOOLEAN optional );

    C_BOOLEAN copy_line_vfs( vfs_FILE * fileread, vfs_FILE * filewrite );
    char * copy_to_delimiter_mem( char * pmem, char * pmem_end, vfs_FILE * filewrite, int delim, char * user_buffer, size_t user_buffer_len );
    C_BOOLEAN copy_to_delimiter_vfs( vfs_FILE * fileread, vfs_FILE * filewrite, int delim, char * buffer, size_t bufflen );

    int    vfs_get_version( vfs_FILE* fileread );
    C_BOOLEAN vfs_put_version( vfs_FILE* filewrite, const int version );

    char   vfs_get_next_char( vfs_FILE * fileread );
    int    vfs_get_next_int( vfs_FILE * fileread );
    float  vfs_get_next_float( vfs_FILE * fileread );
    UFP8_T vfs_get_next_ufp8( vfs_FILE* fileread );
    SFP8_T vfs_get_next_sfp8( vfs_FILE* fileread );
    C_BOOLEAN vfs_get_next_name( vfs_FILE * fileread, char * name, size_t name_len );
    C_BOOLEAN vfs_get_next_range( vfs_FILE* fileread, FRange * prange );
    C_BOOLEAN vfs_get_next_pair( vfs_FILE * fileread, IPair * ppair );
    IDSZ   vfs_get_next_idsz( vfs_FILE * fileread );
    C_BOOLEAN vfs_get_next_bool( vfs_FILE * fileread );
    C_BOOLEAN vfs_get_next_string( vfs_FILE * fileread, char * str, size_t str_len );
    C_BOOLEAN vfs_get_next_line( vfs_FILE * fileread, char * str, size_t str_len );

    char   vfs_get_first_letter( vfs_FILE* fileread );
    Sint32 vfs_get_int( vfs_FILE* fileread );
    UFP8_T vfs_get_ufp8( vfs_FILE* fileread );
    SFP8_T vfs_get_sfp8( vfs_FILE* fileread );
    float  vfs_get_float( vfs_FILE * fileread );
    IDSZ   vfs_get_idsz( vfs_FILE* fileread );
    int    vfs_get_damage_type( vfs_FILE * fileread );
    int    vfs_get_next_damage_type( vfs_FILE * fileread );
    C_BOOLEAN vfs_get_bool( vfs_FILE * fileread );
    Uint8  vfs_get_damage_modifier( vfs_FILE * fileread );
    float  vfs_get_damage_resist( vfs_FILE * fileread );
    C_BOOLEAN vfs_get_name( vfs_FILE* fileread,  char *szName, size_t max_len );
    C_BOOLEAN vfs_get_string( vfs_FILE * fileread, char * str, size_t str_len );
    C_BOOLEAN vfs_get_line( vfs_FILE * fileread, char * str, size_t str_len );
    C_BOOLEAN vfs_get_range( vfs_FILE* fileread, FRange * prange );
    C_BOOLEAN vfs_get_pair( vfs_FILE* fileread, IPair * ppair );

    void vfs_put_int( vfs_FILE* filewrite, const char* text, int ival );
    void vfs_put_float( vfs_FILE* filewrite, const char* text, float fval );
    void vfs_put_ufp8( vfs_FILE* filewrite, const char* text, UFP8_T ival );
    void vfs_put_sfp8( vfs_FILE* filewrite, const char* text, SFP8_T ival );
    void vfs_put_bool( vfs_FILE* filewrite, const char* text, C_BOOLEAN truth );
    void vfs_put_damage_type( vfs_FILE* filewrite, const char* text, Uint8 damagetype );
    void vfs_put_action( vfs_FILE* filewrite, const char* text, Uint8 action );
    void vfs_put_gender( vfs_FILE* filewrite, const char* text, Uint8 gender );
    void vfs_put_range( vfs_FILE* filewrite, const char* text, FRange val );
    void vfs_put_pair( vfs_FILE* filewrite, const char* text, IPair val );
    void vfs_put_string_under( vfs_FILE* filewrite, const char* text, const char* usename );
    void vfs_put_idsz( vfs_FILE* filewrite, const char* text, IDSZ idsz );
    void vfs_put_expansion( vfs_FILE* filewrite, const char* text, IDSZ idsz, int value );
    void vfs_put_expansion_float( vfs_FILE* filewrite, const char* text, IDSZ idsz, float value );
    void vfs_put_expansion_string( vfs_FILE* filewrite, const char* text, IDSZ idsz, const char * str );

    void vfs_put_range_raw( vfs_FILE* filewrite, FRange val );
    int read_skin_vfs( const char *filename );

    void    GLSetup_SupportedFormats( void );
    Uint32  ego_texture_load_vfs( struct s_oglx_texture *texture, const char *filename, Uint32 key );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#if defined(__cplusplus)
}

#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define _egolib_fileutil_h
