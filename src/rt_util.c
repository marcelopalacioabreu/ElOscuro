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

#include "rt_def.h"

#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "watcom.h"
#include "rt_util.h"
#include "isr.h"
#include "z_zone.h"
#include "rt_in.h"
#include "rt_main.h"
#include "scriplib.h"
#include "rt_menu.h"
#include "rt_playr.h"
#include "version.h"
#include "develop.h"
#include "rt_vid.h"
#include "rt_view.h"
#include "modexlib.h"
#include "rt_cfg.h"

#define ERRORROW        2
#define ERRORCOL        11

#define ERRORFILE       ("rotterr.txt")
#define SOFTERRORFILE   ("error.txt")
#define DEBUGFILE       ("rott.dbg")
#define MAPDEBUGFILE    ("mapinfo.txt")

#define SLASHES         ('\\')
#define MAXCHARS        8

#define WeightR  3
#define WeightG  5
#define WeightB  2

int32_t    egacolor[16];
uint8_t   *  origpal;
FILE   *  errout;
FILE   *  debugout;
FILE   *  mapdebugout;

static bool SoftErrorStarted=false;
static bool DebugStarted=false;
static bool MapDebugStarted=false;

static uint8_t egargb[48]={ 0x00,0x00,0x00,
									 0x00,0x00,0xab,
                            0x00,0xab,0x00,
                            0x00,0xab,0xab,
                            0xab,0x00,0x00,
                            0xab,0x00,0xab,
                            0xab,0x57,0x00,
                            0xab,0xab,0xab,
                            0x57,0x57,0x57,
                            0x57,0x57,0xff,
                            0x57,0xff,0x57,
                            0x57,0xff,0xff,
                            0xff,0x57,0x57,
                            0xff,0x57,0xff,
                            0xff,0xff,0x57,
									 0xff,0xff,0xff};

extern const uint8_t * ROTT_ERR;

#define SWAP(a,b) \
   {              \
   a=(a)^(b);     \
   b=(a)^(b);     \
   a=(a)^(b);     \
   }              \

//******************************************************************************
//
// FindDistance
//
//******************************************************************************

int32_t FindDistance(int32_t ix, int32_t iy)
{
  int32_t   t;

  ix= abs(ix);        /* absolute values */
  iy= abs(iy);

  if (ix<iy)
     SWAP(ix,iy);

  t = iy + (iy>>1);

  return (ix - (ix>>5) - (ix>>7)  + (t>>2) + (t>>6));
}


//******************************************************************************
//
// Find_3D_Distance
//
//******************************************************************************

int32_t Find_3D_Distance(int32_t ix, int32_t iy, int32_t iz)
   {
   int32_t   t;

   ix= abs(ix);           /* absolute values */
   iy= abs(iy);
   iz= abs(iz);

   if (ix<iy)
     SWAP(ix,iy);

   if (ix<iz)
     SWAP(ix,iz);

   t = iy + iz;

   return (ix - (ix>>4) + (t>>2) + (t>>3));
   }

//******************************************************************************
//
// atan2_appx
//
//******************************************************************************

int32_t atan2_appx(int32_t dx, int32_t dy)
{int32_t absdx, absdy;
 int32_t angle;
 int32_t ratio;


 if (!(dx||dy))
  return 0;
 absdx = abs(dx);
 absdy = abs(dy);
 if (absdx >= absdy)
  ratio = FixedDiv2(absdy,absdx);
 else
  ratio = FixedDiv2(absdx,absdy);

 if (dx >= 0)
  {if (dy >= 0)
	 {if (absdx >= absdy)
		angle = ratio;	    // 1st octant
	  else
		angle = (2<<16) - ratio; // 2nd octant
	 }
	else
	 {if (absdx >= absdy)
		angle = (8<<16) - ratio; // 8th octant
	  else
		angle = (6<<16) + ratio; // 7th octant
	 }
  }
 else
  {if (dy >= 0)
	 {if (absdx >= absdy)
		angle = (4<<16) - ratio; // 4th octant
	  else
		angle = (2<<16) + ratio; // 3rd octant
	 }
	else
	 {if (absdx >= absdy)
		angle = (4<<16) + ratio; // 5th octant
	  else
		angle = (6<<16) - ratio; // 6th octant
	 }
  }

 return (((int32_t)FixedMul(angle,ANGLESDIV8))&(FINEANGLES-1));
}



