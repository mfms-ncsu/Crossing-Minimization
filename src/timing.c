/**
 *  @file timing.c
 *  @brief Implementation of a simple function to compute CPU milliseconds
 *  (user time only).
 *  @author Matt Stallmann
 *  @date ca. 1990
 *  $Id: timing.c 20 2011-06-26 23:29:46Z mfms $
 */

/* Propagate changes back to the C-Utilities repository. */

#include "timing.h"
#include <sys/time.h>
#include <sys/resource.h>

/** DEPRECATED -- see timing.h */
double currentCPUTime() 
{
   struct rusage ru;
   getrusage(RUSAGE_SELF, &ru);

   return ( ru.ru_utime.tv_sec * 1000
            + ru.ru_utime.tv_usec / 1000
            );
}

double getUserSeconds() {
  struct rusage ru;
  getrusage( RUSAGE_SELF, &ru );
  return ( ru.ru_utime.tv_sec + 
           (double) ru.ru_utime.tv_usec / 1000000.0 );
}

/*  [Last modified: 2011 06 26 at 22:15:46 GMT] */
