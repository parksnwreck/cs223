#include <stdlib.h>
#include <string.h>

#include "smap_test_functions.h"

#define MAX_INT_DIGITS 10

char **make_words(const char *prefix, int n)
{
  int prefix_len = strlen(prefix);
  char **arr = malloc(sizeof(char *) * n);
  for (int i = 0; i < n; i++)
    {
      arr[i] = malloc(sizeof(char) * (prefix_len + MAX_INT_DIGITS + 1));
      sprintf(arr[i], "%s%d", prefix, i);
    }

  return arr;
}

char **make_words_select(char * const *words, int *indices, int n)
{
  char **arr = malloc(sizeof(char *) * n);
  for (int i = 0; i < n; i++)
    {
      arr[i] = malloc(sizeof(char) * (strlen(words[indices[i]]) + 1));
      strcpy(arr[i], words[indices[i]]);
    }

  return arr;
}

char **make_words_concat(const char *prefix, const int *suffixes, int n)
{
  int prefix_len = strlen(prefix);
  char **arr = malloc(sizeof(char *) * n);
  for (int i = 0; i < n; i++)
    {
      arr[i] = malloc(sizeof(char) * (prefix_len + MAX_INT_DIGITS + 1));
      sprintf(arr[i], "%s%d", prefix, suffixes[i]);
    }

  return arr;
}

char **make_random_words(int len, int n)
{
  const char *alphabet = "ABCDEFGHIJKLMNIPQRSTUVWXYZ";
  int alphabet_len = strlen(alphabet);
  
  char **arr = malloc(sizeof(char *) * n);
  for (int i = 0; i < n; i++)
    {
      arr[i] = malloc(sizeof(char) * (len + 1));
      for (int j = 0; j < len; j++)
	{
	  arr[i][j] = alphabet[rand() % alphabet_len];
	}
      arr[i][len] = '\0';
    }
  
  return arr;
}

char **copy_words(char * const *words, int n)
{
  char **arr = malloc(sizeof(char *) * n);
  for (int i = 0; i < n; i++)
    {
      arr[i] = malloc(sizeof(char) * (strlen(words[i]) + 1));
      strcpy(arr[i], words[i]);
    }
  
  return arr;
}

void free_words(char **arr, int n)
{
  for (int i = 0; i < n; i++)
    {
      free(arr[i]);
    }
  free(arr);
}

int hash_string_sum(const char *s)
{
  // not a very good hash function for strings
  
  int hash = 0;
  while (*s != '\0')
    {
      hash = (hash << 1) + *s;
      s++;
    }

  // make positive in case of overflow
  if (hash < 0)
    {
      hash = -hash;
      if (hash < 0)
	{
	  hash = 0;
	}
    }
  return hash;
}

int hash_string_first(const char *s)
{
  return *s;
}