//******************************************************************************
//
// StringsNotEqual
//
//******************************************************************************
bool StringsNotEqual (char * s1, char * s2, int32_t length)
{
   int32_t i;

   for (i=0;i<length;i++)
      if (s1[i]!=s2[i])
         return true;
   return false;
}



void markgetch( void )
{
   int32_t done;
   int32_t i;

   done=0;
   while (done==0)
      {
      IN_UpdateKeyboard ();
      for (i=0;i<127;i++)
         if (Keyboard[i]==1)
            done=i;
      }
   while (Keyboard[done])
      IN_UpdateKeyboard ();
}

/*
====================
=
= FindEGAColors
=
====================
*/

void FindEGAColors ( void )
{
   int32_t i;

	for (i=0;i<16;i++)
		egacolor[i]=BestColor((int32_t)egargb[i*3],(int32_t)egargb[i*3+1],(int32_t)egargb[i*3+2],origpal);
}

//===========================================================================


uint8_t BestColor (int32_t r, int32_t g, int32_t b, uint8_t *palette)
{
	int32_t	i;
	int32_t	dr, dg, db;
	int32_t	bestdistortion, distortion;
	int32_t	bestcolor;
	uint8_t	*pal;

//
// let any color go to 0 as a last resort
//
   bestdistortion = ( WeightR*r*r + WeightG*g*g + WeightB*b*b )*2;
	bestcolor = 0;

	pal = &palette[0];
	for (i=0 ; i<= 255 ; i++,pal+=3)
	{
		dr = r - (int32_t)pal[0];
		dg = g - (int32_t)pal[1];
		db = b - (int32_t)pal[2];
      distortion = WeightR*dr*dr + WeightG*dg*dg + WeightB*db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			bestcolor = i;
		}
	}

	return bestcolor;
}

void ClearGraphicsScreen( void )
{
VL_ClearVideo(0);
}

void ClearBuffer( char * buf, int32_t size )
{
        memset(buf,0,size);
}

/*
=============================================================================

						MISC FUNCTIONS

=============================================================================
*/

/*
=================
=
= Error
=
= For abnormal program terminations
=
=================
*/

void Error (char *error, ...)
{
   char msgbuf[300];
	va_list	argptr;
   int32_t size;
   char * sptr;
   int32_t level;
   static int32_t inerror = 0;


   inerror++;
   if (inerror > 1)
      abort();


	SetTextMode ();
#if defined (ANSIESC)
   DisplayTextSplash (&ROTT_ERR, 7);
#endif
   memset (msgbuf, 0, 300);

	va_start (argptr, error);
   vsprintf (&msgbuf[0], error, argptr);
	va_end (argptr);

   scriptbuffer = &msgbuf[0];
	size = strlen (msgbuf);

	sptr = script_p = scriptbuffer;
	scriptend_p = script_p + size;
	scriptline = 1;
	endofscript = false;
	tokenready = false;

   px = ERRORCOL;
   py = ERRORROW;

   GetToken (true);
   while (!endofscript)
   {
      if ((script_p - sptr) >= 60)
      {
         px = ERRORCOL;
         py++;
         sptr = script_p;
      }

      printf("%s\n",token);
      px++;                //SPACE
      GetToken (true);
   }

#ifdef ANSIESC
   for (i = 0; i < 8; i++)
      printf ("\n");
#endif

   if (player!=NULL)
      {
      printf ("Player X     = %d\n", player->x);
      printf ("Player Y     = %d\n", player->y);
      printf ("Player Angle = %d\n\n", player->angle);
      }
   printf ("Episode      = %d\n", gamestate.episode);

   if (gamestate.episode > 1)
      level = (gamestate.mapon+1) - ((gamestate.episode-1) << 3);
   else
      level = gamestate.mapon+1;

   printf ("Area         = %d\n", level);

   ShutDown();	// DDOI - moved this so that it doesn't try to access player
   		// which is freed by this function.

   SDL_Quit();

   exit (1);
}

/*
=================
=
= SoftwareError
=
=================
*/
void SoftwareError (char *error, ...)
{
	va_list	argptr;

	if (SoftErrorStarted==false)
      return;
	va_start (argptr, error);
   vfprintf (errout, error, argptr);
	va_end (argptr);
}

//#endif


//#if (DEBUG == 1)

