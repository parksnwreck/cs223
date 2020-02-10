#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "location.h"
#include "lugraph.h"

typedef struct 
{
    int a;
    int b;
    double dist;
} edge;

void nearest (int citycount ,char **cities, location *citycoords);
void optimal (int citycount ,char **cities, location *citycoords);
void insnearest (int citycount, char **cities, location *citycoords);
void insfarthest (int citycount, char **cities, location *citycoords);
void greedy (int citycount, char **cities, location *citycoords); 

double gettotaldist (int citycount, int *route, location *citycoords);
int getclosest(int citycount, int start, int *route, location *citycoords);
int getfarthest(int citycount, int start, int *route, location *citycoords);
int reorderandout (double total, int citycount, int *route, char **cities);
void printres (double total, int citycount, int *route, char **cities);

void merge(int n1, const edge a1[], int n2, const edge a2[], edge out[]);
void mergeSort(int n, edge a[], edge out[]);

int main (int argc, char *argv[])
{
// check for file and read file
    if (argc < 2)
    {
        fprintf(stderr, "TSP: missing filename\n");
        exit(1);
    }

    FILE *in;
    in = fopen(argv[1], "r");

// check if file was opened properly
    if (in == NULL)
    {
        fprintf(stderr, "TSP: could not open %s\n", argv[1]);
        exit(2);
    }
    else
    {
        char line1 [1000];
        int scan1 = fscanf(in, "%1000s", line1);

        if (scan1 == EOF || scan1 == 0)
        {
            fclose(in);
            exit(3);
        }

        int citycount = atoi(line1);

        if (citycount < 2)
        {
            fprintf(stderr, "TSP: too few cities\n");
            fclose(in);
            exit(4);
        }
    
    // store array of city codes
        char **cities = malloc(sizeof(char *) * citycount);
        if (cities == NULL)
        {
            fclose(in);
            exit(3);
        }

        int successcount = 0;
        for (int i = 0; i < citycount; i++)
        {
            cities[i] = malloc(sizeof(char) * 4);
            if (cities[i] == NULL)
            {
                for (int i = 0; i < successcount; i++)
                {
                    free(cities[i]);
                }
                free(cities);
                fclose(in);
                exit(3);
            }
            successcount ++;
        }

        char citycode[4];

        for (int i = 0; i < citycount; i++)
        {
            int res = fscanf(in, "%3s", citycode);
            if (res == EOF || res == 0)
            {
                for (int j = 0; j < citycount; j++)
                {
                    free(cities[j]);
                }
                free(cities);
                fclose(in);
                exit(3);
            }
            strcpy (cities[i], citycode);
        }

    //store array of structs with coordinates
        location *citycoords = malloc(sizeof(location) * citycount);

        if (citycoords == NULL)
        {
            for (int i = 0; i < citycount; i++)
            {
                free(cities[i]);
            }
            free(cities);
            fclose(in);
            exit(3);
        }

        for (int i = 0; i < citycount; i++)
        {
            double lat,lon;
            int res = fscanf(in, "%lf %lf", &lat, &lon);
            if (res == EOF || res == 0)
            {
                for (int j = 0; j < citycount; j++)
                {
                    free(cities[j]);
                }
                free(cities);
                free(citycoords);
                fclose(in);
                exit(3);
            }
            citycoords[i].lat = lat;
            citycoords[i].lon = lon;
        }

// check if rest of the arguments are valid.
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-nearest") != 0 && strcmp(argv[i], "-optimal") != 0 && strcmp(argv[i], "-insert") != 0 && strcmp(argv[i], "-greedy") != 0)
            {
                fprintf(stderr, "TSP: invalid method %s\n", argv[i]);
                for (int j = 0; j < citycount; j++)
                {
                    free(cities[j]);
                }
                free(cities);
                free(citycoords);
                fclose(in);
                exit(5);
            }
            else if (i == argc - 1 && strcmp(argv[i], "-insert") == 0)
            {
                fprintf(stderr, "TSP: missing criterion\n");
                for (int j = 0; j < citycount; j++)
                {
                    free(cities[j]);
                }
                free(cities);
                free(citycoords);
                fclose(in);
                exit(6);
            }
            else if (strcmp(argv[i], "-insert") == 0)
            {
                i++;
                if (strcmp(argv[i], "nearest") != 0 && strcmp(argv[i], "farthest") != 0)
                {
                    fprintf(stderr, "TSP: invalid criterion %s\n", argv[i]);
                    for (int j = 0; j < citycount; j++)
                    {
                        free(cities[j]);
                    }
                    free(cities);
                    free(citycoords);
                    fclose(in);
                    exit(7);
                }
            }
        }

// run methods in argv.
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-nearest") == 0)
            {
                nearest (citycount, cities, citycoords);
            }
            else if (strcmp(argv[i], "-insert") == 0)
            {
                i++;
                if (strcmp(argv[i], "nearest")== 0)
                {
                    insnearest (citycount, cities, citycoords);
                }
                else if (strcmp(argv[i], "farthest")== 0)
                {
                    insfarthest (citycount, cities, citycoords);
                }
            }
            else if (strcmp(argv[i], "-optimal") == 0)
            {
                optimal (citycount, cities, citycoords);
            }
            else if (strcmp(argv[i], "-greedy") == 0)
            {
                greedy (citycount, cities, citycoords);
            }
        }
        
        // free everything
        for (int i = 0; i < citycount; i++)
        {
            free(cities[i]);
        }
        free(cities);
        free(citycoords);
    }
    fclose(in);
}

