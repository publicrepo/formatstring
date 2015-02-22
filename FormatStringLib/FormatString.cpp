//
// FormatString.cpp
// Format string utility function
// .Net style formatting with support for standard numeric format strings
//
// Based upon a relatively simple and self contained free, open source snprintf implementation.
// Original source: http://www.fiction.net/blong/programs/snprintf.c  (http://www.fiction.net/blong/programs/#snprintf)
// Use at your own risk, based on FOSS code, please observe any replicated copyright notices.
//

#include <iostream> // For standard library string functions
#include "FormatString.h"

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
 

#define LDOUBLE double

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


static int fmt_braced(char* a_buffer, size_t* a_currlen, size_t a_maxlen, 
                      int a_paramIndex, int a_alignment, const char* a_format,
                      ArgList& a_argList);

//
// dopr(): poor man's version of doprintf
//

enum
{
  FORMAT_TYPE_DEFAULT = 0,            // Non specified, use default
  FORMAT_TYPE_CURRENCY,               // Currency
  FORMAT_TYPE_DECIMAL,                // Decimal, looks like signed integer
  FORMAT_TYPE_NUMBER,                 // Number
  FORMAT_TYPE_EXPONENT,               // Floating point, exponent style
  FORMAT_TYPE_FIXEDPOINT,             // Floating point, fixed point style
  FORMAT_TYPE_GENERAL,                // Numeric general
  FORMAT_TYPE_HEXADECIMAL,            // Hexadecimal
  FORMAT_TYPE_STRING,                 // String
  FORMAT_TYPE_PERCENT,                // Percent
  FORMAT_TYPE_CUSTOM                  // Unknown or custom format
};


enum
{
  DP_F_MINUS    = (1 << 0),                    // Left justify alignment
  DP_F_PLUS     = (1 << 1),                    // Show plus sign prefix for positive value
  DP_F_SPACE    = (1 << 2),                    // Prefix with space if positive to align with signed negative values
  DP_F_NUM      = (1 << 3),                    // Trailing zeros (Currently not supported)
  DP_F_ZERO     = (1 << 4),                    // Leading zeros
  DP_F_UP       = (1 << 5),                    // Upper case letters in numeric strings eg. #INF instead of #inf
  DP_F_UNSIGNED = (1 << 6),                    // Numeric parameter value is unsigned
  DP_F_SEPARATORS = (1 << 7),                  // Decimal seperators (eg. 23,456.34)
};


#define char_to_int(p) (p - '0')
#define MAX(p,q) ((p >= q) ? p : q)
#define MIN(p,q) ((p <= q) ? p : q)

