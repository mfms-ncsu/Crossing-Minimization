/**
 *  @file timing.h
 *  @brief Header for a simple function to compute CPU user milliseconds.
 *  @author Matt Stallmann
 *  @date ca. 1990
 *  $Id: timing.h 20 2011-06-26 23:29:46Z mfms $
 */

/* Propagate changes back to the C-Utilities repository. */

#if ! defined(TIMING_H)
#define TIMING_H

#include <sys/time.h>

/**
 *   Return total user time used by this process in # of
 *   milliseconds, accurate to the nearest microsecond
 *   (depending on system)
 *   DEPRECATED by the function below (usually a bad idea to report
 *   milliseconds). 
 */
double currentCPUTime();

/**
 *   Return total user time used by this process in # of seconds.
 *   Anything below 0.1 should probably be considered "noise"
 */
double getUserSeconds();

#endif /* ! defined(TIMING_H) */

/*  [Last modified: 2009 12 01 at 19:17:38 GMT] */