// application of -nearest method
void nearest (int citycount ,char **cities, location *citycoords) 
{
    double total = 0;
    
    int route[citycount];
    for (int p = 0; p < citycount; p++)
    {
        route[p] = p;
    }


    for (int k = 0; k < citycount - 1; k++)
    {
        double min = DBL_MAX;
        int clost = 0;

        for (int j = citycount - 1; j > k; j--)
        {
            double next = location_distance(&citycoords[route[k]], &citycoords[route[j]]);
            if (next <= min)
            {
                min = next;
                clost = j;
            }
        }

        // swap closet city with the next city in route.
        int tmp = route [k + 1];
        route[k + 1] = route[clost];
        route[clost] = tmp;

        // add shortest route to total route.
        total = total + min;
    }

    // add distance back to the start city
    double last_dist = location_distance(&citycoords[route[0]], &citycoords[route[citycount-1]]);
    total = total + last_dist;

    // print results
    printf("-nearest        :");
    printres(total, citycount, route, cities);
}

// application of -optimal method
void optimal (int citycount, char **cities, location *citycoords) 
{
    int route[citycount];
    for (int p = 0; p < citycount; p++)
    {
        route[p] = p;
    }

    // get total distance of given route
    double total = gettotaldist(citycount, route, citycoords) + location_distance(&citycoords[route[0]], &citycoords[route[citycount - 1]]);

    // print results
    printf("-optimal        :");
    printres(total, citycount, route, cities);
}