/*
=================
=
= DebugError
=
=================
*/
void DebugError (char *error, ...)
{
	va_list	argptr;

   if (DebugStarted==false)
      return;
	va_start (argptr, error);
   vfprintf (debugout, error, argptr);
	va_end (argptr);
}

//#endif

/*
=================
=
= OpenSoftError
=
=================
*/
void OpenSoftError ( void )
{
  errout = fopen(SOFTERRORFILE,"wt+");
  SoftErrorStarted=true;
}

/*
=================
=
= MapDebug
=
=================
*/
void MapDebug (char *error, ...)
{
	va_list	argptr;

   if (MapDebugStarted==false)
      return;
	va_start (argptr, error);
   vfprintf (mapdebugout, error, argptr);
	va_end (argptr);
}

/*
=================
=
= OpenMapDebug
=
=================
*/
void OpenMapDebug ( void )
{
  char filename[ 128 ];

  if (MapDebugStarted==true)
     return;
  GetPathFromEnvironment( filename, ApogeePath, MAPDEBUGFILE );
  mapdebugout = fopen(filename,"wt+");
  MapDebugStarted=true;
}


/*
=================
=
= StartupSoftError
=
=================
*/
void StartupSoftError ( void )
{
}

/*
=================
=
= ShutdownSoftError
=
=================
*/
void ShutdownSoftError ( void )
{
  if (DebugStarted==true)
     {
     fclose(debugout);
     DebugStarted=false;
     }
  if (SoftErrorStarted==true)
     {
     fclose(errout);
     SoftErrorStarted=false;
     }
  if (MapDebugStarted==true)
     {
     fclose(mapdebugout);
     MapDebugStarted=false;
     }
}


/*
=================
=
= CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc-1) or 0 if not present
=
=================
*/

int32_t CheckParm (char *check)
{
	int32_t		i;
	char	*parm;

	for (i = 1;i<_argc;i++)
	{
		parm = _argv[i];
		if ( !isalpha(*parm) )	// skip - / \ etc.. in front of parm
         {
         parm++;
         if (!*parm)
				continue;		// parm was only one char
         }

		if ( !_fstricmp(check,parm) )
			return i;
	}

	return 0;
}



int32_t SafeOpenAppend (char *_filename)
{
	int32_t	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_APPEND
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening for append %s: %s",filename,strerror(errno));

	return handle;
}

int32_t SafeOpenWrite (char *_filename)
{
	int32_t	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}

int32_t SafeOpenRead (char *_filename)
{
	int32_t	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDONLY | O_BINARY);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}


void SafeRead (int32_t handle, void *buffer, int32_t count)
{
	uint32_t	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (read (handle,buffer,iocount) != (int32_t)iocount)
			Error ("File read failure reading %d bytes",count);
		buffer = (void *)( (uint8_t *)buffer + iocount );
		count -= iocount;
	}
}


void SafeWrite (int32_t handle, void *buffer, int32_t count)
{
	uint32_t	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,iocount) != (int32_t)iocount)
			Error ("File write failure writing %d bytes",count);
		buffer = (void *)( (uint8_t *)buffer + iocount );
		count -= iocount;
	}
}

void SafeWriteString (int32_t handle, char * buffer)
{
	uint32_t	iocount;

   iocount=strlen(buffer);
	if (write (handle,buffer,iocount) != (int32_t)iocount)
			Error ("File write string failure writing %s\n",buffer);
}

void *SafeMalloc (int32_t size)
{
	void *ptr;

   if (zonememorystarted==false)
      Error("Called SafeMalloc without starting zone memory\n");
	ptr = Z_Malloc (size,PU_STATIC,NULL);

	if (!ptr)
      Error ("SafeMalloc failure for %d bytes",size);

	return ptr;
}

void *SafeLevelMalloc (int32_t size)
{
	void *ptr;

   if (zonememorystarted==false)
      Error("Called SafeLevelMalloc without starting zone memory\n");
   ptr = Z_LevelMalloc (size,PU_STATIC,NULL);

	if (!ptr)
      Error ("SafeLevelMalloc failure for %d bytes",size);

	return ptr;
}

void SafeFree (void * ptr)
{
   if ( ptr == NULL )
      Error ("SafeFree : Tried to free a freed pointer\n");

	Z_Free (ptr);
}

/*
==============
=
= LoadFile
=
==============
*/

