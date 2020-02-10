#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "track.h"

/**
 * Implementation of track struct.
 * 
 */

typedef struct segment
{
    trackpoint ** trkpts;
    double length;
    int size;
    int cap;
} segment;

struct track {
    segment *segments;
    int trk_size;
    int cap;
};

/**
 * Creates a track with one empty segment.
 *
 * @return a pointer to the new track, or NULL if there was an allocation error
 */

track * track_create()
{
    track  *trk = malloc(sizeof(track));
    if (trk != NULL)
    {
        trk->segments = malloc(sizeof(segment));
        trk->trk_size = 1;
        trk->cap = 1;
        if (trk->segments != NULL)
        {
            trk->segments[0].trkpts = malloc(sizeof(trackpoint*));
            if (trk->segments[0].trkpts != NULL)
            {
                trk->segments[0].length = 0;
                trk->segments[0].size = 0;
                trk->segments[0].cap = 1;
                return trk;
            }
        }
    }
    return NULL;
}

/**
 * Destroys the given track, releasing all memory held by it.
 *
 * @param trk a pointer to a valid track
 */

void track_destroy(track *trk)
{
    // check for valid trk
    if (trk == NULL)
    {
        return;
    }

    // free everything associated with trk
    int trk_size = trk->trk_size;

    for (int i = 0; i < trk_size; i++)
    {
        int seg_size = trk->segments[i].size;
        for (int j = 0; j < seg_size; j++)
        {
            trackpoint_destroy(trk->segments[i].trkpts[j]);
        }
        free(trk->segments[i].trkpts);
    }
    free(trk->segments);
    free(trk);
}

/**
 * Returns the number of segments in the given track.
 *
 * @param trk a pointer to a valid track
 */

int track_count_segments(const track *trk)
{
    // check for valid trk
    if (trk == NULL)
    {
        return 0;
    }
    return trk->trk_size;
}

/**
 * Returns the number of trackpoints in the given segment of the given
 * track.  The segment is specified by a 0-based index.  The return
 * value is 0 if the segment index is invalid.
 *
 * @param trk a pointer to a valid track
 * @param i a nonnegative integer less than the number of segments in trk
 * @return the number of trackpoints in the corresponding segment
 */

int track_count_points(const track *trk, int i)
{
    // check for valid trk
    if (trk == NULL)
    {
        return 0;
    }

    // check validity of other parameters
    if (i >= trk->trk_size || i < 0)
    {
        return 0;
    } 
    else
    {
        return (trk->segments[i].size);
    }
}

/**
 * Returns a copy of the given point in this track.  The segment is
 * specified as a 0-based index, and the point within the segment is
 * specified as a 0-based index into the corresponding segment.  The
 * return value is NULL if either index is invalid or if there is a memory
 * allocation error.  It is the caller's responsibility to destroy the
 * returned trackpoint.
 *
 * @param trk a pointer to a valid track
 * @param i a nonnegative integer less than the number of segments in trk
 * @param j a nonnegative integer less than the number of points in segment i
 * of track trk
 */

trackpoint *track_get_point(const track *trk, int i, int j)
{
    // check for valid trk
    if (trk == NULL)
    {
        return NULL;
    }

    // check validity of other parameters
    if (i >= trk->trk_size || i < 0 || j >= trk->segments[i].size || j < 0)
    {
        return NULL;
    }
    else
    {
        return trackpoint_copy(trk->segments[i].trkpts[j]);
    }
}

/**
 * Returns an array containing the length of each segment in this track.
 * The length of a segment is the sum of the distances between each point
 * in it and the next point.  The length of a segment with fewer than two
 * points is zero.  If there is a memory allocation error then the returned
 * pointer is NULL.  It is the caller's responsibility to free the returned
 * array.
 *
 * @param trk a pointer to a valid track
 */

double *track_get_lengths(const track *trk)
{
    // check for valid trk
    if (trk == NULL)
    {
        return NULL;
    }

    // get length fields from all segments and story into an array;
    double *seg_lengths = malloc(sizeof(double)*trk->trk_size);
    if (seg_lengths != NULL)
    {
        for (int i = 0; i < trk->trk_size; i++)
        {
            seg_lengths[i] = trk->segments[i].length;
        }  
        return seg_lengths;
    }
    return NULL;
}