// application of -insert nearest
void insnearest (int citycount, char **cities, location *citycoords)
{
    int route[citycount];
    for (int p = 0; p < citycount; p++)
    {
        route[p] = p;
    }

    // find closest start pair
    int first;
    int second;
    double min = DBL_MAX;
    
    for (int i = 0; i < citycount - 1; i++)
    {
        for (int j = i + 1; j < citycount; j++)
        {
            double next = location_distance(&citycoords[route[i]], &citycoords[route[j]]);
            if (next <= min)
            {
                min = next;
                first = i;
                second = j;
            }
        }
    }

    // swap first two cities in route with the closest pair  
    int tmp = route [0];
    route[0] = first;
    route[first] = tmp;

    tmp = route[1];
    route[1] = second;
    route[second] = tmp;

    // number of cities sorted
    int comp = 2; 

    for (int i = 2; i < citycount; i++)
    {
        if (i < citycount -1 )
        {
            // get closest index
            int clost = getclosest(citycount, i, route, citycoords);

            // swap closest index
            tmp = route[i];
            route[i] = route[clost];
            route[clost] = tmp; 
        }
    
        comp++;

        // recreate current route with newly inserted city
        int curr [comp];
 
        for (int c = 0; c < comp; c++)
        {
            curr[c] = route[c];
        }

        // minimum total distance of sorted route
        min = gettotaldist(comp, curr, citycoords) + location_distance(&citycoords[curr[comp - 1]], &citycoords[curr[0]]);

        // initialize minimum route to track given "comp" number of sorted cities
        int mincurr[comp];
        for (int k = 0; k < comp; k++)
        {
            mincurr[k] = curr[k];
        }

        for (int var = 0; var < comp - 1; var++)
        {
            // at the beginning of the loop reverse array.
            if (var == 0)
            {
                tmp = curr[comp - 1];
                for (int j = 0; j < comp - 1; j++)
                {    
                    curr[comp - 1] = curr[j];
                    curr[j] = tmp;
                    tmp = curr[comp - 1];
                }
            }
            else
            {
                tmp = curr[var - 1];
                curr[var - 1] = curr[var];
                curr[var] = tmp;                
            }

            double newdist = gettotaldist(comp, curr, citycoords) + location_distance(&citycoords[curr[comp - 1]], &citycoords[curr[0]]);

            // if this variation of the route gives minimum distance, reroute.
            if (newdist <= min)
            {
                min = newdist;
                for (int k = 0; k < comp; k++)
                {
                    mincurr[k] = curr[k];
                }
            }
        }

        for (int k = 0; k < comp; k++)
        {
            route[k] = mincurr[k];
        }
    }
    
    double total = gettotaldist(citycount, route, citycoords) + location_distance(&citycoords[route[citycount - 1]], &citycoords[route[0]]);
    printf("-insert nearest :");
    reorderandout (total, citycount, route, cities);
}

// application of -insert farthest
void insfarthest (int citycount, char **cities, location *citycoords)
{
    int route[citycount];
    for (int p = 0; p < citycount; p++)
    {
        route[p] = p;
    }

    // find farthest start pair
    int first;
    int second;
    double max = 0;
    
    for (int i = 0; i < citycount - 1; i++)
    {
        for (int j = i + 1; j < citycount; j++)
        {
            double next = location_distance(&citycoords[route[i]], &citycoords[route[j]]);
            if (next >= max)
            {
                max = next;
                first = i;
                second = j;
            }
        }
    }

    // swap first two cities in route with the farthest pair  
    int tmp = route [0];
    route[0] = first;
    route[first] = tmp;

    tmp = route[1];
    route[1] = second;
    route[second] = tmp;

    int comp = 2; 

    for (int i = 2; i < citycount; i++)
    {
        if (i < citycount -1 )
        {
            int farthest = getfarthest(citycount, i, route, citycoords);
            // swap farthest index
            tmp = route[i];
            route[i] = route[farthest];
            route[farthest] = tmp;
        }
    
        comp++;
        int curr [comp];
 
        for (int c = 0; c < comp; c++)
        {
            curr[c] = route[c];
        }

        double min = gettotaldist(comp, curr, citycoords) + location_distance(&citycoords[curr[comp - 1]], &citycoords[curr[0]]);

        int mincurr[comp];
        for (int k = 0; k < comp; k++)
        {
            mincurr[k] = curr[k];
        }

        for (int var = 0; var < comp - 1; var++)
        {
            if (var == 0)
            {
                tmp = curr[comp - 1];
                for (int j = 0; j < comp - 1; j++)
                {    
                    curr[comp - 1] = curr[j];
                    curr[j] = tmp;
                    tmp = curr[comp - 1];
                }
            }
            else
            {
                tmp = curr[var - 1];
                curr[var - 1] = curr[var];
                curr[var] = tmp;                
            }

            double newdist = gettotaldist(comp, curr, citycoords) + location_distance(&citycoords[curr[comp - 1]], &citycoords[curr[0]]);

            if (newdist <= min)
            {
                min = newdist;
                for (int k = 0; k < comp; k++)
                {
                    mincurr[k] = curr[k];
                }
            }
        }

        for (int k = 0; k < comp; k++)
        {
            route[k] = mincurr[k];
        }
    }
    
    double total = gettotaldist(citycount, route, citycoords) + location_distance(&citycoords[route[citycount - 1]], &citycoords[route[0]]);
    printf("-insert farthest:");
    reorderandout (total, citycount, route, cities);
}

