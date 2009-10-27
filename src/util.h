
/**
 * Our own versions to work around a broken libc
 */
int  strnlen(char * str, size_t n);
char * FIX_strndup(char * str, size_t n);
