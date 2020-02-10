#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

#include "location.h"

void nearest (int citycount ,char cities [citycount][4], location citycoords [citycount]);
void optimal (int citycount ,char cities [citycount][4], location citycoords [citycount]);
void insnearest (int citycount, char cities [citycount][4], location citycoords [citycount]);
void insfarthest (int citycount, char cities [citycount][4], location citycoords [citycount]);

double gettotaldist (int citycount, int route[citycount], location citycoords [citycount]);
int getclosest(int citycount, int start, int route [citycount], location citycoords [citycount]);
int getfarthest(int citycount, int start, int route [citycount], location citycoords [citycount]);
int reorderandout (double total, int citycount, int route [citycount], char cities [citycount][4]);
void printres (double total, int citycount, int route [citycount + 1], char cities [citycount][4]);

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
            exit(3);
        }

        int citycount = atoi(line1);

        if (citycount < 2)
        {
            fprintf(stderr, "TSP: too few cities\n");
            exit(4);
        }
    
    // store array of city codes
        char cities[citycount][4];
        char citycode[4];

        for (int i = 0; i < citycount; i++)
        {
            int res = fscanf(in, "%3s", citycode);
            if (res == EOF || res == 0)
            {
                exit(3);
            }
            strcpy (cities[i], citycode);
        }

    //store array of structs with coordinates
        location citycoords[citycount];

        for (int i = 0; i < citycount; i++)
        {
            double lat,lon;
            int res = fscanf(in, "%lf %lf", &lat, &lon);
            if (res == EOF || res == 0)
            {
                exit(3);
            }
            location city;
            city.lat = lat;
            city.lon = lon;
            citycoords[i] = city;
        }

// check if rest of the arguments are valid.
        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-nearest") != 0 && strcmp(argv[i], "-optimal") != 0 && strcmp(argv[i], "-insert") != 0)
            {
                fprintf(stderr, "TSP: invalid method %s\n", argv[i]);
                exit(5);
            }
            else if (i == argc - 1 && strcmp(argv[i], "-insert") == 0)
            {
                fprintf(stderr, "TSP: missing criterion\n");
                exit(6);
            }
            else if (strcmp(argv[i], "-insert") == 0)
            {
                i++;
                if (strcmp(argv[i], "nearest") != 0 && strcmp(argv[i], "farthest") != 0)
                {
                    fprintf(stderr, "TSP: invalid criterion %s\n", argv[i]);
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
        }
    }
    fclose(in);
}

// application of -nearest method
void nearest (int citycount ,char cities [citycount][4], location citycoords [citycount]) 
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
void optimal (int citycount, char cities [citycount][4], location citycoords [citycount]) 
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
void insnearest (int citycount, char cities [citycount][4], location citycoords [citycount])
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
void insfarthest (int citycount, char cities [citycount][4], location citycoords [citycount])
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

// reorder array as specified and print
int reorderandout (double total, int citycount, int route [citycount], char cities [citycount][4])
{
    int start;
    int printroute[citycount + 1];
    int printroute2[citycount + 1];

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
                    return 1;
                }
            }
        }
    }
    return 0;
}

//get total distance of given route
double gettotaldist (int citycount, int route[citycount], location citycoords [citycount])
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
int getclosest(int citycount, int start, int route [citycount], location citycoords [citycount])
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
int getfarthest(int citycount, int start, int route [citycount], location citycoords [citycount])
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
void printres (double total, int citycount, int route [citycount], char cities [citycount][4])
{
    printf("% 10.2f ", total);

    for (int k = 0; k < citycount; k++)
    {
        int citind = route[k];
        printf("%s ", cities[citind]);
    }
    printf("%s\n", cities[route[0]]);
}