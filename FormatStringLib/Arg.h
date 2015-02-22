#ifndef ARG_H
#define ARG_H

//
// Arg.h
// Used for boxing arguments / parameters in variable argument lists / overloads
//
// Use at your own risk, please observe any replicated copyright notices in other modules.
//

#include "Utils.h"

#define HAS_CUSTOM_STRING_CLASS 0   // Code referencing custom string class, could be updated to use std::string

// Fwd decls
#if HAS_CUSTOM_STRING_CLASS
class String; // NOTE: Must fwd declare as this header may be included by String.h
#endif //HAS_CUSTOM_STRING_CLASS

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Box a argument / parameter in order to handle variable number of different type arguments / parameters.
// Intended as C++ / Managed language compatible replacement for va_args
class Arg
{
public:
  
  // Supported value types
  enum ArgType
  {
    ARG_TYPE_INVALID = 0,

    ARG_TYPE_CHAR,
    ARG_TYPE_INT8,
    ARG_TYPE_UINT8,
    ARG_TYPE_INT16,
    ARG_TYPE_UINT16,
    ARG_TYPE_INT32,
    ARG_TYPE_UINT32,
    ARG_TYPE_INT64,
    ARG_TYPE_UINT64,
    ARG_TYPE_FLOAT32,
    ARG_TYPE_FLOAT64,
    ARG_TYPE_CONST_PTR,
    ARG_TYPE_CSTR,
    
    ARG_TYPE_NONCONST_PTR,                        // Note, also readable
    ARG_TYPE_CHAR_PTR,                            // Non-const, c-string pointer WARNING: Care must be taken using this type
    ARG_TYPE_INT16_PTR,
    ARG_TYPE_UINT16_PTR,
    ARG_TYPE_INT32_PTR,
    ARG_TYPE_UINT32_PTR,
    ARG_TYPE_INT64_PTR,
    ARG_TYPE_UINT64_PTR,
    ARG_TYPE_FLOAT32_PTR,
    ARG_TYPE_FLOAT64_PTR,
#if HAS_CUSTOM_STRING_CLASS
    ARG_TYPE_STRING_PTR,
#endif //HAS_CUSTOM_STRING_CLASS

    ARG_TYPE_MAX
  };

  ArgType m_type;                                 // Type of value
  union                                           // Union of value types
  {
    // Read only set
    fsChar m_valueChar;
    fsInt8 m_valueInt8;
    fsUInt8 m_valueUInt8;
    fsInt16 m_valueInt16;
    fsUInt16 m_valueUInt16;
    fsInt32 m_valueInt32;
    fsUInt32 m_valueUInt32;
    fsInt64 m_valueInt64;
    fsUInt64 m_valueUInt64;
    fsFloat32 m_valueFloat32;
    fsFloat64 m_valueFloat64;
    const void* m_valueConstPtr;                  // (generic) Pointer to void
    const fsChar* m_valueCString;                 // Pointer to zero terminated string

    // Writable set
    void* m_valueNonConstPtr;                     // (generic) Pointer to void 
    fsChar* m_valueCharPtr;                       // Ptr to char or c-string WARNING: Care must be taken using this type
    fsInt16* m_valueInt16Ptr;                     
    fsUInt16* m_valueUInt16Ptr;
    fsInt32* m_valueInt32Ptr;                     
    fsUInt32* m_valueUInt32Ptr;
    fsInt64* m_valueInt64Ptr;
    fsUInt64* m_valueUInt64Ptr;
    fsFloat32* m_valueFloat32Ptr;
    fsFloat64* m_valueFloat64Ptr;
#if HAS_CUSTOM_STRING_CLASS
    fsString* m_valueStringPtr;
#endif //HAS_CUSTOM_STRING_CLASS
  };

  static Arg Arg::s_null;                     // Static 'null' instance

  // Default constructor
  Arg()
  {
    m_type = ARG_TYPE_INVALID;
    m_valueInt64 = 0;
  }

  // Read only types

  Arg(const fsChar* a_value)
  {
    m_type = ARG_TYPE_CSTR;
    m_valueCString = a_value;
  }
  
#if HAS_CUSTOM_STRING_CLASS
  Arg(const String& a_value);
#endif //HAS_CUSTOM_STRING_CLASS

  Arg(const fsUInt8 a_value)
  {
    m_type = ARG_TYPE_UINT8;
    m_valueInt8 = a_value;
  }

