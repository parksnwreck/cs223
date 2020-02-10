#ifndef __SMAP_TEST_FXNS__
#define __SMAP_TEST_FXNS__

#include <stdio.h>

char **make_words(const char *prefix, int n);
char **make_words_select(char * const *words, int *indices, int n);
char **make_random_words(int len, int n);
char **make_words_concat(const char *prefix, const int *suffixes, int n);
char **copy_words(char * const *words, int n);
void free_words(char **arr, int n);
int hash_string_sum(const char *s);
int hash_string_first(const char *s);

#define PRINT_PASSED printf("PASSED\n")
#define PRINT_FAILED printf("FAILED\n")


#endif