// The syntax for a format is "{[param],[alignment]:[format]}".
static int dopr(char *buffer, size_t maxlen, const char *format, ArgList& a_argList)
{
  // Parse states
  enum
  {
    DP_S_DEFAULT = 0,
    DP_S_PARAM,
    DP_S_ALIGNMENT_START,
    DP_S_ALIGNMENT_SIGN,
    DP_S_ALIGNMENT,
    DP_S_FORMAT_START,
    DP_S_FORMAT,
    DP_S_CONVERT,
    DP_S_ERROR,
    DP_S_RESET_AND_CONTINUE,
    DP_S_DONE
  };

  int state = DP_S_DEFAULT;
  size_t currlen = 0;
  char ch = *format++;
  int flags = 0;
  int total = 0;
  int paramIndex = 0;
  int alignment = 0;
  const int MAX_FORMAT_STRING = 256;
  char formatStringBuffer[MAX_FORMAT_STRING] = "";
  size_t currFormatLen = 0;

  while( state != DP_S_DONE )
  {
    if (ch == '\0')
    {
      state = DP_S_DONE;
    }

    switch(state) 
    {
      case DP_S_DEFAULT:
      {
        if( ch == '{' )
        {
          if( *format == '{' ) // If next is same, escape it
          {
            total += dopr_outch(buffer, &currlen, maxlen, ch);
            ch = *format++;
          }
          else
          {
            state = DP_S_PARAM;
          }
        }
        else if( ch == '}' )
        {
          if( *format == '}' ) // If next is same, escape it
          {
            total += dopr_outch(buffer, &currlen, maxlen, ch);
            ch = *format++;
          }
          else
          {
            // Not expecting single close brace here
            state = DP_S_ERROR;
          }
        }
        else 
        {
          total += dopr_outch(buffer, &currlen, maxlen, ch);
        }
        ch = *format++;
        break;
      }
      case DP_S_PARAM:
      {
        if( isdigit(ch) )
        {
          paramIndex = 10 * paramIndex + char_to_int(ch);
          ch = *format++;
        } 
        else
        {
          state = DP_S_ALIGNMENT_START;
        }
        break;
      }
      case DP_S_ALIGNMENT_START:
      {
        if( ch == ',' ) // Optional alignment
        {
          state = DP_S_ALIGNMENT_SIGN;
          ch = *format++;
        }
        else
        {
          state = DP_S_FORMAT_START;
        }
        break;
      }
      case DP_S_ALIGNMENT_SIGN:
      {
        if( ch == '-' ) // Optional Left Justification
        {
          flags |= DP_F_MINUS;
          ch = *format++;
        }
        state = DP_S_ALIGNMENT;
        break;
      }
      case DP_S_ALIGNMENT:
      {
        if( isdigit(ch) )
        {
          alignment = 10 * alignment + char_to_int(ch);
          ch = *format++;
        } 
        else
        {
          // Apply negative sign to alignment value
          if( flags & DP_F_MINUS )
          {
            if( alignment > 0 )
            {
              alignment = -alignment;
            }
          }

          state = DP_S_FORMAT_START;
        }
        break;
      }
      case DP_S_FORMAT_START:
      {
        if( ch == ':' ) // Optional format
        {
          state = DP_S_FORMAT;
          ch = *format++;
        }
        else
        {
          // Expecting end brace
          if( ch == '}' )
          {
            state = DP_S_CONVERT;
            //NOTE: Don't consume the final brace yet
          }
          else
          {
            state = DP_S_ERROR;
          }
        }
        break;
      }
      case DP_S_FORMAT:
      {
        if( ch == '}' )
        {
          if( *format == '}' ) // If next is same, escape it
          {
            dopr_outch(formatStringBuffer, &currFormatLen, MAX_FORMAT_STRING, ch);
            ch = *format++;
            ch = *format++; // NOTE: Consume both as we are not consuming final brace yet
          }
          else
          {
            // Terminate the format string
            if( currFormatLen < MAX_FORMAT_STRING - 1)
            { 
              formatStringBuffer[currFormatLen] = '\0';
            }
            else 
            {
              formatStringBuffer[MAX_FORMAT_STRING - 1] = '\0';
            }

            state = DP_S_CONVERT;
            //NOTE: Don't consume the final brace yet
          }
        }
        else
        {
          dopr_outch(formatStringBuffer, &currFormatLen, MAX_FORMAT_STRING, ch);
          ch = *format++;
        }
        break;
      }
      case DP_S_CONVERT:
      {
        if( ch == '}' )
        {
          total += fmt_braced(buffer, &currlen, maxlen, 
                              paramIndex, alignment, formatStringBuffer,
                              a_argList);
        
          state = DP_S_RESET_AND_CONTINUE;
        }
        else
        {
          state = DP_S_ERROR;
        }
        ch = *format++; // NOTE: Consuming final brace here, in case it was End Of String
        break;
      }
      case DP_S_RESET_AND_CONTINUE:
      {
        state = DP_S_DEFAULT;
        flags = 0;
        paramIndex = 0;
        alignment = 0;
        formatStringBuffer[0] = '\0';
        currFormatLen = 0;
        break;
      }
      case DP_S_ERROR:
      {
        FS_ASSERT(!"format string exception");
        state = DP_S_DONE; // Just exit
        break;
      }
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
    { 
      buffer[currlen] = '\0';
    }
    else 
    {
      buffer[maxlen - 1] = '\0';
    }
  }
  return total;
}


static void ParseStandardNumericFormat(const char* a_formatString, int& a_formatType, int& a_max, int& a_flags)
{
  // Parse states
  enum
  {
    SF_STATE_TYPE_LETTERS,
    SF_STATE_MAX_OR_PRECISION,
    SF_STATE_ERROR,
    SF_STATE_DONE,
  };

  const char* format = a_formatString;
  int state = SF_STATE_TYPE_LETTERS;
  char ch = *format++;

  // Set defaults
  a_formatType = FORMAT_TYPE_DEFAULT;
  a_max = -1;
  // NOTE: Preserve existing flags content
  
  while( state != SF_STATE_DONE )
  {
    if (ch == '\0')
    {
      state = SF_STATE_DONE;
    }

    switch( state )
    {
      case SF_STATE_TYPE_LETTERS:
      {
        switch( ch )
        {
          case 'c': { a_formatType = FORMAT_TYPE_CURRENCY; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'C': { a_formatType = FORMAT_TYPE_CURRENCY; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'd': { a_formatType = FORMAT_TYPE_DECIMAL; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'D': { a_formatType = FORMAT_TYPE_DECIMAL; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'n': { a_formatType = FORMAT_TYPE_NUMBER; a_flags |= DP_F_SEPARATORS; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'N': { a_formatType = FORMAT_TYPE_NUMBER; a_flags |= (DP_F_UP | DP_F_SEPARATORS); state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'f': { a_formatType = FORMAT_TYPE_FIXEDPOINT; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'F': { a_formatType = FORMAT_TYPE_FIXEDPOINT; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'g': { a_formatType = FORMAT_TYPE_GENERAL; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'G': { a_formatType = FORMAT_TYPE_GENERAL; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'e': { a_formatType = FORMAT_TYPE_EXPONENT; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'E': { a_formatType = FORMAT_TYPE_EXPONENT; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'p': { a_formatType = FORMAT_TYPE_PERCENT; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'P': { a_formatType = FORMAT_TYPE_PERCENT; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'x': { a_formatType = FORMAT_TYPE_HEXADECIMAL; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'X': { a_formatType = FORMAT_TYPE_HEXADECIMAL; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 's': { a_formatType = FORMAT_TYPE_STRING; state = SF_STATE_MAX_OR_PRECISION; break; }
          case 'S': { a_formatType = FORMAT_TYPE_STRING; a_flags |= DP_F_UP; state = SF_STATE_MAX_OR_PRECISION; break; }
          default: { a_formatType = FORMAT_TYPE_CUSTOM; state = SF_STATE_DONE; break; } // Don't parse custom format further
        }
        ch = *format++;
        break;
      }
      case SF_STATE_MAX_OR_PRECISION:
      {
        if( isdigit(ch) )
        {
          if( a_max < 0 )
          {
            a_max = 0;
          }
          a_max = 10 * a_max + char_to_int(ch);
          ch = *format++;
        }
        else
        {
          state = SF_STATE_DONE;
        }
        break;
      }
      case SF_STATE_ERROR:
      {
        state = SF_STATE_DONE; // Just exit
        break;
      }
      case SF_STATE_DONE:
      {
        break;
      }
      default:
      {
        FS_ASSERT(0); // Should not get here
        state = SF_STATE_DONE;
        break;
      }
    }
  }
}


static int fmt_braced(char* a_buffer, size_t* a_currlen, size_t a_maxlen, 
                      int a_paramIndex, int a_alignment, const char* a_format,
                      ArgList& a_argList)
{
  int total = 0;
  int flags = 0;

  // Is parameter index valid
  if( a_paramIndex < 0 || a_paramIndex >= a_argList.Count() )
  {
    return total;
  }

  // Helper functions take flags, not negative numbers
  if( a_alignment < 0 )
  {
    flags |= DP_F_MINUS;
    a_alignment = -a_alignment;
  }

  const Arg& param = a_argList.GetAt(a_paramIndex);
  if( param.m_type == Arg::ARG_TYPE_CHAR )
  {
    int formatType = FORMAT_TYPE_DEFAULT;
    int max = -1;
    ParseStandardNumericFormat(a_format, formatType, max, flags);
    
    switch( formatType )
    {
      case FORMAT_TYPE_NUMBER:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 10, a_alignment, max, flags);
        break;
      }
      case FORMAT_TYPE_DECIMAL:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 10, a_alignment, max, flags);
        break;
      }
      case FORMAT_TYPE_HEXADECIMAL:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 16, a_alignment, max, flags);
        break;
      }
      default: // Char as string
      {
        char charString[2] = {param.m_valueChar, 0};
        total += fmtstr(a_buffer, a_currlen, a_maxlen, charString, flags, a_alignment, max);
        break;
      }
    }
  }
  else if( param.IsCString() )
  {
    int formatType = FORMAT_TYPE_DEFAULT;
    int max = -1;
    ParseStandardNumericFormat(a_format, formatType, max, flags);

    switch( formatType )
    {
      case FORMAT_TYPE_HEXADECIMAL:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 16, a_alignment, max, flags);
        break;
      }
      default: // String
      {
        total += fmtstr(a_buffer, a_currlen, a_maxlen, param.m_valueCString, flags, a_alignment, max);
        break;
      }
    }
  }
  else if( param.IsInteger() )
  {
    int formatType = FORMAT_TYPE_DEFAULT;
    int max = -1;
    ParseStandardNumericFormat(a_format, formatType, max, flags);

    switch( formatType )
    {
      case FORMAT_TYPE_CURRENCY:
      {
        total += dopr_outch(a_buffer, a_currlen, a_maxlen, '$'); // NOTE: Not taking into account locale etc.
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 10, a_alignment, max, flags | DP_F_SEPARATORS);
        break;
      }
      case FORMAT_TYPE_HEXADECIMAL:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 16, a_alignment, max, flags);
        break;
      }
      default: // Decimal
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 10, a_alignment, max, flags);
        break;
      }
    }
  }
  else if( param.IsFloat() )
  {
    int formatType = FORMAT_TYPE_DEFAULT;
    int max = -1;
    ParseStandardNumericFormat(a_format, formatType, max, flags);

    fsFloat64 fValue = param.AsFloat64();

    switch( formatType )
    {
      case FORMAT_TYPE_CURRENCY:
      {
        if( max == -1 )
        {
          max = 2; // Default 2 decimal places for percentage
        }
        if( isfpexception(fValue) ) // Check for FP exception
        {
          total += fmtfp_exception(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags | DP_F_UP); // Note, using upper case as default for float exception format
        }
        else
        {
          total += dopr_outch(a_buffer, a_currlen, a_maxlen, '$'); // NOTE: Not taking into account locale etc.
          total += fmtfp(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags | DP_F_SEPARATORS);
        }
        break;
      }
      case FORMAT_TYPE_PERCENT:
      {
        if( max == -1 )
        {
          max = 2; // Default 2 decimal places for percentage
        }
        if( isfpexception(fValue) ) // Check for FP exception
        {
          total += fmtfp_exception(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags | DP_F_UP); // Note, using upper case as default for float exception format
        }
        else
        {
          total += fmtfp(a_buffer, a_currlen, a_maxlen, fValue * 100.0, a_alignment, max, flags);
          total += dopr_outch(a_buffer, a_currlen, a_maxlen, '%');
        }
        break;
      }
      case FORMAT_TYPE_FIXEDPOINT:
      {
        if( isfpexception(fValue) ) // Check for FP exception
        {
          total += fmtfp_exception(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags | DP_F_UP); // Note, using upper case as default for float exception format
        }
        else
        {
          total += fmtfp(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags);
        }
        break;
      }
      case FORMAT_TYPE_EXPONENT:
      {
        total += fmtfp_exp(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags);
        break;
      }
      case FORMAT_TYPE_NUMBER:
      {
        total += fmtfp(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags);
        break;
      }
      default: // General
      {
        total += fmtfp_gen(a_buffer, a_currlen, a_maxlen, fValue, a_alignment, max, flags);
        break;
      }
    }
  }
  else if( param.m_type == Arg::ARG_TYPE_CONST_PTR ||  param.m_type == Arg::ARG_TYPE_NONCONST_PTR)
  {
    int formatType = FORMAT_TYPE_DEFAULT;
    int max = -1;
    ParseStandardNumericFormat(a_format, formatType, max, flags);

    switch( formatType )
    {
      case FORMAT_TYPE_DECIMAL:
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 10, a_alignment, max, flags);
        break;
      }
      default: // Hex
      {
        total += fmtint_64(a_buffer, a_currlen, a_maxlen, param.AsInt64(), 16, a_alignment, max, flags);
        break;
      }
    }
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
    spadlen = -spadlen; // Left Justifty 

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
    spadlen = -spadlen; // Left Justifty 

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
  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue - fsInt64(fValue) == 0.0 ) // If it looks like an integer, treat it like one
  {
    total += fmtint_64(buffer, currlen, maxlen, (fsInt64)fValue, 10, min, 0, flags); // NOTE: Don't use max or we will zero pad
  }
  else if( fValue < 0.0000 || fValue > 9999.9999) // Only in these ranges (note no zero issues here)
  {
    // Quick and dirty hack to get some scientific style numbers
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
    fsFloat64 numDiv = pow(10.0, floor(numExp));
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
    fsFloat64 numDiv = pow(10.0, floor(numExp));
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
  
  fracpart = round( (pow10(max)) * (ufvalue - intpart) );

  if (fracpart >= pow10(max))
  {
    intpart++;
    fracpart -= (long)pow10(max);
  }

  fracpart += (long)(pow10(max)); // Add leading '1' to allow leading zeros for fraction part

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

  //  -1 for decimal point, another -1 if we are printing a sign 
  padlen = min - iplace - max - 1 - ((signvalue) ? 1 : 0); 
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
    padlen = -padlen; // Left Justifty 

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
  int total = 0;

  if( a_checkFPException && isfpexception(fValue) ) // Check for FP exception
  {
    total += fmtfp_exception(buffer, currlen, maxlen, fValue, min, max, flags);
  }
  else if( fValue - fsInt64(fValue) == 0.0 ) // If it looks like an integer, treat it like one
  {
    total += fmtint_64(buffer, currlen, maxlen, (fsInt64)fValue, 10, min, 0, flags); // NOTE: Don't use max or we will zero pad
  }
  else if( fValue < 0.0000 || fValue > 9999.9999) // Only in these ranges (note no zero issues here)
  {
    // Quick and dirty hack to get some scientific style numbers
    fsFloat64 numAbs = abs(fValue);
    fsFloat64 numExp = Utils::Math_LogAnyBase(10.0, numAbs);
    fsFloat64 numDiv = pow(10.0, floor(numExp));
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
    fsFloat64 numDiv = pow(10.0, floor(numExp));
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
  // Sorry, we only support 16 digits past the decimal because of our 
  // conversion method
  //
  if (max > 16)
    max = 16;

  // We "cheat" by converting the fractional part to integer by
  // multiplying by a factor of 10
  //
  fracpart = round64( (pow10(max)) * (ufvalue - intpart) );

  if (fracpart >= pow10(max))
  {
    intpart++;
    fracpart -= (fsInt64)pow10(max);
  }

  fracpart += (fsInt64)(pow10(max)); // Add leading '1' to allow leading zeros for fraction part

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

  //  -1 for decimal point, another -1 if we are printing a sign 
  padlen = min - iplace - max - 1 - ((signvalue) ? 1 : 0); 
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
    padlen = -padlen; // Left Justifty 

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


// GD Our wrapper
fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, ArgList& a_args)
{
  if( a_str != NULL )
  {
    a_str[0] = 0;
  }
  
  return dopr(a_str, a_count, a_fmt, a_args);
}


END_NAMESPACE_FORMATSTRINGLIB