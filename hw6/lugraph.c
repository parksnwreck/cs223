#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lugraph.h"

struct lugraph
{
  int n;          // the number of vertices
  int *list_size; // the size of each adjacency list
  int *list_cap;  // the capacity of each adjacency list
  int **adj;      // the adjacency lists
};

struct lug_search
{
  const lugraph *g;
  int from;
  int *dist;
  int *pred;
};

#define LUGRAPH_ADJ_LIST_INITIAL_CAPACITY 4

/**
 * Resizes the adjacency list for the given vertex in the given graph.
 * 
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 */
void lugraph_list_embiggen(lugraph *g, int from);

/**
 * Prepares a search result for the given graph starting from the given
 * vertex.  It is the responsibility of the caller to destroy the result.
 *
 * @param g a pointer to a directed graph
 * @param from the index of a vertex in that graph
 * @return a pointer to a search result
 */
lug_search *lug_search_create(const lugraph *g, int from);

/**
 * Adds the to vertex to the from vertex's adjacency list.
 *
 * @param g a pointer to a graph, non-NULL
 * @param from the index of a vertex in g
 * @param to the index of a vertex in g
 */
void lugraph_add_half_edge(lugraph *g, int from, int to);

lugraph *lugraph_create(int n)
{
  if (n < 1)
    {
      return NULL;
    }
  
  lugraph *g = malloc(sizeof(lugraph));
  if (g != NULL)
    {
      g->n = n;
      g->list_size = malloc(sizeof(int) * n);
      g->list_cap = malloc(sizeof(int) * n);
      g->adj = malloc(sizeof(int *) * n);
      
      if (g->list_size == NULL || g->list_cap == NULL || g->adj == NULL)
	{
	  free(g->list_size);
	  free(g->list_cap);
	  free(g->adj);
	  free(g);

	  return NULL;
	}

      for (int i = 0; i < n; i++)
	{
	  g->list_size[i] = 0;
	  g->adj[i] = malloc(sizeof(int) * LUGRAPH_ADJ_LIST_INITIAL_CAPACITY);
	  g->list_cap[i] = g->adj[i] != NULL ? LUGRAPH_ADJ_LIST_INITIAL_CAPACITY : 0;
	}
    }

  return g;
}

int lugraph_size(const lugraph *g)
{
  if (g != NULL)
    {
      return g->n;
    }
  else
    {
      return 0;
    }
}

void lugraph_list_embiggen(lugraph *g, int from)
{
  if (g->list_cap[from] != 0)
    {
      int *bigger = realloc(g->adj[from], sizeof(int) * g->list_cap[from] * 2);
      if (bigger != NULL)
	{
	  g->adj[from] = bigger;
	  g->list_cap[from] = g->list_cap[from] * 2;
	}
    }
}

void lugraph_add_edge(lugraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      lugraph_add_half_edge(g, from, to);
      lugraph_add_half_edge(g, to, from);
    }
}

void lugraph_add_half_edge(lugraph *g, int from, int to)
{
  if (g->list_size[from] == g->list_cap[from])
    {
      lugraph_list_embiggen(g, from);
    }
  
  if (g->list_size[from] < g->list_cap[from])
    {
      g->adj[from][g->list_size[from]++] = to;
    }
}

bool lugraph_has_edge(const lugraph *g, int from, int to)
{
  if (g != NULL && from >= 0 && to >= 0 && from < g->n && to < g->n && from != to)
    {
      int i = 0;
      while (i < g->list_size[from] && g->adj[from][i] != to)
	{
	  i++;
	}
      return i < g->list_size[from];
    }
  else
    {
      return false;
    }
}

int lugraph_degree(const lugraph *g, int v)
{
  if (g != NULL && v >= 0 && v < g->n)
    {
      return g->list_size[v];
    }
  else
    {
      return -1;
    }
}

/**
 * Run bfs on lugraph given a source
 *
 * MODIFIED FROM ASPNES NOTES
 */

