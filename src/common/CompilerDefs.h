/*
 * This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WH_COMPILERDEFS_H
#define WH_COMPILERDEFS_H

#define WH_PLATFORM_WINDOWS 0

// must be first (win 64 also define _WIN32)
#if defined( _WIN64 )
#  define WH_PLATFORM WH_PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define WH_PLATFORM WH_PLATFORM_WINDOWS
#else
#  error "FATAL ERROR: Unknown PLATFORM."
#endif

#define WH_COMPILER_MICROSOFT 0
#define WH_COMPILER_GNU       1
#define WH_COMPILER_BORLAND   2
#define WH_COMPILER_INTEL     3

#ifdef _MSC_VER
#  define WH_COMPILER WH_COMPILER_MICROSOFT
#else
#  error "FATAL ERROR: Unknown compiler."
#endif

#endif
