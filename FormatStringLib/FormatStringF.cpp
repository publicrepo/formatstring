//
// FormatStringF.cpp
// Format string utility function
//
// Based upon a relatively simple and self contained free, open source snprintf implementation.
//
// Changes:
// vargs with C++ style boxed argument list.
// basic support for 64bit ints and pointers.
// basic support for %g and %e formats
// basic support for float exception formats (Ind, Inf, Nan) with signs.
// fixed critical bug where leading zeros in fraction part of float were lost (eg. 1.002 became 1.2)
// experimental support for ',' separator
//
// NOTE: Still missing newer multi byte modifiers like 'llu' (long long unsigned decimal).
//       Note that the new type extensions are unnecessary as types are auto detected.
//       Does not support the $ parameter index format as per printf_p or posix extension
//
// Original Source: http://www.fiction.net/blong/programs/snprintf.c  (http://www.fiction.net/blong/programs/#snprintf)
// Use at your own risk, based on FOSS code, please observe any replicated copyright notices.
//
//

#include <iostream>
#include "FormatStringF.h"

BEGIN_NAMESPACE_FORMATSTRINGLIB

#define UDFS_USE_64BIT 1 // Use 64bit types for precision, support and 64bit addressing
#define UDFS_USE_MOREFLOAT 1 // Use more float handling 

//
//  Copyright Patrick Powell 1995
//  This code is based on code written by Patrick Powell (papowell@astart.com)
//  It may be used for any purpose as long as this notice remains intact
//  on all source code distributions
//

//  Original:
//  Patrick Powell Tue Apr 11 09:48:21 PDT 1995
//  A bombproof version of doprnt (dopr) included.
//  Sigh.  This sort of thing is always nasty do deal with.  Note that
//  the version here does not include floating point...
// 
//  snprintf() is used instead of sprintf() as it does limit checks
//  for string length.  This covers a nasty loophole.
// 
//  The other functions are there to prevent NULL pointers from
//  causing nast effects.
// 
//  More Recently:
//   Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
//   This was ugly.  It is still ugly.  I opted out of floating point
//   numbers, but the formatter understands just about everything
//   from the normal C string format, at least as far as I can tell from
//   the Solaris 2.5 printf(3S) man page.
// 
//   Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
//     Ok, added some minimal floating point support, which means this
//     probably requires libm on most operating systems.  Don't yet
//     support the exponent (e,E) and sigfig (g,G).  Also, fmtint()
//     was pretty badly broken, it just wasn't being exercised in ways
//     which showed it, so that's been fixed.  Also, formated the code
//     to mutt conventions, and removed dead code left over from the
//     original.  Also, there is now a builtin-test, just compile with:
//            gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
//     and run snprintf for results.
//  
//   Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
//     The PGP code was using unsigned hexadecimal formats. 
//     Unfortunately, unsigned formats simply didn't work.
// 
//   Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
//     The original code assumed that both snprintf() and vsnprintf() were
//     missing.  Some systems only have snprintf() but not vsnprintf(), so
//     the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
// 
//   Andrew Tridgell (tridge@samba.org) Oct 1998
//     fixed handling of %.0f
//     added test for HAVE_LONG_DOUBLE
// 
//   Russ Allbery <rra@stanford.edu> 2000-08-26
//     fixed return value to comply with C99
//     fixed handling of snprintf(NULL, ...)
// 
 
#ifdef HAVE_LONG_DOUBLE
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif

static int dopr(char *buffer, size_t maxlen, const char *format, ArgList& a_argList); //va_list args);
static int fmtstr(char *buffer, size_t *currlen, size_t maxlen, const char *value, int flags, int min, int max);

static int fmtint(char *buffer, size_t *currlen, size_t maxlen, long value, int base, int min, int max, int flags);
static int fmtint_64(char *buffer, size_t *currlen, size_t maxlen, fsInt64 a_value, int base, int min, int max, int flags);

static int fmtfp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = false, bool a_allowTrailingZeros = true);
static int fmtfp_gen(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = true);
static int fmtfp_exp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = true);

static int fmtfp64(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = false, bool a_allowTrailingZeros = true);
static int fmtfp64_gen(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = true);
static int fmtfp64_exp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException = true);

static int dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c );

static int isfpexception(LDOUBLE fvalue);
static int fmtfp_exception(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags);


//
// dopr(): poor man's version of doprintf
//

// format read states 
#define DP_S_DEFAULT 0
#define DP_S_FLAGS   1
#define DP_S_MIN     2
#define DP_S_DOT     3
#define DP_S_MAX     4
#define DP_S_MOD     5
#define DP_S_CONV    6
#define DP_S_DONE    7

// format flags - Bits 
#define DP_F_MINUS    (1 << 0)                    // Left justify alignment
#define DP_F_PLUS     (1 << 1)                    // Show plus sign prefix for positive value
#define DP_F_SPACE    (1 << 2)                    // Prefix with space if positive to align with signed negative values
#define DP_F_NUM      (1 << 3)                    // Trailing zeros (Currently not supported)
#define DP_F_ZERO     (1 << 4)                    // Leading zeros
#define DP_F_UP       (1 << 5)                    // Upper case letters in numeric strings eg. #INF instead of #inf
#define DP_F_UNSIGNED (1 << 6)                    // Numeric parameter value is unsigned
#define DP_F_SEPARATORS (1 << 7)                  // EXTENSION: decimal separators (eg. 23,456.34)

// Conversion Flags 
#define DP_C_SHORT   1
#define DP_C_LONG    2
#define DP_C_LDOUBLE 3

#define char_to_int(p) (p - '0')
#define MAX(p,q) ((p >= q) ? p : q)
#define MIN(p,q) ((p <= q) ? p : q)

