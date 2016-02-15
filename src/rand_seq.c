/** @file rand_seq.c a simple program to generate a sequence of random numbers
 *               (uses process id and current time to form the initial seed)
 * @author Matt Stallmann, 23 Jan 2003
 * $Id: rand_seq.c 103 2014-12-30 17:50:27Z mfms $
 */

/* HISTORY:
   2005/05/01 -- added time as a consideration in creating the seed (pid was
   too predictable)
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>              /* atoi() */
#include<limits.h>              /* USHRT_MAX */
#include<unistd.h>              /* getpid() */
#include<sys/types.h>
#include<sys/time.h>            /* struct timeval for gettimeofday() */

struct timeval current_time;

int main( int argc, char * argv[] ) {
  int n = 0;                    /* number of random numbers */
  int i = 0;                    /* loop index */
  long pid = getpid();
  unsigned seed = 0;
  if( argc > 1 ) {
    n = atoi( argv[ 1 ] );
  }
  gettimeofday( & current_time, NULL );
  seed = (pid * current_time.tv_sec) % current_time.tv_usec;
  srand( seed );
  if( n <= 0 ) {
    fprintf(stderr, "How many random numbers? ");
    scanf( "%d", &n );
  }
  for( i = 0; i < n; ++i ) {
    printf( "%u\n", rand() / USHRT_MAX );
  }
  return 0;
}

/*  [Last modified: 2005 08 11 at 20:48:28 GMT] */
