#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smap.h"

// typdef info struct based on information needed to run the Blotto game
typedef struct info {
    char *key;
    int *dist;
    int gplayed;
    double gwon;
    double score;
} info;

void freeVal (const char *key, void *value, void *arg);
int cmpwin (const void * value1, const void * value2);
int cmpscore (const void * value1, const void * value2);

int main (int argc, char *argv[])
{
    // check for correct argc
    if (argc < 4)
    {
        exit(1);
    }

    // open file
    FILE *in;
    in = fopen(argv[1], "r");

    // check if file was opened properly;
    if (in == NULL)
    {
        exit(2);
    }

    // check if the second argument is either win or score
    if (strcmp(argv[2], "win") != 0)
    {
        if (strcmp(argv[2], "score") != 0)
        {
            fclose(in);
            exit(3);
        }
    }

    // initialize array holding values per battlefield
    int num_bf = argc - 3;
    int values_bf[num_bf];

    for (int i = 0; i < num_bf; i++)
    {
        int val = atoi(argv[3 + i]);
        if (val <= 0)
        {
            fclose(in);
            exit(4);
        }
        else
        {
            values_bf[i] = val;
        }
    }

    // read stin into hash map of players and their values
    char* line = NULL;
    size_t size = 0;
    int prev_num_armies = 0;
    smap *m = smap_create(smap_default_hash);

    while (getline(&line, &size, stdin) > 1)
    {
        // make sure player name is read
        char *player = strtok(line, ",");
        if (player == NULL)
        {
            fclose(in);
            free(line);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(3);
        }
        
        // make sure value was allocated properly
        info *value = malloc(sizeof(info));
        if (value == NULL)
        {
            fclose(in);
            free(line);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(4);
        }

        // set fields of value
        value->key = strdup(player);
        if (value->key == NULL)
        {
            fclose(in);
            free(line);
            free(value);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(4);
        }
        value->dist = malloc(num_bf * sizeof(int));
        if (value->dist == NULL)
        {
            fclose(in);
            free(line);
            free(value->key);
            free(value);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(5);
        }
        value->gplayed = 0;
        value->gwon = 0.0;
        value->score = 0.0;

        // initialize player's distribution of armies
        int num_armies = 0;
        for (int i = 0; i < num_bf; i++)
        {
            char* bf = strtok(NULL, ",");
            if (bf == NULL)
            {
                fclose(in);
                free(line);
                free(value->dist);
                free(value->key);
                free(value);
                smap_for_each(m, freeVal, NULL);
                smap_destroy(m);
                exit(3);
            }
            int val = atoi(bf);

            // make sure distribution is valid
            if (val < 0)
            {
                fclose(in);
                free(line);
                free(value->dist);
                free(value->key);
                free(value);
                smap_for_each(m, freeVal, NULL);
                smap_destroy(m);
                exit(6);
            }
            else
            {
                value->dist[i] = val;
                num_armies += val;
            }
        }

        // check if number of armies is the same for each player
        if (prev_num_armies == 0 && num_armies > 0)
        {
            prev_num_armies = num_armies;
        }
        else if (prev_num_armies != num_armies)
        {
            fclose(in);
            free(line);
            free(value->dist);
            free(value->key);
            free(value);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(7);
        }
        
        // put player and value as an entry in our hash map
        bool put_status = smap_put(m, player, value);

        // check if put was successful
        if (put_status == false)
        {
            fclose(in);
            free(line);
            free(value->dist);
            free(value->key);
            free(value);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(7);
        }

        // free the line read and set it to NULL for next line
        free(line);
        line = NULL;
    }

    // free line after reading stin
    free(line);
    
    // initialize parameters for reading file
    char line1[100];
    char p1 [32];
    char p2 [32];

    // read until EOF
    while (fgets(line1, 100, in) != NULL)
    {
        // make sure to only read 31 characters of each name
        if (sscanf(line1, "%31s %31s", p1, p2) != 2)
        {
            fclose(in);
            smap_for_each(m, freeVal, NULL);
            smap_destroy(m);
            exit(7);
        }
        else
        {
            // get the pointer to each player's respective values
            info *val1 = (info *) smap_get(m, p1);
            info *val2 = (info *) smap_get(m, p2);

            if (val1 == NULL || val2 == NULL)
            {
                fclose(in);
                smap_for_each(m, freeVal, NULL);
                smap_destroy(m);
                exit(7);
            }

            // save previous scores
            double prevscore1 = val1->score;
            double prevscore2 = val2->score;

            // play the game for each battlefield
            for (int i = 0; i < num_bf; i++)
            {
                if (val1->dist[i] > val2->dist[i])
                {
                    val1->score += values_bf[i];
                }
                else if (val1->dist[i] < val2->dist[i])
                {
                    val2->score += values_bf[i];
                }
                else
                {
                    double half = values_bf[i] * .5;
                    val1->score += half;
                    val2->score += half;
                }
            }

            // after game has been tallied check who had the higher score for the win
            if ((val1->score - prevscore1) > (val2->score - prevscore2))
            {
                val1->gwon ++;
            }
            else if ((val1->score - prevscore1) < (val2->score - prevscore2))
            {
                val2->gwon ++;
            }
            else
            {
                val1->gwon += 0.5;
                val2->gwon += 0.5;
            }
            val1->gplayed ++;
            val2->gplayed ++;
        }
    }

    // initialize variables to sort players in descending order
    int m_size = smap_size(m);
    const char **keys = smap_keys(m);
    if (keys == NULL)
    {
        fclose(in);
        smap_for_each(m, freeVal, NULL);
        smap_destroy(m);
        exit(7);
    }
    info *values[m_size];
    
    // get an array of pointers to all values in the array
    for (int i = 0; i < m_size; i++)
    {
        values[i] = smap_get(m, keys[i]);
    }
    
    // sort the arrray "values" based on game mode
    if (strcmp(argv[2], "win") == 0)
    {
        qsort(values, m_size, sizeof(info *), cmpwin);
        for (int i = 0; i < m_size; i++)
        {
            info player = *(values[i]);
            double avg = player.gwon / player.gplayed;

            // check if we are dividing by zero
            if (player.gplayed == 0)
            {
                avg = 0;
            }

            // print results
            if (avg < 10)
            {
                printf("  ");
            }
            else if (avg < 100)
            {
                printf(" ");
            }
            printf("%.3lf %s\n", avg, values[i]->key);
        }
    }
    else if (strcmp(argv[2], "score") == 0)
    {
        qsort(values, m_size, sizeof(info *), cmpscore);
        for (int i = 0; i < m_size; i++)
        {
            info player = *(values[i]);
            double avg = player.score / player.gplayed;

            // check if we're dividing by zero
            if (player.gplayed == 0)
            {
                avg = 0;
            }

            // print results
            if (avg < 10)
            {
                printf("  ");
            }
            else if (avg < 100)
            {
                printf(" ");
            }
            printf("%.3lf %s\n", avg, values[i]->key);
        }
    }

    // free everything
    smap_for_each(m, freeVal, NULL);
    smap_destroy(m);
    free(keys);
    fclose(in);
    return 0;
}