// The syntax for a format is "%[flags][width][.precision]type".
static int dopr(char *buffer, size_t maxlen, const char *format, ArgList& a_argList) //va_list args)
{
  char ch;
#if !UDFS_USE_64BIT
  long ivalue;
#endif
#if !UDFS_USE_MOREFLOAT
  LDOUBLE fvalue;
#endif
  int min;
  int max;
  int state;
  int flags;
  int cflags;
  int total;
  size_t currlen;

  state = DP_S_DEFAULT;
  currlen = flags = cflags = min = 0;
  max = -1;
  ch = *format++;
  total = 0;

  while (state != DP_S_DONE)
  {
    if (ch == '\0')
      state = DP_S_DONE;

    switch(state) 
    {
    case DP_S_DEFAULT:
      if (ch == '%') 
        state = DP_S_FLAGS;
      else 
        total += dopr_outch (buffer, &currlen, maxlen, ch);
      ch = *format++;
      break;
    case DP_S_FLAGS:
      switch (ch) 
      {
      case '-':
        flags |= DP_F_MINUS;
        ch = *format++;
        break;
      case '+':
        flags |= DP_F_PLUS;
        ch = *format++;
        break;
      case ' ':
        flags |= DP_F_SPACE;
        ch = *format++;
        break;
      case '#':
        flags |= DP_F_NUM;
        ch = *format++;
        break;
      case '0':
        flags |= DP_F_ZERO;
        ch = *format++;
        break;
     case '\'': // Most common syntax for sprintf with comma is singel quote
     case ',':
        flags |= DP_F_SEPARATORS;
        ch = *format++;
        break;
     default:
        state = DP_S_MIN;
        break;
      }
      break;
    case DP_S_MIN:
      if (isdigit(ch)) 
      {
        min = 10*min + char_to_int (ch);
        ch = *format++;
      } 
      else if (ch == '*') 
      {
        min = a_argList.GetNext().m_valueInt32; //va_arg (args, int);
        ch = *format++;
        state = DP_S_DOT;
      } 
      else 
        state = DP_S_DOT;
      break;
    case DP_S_DOT:
      if (ch == '.') 
      {
        state = DP_S_MAX;
        ch = *format++;
      } 
      else 
        state = DP_S_MOD;
      break;
    case DP_S_MAX:
      if (isdigit(ch)) 
      {
        if (max < 0)
          max = 0;
        max = 10*max + char_to_int (ch);
        ch = *format++;
      } 
      else if (ch == '*') 
      {
        max = a_argList.GetNext().m_valueInt32; //va_arg (args, int);
        ch = *format++;
        state = DP_S_MOD;
      } 
      else 
        state = DP_S_MOD;
      break;
    case DP_S_MOD:
      // Currently, we don't support Long Long, bummer 
      switch (ch) 
      {
      case 'h':
        cflags = DP_C_SHORT;
        ch = *format++;
        break;
      case 'l':
        cflags = DP_C_LONG;
        ch = *format++;
        break;
      case 'L':
        cflags = DP_C_LDOUBLE;
        ch = *format++;
        break;
      default:
        break;
      }
      state = DP_S_CONV;
      break;
    case DP_S_CONV:
      switch (ch) 
      {
      case 'd':
      case 'i':
#if UDFS_USE_64BIT //GD Just use maximum precision for type
        total += fmtint_64(buffer, &currlen, maxlen, a_argList.GetNext().AsInt64(), 10, min, max, flags);
#else
        if (cflags == DP_C_SHORT) 
          ivalue = a_argList.GetNext().m_valueInt16; //va_arg (args, short int);
        else if (cflags == DP_C_LONG)
          ivalue = a_argList.GetNext().m_valueInt32; //va_arg (args, long int);
        else
          ivalue = a_argList.GetNext().m_valueInt32; //va_arg (args, int);
        total += fmtint(buffer, &currlen, maxlen, ivalue, 10, min, max, flags);
#endif
        break;
      case 'o':
        flags |= DP_F_UNSIGNED;
#if UDFS_USE_64BIT //GD Just use maximum precision for type
        total += fmtint_64(buffer, &currlen, maxlen, a_argList.GetNext().AsInt64(), 8, min, max, flags);
#else
        if (cflags == DP_C_SHORT)
          ivalue = a_argList.GetNext().m_valueUInt16; //va_arg (args, unsigned short int);
        else if (cflags == DP_C_LONG)
          ivalue = a_argList.GetNext().m_valueUInt32; //va_arg (args, unsigned long int);
        else
          ivalue = a_argList.GetNext().m_valueUInt32; //va_arg (args, unsigned int);
        total += fmtint(buffer, &currlen, maxlen, ivalue, 8, min, max, flags);
#endif
        break;
      case 'u':
        flags |= DP_F_UNSIGNED;
#if UDFS_USE_64BIT //GD Just use maximum precision for type
        total += fmtint_64(buffer, &currlen, maxlen, a_argList.GetNext().AsInt64(), 10, min, max, flags);
#else
        if (cflags == DP_C_SHORT)
          ivalue = a_argList.GetNext().m_valueUInt16; //va_arg (args, unsigned short int);
        else if (cflags == DP_C_LONG)
          ivalue = a_argList.GetNext().m_valueUInt32; //va_arg (args, unsigned long int);
        else
          ivalue = a_argList.GetNext().m_valueUInt32; //va_arg (args, unsigned int);
        total += fmtint(buffer, &currlen, maxlen, ivalue, 10, min, max, flags);
#endif
        break;
      case 'X':
        flags |= DP_F_UP;
      case 'x':
        flags |= DP_F_UNSIGNED;
#if UDFS_USE_64BIT //GD Just use maximum precision for type
        total += fmtint_64(buffer, &currlen, maxlen, a_argList.GetNext().AsInt64(), 16, min, max, flags);
#else
        if (cflags == DP_C_SHORT)
          ivalue = a_argList.GetNext().m_valueUInt16; //va_arg (args, unsigned short int);
        else if (cflags == DP_C_LONG)
          ivalue = a_argList.GetNext().m_valueUInt32;//va_arg (args, unsigned long int);
        else
          ivalue = a_argList.GetNext().m_valueUInt32; //va_arg (args, unsigned int);
        total += fmtint(buffer, &currlen, maxlen, ivalue, 16, min, max, flags);
#endif
        break;
      case 'f':
#if UDFS_USE_MOREFLOAT //GD Just use maximum precision for type 
        {
          fsFloat64 fValue = a_argList.GetNext().AsFloat64();
          if( isfpexception(fValue) )
          {
            total += fmtfp_exception(buffer, &currlen, maxlen, fValue, min, max, flags | DP_F_UP); // Note, using upper case as default for float exception format
          }
          else
          {
            total += fmtfp64(buffer, &currlen, maxlen, fValue, min, max, flags);
          }
        }
#else
        if (cflags == DP_C_LDOUBLE)
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, LDOUBLE);
        else
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, double);
        // um, floating point? 
        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
#endif
        break;
      case 'E':
        flags |= DP_F_UP;
      case 'e':
#if UDFS_USE_MOREFLOAT //GD Just use maximum precision for type
          total += fmtfp64_exp(buffer, &currlen, maxlen, a_argList.GetNext().AsFloat64(), min, max, flags);
#else
        if (cflags == DP_C_LDOUBLE)
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, LDOUBLE);
        else
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, double);
        // um, floating point? 
        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