  Arg(const fsInt8 a_value)
  {
    m_type = ARG_TYPE_INT8;
    m_valueInt8 = a_value;
  }

  Arg(const fsUInt16 a_value)
  {
    m_type = ARG_TYPE_UINT16;
    m_valueInt16 = a_value;
  }

  Arg(const fsInt16 a_value)
  {
    m_type = ARG_TYPE_INT16;
    m_valueInt16 = a_value;
  }

  Arg(const fsUInt32 a_value)
  {
    m_type = ARG_TYPE_UINT32;
    m_valueInt32 = a_value;
  }

  Arg(const fsInt32 a_value)
  {
    m_type = ARG_TYPE_INT32;
    m_valueInt32 = a_value;
  }

  Arg(const fsUInt64 a_value)
  {
    m_type = ARG_TYPE_UINT64;
    m_valueInt64 = a_value;
  }

  Arg(const fsInt64 a_value)
  {
    m_type = ARG_TYPE_INT64;
    m_valueInt64 = a_value;
  }

  Arg(const fsFloat32 a_value)
  {
    m_type = ARG_TYPE_FLOAT32;
    m_valueFloat32 = a_value;
  }

  Arg(const fsFloat64 a_value)
  {
    m_type = ARG_TYPE_FLOAT64;
    m_valueFloat64 = a_value;
  }

  Arg(const void* a_value)
  {
    m_type = ARG_TYPE_CONST_PTR;
    m_valueConstPtr = a_value;
  }

  Arg(const fsChar a_value)
  {
    m_type = ARG_TYPE_CHAR;
    m_valueChar = a_value;
  }
  
  // Writable ptr types

  Arg(void* a_value)
  {
    m_type = ARG_TYPE_NONCONST_PTR;
    m_valueNonConstPtr = a_value;
  }

  Arg(fsChar* a_value)
  {
    m_type = ARG_TYPE_CHAR_PTR;
    m_valueCharPtr = a_value;
  }

  Arg(fsInt16* a_value)
  {
    m_type = ARG_TYPE_INT16_PTR;
    m_valueInt16Ptr = a_value;
  }

  Arg(fsUInt16* a_value)
  {
    m_type = ARG_TYPE_UINT16_PTR;
    m_valueUInt16Ptr = a_value;
  }

  Arg(fsInt32* a_value)
  {
    m_type = ARG_TYPE_INT32_PTR;
    m_valueInt32Ptr = a_value;
  }

  Arg(fsUInt32* a_value)
  {
    m_type = ARG_TYPE_UINT32_PTR;
    m_valueUInt32Ptr = a_value;
  }

  Arg(fsInt64* a_value)
  {
    m_type = ARG_TYPE_INT64_PTR;
    m_valueInt64Ptr = a_value;
  }

  Arg(fsUInt64* a_value)
  {
    m_type = ARG_TYPE_UINT64_PTR;
    m_valueUInt64Ptr = a_value;
  }

  Arg(fsFloat32* a_value)
  {
    m_type = ARG_TYPE_FLOAT32_PTR;
    m_valueFloat32Ptr = a_value;
  }

  Arg(fsFloat64* a_value)
  {
    m_type = ARG_TYPE_FLOAT64_PTR;
    m_valueFloat64Ptr = a_value;
  }

#if HAS_CUSTOM_STRING_CLASS
  Arg(String* a_value);
#endif //HAS_CUSTOM_STRING_CLASS
  
  fsBool IsInteger() const
  {
    return (   m_type == ARG_TYPE_CHAR
            || m_type == ARG_TYPE_INT8
            || m_type == ARG_TYPE_UINT8
            || m_type == ARG_TYPE_INT16
            || m_type == ARG_TYPE_UINT16
            || m_type == ARG_TYPE_INT32
            || m_type == ARG_TYPE_UINT32
            || m_type == ARG_TYPE_INT64
            || m_type == ARG_TYPE_UINT64 );
  }

  fsBool IsFloat() const
  {
    return (   m_type == ARG_TYPE_FLOAT32
            || m_type == ARG_TYPE_FLOAT64 );
  }

  fsBool IsNumber() const
  {
    return ( IsFloat() || IsInteger() );
  }
  
  fsBool IsCString() const
  {
    return (m_type == ARG_TYPE_CSTR);
  }

