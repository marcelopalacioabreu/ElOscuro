/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2002-2015 icculus.org, GNU/Linux port
Copyright (C) 2018-2019 Marc-Alexandre Espiaut

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// Wad managment utilities

#pragma once

#include <stdint.h>

#include "byteorder.h"

void W_InitMultipleFiles (char**); // Initialize multiple wads
void W_InitFile (char*); // Init a single wad file

int32_t W_CheckNumForName (char*); // Check to see if the named lump exists
int32_t W_GetNumForName (char*); // Get the number for the named lump
char* W_GetNameForNum (int32_t); // Get the name for a number

int32_t W_NumLumps (void); // Get the current number of lumps managed
int32_t W_LumpLength (int32_t); // Get the length of the numbered lump
void W_ReadLump (int32_t, void*); // Read the numbered lump into a buffer
void W_WriteLump (int32_t, void*);

void* W_CacheLumpNum (int32_t, int32_t, converter_t, int32_t); // Cache in the numbered lump with the appropriate memory tag
void* W_CacheLumpName (char*, int32_t, converter_t, int32_t); // Cache in the named lump with the appropriate memory tag

extern int32_t numlumps;
extern void** lumpcache;
