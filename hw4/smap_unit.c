#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smap_test_functions.h"
#include "smap.h"

void test_get();
void test_get_with_destroy();
void test_initial_size(int size, int on);
void test_size(int n);
void test_contains(int n, int (*)(const char *));
void test_put_accepts_NULL();
void test_put_multiple_times();
void test_put_multiple_times_with_destroy();
void test_large_map(int n);
void test_large_map_with_destroy(int n);
void test_for_each(int n);
void test_destroy(int n);
void test_all(int n, int (*)(const char *));
void test_remove();
void test_remove_not_present();
void test_remove_collision();
void test_remove_collision_with_destroy();
void test_remove_destroy();
void test_remove_destroy_with_free();

smap *make_map(const char *prefix, int n, int value);
void add_keys(smap *m, char * const *keys, int n, int value);
void smap_unit_free_value(const char *key, void *value, void *arg);

#define SMALL_TEST_SIZE 4
#define MEDIUM_TEST_SIZE 10000
#define LARGE_TEST_SIZE 1000000
#define VERY_LARGE_TEST_SIZE 10000000

int main(int argc, char **argv)
{
  int test = 0;

  if (argc > 1)
    {
      test = atoi(argv[1]);
    }

  int size;
  int on;
  
  switch (test)
    {
    case 1:
      test_get();
      break;

    case 2:
      test_size(SMALL_TEST_SIZE);
      break;

    case 3:
      test_contains(SMALL_TEST_SIZE, hash_string_sum);
      break;
      
    case 4:
      test_put_accepts_NULL();
      break;

    case 5:
      test_put_multiple_times();
      break;
      
    case 6:
      test_large_map(LARGE_TEST_SIZE);
      break;

    case 7:
      test_for_each(SMALL_TEST_SIZE);
      break;

    case 8:
      test_destroy(SMALL_TEST_SIZE);
      break;

    case 9:
      test_contains(LARGE_TEST_SIZE, smap_default_hash);
      break;
      
    case 10:
      test_all(LARGE_TEST_SIZE, smap_default_hash);
      break;

    case 11:
      test_remove();
      break;

    case 12:
      test_remove_not_present();
      break;

    case 13:
      test_remove_collision();
      break;

    case 14:
      test_remove_destroy();
      break;

    case 15:
      if (argc < 4)
	{
	  printf("FAIL -- no test size or on/off control given for efficiency test\n");
	  return 1;
	}
      size = atoi(argv[2]);
      on = atoi(argv[3]);
      test_initial_size(size, on);
      break;

    case 16:
      test_put_multiple_times_with_destroy();
      break;

    case 17:
      test_large_map_with_destroy(LARGE_TEST_SIZE);
      break;

    case 18:
      test_remove_collision_with_destroy();
      break;

    case 19:
      test_remove_destroy_with_free();
      break;

    case 20:
      test_get_with_destroy();
      break;
      
    default:
      fprintf(stderr, "USAGE: %s test-number\n", argv[0]);
    }
}

smap *make_map(const char *prefix, int n, int value)
{
  smap *m = smap_create(hash_string_sum);

  char **keys = make_words(prefix, n);
  add_keys(m, keys, n, value);
  
  return m;
}

void add_keys(smap *m, char * const *keys, int n, int value)
{
    for (int i = 0; i < n; i++)
    {
      int *temp = malloc(sizeof(int));
      *temp = value;
      smap_put(m, keys[i], temp);
    }
}

void test_get()
{
  smap *m = smap_create(hash_string_sum);

  int twenty = 20;
  smap_put(m, "Twenty", &twenty);

  if (*(int *)smap_get(m, "Twenty") == 20)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- value does not match\n");
    }
  
  // intentionally not destroying
}

void test_get_with_destroy()
{
  smap *m = smap_create(hash_string_sum);

  int twenty = 20;
  smap_put(m, "Twenty", &twenty);

  if (*(int *)smap_get(m, "Twenty") == 20)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- value does not match\n");
    }
  
  smap_destroy(m);
}

void test_initial_size(int size, int on)
{
  if (on)
    {
      smap *m = smap_create(smap_default_hash);
      smap_destroy(m);
    }
}

