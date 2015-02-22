//
// Arg.cpp
//

#include <string.h>

#include "Arg.h"
#include "Utils.h"

BEGIN_NAMESPACE_FORMATSTRINGLIB

// Init statics and constants
Arg Arg::s_null;


#if HAS_CUSTOM_STRING_CLASS
Arg::Arg(const String& a_value)
{
  m_type = ARG_TYPE_CSTR;
  m_valueCString = (const fsChar*)a_value;
}


Arg::Arg(String* a_value)
{
  m_type = ARG_TYPE_STRING_PTR;
  m_valueStringPtr = a_value;
}
#endif //HAS_CUSTOM_STRING_CLASS

fsBool Arg::WriteString(const fsChar* a_string, fsInt a_maxBytes)
{
  FS_ASSERT(a_maxBytes > 0);

  switch( m_type )
  {
#if HAS_CUSTOM_STRING_CLASS
    case ARG_TYPE_STRING_PTR:
    {
      fsInt numChars = strlen(a_string);
      numChars = Utils::Min(numChars, a_maxBytes-1);
        
      (*m_valueStringPtr) = String(a_string, numChars);
      return true;
    }
#endif //HAS_CUSTOM_STRING_CLASS
    case ARG_TYPE_CHAR_PTR:
    {
      Utils::StrLib_Copy(m_valueCharPtr, a_string, a_maxBytes);
      return true;
    }
    default: 
    {
      return false;
    }
  }
}


fsBool Arg::WriteString(const fsChar* a_string)
{
  return WriteString( a_string, (fsInt)strlen(a_string) + 1 ); // +1 to convert num chars to num bytes, including terminating zero
}


fsBool Arg::WriteChar(const fsChar a_char)
{
  switch( m_type )
  {
    case ARG_TYPE_CHAR_PTR: (*m_valueCharPtr) = (fsInt32)a_char; return true;
#if HAS_CUSTOM_STRING_CLASS
    case ARG_TYPE_STRING_PTR:
    {
      fsChar tempString[2] = {a_char, '\0'};
      (*m_valueStringPtr) = tempString;
      return true;
    }
#endif //HAS_CUSTOM_STRING_CLASS
    case ARG_TYPE_INT32_PTR: (*m_valueInt32Ptr) = (fsInt32)a_char; return true;
    case ARG_TYPE_UINT32_PTR: (*m_valueUInt32Ptr) = (fsUInt32)a_char; return true;
    case ARG_TYPE_INT64_PTR: (*m_valueInt64Ptr) = (fsInt64)a_char; return true;
    case ARG_TYPE_UINT64_PTR: (*m_valueUInt64Ptr) = (fsUInt64)a_char; return true;

    default: 
    {
      return false;
    }
  }
}

END_NAMESPACE_FORMATSTRINGLIB