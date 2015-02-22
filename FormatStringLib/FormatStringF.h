#ifndef FORMATSTRINGF_H
#define FORMATSTRINGF_H

//
// FormatStringF.h
// Format string utility function
//

#include "Arg.h"

//
// The syntax for a format is "%[flags][width][.precision]type"
// Standard library printf / snprintf style formatting
//
// Supported format types:
//   d = Decimal integer
//   i = Decimal integer
//   f = Fixed point real number
//   e/E = Exponential (scientific) real number
//   g/G = General real number
//   x/X = Hexadecimal integer
//   u = Unsigned decimal integer
//   o = Unsigned octol integer
//   s = String
//   c = Character
//   p = Pointer
//
// Supported flags:
//  ' ' - Space
//  '-' - Left justify
//  '+' - Plus sign
//  '0' - Leading zeros
//  '#' - Number (NOT implemented, currently ignored)
//
// width : Optional minimum width of converted value, negative for left justification
//
// precision : Optional maximum precision for fixed point or zero padded values
//
// Eg. FormatStringF(buffer, 512, "Count: %d value: %.3f", 34, 123.456789);
//     output buffer contains: "Count: 34 value: 123.457"
// 
//
// NOTE: Handles 64bit integers and pointers WITHOUT size extended format types (eg. 'llu')
//   
//

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Format string with argument list
fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, ArgList& a_args);


// Format string with variable argument overloads
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt)                                                                                  {  ArgListFixed args;                                                 return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1)                                                                        {  ArgListFixed args(a_p1);                                           return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2)                                                              {  ArgListFixed args(a_p1, a_p2);                                     return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3)                                                    {  ArgListFixed args(a_p1, a_p2, a_p3);                               return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4)                                          {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4);                         return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5)                                {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5);                   return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6)                      {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6);             return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7)            {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7);       return FormatStringF(a_str, a_count, a_fmt,  args); }
inline fsInt FormatStringF(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7, Arg a_p8)  {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7, a_p8); return FormatStringF(a_str, a_count, a_fmt,  args); }

END_NAMESPACE_FORMATSTRINGLIB

#endif //FORMATSTRINGF_H