void greedy (int citycount, char **cities, location *citycoords)
{
    // initialize array of all possible edges
    edge *unsorted = malloc(sizeof(edge) * (citycount * (citycount - 1) / 2));
    if (unsorted == NULL)
    {
        return;
    }

    int count = 0;
    for (int i = 0; i < citycount - 1; i++)
    {
        for (int j = i+1; j < citycount; j++)
        {
            unsorted[count].a = i;
            unsorted[count].b = j;
            unsorted[count].dist = location_distance(&citycoords[i], &citycoords[j]);
            count++;
        }
    }

    // merge sort unsorted array of edges based on increasing distance
    edge *edges = malloc(sizeof(edge) * (citycount * (citycount - 1) / 2));
    if (edges == NULL)
    {
        free(unsorted);
        return;
    }
    mergeSort((citycount * (citycount - 1) / 2), unsorted, edges);
    free(unsorted);

    // create graph
    lugraph *g = lugraph_create(citycount);
    if (g == NULL)
    {
        free(edges);
        return;
    }

    // add edge into graph if the two vertices have not yet been connected and the degrees of both vertices are at most one.
    for (int i = 0; i < (citycount * (citycount - 1) / 2); i++)
    {
        int v1 = edges[i].a;
        int v2 = edges[i].b;
        if (!lugraph_connected(g, v1, v2) && lugraph_degree(g, v1) < 2 && lugraph_degree(g, v2) < 2)
        {
            lugraph_add_edge(g, v1, v2);
        }
    }

    // find start and end points of path
    int start = -1;
    int end = -1;

    for (int i = 0; i < citycount; i++)
    {
        if (lugraph_degree(g, i) == 1 && start == -1)
        {
            start = i;
        }
        else if (lugraph_degree(g, i) == 1 && start != -1 && end == -1)
        {
            end = i;
        }
    }

    // run bfs and construct path
    int len;
    lug_search *ls = lugraph_bfs(g, start);
    if (ls == NULL)
    {
        free(edges);
        lugraph_destroy(g);
        return;
    }
    int *route = lug_search_path(ls, end, &len);

    // calculate total distance and output
    double total_dist = gettotaldist(citycount, route, citycoords) + location_distance(&citycoords[route[citycount - 1]], &citycoords[route[0]]);
    printf("-greedy         :");
    reorderandout(total_dist, citycount, route, cities);

    // free everything
    free(edges);
    free(route);
    lug_search_destroy(ls);
    lugraph_destroy(g);
}


