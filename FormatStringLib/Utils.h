#ifndef UTILS_H
#define UTILS_H

//
// Utils.h
// Utilities to support this library
//

#include <assert.h>
#include <math.h> // For log10
#define FS_ASSERT(exp) assert(exp)


// These primitive type names are too handy and nice to use instead of standard types
// Hard coded prefixes reduce symbol clashes without wraping in namespace for now.

#ifdef _MSC_VER // Microsoft compilers
  typedef unsigned __int8       fsUInt8;                  ///< 8bit unsigned integer
  typedef signed __int8         fsInt8;                   ///< 8bit signed integer
  typedef unsigned __int16      fsUInt16;                 ///< 16bit unsigned integer
  typedef signed __int16        fsInt16;                  ///< 16bit signed integer
  typedef unsigned __int32      fsUInt32;                 ///< 32bit unsigned integer
  typedef signed __int32        fsInt32;                  ///< 32bit signed integer
  typedef unsigned __int64      fsUInt64;                 ///< 64bit unsigned integer
  typedef signed __int64        fsInt64;                  ///< 64bit signed integer
  typedef float                 fsFloat32;                ///< 32bit single precision float
  typedef double                fsFloat64;                ///< 64bit double precision float

  typedef char                  fsChar;                   ///< Character for Ansi (or UTF-8) String
  typedef fsUInt16              fsChar16;                 ///< Character for Unicode UTF-16 String
  typedef fsInt32               fsBool;                   ///< Boolean, 0=false !0 = true (Actual type intended to 1) support results from word size bit operations 2) perform well on platform)
  typedef fsUInt32              fsBitField;               ///< Bit field (typically boolean) for tightly packed data eg. fsBitField m_visible:1
  typedef fsInt32               fsInt;                    ///< Machine friendly int, at least 32 bits
  typedef fsUInt32              fsUInt;                   ///< Machine friendly unsigned int, at least 32 bits
  typedef size_t                fsSizeT;                  ///< Size type for memory related sizes, unsigned, at least 32 bits (Usually the same as size_t)
#else // Reasonable default?
  typedef unsigned char         fsUInt8;                  ///< 8bit unsigned integer
  typedef signed char           fsInt8;                   ///< 8bit signed integer
  typedef unsigned short        fsUInt16;                 ///< 16bit unsigned integer
  typedef signed short          fsInt16;                  ///< 16bit signed integer
  typedef unsigned int          fsUInt32;                 ///< 32bit unsigned integer
  typedef signed int            fsInt32;                  ///< 32bit signed integer
  typedef unsigned long long    fsUInt64;                 ///< 64bit unsigned integer
  typedef signed long long      fsInt64;                  ///< 64bit signed integer
  typedef float                 fsFloat32;                ///< 32bit single precision float
  typedef double                fsFloat64;                ///< 64bit double precision float

  typedef char                  fsChar;                   ///< Character for Ansi (or UTF-8) String
  typedef fsUInt16              fsChar16;                 ///< Character for Unicode UTF-16 String
  typedef fsInt32               fsBool;                   ///< Boolean, 0=false !0 = true (Actual type intended to 1) support results from word size bit operations 2) perform well on platform)
  typedef fsUInt32              fsBitField;               ///< Bit field (typically boolean) for tightly packed data eg. fsBitField m_visible:1
  typedef fsInt32               fsInt;                    ///< Machine friendly int, at least 32 bits
  typedef fsUInt32              fsUInt;                   ///< Machine friendly unsigned int, at least 32 bits
  typedef size_t                fsSizeT;                  ///< Size type for memory related sizes, unsigned, at least 32 bits (Usually the same as size_t)
#endif

#ifdef _M_X64
  typedef fsUInt64              fsUIntPtr;                ///< UInt same size as void* for pointer math or machine int. (NOTE: This is Data pointer size, Function and Member Function pointers may vary.)
  typedef fsInt64               fsIntPtr;                 ///< Int same size as void* pointer math or machine int.  (Used for pointer difference calcs eg. where ptrdiff_t would be used)
