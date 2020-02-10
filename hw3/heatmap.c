#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "track.h"

int main (int argc, char *argv[])
{
    // check for correct argc
    if (argc != 5)
    {
        exit(0);
    }

    double width = atof(argv[1]);
    double height = atof(argv[2]);

    // check for correct width and height
    if (width <= 0.0 || width > 360.0 || height <= 0.0 || height > 180.0)
    {
        exit(1);
    }

    // count number of characters in string
    int kcount = strlen(argv[3]);

    // check if n is a valid integer
    int n = atoi(argv[4]);
    if (n <= 0)
    {
        exit(2);
    }

    // make track
    track* trk = track_create();
    if (trk == NULL)
    {
        exit (7);
    }

    // read stdin
    double lat, lon;
    long time;
    char line[5001];
    char c;

    while (fgets(line, 5001, stdin) != NULL)
    {
        if (sscanf(line, "%lf %lf %ld", &lat, &lon, &time) == 3)
        {
            // check if pt was created properly;
            trackpoint* pt = trackpoint_create(lat, lon, time);
            if (pt == NULL)
            {
                track_destroy(trk);
                exit(3);
            }

            bool add_point = track_add_point(trk, pt);
            trackpoint_destroy(pt);

            // check if point was added properly;
            if (add_point == false)
            {
                track_destroy(trk);
                exit(4);
            }
        }
        else if (sscanf(line, "%c", &c) == 1)
        {
            // check for line with just a newline character to make segment
            if (c == '\n')
            {
                track_start_segment(trk);
            }
            else
            {
                track_destroy(trk);
                exit(5);
            }
        }
        else
        {
            track_destroy(trk);
            exit(6);
        }  
    }

    // make heatmap
    int rows;
    int cols;
    int** heatmap;
    track_heatmap(trk, width, height, &heatmap, &rows, &cols);

    // print correct character given kcount and n parameter
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int curr_count = heatmap[i][j];
            int ind = curr_count/n;

            if (ind >= kcount)
            {
                putchar(argv[3][kcount - 1]);
            }
            else
            {
                putchar(argv[3][ind]);
            }
        }
        putchar('\n');
    }
    
    // free heatmap
    for (int i = 0; i < rows; i++)
    {
        free(heatmap[i]);
    }
    free(heatmap);

    // destroy track
    track_destroy(trk);
}