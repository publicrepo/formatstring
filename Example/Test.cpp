#include <stdio.h>

#include "FormatString.h"
#include "FormatStringF.h"
#include "ScanStringF.h"


USING_NAMESPACE_FORMATSTRINGLIB

void main()
{
  const int STR_NUM_BYTES = 256;
  char string1[STR_NUM_BYTES];

  FormatString(string1, STR_NUM_BYTES, "Count: {0} value: {1:F3}", 34, 123.456789);
  printf("%s\n", string1);

  FormatStringF(string1, STR_NUM_BYTES, "Count: %d value: %.3f", 34, 123.456789);
  printf("%s\n", string1);

  int count = 0;
  float value = 0.0f;
  ScanStringF("Count: 34 value: 123.457", "Count: %d value: %f", &count, &value);
  printf("from scan, count: %d, value: %f \n", count, value);

  // Wait for keypress in case windowed
  getchar();
}