#else // _M_X64
  typedef fsUInt32              fsUIntPtr;                ///< UInt same size as void* for pointer math or machine int. (NOTE: This is Data pointer size, Function and Member Function pointers may vary.)
  typedef fsInt32               fsIntPtr;                 ///< Int same size as void* pointer math or machine int.
#endif //_M_X64


// Is float Not-A-Number (Produced by invalid operation or invalid float encoding)
#define FS_FLOAT32_IS_NAN(x)            (((*(const fsUInt32*)&(x)) & 0x7f800000) == 0x7f800000)
#define FS_FLOAT64_IS_NAN(x)            (((*(const fsUInt64*)&(x)) & 0x7ff8000000000000) == 0x7ff8000000000000)
// Is float (pos or neg) Infinity (Produced by overflow exp > max)
#define FS_FLOAT32_IS_INF(x)            (((*(const fsUInt32*)&(x)) & 0x7fffffff) == 0x7f800000)
#define FS_FLOAT64_IS_INF(x)            (((*(const fsUInt64*)&(x)) & 0x7fffffffffffffff) == 0x7ff0000000000000)
// Is float Indefinate (Produced by masked FPU exceptions)
#define FS_FLOAT32_IS_IND(x)            ((*(const fsUInt32*)&(x)) == 0xffc00000)
#define FS_FLOAT64_IS_IND(x)            ((*(const fsUInt64*)&(x)) == 0xfff8000000000000)
// Is float Denormalized (Produced by underflow, result biased exp is > allowed exp)
#define FS_FLOAT32_IS_DENORMAL(x)       (((*(const fsUInt32*)&(x)) & 0x7f800000) == 0x00000000 && ((*(const fsUInt32*)&x) & 0x007fffff) != 0x00000000 )
// Is float zero
#define FS_FLOAT32_NOT_ZERO(x)          (((*(const fsUInt32*)&(x)) & (1<<31))
// Is sign bit set
#define FS_FLOAT32_SIGN_BIT_SET(x)      ((*(const fsUInt32*)&(x)) >> 31)
#define FS_FLOAT64_SIGN_BIT_SET(x)      ((*(const fsUInt64*)&(x)) >> 63)
// Is sign bit clear
#define FS_FLOAT32_SIGN_BIT_NOT_SET(x)  ((~(*(const fsUInt32*)&(x))) >> 31)


// In case we want to wrap in namespace
#define NAMESPACE_FORMATSTRINGLIB FormatStringLib                                   // Name space symbol
#define BEGIN_NAMESPACE_FORMATSTRINGLIB namespace NAMESPACE_FORMATSTRINGLIB {       // Used at start of library header file, after includes
#define END_NAMESPACE_FORMATSTRINGLIB }                                             // Used at end of library header file
#define USING_NAMESPACE_FORMATSTRINGLIB using namespace NAMESPACE_FORMATSTRINGLIB;  // Used at start of .cpp module, after includes

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Misc supporting functions
class Utils
{
public:

  // Copy string into limited size buffer. (Will copy as much of source as possible)
  static void StrLib_Copy(fsChar* a_dest, const fsChar* a_source, fsInt a_destMaxBytes)
  {
    FS_ASSERT(a_dest);
    FS_ASSERT(a_source);
    FS_ASSERT(a_destMaxBytes > 0);

    fsChar* curDstPtr = a_dest;
    const fsChar* curSrcPtr = a_source;
    while( *curSrcPtr && (a_destMaxBytes > 1) )
    {
      *curDstPtr = *curSrcPtr;
      ++curSrcPtr;
      ++curDstPtr;
      --a_destMaxBytes;
    }
    *curDstPtr = 0; // Terminating zero
  }


  // Caculate Log from any base.
  // Uses base conversion formula:  Log a(b) / Log a(c) = Log c(b)
  template< typename Real >
  static Real Math_LogAnyBase(Real a_base, Real a_value)
  {
    return (log10(a_value) / log10(a_base));
  }

  // \brief Round toward zero.
  template< typename Real >
  static Real Math_RoundZero(Real a_num)
  {
    if(a_num < 0)
    {
      return (-floor(-a_num));
    }
    else
    {
      return (floor(a_num));
    }
  }

};

END_NAMESPACE_FORMATSTRINGLIB

#endif //UTILS_H