/**
 * Adds a copy of the given point to the last segment in this track.
 * The point is not added and there is no change to the track if there
 * is a last point in the track (the last point in the current segment
 * or the last point on the previous segment if the current segment
 * is empty) and the timestamp on the new point is
 * not strictly after the timestamp on the last point.  There is no
 * effect if there is a memory allocation error.  The return value
 * indicates whether the point was added.  This function must execute
 * in amortized O(1) time (so a sequence of n consecutive operations must
 * work in worst-case O(n) time).
 *
 * @param trk a pointer to a valid track
 * @param pt a trackpoint with a timestamp strictly after the last trackpoint
 * in the last segment in this track (if there is such a point)
 * @return true if and only if the point was added
 */

bool track_add_point(track *trk, const trackpoint *pt)
{
    // check for valid trk
    if (trk == NULL)
    {
        return false;
    }

    int trk_size = trk->trk_size;

    // check if we are adding first point in newly created track;
    if (trk_size == 1 && trk->segments[0].size == 0 && trk->segments[0].cap == 1)
    {
        trk->segments[0].trkpts[0] = trackpoint_copy(pt);
        if (trk->segments[0].trkpts[0] != NULL)
        {
            trk->segments[0].size ++;
            return true;
        }
        else
        {
            return false;
        }
    }

    // find last seg and its parameters
    int last_seg = trk_size - 1;
    int last_seg_size = trk->segments[last_seg].size;
    int last_seg_cap = trk->segments[last_seg].cap;

    // check if last segment is not empty
    if (last_seg_size != 0)
    {
        // check if trackpoint should be added to segment by comparing timestamps
        if (trackpoint_time(pt) > trackpoint_time(trk->segments[last_seg].trkpts[last_seg_size - 1]))
        {
            // check if more space needs to be allocated
            if (last_seg_size == last_seg_cap)
            {
                trk->segments[last_seg].trkpts = realloc(trk->segments[last_seg].trkpts, sizeof(trackpoint*) * last_seg_cap * 2);
                if (trk->segments[last_seg].trkpts != NULL)
                {
                    trk->segments[last_seg].cap *= 2;
                }
                else
                {
                    return false;
                }
            }

            // add trackpoint to segment
            trk->segments[last_seg].trkpts[last_seg_size] = trackpoint_copy(pt);

            // if there are no allocation errors, increment size of segment, and add the added distance into segment
            if (trk->segments[last_seg].trkpts[last_seg_size] != NULL)
            {
                trk->segments[last_seg].size ++;
                location l1 = trackpoint_location(trk->segments[last_seg].trkpts[last_seg_size - 1]);
                location l2 = trackpoint_location(trk->segments[last_seg].trkpts[last_seg_size]);
                trk->segments[last_seg].length += location_distance(&l1, &l2);
                return true;
            }      
            else
            {
                return false;
            }
        }
    }
    else
    {
        // get the last trackpoint of segment previous to the last;
        int prev_seg = last_seg - 1;
        int prev_seg_size = trk->segments[prev_seg].size;

        // check if point should be added by comparing timestamps
        if (trackpoint_time(pt) > trackpoint_time(trk->segments[prev_seg].trkpts[prev_seg_size - 1]))
        {
            // check if the last seg has capacity to store point and add pt after necessary allocations
            if (last_seg_cap != 0)
            {
                trk->segments[last_seg].trkpts[0] = trackpoint_copy(pt);
                if (trk->segments[last_seg].trkpts[0] != NULL)
                {
                    trk->segments[last_seg].size ++;
                    trk->segments[last_seg].length = 0;
                    return true;
                }                
            }
            else
            {
                trk->segments[last_seg].trkpts = malloc(sizeof(trackpoint*));
                if (trk->segments[last_seg].trkpts != NULL)
                {
                    trk->segments[last_seg].cap = 1;
                    trk->segments[last_seg].trkpts[0] = trackpoint_copy(pt);
                    if (trk->segments[last_seg].trkpts[0] != NULL)
                    {
                        trk->segments[last_seg].size ++;
                        trk->segments[last_seg].length = 0;
                        return true;
                    }       
                }
            }
        }
    }
    return false;
}

/**
 * Starts a new segment in the given track.  There is no effect on the track
 * if the current segment is empty or if there is a memory allocation error.
 *
 * @param trk a pointer to a valid track
 */