  // Is writable / output type
  fsBool IsOutputType() const
  {
    return (   m_type == ARG_TYPE_CHAR_PTR
            || m_type == ARG_TYPE_INT16_PTR
            || m_type == ARG_TYPE_UINT16_PTR
            || m_type == ARG_TYPE_NONCONST_PTR
            || m_type == ARG_TYPE_INT32_PTR
            || m_type == ARG_TYPE_UINT32_PTR
            || m_type == ARG_TYPE_INT64_PTR
            || m_type == ARG_TYPE_UINT64_PTR
            || m_type == ARG_TYPE_FLOAT32_PTR
            || m_type == ARG_TYPE_FLOAT64_PTR
#if HAS_CUSTOM_STRING_CLASS
            || m_type == ARG_TYPE_STRING_PTR 
#endif //HAS_CUSTOM_STRING_CLASS
            );
  }

  // Return numeric type as 'double'
  fsFloat64 AsFloat64() const
  {
    switch( m_type )
    {
      case ARG_TYPE_CHAR: return (fsFloat64) m_valueChar;
      case ARG_TYPE_INT8: return (fsFloat64) m_valueInt8;
      case ARG_TYPE_UINT8: return (fsFloat64) m_valueUInt8;
      case ARG_TYPE_INT16: return (fsFloat64) m_valueInt16;
      case ARG_TYPE_UINT16: return (fsFloat64) m_valueUInt16;
      case ARG_TYPE_INT32: return (fsFloat64) m_valueInt32;
      case ARG_TYPE_UINT32: return (fsFloat64) m_valueUInt32;
      case ARG_TYPE_INT64: return (fsFloat64) m_valueInt64;
      case ARG_TYPE_UINT64: return (fsFloat64) m_valueUInt64;
      case ARG_TYPE_FLOAT32: return (fsFloat64) m_valueFloat32;
      case ARG_TYPE_FLOAT64: return (fsFloat64) m_valueFloat64;
      default: return 0;
    }
  }
  
  // Return numeric type as 'float'
  fsFloat32 AsFloat32() const
  {
    switch( m_type )
    {
      case ARG_TYPE_CHAR: return (fsFloat32) m_valueChar;
      case ARG_TYPE_INT8: return (fsFloat32) m_valueInt8;
      case ARG_TYPE_UINT8: return (fsFloat32) m_valueUInt8;
      case ARG_TYPE_INT16: return (fsFloat32) m_valueInt16;
      case ARG_TYPE_UINT16: return (fsFloat32) m_valueUInt16;
      case ARG_TYPE_INT32: return (fsFloat32) m_valueInt32;
      case ARG_TYPE_UINT32: return (fsFloat32) m_valueUInt32;
      case ARG_TYPE_INT64: return (fsFloat32) m_valueInt64;
      case ARG_TYPE_UINT64: return (fsFloat32) m_valueUInt64;
      case ARG_TYPE_FLOAT32: return (fsFloat32) m_valueFloat32;
      case ARG_TYPE_FLOAT64: return (fsFloat32) m_valueFloat64;
      default: return 0;
    }
  }

  // Return numeric type as 'int'
  fsInt32 AsInt32() const
  {
    switch( m_type )
    {
      case ARG_TYPE_CHAR: return (fsInt32) m_valueChar;
      case ARG_TYPE_INT8: return (fsInt32) m_valueInt8;
      case ARG_TYPE_UINT8: return (fsInt32) m_valueUInt8;
      case ARG_TYPE_INT16: return (fsInt32) m_valueInt16;
      case ARG_TYPE_UINT16: return (fsInt32) m_valueUInt16;
      case ARG_TYPE_INT32: return (fsInt32) m_valueInt32;
      case ARG_TYPE_UINT32: return (fsInt32) m_valueUInt32;
      case ARG_TYPE_INT64: return (fsInt32) m_valueInt64;
      case ARG_TYPE_UINT64: return (fsInt32) m_valueUInt64;
      case ARG_TYPE_FLOAT32: return (fsInt32) m_valueFloat32;
      case ARG_TYPE_FLOAT64: return (fsInt32) m_valueFloat64;
      case ARG_TYPE_CONST_PTR: return (fsInt32) m_valueConstPtr;
      case ARG_TYPE_NONCONST_PTR: return (fsInt32) m_valueNonConstPtr;
      default: return 0;
    }
  }

