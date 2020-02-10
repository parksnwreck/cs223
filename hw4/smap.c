#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smap.h"

// parameters inspired from Aspnes Notes Chapter 5.5
#define INITIAL_TABLESIZE (8)
#define TABLESIZE_MULTIPLIER (2)
#define MULTIPLIER (97)

// typdef struct for an element in hashmap
typedef struct elt {
    int hash;
    struct elt *next;
    char *key;
    void *value;
} elt;

// define smap struct
struct smap {
    int cap;
    int size;
    int (*h)(const char *s);
    elt **table;
};

static void smapResize(smap *m, int mode);
elt *smapFindElt(const smap *m, const char *key);


/**
 * Returns a hash value for the given string.
 * 
 * MODIFIED FROM ASPNES NOTES CHAPTER 5.5
 * 
 * @param s a string, non-NULL
 * @return an int
 */

int smap_default_hash(const char *s)
{
    unsigned const char *us;
    int h;

    h = 0;

    for (us = (unsigned const char *) s; *us; us++) 
    {
        h = h * MULTIPLIER + *us;
    }

    if (h < 0)
    {
        h = h * -1;
    }

    return h;
}

/**
 * Creates an empty map that uses the given hash function.
 *
 * @param h a pointer to a function that takes a string and returns
 * its hash code, non-NULL
 * @return a pointer to the new map or NULL if it could not be created;
 * it is the caller's responsibility to destroy the map
 */
smap *smap_create(int (*h)(const char *s))
{
    smap *m = malloc(sizeof(smap));
    if (m == NULL)
    {
        return NULL;
    }
    m->cap = INITIAL_TABLESIZE;
    m->size = 0;
    m->h = h;
    m->table = calloc(INITIAL_TABLESIZE, sizeof(elt*));
    if (m->table == NULL)
    {
        free(m);
        return NULL;
    }
    return m;
}

/**
 * Returns the number of (key, value) pairs in the given map.
 *
 * @param m a pointer to a map, non-NULL
 * @return the size of m
 */
int smap_size(const smap *m)
{
    if (m != NULL)
    {
        return m->size;
    }
    else
    {
        return 0;
    }
}

/**
 * Adds a copy of the given key with value to this map.
 * If the key is already present then the old value is replaced.
 * The caller retains ownership of the value.  If key is new
 * and space could not be allocated then there is no effect on the map
 * and the return value is false.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a string, non-NULL
 * @param value a pointer
 * @return true if the put was successful, false otherwise
 */
bool smap_put(smap *m, const char *key, void *value)
{
    // make sure parameters are not NULL
    if (m == NULL || key == NULL)
    {
        return false;
    }

    // find position of key
    int hash = m->h(key);
    int pos = hash % (m->cap);

    // check if the position is empty
    if (m->table[pos] == NULL)
    {
        // allocate an entry at the position
        m->table[pos] = malloc(sizeof(elt));
        if (m->table[pos] == NULL)
        {
            return false;
        }

        // set fields of entry
        m->table[pos]->hash = hash;
        m->table[pos]->next = NULL;
        m->table[pos]->key = strdup(key);
        if (m->table[pos]->key == NULL)
        {
            return false;
        }
        m->table[pos]->value = value;

        // increment size of map by 1
        m->size ++;

        // check if map needs to be resized and rehashed based on load factor
        if (m->size > (m->cap / 2))
        {
            smapResize(m, 0);
        }
        return true;
    }
    else
    {
        // set curr to the the first entry in the position
        elt *curr = m->table[pos];

        // iterate through list to find a match for key
        while (curr != NULL)
        {
            // if match is found set the new value of that key
            if (strcmp(curr->key, key) == 0)
            {
                curr->value = value;
                return true;
            }
            curr = curr->next;
        }

        // if match is not found allocate a new entry to add
        elt *new = malloc(sizeof(elt));
        if (new == NULL)
        {
            return false;
        }

        // set fields of new entry
        new->hash = hash;
        new->key = strdup(key);
        if (new->key == NULL)
        {
            return false;
        }
        new->value = value;

        // add to beginning of linked list
        new->next = m->table[pos];
        m->table[pos] = new;

        // increment size of map by 1
        m->size ++;

        // check if map needs to be resized and rehashed based on load factor
        if (m->size > (m->cap / 2))
        {
            smapResize(m, 0);
        }
        return true;
    }
}

/**
 * Determines if the given key is present in this map.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a string, non-NULL
 * @return true if key is present in this map, false otherwise
 */
bool smap_contains_key(const smap *m, const char *key)
{
    elt *e = smapFindElt(m, key);

    // if pointer of key is found return true or else return false
    if (e != NULL)
    {
        return true;
    }
    else
    {
        return false;
    } 
}

/**
 * Returns the value associated with the given key in this map.
 * If the key is not present in this map then the returned value is
 * NULL.  The value returned is the original value passed to smap_put,
 * and it remains the responsibility of whatever called smap_put to
 * release the value (no ownership transfer results from smap_get).
 *
 * @param m a pointer to a map, non-NULL
 * @param key a string, non-NULL
 * @return the assocated value, or NULL if they key is not present
 */
void *smap_get(smap *m, const char *key)
{
    elt *e = smapFindElt(m, key);

    // if pointer of key is found return its value or return false
    if (e != NULL)
    {
        return e->value;
    }
    else
    {
        return NULL;
    }
}

/**
 * Removes the given key and its associated value from the given map if
 * the key is present.  The return value is NULL and there is no effect
 * on the map if the key is not present.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a key, non-NULL
 * @return the value associated with the key
 */
