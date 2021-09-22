#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
 * --tail オプションの引数解析
 *     数値 + K,M,G 
 */

long long timeConv(unsigned char *p)
{
  long long     ret = 0;
  char          tmp[BUFSIZ];
  char          *end = NULL;

  strncpy( tmp, p, BUFSIZ );
  ret = strtoll( tmp, &end, 10 );
  if ( ret > 0 && end != NULL ) {
    switch ( *end ) {
    case  'K':
      ret *= 1000 ; break;
    case  'M':
      ret *= 1000 * 1000 ; break;
    case  'G':
      ret *= 1000 * 1000 * 1000 ; break;
    }
  }
#ifdef DEBUG
  printf("%lld %s\n",ret, end);
#endif
  
  return ret;
}