#endif
        break;
      case 'G':
        flags |= DP_F_UP;
      case 'g':
#if UDFS_USE_MOREFLOAT //GD Just use maximum precision for type
        {
          total += fmtfp64_gen(buffer, &currlen, maxlen, a_argList.GetNext().AsFloat64(), min, max, flags); 
        }
#else
        if (cflags == DP_C_LDOUBLE)
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, LDOUBLE);
        else
          fvalue = a_argList.GetNext().AsFloat64(); //va_arg (args, double);
        // um, floating point? 
        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
#endif
        break;
      case 'c':
        total += dopr_outch(buffer, &currlen, maxlen, (char)a_argList.GetNext().AsInt32()); //va_arg (args, int));
        break;
      case 's':
      {
        const Arg& curArg = a_argList.GetNext();
        const char* cstringPtr = curArg.m_valueCString;
        if( !curArg.IsCString() ) // Check valid string pointer type
        {
#if 0 // MAYBE Be forgiving and output values for known types?
          if( curArg.IsFloat() )
          {
            total += fmtfp64_exp(buffer, &currlen, maxlen, curArg.AsFloat64(), min, max, flags); // Use max precision float
            break;
          }
          else if( curArg.IsInteger() )
          {
            total += fmtint_64(buffer, &currlen, maxlen, curArg.AsInt64(), 10, min, max, flags); // Use max precision int
            break;
          }
          else
#endif // Known value types
          {
            // NOTE: If we wanted to be extra smart here we could try to convert integers into strings. Non-integer values would still be errors.
            cstringPtr = "#err#"; // Insert error tag
          }
        }
        total += fmtstr(buffer, &currlen, maxlen, cstringPtr, flags, min, max);
        break;
      }
      case 'p':
#if 1 //GD Just use maximum precision for type
        total += fmtint_64(buffer, &currlen, maxlen, (fsUIntPtr)(const char*)a_argList.GetNext().m_valueConstPtr, 16, min, max, flags);
#else
        total += fmtint(buffer, &currlen, maxlen, (long)(const char*)a_argList.GetNext().m_valueConstPtr, 16, min, max, flags);
#endif
        break;
#if 0 // GD I don't think we want to support this... (From help) 'Number of characters successfully written so far to the stream or buffer; this value is stored in the integer whose address is given as the argument'
      case 'n':
        if (cflags == DP_C_SHORT) 
        {
          short int *num;
          num = (short int *)a_argList.GetNext().m_valuePtr; //va_arg (args, short int *);
          *num = currlen;
        } 
        else if (cflags == DP_C_LONG) 
        {
          long int *num;
          num = (long int *)a_argList.GetNext().m_valuePtr; //va_arg (args, long int *);
          *num = currlen;
        } 
        else 
        {
          int *num;
          num = (int *)a_argList.GetNext().m_valuePtr; //va_arg (args, int *);
          *num = currlen;
        }
        break;
#endif
      case '%':
        total += dopr_outch(buffer, &currlen, maxlen, ch);
        break;
      case 'w':
        // not supported yet, treat as next char 
        ch = *format++;
        break;
      default:
        // Unknown, skip 
        break;
      }
      ch = *format++;
      state = DP_S_DEFAULT;
      flags = cflags = min = 0;
      max = -1;
      break;
    case DP_S_DONE:
      break;
    default:
      // hmm? 
      break; // some picky compilers need this 
    }
  }
  if (buffer != NULL)
  {
    if (currlen < maxlen - 1) 
      buffer[currlen] = '\0';
    else 
      buffer[maxlen - 1] = '\0';
  }
  return total;
}

static int fmtstr(char *buffer, size_t *currlen, size_t maxlen,
                  const char *value, int flags, int min, int max)
{
  int padlen, strln;     // amount to pad 
  int cnt = 0;
  int total = 0;
  
  if (value == 0)
  {
    value = "<NULL>";
  }

  for (strln = 0; value[strln]; ++strln); // strlen
  if (max >= 0 && max < strln)
    strln = max;
  padlen = min - strln;
  if (padlen < 0) 
    padlen = 0;
  if (flags & DP_F_MINUS) 
    padlen = -padlen; // Left Justify

  while (padlen > 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    --padlen;
  }
  while (*value && ((max < 0) || (cnt < max)))
  {
    total += dopr_outch(buffer, currlen, maxlen, *value++);
    ++cnt;
  }
  while (padlen < 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    ++padlen;
  }
  return total;
}

