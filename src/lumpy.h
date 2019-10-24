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
#ifndef _lumpy_
#define _lumpy_

#include <stdint.h>
#include "rt_util.h"
//****************************************************************************
//
// Public header for LUMPY typedefs
//
//****************************************************************************


typedef struct
{
   uint8_t     width,height;
   uint8_t     data;
} pic_t;

#define CONVERT_ENDIAN_pic_t(pp) { }

typedef struct
{
   short     width,height;
   short     orgx,orgy;
   uint8_t     data;
} lpic_t;

#define CONVERT_ENDIAN_lpic_t(lp)            \
    {                                        \
        SwapIntelShort(&lp->width);          \
        SwapIntelShort(&lp->height);         \
        SwapIntelShort(&lp->orgx);           \
        SwapIntelShort(&lp->orgy);           \
    }
    
typedef struct
{
	short height;
   char  width[256];
   short charofs[256];
   uint8_t  data;       // as much as required
} font_t;

#define CONVERT_ENDIAN_font_t(fp)            \
    {                                        \
        SwapIntelShort(&fp->height);         \
        for (int local_cefontt = 0; local_cefontt < 256; local_cefontt++) {      \
            SwapIntelShort(&fp->charofs[local_cefontt]); \
        }                                    \
    }

typedef struct
{
   short width;
   short height;
   uint8_t palette[768];
   uint8_t data;
} lbm_t;

#define CONVERT_ENDIAN_lbm_t(lp)             \
    {                                        \
        SwapIntelShort(&lp->width);          \
        SwapIntelShort(&lp->height);         \
    }
 
typedef struct
{
   short          origsize;         // the orig size of "grabbed" gfx
   short          width;            // bounding box size
   short          height;
   short          leftoffset;       // pixels to the left of origin
   short          topoffset;        // pixels above the origin
   unsigned short collumnofs[320];  // only [width] used, the [0] is &collumnofs[width]
} patch_t;

#define CONVERT_ENDIAN_patch_t(pp)           \
    {                                        \
        SwapIntelShort(&pp->origsize);       \
        SwapIntelShort(&pp->width);          \
        SwapIntelShort(&pp->height);         \
        SwapIntelShort(&pp->leftoffset);     \
        SwapIntelShort(&pp->topoffset);      \
        for (int local_cepatcht = 0; local_cepatcht < pp->width; local_cepatcht++) {\
            SwapIntelShort((short*)&pp->collumnofs[local_cepatcht]); \
        }                                    \
    }

typedef struct
{
   short origsize;         // the orig size of "grabbed" gfx
   short width;            // bounding box size
   short height;
   short leftoffset;       // pixels to the left of origin
   short topoffset;        // pixels above the origin
   short translevel;
   short collumnofs[320];  // only [width] used, the [0] is &collumnofs[width]
} transpatch_t;

#define CONVERT_ENDIAN_transpatch_t(pp)      \
    {                                        \
        SwapIntelShort(&pp->origsize);       \
        SwapIntelShort(&pp->width);          \
        SwapIntelShort(&pp->height);         \
        SwapIntelShort(&pp->leftoffset);     \
        SwapIntelShort(&pp->topoffset);      \
        SwapIntelShort(&pp->translevel);     \
        for (int local_cetranspatcht = 0; local_cetranspatcht < pp->width; local_cetranspatcht++) {\
            SwapIntelShort((short*)&pp->collumnofs[local_cetranspatcht]); \
        }                                    \
    }

typedef struct
{
   uint8_t  color;
   short height;
   char  width[256];
   short charofs[256];
   uint8_t  pal[0x300];
   uint8_t  data;       // as much as required
} cfont_t;

#define CONVERT_ENDIAN_cfont_t(pp)           \
    {                                        \
        SwapIntelShort(&pp->height);         \
        for (int local_cecfontt = 0; local_cecfontt < 256; local_cecfontt++) {      \
            SwapIntelShort(&pp->charofs[local_cecfontt]); \
        }                                    \
    }

#endif