void track_start_segment(track *trk)
{
    // check for valid trk
    if (trk == NULL)
    {
        return;
    }

    int trk_size = trk->trk_size;
    int trk_cap = trk->cap;
    int last_seg = trk_size - 1;
    int last_seg_size = trk->segments[last_seg].size;

    // check if segment is empty
    if (last_seg_size != 0)
    {
        // check if capacity of track has been reached and allocate memory if necessary
        if (trk_size == trk_cap)
        {
            trk->segments = realloc(trk->segments, sizeof(segment) * trk_cap * 2);
            if (trk->segments != NULL)
            {
                trk->cap *= 2;
            }
            else
            {
                return;
            }
                   
        }

        // make new segment
        trk->segments[last_seg + 1].trkpts = malloc(sizeof(trackpoint*));
        if (trk->segments[last_seg + 1].trkpts != NULL)
        {
            trk->trk_size ++;
            trk->segments[last_seg + 1].length = 0;
            trk->segments[last_seg + 1].size = 0;
            trk->segments[last_seg + 1].cap = 1;
            return;
        } 
    }
    return;
}


/**
 * Merges the given range of segments in this track into one.  The segments
 * to merge are specified as the 0-based index of the first segment to
 * merge and one more than the index of the last segment to merge.
 * The resulting segment replaces the first merged one and later segments
 * are moved up to replace the other merged segments.  If the range is
 * invalid then there is no effect.
 *
 * @param trk a pointer to a valid track
 * @param start an integer greather than or equal to 0 and strictly less than
 * the number if segments in trk
 * @param end an integer greater than or equal to start and less than or
 * equal to the number of segments in trk
 */
void track_merge_segments(track *trk, int start, int end)
{
    // check for valid trk
    if (trk == NULL)
    {
        return;
    }

    int trk_size = trk->trk_size;
    
    // check validity of other parameters
    if (start < 0 || start >= trk_size || end > trk_size || end < start + 2)
    {
        return;
    }
    else
    {
        int diff = end - start;
        int start_size = trk->segments[start].size;
        int start_cap = trk->segments[start].cap;
        int trk_size = trk->trk_size;

        // initialize new size and new length of our start segment
        int new_size = 0;
        double new_length = 0;
        
        // add sizes and lengths of segments being merged
        for (int i = 0; i < diff; i++)
        {
            new_size += trk->segments[start + i].size;
            new_length += trk->segments[start + i].length;
        }

        // check if the start segment has enough capacity
        if (new_size > start_cap)
        {
            trk->segments[start].trkpts = realloc(trk->segments[start].trkpts, sizeof(trackpoint*) * new_size);
            if (trk->segments[start].trkpts != NULL)
            {
                trk->segments[start].cap = new_size;
            }
            else
            {
                return;
            }
        }

        // set newly allocated trackpoint*'s in start segment to trackpoint*'s in merged segments, set trackpoint*'s in merged segments to NULL
        for (int i = 1; i < diff; i++)
        {
            int curr_seg_size = trk->segments[start + i].size;

            for (int j = 0; j < curr_seg_size; j++)
            {
                trk->segments[start].trkpts[start_size + j] = trk->segments[start + i].trkpts[j];
                trk->segments[start + i].trkpts[j] = NULL;
            }

            // each time we join segments add the distance between the end of the start segment to start of the merged segment
            location l1 = trackpoint_location(trk->segments[start].trkpts[start_size]);
            location l2 = trackpoint_location(trk->segments[start].trkpts[start_size - 1]);
            new_length += location_distance(&l1, &l2);
            start_size += curr_seg_size;
        }

        // set new size and length of newly merged segment
        trk->segments[start].size = new_size;
        trk->segments[start].length = new_length;

        // shift new trk to get rid of gap where merged segments used to be
        for (int i = end; i < trk_size; i ++)
        {
            // move segments after and including the end segment to directly follow start segments
            free(trk->segments[start + i - end + 1].trkpts);
            trk->segments[start + i - end + 1].trkpts = trk->segments[i].trkpts;
            trk->segments[start + i - end + 1].size = trk->segments[i].size;
            trk->segments[start + i - end  + 1].length = trk->segments[i].length;
            trk->segments[start + i - end + 1].cap = trk->segments[i].cap;

            // turn segments at the end into empty segments with 0 capacity;
            trk->segments[i].trkpts = NULL;
            trk->segments[i].size = 0;
            trk->segments[i].length = 0;
            trk->segments[i].cap = 0;
        }

        // set new track size
        trk->trk_size -= (diff - 1);
    }
}


