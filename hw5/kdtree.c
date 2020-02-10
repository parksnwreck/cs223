#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "kdtree.h"
#include "location.h"

// define node struct
typedef struct node {
    location loc;
    bool mode;
    struct node *left;
    struct node *right;
} node;

// define kdtree struct
struct _kdtree {
    int size;
    node *root;
};

void mergex(int n1, const location a1[], int n2, const location a2[], location out[]);
void mergey(int n1, const location a1[], int n2, const location a2[], location out[]);
void mergeSort(int n, const location a[], location out[], bool mode);
node *buildtree(kdtree *t, int n, const location cut[], const location other[], node *branch, bool mode);
bool FindNode(node *root, const location *p, bool mode);
node *AddNode(kdtree *t, node *root, const location *p, bool mode);
void FreeNodes(node *root);
void ForEachNode(node* root, void (*f)(const location *, void *), void *arg);
void NearestN(node *curr, location *lleft, location *uright, const location *p, location *neighbor, double *d, bool mode);

/**
 * Creates a balanced k-d tree containing copies of the points in the
 * given array of locations.  If the array is NULL and n is 0 then the
 * returned tree is empty.  If the tree could not be created then the
 * returned value is NULL.
 *
 * @param pts an array of unique valid locations, non-NULL if n != 0
 * @param n the number of points in that array, or 0 if pts is NULL
 * @return a pointer to the newly created collection of points, or NULL
 * if it could not be created
 */
kdtree *kdtree_create(const location *pts, int n)
{
    // check for correct n value
    if (n < 0)
    {
        return NULL;
    }

    // initialize kdtree
    kdtree *tree = malloc(sizeof(kdtree));
    if (tree == NULL)
    {
        return NULL;
    }
    tree->size = 0;

    // sort locations based on x and y coordinates
    location *xsorted = malloc(sizeof(location) * n);
    if (xsorted == NULL)
    {
        return NULL;
    }
    mergeSort(n, pts, xsorted, true);

    location *ysorted = malloc(sizeof(location) * n);
    if (ysorted == NULL)
    {
        return NULL;
    }
    mergeSort(n, pts, ysorted, false);

    // build the tree
    tree->root = buildtree(tree, n, xsorted, ysorted, tree->root, true);

    // free sorted arrays
    free(xsorted);
    free(ysorted);

    return tree;
}

/**
 * Adds a copy of the given point to the given k-d tree.  There is no
 * effect if the point is already in the tree.  The tree need not be
 * balanced after the add.  The return value is true if the point was
 * added successfully and false otherwise (if the point was already in the
 * tree or if there was a memory allocation error when trying to add).
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 * @return true if and only if the point was successfully added
 */
