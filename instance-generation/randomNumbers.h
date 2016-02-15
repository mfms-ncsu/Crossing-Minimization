/**
 * @file randomNumbers.h
 *  @brief Header for C-utilities that are based on the IEEE-48
 *         random number standard.
 * @author Matt Stallmann
 * @date 19 Feb 2003
 * $Id: randomNumbers.h 9 2011-06-13 01:29:18Z mfms $
 *
 * [Part of my C-Utilities library -- any updates should be propagated
 *  back to there]
 */

#ifndef RANDOMNUMBERS_H
#define RANDOMNUMBERS_H

#ifndef bool
#define bool char
#define false 0
#define true 1
#endif

/**
 * PRE: seed is an array of 3 numbers
 * POST: the current stream is initialized using seed
 */
extern void RN_setSeed( unsigned short seed[] );

/**
 * POST: the current stream is initialized using time and pid
 */
extern void RN_setRandomSeed(void);

/**
 * POST: retval == pointer to current seed
 */
extern const unsigned short * RN_getSeed( void );

/**
 * PRE: lb <= ub
 * POST: retval == a random integer in the interval [lb,ub]
 */
extern int RN_integer( int lb, int ub );

/**
 * PRE: lb <= ub
 * POST: retval == a random real (double) in the interval [lb,ub)
 */
extern double RN_real( double lb, double ub );

/**
 * PRE: 0 <= p <= 1
 * POST: retval == true with probability p, false with probability 1-p
 */
extern bool RN_boolean( double p );

/**
 * PRE: A is an array of 'length' items, each of which is 'element_size'
 *      bytes long.
 * POST: A has been randomly permuted
 */
extern void RN_permute( void * A, int length, int element_size );

/**
 * PRE: A is an array of 'length' items, each of which is 'element_size'
 *      bytes long.
 * POST: A has been randomly permuted
 *       retval == an allocated array of int's containing the actual
 *                 permutation: the old A[retval[i]] is now at A[i]
 *       To restore the array A back to its original state, do
 *            for( i = 0; i < length; ++i ) {
 *              B[ i ] = A[ i ];  where B is an array of the same type as A
 *            }
 *            for( i = 0; i < length; ++i ) {
 *              A[ retval[ i ] ] = B[ i ];
 *            }
 */
extern int * RN_permutation( void * A, int length, int element_size );

#endif

/*  [Last modified: 2008 09 14 at 00:11:38 GMT] */
