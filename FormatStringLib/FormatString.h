#ifndef FORMATSTRING_H
#define FORMATSTRING_H

//
// FormatString.h
// Format string utility function
//

#include "Arg.h"

//
// The syntax for a format is "{param[,alignment][:format]}"
// .Net style formatting with support for standard numeric format strings
//
// param - Zero based parameter index
// alignment - Optional minimum width of converted value, negative for left justification
// format - Optional format string eg. 'F3' for fixed point with 3 decimal places of precision
//
// Eg. FormatString(buffer, 512, "Count: {0} value: {1:F3}", 34, 123.456789);
//     output buffer contains: "Count: 34 value: 123.457"
//
// Supported format types:
//   d/D = Decimal integer (auto detect unsigned)
//   f/F = Fixed point real number
//   e/E = Exponential (scientific) real number
//   g/G = General real number
//   x/X = Hexadecimal integer
//   s/S = String
//   p/P = Percentage
//   n/N = Number (only comma group separators)
//   c/C = Currency (only $)
//

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Format string with argument list
fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, ArgList& a_args);


// Format string with variable argument overloads
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt)                                                                                  {  ArgListFixed args;                                                 return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1)                                                                        {  ArgListFixed args(a_p1);                                           return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2)                                                              {  ArgListFixed args(a_p1, a_p2);                                     return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3)                                                    {  ArgListFixed args(a_p1, a_p2, a_p3);                               return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4)                                          {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4);                         return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5)                                {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5);                   return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6)                      {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6);             return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7)            {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7);       return FormatString(a_str, a_count, a_fmt,  args); }
inline fsInt FormatString(fsChar* a_str, size_t a_count, const fsChar* a_fmt, Arg a_p1, Arg a_p2, Arg a_p3, Arg a_p4, Arg a_p5, Arg a_p6, Arg a_p7, Arg a_p8)  {  ArgListFixed args(a_p1, a_p2, a_p3, a_p4, a_p5, a_p6, a_p7, a_p8); return FormatString(a_str, a_count, a_fmt,  args); }

END_NAMESPACE_FORMATSTRINGLIB

#endif FORMATSTRING_H