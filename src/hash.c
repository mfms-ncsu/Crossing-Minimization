/**
 * @file hash.c
 * @brief Implementation of access functions for hash table.
 * @author Matthias Stallmann
 * @date 2008/12/21
 * $Id: hash.c 2 2011-06-07 19:50:41Z mfms $
 *
 * Uses a polynomial hash function.
 */

#include"defs.h"
#include"hash.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define LOAD_FACTOR 0.75
#define POLYNOMIAL_VALUE 37
#define MIN_TABLE_SIZE 8

/**
 * 2^k - 1 where k is chosen to give the right table size; this is also a the
 * table size.
 */
static unsigned int modulus;

static Nodeptr * hash_table = NULL;

// for statistics
static int number_of_probes = 0;
static int number_of_accesses = 0;

/**
 * Computes a suitable table size (modulus) based on a given desired number
 * of entries. The choice used here is a power of two minus one.
 */
static unsigned int getTableSize( int entries );

/**
 * Compute the reverse of a string
 * @param reversed a buffer to hold the reversed version (destination)
 * @param original the original string
 */
static void reverse( char * reversed, const char * original );

/**
 * Calculates the hash value of a given node name. Overflow doesn't
 * matter because this is supposed to be 'random' anyway.
 * The name is reversed because most node names have common prefixes.
 */
static unsigned int hashValue( const char * name );

/**
 * Computes an index into the table from the name.
 */
static unsigned int hashIndex( const char * name );

/**
 * @param name The name of a node
 * @return the index of the position in the hash table where the node has
 * been found or of the first empty position after the hash value of the name. 
 */
static unsigned int getIndex( const char * name );

#ifdef DEBUG
static void printHashTable();
#endif

void initHashTable( int number_of_items )
{
  modulus = getTableSize( number_of_items );
  hash_table = (Nodeptr *) calloc( modulus, sizeof(Nodeptr) );
  // the following is not really necessary since calloc fills the allocated
  // memory with 0's, but it doesn't hurt to be careful
  int i = 0;
  for( ; i < modulus; i++ ) hash_table[i] = NULL;
  number_of_probes = 0;
  number_of_accesses = 0;
}

void insertInHashTable( const char * name, Nodeptr node )
{
  unsigned int index = getIndex( name );
  if( hash_table[index] != NULL )
    {
      fprintf( stderr, "insertInHashTable: Entry for '%s' already exists\n",
               name );
      abort();
    }
  hash_table[index] = node;
#ifdef DEBUG
  printf("*** insert: name='%s' node->name='%s' position=%u"
         " index=%u value=%u\n",
         name, node->name, hashIndex(name), index, hashValue(name) );
  printHashTable();
  printf("***\n");
#endif
}

Nodeptr getFromHashTable( const char * name )
{
  unsigned int index = getIndex( name );
  return hash_table[index];
}

void removeHashTable()
{
  free(hash_table);
}

double getAverageNumberOfProbes()
{
  return ((double) number_of_probes) / number_of_accesses;
}

#ifdef DEBUG
static void printHashTable()
{
  printf("--\n hash_table, size = %u\n", modulus);
  int i = 0;
  for( ; i < modulus; i++ )
    {
      if( hash_table[i] == NULL ) printf("  0\n");
      else printf("  %4d: '%s' position=%u index=%u value=%u\n",
                  i, hash_table[i]->name,
                  hashIndex( hash_table[i]->name ),
                  getIndex( hash_table[i]->name ),
                  hashValue( hash_table[i]->name )
                  );
    }
  printf("--\n");
}
#endif

static unsigned int getTableSize( int entries )
{
  int target_value = (int) (entries / LOAD_FACTOR);
  unsigned int table_size = MIN_TABLE_SIZE;
  for( ; table_size < target_value; table_size *= 2 );
  return table_size - 1;
}

static void reverse( char * reversed, const char * original )
{
  char * rev_ptr = reversed;
  const char * org_ptr = original + strlen(original) - 1;
  // store the reverse of the string in buffer
  for( ; org_ptr >= original; org_ptr-- )
    {
      * rev_ptr++ = * org_ptr;
    }
  * rev_ptr = '\0';
}

static unsigned int hashValue( const char * name )
{
  char rev_buf[MAX_NAME_LENGTH];
  reverse( rev_buf, name );
  char * rev_ptr = rev_buf;
  unsigned int value = 0;
  for( ; * rev_ptr != '\0'; rev_ptr++ )
    {
      value = (value + POLYNOMIAL_VALUE * value + * rev_ptr);
    }
  return value;
}

static unsigned int hashIndex( const char * name )
{
  return hashValue( name ) % modulus;
}

static unsigned int getIndex( const char * name )
{
  number_of_accesses++;
  unsigned int index = hashIndex( name );
  number_of_probes++;
  while( hash_table[ index ] != NULL
         && strcmp( name, hash_table[ index ]->name ) != 0 )
    {
      index = (index + 1) % modulus;
      number_of_probes++;
    }
  return index;
}

#ifdef TEST
int main()
{
  initHashTable( 7 );
  char name[MAX_NAME_LENGTH];
  fgets( name, MAX_NAME_LENGTH, stdin );
  name[ strlen(name) - 1 ] = '\0';
  Nodeptr new_node = (Nodeptr) malloc( sizeof(struct node_struct));
  new_node->name = (char *) malloc( strlen(name) + 1 );
  strcpy( new_node->name, name );
  insertInHashTable( name, new_node );
  return 0;
}
#endif

/*  [Last modified: 2008 12 29 at 16:35:13 GMT] */