int32_t	LoadFile (char *filename, void **bufferptr)
{
	int32_t		handle;
	int32_t	length;

	handle = SafeOpenRead (filename);
	length = filelength (handle);
	*bufferptr = SafeMalloc (length);
	SafeRead (handle,*bufferptr, length);
	close (handle);
	return length;
}


/*
==============
=
= SaveFile
=
==============
*/

void	SaveFile (char *filename, void *buffer, int32_t count)
{
	int32_t		handle;

	handle = SafeOpenWrite (filename);
	SafeWrite (handle, buffer, count);
	close (handle);
}


void FixFilePath(char *filename)
{
#if PLATFORM_UNIX
    char *ptr;
    char *lastsep = filename;

    if ((!filename) || (*filename == '\0'))
        return;

    if (access(filename, F_OK) == 0)  /* File exists; we're good to go. */
        return;

    for (ptr = filename; 1; ptr++)
    {
        if (*ptr == '\\')
            *ptr = PATH_SEP_CHAR;

        if ((*ptr == PATH_SEP_CHAR) || (*ptr == '\0'))
        {
            char pch = *ptr;
            struct dirent *dent = NULL;
            DIR *dir;

            if ((pch == PATH_SEP_CHAR) && (*(ptr + 1) == '\0'))
                return; /* eos is pathsep; we're done. */

            if (lastsep == ptr)
                continue;  /* absolute path; skip to next one. */

            *ptr = '\0';
            if (lastsep == filename) {
                dir = opendir((*lastsep == PATH_SEP_CHAR) ? ROOTDIR : CURDIR);
                
                if (*lastsep == PATH_SEP_CHAR) {
                    lastsep++;
                }
            } 
            else
            {
                *lastsep = '\0';
                dir = opendir(filename);
                *lastsep = PATH_SEP_CHAR;
                lastsep++;
            }

            if (dir == NULL)
            {
                *ptr = PATH_SEP_CHAR;
                return;  /* maybe dir doesn't exist? give up. */
            }

            while ((dent = readdir(dir)) != NULL)
            {
                if (strcasecmp(dent->d_name, lastsep) == 0)
                {
                    /* found match; replace it. */
                    strcpy(lastsep, dent->d_name);
                    break;
                }
            }

            closedir(dir);
            *ptr = pch;
            lastsep = ptr;

            if (dent == NULL)
                return;  /* no match. oh well. */

            if (pch == '\0')  /* eos? */
                return;
        }
    }
#endif
}


#if PLATFORM_WIN32
int32_t _dos_findfirst(char *filename, struct find_t *f)
{
    int32_t rc = _findfirst(filename, &f->data);
    f->handle = rc;
    if (rc != -1)
    {
        strncpy(f->name, f->data.name, sizeof (f->name) - 1);
        f->name[sizeof (f->name) - 1] = '\0';
        return(0);
    }
    return(1);
}

int32_t _dos_findnext(struct find_t *f)
{
    int32_t rc = 0;
    if (f->handle == -1)
        return(1);   /* invalid handle. */

    rc = _findnext(f->handle, &f->data);
    if (rc == -1)
    {
        _findclose(f->handle);
        f->handle = -1;
        return(1);
    }

    strncpy(f->name, f->data.name, sizeof (f->name) - 1);
    f->name[sizeof (f->name) - 1] = '\0';
    return(0);
}

#elif PLATFORM_UNIX 
int32_t _dos_findfirst(char *filename, struct find_t *f)
{
    char *ptr;

    if (strlen(filename) >= sizeof (f->pattern))
        return(1);

    strcpy(f->pattern, filename);
    FixFilePath(f->pattern);
    ptr = strrchr(f->pattern, PATH_SEP_CHAR);

    if (ptr == NULL)
    {
        ptr = filename;
        f->dir = opendir(CURDIR);
    }
    else
    {
        *ptr = '\0';
        f->dir = opendir(f->pattern);
        memmove(f->pattern, ptr + 1, strlen(ptr + 1) + 1);
    }

    return(_dos_findnext(f));
}


static int32_t check_pattern_nocase(const char *x, const char *y)
{
    if ((x == NULL) || (y == NULL))
        return(0);  /* not a match. */

    while ((*x) && (*y))
    {
        if (*x == '*')
            Error("Unexpected wildcard!");  /* FIXME? */

        else if (*x == '?')
        {
            if (*y == '\0')
                return(0);  /* anything but EOS is okay. */
        }

        else
        {
            if (toupper((int32_t) *x) != toupper((int32_t) *y))
                return(0);  /* not a match. */
        }

        x++;
        y++;
    }

    return(*x == *y);  /* it's a match (both should be EOS). */
}

