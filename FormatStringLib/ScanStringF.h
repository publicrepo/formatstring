#ifndef SCANSTRINGF_H
#define SCANSTRINGF_H

//
// ScanStringF.h
// Scan string, scanf style
//

#include "Arg.h"


// The syntax for a format is "%[width]type"
// Standard library scanf style string scan
//
// Supported format types:
//   d/i = Decimal integer
//   e/E/f/F/g/G = Floating point real number with optional sign and exponent
//   x/X = Hexadecimal integer
//   u = Unsigned decimal integer
//   o = Unsigned octol integer
//   s = String, scanned up to the first white-space character (space, tab, newline). To read strings not delimited by space characters, use set of square brackets ([ ]).
//   c = Character
//   p = Pointer
//   [] = Control string.  Scanning continues until character is not in control set.  If first character is '^', effect is reversed.
//
// width : Max characters to read / write for string values, NOT including terminating zero.
//
// Returns number of converted values or EOF (-1) on error
//
// Eg. 
//     int count = 0;
//     float valuef = 0.0f;
//     ScanStringF("Count: 34 value: 123.457", "Count: %d value: %f", &count, &valuef);
// 
// NOTE: Handles 64bit integers and pointers WITHOUT size extended format types (eg. 'llu')
//   
//

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Scan string with argument list
fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, ArgList& a_args);


inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt)                                                                                  {  ArgListFixed args;                                                 return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1)                                                                        {  ArgListFixed args(a_p1);                                           return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2)                                                              {  ArgListFixed args(a_p1, a_p2);                                     return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3)                                                    {  ArgListFixed args(a_p1, a_p2, a_p3);                               return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4)                                          {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4);                         return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5)                                {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5);                   return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6)                      {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6);             return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7)            {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7);       return ScanStringF(a_str, a_fmt, args); }
inline fsInt ScanStringF(const fsChar* a_str, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7, Arg a_p8)  {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7, a_p8); return ScanStringF(a_str, a_fmt, args); }


END_NAMESPACE_FORMATSTRINGLIB

#endif //SCANSTRINGF_H