  // Return numeric type as 'int'
  fsInt64 AsInt64() const
  {
    switch( m_type )
    {
      case ARG_TYPE_CHAR: return (fsInt64) m_valueChar;
      case ARG_TYPE_INT8: return (fsInt64) m_valueInt8;
      case ARG_TYPE_UINT8: return (fsInt64) m_valueUInt8;
      case ARG_TYPE_INT16: return (fsInt64) m_valueInt16;
      case ARG_TYPE_UINT16: return (fsInt64) m_valueUInt16;
      case ARG_TYPE_INT32: return (fsInt64) m_valueInt32;
      case ARG_TYPE_UINT32: return (fsInt64) m_valueUInt32;
      case ARG_TYPE_INT64: return (fsInt64) m_valueInt64;
      case ARG_TYPE_UINT64: return (fsInt64) m_valueUInt64;
      case ARG_TYPE_FLOAT32: return (fsInt64) m_valueFloat32;
      case ARG_TYPE_FLOAT64: return (fsInt64) m_valueFloat64;
      case ARG_TYPE_CONST_PTR: return (fsInt64)(fsUIntPtr)m_valueConstPtr; // Prevent sign extension
      case ARG_TYPE_NONCONST_PTR: return (fsInt64)(fsUIntPtr)m_valueNonConstPtr; // Prevent sign extension
      default: return 0;
    }
  }

  // Write as float 32 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsFloat32(fsFloat32 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  // Write as float 64 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsFloat64(fsFloat64 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  // Write as int 32 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsInt32(fsInt32 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  // Write as unsigned int 32 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsUInt32(fsUInt32 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  // Write as int 64 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsInt64(fsInt64 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_NONCONST_PTR: (*(fsIntPtr*)m_valueNonConstPtr) = (fsIntPtr)a_value; return true; // Allow generic pointer
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  // Write as unsigned int 64 value to all plausible types
  // WARNING: Output value may be truncated or bit-complemented if it does not fit destination type
  fsBool WriteAsUInt64(fsUInt64 a_value)
  {
    switch( m_type )
    {
      case ARG_TYPE_NONCONST_PTR: (*(fsIntPtr*)m_valueNonConstPtr) = (fsIntPtr)a_value; return true; // Allow generic pointer
      case ARG_TYPE_INT16_PTR: (*m_valueInt16Ptr) = (fsInt16)a_value; return true;
      case ARG_TYPE_UINT16_PTR: (*m_valueUInt16Ptr) = (fsUInt16)a_value; return true;
      case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_value; return true;
      case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_value; return true;
      case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_value; return true;
      case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_value; return true;
      case ARG_TYPE_FLOAT32_PTR: (*m_valueFloat32Ptr) = (fsFloat32)a_value; return true;
      case ARG_TYPE_FLOAT64_PTR: (*m_valueFloat64Ptr) = (fsFloat64)a_value; return true;
      default: return false;
    }
  }

  fsBool WriteString(const fsChar* a_string, fsInt a_maxBytes);

  fsBool WriteString(const fsChar* a_string);

  fsBool WriteChar(const fsChar a_char);

};



// Interface to an argument list
// Note, currently not designed for reentrant use. Could supply iterator or copy operation if ever needed.
class ArgList
{
public:

  //
  // Interface for access
  //

  // Start or reset argument processing.  Returns the number of arguments present.
  virtual fsInt Start()                   = 0;
  // Return the number of arguments
  virtual fsInt Count()                   = 0;
  // Get the next argument.  Returns null arg if out of range.  Advances current arg index.
  virtual Arg& GetNext()                  = 0;
  // Get argument by index.  Returns null arg if out of range.
  virtual Arg& GetAt(fsInt a_index)       = 0;

  //
  // Interface for construction
  //

  // Append arg to list.  Returns true if succeeded.
  virtual fsBool Add(const Arg& a_arg)    = 0;
};


// Fixed size argument list for general use
class ArgListFixed : public ArgList
{
public:
  enum
  {
    MAX_FIXED_ARGS = 20,
  };

  // Default constructor
  ArgListFixed()
  {
    InternalInit();
  }