/**
 * Creates a heatmap of the given track.  The heatmap will be a
 * rectangular 2-D array with each row separately allocated.  The last
 * three paramters are (simulated) reference parameters used to return
 * the heatmap and its dimensions.  Each element in the heatmap
 * represents an area bounded by two circles of latitude and two
 * meridians of longitude.  The circle of latitude bounding the top of
 * the top row is the northernmost (highest) latitude of any
 * trackpoint in the given track.  The meridian bounding the left of
 * the first column is the western edge of the smallest spherical
 * wedge bounded by two meridians the contains all the points in the
 * track (the "western edge" for a nontrivial wedge being the one
 * that, when you move east from it along the equator, you stay in the
 * wedge).  When there are multple such wedges, choose the one with
 * the lowest normalized (adjusted to the range -180 (inclusive) to
 * 180 (exclusive)) longitude.  The distance (in degrees) between the
 * bounds of adjacent rows and columns is given by the last two
 * parameters.  The heat map will have just enough rows and just
 * enough columns so that all points in the track fall into some cell.
 * The value in each entry in the heatmap is the number of trackpoints
 * located in the corresponding cell.  If a trackpoint is on the
 * border of two or more cells then it is counted in the bottommost
 * and rightmost cell it is on the border of, but do not add a row or
 * column just to place points on the south and east borders into
 * them and instead place the points on those borders by breaking ties
 * only between cells that already exist.
 * If there are no trackpoints in the track then the function
 * creates a 1x1 heatmap with the single element having a value of 0.
 * If the cell size is invalid or if there is a memory allocation
 * error then the map is set to NULL and the rows and columns
 * parameters are unchanged.  It is the caller's responsibility to
 * free each row in the returned array and the array itself.
 *
 * @param trk a pointer to a valid trackpoint
 * @param cell_width a positive double less than or equal to 360.0
 * @param cell_height a positive double less than or equal to 180.0
 * @param map a pointer to a pointer to a 2-D array of ints
 * @param rows a pointer to an int
 * @param cols a pointer to an int
 */
