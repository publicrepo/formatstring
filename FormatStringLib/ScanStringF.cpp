//
// ScanStringF.h
// Scan string, scanf style
//
// Inspired by:
//   Plaugers standard C library 1991
//   Libslack 1999
// Note that no code has been directly copied from these sources due to unknown or incompatible licensing
//
// Use at your own risk, based on FOSS code, please observe any replicated copyright notices.
//

#include <iostream> // For standard library string functions
#include "ScanStringF.h"

//
// Uses standard library functions: isdigit, isspace, memchr, strtol, strtoul, strtod
//

#define SCANSTRING_USE_PARSER 0    // Enable to use our parser instead of standard or extended library functions
#if SCANSTRING_USE_PARSER
#include "Lexer.h"
#endif // SCANSTRING_USE_PARSER

BEGIN_NAMESPACE_FORMATSTRINGLIB

fsInt ScanStringF(const fsChar* a_string, const fsChar* a_format, ArgList& a_args)
{
#if SCANSTRING_USE_PARSER
  Lexer parser(LF_NoErrors | LF_NoStringConcat | LF_NoStringEscapeChars | LF_NoDefines); // Parser for numeric types
#endif //SCANSTRING_USE_PARSER

  const fsInt FAIL_CODE = EOF;                    // As per standard, return EOF (-1) on error
  const fsInt MAX_BUFFER_TZ = 1024;               // Buffer size including terminating zero
  const fsInt MAX_BUFFER = MAX_BUFFER_TZ - 1;     // Buffer size excluding terminating zero
  fsChar buffer[MAX_BUFFER_TZ] = "";              // Buffer for sub strings and numeric values for convenience

  const fsChar *format = a_format;
  const fsChar *string = a_string;
  fsInt convertedCount = 0;                       // Number of successful conversions
 
  for( ; *format; ++format ) // While format string chars remain
  {
    if( *format == '%' ) // Found argument reference
    {
      fsInt width = 0;
      fsBool doConvert = true;
 
      if( *++format == '*' ) // Assignment suppression
      {
        ++format = 0;
        doConvert = false;
      }
 
      for( ; isdigit(*format); ++format ) // Parse width
      {
        width *= 10;
        width += *format - '0';
      }
 
      if( *format == 'h' || *format == 'l' || *format == 'L' ) // Output sizes
      {
        format++;  // Skip, we don't make use of explicit size
      }
 
      if( *format != '[' && *format != 'c' && *format != 'n' )
      {
        while (isspace(*string))
        {
          ++string;
        }
      }
 
      switch( *format )
      {
        case 'p': case 'd':  case 'i':  case 'o':  case 'u':  case 'x': case 'X': 
        {
          // Handle integer numbers
          static const fsChar typeSet[] = "diouxXp";
          static const fsInt baseSet[] = { 10, 0, 8, 10, 16, 16, 16 };
          static const fsChar digitSet[] = "0123456789abcdefABCDEF";

          fsChar *curBuf = buffer;
          fsInt base = baseSet[ strchr(typeSet, *format) - typeSet ];
          fsBool digit = false;
          if( width <= 0 || width > MAX_BUFFER )
          {
            width = MAX_BUFFER;
          }
          if( width && (*string == '+' || *string == '-') )
          {
            *curBuf++ = *string++; --width;
          }
          if( width && (*string == '0') )
          {
            *curBuf++ = *string++; --width;
            digit = true;
            if( width && ((*string == 'x' || *string == 'X') && (base == 0 || base == 16)) )  // If src string is hex and format is hex or undefined
            {
              *curBuf++ = *string++; --width;
              base = 16;
            }
            else
            {
              base = 8;
            }
          }

          fsInt setSize = 10; // if ((base == 0) || (base == 10) )
          if( base == 8 )
          {
            setSize = 8;
          }
          else if( base == 16 )
          {
            setSize = 16 + 6;
          }

          while( width && (memchr(digitSet, *string, setSize) != NULL) )
          {
            *curBuf++ = *string++; --width;
            digit = true;
          }
          if( !digit )
          {
            return FAIL_CODE;
          }
          *curBuf = '\0';
          if( doConvert )
          {
            if( *format == 'd' || *format == 'i' )
            {
#if SCANSTRING_USE_PARSER
              parser.OpenFromString(buffer);
              fsInt64 data = parser.ParseInt64();
#else //SCANSTRING_USE_PARSER
              //long data = strtol(buffer, NULL, base);        // String to int32
              fsInt64 data = _strtoi64(buffer, NULL, base);    // String to int64
#endif //SCANSTRING_USE_PARSER

              a_args.GetNext().WriteAsInt64(data);
            }
            else
            {
#if SCANSTRING_USE_PARSER
              parser.OpenFromString(buffer);
              fsUInt64 data = (fsUInt64)parser.ParseInt64();
#else //SCANSTRING_USE_PARSER
              //unsigned long data = strtoul(buffer, NULL, base);  // String to uint32
              fsUInt64 data = _strtoui64(buffer, NULL, base);      // String to uint64
#endif //SCANSTRING_USE_PARSER

              a_args.GetNext().WriteAsUInt64(data);
            }
            ++convertedCount;
          }
          break;
        }
 
        case 'e': case 'E': case 'f': case 'g': case 'G': 
        {
          // Handle floating point numbers
          fsChar *curBuf = buffer;
          fsBool digit = false;
          if( width <= 0 || width > MAX_BUFFER)
          {
            width = MAX_BUFFER;
          }
          if( width && (*string == '+' || *string == '-') )
          {
            *curBuf++ = *string++; --width;
          }
          while( width && isdigit(*string) )
          {
            *curBuf++ = *string++; --width;
            digit = true;
          }
          if( width && (*string == '.') )
          {
            *curBuf++ = *string++; --width;
          }
          while( width && isdigit(*string) )
          {
            *curBuf++ = *string++; --width;
            digit = true;
          }
          if( width && (digit && (*string == 'e' || *string == 'E')) )
          {
            *curBuf++ = *string++; --width;
            if( width && (*string == '+' || *string == '-') )
            {
              *curBuf++ = *string++; --width;
            }
            digit = false;
            while( width && isdigit(*string) )
            {
              *curBuf++ = *string++; --width;
              digit = true;
            }
          }

          if( !digit )
          {
            return FAIL_CODE;
          }
          *curBuf = '\0';
          if( doConvert )
          {
#if SCANSTRING_USE_PARSER
            parser.OpenFromString(buffer);
            fsFloat64 data = parser.ParseFloat64();
#else //SCANSTRING_USE_PARSER
            fsFloat64 data = strtod(buffer, NULL);      // String to float32
#endif //SCANSTRING_USE_PARSER

            a_args.GetNext().WriteAsFloat64(data);

            ++convertedCount;
          }
          break;
        }
 
        // Convert a string
        case 's': 
        {
          // Handle strings (white space delimited)
          fsChar* curBuf = buffer;

          if( width <= 0 || width > MAX_BUFFER ) // Clamp or set default string length
          {
            width = MAX_BUFFER;
          }

          while( width-- && *string && !isspace(*string) )
          {
            if( doConvert )
            {
              *curBuf++ = *string++;
            }
          }
          if( doConvert )
          {
            *curBuf = '\0';
            ++convertedCount;
          }

          a_args.GetNext().WriteString(buffer);

          break;
        }
 
        case '[':
        {
          // Handle strings (char set delimited)
          fsChar* curBuf = buffer;

          if( width <= 0 || width > MAX_BUFFER ) // Clamp or set default string length
          {
            width = MAX_BUFFER;
          }

          fsBool setInverse = false;
          if( *++format == '^' ) // Invert sequence to exclude instead of include
          {
            setInverse = true;
            ++format;
          }

          const fsChar* start = format;
          if( *format == ']' )
          {
            ++start;
          }
          const fsChar* end = strchr(start, ']');
          if( !end )
          {
            return FAIL_CODE; // May want to return converted count here
          }

          size_t setSize = end - format;
          while( width-- && *string )
          {
            if( !setInverse && !memchr(format, *string, setSize) )
            {
              break;
            }
            if( setInverse && memchr(format, *string, setSize) )
            {
              break;
            }
            if( doConvert )
            {
              *curBuf++ = *string++;
            }
          }
          if( doConvert )
          {
            *curBuf = '\0';
            ++convertedCount;
          }
          format = end;

          a_args.GetNext().WriteString(buffer);

          break;
        }
 
        case 'c':
        {
          // Handle character and character arrays

          // NOTE: May want to support multi byte chars AND multi byte char sequences (eg FOURCC)
          //       Would have to handle unicode AND endian conversion for consistency

          fsChar* curBuf = buffer;

          if( width <= 0 )
          {
            width = 1; // Default is single character 
          }
          else if( width > MAX_BUFFER ) // Clamp string length
          {
            width = MAX_BUFFER;
          }

          fsInt origWidth = width;

          while( width-- )
          {
            if( !(*string) )
            {
              return FAIL_CODE;
            }
            if( doConvert )
            {
              *curBuf++ = *string++;
            }
          }
          if( doConvert )
          {
            ++convertedCount;
          }
          
          if( origWidth == 1 )
          {
            a_args.GetNext().WriteChar(buffer[0]);
          }
          else
          {
            a_args.GetNext().WriteString(buffer);
          }

          break;
        }
 
        case 'n':
        {
          // Handle number of characters read so far
          fsIntPtr data = (fsIntPtr)(string - a_string);
          a_args.GetNext().WriteAsInt64(data);

          break;
        }
 
        case '%':
        {
          if( *string++ != '%' )  // Expect reserved character
          {
            return convertedCount;
          }
          break;
        }
 
        default:
        {
          return FAIL_CODE;
        }
      }
    }
    else if( isspace(*format) )
    {
      // Consume whitespace
      while( isspace(format[1]) )
      {
        ++format;
      }
      while( isspace(*string) )
      {
        ++string;
      }
    }
    else
    {
      if( *string++ != *format ) // Expect same characters for format and string
      {
        return convertedCount;
      }
    }
  }
 
  return convertedCount;
}


END_NAMESPACE_FORMATSTRINGLIB