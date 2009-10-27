#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/** One standard library used during the development of this
 *  program had a bug in the following two functions
 *  Created our own as a workaround
 */
int  strnlen(char * str, size_t n)
{
	size_t i = 0;
	while (*str++ && i++ < n);
	return i;
}

char * FIX_strndup(char * str, size_t n)
{
  size_t len = strnlen (str, n);
  char *copy = (char*)malloc (len + 1);
  if (copy)
    {
      memcpy (copy, str, len);
      copy[len] = '\0';
    }
  return copy;
}