int32_t _dos_findnext(struct find_t *f)
{
    struct dirent *dent;

    if (f->dir == NULL)
        return(1);  /* no such dir or we're just done searching. */

    while ((dent = readdir(f->dir)) != NULL)
    {
        if (check_pattern_nocase(f->pattern, dent->d_name))
        {
            if (strlen(dent->d_name) < sizeof (f->name))
            {
                strcpy(f->name, dent->d_name);
                return(0);  /* match. */
            }
        }
    }

    closedir(f->dir);
    f->dir = NULL;
    return(1);  /* no match in whole directory. */
}
#else
#error please define for your platform.
#endif

void _dos_getdate(struct dosdate_t *date)
{
	time_t curtime = time(NULL);
	struct tm *tm;
	
	if (date == NULL) {
		return;
	}
	
	memset(date, 0, sizeof(struct dosdate_t));
	
	if ((tm = localtime(&curtime)) != NULL) {
		date->day = tm->tm_mday;
		date->month = tm->tm_mon + 1;
		date->year = tm->tm_year + 1900;
		date->dayofweek = tm->tm_wday + 1;
	}
}

void GetPathFromEnvironment( char *fullname, const char *envname, const char *filename )
   {
   const char *path;
   path = envname;

   if ( path != NULL )
      {
      strcpy( fullname, path );
      if ( fullname[ strlen( fullname ) - 1 ] != PATH_SEP_CHAR )
         {
         strcat( fullname, PATH_SEP_STR );
         }
      strcat( fullname, filename );
      }
   else
      {
      strcpy( fullname, filename );
      }

      FixFilePath(fullname);
   }

void DefaultExtension (char *path, char *extension)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != PATH_SEP_CHAR && src != path)
	{
		if (*src == '.')
			return;			// it has an extension
		src--;
	}

	strcat (path, extension);
}

void DefaultPath (char *path, char *basepath)
{
	char	temp[128];

	if (path[0] == PATH_SEP_CHAR)
		return;							// absolute path location
	strcpy (temp,path);
	strcpy (path,basepath);
	strcat (path,temp);
}


void ExtractFileBase (char *path, char *dest)
{
	char	*src;
	int32_t		length;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != PATH_SEP_CHAR)
		src--;

//
// copy up to eight characters
//
	memset (dest,0,8);
	length = 0;
	while (*src && *src != '.')
	{
		if (++length == 9)
			Error ("Filename base of %s >8 chars",path);
		*dest++ = toupper(*src++);
	}
}


/*
==============
=
= ParseNum / ParseHex
=
==============
*/

int32_t ParseHex (char *hex)
{
	char	*str;
	int32_t	num;

	num = 0;
	str = hex;

	while (*str)
	{
		num <<= 4;
		if (*str >= '0' && *str <= '9')
			num += *str-'0';
		else if (*str >= 'a' && *str <= 'f')
			num += 10 + *str-'a';
		else if (*str >= 'A' && *str <= 'F')
			num += 10 + *str-'A';
		else
			Error ("Bad hex number: %s",hex);
		str++;
	}

	return num;
}


int32_t ParseNum (char *str)
{
	if (str[0] == '$')
		return ParseHex (str+1);
	if (str[0] == '0' && str[1] == 'x')
		return ParseHex (str+2);
	return atol (str);
}


#if (BYTE_ORDER == LITTLE_ENDIAN)
#define KeepInt16_T IntelShort
#define SwapInt16_T MotoShort
#define KeepInt32_T IntelLong
#define SwapInt32_T MotoLong
#else
#define KeepInt16_T MotoShort
#define SwapInt16_T IntelShort
#define KeepInt32_T MotoLong
#define SwapInt32_T IntelLong
#endif

int16_t	SwapInt16_T (int16_t l)
{
	uint8_t	b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

int16_t	KeepInt16_T (int16_t l)
{
	return l;
}


int32_t	SwapInt32_T (int32_t l)
{
	uint8_t	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int32_t)b1<<24) + ((int32_t)b2<<16) + ((int32_t)b3<<8) + (int32_t)b4;
}

int32_t	KeepInt32_T (int32_t l)
{
	return l;
}


#undef KeepShort
#undef KeepLong
#undef SwapShort
#undef SwapLong