lug_search *lugraph_bfs(const lugraph *g, int from)
{
  if (g == NULL || from < 0 || from >= g->n)
  {
    return NULL;
  }

  lug_search *ls = lug_search_create(g, from);

  ls->dist[from] = 0;
  ls->pred[from] = from;

  int head = 0;
  int tail = 0;
  int curr = 0;

  int *q = malloc(sizeof(int) * g->n);

  if (q == NULL)
  {
    lug_search_destroy(ls);
    return NULL;
  }

  q[tail++] = from;

  while (head < tail) {
    curr = q[head++];

    for (int i = 0; i < g->list_size[curr]; i++) {
        if(ls->pred[g->adj[curr][i]] == -1) 
        {
            /* haven't seen this guy */
            /* push it */
            ls->pred[g->adj[curr][i]] = curr;
            ls->dist[g->adj[curr][i]] = ls->dist[curr] + 1;
            q[tail++] = g->adj[curr][i];
        }
    }
  }

  free(q);
  return ls;
}


bool lugraph_connected(const lugraph *g, int from, int to)
{
  // check for valid parameters
  if (g == NULL || from < 0 || from >= g->n || to < 0 || to >= g->n || from == to)
  {
    return false;
  }

  lug_search * ls = lugraph_bfs(g, from);
  if (ls == NULL)
  {
    return false;
  }

  // check for connectivity
  if (ls->dist[to] >= 0)
  {
    lug_search_destroy(ls);
    return true;
  }

  lug_search_destroy(ls);
  return false;
}

void lugraph_destroy(lugraph *g)
{
  if (g != NULL)
    {
      for (int i = 0; i < g->n; i++)
	{
	  free(g->adj[i]);
	}
      free(g->adj);
      free(g->list_cap);
      free(g->list_size);
      free(g);
    }
}

lug_search *lug_search_create(const lugraph *g, int from)
{
  if (g != NULL && from >= 0 && from < g->n)
    {
      lug_search *s = malloc(sizeof(lug_search));
      
      if (s != NULL)
	{
	  s->g = g;
	  s->from = from;
	  s->dist = malloc(sizeof(int) * g->n);
	  s->pred = malloc(sizeof(int) * g->n);

	  if (s->dist != NULL && s->pred != NULL)
	    {
	      for (int i = 0; i < g->n; i++)
		{
		  s->dist[i] = -1;
		  s->pred[i] = -1;
		}
	    }
	  else
	    {
	      free(s->pred);
	      free(s->dist);
	      free(s);
	      return NULL;
	    }
	}

      return s;
    }
  else
    {
      return NULL;
    }
}

int lug_search_distance(const lug_search *s, int to)
{
  // check for valid input
  if (s == NULL || s->g == NULL || to < 0 || to >= s->g->n)
  {
    return -1;
  }
  else
  {
    return s->dist[to];    
  }
}

int *lug_search_path(const lug_search *s, int to, int *len)
{  
  // check for valid parameters
  if (s == NULL || s->g == NULL || to < 0 || to >= s->g->n || s->dist[to] == -1)
  {
    *len = -1;
    return NULL;
  }

  // get distance from source;
  *len = s->dist[to];

  if (*len == -1)
  {
    return NULL;
  }

  // allocate path
  int *path = malloc(sizeof(int) * (*len + 1));
  if (path == NULL)
  {
    return NULL;
  }

  // set last vertex in path as our "to" vertex
  path[*len] = to;
  if (*len == 1)
  {
    path[0] = s->from;
    return path;
  }
  else if (*len == 0)
  {
    return path;
  }
  else
  {
    // reconstruct path based on to's predecessors
    for (int i = 1; i < *len + 1; i++)
    {
      path[*len - i] = s->pred[path[*len - i + 1]];
    }
    return path;
  }
}

void lug_search_destroy(lug_search *s)
{
  if (s != NULL)
    {
      free(s->dist);
      free(s->pred);
      free(s);
    }
}