// Have to handle DP_F_NUM (ie 0x and 0 alternates) 

static int fmtint_64(char *buffer, size_t *currlen, size_t maxlen,
                     fsInt64 a_value, int base, int min, int max, int flags)
{
  fsChar signvalue = 0;
  fsUInt64 uvalue;
  const int MAX_CONVERT_CHARS = 64;
  char convert[MAX_CONVERT_CHARS];
  int place = 0;
  int spadlen = 0; // amount to space pad 
  int zpadlen = 0; // amount to zero pad 
  int caps = 0;
  int total = 0;

  if (max < 0)
    max = 0;

  uvalue = a_value;

  if(!(flags & DP_F_UNSIGNED))
  {
    if( a_value < 0 ) 
    {
      signvalue = '-';
      uvalue = -a_value;
    }
    else
      if (flags & DP_F_PLUS)  // Do a sign (+/i) 
        signvalue = '+';
      else
        if (flags & DP_F_SPACE)
          signvalue = ' ';
  }

  if (flags & DP_F_UP) caps = 1; // Should characters be upper case? 

  int digitIndex = 0;
  do 
  {
    convert[place++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")
      [uvalue % (unsigned)base  ];
    uvalue = (uvalue / (unsigned)base );

    if( (flags & DP_F_SEPARATORS) && (base == 10) && (uvalue > 0) ) // Number format
    {
      ++digitIndex;
      if( (digitIndex % 3) == 0 ) // Insert comma every 3 places
      {
        convert[place++] = ',';
      }
    }
  } while(uvalue && (place < MAX_CONVERT_CHARS));
  if (place == MAX_CONVERT_CHARS) place--;
  convert[place] = 0;

  zpadlen = max - place;
  spadlen = min - MAX (max, place) - (signvalue ? 1 : 0);
  if (zpadlen < 0) zpadlen = 0;
  if (spadlen < 0) spadlen = 0;
  if (flags & DP_F_ZERO)
  {
    zpadlen = MAX(zpadlen, spadlen);
    spadlen = 0;
  }
  if (flags & DP_F_MINUS) 
    spadlen = -spadlen; // Left Justify 

#ifdef DEBUG_SNPRINTF
  dprint (1, (debugfile, "zpad: %d, spad: %d, min: %d, max: %d, place: %d\n", zpadlen, spadlen, min, max, place));
#endif

  // Spaces 
  while (spadlen > 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    --spadlen;
  }

  // Sign 
  if (signvalue) 
    total += dopr_outch(buffer, currlen, maxlen, signvalue);

  // Zeros 
  if (zpadlen > 0) 
  {
    while (zpadlen > 0)
    {
      total += dopr_outch(buffer, currlen, maxlen, '0');
      --zpadlen;
    }
  }

  // Digits 
  while (place > 0) 
    total += dopr_outch(buffer, currlen, maxlen, convert[--place]);

  // Left Justified spaces 
  while (spadlen < 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    ++spadlen;
  }

  return total;
}


static int fmtint(char *buffer, size_t *currlen, size_t maxlen,
                  long value, int base, int min, int max, int flags)
{
  fsChar signvalue = 0;
  unsigned long uvalue;
  const int MAX_CONVERT_CHARS = 64;
  char convert[MAX_CONVERT_CHARS];
  int place = 0;
  int spadlen = 0; // amount to space pad 
  int zpadlen = 0; // amount to zero pad 
  int caps = 0;
  int total = 0;

  if (max < 0)
    max = 0;

  uvalue = value;

  if(!(flags & DP_F_UNSIGNED))
  {
    if( value < 0 ) {
      signvalue = '-';
      uvalue = -value;
    }
    else
      if (flags & DP_F_PLUS)  // Do a sign (+/i) 
        signvalue = '+';
      else
        if (flags & DP_F_SPACE)
          signvalue = ' ';
  }

  if (flags & DP_F_UP) caps = 1; // Should characters be upper case? 

  int digitIndex = 0;
  do 
  {
    convert[place++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")
      [uvalue % (unsigned)base  ];
    uvalue = (uvalue / (unsigned)base );

    if( (flags & DP_F_SEPARATORS) && (base == 10) && (uvalue > 0) ) // Number format
    {
      ++digitIndex;
      if( (digitIndex % 3) == 0 ) // Insert comma every 3 places
      {
        convert[place++] = ',';
      }
    }
  } while(uvalue && (place < MAX_CONVERT_CHARS));
  if (place == MAX_CONVERT_CHARS) place--;
  convert[place] = 0;

  zpadlen = max - place;
  spadlen = min - MAX (max, place) - (signvalue ? 1 : 0);
  if (zpadlen < 0) zpadlen = 0;
  if (spadlen < 0) spadlen = 0;
  if (flags & DP_F_ZERO)
  {
    zpadlen = MAX(zpadlen, spadlen);
    spadlen = 0;
  }
  if (flags & DP_F_MINUS) 
    spadlen = -spadlen; // Left Justify 

#ifdef DEBUG_SNPRINTF
  dprint (1, (debugfile, "zpad: %d, spad: %d, min: %d, max: %d, place: %d\n", zpadlen, spadlen, min, max, place));
#endif

  // Spaces 
  while (spadlen > 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    --spadlen;
  }

  // Sign 
  if (signvalue) 
    total += dopr_outch(buffer, currlen, maxlen, signvalue);

  // Zeros 
  if (zpadlen > 0) 
  {
    while (zpadlen > 0)
    {
      total += dopr_outch(buffer, currlen, maxlen, '0');
      --zpadlen;
    }
  }

  // Digits 
  while (place > 0) 
    total += dopr_outch(buffer, currlen, maxlen, convert[--place]);

  // Left Justified spaces 
  while (spadlen < 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    ++spadlen;
  }

  return total;
}


static LDOUBLE abs_val(LDOUBLE value)
{
  LDOUBLE result = value;

  if (value < 0)
    result = -value;

  return result;
}

static LDOUBLE pow10(int exp)
{
  LDOUBLE result = 1;

  while (exp)
  {
    result *= 10;
    exp--;
  }
  
  return result;
}

static long round(LDOUBLE value)
{
#if 1 // This produces better rounded values than the original
  long intpart;
  intpart = (long)floor(value+0.5); // Guarantee int conversion method with floor first
  return intpart;
#else // Original code
  long intpart;

  intpart = (long)value;
  value = value - intpart;
  if (value >= 0.5)
    intpart++;

  return intpart;
#endif
}

static fsInt64 round64(LDOUBLE value)
{
#if 1 // This produces better rounded values than the original
  fsInt64 intpart;
  intpart = (fsInt64)floor(value+0.5); // Guarantee int conversion method with floor first
  return intpart;
#else // Original code
  fsInt64 intpart;

  intpart = (fsInt64)value;
  value = value - intpart;
  if (value >= 0.5)
    intpart++;

  return intpart;
#endif
}


// Is floating point exception format
static int isfpexception(LDOUBLE fvalue)
{
  if( FS_FLOAT64_IS_NAN(fvalue) || FS_FLOAT64_IS_INF(fvalue) || FS_FLOAT64_IS_IND(fvalue) )
  {
    return true;
  }
  return false;
}


// Format floating point exceptions
static int fmtfp_exception(char *buffer, size_t *currlen, size_t maxlen,
                    LDOUBLE fvalue, int min, int max, int flags)
{
  // GD handle Inf, Nan, Ind non-float 
  int total = 0;

  if( FS_FLOAT64_IS_IND(fvalue) ) // NOTE: Test Indefinites first or they will just be classified NaN
  {
    if( FS_FLOAT64_SIGN_BIT_SET(fvalue) )
    {
      total += dopr_outch(buffer, currlen, maxlen, '-');
    }
    total += dopr_outch(buffer, currlen, maxlen, '1');
    total += dopr_outch(buffer, currlen, maxlen, '.');
    total += dopr_outch(buffer, currlen, maxlen, '#');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'I' : 'i');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'N' : 'n');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'D' : 'd');
  }
  else if( FS_FLOAT64_IS_NAN(fvalue) )
  {
    total += dopr_outch(buffer, currlen, maxlen, '1');
    total += dopr_outch(buffer, currlen, maxlen, '.');
    total += dopr_outch(buffer, currlen, maxlen, '#');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'N' : 'n');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'A' : 'a');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'N' : 'n');
  }
  else if( FS_FLOAT64_IS_INF(fvalue) )
  {
    if( FS_FLOAT64_SIGN_BIT_SET(fvalue) )
    {
      total += dopr_outch(buffer, currlen, maxlen, '-');
    }
    total += dopr_outch(buffer, currlen, maxlen, '1');
    total += dopr_outch(buffer, currlen, maxlen, '.');
    total += dopr_outch(buffer, currlen, maxlen, '#');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'I' : 'i');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'N' : 'n');
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'F' : 'f');
  }

  return total;
}