void track_heatmap(const track *trk, double cell_width, double cell_height, int ***map, int *rows, int *cols)
{
    // check validity of parameters
    if (trk == NULL || cell_width <= 0.0 || cell_width > 360.0 || cell_height <= 0.0 || cell_height > 180.0)
    {
        return;
    }

    // find number of total points
    int trk_size = trk->trk_size;
    int num_of_pts = 0;
    for (int i = 0; i < trk_size; i++)
    {
        num_of_pts += trk->segments[i].size;
    }

    // check if number of pts is = 0 or = 1;
    if (num_of_pts == 0 || num_of_pts == 1)
    {
        // set rows and cols values
        *rows = 1;
        *cols = 1;

        int** m;

        // initialize rows of map
        m = malloc(sizeof(int*) * *rows);
        if (m == NULL)
        {
            return;
        }

        // initialize columns of map
        for (int i = 0; i < *rows; i++)
        {
            m[i] = calloc(*cols , sizeof(int));
            if (m[i] == NULL)
            {
                return;
            }
        }

        // increment box if number of points is equal to 1
        if (num_of_pts == 1)
        {
            m[0][0] = 1;
        }

        // pass in m to *map
        *map = m;
        return;
    }

    // copy trackpoints in given struct into trkpts
    trackpoint ** trkpts = malloc(sizeof(trackpoint*) * num_of_pts);
    if (trkpts == NULL)
    {
        return;
    }

    int count = 0;
    for (int i = 0; i < trk_size; i++)
    {
        int curr_seg_size = trk->segments[i].size;
        for (int j = 0; j < curr_seg_size; j++)
        {
            trkpts[count] = trackpoint_copy(trk->segments[i].trkpts[j]);
            if (trkpts[count] == NULL)
            {
                return;
            }
            count++;
        }
    }

    // bubble sort array in increasing order of longitude
    for (int i = 0; i < num_of_pts - 1; i++)
    {
        for (int j = 0; j < num_of_pts - 1 - i; j++)
        {
            double currlon = trackpoint_location(trkpts[j]).lon;
            double nextlon = trackpoint_location(trkpts[j + 1]).lon;
            if (currlon > nextlon)
            {
                trackpoint * tmp = trkpts[j];
                trkpts[j] = trkpts[j + 1];
                trkpts[j + 1] = tmp;
            }
        }
    }

    // calculate differences between each pair of longitudes
    double lon_diffs[num_of_pts];

    for (int i = 0; i < num_of_pts; i++)
    {
        if (i == num_of_pts - 1)
        {
            double currlon = trackpoint_location(trkpts[i]).lon;
            double nextlon = trackpoint_location(trkpts[0]).lon;
            double diff = 360 - (currlon - nextlon);
            lon_diffs[i] = diff;
        }
        else
        {
            double currlon = trackpoint_location(trkpts[i]).lon;
            double nextlon = trackpoint_location(trkpts[i + 1]).lon;
            double diff = nextlon - currlon;
            lon_diffs[i] = diff;
        }
    }    

    // get index that gives you max difference in longitude
    int max_diff_index = 0;
    for (int i = 0; i < num_of_pts; i++)
    {
        if (lon_diffs[max_diff_index] < lon_diffs[i])
        {
            max_diff_index = i;
        }
    }

    // set index as left border and the point it connects to as right border
    int right_ind = max_diff_index;
    int left_ind = (right_ind + 1) % num_of_pts;
    
    // calculate max lat;
    int max_lat_ind = 0;
    for (int i = 0; i < num_of_pts; i++)
    {
        if (trackpoint_location(trkpts[i]).lat > trackpoint_location(trkpts[max_lat_ind]).lat)
        {
            max_lat_ind = i;
        }
    }

    // calculate min lat;
    int min_lat_ind = 0;
    for (int i = 0; i < num_of_pts; i++)
    {
        if (trackpoint_location(trkpts[i]).lat < trackpoint_location(trkpts[min_lat_ind]).lat)
        {
            min_lat_ind = i;
        }
    }

    // set map/grid parameters
    double grid_height = trackpoint_location(trkpts[max_lat_ind]).lat - trackpoint_location(trkpts[min_lat_ind]).lat;
    double grid_width = 360.0 - lon_diffs[max_diff_index];

    // set rows and cols
    *rows = (int) ceil(grid_height/cell_height);
    *cols = (int) ceil(grid_width/cell_width);

    int** m;

    // initialize rows of map
    m = malloc(sizeof(int*) * *rows);
    if (m == NULL)
    {
        return;
    }

    // initialize columns of map
    for (int i = 0; i < *rows; i++)
    {
        m[i] = calloc(*cols , sizeof(int));
        if (m[i] == NULL)
        {
            return;
        }
    }

    // normalize boundaries of grid
    double lower_lat = trackpoint_location(trkpts[min_lat_ind]).lat;
    lower_lat += 90.0;

    double left_lon = trackpoint_location(trkpts[left_ind]).lon;
    left_lon += 180.0;

    double upper_lat = trackpoint_location(trkpts[max_lat_ind]).lat;
    upper_lat += 90.0;

    double right_lon = trackpoint_location(trkpts[right_ind]).lon;
    right_lon += 180.0; 

    // sort into grid;
    for (int i = 0; i < num_of_pts; i++)
    {
        double lat = trackpoint_location(trkpts[i]).lat;
        lat += 90.0;

        double lon = trackpoint_location(trkpts[i]).lon;
        lon += 180.0;
        
        // check if you need to wrap around
        if (left_ind  > right_ind)
        {
            if (lon <= right_lon)
            {
                lon = lon - left_lon + 360;
            }
            else
            {
                lon = lon - left_lon;
            }
            lat = upper_lat - lat;
        }
        else
        {
            lon = lon - left_lon;
            lat = upper_lat - lat;
        }

        // find row and col to place location in grid
        int row = floor(lat/cell_height);
        int col = floor(lon/cell_width);

        // check if location is on the edge of the last row or last column
        if (col == *cols)
        {
            col = *cols - 1;
        }

        if (row == *rows)
        {
            row = *rows - 1;
        }

        // increment count of points in a certain box in grid;
        m[row][col]++;
    }

    // pass values to map;
    *map = m;

    // free trkpts;
    for (int i = 0; i < num_of_pts; i++)
    {
        trackpoint_destroy(trkpts[i]);
    }
    free(trkpts);
}