void test_size(int n)
{
  smap *m = smap_create(hash_string_sum);

  if (smap_size(m) != 0)
    {
      printf("FAILED -- initial size not 0\n");
      return;
    }
    
  char **keys = make_words("word", n);
  add_keys(m, keys, n, 0);

  if (smap_size(m) != n)
    {
      printf("FAILED -- size not correct after adding keys\n");
      return;
    }
  
  smap_put(m, "word0", smap_get(m, "word0"));

  if (smap_size(m) == n)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- size changed after adding duplicate key\n");
    }
  
    // intentionally not destroying
}

void test_contains(int n, int (*hash)(const char *))
{
  smap *m = smap_create(hash);
  char **keys = make_words("word", n);
  char **not_keys = make_words("worte", n);
  
  add_keys(m, keys, n, 0);

  for (int i = 0; i < n; i++)
    {
      if (!smap_contains_key(m, keys[i]))
	{
	  printf("FAILED -- map does not contain %s\n", keys[i]);
	  return;
	}
      if (smap_contains_key(m, not_keys[i]))
	{
	  printf("FAILED -- map contains %s\n", not_keys[i]);
	  return;
	}
    }
  PRINT_PASSED;
  
  // intentionally not destroying or freeing
}

void test_put_accepts_NULL()
{
  smap *m = smap_create(hash_string_sum);

  if (smap_put(m, "Twenty", NULL) && smap_contains_key(m, "Twenty") && smap_get(m, "Twenty") == NULL)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- put with NULL value did not record (key, value) pair\n");
    }
  
  // intentionally not destroying
}

void test_put_multiple_times()
{
  smap *m = smap_create(hash_string_sum);

  int *values = malloc(sizeof(int) * 20);
  for (int i = 0; i < 20; i++)
    {
      values[i] = i + 1;
      smap_put(m, "Twenty", values + i);
    }

  if (*(int *)smap_get(m, "Twenty") == 20)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- wrong value returned after multiple puts with same key\n");
    }
  
  // intentionally not destroying
}

void test_put_multiple_times_with_destroy()
{
  smap *m = smap_create(hash_string_sum);

  int *values = malloc(sizeof(int) * 20);
  for (int i = 0; i < 20; i++)
    {
      values[i] = i + 1;
      smap_put(m, "Twenty", values + i);
    }

  if (*(int *)smap_get(m, "Twenty") == 20)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- wrong value returned after multiple puts with same key\n");
    }
  
  smap_destroy(m);
  free(values);
}

void test_large_map(int n)
{
  smap *m = smap_create(smap_default_hash);
  char **keys = make_words("word", n);
  char **not_keys = make_words("wort", n);

  for (int i = 0; i < n; i++)
    {
      int *value = malloc(sizeof(int));
      *value = n - i;
      smap_put(m, keys[i], value);
    }

  if (smap_size(m) != n)
    {
      printf("FAILED -- size of large map is not %d\n", n);
      return;
    }

  for (int i = 0; i < n; i++)
    {
      if (!smap_contains_key(m, keys[i]))
	{
	  printf("FAILED -- large map missing key %s\n", keys[i]);
	  return;
	}

      if (*(int *)smap_get(m, keys[i]) != n - i)
	{
	  printf("FAILED -- wrong value in large map for key %s\n", keys[i]);
	  return;
	}

      if (smap_contains_key(m, not_keys[i]))
	{
	  printf("FAILED -- large map contains key %s that was not added\n", not_keys[i]);
	  return;
	}
    }

  // intentionally not freeing or destroying
  
  PRINT_PASSED;
}

void test_large_map_with_destroy(int n)
{
  smap *m = smap_create(smap_default_hash);
  char **keys = make_words("word", n);
  char **not_keys = make_words("wort", n);

  for (int i = 0; i < n; i++)
    {
      int *value = malloc(sizeof(int));
      *value = n - i;
      smap_put(m, keys[i], value);
    }

  if (smap_size(m) != n)
    {
      printf("FAILED -- size of large map is not %d\n", n);
      return;
    }

  for (int i = 0; i < n; i++)
    {
      if (!smap_contains_key(m, keys[i]))
	{
	  printf("FAILED -- large map missing key %s\n", keys[i]);
	  return;
	}

      if (*(int *)smap_get(m, keys[i]) != n - i)
	{
	  printf("FAILED -- wrong value in large map for key %s\n", keys[i]);
	  return;
	}

      if (smap_contains_key(m, not_keys[i]))
	{
	  printf("FAILED -- large map contains key %s that was not added\n", not_keys[i]);
	  return;
	}
    }

  free_words(keys, n);
  free_words(not_keys, n);
  smap_for_each(m, smap_unit_free_value, NULL);
  smap_destroy(m);
  
  PRINT_PASSED;
}