// reorder array as specified and print
int reorderandout (double total, int citycount, int *route, char **cities)
{
    int start;

    int *printroute = malloc(sizeof(int) * (citycount + 1));
    if (printroute == NULL)
    {
        return 0;
    }

    int *printroute2 = malloc(sizeof(int) * (citycount + 1));
    if (printroute2 == NULL)
    {
        free(printroute);
        return 0;
    }

    for (int i = 0; i < citycount; i++)
    {
        if (route[i] == 0)
        {
            start = i;
            if (start != 0 && start != citycount - 1)
            {
                for (int k = start; k < citycount; k ++)
                {
                    printroute[k - start] = route[k];
                }
                for (int k = 0; k < start; k ++)
                {
                    printroute[citycount - start + k] = route[k];
                }

                if (printroute[1] < printroute[citycount - 1])
                {
                    printroute[citycount] = printroute[0];
                    printres(total, citycount, printroute, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
                else
                {
                    printroute[citycount] = printroute[0];
                    for (int k = 0; k < citycount + 1; k++)
                    {
                        printroute2[citycount - k] = printroute[k];
                    }
                    printres(total, citycount, printroute2, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
            }
            else if (start == 0)
            {
                if (route[1] < route[citycount - 1])
                {
                    for (int k = 0; k < citycount; k++)
                    {
                        printroute[k] = route[k];
                    }
                    printroute[citycount] = printroute[0];
                    printres(total, citycount, printroute, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
                else
                {
                    for (int k = 0; k < citycount; k++)
                    {
                        if (k == 0)
                        {
                            printroute[0] = route[0];
                        }
                        else
                        {
                            printroute[citycount - k] = route[k];
                        }
                    }
                    printroute[citycount] = printroute[0];
                    printres(total, citycount, printroute, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
            }
            else if (start == citycount - 1)
            {
                if (route[0] < route[citycount - 2])
                {
                    for (int k = 0; k < citycount; k++)
                    {
                        if (k == 0)
                        {
                            printroute[0] = route[citycount - 1];
                        }
                        else
                        {
                            printroute[k] = route[k - 1];
                        }
                    }
                    printroute[citycount] = printroute[0];
                    printres(total, citycount, printroute, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
                else
                {
                    for (int k = 0; k < citycount; k++)
                    {
                        if (k == 0)
                        {
                            printroute[0] = route[citycount - 1];
                        }
                        else
                        {
                            printroute[k] = route[citycount - 1 - k];
                        }
                    }
                    printroute[citycount] = printroute[0];
                    printres(total, citycount, printroute, cities);
                    free(printroute);
                    free(printroute2);
                    return 1;
                }
            }
        }
    }
    free(printroute);
    free(printroute2);
    return 0;
}

//get total distance of given route
double gettotaldist (int citycount, int *route, location *citycoords)
{
    double total = 0;

    for (int i = 0; i < citycount - 1; i++)
    {
        double next = location_distance(&citycoords[route[i]], &citycoords[route[i + 1]]);
        total = total + next;
    }

    return total;
}

//get index of the "nearest" city for insert
int getclosest(int citycount, int start, int *route, location *citycoords)
{
    int clost = 0;
    double closestdist = DBL_MAX;

    for (int i = start; i < citycount; i++)
    {
        for(int j = 0; j < start; j++)
        {
            double next = location_distance(&citycoords[route[j]], &citycoords[route[i]]);
            if (next <= closestdist)
            {
                closestdist = next;
                clost = i;
            }
        }
    }
    return clost;
}

//get index of the "farthest" city for insert
int getfarthest(int citycount, int start, int *route, location *citycoords)
{
    int farthest = 0;
    double farthestdist = 0;

    for (int i = start; i < citycount; i++)
    {
        for(int j = 0; j < start; j++)
        {
            double next = location_distance(&citycoords[route[j]], &citycoords[route[i]]);
            if (next >= farthestdist)
            {
                farthestdist = next;
                farthest = i;
            }
        }
    }
    return farthest;
}

// print results given cities, routes and total distance
void printres (double total, int citycount, int *route, char **cities)
{
    printf("% 10.2f ", total);

    for (int k = 0; k < citycount; k++)
    {
        int citind = route[k];
        printf("%s ", cities[citind]);
    }
    printf("%s\n", cities[route[0]]);
}

/**
 * Merge based on increasing distance
 *
 * MODIFIED FROM ASPNES NOTES
 */

void merge(int n1, const edge a1[], int n2, const edge a2[], edge out[])
{
    int i1;
    int i2;
    int iout;

    i1 = i2 = iout = 0;

    while(i1 < n1 || i2 < n2) {
        if(i2 >= n2 || ((i1 < n1) && (a1[i1].dist < a2[i2].dist))) {
            /* a1[i1] exists and is smaller */
            out[iout++] = a1[i1++];
        }  else {
            /* a1[i1] doesn't exist, or is bigger than a2[i2] */
            out[iout++] = a2[i2++];
        }
    }
}

/**
 * Mergesort based on distances
 *
 * MODIFIED FROM ASPNES NOTES
 */

void mergeSort(int n, edge a[], edge out[])
{
    edge *a1;
    edge *a2;

    if(n < 2) {
        /* 0 or 1 elements is already sorted */
        memcpy(out, a, sizeof(edge) * n);
    } else {
        /* sort into temp arrays */
        a1 = malloc(sizeof(edge) * (n/2));
        a2 = malloc(sizeof(edge) * (n - n/2));

        mergeSort(n/2, a, a1);
        mergeSort(n - n/2, a + n/2, a2);

        /* merge results */
        merge(n/2, a1, n - n/2, a2, out);

        /* free the temp arrays */
        free(a1);
        free(a2);
    }
}