// ------ END OF MAIN FUNCTION --------

// free value at associated key;
void freeVal (const char *key, void *value, void *arg)
{
    if (value == NULL)
    {
        return;
    }
    else
    {
        info val = *((info *) value);
        free(val.key);
        free(val.dist);
        free(value);   
    } 
}

// comparison function based on higher avg win.
int cmpwin (const void * value1, const void * value2)
{
    const info* const* val1 = value1;
    const info* const* val2 = value2;
    double avgwin1 = (*val1)->gwon / (*val1)->gplayed;
    double avgwin2 = (*val2)->gwon / (*val2)->gplayed;

    // check if we're dividing by zero
    if ((*val1)->gplayed == 0)
    {
        avgwin1 = 0;
    }
    else if ((*val2)->gplayed == 0)
    {
        avgwin2 = 0;
    }

    if (avgwin1 < avgwin2)
    {
        return 1;
    }
    else if (avgwin1 > avgwin2)
    {
        return -1;
    }
    else
    {
        return strcmp((*val1)->key, (*val2)->key);
    }
}

// comparison function based on higher avg score
int cmpscore (const void * value1, const void * value2)
{
    const info* const* val1 = value1;
    const info* const* val2 = value2;
    double avgscore1 = (*val1)->score / (*val1)->gplayed;
    double avgscore2 = (*val2)->score / (*val2)->gplayed;

    // check if we're dividing by zero
    if ((*val1)->gplayed == 0)
    {
        avgscore1 = 0;
    }
    else if ((*val2)->gplayed == 0)
    {
        avgscore2 = 0;
    }

    if (avgscore1 < avgscore2)
    {
        return 1;
    }
    else if (avgscore1 > avgscore2)
    {
        return -1;
    }
    else
    {
        return strcmp((*val1)->key, (*val2)->key);
    }
}
