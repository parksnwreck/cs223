#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lugraph.h"

int num_vertices = 6;
int test_edges[][2] = {{0, 1}, {1, 2}, {1, 3}, {2, 3}, {0, 4}};
int num_edges = sizeof(test_edges) / sizeof(int[2]);

void test_connected(const lugraph *g, int v1, int v2, bool expected);
void test_distance(const lugraph *g, int v1, int v2, int expected);
void test_path(const lugraph *g, int v1, int v2, int expected);

int main(int argc, char **argv)
{
  if (argc == 1)
    {
      return 0;
    }
  int test = atoi(argv[1]);
  
  lugraph *g = lugraph_create(num_vertices);
  if (g == NULL)
    {
      printf("FAILED -- could not allocate graph\n");
      return 1;
    }

  for (int i = 0; i < num_edges; i++)
    {
      lugraph_add_edge(g, test_edges[i][0], test_edges[i][1]);
    }

  switch (test)
    {
    case 1:
      test_connected(g, 0, 3, true);
      break;

    case 2:
      test_connected(g, 0, 5, false);
      break;

    case 3:
      test_distance(g, 0, 0, 0);
      break;

    case 4:
      test_distance(g, 0, 1, 1);
      break;

    case 5:
      test_distance(g, 0, 3, 2);
      break;

    case 6:
      test_distance(g, 5, 0, -1);
      break;

    case 7:
      test_path(g, 0, 0, 0);
      break;

    case 8:
      test_path(g, 0, 1, 1);
      break;

    case 9:
      test_path(g, 4, 1, 2);
      break;

    case 10:
      test_path(g, 4, 3, 3);
      break;

    case 11:
      test_path(g, 5, 0, -1);
      break;

    case 12:
      test_connected(g, -1, 6, false);
      break;

    case 13:
      test_distance(g, 0, 6, -1);
      break;

    case 14:
      test_distance(g, 6, 0, -1);
      break;

    case 15:
      test_path(g, 0, 6, -1);
      break;

    case 16:
      test_path(g, -1, 0, -1);
      break;

    default:
      printf("INVALID TEST NUMBER %s\n", argv[1]);
      break;
    }
      
  lugraph_destroy(g);
}

void test_connected(const lugraph *g, int v1, int v2, bool expected)
{
  bool result = lugraph_connected(g, v1, v2);
  if ((result && !expected) || (!result && expected))
    {
      printf("FAILED\n");
    }
  else
    {
      printf("PASSED\n");
    }
}

void test_distance(const lugraph *g, int v1, int v2, int expected)
{
  lug_search *s = lugraph_bfs(g, v1);
  if (s == NULL && v1 >= 0 && v1 < lugraph_size(g))
    {
      printf("FAILED -- no search results\n");
      return;
    }
  else if (s == NULL)
    {
      // no search results b/c invalid vertex is OK
      printf("PASSED\n");
      return;
    }
  
  int d = lug_search_distance(s, v2);
  
  if (d != expected)
    {
      printf("FAILED -- reported distance is %d, not %d\n", d, expected);
    }
  else
    {
      printf("PASSED\n");
    }
  
  lug_search_destroy(s);
}

void test_path(const lugraph *g, int v1, int v2, int expected)
{
  lug_search *s = lugraph_bfs(g, v1);
  if (s == NULL && v1 >= 0 && v1 < lugraph_size(g))
    {
      printf("FAILED -- no search results\n");
      return;
    }
  else if (s == NULL)
    {
      // no search results b/c invalid vertex is OK
      printf("PASSED\n");
      return;
    }
  
  int d;
  int *path = lug_search_path(s, v2, &d);
  if (path == NULL && expected != -1)
    {
      // no path returned and there should have been one
      printf("FAILED -- no path\n");
    }
  else if (d != expected)
    {
      // path returned is not of the expected distance
      printf("FAILED -- reported distance is %d, not %d\n", d, expected);
    }
  else if (d != -1)
    {
      // there is a path; check the endpoints and edges
      if (path[0] != v1 || path[d] != v2)
	{
	  printf("FAILED -- wrong endpoints\n");
	}
      else
	{
	  int i = 1;
	  while (i < d && lugraph_has_edge(g, path[i - 1], path[i]))
	    {
	      i++;
	    }
	  if (i < d)
	    {
	      // found an invalid edge
	      printf("FAILED -- path has invalid edge (%d, %d)\n", path[i - 1], path[i]);
	    }
	  else
	    {
	      // all edges checked out; done!
	      printf("PASSED\n");
	    }
	}
    }
  else
    {
      printf("PASSED\n");
    }

  free(path);
  lug_search_destroy(s);
}