void *smap_remove(smap *m, const char *key)
{
    if (m == NULL || key == NULL)
    {
        return false;
    }
    
    int hash = m->h(key);
    int pos = hash % (m->cap);

    // make sure there's at least one element to remove at position
    if (m->table[pos] == NULL)
    {
        return NULL;
    }
    else
    {
        elt *curr = m->table[pos];
        elt *next = m->table[pos]->next;

        // check if the entry that needs to be removed is the first entry
        if (strcmp(curr->key, key) == 0)
        {
            void *value = curr->value;
            free(curr->key);
            free(curr);
            m->table[pos] = next;
            m->size --;

            // check if map needs to be resized after removal
            if (m->size < (m->cap / 8))
            {
                smapResize(m, 1);
            }
            return value;
        }
        else
        {
            elt *prev = curr;
            curr = next;
            next = curr->next;

            // iterate through list until match is found and remove
            while (curr != NULL)
            {
                if (strcmp(curr->key, key) == 0)
                {
                    void *value = curr->value;
                    prev->next = curr->next;
                    free(curr->key);
                    free(curr);
                    m->size --;

                    // check if map needs to be resized after removal
                    if (m->size < (m->cap / 8))
                    {
                        smapResize(m, 0);
                    }
                    return value;
                }

                // get next entry
                prev = curr;
                curr = next;
                if (curr != NULL)
                {
                    next = curr->next;
                }
            }
        }
        return NULL;
    }
}

/**
 * Calls the given function for each (key, value) pair in this map, passing
 * the extra argument as well.  This function does not add or remove from
 * the map.
 *
 * @param m a pointer to a map, non-NULL
 * @param f a pointer to a function that takes a key, a value, and an
 * extra argument, and does not add to or remove from the map, no matter
 * what the extra argument is; non-NULL
 * @param arg a pointer
 */
void smap_for_each(smap *m, void (*f)(const char *, void *, void *), void *arg)
{
    if (m == NULL || f == NULL)
    {
        return;
    }

    int cap = m->cap;

    for (int i = 0; i < cap; i++)
    {
        elt *curr = m->table[i];
        while (curr != NULL)
        {
            f(curr->key, curr->value, arg);
            curr = curr->next;
        }
    }   
    return;
}

/**
 * Returns a dynamically array containing pointers to the keys in the
 * given map.  It is the caller's responsibility to free the array,
 * but the map retains ownership of the keys.  If there is a memory
 * allocation error then the returned value is NULL.  If the map is empty
 * then the returned value is NULL.
 *
 * @param m a pointer to a map, non NULL
 * @return a pointer to a dynamically allocated array of pointer to the keys
 */
const char **smap_keys(smap *m)
{
    if (m == NULL)
    {
        return NULL;
    }

    const char **keys = malloc(m->size * sizeof(char *));
    
    if (keys == NULL)
    {
        return NULL;
    }
    int count = 0;
    for (int i = 0; i < m->cap; i++)
    {
        if (m->table[i] != NULL)
        {
            elt *curr = m->table[i];
            while (curr != NULL)
            {
                keys[count] = curr->key;
                curr = curr->next;
                count++;
            }
        }
    }
    return keys;
}

/**
 * Destroys the given map.
 *
 *  MODIFIED FROM ASPNES NOTES CHAPTER 5.5
 * 
 * @param m a pointer to a map, non-NULL
 */
void smap_destroy(smap *m)
{
    if (m == NULL)
    {
        return;
    }

    int cap = m->cap;

    elt *curr;
    elt *next;

    for (int i = 0; i < cap; i++)
    {
        for (curr = m->table[i]; curr != 0; curr = next) 
        {
			next = curr->next;
			free(curr->key);
            free(curr);
		}
    }
    free(m->table);
    free(m);
    return;
}

/**
 * Resizes the given map by TABLESIZE_MULTIPLIER; If mode is 1, it shrinks the table, if mode is 0, it grows the table.
 *
 * MODIFIED FROM ASPNES NOTES CHAPTER 5.5.
 * 
 * @param m a pointer to a map, non-NULL
 * @param mode an integer either 0 or 1
 */

static void smapResize(smap *m, int mode)
{
    if (m == NULL)
    {
        return;
    }

	elt **old_table = m->table; 
    int old_cap = m->cap;
	elt *e;
	elt *next; 
    int new_pos;

    // readjust the cap depending on the mode
    if (mode == 1)
    {
        m->cap = m->cap/TABLESIZE_MULTIPLIER;
    }
    else if (mode == 0)
    {
        m->cap *= TABLESIZE_MULTIPLIER;
    }

    // make new table
	m->table = calloc(m->cap, sizeof(elt*));

    // check for allocation error and revert to old table if there is
    if(m->table == NULL) 
    {
		m->table = old_table; 
        m->cap = old_cap; 
        return;
	}

    // move entries to new table
	for(int i = 0; i < old_cap; i++) 
    {
		for (e = old_table[i]; e != NULL; e = next) 
        {
			next = e->next;
			new_pos = e->hash % m->cap;
			e->next = m->table[new_pos];
			m->table[new_pos] = e;
		} 
    }
	// free old table
	free(old_table);
}

/**
 * Returns pointer to entry with matching key
 *  
 * @param m a pointer to a map, non-NULL
 * @param key a key, non-NULL
 */

elt *smapFindElt(const smap *m, const char *key)
{
    if(m == NULL || key == NULL)
    {
        return NULL;
    }

    int pos = (m->h(key)) % (m->cap);

    if (m->table[pos] == NULL)
    {
        return NULL;
    }
    else
    {
        elt *curr = m->table[pos];
        while (curr != NULL)
        {
            if (strcmp(curr->key, key) == 0)
            {
                return curr;
            }
            curr = curr->next;
        }
        return NULL;
    }
}