void SwapIntelLong(int32_t *l)
{
    *l = IntelLong(*l);
}

void SwapIntelShort(int16_t *s)
{
    *s = IntelShort(*s);
}

void SwapIntelLongArray(int32_t *l, int32_t num)
{
    while (num--) {
        SwapIntelLong(l);
        l++;
    }
}

void SwapIntelShortArray(int16_t *s, int32_t num)
{
    while (num--) {
        SwapIntelShort(s);
        s++;
    }
}

/*
============================================================================

						BASIC GRAPHICS

============================================================================
*/

/*
==============
=
= GetaPalette
=
= Return an 8 bit / color palette
=
==============
*/

void
GetaPalette (uint8_t * palette)
{
  SDL_Palette *pal = sdl_surface->format->palette;

  for (int32_t i = 0; i < 256; ++i)
    {
      palette[0] = pal->colors[i].r;
      palette[1] = pal->colors[i].g;
      palette[2] = pal->colors[i].b;

      palette += 3;
    }
}

/*
==============
=
= SetaPalette
=
= Sets an 8 bit / color palette
=
==============
*/

void
SetaPalette (uint8_t * pal)
{
  SDL_Color cmap[256];

  for (int32_t i = 0; i < 256; ++i)
    {
      cmap[i].r = pal[i * 3 + 0];
      cmap[i].g = pal[i * 3 + 1];
      cmap[i].b = pal[i * 3 + 2];
    }

  SDL_SetPaletteColors (sdl_surface->format->palette, cmap, 0, 256);
}

void
GetPalette (char *palette)
{
  SDL_Palette *pal = sdl_surface->format->palette;

  for (int32_t i = 0; i < 256; ++i)
    {
      palette[0] = pal->colors[i].r;
      palette[1] = pal->colors[i].g;
      palette[2] = pal->colors[i].b;

      palette += 3;
    }
}

void SetPalette ( uint8_t * pal )
{
   VL_SetPalette (pal);
}



//******************************************************************************
//
// US_CheckParm() - checks to see if a string matches one of a set of
//    strings. The check is case insensitive. The routine returns the
//    index of the string that matched, or -1 if no matches were found
//
//******************************************************************************

int32_t US_CheckParm (char *parm, char **strings)
{
   char  cp,cs,
         *p,*s;
   int32_t      i;
   int32_t      length;

   length=strlen(parm);
   while ( (!isalpha(*parm)) && (length>0)) // Skip non-alphas
      {
      length--;
      parm++;
      }

   for (i = 0;*strings && **strings;i++)
   {
      for (s = *strings++,p = parm,cs = cp = 0;cs == cp;)
      {
         cs = *s++;
         if (!cs)
            return(i);
         cp = *p++;

         if (isupper(cs))
            cs = tolower(cs);
         if (isupper(cp))
            cp = tolower(cp);
      }
   }
   return(-1);
}

/*
=============================================================================

                  PALETTE OPS

      To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void
VL_FillPalette (int32_t red, int32_t green, int32_t blue)
{
  SDL_Color cmap[256];

  for (int32_t i = 0; i < 256; ++i)
    {
      cmap[i].r = red << 2;
      cmap[i].g = green << 2;
      cmap[i].b = blue << 2;
    }

  SDL_SetPaletteColors (sdl_surface->format->palette, cmap, 0, 256);
}

/*
=================
=
= VL_NormalizePalette
=
=================
*/

void VL_NormalizePalette (uint8_t *palette)
{
   int32_t   i;

   for (i = 0; i < 768; i++)
      *(palette+i)=(*(palette+i))>>2;
}


/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= -some cards don't like outsb palette setting-
=
=================
*/

void
VL_SetPalette (uint8_t * palette)
{
  SDL_Color cmap[256];

  for (int32_t i = 0; i < 256; i++)
    {
      cmap[i].r = gammatable[(gammaindex << 6) + (*palette++)] << 2;
      cmap[i].g = gammatable[(gammaindex << 6) + (*palette++)] << 2;
      cmap[i].b = gammatable[(gammaindex << 6) + (*palette++)] << 2;
    }

  SDL_SetPaletteColors (sdl_surface->format->palette, cmap, 0, 256);
}