static int fmtfp_gen(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fValue, int min, int max, int flags, bool a_checkFPException)
{
  // NOTE: Although the current behavior is okay, the actual spec says 'g' should return the shorter representation of 'f' and 'e' formats.
  //       I notice the behavior is different though eg. %.2f, %.2e, %.2g when value is eg. 3.0041.

  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue - fsInt64(fValue) == 0.0 ) // If it looks like an integer, treat it like one
  {
    total += fmtint_64(buffer, currlen, maxlen, (fsInt64)fValue, 10, min, 0, flags); // NOTE: Don't use max or we will zero pad
  }
  else if( abs(fValue) < 0.00001 || abs(fValue) > 9999.9999) // Only in these ranges (note no zero issues here)
  {
    // Quick and dirty hack to get some scientific style numbers
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
              numExp = Utils::Math_RoundZero(numExp); // Round down (toward zero, still signed)
    fsFloat64 numDiv = pow(10.0, numExp);
    fsFloat64 numMant = fValue / numDiv;
            
    total += fmtfp(buffer, currlen, maxlen, numMant, 0, max, 0, false, false);                   // Mantissa (signed)
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'E' : 'e');   // Exponent symbol
    total += fmtint(buffer, currlen, maxlen, (int)numExp, 10, 0, -1, 0);           // Exponent (signed)
  }
  else
  {
    total += fmtfp(buffer, currlen, maxlen, fValue, min, max, flags, false, false); 
  }

  return total;
}


static int fmtfp_exp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fValue, int min, int max, int flags, bool a_checkFPException)
{
  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue != 0.0 ) // Avoid zero / inf issues
  {
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
              numExp = Utils::Math_RoundZero(numExp); // Round down (toward zero, still signed)
    fsFloat64 numDiv = pow(10.0, numExp);
    fsFloat64 numMant = fValue / numDiv;

    total += fmtfp(buffer, currlen, maxlen, numMant, 0, max, 0);                   // Mantissa (signed)
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'E' : 'e');   // Exponent symbol
    total += fmtint(buffer, currlen, maxlen, (int)numExp, 10, 0, -1, 0);           // Exponent (signed)
  }
  else
  {
    total += fmtfp(buffer, currlen, maxlen, fValue, min, max, flags);
  }

  return total;
}