void sum_keys_and_values(const char *key, void *value, void *p)
{
  int *sum = p;
  *sum += atoi(key);
  *sum += *(int *)value;
}

void test_for_each(int n)
{
  int sum = 0;
  smap *m = make_map("", n, 2);
  smap_for_each(m, sum_keys_and_values, &sum);

  if (sum == n * (n - 1) / 2 + 2 * n)
    {
      PRINT_PASSED;
    }
  else
    {
      printf("FAILED -- incorrect sum %d after for_each\n", sum);
    }

  // intentionally not destroying or freeing
}

void test_destroy(int n)
{
  smap *m = make_map("word", n, 0);

  smap_destroy(m);
  PRINT_PASSED;
}

void smap_unit_free_value(const char *key, void *value, void *arg)
{
  free(value);
}

void test_all(int n, int (*hash)(const char *))
{
  smap *m = smap_create(hash);
  char **keys = make_words("word", n);
  char **not_keys = make_words("wort", n);

  int *values = malloc(sizeof(int) * n);

  for (int i = 0; i < n; i++)
    {
      values[i] = n - i;
      smap_put(m, keys[i], values + i);
    }

  if (smap_size(m) != n)
    {
      printf("FAILED -- reported size is %d\n", smap_size(m));
      return;
    }

  for (int i = 0; i < n; i++)
    {
      if (!smap_contains_key(m, keys[i]))
	{
	  printf("FAILED -- map does not contain key %s\n", keys[i]);
	  return;
	}
      
      if (*(int *)smap_get(m, keys[i]) != n - i)
	{
	  printf("FAILED -- value for key %s is incorrect\n", keys[i]);
	  return;
	}
      
      if (smap_contains_key(m, not_keys[i]))
	{
	  printf("FAILED -- contains key %s that was not added\n", not_keys[i]);
	  return;
	}
    }

  free(values);
  smap_destroy(m);
  free_words(keys, n);
  free_words(not_keys, n);
  
  PRINT_PASSED;
}

void test_remove()
{
  smap *m = smap_create(hash_string_sum);
  int *nbo_value = malloc(sizeof(int));
  *nbo_value = 10;
  int *cai_value = malloc(sizeof(int));
  *cai_value = 11;
  
  smap_put(m, "NBO", nbo_value);
  smap_put(m, "CAI", cai_value);
  smap_remove(m, "NBO");

  if (smap_contains_key(m, "NBO"))
    {
      printf("FAILED -- map contains removed key NBO\n");
      return;
    }

  if (!smap_contains_key(m, "CAI"))
    {
      printf("FAILED -- map lost key CAI after removing NBO\n");
      return;
    }

  if (*(int *)smap_get(m, "CAI") != 11)
    {
      printf("FAILED -- map changed value for CAI after removing NBO\n");
      return;
    }

  if (smap_size(m) != 1)
    {
      printf("FAILED -- incorrect size %d after remove\n", smap_size(m));
      return;
    }
  
  // intentionally not destroying or freeing
  PRINT_PASSED;
}

void test_remove_not_present()
{
  smap *m = smap_create(hash_string_sum);
  int *nbo_value = malloc(sizeof(int));
  *nbo_value = 10;
  int *cai_value = malloc(sizeof(int));
  *cai_value = 11;
  
  smap_put(m, "NBO", nbo_value);
  smap_remove(m, "CAI");

  if (smap_contains_key(m, "CAI"))
    {
      printf("FAILED -- map contains key CAI that was never added\n");
      return;
    }

  if (!smap_contains_key(m, "NBO"))
    {
      printf("FAILED -- map lost key NBO after removing CAI\n");
      return;
    }

  if (*(int *)smap_get(m, "NBO") != 10)
    {
      printf("FAILED -- map changed value for NBO after removing CAI\n");
      return;
    }

  if (smap_size(m) != 1)
    {
      printf("FAILED -- incorrect size %d after remove\n", smap_size(m));
      return;
    }
  
  // intentionally not destroying or freeing
  PRINT_PASSED;
}

