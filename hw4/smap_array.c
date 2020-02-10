#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "smap.h"

/**
 * An implementation of a map of strings as an unsorted fixed-size array.
 * This implementation will not meet the requirements of Assignment #4 and
 * not completely tested itself but should be sufficient for testing your
 * Blotto competition program.
 */

typedef struct
{
  char *key;
  void* value;
} entry;

struct smap
{
  int capacity;
  int size;
  int (*hash)(const char *);
  entry *table;
};

#define SMAP_INITIAL_CAPACITY 100

int smap_table_find_key(const entry *table, const char *key, int (*hash)(const char *), int size, int capacity);
void smap_embiggen(smap *m);

smap *smap_create(int (*h)(const char *s))
{
  // this implementation doesn't need the hash function so we skip the
  // check that it is not NULL
  
  smap *result = malloc(sizeof(smap));
  if (result != NULL)
    {
      result->size = 0;
      result->hash = h;
      result->table = malloc(sizeof(entry) * SMAP_INITIAL_CAPACITY);
      result->capacity = (result->table != NULL ? SMAP_INITIAL_CAPACITY : 0);
    }
  return result;
}

int smap_size(const smap *m)
{
  if (m == NULL)
    {
      return 0;
    }
  
  return m->size;
}

/**
 * Returns the index where key is located in the given map, or the index
 * where it would go if it is not present.
 * 
 * @param table a table with at least one free slot
 * @param key a string, non-NULL
 * @param hash a hash function for strings
 * @param size the size of the table
 * @param capacity the capacity of the table
 * @return the index of key in table, or the index of the empty slot to put it in if it is not present
 */
int smap_table_find_key(const entry *table, const char *key, int (*hash)(const char *), int size, int capacity)
{
  // sequential search for key
  int i = 0;
  while (i < size && strcmp(table[i].key, key) != 0)
    {
      i++;
    }
  return i;
}

bool smap_put(smap *m, const char *key, void *value)
{
  if (m != NULL && key != NULL)
    {
      if (m->size == m->capacity)
	{
	  // table is full (would that be the right condition for a hash table?); try to expand
	  smap_embiggen(m); // does nothing in this implementation
	}

      // find key
      int index = smap_table_find_key(m->table, key, m->hash, m->size, m->capacity);
      if (index < m->size)
	{
	  // key already present
	  m->table[index].value = value;
	  return true;
	}
      else
	{
	  // new key; check whether there is room for the new entry
	  if (m->size == m->capacity)
	    {
	      // still no room :(
	      return false;
	    }
	  
	  // make a copy of the new key
	  char *copy = malloc(strlen(key) + 1);
	  strcpy(copy, key);
	  
	  if (copy != NULL)
	    {
	      m->table[index].key = copy;
	      m->table[index].value = value;
	      m->size++;
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}
    }
  else
    {
      // NULL matrix or key
      return false;
    }
}

void *smap_remove(smap *m, const char *key)
{
  // TO-DO: write and test remove
  return NULL;
}

void smap_embiggen(smap *m)
{
  // TO-DO: write and test embiggen
}

bool smap_contains_key(const smap *m, const char *key)
{
  return smap_table_find_key(m->table, key, m->hash, m->size, m->capacity) < m->size;
}

void *smap_get(smap *m, const char *key)
{
  if (m == NULL || key == NULL)
    {
      return NULL;
    }
  
  int index = smap_table_find_key(m->table, key, m->hash, m->size, m->capacity);
  if (index < m->size)
    {
      return m->table[index].value;
    }
  else
    {
      return NULL;
    }
}

typedef struct key_array
{
  int count;
  const char **keys;
} key_array;

void copy_key(const char *key, void *value, void *arg);

const char **smap_keys(smap *m)
{
  if (m == NULL || smap_size(m) == 0)
    {
      return NULL;
    }

  key_array k = {0, malloc(sizeof(char *) * smap_size(m))};

  if (k.keys == NULL)
    {
      return NULL;
    }

  smap_for_each(m, copy_key, &k);

  return k.keys;
}

void copy_key(const char *key, void *value, void *arg)
{
  key_array *k = arg;

  k->keys[k->count++] = key;
}

void smap_for_each(smap *m, void (*f)(const char *, void *, void *), void *arg)
{
  if (m == NULL || f == NULL)
    {
      return;
    }
  
  for (int i = 0; i < m->size; i++)
    {
      f(m->table[i].key, m->table[i].value, arg);
    }
}  

void smap_destroy(smap *m)
{
  if (m == NULL)
    {
      return;
    }
  
  // free all the copies of the keys we made
  for (int i = 0; i < m->size; i++)
    {
      free(m->table[i].key);
    }

  // free the table
  free(m->table);

  // free the smap struct
  free(m);
}

int smap_default_hash(const char *s)
{
  // don't use this -- it is a terrible hash function!
  if (s == NULL)
    {
      return 0;
    }
  else
    {
      return s[0];
    }
}