bool kdtree_add(kdtree *t, const location *p)
{
    if (t == NULL || p == NULL)
    {
        return false;
    }

    // store size of the tree prior to adding a node
    int prevsize = t->size;

    // add node
    t->root = AddNode(t, t->root, p, true);

    // check if new node was added properly
    if (t->size > prevsize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Determines if the given tree contains a point with the same coordinates
 * as the given point.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 * @return true if and only of the tree contains the location
 */
bool kdtree_contains(const kdtree *t, const location *p)
{
    if (t == NULL || p == NULL)
    {
        return false;
    }
    return FindNode(t->root, p, true);
}

/**
 * Copies the nearest neighbor to the given point among points in the
 * given tree to the given location.  Ties are broken arbitrarily.
 * There is no change to neighbor and distance is set to infinity if
 * the tree is empty.  If p is equal to a point in the tree then p is
 * copied into neighbor and distance is set to zero.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param p a pointer to a valid location, non-NULL
 * @param d a pointer to a double, non-NULL
 * @param neighbor a pointer to a valid location, non-NULL
 */
void kdtree_nearest_neighbor(kdtree *t, const location *p, location *neighbor, double *d)
{
    if (t == NULL || p == NULL || d == NULL || neighbor == NULL)
    {
        return;
    }

    node *root = t->root;

    // set lower left corner of initial box
    location *lleft = malloc(sizeof(location));
    if (lleft == NULL)
    {
        return;
    }
    lleft->x = INFINITY * -1;
    lleft->y = INFINITY * -1;

    // set upper right corner of initial box
    location *uright = malloc(sizeof(location));
    if (uright == NULL)
    {
        free(lleft);
        return;
    }
    uright->x = INFINITY;
    uright->y = INFINITY;

    // set initial minimum distance
    *d = INFINITY;

    // run nearest neighbor
    NearestN(root, lleft, uright, p, neighbor, d, true);

    // free everything
    free(lleft);
    free(uright);
    
    return;
}

/**
 * Passes the points in the given tree to the given function
 * in an arbitrary order.  The last argument to this function is also passed
 * to the given function along with each point.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * @param f a pointer to a function that takes a location and an extra
 * argument, and does not modify t, non-NULL
 * @param arg a pointer
 */
void kdtree_for_each(const kdtree* r, void (*f)(const location *, void *), void *arg)
{
    if (r == NULL || f == NULL)
    {
        return;
    }

    ForEachNode(r->root, f, arg);
}

/**
 * Destroys the given k-d tree.  The tree is invalid after being destroyed.
 *
 * @param t a pointer to a valid k-d tree, non-NULL
 * 
 */
void kdtree_destroy(kdtree *t)
{
    if (t == NULL)
    {
        return;
    }

    FreeNodes(t->root);
    free(t);
}

/**
 * Merge two arrays based on their x coordinates
 *
 * MODIFIED FROM ASPNES NOTES
 */

void mergex(int n1, const location a1[], int n2, const location a2[], location out[])
{
    int i1;
    int i2;
    int iout;

    i1 = i2 = iout = 0;

    while(i1 < n1 || i2 < n2) {
        if(i2 >= n2 || ((i1 < n1) && (a1[i1].x < a2[i2].x))) {
            /* a1[i1] exists and is smaller */
            out[iout++] = a1[i1++];
        }  else {
            /* a1[i1] doesn't exist, or is bigger than a2[i2] */
            out[iout++] = a2[i2++];
        }
    }
}

/**
 * Merge two arrays based on their y coordinates
 *
 * MODIFIED FROM ASPNES NOTES
 */

void mergey(int n1, const location a1[], int n2, const location a2[], location out[])
{
    int i1;
    int i2;
    int iout;

    i1 = i2 = iout = 0;

    while(i1 < n1 || i2 < n2) {
        if(i2 >= n2 || ((i1 < n1) && (a1[i1].y < a2[i2].y))) {
            /* a1[i1] exists and is smaller */
            out[iout++] = a1[i1++];
        }  else {
            /* a1[i1] doesn't exist, or is bigger than a2[i2] */
            out[iout++] = a2[i2++];
        }
    }
}

/**
 * Mergesort based on mode. If mode == true, mergesort based on x coordinate, if not
 * mergesort based on y coordinate.
 *
 * MODIFIED FROM ASPNES NOTES
 */

void mergeSort(int n, const location a[], location out[], bool mode)
{
    location *a1;
    location *a2;

    if(n < 2) {
        /* 0 or 1 elements is already sorted */
        memcpy(out, a, sizeof(location) * n);
    } else {
        /* sort into temp arrays */
        a1 = malloc(sizeof(location) * (n/2));
        a2 = malloc(sizeof(location) * (n - n/2));

        mergeSort(n/2, a, a1, mode);
        mergeSort(n - n/2, a + n/2, a2, mode);

        /* merge results */
        if (mode == true)
        {
            mergex(n/2, a1, n - n/2, a2, out);
        }
        else
        {
            mergey(n/2, a1, n - n/2, a2, out);
        }

        /* free the temp arrays */
        free(a1);
        free(a2);
    }
}

/**
 * Recursive function to build tree.
 * 
 */

node* buildtree(kdtree* t, int n, const location cut[], const location other[], node *branch, bool mode)
{
    // establish base cases of n = 0 and 1;
    if (n == 0)
    {
        return NULL;
    }
    else if (n == 1)
    {
        node *leaf = malloc(sizeof(node));
        if (leaf == NULL)
        {
            return NULL;
        }
        leaf->loc = cut[0];
        leaf->mode = mode;
        leaf->left = NULL;
        leaf->right = NULL;
        t->size ++;
        return leaf;
    }
    else
    {
        // establish left and right sizes based on the middle of the array
        int mid = n/2;
        int leftsize = n/2;
        int rightsize = n - 1 - n/2;

        // make a new node and set its location equal to the mid of the cut
        node *newnode = malloc(sizeof(node));
        if (newnode == NULL)
        {
            return NULL;
        }
        newnode->loc = cut[mid];
        newnode->mode = mode;

        // create a new array for the left half of cut
        location *cutleft = malloc(sizeof(location) * leftsize);
        if (cutleft == NULL)
        {
            return NULL;
        }

        // set values of cutleft
        for (int i = 0; i < leftsize; i++)
        {
            cutleft[i] = cut[i];
        }

        // create a new array for the right half of cut
        location *cutright = malloc(sizeof(location) * rightsize);
        if (cutright == NULL)
        {
            free(cutleft);
            return NULL;
        }

        // set values of cutright
        for (int i = 0; i < rightsize; i++)
        {
            cutright[i] = cut[n/2 + 1 + i];
        }

        // create new arrays for left and right half of other
        location *otherleft = malloc(sizeof(location) * leftsize);
        if (otherleft == NULL)
        {
            free(cutleft);
            free(cutright);
            return NULL;
        }

        location *otherright = malloc(sizeof(location) * rightsize);
        if (otherright == NULL)
        {
            free(cutleft);
            free(cutright);
            free(otherright);
            return NULL;
        }

        int countl = 0;
        int countr = 0;

        // split other by x
        if (mode == true)
        {
            for (int i = 0; i < n; i++)
            {
                if (other[i].x < cut[mid].x)
                {
                    otherleft[countl] = other[i];
                    countl++;
                }
                else if (other[i].x > cut[mid].x)
                {
                    otherright[countr] = other[i];
                    countr++;
                }
                // if yxvalues are the same, compare by y values
                else if (other[i].x == cut[mid].x)
                {
                    if (other[i].y < cut[mid].y)
                    {
                        otherleft[countl] = other[i];
                        countl++;
                    }
                    else if (other[i].y > cut[mid].y)
                    {
                        otherright[countr] = other[i];
                        countr++;
                    }
                }
            }
        }
        // split other by y
        else if (mode == false)
        {
            for (int i = 0; i < n; i++)
            {
                if (other[i].y < cut[mid].y)
                {
                    otherleft[countl] = other[i];
                    countl++;
                }
                else if (other[i].y > cut[mid].y)
                {
                    otherright[countr] = other[i];
                    countr++;
                }
                // if y values are the same, compare by x values
                else if (other[i].y == cut[mid].y)
                {
                    if (other[i].x < cut[mid].x)
                    {
                        otherleft[countl] = other[i];
                        countl++;
                    }
                    else if (other[i].x > cut[mid].x)
                    {
                        otherright[countr] = other[i];
                        countr++;
                    }
                }
            }
        }

        // recursively call on the left and right side
        newnode->left = buildtree(t, leftsize, otherleft, cutleft, newnode->left, !mode);
        newnode->right = buildtree(t, rightsize, otherright, cutright, newnode->right, !mode);
        t->size++;

        // free everything
        free(cutleft);
        free(cutright);
        free(otherleft);
        free(otherright);

        return newnode;
    }
}

/**
 * Recursive function to add node in a tree depending on parameters.
 * 
 */

node *AddNode(kdtree *t, node *root, const location *p, bool mode)
{
    // establish base case of when the current node is a NULL pointer
    if (root == NULL) 
    {
        root = malloc(sizeof(node));
        if (root == NULL)
        {
            return NULL;
        }
        root->loc = *p;
        root->mode = mode;
        root->left = NULL;
        root->right = NULL;
        t->size ++;
        return root;
    } 

    // get the location of current node
    location currloc = root->loc;

    // if the current node has the same location as the point we are trying to add, don't add.
    if (currloc.x == p->x && currloc.y == p->y)
    {
        return root;
    }
    // compare by y if mode is false
    else if (mode == false) 
    {
        if (p->y < currloc.y)
        {
            root->left = AddNode(t, root->left, p, !mode);
            return root;
        }
        else if (p->y > currloc.y)
        {
            root->right = AddNode(t, root->right, p, !mode);
            return root;
        }
        // compare by x if y values are the same
        else if (p->y == currloc.y)
        {
            if (p->x < currloc.x)
            {
                root->left = AddNode(t, root->left, p, !mode);
                return root;
            }
            else if (p->x > currloc.x)
            {
                root->right = AddNode(t, root->right, p, !mode);
                return root;
            }
        }
    } 
    // compare by x if mode is true
    else 
    {
        if (p->x < currloc.x)
        {
            root->left = AddNode(t, root->left, p, !mode);
            return root;
        }
        else if (p->x > currloc.x)
        {
            root->right = AddNode(t, root->right, p, !mode);
            return root;
        }
        // compare by y if x values are the same
        else if (p->x == currloc.x)
        {
            if (p->y < currloc.y)
            {
                root->left = AddNode(t, root->left, p, !mode);
                return root;
            }
            else if (p->y > currloc.y)
            {
                root->right = AddNode(t, root->right, p, !mode);
                return root;
            }            
        }
    }
    return NULL;
}

/**
 * Recursive function to find node in a tree depending on parameters.
 * 
 */

bool FindNode(node *root, const location *p, bool mode)
{
    // establish base case of when current node is a NULL pointer.
    if (root == NULL) 
    {
        return false;
    } 

    location currloc = root->loc;

    // check if current node has the same location as the query location.
    if (currloc.x == p->x && currloc.y == p->y)
    {
        return true;
    }
    // compare by y if mode is false
    else if (mode == false) 
    {
        if (p->y < currloc.y)
        {
            return FindNode(root->left, p, !mode);
        }
        else if (p->y > currloc.y)
        {
            return FindNode(root->right, p, !mode);
        }
        // compare by x if y values are the same
        else if (p->y == currloc.y)
        {
            if (p->x < currloc.x)
            {
                return FindNode(root->left, p, !mode);
            }
            else if (p->x > currloc.x)
            {
                return FindNode(root->right, p, !mode);
            }
        }
    } 
    // compare by x if mode is true
    else 
    { 
        if (p->x < currloc.x)
        {
            return FindNode(root->left, p, !mode);
        }
        else if (p->x > currloc.x)
        {
            return FindNode(root->right, p, !mode);
        }
        // compare by y if x values are the same
        else if (p->x == currloc.x)
        {
            if (p->y < currloc.y)
            {
                return FindNode(root->left, p, !mode);
            }
            else if (p->y > currloc.y)
            {
                return FindNode(root->right, p, !mode);
            }            
        }
    }
    return false;
}

/**
 * Recursive function to free all nodes in a tree depending on parameters.
 * 
 */

void FreeNodes(node *root)
{
    if (root == NULL)
    {
        return;
    }
    // free all nodes using post order traversal
    FreeNodes(root->left);  
    FreeNodes(root->right);
    free(root);
}

/**
 * Recursive function call passed function on each node.
 * 
 */

void ForEachNode(node* root, void (*f)(const location *, void *), void *arg)
{
    if (root == NULL)
    {
        return;
    }
    // call function on each node using post order traversal
    ForEachNode(root->left, f, arg);
    ForEachNode(root->right, f, arg);
    location currloc = root->loc;
    f(&currloc, arg);
}

/**
 * Recursive function for nearest neighbor.
 * 
 */

void NearestN(node *curr, location *lleft, location *uright, const location *p, location *neighbor, double *d, bool mode)
{
    // establish base cases
    if (curr == NULL || *d == 0)
    {
        return;
    }
    else if (location_distance_to_rectangle(&(curr->loc), lleft, uright) >= *d)
    {
        return;
    }

    double currdist = location_distance(&(curr->loc), p);

    // save new minimum distance and location if needed
    if (currdist < *d)
    {
        *neighbor = curr->loc;
        *d = currdist;
    }

    // allocate a new upper right corner of box
    location *newuright = malloc(sizeof(location));
    if (newuright == NULL)
    {
        return;
    }
    newuright->x = uright->x;
    newuright->y = uright->y;

    // allocate a new lower left corner of box
    location *newlleft = malloc(sizeof(location));
    if (newlleft == NULL)
    {
        free(newuright);
        return;
    }
    newlleft->x = lleft->x;
    newlleft->y = lleft->y;

    // compare by x if mode is true and set new parameters for the corners of the new box
    if (mode == true && p->x < curr->loc.x)
    {
        newuright->x = curr->loc.x;
        newlleft->x = curr->loc.x;

        NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
        NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
    }
    else if (mode == true && p->x > curr->loc.x)
    {
        newuright->x = curr->loc.x;
        newlleft->x = curr->loc.x;

        NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
        NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
    }
    // compare by y if mode is true and x values are the same
    else if (mode == true && p->x == curr->loc.x)
    {
        if (p->y < curr->loc.y)
        {
            newuright->y = curr->loc.y;
            newlleft->y = curr->loc.y;

            NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
            NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
        }
        else if (p->y > curr->loc.y)
        {
            newuright->y = curr->loc.y;
            newlleft->y = curr->loc.y;

            NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
            NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
        }
    }
    // compare by y if mode is false and set new parameters for the corners of the new box
    else if (mode == false && p->y < curr->loc.y)
    {
        newuright->y = curr->loc.y;
        newlleft->y = curr->loc.y;

        NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
        NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
    }
    else if (mode == false && p->y > curr->loc.y)
    {
        newuright->y = curr->loc.y;
        newlleft->y = curr->loc.y;

        NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
        NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
    }
    // compare by x if mode is false and y values are the same
    else if (mode == false && p->y == curr->loc.y)
    {
        if (p->x < curr->loc.x)
        {
            newuright->x = curr->loc.x;
            newlleft->x = curr->loc.x;

            NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
            NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
        }
        else if (p->x > curr->loc.x)
        {
            newuright->x = curr->loc.x;
            newlleft->x = curr->loc.x;

            NearestN(curr->right, newlleft, uright, p, neighbor, d, !mode);
            NearestN(curr->left, lleft, newuright, p, neighbor, d, !mode);
        }
    }

    free(newuright);
    free(newlleft);
}