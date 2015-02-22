//
// Utils.cpp
//

#include <math.h> // For log10
#include "Utils.h"

BEGIN_NAMESPACE_FORMATSTRINGLIB

void Utils::StrLib_Copy(fsChar* a_dest, const fsChar* a_source, fsInt a_destMaxBytes)
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


fsFloat32 Utils::Math_LogAnyBase(fsFloat32 a_base, fsFloat32 a_value)
{
  return (fsFloat32)(log10(a_value) / log10(a_base));
}


fsFloat64 Utils::Math_LogAnyBase(fsFloat64 a_base, fsFloat64 a_value)
{
  return (log10(a_value) / log10(a_base));
}


END_NAMESPACE_FORMATSTRINGLIB