  // Constructor overloads to allow convenient replacement of vargs with ctor wrapping syntax
  ArgListFixed(const Arg& a_p1)  
  { InternalInit(); Add(a_p1); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2)  
  { InternalInit(); Add(a_p1); Add(a_p2); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3)  
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); }
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); }

  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10, const Arg& a_p11)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8, const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8,const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15, const Arg& a_p16)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15); Add(a_p16);}

  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8,const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15, const Arg& a_p16, const Arg& a_p17)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15); Add(a_p16); Add(a_p17);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8,const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15, const Arg& a_p16, const Arg& a_p17, const Arg& a_p18)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15); Add(a_p16); Add(a_p17); Add(a_p18);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8,const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15, const Arg& a_p16, const Arg& a_p17, const Arg& a_p18, const Arg& a_p19)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15); Add(a_p16); Add(a_p17); Add(a_p18); Add(a_p19);}
  ArgListFixed(const Arg& a_p1, const Arg& a_p2, const Arg& a_p3, const Arg& a_p4, const Arg& a_p5, const Arg& a_p6, const Arg& a_p7, const Arg& a_p8,const Arg& a_p9, const Arg& a_p10, const Arg& a_p11, const Arg& a_p12, const Arg& a_p13, const Arg& a_p14, const Arg& a_p15, const Arg& a_p16, const Arg& a_p17, const Arg& a_p18, const Arg& a_p19, const Arg& a_p20)
  { InternalInit(); Add(a_p1); Add(a_p2); Add(a_p3); Add(a_p4); Add(a_p5); Add(a_p6); Add(a_p7); Add(a_p8); Add(a_p9); Add(a_p10); Add(a_p11); Add(a_p12); Add(a_p13); Add(a_p14); Add(a_p15); Add(a_p16); Add(a_p17); Add(a_p18); Add(a_p19); Add(a_p20);}

  virtual fsInt Start()                     
  {
    m_currentIndex = 0;
    return Count(); 
  }

  virtual fsInt Count()
  {
    return m_count; 
  }

  virtual Arg& GetNext()
  {
    fsInt indexToRet = m_currentIndex;
    ++m_currentIndex;                             // Advance
    if( indexToRet < 0 || indexToRet >= m_count )  // Validate
    {
      return Arg::s_null; 
    }
    return m_args[indexToRet];                    // Return
  }

  virtual Arg& GetAt(fsInt a_index)
  {
    if( a_index < 0 || a_index >= m_count )
    {
      return Arg::s_null; 
    }
    return m_args[a_index];
  }
  
  virtual fsBool Add(const Arg& a_arg) 
  {
    if( m_count < MAX_FIXED_ARGS )
    {
      m_args[m_count] = a_arg;
      ++m_count;
      return true;
    }
    else
    {
      FS_ASSERT( m_count < MAX_FIXED_ARGS ); // Alert, we probably want to know about this
      return false;
    }
  }

protected:

  Arg m_args[MAX_FIXED_ARGS];
  fsInt m_count;
  fsInt m_currentIndex;

  inline void InternalInit()
  {
    m_count = 0;
    m_currentIndex = 0;
  }
};


#if 0 // Only enable if we actually need this
// Variable size argument list for extreme programming!
class ArgListVariable : public ArgList
{
public:

  // Default constructor
  ArgListVariable()
  {
    InternalInit();
  }

  virtual fsInt Start()                     
  {
    m_currentIndex = 0;
    return m_args.GetSize(); 
  }

  virtual fsInt Count()
  {
    return m_args.GetSize();
  }

  virtual Arg& GetNext()
  {
    fsInt indexToRet = m_currentIndex;
    ++m_currentIndex;                             // Advance
    if( indexToRet < 0 || indexToRet >= m_args.GetSize() )  // Validate
    {
      return Arg::s_null; 
    }
    return m_args[indexToRet];                    // Return
  }

  virtual Arg& GetAt(fsInt a_index)
  {
    if( a_index < 0 || a_index >= m_args.GetSize() )
    {
      return Arg::s_null; 
    }
    return m_args[a_index];
  }
  
  virtual fsBool Add(const Arg& a_arg) 
  {
    m_args.Add(a_arg);
    return true;
  }

protected:

  Array<Arg> m_args;
  fsInt m_currentIndex;

  inline void InternalInit()
  {
    m_args.SetSize(0, 8);
    m_currentIndex = 0;
  }
};
#endif

END_NAMESPACE_FORMATSTRINGLIB

#endif //ARG_H