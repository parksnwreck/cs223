#ifndef __LUGRAPH_H__
#define __LUGRAPH_H__

#include <stdbool.h>

typedef struct lugraph lugraph;
typedef struct lug_search lug_search;

/**
 * Creates a new undirected graph with the given number of vertices.  The
 * vertices will be numbered 0, ..., n-1.
 *
 * @param n a nonnegative integer
 * @return a pointer to the new graph
 */
lugraph *lugraph_create(int n);

/**
 * Returns the number of vertices in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @return the number of vertices in that graph
 */
int lugraph_size(const lugraph *g);

/**
 * Adds an undirected edge between the given pair of vertices to
 * the given undirected graph.  The behavior is undefined if the edge
 * already exists.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph, not equal to from
 */
void lugraph_add_edge(lugraph *g, int v1, int v2);

/**
 * Determines if the given undirected graph contains an edge between
 * the given vertices.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph, not equal to from
 * @return true if and only if the edge exists
 */
bool lugraph_has_edge(const lugraph *g, int v1, int v2);

/**
 * Returns the degree of the given vertex in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v the index of a vertex in the given graph
 * @return the degree of v
 */
int lugraph_degree(const lugraph *g, int v);

/**
 * Returns the result of running breadth-first search on the given
 * graph starting with the given vertex.  When the search arrives
 * at a vertex, its neighbors are considered in the order the
 * corresponding edges were added to the graph.  The result is returned
 * as a pointer to a lug_search that must subsequently be passed to
 * lugraph_search_destroy. 
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param from the index of a vertex in the given graph
 * @return a pointer to the result of the search
 */
lug_search *lugraph_bfs(const lugraph *g, int from);

/**
 * Determines if the two given vertices are connected by some path
 * in the given graph.
 *
 * @param g a pointer to an undirected graph, non-NULL
 * @param v1 the index of a vertex in the given graph
 * @param v2 the index of a vertex in the given graph
 * @return true if the two vertices are connected in the graph, false otherwise
 */
bool lugraph_connected(const lugraph *g, int v1, int v2);

/**
 * Destroys the given undirected graph.
 *
 * @param g a pointer to a undirected graph, non-NULL
 */
void lugraph_destroy(lugraph *g);

/**
 * Returns the path from the staring point of the given search to the
 * given vertex.  The returned path starts with the origin of the
 * search and ends with the to vertex.  If no such path exists then
 * the return value is NULL and the length is -1.  The integer pointed
 * to by len is set to the length of (number of edges on) the path.
 * It is the caller's responsibility to free the returned array.
 *
 * @param s a pointer to a search result, non-NULL
 * @param to a vertex in the graph the search was performed in
 * @param len a pointer to an int, non-NULL
 * @return an array containing the vertices on the path, or NULL if there
 * is no such path
 */
int *lug_search_path(const lug_search *s, int to, int *len);

/**
 * Returns the distance from the source to the given vertex after the
 * given search.  If the vertex was not reached then then return value is
 * -1.
 *
 * @param s a pointer to a search result, non-NULL
 * @param to a vertex in the graph the search was performed in
 * @return the distance from the source of the search to that vertex, or -1
 */
int lug_search_distance(const lug_search *s, int to);

/**
 * Destroys the given search result.
 *
 * @param s a pointer to a search result, non-NULL
 */
void lug_search_destroy(lug_search *s);

#endif