//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void
VL_GetPalette (uint8_t * palette)
{
  SDL_Palette *pal = sdl_surface->format->palette;

  for (int32_t i = 0; i < 256; ++i)
    {
      palette[0] = pal->colors[i].r >> 2;
      palette[1] = pal->colors[i].g >> 2;
      palette[2] = pal->colors[i].b >> 2;

      palette += 3;
    }
}

//******************************************************************************
//
// SideOfLine
//
//******************************************************************************

int32_t SideOfLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3)
{
   int32_t a1,b1,c1;

   /* Compute a1, b1, c1, where line joining points 1 and 2
    * is "a1 x  +  b1 y  +  c1  =  0".
    */

   a1 = y2 - y1;
   b1 = x1 - x2;
   c1 = FixedMulShift(x2,y1,16) - FixedMulShift(x1,y2,16);

   return SGN0(FixedMulShift(a1,x3,16) + FixedMulShift(b1,y3,16) + c1);
}



//******************************************************************************
//
// HSORT - heap sort
//
//******************************************************************************

typedef int32_t (*PFI)();           /* pointer to a function returning int32_t  */
typedef void (*PFV)();           /* pointer to a function returning int32_t  */
static PFI Comp;                        /* pointer to comparison routine                */
static PFV Switch;                        /* pointer to comparison routine                */
static int32_t Width;                       /* width of an object in bytes                  */
static char *Base;                      /* pointer to element [-1] of array             */


static void newsift_down(L,U) int32_t L,U;
{  int32_t c;

   while(1)
      {c=L+L;
      if(c>U) break;
      if( (c+Width <= U) && ((*Comp)(Base+c+Width,Base+c)>0) ) c+= Width;
      if ((*Comp)(Base+L,Base+c)>=0) break;
      (*Switch)(Base+L, Base+c);
      L=c;
      }
}

void hsort(char * base, int32_t nel, int32_t width, int32_t (*compare)(), void (*switcher)())
{
static int32_t i,n,stop;
        /*      Perform a heap sort on an array starting at base.  The array is
                nel elements large and width is the size of a single element in
                bytes.  Compare is a pointer to a comparison routine which will
                be passed pointers to two elements of the array.  It should
                return a negative number if the left-most argument is less than
                the rightmost, 0 if the two arguments are equal, a positive
                number if the left argument is greater than the right.  (That
                is, it acts like a "subtract" operator.) If compare is 0 then
                the default comparison routine, argvcmp (which sorts an
                argv-like array of pointers to strings), is used.                                       */

   Width=width;
   Comp= compare;
   Switch= switcher;
   n=nel*Width;
   Base=base-Width;
   for (i=(n/Width/2)*Width; i>=Width; i-=Width) newsift_down(i,n);
   stop=Width+Width;
   for (i=n; i>=stop; )
      {
      (*Switch)(base, Base+i);
      newsift_down(Width,i-=Width);
      }

}

/*---------------------------------------------------------------------------*/

//******************************************************************************
//
// UL_GetPath
//
// Purpose
//    To parse the directory entered by the user to make the directory.
//
// Parms
//    Path - the path to be parsed.
//
// Returns
//    Pointer to next path
//
//******************************************************************************

char * UL_GetPath (char * path, char *dir)
{
   bool done      = 0;
   char *dr          = dir;
   int32_t cnt           = 0;

   if (*path == SLASHES)
      path++;

   while (!done)
   {
      *dr = *path;

      cnt++;                  // make sure the number of characters in the dir
      if (cnt > MAXCHARS)     // name doesn't exceed acceptable limits.
         Error ("ERROR : Directory name can only be %d characters long.\n", MAXCHARS);

      path++;
      dr++;

      if ((*path == SLASHES) || (*path == 0))
         done = true;
   }

   *dr = 0;
   return (path);
}


//******************************************************************************
//
// UL_ChangeDirectory ()
//
// Purpose
//    To change to a directory.  Checks for drive changes.
//
// Parms
//    path - The path to change to.
//
// Returns
//    TRUE  - If successful.
//    FALSE - If unsuccessful.
//
//******************************************************************************

bool UL_ChangeDirectory (char *path)
{
	if (!path || !*path) {
		return true;
	}
	
	if (chdir (path) == -1) {
	         return (false);
	}
	
	return true;
}

/*
=============
=
= AbortCheck
=
=============
*/
void AbortCheck (char * abortstring)
{
   // User abort check

   IN_UpdateKeyboard ();

   if (Keyboard[sc_Escape])
      Error("%s\n",abortstring);
}
