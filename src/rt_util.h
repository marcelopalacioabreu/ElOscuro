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
//***************************************************************************
//
//    RT_UTIL.C - various utils palette funcs and modex stuff
//
//***************************************************************************

#ifndef _rt_util_public
#define _rt_util_public

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "rt_def.h"

#define ERRORVERSIONROW 1
#define ERRORVERSIONCOL 67

#define LOWMEMORYROW 18
#define LOWMEMORYCOL 1

#include "develop.h"

extern  int32_t    egacolor[16];
extern  uint8_t   *  origpal;
extern  int32_t      _argc;
extern  char **  _argv;

void  markgetch( void );
bool StringsNotEqual (char * s1, char * s2, int32_t length);
void  GetPalette(char * pal);
void  ClearGraphicsScreen( void );
void  ClearBuffer( char * buf, int32_t size );
void  Error (char *error, ...) __attribute__((noreturn,format(printf,1,2)));
void  StartupSoftError ( void );
void  ShutdownSoftError ( void );
int32_t   CheckParm (char *check);
int32_t   SafeOpenWrite (char *filename);
int32_t   SafeOpenAppend (char *filename);
int32_t   SafeOpenRead (char *filename);
void  SafeRead (int32_t handle, void *buffer, int32_t count);
void  SafeWrite (int32_t handle, void *buffer, int32_t count);
void  SafeWriteString (int32_t handle, char * buffer);
void  *SafeMalloc (int32_t size);
void  *SafeLevelMalloc (int32_t size);
void  SafeFree (void * ptr);
int32_t  LoadFile (char *filename, void **bufferptr);
void  SaveFile (char *filename, void *buffer, int32_t count);
void  GetPathFromEnvironment( char *fullname, const char *envname, const char *filename );
void  DefaultExtension (char *path, char *extension);
void  DefaultPath (char *path, char *basepath);
void  ExtractFileBase (char *path, char *dest);
int32_t  ParseHex (char *hex);
int32_t  ParseNum (char *str);
int16_t MotoShort (int16_t l);
int16_t IntelShort (int16_t l);
int32_t   MotoLong (int32_t l);
int32_t   IntelLong (int32_t l);
void  SwapIntelLong (int32_t *l);
void  SwapIntelShort(int16_t *s);
void  SwapIntelLongArray (int32_t *l, int32_t num);
void  SwapIntelShortArray (int16_t *s, int32_t num);
int32_t   US_CheckParm (char *parm, char **strings);
uint8_t  BestColor (int32_t r, int32_t g, int32_t b, uint8_t *palette);
int32_t   atan2_appx(int32_t ,int32_t);
int32_t   FindDistance(int32_t ix, int32_t iy);
int32_t   Find_3D_Distance(int32_t ix, int32_t iy, int32_t iz);
void  SetPalette ( uint8_t * pal );
void  SetaPalette ( uint8_t * pal );
void  FindEGAColors ( void );
void  VL_FillPalette (int32_t red, int32_t green, int32_t blue);
void  VL_SetPalette (uint8_t *palette);
void  VL_GetPalette (uint8_t *palette);
void  VL_NormalizePalette (uint8_t *palette);
void  MapDebug (char *error, ...) __attribute__((format(printf,1,2)));
void  OpenMapDebug ( void );

int32_t   SideOfLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);

void hsort(char * base, int32_t nel, int32_t width, int32_t (*compare)(), void (*switcher)());

char * UL_GetPath (char * path, char *dir);
bool UL_ChangeDirectory (char *path);
void AbortCheck (char * abortstring);

void FixFilePath(char *filename);


#if PLATFORM_WIN32
#include <io.h>
struct find_t
{
	int32_t handle;
    struct _finddata_t data;
	char name[MAX_PATH];
};
int32_t _dos_findfirst(char *filename, struct find_t *f);
int32_t _dos_findnext(struct find_t *f);

#elif PLATFORM_UNIX
struct find_t
{
    DIR *dir;
    char pattern[MAX_PATH];
    char name[MAX_PATH];
};
int32_t _dos_findfirst(char *filename, struct find_t *f);
int32_t _dos_findnext(struct find_t *f);
#else
#error please define for your platform.
#endif


struct dosdate_t
{
    uint8_t day;
    uint8_t month;
    uint32_t year;
    uint8_t dayofweek;
};

void _dos_getdate(struct dosdate_t *date);

void  SoftwareError (char *error, ...) __attribute__((format(printf,1,2)));
//#define SoftError  SoftwareError

#define SoftError  if (1) {} else SoftwareError

void  DebugError (char *error, ...) __attribute__((format(printf,1,2)));
#define Debug  DebugError
//#define Debug

void Square (void);

#define my_outp(a,b)
#define OUTP                              my_outp

#endif