void test_remove_collision()
{
  smap *m = smap_create(hash_string_first);
  int *bwi_value = malloc(sizeof(int));
  *bwi_value = 10;
  int *bos_value = malloc(sizeof(int));
  *bos_value = 11;
  int *btr_value = malloc(sizeof(int));
  *btr_value = 12;
  
  smap_put(m, "BWI", bwi_value);
  smap_put(m, "BOS", bos_value);
  smap_put(m, "BTR", btr_value);
  smap_remove(m, "BOS");

  if (smap_contains_key(m, "BOS"))
    {
      printf("FAILED -- map contains removed key BOS\n");
      return;
    }

  if (!smap_contains_key(m, "BWI"))
    {
      printf("FAILED -- map lost key BWI after removing BOS\n");
      return;
    }

  if (!smap_contains_key(m, "BTR"))
    {
      printf("FAILED -- map lost key BTR after removing BOS\n");
      return;
    }

  if (*(int *)smap_get(m, "BWI") != 10)
    {
      printf("FAILED -- value changed for key BWI after removing BOS\n");
    }

  if (*(int *)smap_get(m, "BTR") != 12)
    {
      printf("FAILED -- value changed for key BTR after removing BOS\n");
    }

  smap_remove(m, "BTR");
  
  if (smap_contains_key(m, "BTR"))
    {
      printf("FAILED -- map contains removed key BTR\n");
      return;
    }

  if (!smap_contains_key(m, "BWI"))
    {
      printf("FAILED -- map lost key BWI after removing BTR\n");
      return;
    }

  if (smap_size(m) != 1)
    {
      printf("FAILED -- incorrect size %d after removes\n", smap_size(m));
      return;
    }
  
  // intentionally not destroying or freeing
  PRINT_PASSED;
}

void test_remove_collision_with_destroy()
{
  smap *m = smap_create(hash_string_first);
  int *bwi_value = malloc(sizeof(int));
  *bwi_value = 10;
  int *bos_value = malloc(sizeof(int));
  *bos_value = 11;
  int *btr_value = malloc(sizeof(int));
  *btr_value = 12;
  
  smap_put(m, "BWI", bwi_value);
  smap_put(m, "BOS", bos_value);
  smap_put(m, "BTR", btr_value);
  smap_remove(m, "BOS");

  if (smap_contains_key(m, "BOS"))
    {
      printf("FAILED -- map contains removed key BOS\n");
      return;
    }

  if (!smap_contains_key(m, "BWI"))
    {
      printf("FAILED -- map lost key BWI after removing BOS\n");
      return;
    }

  if (!smap_contains_key(m, "BTR"))
    {
      printf("FAILED -- map lost key BTR after removing BOS\n");
      return;
    }

  if (*(int *)smap_get(m, "BWI") != 10)
    {
      printf("FAILED -- value changed for key BWI after removing BOS\n");
    }

  if (*(int *)smap_get(m, "BTR") != 12)
    {
      printf("FAILED -- value changed for key BTR after removing BOS\n");
    }

  smap_remove(m, "BTR");
  
  if (smap_contains_key(m, "BTR"))
    {
      printf("FAILED -- map contains removed key BTR\n");
      return;
    }

  if (!smap_contains_key(m, "BWI"))
    {
      printf("FAILED -- map lost key BWI after removing BTR\n");
      return;
    }

  if (smap_size(m) != 1)
    {
      printf("FAILED -- incorrect size %d after removes\n", smap_size(m));
      return;
    }

  free(bwi_value);
  free(bos_value);
  free(btr_value);
  smap_destroy(m);
  
  PRINT_PASSED;
}

void test_remove_destroy()
{
  smap *m = smap_create(hash_string_sum);
  int *nbo_value = malloc(sizeof(int));
  *nbo_value = 10;
  int *cai_value = malloc(sizeof(int));
  *cai_value = 11;
  
  smap_put(m, "NBO", nbo_value);
  smap_put(m, "CAI", cai_value);
  smap_remove(m, "NBO");
  smap_remove(m, "CAI");
  
  // intentionally not freeing
  smap_destroy(m);
  PRINT_PASSED;
}

void test_remove_destroy_with_free()
{
  smap *m = smap_create(hash_string_sum);
  int *nbo_value = malloc(sizeof(int));
  *nbo_value = 10;
  int *cai_value = malloc(sizeof(int));
  *cai_value = 11;
  
  smap_put(m, "NBO", nbo_value);
  smap_put(m, "CAI", cai_value);
  smap_remove(m, "NBO");
  smap_remove(m, "CAI");
  
  smap_destroy(m);
  free(nbo_value);
  free(cai_value);
  PRINT_PASSED;
}
