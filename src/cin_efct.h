/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

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
#ifndef _cin_efct_public
#define _cin_efct_public

#include <stdint.h>

#include "cin_def.h"

typedef struct
{
  char name[10];
  int32_t duration;
  int32_t frame;
  int32_t frametime;
  int32_t numframes;
  int32_t framedelay;
  int32_t x;
  int32_t y;
  int32_t scale;
  int32_t dx;
  int32_t dy;
  int32_t dscale;
} spriteevent;

typedef struct
{
  char name[10];
  int32_t duration;
  int32_t backdropwidth;
  int32_t currentoffset;
  int32_t dx;
  int32_t yoffset;
  int32_t height;
  uint8_t *data;
} backevent;

typedef struct
{
  char name[10];
} paletteevent;

typedef struct
{
  char name[10];
  bool loop;
  bool usefile;
} flicevent;

flicevent * SpawnCinematicFlic ( char * name, bool loop, bool usefile );
spriteevent * SpawnCinematicSprite ( char * name,
                                     int duration,
                                     int numframes,
                                     int framedelay,
                                     int x,
                                     int y,
                                     int scale,
                                     int endx,
                                     int endy,
                                     int endscale
                                   );
backevent * SpawnCinematicBack ( char * name,
                                 int duration,
                                 int width,
                                 int startx,
                                 int endx,
                                 int yoffset
                               );

backevent * SpawnCinematicMultiBack ( char * name,
                                      char * name2,
                                      int duration,
                                      int startx,
                                      int endx,
                                      int yoffset
                                    );
paletteevent * SpawnCinematicPalette ( char * name );
void DrawFlic ( flicevent * flic );
void DrawCinematicBackdrop ( backevent * back );
void DrawCinematicBackground ( backevent * back );
void DrawPalette (void);
void DrawCinematicSprite ( spriteevent * sprite );
void DrawClearBuffer ( void );
void DrawBlankScreen ( void );
bool DrawCinematicEffect ( enum_eventtype type, void * effect );
bool UpdateCinematicBack ( backevent * back );
bool UpdateCinematicSprite ( spriteevent * sprite );
bool UpdateCinematicEffect ( enum_eventtype type, void * effect );
void PrecacheCinematicEffect ( enum_eventtype type, void * effect );
void ProfileDisplay ( void );
void DrawPostPic ( int lumpnum );

#endif

