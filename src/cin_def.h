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
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FRACTIONBITS 16
#define FRACTIONUNIT 1<<FRACTIONBITS

typedef enum
{
  background_noscrolling,
  background_scrolling,
  background_multi,
  sprite_background,
  sprite_foreground,
  backdrop_scrolling,
  backdrop_noscrolling,
  palette,
  flic,
  fadeout,
  cinematicend,
  blankscreen,
  clearbuffer
} enum_eventtype;
