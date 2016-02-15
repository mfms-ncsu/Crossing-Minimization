/**
 * @file defs.h
 * @brief
 * Definitions common to all edge crossing heuristic source files
 * @author Matt Stallmann
 * @date 2008/12/19
 * $Id: defs.h 2 2011-06-07 19:50:41Z mfms $
 */

//     Copyright (C) 2008 Matthias Stallmann.
//     Contact: matt_stallmann@ncsu.edu
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program (file COPYING.txt); if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
//     USA.

#ifndef DEFS_H
#define DEFS_H

/** standard size for all buffers holding names */
#define MAX_NAME_LENGTH 512
/** maximum length of a line in a .ord file during output */
#define LINE_LENGTH 75
/** starting capacity and additional capacity for dynamic arrays */
#define CAPACITY_INCREMENT 32

/**
 * Used with sorting heuristics to indicate whether weights are computed
 * based on edges above, below, or on both sides of a layer to be
 * sorted. This is referred to as 'orientation' in the thesis 
 */
typedef enum { UPWARD, DOWNWARD, BOTH } Orientation;

#endif

/*  [Last modified: 2011 06 02 at 22:43:48 GMT] */

