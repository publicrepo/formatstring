Format String Library

The purpose of this library is to provide simpler, safer updated versions of snprintf and scanf style functions.
Specifically, it adds:
 64bit float and int support
 Type checking
 Type conversion (eg. float <--> int)
 Optional .Net format syntax

Usage is described in the header files for each primary function:
  FormatString - Replacement for snprintf using .net style format string
  FormatStringF - Replacement for snprintf
  ScanStringF - Replacement for scanf

To compile:
  Add the \FormatStringLib files to your project

To use:
  See the \Example\Test.cpp file for example usage
  

This software is Free and Open Source.  Use at your own risk and please observe any copyright notices from contributors.


Change log:

30 Nov 2013
Fix FormatStringF handling %s when not passed string value type. #defined alternate code path to convert non string type. Current behavior is '#err#'.

22 Feb 2014
Fix %.2g incorrect output.

12 May 2016
Fix %2.0f incorrect padding with zero decimal places.