static int fmtfp(char *buffer, size_t *currlen, size_t maxlen,
                 LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException, bool a_allowTrailingZeros)
{
  fsChar signvalue = 0;
  LDOUBLE ufvalue;
  const int MAX_CONVERT_CHARS = 64;
  char iconvert[MAX_CONVERT_CHARS];
  char fconvert[MAX_CONVERT_CHARS];
  int iplace = 0;
  int fplace = 0;
  int padlen = 0; // amount to pad 
  int zpadlen = 0; 
  int caps = 0;
  int total = 0;
  long intpart;
  long fracpart;

  if( a_checkFPException && isfpexception(fvalue) ) // Check for fp-execption
  {
    return fmtfp_exception(buffer, currlen, maxlen, fvalue, min, max, flags);
  }

  // 
  // AIX manpage says the default is 0, but Solaris says the default
  // is 6, and sprintf on AIX defaults to 6
  //
  if (max < 0)
    max = 6;

  ufvalue = abs_val(fvalue);

  if (fvalue < 0)
    signvalue = '-';
  else
    if (flags & DP_F_PLUS)  // Do a sign (+/i) 
      signvalue = '+';
    else
      if (flags & DP_F_SPACE)
        signvalue = ' ';

#if 0
  if (flags & DP_F_UP) caps = 1; // Should characters be upper case? 
#endif

  intpart = (long)ufvalue;

  // 
  // Sorry, we only support 9 digits past the decimal because of our 
  // conversion method
  //
  if (max > 9)
    max = 9;

  // We "cheat" by converting the fractional part to integer by
  // multiplying by a factor of 10
  //
  
  LDOUBLE pow10Max = pow10(max);
  fracpart = round( pow10Max * (ufvalue - intpart) );

  if (fracpart >= pow10Max)
  {
    intpart++;
    fracpart -= (long)pow10Max;
  }

  fracpart += (long)pow10Max; // Add leading '1' to allow leading zeros for fraction part

#ifdef DEBUG_SNPRINTF
  dprint (1, (debugfile, "fmtfp: %f =? %d.%d\n", fvalue, intpart, fracpart));
#endif

  // Convert integer part 
  int digitIndex = 0;
  do 
  {
    iconvert[iplace++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")[intpart % 10];
    intpart = (intpart / 10);

    if( (flags & DP_F_SEPARATORS) && (intpart > 0) ) // Number format
    {
      ++digitIndex;
      if( (digitIndex % 3) == 0 ) // Insert comma every 3 places
      {
        iconvert[iplace++] = ',';
      }
    }

  } while(intpart && (iplace < MAX_CONVERT_CHARS));
  if (iplace == MAX_CONVERT_CHARS) iplace--;
  iconvert[iplace] = 0;

  // Convert fractional part 
#if 1 // Configurable trailing zeros for general numbers
  bool flagTrailingZeros = !a_allowTrailingZeros;
  do 
  {
    int remainder = fracpart % 10;
    if( flagTrailingZeros && remainder )
    {
      flagTrailingZeros = false;
    }
    if( !flagTrailingZeros || remainder )
    {
      fconvert[fplace++] = (caps? "0123456789ABCDEF":"0123456789abcdef")[remainder];
    }
    fracpart = (fracpart / 10);
  } while(fracpart && (fplace < MAX_CONVERT_CHARS));
#else
  do {
    fconvert[fplace++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")[fracpart % 10];
    fracpart = (fracpart / 10);
  } while(fracpart && (fplace < MAX_CONVERT_CHARS));
#endif
  if (fplace == MAX_CONVERT_CHARS) fplace--;
  if (fplace > 0) --fplace; // Rewind 1 char to remove the leading '1'
  fconvert[fplace] = 0;

  //  -1 for decimal point if there are any, another -1 if we are printing a sign 
  padlen = min - iplace - max; 
  if( (max > 0) && (fplace > 0) ) // decimal point
    { padlen -= 1; }
  if( signvalue ) // sign
    { padlen -= 1; }

#if 1 // Configurable trailing zeros for general numbers
  if( a_allowTrailingZeros )
  {
    zpadlen = max - fplace;
  }
#else
  zpadlen = max - fplace;
#endif
  if (zpadlen < 0)
    zpadlen = 0;
  if (padlen < 0) 
    padlen = 0;
  if (flags & DP_F_MINUS) 
    padlen = -padlen; // Left Justify

  if ((flags & DP_F_ZERO) && (padlen > 0)) 
  {
    if (signvalue) 
    {
      total += dopr_outch(buffer, currlen, maxlen, signvalue);
      --padlen;
      signvalue = 0;
    }
    while (padlen > 0)
    {
      total += dopr_outch(buffer, currlen, maxlen, '0');
      --padlen;
    }
  }
  while (padlen > 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    --padlen;
  }
  if (signvalue) 
    total += dopr_outch(buffer, currlen, maxlen, signvalue);

  while (iplace > 0) 
    total += dopr_outch(buffer, currlen, maxlen, iconvert[--iplace]);

  //
  // Decimal point.  This should probably use locale to find the correct
  // char to print out.
  //
  if( (max > 0) && (fplace > 0) )
  {
    total += dopr_outch(buffer, currlen, maxlen, '.');

    while (fplace > 0) 
      total += dopr_outch(buffer, currlen, maxlen, fconvert[--fplace]);
  }

  while (zpadlen > 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, '0');
    --zpadlen;
  }

  while (padlen < 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    ++padlen;
  }

  return total;
}


static int fmtfp64_gen(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fValue, int min, int max, int flags, bool a_checkFPException)
{
  // NOTE: Although the current behavior is okay, the actual spec says 'g' should return the shorter representation of 'f' and 'e' formats.
  //       I notice the behavior is different though eg. %.2f, %.2e, %.2g when value is eg. 3.0041.

  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue - fsInt64(fValue) == 0.0 ) // If it looks like an integer, treat it like one
  {
    total += fmtint_64(buffer, currlen, maxlen, (fsInt64)fValue, 10, min, 0, flags); // NOTE: Don't use max or we will zero pad
  }
  else if( abs(fValue) < 0.00001 || abs(fValue) > 9999.9999) // Only in these ranges (note no zero issues here)
  {
    // Quick and dirty hack to get some scientific style numbers
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
              numExp = Utils::Math_RoundZero(numExp); // Round down (toward zero, still signed)
    fsFloat64 numDiv = pow(10.0, numExp);
    fsFloat64 numMant = fValue / numDiv;
            
    total += fmtfp64(buffer, currlen, maxlen, numMant, 0, max, 0, false, false);                   // Mantissa (signed)
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'E' : 'e');   // Exponent symbol
    total += fmtint(buffer, currlen, maxlen, (int)numExp, 10, 0, -1, 0);           // Exponent (signed)
  }
  else
  {
    total += fmtfp64(buffer, currlen, maxlen, fValue, min, max, flags, false, false); 
  }

  return total;
}


