//CPP startup
#include <string.h>
#include <stdio.h>

#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

void *__INIT_LIST__[2]={ 0,0 };
void *__EXIT_LIST__[2]={ 0,0 };

void* memset ( void * ptr, int value, size_t num )
{
	unsigned char* pTemp = (unsigned char*) ptr;

	while ( num-- )
	{
		*pTemp++=value;
	}

	return ptr;
}

// ----------------------------------------------------------------------------------------

inline char *strcpy(char *dest, const char* src)
{
	char *ret = dest;
	while ((*dest++ = *src++)) {
	};

	return ret;
}

int strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2; s1++, s2++)
	{
		if (*s1 == '\0')
			return 0;
	}
   
   return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
}

void* memmove(void *destination, const void *source, size_t n)
{
	char* dest = (char*)destination;
	char* src = (char*)source;

	/* No need to do that thing. */
	if (dest == src)
		return destination;

	/* Check for destructive overlap.  */
	if (src < dest && dest < src + n) {
		/* Destructive overlap ... have to copy backwards.  */
		src += n;
		dest += n;
		while (n-- > 0)
			*--dest = *--src;
	} else {
		/* Do an ascending copy.  */
		while (n-- > 0)
			*dest++ = *src++;
	}

	return destination;
}

// ----------------------------------------------------------------------------------------

int atoi( const char* pStr )
{
  int iRetVal = 0;
  int iTens = 1;

  if ( pStr )
  {
	const char* pCur = pStr;
	while (*pCur)
	  pCur++;

	pCur--;

	while ( pCur >= pStr && *pCur <= '9' && *pCur >= '0' )
	{
	  iRetVal += ((*pCur - '0') * iTens);
	  pCur--;
	  iTens *= 10;
	}
  }
  return iRetVal;
}

// ----------------------------------------------------------------------------------------

/* from http://clc-wiki.net/wiki/strncpy */
char *strncpy(char *dest, const char *src, size_t n)
{
	char *ret = dest;
	do {
		if (!n--)
			return ret;
	} while ((*dest++ = *src++));
	while (n--)
		*dest++ = 0;
	return ret;
}

/* from http://clc-wiki.net/wiki/strncmp */
int strncmp(const char* s1, const char* s2, size_t n)
{
	while(n--)
		if(*s1++!=*s2++)
			return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
	return 0;
}

/* from http://clc-wiki.net/wiki/strcat */
char *strcat(char *dest, const char *src)
{
	char *ret = dest;
	while (*dest)
		dest++;
	while ((*dest++ = *src++))
		;
	return ret;
}

#if(0)
/* from http://clc-wiki.net/wiki/strstr */
/* uses memcmp, strlen */
/* For 52 more bytes, an assembly optimized version is available in the .s file */
char *strstr(const char *s1, const char *s2)
{
	size_t n = strlen(s2);
	while(*s1)
		if(!memcmp(s1++,s2,n))
			return (char *) (s1-1);
	return (char *)0;
}
#endif

/* from http://clc-wiki.net/wiki/C_standard_library:string.h:strchr */
char *strchr(const char *s, int c)
{
	while (*s != (char)c)
		if (!*s++)
			return 0;
	return (char *)s;
}

/* from http://clc-wiki.net/wiki/memcmp */
int memcmp(const void* s1, const void* s2,size_t n)
{
	const unsigned char *p1 = s1, *p2 = s2;
	while(n--)
		if( *p1 != *p2 )
			return *p1 - *p2;
		else
			p1++,p2++;
	return 0;
}

/* from http://clc-wiki.net/wiki/C_standard_library:string.h:memchr */
void *memchr(const void *s, int c, size_t n)
{
	unsigned char *p = (unsigned char*)s;
	while( n-- )
		if( *p != (unsigned char)c )
			p++;
		else
			return p;
	return 0;
}

char *strlwr (char * string ) {
	char * cp;

	for (cp=string; *cp; ++cp) {
		if ('A' <= *cp && *cp <= 'Z') {
			*cp += 'a' - 'A';
		}
	}

	return(string);
}

int touppers (int c) {
	if ('a' <= c && c <= 'z') {
		c -= 'a' - 'A';
	}
	return c;
}

int tolowers (int c) {
	if ('A' <= c && c <= 'Z') {
		c += 'a' - 'A';
	}
	return c;
}

void sleep(int secs)
{
	Delay( secs * 50 );
}