static int fmtfp64_exp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fValue, int min, int max, int flags, bool a_checkFPException)
{
  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue != 0.0 ) // Avoid zero / inf issues
  {
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
              numExp = Utils::Math_RoundZero(numExp); // Round down (toward zero, still signed)
    fsFloat64 numDiv = pow(10.0, numExp);
    fsFloat64 numMant = fValue / numDiv;

    total += fmtfp64(buffer, currlen, maxlen, numMant, 0, max, 0);                   // Mantissa (signed)
    total += dopr_outch(buffer, currlen, maxlen, (flags & DP_F_UP) ? 'E' : 'e');   // Exponent symbol
    total += fmtint(buffer, currlen, maxlen, (int)numExp, 10, 0, -1, 0);           // Exponent (signed)
  }
  else
  {
    total += fmtfp64(buffer, currlen, maxlen, fValue, min, max, flags);
  }

  return total;
}


static int fmtfp64(char *buffer, size_t *currlen, size_t maxlen,
                   LDOUBLE fvalue, int min, int max, int flags, bool a_checkFPException, bool a_allowTrailingZeros)
{
  fsChar signvalue = 0;
  LDOUBLE ufvalue;
  const int MAX_CONVERT_CHARS = 64;
  char iconvert[MAX_CONVERT_CHARS];
  char fconvert[MAX_CONVERT_CHARS];
  int iplace = 0;
  int fplace = 0;
  int padlen = 0; // amount to pad 
  int zpadlen = 0; 
  int caps = 0;
  int total = 0;
  fsInt64 intpart;
  fsInt64 fracpart;

  if( a_checkFPException && isfpexception(fvalue) ) // Check for fp-execption
  {
    return fmtfp_exception(buffer, currlen, maxlen, fvalue, min, max, flags);
  }

  // 
  // AIX manpage says the default is 0, but Solaris says the default
  // is 6, and sprintf on AIX defaults to 6
  //
  if (max < 0)
    max = 6;

  ufvalue = abs_val(fvalue);

  if (fvalue < 0)
    signvalue = '-';
  else
    if (flags & DP_F_PLUS)  // Do a sign (+/i) 
      signvalue = '+';
    else
      if (flags & DP_F_SPACE)
        signvalue = ' ';

#if 0
  if (flags & DP_F_UP) caps = 1; // Should characters be upper case? 
#endif

  intpart = (fsInt64)ufvalue;

  // 
  // Sorry, we only support 16 digits past the decimal because of our conversion method
  // So 1.0000000000000003e-01 won't be preserved. It will round to "0.1" instead of "0.10000000000000003"
  //
  if (max > 16)
    max = 16;

  // We "cheat" by converting the fractional part to integer by
  // multiplying by a factor of 10
  //
  LDOUBLE pow10Max = pow10(max);
  fracpart = round64( pow10Max * (ufvalue - intpart) );

  if (fracpart >= pow10Max)
  {
    intpart++;
    fracpart -= (fsInt64)pow10Max;
  }

  fracpart += (fsInt64)pow10Max; // Add leading '1' to allow leading zeros for fraction part

#ifdef DEBUG_SNPRINTF
  dprint (1, (debugfile, "fmtfp: %f =? %d.%d\n", fvalue, intpart, fracpart));
#endif

  // Convert integer part 
  int digitIndex = 0;
  do 
  {
    iconvert[iplace++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")[intpart % 10];
    intpart = (intpart / 10);

    if( (flags & DP_F_SEPARATORS) && (intpart > 0) ) // Number format
    {
      ++digitIndex;
      if( (digitIndex % 3) == 0 ) // Insert comma every 3 places
      {
        iconvert[iplace++] = ',';
      }
    }

  } while(intpart && (iplace < MAX_CONVERT_CHARS));
  if (iplace == MAX_CONVERT_CHARS) iplace--;
  iconvert[iplace] = 0;

  // Convert fractional part 
#if 1 // Configurable trailing zeros for general numbers
  bool flagTrailingZeros = !a_allowTrailingZeros;
  do 
  {
    int remainder = fracpart % 10;
    if( flagTrailingZeros && remainder )
    {
      flagTrailingZeros = false;
    }
    if( !flagTrailingZeros || remainder )
    {
      fconvert[fplace++] = (caps? "0123456789ABCDEF":"0123456789abcdef")[remainder];
    }
    fracpart = (fracpart / 10);
  } while(fracpart && (fplace < MAX_CONVERT_CHARS));
#else
  do {
    fconvert[fplace++] =
      (caps? "0123456789ABCDEF":"0123456789abcdef")[fracpart % 10];
    fracpart = (fracpart / 10);
  } while(fracpart && (fplace < MAX_CONVERT_CHARS));
#endif
  if (fplace == MAX_CONVERT_CHARS) fplace--;
  if (fplace > 0) --fplace; // Rewind 1 char to remove the leading '1'
  fconvert[fplace] = 0;

  //  -1 for decimal point if there are any, another -1 if we are printing a sign
  padlen = min - iplace - max;
  if( (max > 0) && (fplace > 0) ) // decimal point
    { padlen -= 1; }
  if( signvalue ) // sign
    { padlen -= 1; }

#if 1 // Configurable trailing zeros for general numbers
  if( a_allowTrailingZeros )
  {
    zpadlen = max - fplace;
  }
#else
  zpadlen = max - fplace;
#endif
  if (zpadlen < 0)
    zpadlen = 0;
  if (padlen < 0) 
    padlen = 0;
  if (flags & DP_F_MINUS) 
    padlen = -padlen; // Left Justify 

  if ((flags & DP_F_ZERO) && (padlen > 0)) 
  {
    if (signvalue) 
    {
      total += dopr_outch(buffer, currlen, maxlen, signvalue);
      --padlen;
      signvalue = 0;
    }
    while (padlen > 0)
    {
      total += dopr_outch(buffer, currlen, maxlen, '0');
      --padlen;
    }
  }
  while (padlen > 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    --padlen;
  }
  if (signvalue) 
    total += dopr_outch(buffer, currlen, maxlen, signvalue);

  while (iplace > 0) 
    total += dopr_outch(buffer, currlen, maxlen, iconvert[--iplace]);

  //
  // Decimal point.  This should probably use locale to find the correct
  // char to print out.
  //
  if( (max > 0) && (fplace > 0) )
  {
    total += dopr_outch(buffer, currlen, maxlen, '.');

    while (fplace > 0) 
      total += dopr_outch(buffer, currlen, maxlen, fconvert[--fplace]);
  }

  while (zpadlen > 0)
  {
    total += dopr_outch(buffer, currlen, maxlen, '0');
    --zpadlen;
  }

  while (padlen < 0) 
  {
    total += dopr_outch(buffer, currlen, maxlen, ' ');
    ++padlen;
  }

  return total;
}


static int dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c)
{
  if (*currlen + 1 < maxlen)
    buffer[(*currlen)++] = c;
  return 1;
}

#if 0 //GD

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *str, size_t count, const char *fmt, va_list args)
{
  if (str != NULL)
    str[0] = 0;
  return dopr(str, count, fmt, args);
}
#endif // !HAVE_VSNPRINTF 

#ifndef HAVE_SNPRINTF
// VARARGS3 
#ifdef HAVE_STDARGS
int NEW_snprintf(char *str,size_t count,const char *fmt,...)
#else
int NEW_snprintf(va_dcl va_alist) 
#endif
{
#ifndef HAVE_STDARGS
  char *str;
  size_t count;
  char *fmt;
#endif
  VA_LOCAL_DECL;
  int total;
    
  VA_START (fmt);
  VA_SHIFT (str, char *);
  VA_SHIFT (count, size_t );
  VA_SHIFT (fmt, char *);
  total = vsnprintf(str, count, fmt, ap);
  VA_END;
  return total;
}
#endif // !HAVE_SNPRINTF 

#ifdef TEST_SNPRINTF
#ifndef LONG_STRING
#define LONG_STRING 1024
#endif
int main(void)
{
  char buf1[LONG_STRING];
  char buf2[LONG_STRING];
  char *fp_fmt[] = {
    "%-1.5f",
    "%1.5f",
    "%123.9f",
    "%10.5f",
    "% 10.5f",
    "%+22.9f",
    "%+4.9f",
    "%01.3f",
    "%4f",
    "%3.1f",
    "%3.2f",
    "%.0f",
    "%.1f",
    NULL
  };
  double fp_nums[] = { -1.5, 134.21, 91340.2, 341.1234, 0203.9, 0.96, 0.996, 
    0.9996, 1.996, 4.136, 0};
  char *int_fmt[] = {
    "%-1.5d",
    "%1.5d",
    "%123.9d",
    "%5.5d",
    "%10.5d",
    "% 10.5d",
    "%+22.33d",
    "%01.3d",
    "%4d",
    NULL
  };
  long int_nums[] = { -1, 134, 91340, 341, 0203, 0};
  int x, y;
  int fail = 0;
  int num = 0;

  printf("Testing snprintf format codes against system sprintf...\n");

  for (x = 0; fp_fmt[x] != NULL ; x++)
    for (y = 0; fp_nums[y] != 0 ; y++)
    {
      snprintf(buf1, sizeof(buf1), fp_fmt[x], fp_nums[y]);
      sprintf(buf2, fp_fmt[x], fp_nums[y]);
      if (strcmp(buf1, buf2))
      {
  printf("snprintf doesn't match Format: %s\n\tsnprintf = %s\n\tsprintf  = %s\n", 
      fp_fmt[x], buf1, buf2);
  fail++;
      }
      num++;
    }

  for (x = 0; int_fmt[x] != NULL ; x++)
    for (y = 0; int_nums[y] != 0 ; y++)
    {
      snprintf(buf1, sizeof (buf1), int_fmt[x], int_nums[y]);
      sprintf(buf2, int_fmt[x], int_nums[y]);
      if (strcmp(buf1, buf2))
      {
  printf("snprintf doesn't match Format: %s\n\tsnprintf = %s\n\tsprintf  = %s\n", 
      int_fmt[x], buf1, buf2);
  fail++;
      }
      num++;
    }
  printf("%d tests failed out of %d.\n", fail, num);
}
#endif // SNPRINTF_TEST 

#endif

// GD Our wrapper
fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, ArgList& a_args)
{
  if( a_str != NULL )
  {
    a_str[0] = 0;
  }
  
  return dopr(a_str, a_count, a_fmt, a_args);
}


END_NAMESPACE_FORMATSTRINGLIB